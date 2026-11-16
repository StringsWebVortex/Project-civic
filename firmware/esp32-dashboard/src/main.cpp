/*  Civic CAN Dashboard — ESP32 firmware (ESP-32S NodeMCU v1.1)
 *  Wi-Fi AP  : Civic-CAN-Monitor, WPA2 password CANmonitor26 (fixed, owner request).
 *  HTTP      : port 80, no web login (owner request).
 *  WebSocket : port 81, live JSON telemetry. Commands (TX, ARM/DISARM, UDS)
 *              are accepted from a client that sent the session token from
 *              GET /token (no web login; Wi-Fi WPA2 is the gate).
 *  HTTP POST : /update       → ESP32 OTA (multipart, image verified by Update.end)
 *  HTTP POST : /update-stm32 → STM32 flash via ROM UART bootloader (ST AN3155):
 *              vector table checked before erase, read-back verified after write.
 *  STM32 UART: USART3 PB10_TX -> ESP32 GPIO16 RX2; PB11_RX <- GPIO17 TX2.
 *              Application mode is 2,000,000 baud 8N1. ROM bootloader mode
 *              reuses these exact wires at 57,600 baud 8E1 (AN2606/AN3155).
 *  Safety    : when the last logged-in dashboard client disconnects, the
 *              ESP32 sends DISARM, so the bus never stays armed unattended.
 *  STM32 boot: GPIO26 → BOOT0, GPIO25 → NRST (open-drain). No PB2 GPIO wire:
 *              MB997 SB19 must be fitted ON to hold BOOT1 low for ROM boot.
 *  Transceiver: TJA1051 (NXP) — only supported CAN transceiver.
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <esp_task_wdt.h>
#include <Preferences.h>
#include <esp_random.h>
#include <esp_heap_caps.h>
extern "C" {
#include "gvret.h"      /* GVRET/SavvyCAN binary codec (shared with the STM32) */
}
#include <LittleFS.h>
#include <time.h>

static constexpr char     AP_SSID[]  = "Civic-CAN-Monitor";
static constexpr char     WEB_USER[] = "admin";
static constexpr int      AP_CHANNEL = 6;
static constexpr int      AP_MAX_CONN= 4;
static constexpr int      STM32_RX   = 16; /* ESP32 GPIO16 RX <- PB10 USART3_TX */
static constexpr int      STM32_TX   = 17; /* ESP32 GPIO17 TX -> PB11 USART3_RX */
static constexpr uint32_t STM32_BAUD = 2000000; /* application USART3: 8N1 */
static constexpr int STM32_BOOT0 = 26; /* push-pull: HIGH for ROM boot, LOW for flash boot */
static constexpr int STM32_NRST  = 25; /* open-drain: LOW pulse resets, HIGH releases */

/* STM32F407 Discovery MB997 Rev E boot prerequisites:
 * - PB2 is BOOT1; default SB19 OFF leaves it pulled HIGH through 10 kOhm.
 * - Fit SB19 ON to connect the 510-ohm pull-down and force BOOT1 LOW.
 *   BOOT0=1 with BOOT1=0 selects system-memory ROM boot. No PB2/BOOT1 ESP32
 *   wire exists in the supplied schematic. blEnter requires ROM 0x7F ACK
 *   before any erase operation, so a missing SB19 fails closed.
 * - The application and ROM loader both use USART3 PB10/PB11 on GPIO16/17.
 *   Application: 2 Mbaud 8N1. ROM loader: 57,600 baud 8E1 (AN2606/AN3155).
 * - PB10 also clocks the onboard MEMS microphone; USART3_TX activity disables
 *   useful microphone sampling. PB6/PB9 are reused for CAN with CS43L22 held
 *   in reset by the STM32 (onboard audio disabled). */
/* Mass erase: DS8626 Rev 12 Table 40 tME max 32 s at x8 parallelism. */
static constexpr uint32_t BL_MASS_ERASE_TIMEOUT_MS = 40000;

WebServer        http(80);
WebSocketsServer ws(81);
HardwareSerial   Stm32(2);

/* GVRET-over-TCP: SavvyCAN and RealDash connect over Wi-Fi (Network connection,
 * port 23). Every captured CAN frame is streamed out GVRET-encoded; incoming
 * GVRET BUILD_CAN_FRAME commands become STM32 TX commands (armed only). */
WiFiServer          gvretSrv(23);
WiFiClient          gvretCli;
static gvret_parser gvretParser;
static bool         gvretReady = false;

static char     lineBuf[1224];   /* must hold the STM32's longest UDS line */
static uint16_t lineLen    = 0;
static bool     lineDiscard = false;     /* inside an over-long line: drop to '\n' */

/* ── access control ── */
static String devicePass;                /* AP WPA2 + HTTP login, per device (NVS) */
static String wsToken;                   /* per boot; handed out by GET /token     */
static bool   wsAuthed[WEBSOCKETS_SERVER_CLIENT_MAX];
static bool   stm32FlashRunning = false; /* UART belongs to the ROM bootloader */

/* esp_random() is a true RNG while the RF subsystem is on (Espressif ESP-IDF
 * "Random Number Generation"), so call this after WiFi.mode(). */
static String randomSecret(size_t n) {
    static const char al[] = "abcdefghjkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    String out;
    for (size_t i = 0; i < n; i++) out += al[esp_random() % (sizeof(al) - 1)];
    return out;
}

static void loadDevicePass() {
    devicePass = "CANmonitor26";         /* fixed Wi-Fi password (owner request) */
}

static bool requireLogin() {
    return true;                         /* no web login (owner request); Wi-Fi WPA2 is the gate */
}
static uint32_t framesSeen = 0;
static uint32_t bitrate    = 0;
static bool     locked     = false;
static uint32_t dwellMs    = 0;

/* ═══════════════════════════════════════════════════════════════════
   PAGE — exactly matches the sample dashboard screenshot
   ═══════════════════════════════════════════════════════════════════ */
static const char PAGE[] PROGMEM = R"HTML(<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Vehicle Telemetry Dashboard</title><style>
*{box-sizing:border-box;margin:0;padding:0}
body{font:13px/1.5 system-ui,sans-serif;background:#0d1117;color:#c9d1d9;min-height:100vh;padding:10px 12px 92px;transition:background .18s,color .18s}
.page-head{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:6px}
.page-head h1{margin:0}
#globalModeToggle{flex:0 0 auto;padding:7px 15px;border:1px solid #58a6ff;border-radius:7px;background:#1c3a5e;color:#e6edf3;font-size:.75rem;font-weight:800;letter-spacing:.04em;cursor:pointer}
#writeModeBanner{display:none;margin:6px 0 9px;padding:7px 10px;border:1px solid #d97706;border-radius:7px;background:#3b2410;color:#ffdfaa;font-size:.72rem}
.edit-overlay{position:fixed;inset:0;z-index:300;background:#000b;align-items:center;justify-content:center;padding:16px}
.edit-dialog{width:min(440px,100%);background:#1b232d;border:1px solid #d97706;border-radius:12px;padding:15px;box-shadow:0 16px 45px #000b}
body.write-mode .edit-dialog{background:#2b1d15;border-color:#ffad55;color:#fff0d8}
body.write-mode{background:#20140f;color:#fff0d8}
body.write-mode h1{color:#ffe6bd}
body.write-mode #sideNav{background:#2a1b13;border-color:#a85d1d}
body.write-mode #sideNav a{color:#e8c8a2}
body.write-mode #sideNav a:hover,body.write-mode #sideNav a.active{background:#4b2b16;color:#ffd08a;border-color:#b86b22}
body.write-mode #statusBar,body.write-mode #vehBar{background:#2a1c14;border-color:#8e5425;color:#f1d4ad}
body.write-mode .card,body.write-mode .mcard,body.write-mode .dq-panel{background:#2b1d15;border-color:#885128;color:#ffedda}
body.write-mode .card-title,body.write-mode .mcard-val,body.write-mode .mcard-title{color:#ffe1b8}
body.write-mode thead th{color:#e6bd8c;border-color:#71421f}
body.write-mode tbody td{border-color:#53331e}
body.write-mode canvas{background:#21150f}
body.write-mode .mtab{background:#372519;border-color:#895229;color:#e7c39d}
body.write-mode .mtab.on,body.write-mode .btn,body.write-mode #globalModeToggle{background:#8a3f12;border-color:#ffad55;color:#fff1db}
body.write-mode .btn:hover{background:#a84e16}
body.write-mode .fld select,body.write-mode .fld input[type=text],body.write-mode .byte-row input{background:#21150f;border-color:#895229;color:#fff0dc}
body.write-mode #writeModeBanner{display:block}
@media(max-width:600px){.page-head{align-items:flex-start;flex-wrap:wrap}.page-head h1{font-size:1rem}#globalModeToggle{margin-left:auto}}
#sideNav{position:fixed;z-index:100;left:0;right:0;top:auto;bottom:0;width:100%;min-height:66px;padding:7px 10px calc(7px + env(safe-area-inset-bottom));background:#111820;border-top:1px solid #30363d;overflow-x:auto;overflow-y:hidden;display:flex;flex-direction:row;align-items:center;gap:5px;box-shadow:0 -5px 18px #0008;scrollbar-width:thin}
#sideNav .nav-brand{font-size:.72rem;font-weight:800;color:#e6edf3;padding:5px 9px;border-right:1px solid #30363d;margin-right:4px;white-space:nowrap;flex:0 0 auto}
#sideNav .nav-label{display:none}
#sideNav a{display:block;white-space:nowrap;flex:0 0 auto;padding:8px 10px;border-radius:7px;color:#aeb8c4;text-decoration:none;font-size:.72rem;border:1px solid transparent}
#sideNav a:hover,#sideNav a.active{background:#1c2b3a;color:#79c0ff;border-color:#29445e}
#sideNav .nav-spacer{display:none}
#sideNav .nav-foot{display:none}
#backTop{position:fixed;right:18px;bottom:82px;z-index:90;background:#1c3a5e;border:1px solid #58a6ff;color:#e6edf3;border-radius:20px;padding:8px 13px;font-weight:700;cursor:pointer;box-shadow:0 3px 12px #0008}
@media(max-width:760px){body{padding:8px 9px calc(88px + env(safe-area-inset-bottom))}#sideNav{min-height:62px;padding:6px 7px calc(6px + env(safe-area-inset-bottom));gap:3px}#sideNav .nav-brand{display:none}#sideNav a{padding:7px 9px;font-size:.68rem}#backTop{right:12px;bottom:76px}}
h1{font-size:1.1rem;font-weight:700;color:#c9d1d9;margin-bottom:10px;letter-spacing:.02em}
/* ── top metric cards ── */
.top-cards{display:grid;grid-template-columns:repeat(4,1fr) 220px;gap:8px;margin-bottom:8px;align-items:start}
.mcard{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:10px 12px;position:relative}
.mcard-head{display:flex;justify-content:space-between;align-items:center;margin-bottom:2px}
.mcard-title{font-size:.75rem;font-weight:600;color:#8b949e;letter-spacing:.04em;text-transform:uppercase}
.badge{font-size:.65rem;font-weight:700;padding:1px 7px;border-radius:20px;letter-spacing:.03em}
.badge-ok  {background:#1a3829;color:#56d364}
.badge-warn{background:#3d2e08;color:#e3b341}
.badge-alrt{background:#3d1212;color:#f85149}
.badge-none{background:#21262d;color:#8b949e}
.mcard-val{font-size:1.9rem;font-weight:700;line-height:1.05;color:#e6edf3;margin:3px 0 2px}
.mcard-val small{font-size:.75rem;font-weight:400;color:#8b949e}
.mcard-sub{font-size:.68rem;color:#8b949e;margin-bottom:4px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.spark svg{display:block;width:100%;height:38px}
/* ── quality side panel ── */
.dq-panel{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:10px 12px}
.dq-title{font-size:.72rem;font-weight:700;color:#8b949e;text-transform:uppercase;letter-spacing:.04em;margin-bottom:8px}
.dq-row{display:flex;align-items:center;gap:6px;margin-bottom:5px}
.dq-name{font-size:.7rem;color:#8b949e;width:52px;flex-shrink:0}
.dq-bar-wrap{flex:1;height:7px;background:#21262d;border-radius:3px;overflow:hidden}
.dq-bar{height:7px;border-radius:3px;transition:width .5s}
.dq-pct{font-size:.68rem;width:28px;text-align:right;flex-shrink:0}
/* ── main grid ── */
.main{display:grid;grid-template-columns:1fr 320px;gap:8px;margin-bottom:8px}
.left-col{display:flex;flex-direction:column;gap:8px}
.right-col{display:flex;flex-direction:column;gap:8px}
/* ── cards ── */
.card{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:10px 12px}
.card-head{display:flex;justify-content:space-between;align-items:center;margin-bottom:6px}
.card-title{font-size:.78rem;font-weight:700;color:#c9d1d9}
.card-sub{font-size:.65rem;color:#8b949e}
/* ── chart canvases ── */
canvas{display:block;border-radius:4px;background:#0d1117}
/* ── temp legend ── */
.tleg{display:flex;gap:12px;font-size:.68rem;color:#8b949e;align-items:center}
.tleg-dot{width:7px;height:7px;border-radius:50%;display:inline-block;margin-right:3px;vertical-align:middle}
/* ── alerts table ── */
table{width:100%;border-collapse:collapse;font-size:.72rem}
thead th{color:#8b949e;padding:4px 8px;border-bottom:1px solid #30363d;text-align:left;font-weight:600;white-space:nowrap}
tbody td{padding:4px 8px;border-bottom:1px solid #21262d;white-space:nowrap}
.al-warn td{background:rgba(227,179,65,.07)}
.al-crit td{background:rgba(248,81,73,.07)}
/* ── status bar ── */
#statusBar{font-size:.68rem;color:#8b949e;margin-bottom:8px;padding:4px 10px;background:#161b22;border:1px solid #30363d;border-radius:6px}
/* ── CAN analyzer ── */
.can-wrap{overflow-x:auto;max-height:300px;overflow-y:auto}
.can-wrap table{font-size:.68rem;font-family:'ui-monospace',monospace}
.can-wrap thead th{padding:3px 6px}
.can-wrap tbody td{padding:2px 6px}
.live-badge{font-size:.65rem;font-weight:700;padding:2px 8px;border-radius:20px;background:#1a3829;color:#56d364}
.live-badge.off{background:#21262d;color:#8b949e}
/* ── sig explorer ── */
.sig-group{border:1px solid #21262d;border-radius:6px;margin-bottom:6px;overflow:hidden}
.sig-group-hdr{padding:4px 10px;background:#1c2128;font-size:.7rem;font-weight:600;color:#79c0ff;border-bottom:1px solid #21262d}
.sig-row{cursor:pointer}
.sig-row:hover td{background:#1c2128}
.sig-row.sel td{background:#162032}
/* ── mode panel ── */
.mode-tabs{display:flex;gap:6px;margin-bottom:8px}
.mtab{padding:4px 14px;border-radius:6px;border:1px solid #30363d;background:#21262d;color:#8b949e;cursor:pointer;font-size:.75rem;font-weight:600}
.mtab.on{background:#1c3a5e;border-color:#58a6ff;color:#c9d1d9}
.fld{margin:6px 0}
.fld label{font-size:.72rem;color:#8b949e;display:block;margin-bottom:2px}
.fld select,.fld input[type=text]{padding:5px 8px;background:#21262d;border:1px solid #30363d;border-radius:5px;color:#e6edf3;font-size:.75rem}
.byte-row{display:flex;gap:4px;flex-wrap:wrap;margin:4px 0}
.byte-row>label{display:flex;flex-direction:column;align-items:center;gap:2px;font-size:.65rem;color:#8b949e}
.byte-row input{width:32px;text-align:center;padding:3px;background:#21262d;border:1px solid #30363d;border-radius:4px;color:#c9d1d9;font-family:monospace;font-size:.75rem}
.btn{padding:5px 14px;border-radius:6px;border:1px solid #30363d;background:#1c3a5e;color:#c9d1d9;cursor:pointer;font-size:.75rem;font-weight:600;margin-right:6px}
.btn:hover{background:#1f4878}
.btn-sm{padding:3px 10px;font-size:.68rem}
#txStat{margin-top:5px;font-size:.68rem;color:#8b949e;min-height:1.2em}
/* ── full-width sections ── */
.full{margin-bottom:8px}
@media(max-width:900px){.top-cards{grid-template-columns:repeat(2,1fr)}.main{grid-template-columns:1fr}}
</style></head><body>
<nav id="sideNav" aria-label="Dashboard navigation">
  <div class="nav-brand">CIVIC CAN<br><span style="font-size:.65rem;color:#8b949e;font-weight:500">Vehicle Monitor</span></div>
  <div class="nav-label">Overview</div>
  <a href="#top" class="active">⌂ Dashboard</a>
  <a href="#events">⚑ Fault Events</a>
  <a href="#canAnalyzer">⌁ CAN Analyzer</a>
  <a href="#signalExplorer">◫ Signal Explorer</a>
  <div class="nav-label">Diagnostics</div>
  <a href="#controllers">▤ Controllers &amp; Bus</a>
  <a href="#diagnostics">⚙ Fault Codes &amp; Sensors</a>
  <a href="#operatingMode">⇄ Operating Mode</a>
  <div class="nav-label">Data &amp; Tools</div>
  <a href="/log.csv">↓ Download Current Data</a>
  <a href="/log.csv?historical=1">◷ Historical Data (CSV)</a>
  <a href="#fwCard">↻ Firmware Update</a>
  <div class="nav-spacer"></div>
  <a href="#top">↑ Back to Dashboard Top</a>
  <div class="nav-foot">Navigation stays available while scrolling</div>
</nav>
<div id="top"></div>
<div class="page-head"><h1>Vehicle Telemetry Dashboard</h1><button id="globalModeToggle" type="button" onclick="toggleGlobalMode()">ENTER WRITE MODE</button></div>
<div id="writeModeBanner">WRITE DISPLAY MODE · Read telemetry and event monitoring continue in the background. Live sensor edits below are staged locally only; they are not transmitted to the vehicle because these measurements do not have verified write encoders.</div>
<div id="statusBar">Connecting…</div>
<div id="vehBar" style="margin:6px 0 10px;padding:8px 12px;border:1px solid #30363d;border-radius:8px;font-size:.85rem">Key: <b>—</b></div>

<!-- ══ TOP ROW: 4 metric cards + data quality panel ══ -->
<div class="top-cards">

  <!-- RPM -->
  <div class="mcard">
    <div class="mcard-head"><span class="mcard-title">RPM</span><span class="badge badge-none" id="b0C">NO DATA</span></div>
    <div class="mcard-val" id="v0C">—<small> rpm</small></div>
    <div class="mcard-sub" id="s0C">Avg — rpm</div>
    <div class="spark"><svg id="sk0C" viewBox="0 0 260 38" preserveAspectRatio="none"><line x1="0" y1="30" x2="260" y2="30" stroke="#30363d" stroke-width="1"/></svg></div>
  </div>

  <!-- Coolant Temp -->
  <div class="mcard">
    <div class="mcard-head"><span class="mcard-title">Coolant Temp</span><span class="badge badge-none" id="b05">NO DATA</span></div>
    <div class="mcard-val" id="v05">—<small> °C</small></div>
    <div class="mcard-sub" id="s05">Range —–— °C</div>
    <div class="spark"><svg id="sk05" viewBox="0 0 260 38" preserveAspectRatio="none"><line x1="0" y1="30" x2="260" y2="30" stroke="#30363d" stroke-width="1"/></svg></div>
  </div>

  <!-- Fuel Level -->
  <div class="mcard">
    <div class="mcard-head"><span class="mcard-title">Fuel Level</span><span class="badge badge-none" id="b2F">NO DATA</span></div>
    <div class="mcard-val" id="v2F">—<small> %</small></div>
    <div class="mcard-sub" id="s2F">Most common —</div>
    <div class="spark"><svg id="sk2F" viewBox="0 0 260 38" preserveAspectRatio="none"><line x1="0" y1="30" x2="260" y2="30" stroke="#30363d" stroke-width="1"/></svg></div>
  </div>

  <!-- Battery Voltage -->
  <div class="mcard">
    <div class="mcard-head"><span class="mcard-title">Battery Voltage</span><span class="badge badge-none" id="b42">NO DATA</span></div>
    <div class="mcard-val" id="v42">—<small> V</small></div>
    <div class="mcard-sub" id="s42">Avg — V</div>
    <div class="spark"><svg id="sk42" viewBox="0 0 260 38" preserveAspectRatio="none"><line x1="0" y1="30" x2="260" y2="30" stroke="#30363d" stroke-width="1"/></svg></div>
  </div>

  <!-- Data Quality panel (top-right, next to metric cards) -->
  <div class="dq-panel">
    <div class="dq-title">Data Quality <span style="font-weight:400;text-transform:none">Missing % by Metric</span></div>
    <div id="dqRows"></div>
  </div>

</div><!-- end top-cards -->

<!-- ══ DASHBOARD: every received live ECU sensor value ══ -->
<div class="card full" id="liveEngineSensors">
  <div class="card-head"><span class="card-title">Live Engine Sensors &amp; Parameters</span><span class="live-badge off" id="liveSensorState">WAITING FOR ECU DATA</span></div>
  <div style="font-size:.68rem;color:#8b949e;margin-bottom:7px">F-CAN read-only polling (SAE Mode 01 sensors, Honda gear DID 2612, and DTC checks 03/07/0A + UDS 19/02 every 30 s) runs only in key position II while the F-CAN ARM window is explicitly active. It stops on key-off or ARM expiry; no diagnostic requests are sent while disarmed.</div>
  <div id="autoPidState" style="font-size:.68rem;color:#8b949e;margin-bottom:6px">Automatic PID monitor: waiting for key II and F-CAN ARM.</div>
  <div style="border:1px solid #30363d;border-radius:7px;padding:8px;margin:7px 0;background:#101820">
    <div style="font-size:.72rem;font-weight:700">Honda transmission gear acquisition · UDS 22 2612</div>
    <div id="hondaGearState" style="font-size:.65rem;color:#8b949e;margin:4px 0">Waiting for key II and F-CAN ARM.</div>
    <div style="display:flex;gap:16px;flex-wrap:wrap;font-size:.72rem">
      <span>Gear variant 2: <b id="hondaGearV2">—</b> <small>raw</small></span>
      <span>Gear variant 3: <b id="hondaGearV3">—</b> <small>raw</small></span>
      <span>Last reply: <b id="hondaGearAge">—</b></span>
    </div>
    <div style="font-size:.62rem;color:#8b949e;margin-top:4px">29-bit physical read DA0E→0E; response bits 352–359 and 368–375. Raw candidates until the returned values are checked against the shift lever on this vehicle.</div>
    <div style="display:flex;gap:14px;flex-wrap:wrap;align-items:center;font-size:.68rem;margin-top:6px">
      <label><input type="checkbox" id="gearLedOn" checked onchange="gearLedToggle()"> STM32 LED shift chase (LD4 green → LD3 orange → LD5 red → LD6 blue on upshift, reverse on downshift)</label>
      <span>Source: <select id="gearLedSrc" style="font-size:.68rem"><option value="v2" selected>variant 2</option><option value="v3">variant 3</option></select></span>
      <span>Last sent: <b id="gearLedLast">—</b></span>
    </div>
    <div style="font-size:.62rem;color:#8b949e;margin-top:3px">A gear is forwarded to the STM32 (GEAR,&lt;n&gt;) only after two consecutive identical replies; the STM32 animates only on a change and never drives a bus from it. The MB997 has four separate single-colour user LEDs (no RGB LED), so the light→dark colour order is mapped onto those four.</div>
  </div>
  <div style="border:1px solid #30363d;border-radius:7px;padding:8px;margin:7px 0;background:#101820">
    <div style="font-size:.72rem;font-weight:700">R18Z1 5AT substitute candidates · ECU 1E (18DA1EF1 → 18DAF11E) · UDS 22 2301 / 22 2322 / 22 3086</div>
    <div id="hondaAltState" style="font-size:.65rem;color:#8b949e;margin:4px 0">Waiting for key II and F-CAN ARM.</div>
    <div style="display:flex;gap:16px;flex-wrap:wrap;font-size:.7rem;font-family:monospace">
      <span>2301: <b id="hondaAlt2301">—</b></span>
      <span>2322: <b id="hondaAlt2322">—</b></span>
      <span>3086: <b id="hondaAlt3086">—</b></span>
    </div>
    <div style="font-size:.62rem;color:#8b949e;margin-top:4px">Read-only round-robin (one DID per 500 ms) interleaved with DID 2612. Raw response bytes after the 62 xx xx header are shown as hex so the owner can correlate them with the shift lever; negative responses (7F 22 xx) are shown as-is. Nothing is decoded or acted on from these values.</div>
  </div>
  <div class="can-wrap" style="max-height:250px;overflow:auto"><table><thead><tr><th>PID</th><th>Sensor / Parameter</th><th>Live value</th></tr></thead>
    <tbody id="pidRows"><tr><td colspan="3" style="color:#8b949e">Waiting for engine / ECU telemetry…</td></tr></tbody></table></div>
</div>

<!-- ══ PERSISTENT DTC EVENT JOURNAL ══ -->
<div class="card full" id="events">
  <div class="card-head"><span class="card-title">Fault Event Journal</span><span class="card-sub" id="eventSummary">0 current · 0 historical</span></div>
  <div style="font-size:.68rem;color:#8b949e;margin-bottom:7px">Each ECU-reported DTC is journaled with up to 60 seconds of pre-trigger and 10 seconds of post-trigger telemetry. Current alarms remain until marked seen; seen records move to history and survive reboot.</div>
  <div class="can-wrap" style="max-height:240px;overflow:auto"><table><thead><tr><th>Captured</th><th>Bus / ECU</th><th>DTC</th><th>Source / repeats</th><th>Snapshot</th><th></th></tr></thead>
    <tbody id="eventCurrent"><tr><td colspan="6" style="color:#8b949e">Waiting for event journal…</td></tr></tbody></table></div>
  <details style="margin-top:8px"><summary style="cursor:pointer;color:#8b949e;font-size:.72rem">Historical events (marked seen)</summary>
    <div class="can-wrap" style="max-height:240px;overflow:auto;margin-top:6px"><table><thead><tr><th>Captured</th><th>Bus / ECU</th><th>DTC</th><th>Source / repeats</th><th>Snapshot</th></tr></thead><tbody id="eventHistory"><tr><td colspan="5" style="color:#8b949e">No history yet.</td></tr></tbody></table></div>
  </details>
</div>

<!-- ══ MAIN GRID: left charts + right panels ══ -->
<div class="main">
  <div class="left-col">

    <!-- RPM Over Samples -->
    <div class="card">
      <div class="card-head">
        <span class="card-title">RPM Over Samples</span>
        <span class="card-sub" id="rpmSubLbl">— samples</span>
      </div>
      <canvas id="cRpm" height="200" style="width:100%"></canvas>
    </div>

    <!-- Temperatures -->
    <div class="card">
      <div class="card-head">
        <span class="card-title">Temperatures</span>
        <div class="tleg">
          <span><span class="tleg-dot" style="background:#f85149"></span>Coolant</span>
          <span><span class="tleg-dot" style="background:#e3b341"></span>Intake</span>
          <span><span class="tleg-dot" style="background:#56d364"></span>Ambient</span>
        </div>
      </div>
      <canvas id="cTemp" height="130" style="width:100%"></canvas>
    </div>

    <!-- Alerts -->
    <div class="card">
      <div class="card-head"><span class="card-title">Alerts</span><span class="card-sub" id="alCount">0 active</span></div>
      <table>
        <thead><tr><th>Status</th><th>Rule</th><th>Value</th><th>Time</th></tr></thead>
        <tbody id="alBody"><tr><td colspan="4" style="color:#8b949e;padding:8px">No alerts.</td></tr></tbody>
      </table>
    </div>

  </div><!-- end left-col -->

  <div class="right-col">

    <!-- RPM vs Throttle -->
    <div class="card">
      <div class="card-head"><span class="card-title">RPM vs Throttle</span></div>
      <div style="display:grid;grid-template-columns:1fr 80px;gap:4px;margin-top:4px">
        <canvas id="cScat" height="200" style="width:100%"></canvas>
        <canvas id="cHist" height="200" style="width:100%"></canvas>
      </div>
      <div style="font-size:.63rem;color:#8b949e;margin-top:3px;text-align:center">Throttle (%)</div>
    </div>

  </div><!-- end right-col -->
</div><!-- end main -->

<!-- ══ FULL WIDTH: CAN Analyzer ══ -->
<div class="card full" id="canAnalyzer">
  <div class="card-head">
    <span class="card-title">CAN Analyzer</span>
    <span class="live-badge off" id="canBadge">WAITING FOR FRAMES</span>
  </div>
  <div class="can-wrap">
    <table>
      <thead><tr><th>ID</th><th>DLC</th><th>B0</th><th>B1</th><th>B2</th><th>B3</th><th>B4</th><th>B5</th><th>B6</th><th>B7</th><th>Rate</th><th>Frames</th><th>Time</th></tr></thead>
      <tbody id="canBody"><tr><td colspan="13" style="color:#8b949e;padding:10px">Waiting for incoming CAN frames…</td></tr></tbody>
    </table>
  </div>
</div>

<!-- ══ FULL WIDTH: Signal Explorer ══ -->
<div class="card full" id="signalExplorer">
  <div class="card-head">
    <span class="card-title">Signal Explorer — All ECU Signals</span>
    <span class="card-sub" id="sigCnt">0 signals</span>
  </div>
  <div style="max-height:380px;overflow-y:auto" id="sigExp"><p style="color:#8b949e;padding:8px">Waiting for PROFILE/SIG data…</p></div>
</div>

<!-- ══ FULL WIDTH: Controllers & bus health ══ -->
<div class="card full" id="controllers">
  <div class="card-head">
    <span class="card-title">Controllers &amp; Bus Health</span>
    <span class="card-sub" id="busSum">—</span>
  </div>
  <div style="display:grid;grid-template-columns:repeat(auto-fit,minmax(130px,1fr));gap:8px;font-size:.72rem;margin-bottom:8px">
    <div>Broadcast IDs<br><b id="bhIds">0</b></div>
    <div>Frames/s<br><b id="bhFps">0</b></div>
    <div>Bus load (est.)<br><b id="bhLoad">—</b></div>
    <div>CAN state<br><b id="bhState">—</b></div>
    <div>TEC / REC<br><b id="bhTecRec">—</b></div>
    <div>Bus errors / LEC<br><b id="bhErr">—</b></div>
    <div>Dropped (queue/UART)<br><b id="bhDrop">—</b></div>
  </div>
  <div style="font-size:.65rem;color:#8b949e;margin-bottom:6px">Broadcast frames do not carry the sender's address, so controllers are counted by the diagnostic scan below (needs ARM; each address gets one read-only request).</div>
  <div style="display:flex;gap:6px;flex-wrap:wrap;margin-bottom:6px">
    <button class="btn btn-sm" onclick="scanEcu('obd')">Scan OBD 11-bit (7E0-7E7, 01 00)</button>
    <button class="btn btn-sm" onclick="scanEcu('uds')">Scan UDS 11-bit (7E0-7E7, 3E 00)</button>
    <button class="btn btn-sm" onclick="scanEcu('x29')">Scan 29-bit (18DAxxF1, 3E 00)</button>
    <button class="btn btn-sm" onclick="wsSend({op:'ecuscan_abort'})">Stop scan</button>
    <button class="btn btn-sm" onclick="identifyAll()">Identify all</button>
  </div>
  <div class="can-wrap" style="max-height:260px;overflow:auto">
    <table><thead><tr><th>Request</th><th>Response</th><th>First reply</th><th>Identity</th><th></th></tr></thead>
    <tbody id="ecuRows"><tr><td colspan="5" style="color:#8b949e">No scan yet.</td></tr></tbody></table>
  </div>
</div>

<!-- ══ FULL WIDTH: Diagnostics (DTC / reset / live PIDs) ══ -->
<div class="card full" id="diagnostics">
  <div class="card-head"><span class="card-title">Fault Codes &amp; Live Sensors</span><span class="card-sub" id="dtcSum">—</span></div>
  <div style="font-size:.65rem;color:#8b949e;margin-bottom:6px">Needs ARM. Reads go to every controller found by the scan (or the engine ECU at 7E0 if no scan was run).</div>
  <div style="display:flex;gap:6px;flex-wrap:wrap;margin-bottom:6px">
    <button class="btn btn-sm" onclick="readDtcs()">Read all DTCs</button>
    <button class="btn btn-sm" onclick="clearDtcs()">Clear all DTCs</button>
    <select id="rstEcu" style="min-width:160px"></select>
    <select id="rstType"><option value="01">Hard reset (01)</option><option value="03">Soft reset (03)</option><option value="02">Key off/on (02)</option></select>
    <button class="btn btn-sm" onclick="ecuReset()">ECU reset</button>
  </div>
  <div style="display:flex;gap:6px;flex-wrap:wrap;align-items:end;margin-bottom:6px">
    <div class="fld"><label>Controller</label><select id="wrEcu" style="min-width:160px"></select></div>
    <div class="fld"><label>DID / routine (hex, 4)</label><input type="text" id="wrDid" maxlength="4" placeholder="F190" style="width:80px"></div>
    <div class="fld"><label>Data (hex)</label><input type="text" id="wrData" maxlength="96" placeholder="" style="width:200px"></div>
    <button class="btn btn-sm" onclick="didOp('read')">Read value (22)</button>
    <button class="btn btn-sm" onclick="didOp('write')">Write value (2E)</button>
    <button class="btn btn-sm" onclick="didOp('io')">Override output (2F 03)</button>
    <button class="btn btn-sm" onclick="didOp('ioret')">Return control (2F 00)</button>
    <button class="btn btn-sm" onclick="didOp('routine')">Start routine (31 01)</button>
    <button class="btn btn-sm" onclick="didOp('session')">Extended session (10 03)</button>
  </div>
  <div id="wrOut" style="font-size:.7rem;font-family:monospace;color:#c9d1d9;margin-bottom:6px"></div>
  <div class="can-wrap" style="max-height:240px;overflow:auto"><table><thead><tr><th>Controller</th><th>DTC</th><th>Source</th><th>Status</th></tr></thead>
    <tbody id="dtcRows"><tr><td colspan="4" style="color:#8b949e">Not read yet.</td></tr></tbody></table></div>
</div>

<!-- ══ FULL WIDTH: Operating Mode ══ -->
<div class="card full" id="operatingMode">
  <div class="card-head"><span class="card-title">Operating Mode</span></div>
  <div id="pnlR"><p style="font-size:.75rem;color:#8b949e">Read telemetry stays active and visible in both display modes. Switching the dashboard palette does not stop engine data, CAN capture, alerts, or event recording.</p></div>
  <div class="card" style="margin-top:8px">
    <div class="card-h">Dual-bus &amp; advanced control</div>
    <div style="padding:8px;display:flex;flex-wrap:wrap;gap:12px;align-items:flex-end">
      <div class="fld"><label>Active bus (all write commands)</label>
        <select id="busSel" onchange="setBus(this.value)"><option value="F">F-CAN (powertrain)</option><option value="B">B-CAN (body)</option></select></div>
      <div class="fld"><label>Controller mode</label>
        <select id="modeSel"><option>SILENT</option><option>NORMAL</option><option>LOOPBACK</option><option>LOOPBACKSILENT</option></select>
        <button class="btn btn-sm" onclick="wsSend({op:'mode',mode:document.getElementById('modeSel').value})">Set</button></div>
      <div class="fld"><label>Bitrate (AUTO or Hz)</label>
        <input id="brVal" value="AUTO" style="width:90px"><button class="btn btn-sm" onclick="wsSend({op:'bitrate',value:document.getElementById('brVal').value.trim().toUpperCase()})">Set</button></div>
      <div class="fld"><label>Gateway F&harr;B</label>
        <button class="btn btn-sm" onclick="wsSend({op:'gateway',on:true})">On</button>
        <button class="btn btn-sm" onclick="wsSend({op:'gateway',on:false})">Off</button></div>
      <span id="busState" class="card-sub">Active bus: F-CAN (powertrain)</span>
    </div>
    <div style="padding:8px;border-top:1px solid #30363d">
      <b style="font-size:.8rem">Remote engine start</b>
      <div style="display:flex;flex-wrap:wrap;gap:10px;align-items:flex-end;margin-top:6px">
        <div class="fld"><label>Auto-release timeout (s)</label><input id="rsTo" value="2" style="width:70px"></div>
        <label style="font-size:.75rem"><input type="checkbox" id="rsNeutral"> In Park/Neutral</label>
        <label style="font-size:.75rem"><input type="checkbox" id="rsBrake"> Brake pressed</label>
        <label style="font-size:.75rem"><input type="checkbox" id="rsImmob"> Immobilizer OK</label>
        <button class="btn btn-sm" onclick="doStartCfg()">Apply safety</button>
        <button class="btn" id="startBtn" disabled title="Turn the key to position II (ON) first" style="background:#1f6f3f;color:#fff;opacity:.35;cursor:not-allowed" onclick="doStart()">START engine</button>
        <button class="btn btn-sm" onclick="wsSend({op:'start_cancel'})">Cancel</button>
      </div>
      <p style="font-size:.7rem;color:#8b949e;margin:6px 0 0">The starter releases the instant the ECU reports the engine running (RPM over threshold), or after the timeout if it never catches. Requires the active bus ARMed; the interlocks you tick are your attestation that the car is safe to crank.</p>
    </div>
  </div>
  <div id="pnlW" style="display:none">
    <div class="card" style="margin-bottom:10px;border-color:#d97706">
      <div class="card-head"><span class="card-title">Decoded-value edit staging</span><span class="card-sub" id="editDraftCount">0 staged</span></div>
      <p style="font-size:.68rem;color:#8b949e;margin-bottom:6px">Write display mode mein Live Engine Sensors ki value ko double-tap karein. Mobile keyboard khulay ga. Send value ko local staging list mein rakhega; generic sensor measurements ke liye verified ECU write encoder/address available nahi, is liye is action se CAN transmission nahi hoti.</p>
      <div class="can-wrap" style="max-height:170px;overflow:auto"><table><thead><tr><th>Field</th><th>Observed</th><th>Proposed</th><th>Status</th></tr></thead><tbody id="editDraftRows"><tr><td colspan="4" style="color:#8b949e">No staged edits.</td></tr></tbody></table></div>
    </div>
    <div style="display:grid;grid-template-columns:160px 200px 1fr;gap:10px;align-items:end">
      <div class="fld"><label>Format</label><select id="txFmt" style="width:100%"><option value="S">Standard 11-bit</option><option value="E">Extended 29-bit</option></select></div>
      <div class="fld"><label>CAN ID (hex)</label><input type="text" id="txId" placeholder="7DF" maxlength="8" style="width:100%"></div>
      <div class="fld" style="display:flex;align-items:center;gap:10px;padding-bottom:1px">
        <div><label>DLC</label><input type="text" id="txDlc" placeholder="8" maxlength="1" style="width:40px"></div>
        <div style="display:flex;align-items:center;gap:4px;margin-top:14px"><input type="checkbox" id="txRtr"><label for="txRtr" style="font-size:.72rem;color:#8b949e">RTR</label></div>
      </div>
    </div>
    <div class="fld"><label>Data Bytes</label>
      <div class="byte-row">
        <label>B0<input type="text" id="b0" maxlength="2" placeholder="00"></label>
        <label>B1<input type="text" id="b1" maxlength="2" placeholder="00"></label>
        <label>B2<input type="text" id="b2" maxlength="2" placeholder="00"></label>
        <label>B3<input type="text" id="b3" maxlength="2" placeholder="00"></label>
        <label>B4<input type="text" id="b4" maxlength="2" placeholder="00"></label>
        <label>B5<input type="text" id="b5" maxlength="2" placeholder="00"></label>
        <label>B6<input type="text" id="b6" maxlength="2" placeholder="00"></label>
        <label>B7<input type="text" id="b7" maxlength="2" placeholder="00"></label>
      </div>
    </div>
    <button class="btn" onclick="sendFrame()">Send Frame</button>
    <button class="btn btn-sm" onclick="loadDef()">Load Default</button>
    <div id="txStat"></div>
    <div style="margin-top:12px;border-top:1px solid #30363d;padding-top:10px">
      <div style="font-size:.75rem;font-weight:700;color:#e3b341;margin-bottom:6px">Transmit arming</div>
      <div style="display:flex;gap:8px;align-items:end;flex-wrap:wrap">
        <div class="fld"><label>Arm window (s, 1-600)</label><input type="text" id="armSec" value="60" maxlength="3" style="width:70px"></div>
        <button class="btn" onclick="doArm()">ARM</button>
        <button class="btn btn-sm" onclick="doDisarm()">DISARM</button>
        <span id="armState" style="font-size:.75rem;color:#8b949e">Disarmed (silent, receive-only)</span>
      </div>
    </div>
    <div style="margin-top:12px;border-top:1px solid #30363d;padding-top:10px">
      <div style="font-size:.75rem;font-weight:700;color:#e3b341;margin-bottom:6px">Diagnostics (ISO-TP / UDS / OBD-II) — needs ARM</div>
      <div style="display:grid;grid-template-columns:140px 120px 120px 1fr;gap:8px;align-items:end">
        <div class="fld"><label>Format</label><select id="udsFmt" style="width:100%"><option value="S">Standard 11-bit</option><option value="E">Extended 29-bit</option></select></div>
        <div class="fld"><label>Request ID</label><input type="text" id="udsTx" value="7DF" maxlength="8" style="width:100%"></div>
        <div class="fld"><label>Response ID</label><input type="text" id="udsRx" value="7E8" maxlength="8" style="width:100%"></div>
        <div class="fld"><label>Request bytes (hex)</label><input type="text" id="udsReq" placeholder="0902" maxlength="128" style="width:100%"></div>
      </div>
      <div style="margin-top:6px;display:flex;gap:6px;flex-wrap:wrap">
        <button class="btn" onclick="sendUds()">Send request</button>
        <button class="btn btn-sm" onclick="preset('03')">OBD read DTCs (03)</button>
        <button class="btn btn-sm" onclick="preset('0902')">OBD read VIN (09 02)</button>
        <button class="btn btn-sm" onclick="preset('04')">OBD clear DTCs (04)</button>
        <button class="btn btn-sm" onclick="preset('1902FF')">UDS read DTCs (19 02 FF)</button>
        <button class="btn btn-sm" onclick="preset('14FFFFFF')">UDS clear DTCs (14)</button>
        <button class="btn btn-sm" onclick="preset('1101')">UDS ECU reset (11 01)</button>
        <button class="btn btn-sm" onclick="preset('1003')">Extended session (10 03)</button>
        <button class="btn btn-sm" onclick="preset('3E00')">Tester present (3E 00)</button>
        <button class="btn btn-sm" onclick="wsSend({op:'uds_abort'})">Abort</button>
      </div>
      <div style="font-size:.65rem;color:#8b949e;margin-top:4px">Routine control (31 ..) and other services: type the bytes. ECU addresses and routine IDs are not published by Honda; find them on your car.</div>
      <pre id="udsLog" style="margin-top:6px;max-height:160px;overflow:auto;font-size:.68rem;background:#0d1117;border:1px solid #30363d;border-radius:4px;padding:6px;white-space:pre-wrap"></pre>
    </div>
  </div>
</div>

<div id="fieldEditor" class="edit-overlay" style="display:none" role="dialog" aria-modal="true" aria-labelledby="fieldEditorTitle">
  <div class="edit-dialog">
    <div class="card-head"><b id="fieldEditorTitle">Stage decoded value</b><button class="btn btn-sm" type="button" onclick="closeFieldEditor()">Cancel</button></div>
    <div id="fieldEditorMeta" style="font-size:.68rem;color:#8b949e;margin-bottom:7px"></div>
    <label for="fieldEditInput" style="font-size:.7rem;color:#8b949e">Proposed numeric value</label>
    <input id="fieldEditInput" type="text" inputmode="decimal" autocomplete="off" spellcheck="false" style="width:100%;padding:10px;margin:5px 0 9px;background:#111820;border:1px solid #8b5429;border-radius:6px;color:#fff0d8;font-size:1rem">
    <button class="btn" id="fieldEditorSend" type="button" onclick="submitFieldEdit()">Send to staging</button>
    <p style="font-size:.64rem;color:#e3b341;margin-top:7px">This stages a proposal only. It does not write a sensor, ECU register, actuator, or CAN signal.</p>
  </div>
</div>

<script>
/* ── state ── */
const pid$  = {};  // pid→[{ts,v}] max300
const prof$ = {};  // id→{dlc,rate,label}
const sig$  = {};  // "id:b"→{last,min,max,rate,label,hist:[]}
const frm$  = {};  // id→{n,rate,ts,bytes:[],prev:[],dlc,label}
const alts  = [];  // [{type,rule,val,time}]
const scat  = [];  // [{thr,rpm}] max500
const seen$ = {};  // pid→epoch
let   sock  = null;
const $     = id => document.getElementById(id);
const clamp = (v,lo,hi) => Math.max(lo,Math.min(hi,v));
const avg   = a  => a.length ? a.reduce((s,x)=>s+x,0)/a.length : 0;

/* ── metric card config ── */
const CARDS = {
  '0C':{el:'0C', unit:'rpm',  color:'#58a6ff', wdir:null,  wval:0,    wlbl:''},
  '05':{el:'05', unit:'°C',   color:'#3fb950', wdir:'>=',  wval:105,  wlbl:'Hot'},
  '2F':{el:'2F', unit:'%',    color:'#e3b341', wdir:'<=',  wval:30,   wlbl:'Low'},
  '42':{el:'42', unit:'V',    color:'#f85149', wdir:'<=',  wval:12.0, wlbl:'Alert'},
};

/* ── sparkline ── */
function drawSpark(svgId, samples, color){
  const el=$(svgId); if(!el||samples.length<2) return;
  const W=260,H=38,PAD=4;
  const mn=Math.min(...samples), mx=Math.max(...samples), rng=Math.max(mx-mn,0.001);
  const pts=samples.map((v,i)=>{
    const x=(i/(samples.length-1))*W;
    const y=H-PAD-((v-mn)/rng)*(H-PAD*2);
    return x.toFixed(1)+','+y.toFixed(1);
  }).join(' ');
  el.innerHTML='<polyline points="'+pts+'" fill="none" stroke="'+color+'" stroke-width="1.8"/>';
}

/* ── update metric card ── */
function updCard(pid, val){
  const c=CARDS[pid]; if(!c) return;
  const smp=(pid$[pid]||[]).map(x=>x.v);
  /* value */
  const d=pid==='42'?val.toFixed(2):pid==='0C'?Math.round(val).toString():val.toFixed(1);
  $('v'+c.el).innerHTML=d+'<small> '+c.unit+'</small>';
  /* badge */
  const bEl=$('b'+c.el);
  let cls='badge ', txt='';
  if(c.wdir===null){ cls+='badge-ok'; txt='LIVE'; }
  else {
    const trig=(c.wdir==='>='&&val>=c.wval)||(c.wdir==='<='&&val<=c.wval);
    if(trig){ cls+=(c.wdir==='>='?'badge-warn':'badge-alrt'); txt=c.wlbl; }
    else     { cls+='badge-ok'; txt='OK'; }
  }
  bEl.className=cls; bEl.textContent=txt;
  /* sub-label */
  const sEl=$('s'+c.el);
  if(pid==='0C'){
    sEl.textContent='Avg '+Math.round(avg(smp))+' rpm';
  } else if(pid==='05'){
    const mn=smp.length?Math.min(...smp).toFixed(0):'—';
    const mx=smp.length?Math.max(...smp).toFixed(0):'—';
    sEl.textContent='Range '+mn+'–'+mx+' °C';
  } else if(pid==='2F'){
    const freq={};
    smp.forEach(v=>{const k=Math.round(v);freq[k]=(freq[k]||0)+1;});
    let mV='—',mC=0;
    for(const[k,c2] of Object.entries(freq)) if(c2>mC){mC=c2;mV=k;}
    sEl.textContent='Most common '+(mV!=='—'?mV+'% ('+mC+'x)':'—');
  } else if(pid==='42'){
    sEl.textContent='Avg '+avg(smp).toFixed(2)+' V';
  }
  /* sparkline */
  drawSpark('sk'+c.el, smp, c.color);
}

/* ── data quality ── */
const DQ=[
  {pid:'0C',name:'RPM'},{pid:'05',name:'Coolant'},{pid:'0F',name:'Intake'},
  {pid:'46',name:'Ambient'},{pid:'2F',name:'Fuel'},{pid:'42',name:'Voltage'},
  {pid:'10',name:'MAF'},{pid:'04',name:'Load'},
];
function updQuality(){
  const now=Date.now();
  $('dqRows').innerHTML=DQ.map(({pid,name})=>{
    const ls=seen$[pid];
    const miss=ls?Math.min(100,Math.floor((now-ls)/1000)):100;
    const pct=100-miss;
    const col=miss<30?'#56d364':miss<70?'#e3b341':'#f85149';
    return `<div class="dq-row">
      <span class="dq-name">${name}</span>
      <div class="dq-bar-wrap"><div class="dq-bar" style="width:${pct}%;background:${col}"></div></div>
      <span class="dq-pct" style="color:${col}">${miss}%</span>
    </div>`;
  }).join('');
}

/* ── alerts ── */
const ALT_RULES=[
  {pid:'05',rule:'Coolant Temp High', unit:'°C', chk:v=>v>=105, type:'warn'},
  {pid:'2F',rule:'Fuel Level Low',    unit:'%',  chk:v=>v<=30,  type:'warn'},
  {pid:'42',rule:'Battery Voltage Low',unit:'V', chk:v=>v<=12.0,type:'crit'},
];
function chkAlerts(pid,val){
  for(const r of ALT_RULES){
    if(r.pid!==pid) continue;
    if(r.chk(val)){
      const t=new Date().toTimeString().slice(0,8);
      const v=r.unit==='V'?val.toFixed(2):Math.round(val);
      if(!alts.length||alts[0].rule!==r.rule){
        alts.unshift({type:r.type,rule:r.rule,val:v+' '+r.unit,time:t});
        if(alts.length>20) alts.pop();
      }
    }
  }
}
function updAlerts(){
  $('alCount').textContent=alts.length+' active';
  if(!alts.length){
    $('alBody').innerHTML='<tr><td colspan="4" style="color:#8b949e;padding:8px">No alerts.</td></tr>';
    return;
  }
  $('alBody').innerHTML=alts.map(a=>{
    const ic=a.type==='crit'?'<span style="color:#f85149">&#9679;</span>':'<span style="color:#e3b341">&#9651;</span>';
    return `<tr class="${a.type==='crit'?'al-crit':'al-warn'}"><td>${ic}</td><td>${a.rule}</td><td style="font-family:monospace">${a.val}</td><td style="color:#8b949e">${a.time}</td></tr>`;
  }).join('');
}

/* ── RPM chart ── */
function drawRpm(){
  const c=$('cRpm'); if(!c) return;
  const dpr=window.devicePixelRatio||1, W=c.clientWidth||600, H=200;
  c.width=W*dpr; c.height=H*dpr;
  const ctx=c.getContext('2d'); ctx.scale(dpr,dpr);
  const L=38,R=8,T=12,B=22;
  ctx.fillStyle='#0d1117'; ctx.fillRect(0,0,W,H);
  const pts=(pid$['0C']||[]).map(x=>x.v);
  const YMAX=6000;
  /* grid */
  ctx.strokeStyle='#21262d'; ctx.lineWidth=1;
  ctx.fillStyle='#6e7681'; ctx.font='9px system-ui'; ctx.textAlign='right';
  [0,1000,2000,3000,4000,5000,6000].forEach(y=>{
    const py=T+(YMAX-y)/YMAX*(H-T-B);
    ctx.beginPath(); ctx.moveTo(L,py); ctx.lineTo(W-R,py); ctx.stroke();
    if(y<6000) ctx.fillText(y,L-3,py+3);
  });
  /* x labels */
  ctx.textAlign='center'; ctx.fillStyle='#6e7681';
  [50,100,150,200,250,300].forEach(n=>{
    if(n>pts.length) return;
    const px=L+(n/300)*(W-L-R);
    ctx.beginPath(); ctx.moveTo(px,H-B); ctx.lineTo(px,H-B+3); ctx.strokeStyle='#30363d'; ctx.stroke();
    ctx.fillText(n,px,H-6);
  });
  /* line */
  if(pts.length>=2){
    ctx.strokeStyle='#58a6ff'; ctx.lineWidth=1.6; ctx.lineJoin='round';
    ctx.beginPath();
    pts.forEach((v,i)=>{
      const px=L+(i/Math.max(pts.length-1,1))*(W-L-R);
      const py=T+(YMAX-clamp(v,0,YMAX))/YMAX*(H-T-B);
      i?ctx.lineTo(px,py):ctx.moveTo(px,py);
    });
    ctx.stroke();
  } else {
    ctx.fillStyle='#30363d'; ctx.textAlign='center'; ctx.font='12px system-ui';
    ctx.fillText('Waiting for RPM…',W/2,H/2);
  }
  $('rpmSubLbl').textContent=pts.length+' samples';
}

/* ── Temperature chart ── */
function drawTemp(){
  const c=$('cTemp'); if(!c) return;
  const dpr=window.devicePixelRatio||1, W=c.clientWidth||600, H=130;
  c.width=W*dpr; c.height=H*dpr;
  const ctx=c.getContext('2d'); ctx.scale(dpr,dpr);
  const L=32,R=8,T=8,B=18,YMIN=50,YMAX=150;
  ctx.fillStyle='#0d1117'; ctx.fillRect(0,0,W,H);
  ctx.strokeStyle='#21262d'; ctx.lineWidth=1;
  ctx.fillStyle='#6e7681'; ctx.font='9px system-ui'; ctx.textAlign='right';
  [50,75,100,125,150].forEach(y=>{
    const py=T+(YMAX-y)/(YMAX-YMIN)*(H-T-B);
    ctx.beginPath(); ctx.moveTo(L,py); ctx.lineTo(W-R,py); ctx.stroke();
    ctx.fillText(y,L-3,py+3);
  });
  [
    {pid:'05',col:'#f85149'},
    {pid:'0F',col:'#e3b341'},
    {pid:'46',col:'#56d364'},
  ].forEach(({pid,col})=>{
    const d=(pid$[pid]||[]).map(x=>x.v);
    if(d.length<2) return;
    ctx.strokeStyle=col; ctx.lineWidth=1.6; ctx.lineJoin='round';
    ctx.beginPath();
    d.forEach((v,i)=>{
      const px=L+(i/Math.max(d.length-1,1))*(W-L-R);
      const py=T+(YMAX-clamp(v,YMIN,YMAX))/(YMAX-YMIN)*(H-T-B);
      i?ctx.lineTo(px,py):ctx.moveTo(px,py);
    });
    ctx.stroke();
  });
}

/* ── Scatter + histogram ── */
function drawScat(){
  /* scatter */
  const cs=$('cScat'); if(!cs) return;
  const dpr=window.devicePixelRatio||1;
  {
    const W=cs.clientWidth||220, H=200;
    cs.width=W*dpr; cs.height=H*dpr;
    const ctx=cs.getContext('2d'); ctx.scale(dpr,dpr);
    const L=30,R=4,T=6,B=18;
    ctx.fillStyle='#0d1117'; ctx.fillRect(0,0,W,H);
    ctx.strokeStyle='#21262d'; ctx.lineWidth=1;
    ctx.fillStyle='#6e7681'; ctx.font='8px system-ui';
    [0,1000,2000,3000,4000].forEach(y=>{
      const py=T+(4000-y)/4000*(H-T-B);
      ctx.beginPath(); ctx.moveTo(L,py); ctx.lineTo(W-R,py); ctx.stroke();
      ctx.textAlign='right'; ctx.fillText(y||'0',L-2,py+3);
    });
    [0,25,50,75,100].forEach(x=>{
      const px=L+x/100*(W-L-R);
      ctx.beginPath(); ctx.moveTo(px,T); ctx.lineTo(px,H-B); ctx.stroke();
      ctx.textAlign='center'; ctx.fillText(x,px,H-4);
    });
    ctx.fillStyle='rgba(88,166,255,0.6)';
    scat.forEach(p=>{
      const px=L+clamp(p.thr,0,100)/100*(W-L-R);
      const py=T+(4000-clamp(p.rpm,0,4000))/4000*(H-T-B);
      ctx.beginPath(); ctx.arc(px,py,2.5,0,Math.PI*2); ctx.fill();
    });
  }
  /* histogram */
  const ch=$('cHist'); if(!ch) return;
  {
    const W=ch.clientWidth||80, H=200;
    ch.width=W*dpr; ch.height=H*dpr;
    const ctx=ch.getContext('2d'); ctx.scale(dpr,dpr);
    const L=4,R=4,T=6,B=18;
    ctx.fillStyle='#0d1117'; ctx.fillRect(0,0,W,H);
    const bkt=new Array(10).fill(0);
    scat.forEach(p=>{ bkt[clamp(Math.floor(p.thr/10),0,9)]++; });
    const mx=Math.max(...bkt,1);
    const bw=(W-L-R)/10;
    bkt.forEach((n,i)=>{
      const bh=(n/mx)*(H-T-B);
      const x=L+i*bw;
      ctx.fillStyle='#1c3a5e'; ctx.fillRect(x+1,T+H-T-B-bh,bw-2,bh);
      ctx.fillStyle='#58a6ff'; ctx.fillRect(x+1,T+H-T-B-bh,bw-2,2);
    });
    ctx.fillStyle='#6e7681'; ctx.font='7px system-ui'; ctx.textAlign='center';
    bkt.forEach((_,i)=>ctx.fillText(i*10+'%',L+i*bw+bw/2,H-4));
  }
}

/* ── CAN Analyzer ── */
function updCAN(){
  const ids=Object.keys(frm$).sort((a,b)=>(frm$[b].rate||0)-(frm$[a].rate||0)).slice(0,60);
  if(!ids.length){
    $('canBody').innerHTML='<tr><td colspan="13" style="color:#8b949e;padding:8px">Waiting…</td></tr>';
    $('canBadge').textContent='WAITING FOR FRAMES'; $('canBadge').className='live-badge off';
    return;
  }
  $('canBadge').textContent='LIVE · '+ids.length+' IDs'; $('canBadge').className='live-badge';
  $('canBody').innerHTML=ids.map(id=>{
    const s=frm$[id];
    const t=new Date(s.ts).toTimeString().slice(0,8);
    const bCells=Array.from({length:8},(_,i)=>{
      const b=s.bytes[i]||'--', p=s.prev[i];
      const col=b!=='--'&&p!==undefined&&p!==b?'#f85149':(b!=='--'?'#56d364':'#30363d');
      return `<td style="color:${col}">${b}</td>`;
    }).join('');
    return `<tr><td>${id}</td><td>${s.dlc||'—'}</td>${bCells}<td>${s.rate?s.rate.toFixed(1):'—'} Hz</td><td>${s.n}</td><td style="color:#8b949e">${t}</td></tr>`;
  }).join('');
}

/* ── Signal Explorer ── */
function tsSVG(hist){
  if(!hist||hist.length<2) return '<svg width="56" height="18"></svg>';
  const W=56,H=18, mn=Math.min(...hist),mx=Math.max(...hist),rng=Math.max(mx-mn,1);
  const pts=hist.map((v,i)=>{
    const x=(i/(hist.length-1))*W, y=H-2-((v-mn)/rng)*(H-4);
    return x.toFixed(1)+','+y.toFixed(1);
  }).join(' ');
  return `<svg width="${W}" height="${H}" viewBox="0 0 ${W} ${H}"><polyline points="${pts}" fill="none" stroke="#58a6ff" stroke-width="1.2"/></svg>`;
}
let selSig=null;
function updSigs(){
  const keys=Object.keys(sig$);
  $('sigCnt').textContent=keys.length+' signals';
  if(!keys.length){$('sigExp').innerHTML='<p style="color:#8b949e;padding:8px">Waiting for PROFILE/SIG data…</p>';return;}
  const byId={};
  keys.forEach(k=>{const id=k.split(':')[0];(byId[id]=byId[id]||[]).push(k);});
  $('sigExp').innerHTML=Object.keys(byId).sort().map(id=>{
    const lbl=prof$[id]?prof$[id].label:'—';
    const rows=byId[id].sort((a,b)=>+a.split(':')[1]-+b.split(':')[1]).map(k=>{
      const s=sig$[k], sel=selSig===k;
      return `<tr class="sig-row${sel?' sel':''}" onclick="selSig='${k}';updSigs()">
        <td style="font-family:monospace">${id}</td><td style="color:#79c0ff">${lbl}</td>
        <td>${s.byte}</td><td>${s.label||'—'}</td>
        <td style="font-family:monospace">${s.last}</td>
        <td style="font-family:monospace;color:#6e7681">${s.min}</td>
        <td style="font-family:monospace;color:#6e7681">${s.max}</td>
        <td>${s.rate?s.rate.toFixed(1):'—'}</td>
        <td>${tsSVG(s.hist)}</td></tr>`;
    }).join('');
    return `<div class="sig-group"><div class="sig-group-hdr">0x${id} — ${lbl}</div>
      <table><thead><tr><th>ID</th><th>Frame</th><th>Byte</th><th>Signal</th><th>Last</th><th>Min</th><th>Max</th><th>Rate</th><th>Trend</th></tr></thead>
      <tbody>${rows}</tbody></table></div>`;
  }).join('');
}

/* ── mode switch ── */
let dashboardWriteMode=false;
function setMode(m){
  const w=m==='w';
  dashboardWriteMode=w;
  document.body.classList.toggle('write-mode',w);
  $('pnlW').style.display=w?'block':'none';
  $('pnlR').style.display='block'; /* never stop or hide the live read side */
  const toggle=$('globalModeToggle');
  if(toggle) toggle.textContent=w?'EXIT WRITE MODE':'ENTER WRITE MODE';
}
function toggleGlobalMode(){ setMode(dashboardWriteMode?'r':'w'); }

/* Live measurements have no reverse encoding. Double-tap opens a local draft
 * editor only; it never calls wsSend(), the UDS queue, or the CAN TX path. */
let editingField=null;
const stagedFieldEdits=[];
function openFieldEditor(node){
  if(!dashboardWriteMode || !node || !node.dataset.fieldId) return;
  const value=node.dataset.value||'';
  if(!/^[+-]?(?:\d+(?:\.\d*)?|\.\d+)$/.test(value)) return;
  editingField={id:node.dataset.fieldId,label:node.dataset.fieldLabel||node.dataset.fieldId,observed:value};
  $('fieldEditorTitle').textContent='Stage '+editingField.label;
  $('fieldEditorMeta').textContent=editingField.id+' · current observed value: '+value;
  $('fieldEditInput').value=value;
  $('fieldEditor').style.display='flex';
  setTimeout(()=>{$('fieldEditInput').focus();$('fieldEditInput').select();},40);
}
function closeFieldEditor(){ $('fieldEditor').style.display='none'; editingField=null; }
function drawEditDrafts(){
  $('editDraftRows').innerHTML=stagedFieldEdits.length?stagedFieldEdits.slice().reverse().map(e=>`<tr><td>${esc(e.label)}<br><small>${esc(e.id)}</small></td><td>${esc(e.observed)}</td><td><b>${esc(e.proposed)}</b></td><td style="color:#e3b341">STAGED · NOT SENT</td></tr>`).join(''):'<tr><td colspan="4" style="color:#8b949e">No staged edits.</td></tr>';
  $('editDraftCount').textContent=stagedFieldEdits.length+' staged';
}
function submitFieldEdit(){
  if(!editingField) return;
  const raw=$('fieldEditInput').value.trim();
  if(raw.length>24 || !/^[+-]?(?:\d+(?:\.\d*)?|\.\d+)$/.test(raw) || !Number.isFinite(Number(raw))){
    $('fieldEditorMeta').textContent='Invalid numeric value. Use digits and an optional decimal point.'; return;
  }
  stagedFieldEdits.push({...editingField,proposed:String(Number(raw)),stagedAt:new Date().toISOString()});
  if(stagedFieldEdits.length>40) stagedFieldEdits.shift();
  closeFieldEditor(); drawEditDrafts();
  $('wrOut').textContent='Staged '+editingFieldSafeLabel(stagedFieldEdits[stagedFieldEdits.length-1])+' locally. Not transmitted to vehicle.';
}
function editingFieldSafeLabel(e){ return e.label+' ('+e.id+')'; }
let lastFieldTapAt=0,lastFieldTapId='';
document.addEventListener('pointerup',ev=>{
  if(!dashboardWriteMode || ev.pointerType!=='touch') return;
  const node=ev.target.closest('[data-field-id]'); if(!node) return;
  const now=Date.now(), id=node.dataset.fieldId||'';
  if(id && id===lastFieldTapId && now-lastFieldTapAt<420){ lastFieldTapAt=0;lastFieldTapId='';openFieldEditor(node); }
  else { lastFieldTapAt=now;lastFieldTapId=id; }
},true);
document.addEventListener('dblclick',ev=>{
  if(!dashboardWriteMode || $('fieldEditor').style.display==='flex') return;
  const node=ev.target.closest('[data-field-id]'); if(node){ev.preventDefault();openFieldEditor(node);}
},true);

/* ── send frame ── */
function sendFrame(){
  if(!sock||sock.readyState!==1){$('txStat').textContent='Not connected.';return;}
  const fmt=$('txFmt').value, id=$('txId').value.trim().toUpperCase();
  const bytes=['b0','b1','b2','b3','b4','b5','b6','b7'].map(i=>($( i).value||'00').trim().toUpperCase().padStart(2,'0'));
  const rtr=$('txRtr').checked?1:0;
  let dlc=parseInt($('txDlc').value)||0;
  if(!dlc){dlc=8;for(let i=7;i>=0;i--){if(bytes[i]!=='00'){dlc=i+1;break;}}if(!dlc)dlc=1;}
  dlc=clamp(dlc,1,8);
  const maxId=fmt==='S'?0x7FF:0x1FFFFFFF;
  if(!/^[0-9A-F]{1,8}$/.test(id)||parseInt(id,16)>maxId){$('txStat').textContent='Invalid ID.';return;}
  const data=bytes.slice(0,dlc).join('');
  if(!wsSend({op:'tx',format:fmt,id,rtr,dlc,data})) return;
  $('txStat').textContent='Sent → TX,'+fmt+','+id+','+rtr+','+dlc+','+data;
}
function loadDef(){
  const id=$('txId').value.trim().toUpperCase();
  if(!id){$('txStat').textContent='Enter a CAN ID first.';return;}
  for(let i=0;i<8;i++){const k=id+':'+i;if(sig$[k])$('b'+i).value=sig$[k].last||'00';}
  const p=prof$[id]; if(p&&p.dlc) $('txDlc').value=p.dlc;
  $('txStat').textContent='Loaded defaults for '+id;
}

/* ── command channel (no web login: Wi-Fi WPA2 is the gate) ── */
let authOk=false, armedLeft=0, armedBus='', udsBuf='', curBus='F';
function setBus(b){ curBus=(b==='B')?'B':'F'; const e=$('busState'); if(e) e.textContent='Active bus: '+(curBus==='F'?'F-CAN (powertrain)':'B-CAN (body)'); }
function wsSend(o){
  if(!sock||sock.readyState!==1){ diagLog('Not connected.'); return false; }
  if(o && o.bus===undefined) o.bus=curBus;   /* default every command to the selected bus */
  sock.send(JSON.stringify(o)); return true;
}
function diagLog(t){
  const el=$('udsLog'); el.textContent=(new Date().toLocaleTimeString()+'  '+t+'\n'+el.textContent).slice(0,6000);
}
function showArm(){
  $('armState').textContent=armedLeft>0?('ARMED — '+armedLeft+' s left (CAN controller active)'):'Disarmed (silent, receive-only)';
  $('armState').style.color=armedLeft>0?'#f85149':'#8b949e';
}
setInterval(()=>{ if(armedLeft>0){armedLeft--; if(armedLeft===0) armedBus=''; showArm(); syncAutoPids();} },1000);
function doArm(){
  const s=parseInt($('armSec').value,10);
  if(!(s>=1&&s<=600)){ diagLog('Arm window must be 1..600 s.'); return; }
  if(!confirm('ARM transmit for '+s+' s?\nThe STM32 leaves silent mode: it will ACK frames and can send on the vehicle bus.')) return;
  wsSend({op:'arm',seconds:s});
}
function doDisarm(){ wsSend({op:'disarm'}); }
const READ_SIDS=['01','02','03','06','07','09','0A','19','22','3E'];
const SID_NAMES={'04':'OBD-II clear DTCs and stored values','11':'ECU reset','14':'clear diagnostic information (DTCs)',
  '27':'security access','28':'communication control','2E':'write data by identifier','2F':'input/output control',
  '31':'routine control','34':'request download','3D':'write memory','85':'control DTC setting','10':'diagnostic session change'};
function needsConfirm(req){
  const sid=req.slice(0,2);
  if(READ_SIDS.includes(sid)) return false;
  if(sid==='10'){ const sub=parseInt(req.slice(2,4)||'0',16)&0x7F; return !(sub===1||sub===3); }
  return true;
}
function preset(r){ $('udsReq').value=r; }
function sendUds(){
  const fmt=$('udsFmt').value, tx=$('udsTx').value.trim().toUpperCase(), rx=$('udsRx').value.trim().toUpperCase();
  const req=$('udsReq').value.replace(/\s+/g,'').toUpperCase();
  const max=fmt==='S'?0x7FF:0x1FFFFFFF;
  if(!/^[0-9A-F]{1,8}$/.test(tx)||parseInt(tx,16)>max||!/^[0-9A-F]{1,8}$/.test(rx)||parseInt(rx,16)>max){ diagLog('Invalid request/response ID.'); return; }
  if(!/^([0-9A-F]{2}){1,64}$/.test(req)){ diagLog('Request must be 1..64 hex bytes.'); return; }
  let conf=false;
  if(needsConfirm(req)){
    const sid=req.slice(0,2);
    if(!confirm('This request can change the ECU: '+(SID_NAMES[sid]||('service 0x'+sid))+'.\nSend '+req+' to '+tx+'?')) return;
    conf=true;
  }
  udsBuf='';
  if(wsSend({op:'uds',format:fmt,tx,rx,req,confirm:conf})) diagLog('→ '+tx+' '+req+(conf?' (confirmed)':''));
}
function onDiag(t){
  const f=t.split(',');
  if(t.startsWith('ECU,')){ onEcu(t); return; }

  /* STM32 prefixes per-bus diagnostic lines as UDS,F,<event>,... / UDS,B,...
     (older captures without the bus field are accepted too). */
  if(f[0]==='UDS'){
    const tagged=(f[1]==='F'||f[1]==='B');
    const ei=tagged?2:1, ev=f[ei]||'';
    if(ev==='DATA'){
      udsBuf+=(f[ei+3]||'');
      return;
    }
    if(ev==='POS'){
      if(idCur){ const h=udsBuf; udsBuf=''; idDone(h); }
      else { diagLog('← '+(f[ei+1]||'ECU')+' positive, '+(f[ei+2]||'0')+' bytes: '+udsBuf); udsBuf=''; }
      return;
    }
    if(ev==='NEG'){
      const nrc=f[ei+3]||'??';
      if(idCur) idDone(null,'NRC 0x'+nrc);
      else diagLog('← '+(f[ei+1]||'ECU')+' negative: service 0x'+(f[ei+2]||'??')+' NRC 0x'+nrc);
      return;
    }
    if(ev==='SENT'){ if(idCur) idDone('00','SENT'); else diagLog(t); return; }
    if(ev==='ERR'||ev==='TXERR'){
      const why=f.slice(ei+1).join(',')||ev;
      if(idCur) idDone(null,why); else diagLog(t);
      return;
    }
    if(ev==='PENDING') return;
  }

  /* Per-bus ARM/DISARM acknowledgements are STATUS,F|B,ARMED|DISARMED,... */
  if(f[0]==='STATUS' && (f[1]==='F'||f[1]==='B')){
    if(f[2]==='ARMED'){ armedBus=f[1]; armedLeft=parseInt(f[3],10)||0; showArm(); syncAutoPids(); diagLog(t); return; }
    if(f[2]==='DISARMED'){ if(armedBus===f[1]){armedLeft=0;armedBus='';} showArm(); syncAutoPids(); diagLog(t); return; }
  }
  if(t.startsWith('STATUS,ARMED,')){ armedBus='F'; armedLeft=parseInt(f[2],10)||0; showArm(); syncAutoPids(); diagLog(t); return; }
  if(t.startsWith('STATUS,DISARMED,')){ armedLeft=0; armedBus=''; showArm(); syncAutoPids(); diagLog(t); return; }
  diagLog(t);
}

/* ── controllers & bus health ── */
let lastBitrate=0;
const ecus={};          // rx id → {tx,rx,fmt,reply,info:{}}
let idq=[], idCur=null, idTimer=null;
const esc=v=>String(v).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
let busFps=[0,0], busBps=[0,0], bcPedal=null, veh={state:'OFF'};
/* Bus health for the selected bus only. Rates/bits are counted on the ESP32
   from every received frame (not from the profiler), so load is real. */
function updBus(){
  const b=curBus==='B'?1:0;
  $('bhIds').textContent=Object.values(frm$).filter(s=>(s.bus||0)===b).length;
  $('bhFps').textContent=busFps[b]||0;
  const br=b===0?lastBitrate:0;
  $('bhLoad').textContent=br?Math.min(100,100*(busBps[b]||0)/br).toFixed(1)+' %':'—';
  $('busSum').textContent=Object.keys(ecus).length+' diagnostic controller(s) found';
}
setInterval(updBus,1000);
function drawVeh(){
  const st=veh.state||'OFF', on=(st==='ON'||st==='RUN');
  const nm={OFF:'OFF / locked — parked',ACC:'ACC (key I)',ON:'ON (key II)',RUN:'ENGINE RUNNING'}[st]||st;
  const col={OFF:'#8b949e',ACC:'#e3b341',ON:'#58a6ff',RUN:'#3fb950'}[st]||'#8b949e';
  const src=veh.src==='OBD'?'OBD':veh.src==='BCAST'?'broadcast 0x17C/0x158 (unverified)':'—';
  $('vehBar').innerHTML='Key: <b style="color:'+col+'">'+nm+'</b> | RPM <b>'+(on?veh.rpm:'—')+'</b> | Speed <b>'+
    (on?Number(veh.spd||0).toFixed(1)+' km/h':'—')+'</b> | Pedal (0x17C B0) <b>'+(on&&bcPedal!==null?bcPedal:'—')+
    '</b> <small style="color:#8b949e">src: '+src+(veh.acc?' · ACC wire high':'')+'</small>';
  const b=$('startBtn'); if(!b) return;
  const ok=(st==='ON');
  b.disabled=!ok; b.style.opacity=ok?'1':'.35'; b.style.cursor=ok?'pointer':'not-allowed';
  b.title=ok?'':(st==='RUN'?'Engine already running':'Turn the key to position II (ON) first');
}
function onStats(t,bus){
  if((bus||'F')!==curBus) return;   // F and B stats used to overwrite each other
  const f=t.split(',');   // rx,qdrop,udrop,canerr,lec,tec,rec,state
  $('bhErr').textContent=(f[3]||'—')+' / '+(f[4]||'—');
  $('bhDrop').textContent=(f[1]||'—')+' / '+(f[2]||'—');
  if(f.length>=8){
    $('bhTecRec').textContent=f[5]+' / '+f[6];
    $('bhState').textContent=f[7];
    $('bhState').style.color=f[7]==='ACTIVE'?'#3fb950':(f[7]==='WARNING'?'#e3b341':'#f85149');
  }
}
function scanEcu(kind){
  const p={obd:['S','7E0','7E7','0100'],uds:['S','7E0','7E7','3E00'],x29:['E','00','FF','3E00']}[kind];
  if(wsSend({op:'ecuscan',format:p[0],first:p[1],last:p[2],req:p[3],wait:60})) diagLog('ECU scan '+p.join(' '));
}
function drawEcus(){
  const k=Object.keys(ecus);
  if(!k.length){ $('ecuRows').innerHTML='<tr><td colspan="5" style="color:#8b949e">No controller answered.</td></tr>'; return; }
  $('ecuRows').innerHTML=k.map(rx=>{ const e=ecus[rx];
    const info=Object.entries(e.info).map(([n,v])=>esc(n)+': '+esc(v)).join('<br>')||'—';
    return `<tr><td>${esc(e.tx)}</td><td>${esc(e.rx)}</td><td>${esc(e.reply)}</td><td>${info}</td>`+
      `<td><button class="btn btn-sm" onclick="identify('${esc(rx)}')">Identify</button> `+
      `<button class="btn btn-sm" onclick="useEcu('${esc(rx)}')">Use</button></td></tr>`; }).join('');
}
function useEcu(rx){ const e=ecus[rx]; if(!e) return; $('udsFmt').value=e.fmt; $('udsTx').value=e.tx; $('udsRx').value=e.rx; }
/* Identification DIDs (ISO 14229-1 Annex C names, as listed in python-udsoncan DataIdentifier) */
const ID_DIDS=[['F190','VIN'],['F187','Spare part no.'],['F18C','ECU serial'],['F191','HW no.'],['F195','SW version'],['F18A','Supplier']];
/* ── generic diagnostic request queue: one request at a time, answer → cb(hex|null, why) ── */
function diagReq(e,req,cb,confirmed,autoPid){ idq.push({e,req,cb,confirmed:!!confirmed,autoPid:!!autoPid}); idNext(); }
function idNext(){
  if(idCur||!idq.length) return;
  idCur=idq.shift(); udsBuf='';
  if(!wsSend({op:'uds',format:idCur.e.fmt,tx:idCur.e.tx,rx:idCur.e.rx,req:idCur.req,confirm:idCur.confirmed})){
    const q=[idCur,...idq]; idq=[]; idCur=null; q.forEach(j=>j.cb(null,'NOT_CONNECTED')); return;
  }
  idTimer=setTimeout(()=>idDone(null,'NO_ANSWER'),8000);
}
function idDone(hex,why){
  if(!idCur) return;
  clearTimeout(idTimer);
  const j=idCur; idCur=null;
  try{ j.cb(hex&&hex.length>=2?hex:null, why||''); }catch(err){ diagLog('UI error: '+err); }
  setTimeout(idNext,j.autoPid?200:20);
}
const hexBytes=h=>h.match(/../g).map(x=>parseInt(x,16));
function identify(rx){
  const e=ecus[rx]; if(!e) return;
  const put=(name,skip)=>(hex)=>{ if(!hex) return; const b=hexBytes(hex).slice(skip);
    const txt=b.filter(c=>c>=0x20&&c<0x7F).map(c=>String.fromCharCode(c)).join('').trim();
    e.info[name]=(txt.length>=Math.max(1,b.length/2))?txt:b.map(x=>x.toString(16).padStart(2,'0')).join(' ');
    drawEcus(); };
  ID_DIDS.forEach(([d,n])=>diagReq(e,'22'+d,put(n,3)));
  if(e.fmt==='S'){ diagReq(e,'0902',put('VIN (OBD)',3)); diagReq(e,'090A',put('ECU name (OBD)',3)); }
}
function identifyAll(){ Object.keys(ecus).forEach(identify); }

/* ── DTCs ── SAE J2012 2-byte code: bits 15-14 system (P/C/B/U), then 4 hex digits */
function dtcName(b0,b1){ return 'PCBU'[b0>>6]+((b0>>4)&3)+(b0&15).toString(16).toUpperCase()+b1.toString(16).toUpperCase().padStart(2,'0'); }
/* ISO 14229-1 DTC status bits */
const DTC_ST=[['testFailed',0],['pending',2],['confirmed',3],['MIL on',7]];
const dtcRows=[];   // {ecu,code,info,src}
function drawDtcs(){
  $('dtcRows').innerHTML=dtcRows.length?dtcRows.map(r=>`<tr><td>${esc(r.ecu)}</td><td><b>${esc(r.code)}</b></td><td>${esc(r.src)}</td><td>${esc(r.info)}</td></tr>`).join('')
    :'<tr><td colspan="4" style="color:#8b949e">No DTCs reported.</td></tr>';
  $('dtcSum').textContent=dtcRows.length+' code(s)';
}
const eventRows=[];
function eventSnapshot(e){
  const rpm=e.rpm>0?e.rpm+' rpm':'RPM —', speed=Number.isFinite(Number(e.speed))?Number(e.speed).toFixed(1)+' km/h':'Speed —';
  const p=e.pids?(' · '+e.pids):'';
  const csv=e.snapshotReady?`<a href="/event.csv?id=${Number(e.id)||0}" target="_blank" rel="noopener">Download CSV</a>`:'<span style="color:#e3b341">Capturing / unavailable</span>';
  return `<details><summary>${esc(rpm+' / '+speed)}</summary><div style="white-space:normal;max-width:360px;overflow-wrap:anywhere;font-size:.65rem;color:#8b949e">State ${esc(e.vehicleState||'?')} · RPM source ${esc(e.rpmSource||'—')} · frames ${esc(e.frames||0)} · F/B FPS ${esc(e.fpsF||0)}/${esc(e.fpsB||0)}${p?'<br>'+esc(p):''}<br>${csv}</div></details>`;
}
function drawEvents(){
  const current=eventRows.filter(e=>!e.seen).sort((a,b)=>a.id-b.id);
  const history=eventRows.filter(e=>e.seen).sort((a,b)=>b.id-a.id);
  const row=(e,withAction)=>`<tr><td>${esc(e.timestamp||'—')}</td><td>${esc((e.bus||'F')+' / '+(e.ecu||'—'))}</td><td><b>${esc(e.code||'—')}</b><br><small>${esc(e.status||'')}</small></td><td>${esc(e.source||'—')} ×${esc(e.repeats||1)}</td><td>${eventSnapshot(e)}</td>${withAction?`<td><button class="btn btn-sm" onclick="markEventSeen(${Number(e.id)||0})">Mark seen</button></td>`:''}</tr>`;
  $('eventCurrent').innerHTML=current.length?current.map(e=>row(e,true)).join(''):'<tr><td colspan="6" style="color:#8b949e">No unacknowledged DTC events.</td></tr>';
  $('eventHistory').innerHTML=history.length?history.map(e=>row(e,false)).join(''):'<tr><td colspan="5" style="color:#8b949e">No history yet.</td></tr>';
  $('eventSummary').textContent=current.length+' current · '+history.length+' historical';
}
function markEventSeen(id){ if(id>0) wsSend({op:'event_seen',id}); }
function targets(){ const k=Object.keys(ecus); return k.length?k.map(r=>ecus[r]):[{tx:'7E0',rx:'7E8',fmt:'S',info:{},reply:''}]; }
function ecuLabel(e){ return e.tx+'→'+e.rx+(e.info['ECU name (OBD)']?' '+e.info['ECU name (OBD)']:''); }
function readDtcs(){
  dtcRows.length=0; drawDtcs();
  targets().forEach(e=>{
    /* OBD modes 03 stored, 07 pending, 0A permanent: reply 43/47/4A [count] then 2-byte codes */
    [['03','stored'],['07','pending'],['0A','permanent']].forEach(([m,src])=>diagReq(e,m,(hex)=>{
      if(!hex) return; const b=hexBytes(hex); let i=1;
      if((b.length-1)%2===1) i=2;           // CAN replies carry a count byte
      for(;i+1<b.length;i+=2){ if(b[i]||b[i+1]) dtcRows.push({ecu:ecuLabel(e),code:dtcName(b[i],b[i+1]),src:'OBD '+src,info:''}); }
      drawDtcs(); }));
    /* UDS 19 02 FF: 59 02 <availability mask> then 4-byte records DTC(3) + status */
    diagReq(e,'1902FF',(hex)=>{
      if(!hex) return; const b=hexBytes(hex);
      for(let i=3;i+3<b.length;i+=4){
        const st=b[i+3], flags=DTC_ST.filter(([,bit])=>st&(1<<bit)).map(([n])=>n).join(', ');
        dtcRows.push({ecu:ecuLabel(e),code:dtcName(b[i],b[i+1])+'-'+b[i+2].toString(16).toUpperCase().padStart(2,'0'),src:'UDS',info:'status 0x'+st.toString(16).padStart(2,'0')+(flags?' ('+flags+')':'')});
      }
      drawDtcs(); });
  });
}
function clearDtcs(){
  if(!confirm('Clear DTCs and freeze-frame data on every controller found?\nThis turns the MIL off until a fault is detected again.')) return;
  targets().forEach(e=>{
    diagReq(e,'04',(h,w)=>diagLog(e.tx+' OBD clear: '+(h?'done':w||'refused')),true);
    diagReq(e,'14FFFFFF',(h,w)=>diagLog(e.tx+' UDS clear: '+(h?'done':w||'refused')),true);
  });
}
function ecuReset(){
  const e=ecus[$('rstEcu').value]; if(!e){ diagLog('Pick a controller first (run a scan).'); return; }
  const t=$('rstType').value;
  if(!confirm('Send ECU reset 11 '+t+' to '+e.tx+'?\nThe controller restarts; do this with the engine off.')) return;
  diagReq(e,'11'+t,(h,w)=>diagLog(e.tx+' ECU reset: '+(h?'accepted':w||'refused')),true);
}

/* ── live OBD-II PIDs (SAE J1979 mode 01 formulas) ── */
const PIDS={0x04:['Engine load','%',b=>b[0]*100/255],0x05:['Coolant temp','°C',b=>b[0]-40],
 0x06:['ST fuel trim B1','%',b=>(b[0]-128)*100/128],0x07:['LT fuel trim B1','%',b=>(b[0]-128)*100/128],
 0x0A:['Fuel pressure','kPa',b=>3*b[0]],0x0B:['Intake MAP','kPa',b=>b[0]],0x0C:['Engine RPM','rpm',b=>(256*b[0]+b[1])/4],
 0x0D:['Vehicle speed','km/h',b=>b[0]],0x0E:['Timing advance','°',b=>b[0]/2-64],0x0F:['Intake air temp','°C',b=>b[0]-40],
 0x10:['MAF','g/s',b=>(256*b[0]+b[1])/100],0x11:['Throttle','%',b=>b[0]*100/255],0x1F:['Run time','s',b=>256*b[0]+b[1]],
 0x21:['Distance with MIL','km',b=>256*b[0]+b[1]],0x2F:['Fuel level','%',b=>b[0]*100/255],0x31:['Distance since clear','km',b=>256*b[0]+b[1]],
 0x33:['Baro pressure','kPa',b=>b[0]],0x42:['Module voltage','V',b=>(256*b[0]+b[1])/1000],0x43:['Absolute load','%',b=>(256*b[0]+b[1])*100/255],
 0x44:['Commanded λ','',b=>(256*b[0]+b[1])*2/65536],0x45:['Rel. throttle','%',b=>b[0]*100/255],0x46:['Ambient temp','°C',b=>b[0]-40],
 0x49:['Accel pedal D','%',b=>b[0]*100/255],0x4C:['Cmd throttle','%',b=>b[0]*100/255],0x5C:['Oil temp','°C',b=>b[0]-40],
 0x5E:['Fuel rate','L/h',b=>(256*b[0]+b[1])/20]};
let pidList=[], pidOn=false, pidVals={}, engineState='OFF', lastAutoDtcPoll=0;
/* OBDb Honda Civic signalset records DID 2612 at DA0E→0E for 2012/2013.
   29-bit normal-fixed addressing: tester F1 → ECU 0E = 18DA0EF1;
   response ECU 0E → tester F1 = 18DAF10E. Read-only service 22. */
const HONDA_GEAR_ECU={tx:'18DA0EF1',rx:'18DAF10E',fmt:'E',info:{}};
let hondaGearOn=false,hondaGearPending=false,hondaGearTimer=null,hondaGearPollMs=250;
let hondaGear={v2:null,v3:null,updated:0,errors:0};
function drawHondaGear(){
  const v2=$('hondaGearV2'),v3=$('hondaGearV3'),age=$('hondaGearAge'),state=$('hondaGearState');
  if(v2) v2.textContent=hondaGear.v2===null?'—':String(hondaGear.v2);
  if(v3) v3.textContent=hondaGear.v3===null?'—':String(hondaGear.v3);
  const fresh=hondaGear.updated && Date.now()-hondaGear.updated<2000;
  if(age) age.textContent=fresh?(Math.max(0,Date.now()-hondaGear.updated)+' ms'):'stale / none';
  if(state) state.textContent=(hondaGearOn && fresh)?'LIVE · read-only DID 2612 · 29-bit Honda ECU 0E':(!hondaGearOn && hondaGear.updated)?'POLLING STOPPED · last valid raw values retained':(hondaGear.errors?'No valid reply yet · last status: '+hondaGear.errors:'Waiting for a valid 62 26 12 response.');
}
function pollHondaGear(){
  if(!hondaGearOn || !pidOn) return;
  if(hondaGearPending){ hondaGearTimer=setTimeout(pollHondaGear,hondaGearPollMs); return; }
  hondaGearPending=true;
  diagReq(HONDA_GEAR_ECU,'222612',(hex,why)=>{
    hondaGearPending=false;
    if(hex){
      const b=hexBytes(hex);
      /* DID data starts after 62 26 12; bit offsets from OBDb are absolute
         within that data block (byte 44 = variant 2, byte 46 = variant 3). */
      if(b.length>=50 && b[0]===0x62 && b[1]===0x26 && b[2]===0x12){
        hondaGear.v2=b[47]; hondaGear.v3=b[49]; hondaGear.updated=Date.now(); hondaGear.errors=0;
        gearLedFeed();
      } else hondaGear.errors='malformed 22 2612 response ('+b.length+' bytes)';
    } else if(why && why!=='NO_ANSWER') hondaGear.errors=why;
    drawHondaGear();
    if(hondaGearOn && pidOn){ hondaGearTimer=setTimeout(pollHondaGear,hondaGearPollMs); pollHondaAlt(); }
  },false,true);
}
/* ---- gear -> STM32 LED chase (GEAR,<n>) ------------------------------
   Debounce: forward only after two consecutive identical raw values from the
   selected variant, and only when the value changed since the last send. The
   STM32 side animates only on a real change (GEAR,ACK,<prev>,<new>,UP|DOWN). */
let gearLed={prev:null,stable:0,sent:null};
function gearLedToggle(){
  const on=$('gearLedOn')&&$('gearLedOn').checked;
  if(!on){ wsSend({op:'gear_clear'}); gearLed={prev:null,stable:0,sent:null}; const l=$('gearLedLast'); if(l) l.textContent='off'; }
}
function gearLedFeed(){
  const src=($('gearLedSrc')&&$('gearLedSrc').value)||'v2';
  const v=hondaGear[src];
  if(!Number.isInteger(v) || v<0 || v>15){ gearLed.prev=null; gearLed.stable=0; return; }
  if(v===gearLed.prev) gearLed.stable++; else { gearLed.prev=v; gearLed.stable=1; }
  if(gearLed.stable<2 || v===gearLed.sent) return;
  const on=$('gearLedOn')&&$('gearLedOn').checked;
  if(!on) return;
  gearLed.sent=v; wsSend({op:'gear',gear:v});
  const l=$('gearLedLast'); if(l) l.textContent=String(v)+' ('+src+')';
}
/* ---- R18Z1 5AT substitute candidates: ECU 1E, DIDs 2301 / 2322 / 3086 ----
   Read-only service 22, one DID per 500 ms, round-robin, same key-II + ARM
   gate as DID 2612 (it is only called from pollHondaGear's callback). */
const HONDA_ALT_ECU={tx:'18DA1EF1',rx:'18DAF11E',fmt:'E',info:{}};
const HONDA_ALT_DIDS=['2301','2322','3086'];
let hondaAlt={i:0,pending:false,last:0,pollMs:500,res:{},updated:0};
function drawHondaAlt(){
  for(const d of HONDA_ALT_DIDS){ const el=$('hondaAlt'+d); if(el) el.textContent=hondaAlt.res[d]||'—'; }
  const st=$('hondaAltState'); if(!st) return;
  const fresh=hondaAlt.updated && Date.now()-hondaAlt.updated<3000;
  st.textContent=(hondaGearOn&&fresh)?'LIVE · read-only 22 2301/2322/3086 · 29-bit Honda ECU 1E':(!hondaGearOn&&hondaAlt.updated)?'POLLING STOPPED · last raw values retained':'Waiting for a reply from ECU 1E.';
}
function pollHondaAlt(){
  if(!hondaGearOn || !pidOn || hondaAlt.pending) return;
  if(Date.now()-hondaAlt.last<hondaAlt.pollMs) return;
  const did=HONDA_ALT_DIDS[hondaAlt.i]; hondaAlt.i=(hondaAlt.i+1)%HONDA_ALT_DIDS.length;
  hondaAlt.pending=true; hondaAlt.last=Date.now();
  diagReq(HONDA_ALT_ECU,'22'+did,(hex,why)=>{
    hondaAlt.pending=false;
    if(hex){
      const b=hexBytes(hex);
      if(b.length>=3 && b[0]===0x62) hondaAlt.res[did]=b.slice(3).map(x=>x.toString(16).padStart(2,'0')).join(' ')||'(empty)';
      else hondaAlt.res[did]='NRC '+b.map(x=>x.toString(16).padStart(2,'0')).join(' ');
      hondaAlt.updated=Date.now();
    } else if(why && why!=='NO_ANSWER') hondaAlt.res[did]=why;
    drawHondaAlt();
  },false,true);
}
function startHondaGearPoll(){
  if(hondaGearOn) return;
  hondaGearOn=true; hondaGearPending=false; hondaGear.errors=0;
  if(hondaGearTimer) clearTimeout(hondaGearTimer);
  drawHondaGear(); pollHondaGear();
}
function stopHondaGearPoll(){
  hondaGearOn=false; hondaGearPending=false;
  if(hondaGearTimer) clearTimeout(hondaGearTimer);
  hondaGearTimer=null; drawHondaGear(); hondaAlt.pending=false; drawHondaAlt();
}
function pidEcu(){ const k=Object.keys(ecus).filter(r=>ecus[r].fmt==='S'); return k.length?ecus[k[0]]:{tx:'7E0',rx:'7E8',fmt:'S',info:{}}; }
function discoverPids(cb){
  pidList=[]; const e=pidEcu();
  const step=(base)=>diagReq(e,'01'+base.toString(16).padStart(2,'0').toUpperCase(),(hex)=>{
    if(!hex){ cb(); return; } const b=hexBytes(hex).slice(2);
    for(let i=0;i<32;i++) if(b[i>>3]&(0x80>>(i&7))){ const p=base+i+1; if((p&0x1F)!==0) pidList.push(p); }
    if((b[3]&1)&&base<0xC0) step(base+0x20); else cb(); },false,true);
  step(0);
}
function stopAutoPids(reason){
  if(!pidOn) return;
  pidOn=false; pidList=[]; stopHondaGearPoll();
  idq=idq.filter(q=>!q.autoPid);
  if(idCur && idCur.autoPid){
    clearTimeout(idTimer); idCur=null; udsBuf='';
    wsSend({op:'uds_abort',bus:'F'});
    setTimeout(idNext,30);
  }
  const state=$('autoPidState'); if(state) state.textContent='Automatic PID monitor paused: '+reason;
}
function syncAutoPids(){
  const shouldRun=(armedBus==='F' && armedLeft>0 && (engineState==='ON'||engineState==='RUN'));
  if(shouldRun && !pidOn){
    pidOn=true; pidVals={}; lastAutoDtcPoll=0; drawPids();
    const state=$('autoPidState'); if(state) state.textContent='Automatic monitor active · standard Mode 01 + Honda read-only DID 2612 · F-CAN ARM gate';
    startHondaGearPoll();
    discoverPids(()=>{ if(!pidOn) return; diagLog(pidList.length+' supported Mode 01 PIDs discovered'); pollPids(); });
  } else if(!shouldRun && pidOn){
    stopAutoPids(armedBus==='F' && armedLeft===0?'F-CAN ARM window expired':'key left position II or F-CAN disarmed');
  } else if(!shouldRun){
    const state=$('autoPidState');
    if(state) state.textContent=(armedBus==='F'&&armedLeft>0)?'Automatic PID monitor: waiting for key position II.':'Automatic PID monitor: waiting for key II and F-CAN ARM.';
  }
}
function pollDtcStatus(done){
  const e=pidEcu(), requests=['03','07','0A','1902FF']; let left=requests.length;
  requests.forEach(req=>diagReq(e,req,(hex)=>{
    if(hex) diagLog('Automatic DTC status received from '+e.rx+' ('+req+')');
    if(--left===0 && pidOn) done();
  },false,true));
}
function pollPids(){
  if(!pidOn || armedBus!=='F' || armedLeft<=0 || (engineState!=='ON'&&engineState!=='RUN')) return;
  if(!pidList.length){
    /* Keep the read-only DTC monitor alive even if this ECU exposes no Mode 01 sensor PIDs. */
    const now=Date.now();
    if(!lastAutoDtcPoll || now-lastAutoDtcPoll>=30000){
      lastAutoDtcPoll=now; pollDtcStatus(()=>setTimeout(pollPids,30000));
    } else setTimeout(pollPids,1000);
    return;
  }
  const e=pidEcu(); let left=pidList.length;
  pidList.forEach(p=>diagReq(e,'01'+p.toString(16).padStart(2,'0').toUpperCase(),(hex)=>{
    if(hex){ const b=hexBytes(hex); if(b[1]===p){ const d=b.slice(2), f=PIDS[p];
      pidVals[p]=f?[f[0],(Math.round(f[2](d)*100)/100)+' '+f[1]]:['PID 0x'+p.toString(16).toUpperCase(),d.map(x=>x.toString(16).padStart(2,'0')).join(' ')]; } }
    if(--left===0){
      drawPids();
      const now=Date.now();
      if(!lastAutoDtcPoll || now-lastAutoDtcPoll>=30000){
        lastAutoDtcPoll=now;
        pollDtcStatus(()=>setTimeout(pollPids,500));
      } else setTimeout(pollPids,500);
    }
  },false,true));
}
function drawPids(){
  drawHondaGear();
  const k=Object.keys(pidVals).map(Number).sort((a,b)=>a-b);
  $('pidRows').innerHTML=k.map(p=>{
    const pidHex=p.toString(16).toUpperCase().padStart(2,'0');
    const valueText=String(pidVals[p][1]), match=valueText.match(/^[+-]?(?:\d+(?:\.\d*)?|\.\d+)/);
    const numeric=match?match[0]:'';
    const editable=numeric?`<b data-field-id="OBD01:${pidHex}" data-field-label="${esc(pidVals[p][0])}" data-value="${esc(numeric)}" title="Double-tap in Write Mode to stage a proposed value">${esc(valueText)}</b>`:`<b>${esc(valueText)}</b>`;
    return `<tr><td>0x${pidHex}</td><td>${esc(pidVals[p][0])}</td><td>${editable}</td></tr>`;
  }).join('')||'<tr><td colspan="3" style="color:#8b949e">Waiting for engine / ECU telemetry…</td></tr>';
  const badge=$('liveSensorState');
  if(badge){
    const running=engineState==='RUN';
    badge.textContent=k.length?'LIVE · '+k.length+' PARAMETERS':(running?'ENGINE RUNNING · WAITING FOR ECU DATA':engineState==='OFF'?'ENGINE OFF':'IGNITION ON · WAITING FOR DATA');
    badge.className='live-badge'+(k.length?'':' off');
  }
}
function togglePids(){
  /* Kept as a guarded UI hook for future controls; never bypass the ARM/key gate. */
  if(pidOn){ stopAutoPids('monitor stopped by operator'); return; }
  syncAutoPids();
  if(!pidOn) diagLog('Automatic PID polling requires key position II and an active F-CAN ARM window.');
}
/* Read / write a value by identifier, IO control and routines (ISO 14229-1 services 22/2E/2F/31).
   Honda does not publish DIDs; find them with reads and captures. Writes need confirmation. */
function didOp(kind){
  const e=ecus[$('wrEcu').value]; if(!e){ diagLog('Pick a controller first (run a scan).'); return; }
  const did=$('wrDid').value.trim().toUpperCase(), data=$('wrData').value.replace(/\s+/g,'').toUpperCase();
  if(kind!=='session' && !/^[0-9A-F]{4}$/.test(did)){ diagLog('DID / routine must be 4 hex digits.'); return; }
  if(data && !/^([0-9A-F]{2})+$/.test(data)){ diagLog('Data must be hex bytes.'); return; }
  const req={read:'22'+did, write:'2E'+did+data, io:'2F'+did+'03'+data, ioret:'2F'+did+'00',
             routine:'3101'+did+data, session:'1003'}[kind];
  if(kind==='write' && !data){ diagLog('Write needs data.'); return; }
  if(req.length/2>64){ diagLog('Request too long (64 bytes max).'); return; }
  const ro=(kind==='read'||kind==='session');
  if(!ro && !confirm('Send '+req+' to '+e.tx+'?\nThis changes the controller ('+({write:'stored value',io:'output override',ioret:'return output to ECU',routine:'routine start'})[kind]+').')) return;
  $('wrOut').textContent='→ '+e.tx+' '+req;
  diagReq(e,req,(hex,why)=>{
    let txt=hex?hexBytes(hex).map(x=>x.toString(16).padStart(2,'0').toUpperCase()).join(' '):(why||'no answer');
    if(hex && kind==='read'){ const b=hexBytes(hex).slice(3); const a=b.filter(c=>c>=0x20&&c<0x7F).map(c=>String.fromCharCode(c)).join('');
      if(a.length===b.length&&a.length) txt+='  "'+a+'"'; }
    $('wrOut').textContent='← '+e.rx+' '+txt; diagLog(e.tx+' '+req+' → '+txt);
  }, !ro);
}
function fillRstEcus(){ $('wrEcu').innerHTML=Object.keys(ecus).map(r=>`<option value="${esc(r)}">${esc(ecuLabel(ecus[r]))}</option>`).join(''); $('rstEcu').innerHTML=Object.keys(ecus).map(r=>`<option value="${esc(r)}">${esc(ecuLabel(ecus[r]))}</option>`).join(''); }
function onEcu(t){
  const f=t.split(',');
  const tagged=(f[1]==='F'||f[1]==='B');
  const ei=tagged?2:1, ev=f[ei]||'';
  if(ev==='FOUND'){
    const base=ei+1, tx=f[base]||'', rx=f[base+1]||'';
    if(!tx||!rx){ diagLog('Malformed ECU result: '+t); return; }
    ecus[rx]={tx,rx,fmt:tx.length>3?'E':'S',reply:f[base+2]||'',info:(ecus[rx]&&ecus[rx].info)||{}};
    drawEcus(); fillRstEcus(); diagLog(t); return;
  }
  if(ev==='DONE'){ diagLog('ECU scan finished: '+(f[ei+1]||'0')+' controller(s) answered'); drawEcus(); return; }
  diagLog(t);
}

/* ── message handler ── */
function onMsg(m){
  if(m.type==='events_reset'){ eventRows.length=0; drawEvents(); return; }
  if(m.type==='event'){
    const i=eventRows.findIndex(e=>e.id===m.id);
    if(i>=0) eventRows[i]=m; else eventRows.push(m);
    drawEvents(); return;
  }
  if(m.type==='stats'){ onStats(String(m.message||''), m.bus); return; }
  if(m.type==='frames'){
    (m.l||[]).forEach(r=>{ const id=r[0], hex=r[3]||'';
      const s=frm$[id]||(frm$[id]={n:0,rate:0,ts:0,bytes:[],prev:[],dlc:0,label:'',bus:0});
      s.prev=[...s.bytes]; s.n=r[4]; s.rate=r[5]; s.ts=Date.now(); s.dlc=r[2]; s.bus=r[1];
      s.bytes=Array.from({length:8},(_,i)=>hex.length>=(i+1)*2?hex.substr(i*2,2):'--');
      if(prof$[id]&&prof$[id].label) s.label=prof$[id].label;
      if(r[1]===0&&id==='17C'&&hex.length>=2) bcPedal=parseInt(hex.substr(0,2),16);
    });
    busFps=m.fps||[0,0]; busBps=m.bps||[0,0]; if(m.bitrate) lastBitrate=m.bitrate;
    $('statusBar').textContent=(m.locked?'🔒 Locked ':'🔍 Scanning ')+(m.bitrate?m.bitrate+' bps':'—')+' | Frames: '+(m.total||0);
    return;
  }
  if(m.type==='veh'){
    const previous=engineState;
    engineState=m.state||'OFF'; veh=m;
    /* End the live session when the ignition leaves position II. */
    if((previous==='RUN'||previous==='ON') && engineState!=='RUN' && engineState!=='ON'){
      pidVals={}; Object.keys(pid$).forEach(k=>delete pid$[k]);
      Object.keys(seen$).forEach(k=>delete seen$[k]);
      drawPids();
    }
    /* STM32 reports src=OBD for decoded responses; src=BCAST is the
       candidate Honda RPM/speed broadcast. No guessed sensor values. */
    const on=(m.state==='ON'||m.state==='RUN');
    if(on && m.src==='BCAST'){ onMsg({type:'pid',pid:'0C',value:m.rpm}); onMsg({type:'pid',pid:'0D',value:m.spd}); }
    drawPids(); drawVeh(); syncAutoPids(); return;
  }
  if(m.type==='auth'){ authOk=!!m.ok; if(!authOk) diagLog('Command channel login failed.'); return; }
  if(m.type==='diag'){ onDiag(String(m.message||'')); return; }
  if(m.type==='status'){
    $('statusBar').textContent=(m.locked?'🔒 Locked ':'🔍 Scanning ')
      +(m.bitrate?(lastBitrate=m.bitrate)+' bps':'—')
      +' | Frames: '+(m.frames||0)
      +(m.dwell?' | Dwell: '+m.dwell+' ms':'');
    return;
  }
  if(m.type==='frame'){
    const id=m.id; if(!id) return;
    const s=frm$[id]||(frm$[id]={n:0,rate:0,ts:0,bytes:[],prev:[],dlc:0,label:''});
    s.prev=[...s.bytes]; s.n++; s.ts=Date.now(); s.dlc=m.dlc;
    const hex=(m.data||'').replace(/[^0-9a-fA-F]/g,'');
    s.bytes=Array.from({length:8},(_,i)=>hex.length>=(i+1)*2?hex.substr(i*2,2).toUpperCase():'--');
    if(prof$[id]) s.rate=prof$[id].rate;
    return;
  }
  if(m.type==='pid'){
    const pid=(m.pid||'').toUpperCase(), val=parseFloat(m.value);
    if(isNaN(val)) return;
    const pidNo=parseInt(pid,16);
    if(Number.isFinite(pidNo)){
      const meta=PIDS[pidNo];
      const label=m.label|| (meta&&meta[0]) || ('PID 0x'+pid);
      const unit=m.unit|| (meta&&meta[1]) || '';
      pidVals[pidNo]=[label,(Math.round(val*100)/100)+' '+unit];
      drawPids();
    }
    if(!pid$[pid]) pid$[pid]=[];
    pid$[pid].push({ts:Date.now(),v:val});
    if(pid$[pid].length>300) pid$[pid].shift();
    seen$[pid]=Date.now();
    updCard(pid,val);
    chkAlerts(pid,val); updAlerts();
    if(pid==='0C'&&pid$['11']&&pid$['11'].length){
      const thr=pid$['11'][pid$['11'].length-1].v;
      scat.push({thr,rpm:val}); if(scat.length>500) scat.shift();
    }
    return;
  }
  if(m.type==='profile'){
    const id=(m.id||'').toUpperCase();
    prof$[id]={dlc:m.dlc,rate:parseFloat(m.rate)||0,label:m.label||''};
    if(frm$[id]) frm$[id].label=m.label||'';
    if(frm$[id]) frm$[id].rate=parseFloat(m.rate)||0;
    return;
  }
  if(m.type==='sig'){
    const id=(m.id||'').toUpperCase(), b=m.byte||0, k=id+':'+b;
    const s=sig$[k]||(sig$[k]={last:'',min:'',max:'',rate:0,label:'',hist:[],byte:b});
    s.last=m.last||''; s.min=m.min||''; s.max=m.max||'';
    s.rate=parseFloat(m.rate)||0; s.label=m.label||''; s.byte=b;
    const n=parseInt(m.last,16);
    if(!isNaN(n)){s.hist.push(n);if(s.hist.length>20)s.hist.shift();}
    return;
  }
  if(m.type==='tx'){ $('txStat').textContent='STM32: '+(m.message||''); return; }
  if(m.type==='stm32_progress'){ handleFwProgress(m); return; }
}

/* ── remote engine start helpers ── */
function doStartCfg(){
  let to=Math.round((parseFloat(document.getElementById('rsTo').value)||2)*1000);
  if(to<500)to=500; if(to>10000)to=10000;
  wsSend({op:'startinputs',neutral:document.getElementById('rsNeutral').checked?1:0,brake:document.getElementById('rsBrake').checked?1:0,immob:document.getElementById('rsImmob').checked?1:0});
  wsSend({op:'startcfg',timeout:to,armed:1,neutral:1,brake:1,immob:1,rpm:400});
  diagLog('Remote-start safety applied (timeout '+(to/1000)+' s).');
}
function doStart(){
  if(!confirm('Crank the engine now?\nThe starter engages until the ECU reports RPM or the timeout elapses. Make sure the car is safe to start.')) return;
  wsSend({op:'startinputs',neutral:document.getElementById('rsNeutral').checked?1:0,brake:document.getElementById('rsBrake').checked?1:0,immob:document.getElementById('rsImmob').checked?1:0});
  wsSend({op:'start'});
  diagLog('→ remote START requested on '+curBus);
}

/* ── websocket ── */
function connect(){
  $('statusBar').textContent='Connecting…';
  try{ sock=new WebSocket('ws://'+location.hostname+':81/'); }
  catch(e){ $('statusBar').textContent='WS error: '+e.message; setTimeout(connect,2000); return; }
  sock.onopen =()=>{
    $('statusBar').textContent='Connected — waiting for STM32…';
    authOk=true;                 /* Wi-Fi WPA2 is the only gate (owner request) */
  };
  sock.onclose=()=>{ $('statusBar').textContent='Disconnected — reconnecting…'; setTimeout(connect,1500); };
  sock.onerror=()=>$('statusBar').textContent='WebSocket error';
  sock.onmessage=e=>{ try{onMsg(JSON.parse(e.data));}catch{} };
}

/* ── 1s redraw loop ── */
setInterval(()=>{ drawRpm(); drawTemp(); drawScat(); updQuality(); updCAN(); updSigs(); }, 1000);
window.addEventListener('resize',()=>{ drawRpm(); drawTemp(); drawScat(); });
connect();
/* Sidebar section navigation: smooth scroll, active item, and clear return path. */
(function initNavigation(){
  const nav=document.getElementById('sideNav');
  if(!nav) return;
  const links=[...nav.querySelectorAll('a[href^="#"]')];
  function mark(){
    const y=window.scrollY+120;
    let current='#top';
    links.forEach(a=>{const el=document.querySelector(a.getAttribute('href'));if(el&&el.offsetTop<=y)current=a.getAttribute('href');});
    links.forEach(a=>a.classList.toggle('active',a.getAttribute('href')===current));
  }
  links.forEach(a=>a.addEventListener('click',e=>{
    const target=document.querySelector(a.getAttribute('href'));
    if(target){e.preventDefault();target.scrollIntoView({behavior:'smooth',block:'start'});history.replaceState(null,'',a.getAttribute('href'));}
  }));
  window.addEventListener('scroll',mark,{passive:true});mark();
})();
</script>
<button id="backTop" type="button" onclick="window.scrollTo({top:0,behavior:'smooth'})" aria-label="Back to dashboard top">↑ Back to top</button>

<!-- ══ FULL WIDTH: Firmware Update ══ -->
<div class="card full" id="fwCard">
  <div class="card-head">
    <span class="card-title">⚡ Firmware Update</span>
    <span class="card-sub" id="fwStatus">Ready</span>
  </div>
  <div style="display:grid;grid-template-columns:1fr 1fr;gap:16px;margin-top:4px">

    <!-- ESP32 update -->
    <div style="background:#0d1117;border:1px solid #30363d;border-radius:6px;padding:12px">
      <div style="font-size:.78rem;font-weight:700;color:#79c0ff;margin-bottom:6px">ESP32 Dashboard Firmware</div>
      <div style="font-size:.68rem;color:#8b949e;margin-bottom:8px">Upload a new <code style="background:#21262d;padding:1px 4px;border-radius:3px">.bin</code> built with PlatformIO. The ESP32 will reboot automatically after flashing.</div>
      <div id="esp32DropZone" style="border:2px dashed #30363d;border-radius:6px;padding:14px;text-align:center;cursor:pointer;transition:border-color .2s" ondragover="event.preventDefault();this.style.borderColor='#58a6ff'" ondragleave="this.style.borderColor='#30363d'" ondrop="dropFw(event,'esp32')">
        <div style="font-size:.72rem;color:#8b949e">Drag &amp; drop <b>.bin</b> here, or</div>
        <label style="margin-top:6px;display:inline-block">
          <input type="file" id="esp32File" accept=".bin" style="display:none" onchange="selectFw('esp32')">
          <span class="btn btn-sm" style="margin:0" onclick="document.getElementById('esp32File').click()">Browse</span>
        </label>
        <div id="esp32FileName" style="font-size:.68rem;color:#8b949e;margin-top:4px">No file selected</div>
      </div>
      <div style="margin-top:8px">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:4px">
          <span style="font-size:.68rem;color:#8b949e" id="esp32Pct">0%</span>
          <button class="btn btn-sm" onclick="flashESP32()" id="esp32Btn" disabled>Flash ESP32</button>
        </div>
        <div style="height:6px;background:#21262d;border-radius:3px;overflow:hidden">
          <div id="esp32Bar" style="height:6px;width:0%;background:#58a6ff;border-radius:3px;transition:width .2s"></div>
        </div>
        <div id="esp32Msg" style="font-size:.65rem;color:#8b949e;margin-top:4px;min-height:1em"></div>
      </div>
    </div>

    <!-- STM32 update -->
    <div style="background:#0d1117;border:1px solid #30363d;border-radius:6px;padding:12px">
      <div style="font-size:.78rem;font-weight:700;color:#e3b341;margin-bottom:6px">STM32F407 Firmware</div>
      <div style="font-size:.68rem;color:#8b949e;margin-bottom:4px">Upload a <code style="background:#21262d;padding:1px 4px;border-radius:3px">.bin</code> built with <code style="background:#21262d;padding:1px 4px;border-radius:3px">make firmware</code>. Application and ROM loader share USART3 PB10/PB11 on GPIO16/17. <b style="color:#f85149">Hardware prerequisite:</b> fit MB997 SB19 ON so PB2/BOOT1 is held low. Not verified on a physical board.</div>
      <div style="font-size:.65rem;background:#1c2128;border:1px solid #30363d;border-radius:4px;padding:6px;margin-bottom:8px;color:#8b949e">
        <b style="color:#e3b341">⚠ Wiring required:</b> ESP32 GPIO26 → BOOT0 (P2-21) &nbsp;|&nbsp; GPIO25 → NRST (P1-6, open-drain) &nbsp;|&nbsp; STM32 PB10/USART3_TX → GPIO16/RX &nbsp;|&nbsp; GPIO17/TX → PB11/USART3_RX. <b>Fit SB19 ON (BOOT1 low); no ESP32 PB2 wire.</b>
      </div>
      <div id="stm32DropZone" style="border:2px dashed #30363d;border-radius:6px;padding:14px;text-align:center;cursor:pointer;transition:border-color .2s" ondragover="event.preventDefault();this.style.borderColor='#e3b341'" ondragleave="this.style.borderColor='#30363d'" ondrop="dropFw(event,'stm32')">
        <div style="font-size:.72rem;color:#8b949e">Drag &amp; drop <b>.bin</b> here, or</div>
        <label style="margin-top:6px;display:inline-block">
          <input type="file" id="stm32File" accept=".bin" style="display:none" onchange="selectFw('stm32')">
          <span class="btn btn-sm" style="margin:0;background:#2d2008;border-color:#e3b341;color:#e3b341" onclick="document.getElementById('stm32File').click()">Browse</span>
        </label>
        <div id="stm32FileName" style="font-size:.68rem;color:#8b949e;margin-top:4px">No file selected</div>
      </div>
      <div style="margin-top:8px">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:4px">
          <span style="font-size:.68rem;color:#8b949e" id="stm32Pct">0%</span>
          <button class="btn btn-sm" onclick="flashSTM32()" id="stm32Btn" disabled style="background:#2d2008;border-color:#e3b341;color:#e3b341">Flash STM32</button>
        </div>
        <div style="height:6px;background:#21262d;border-radius:3px;overflow:hidden">
          <div id="stm32Bar" style="height:6px;width:0%;background:#e3b341;border-radius:3px;transition:width .2s"></div>
        </div>
        <div id="stm32Msg" style="font-size:.65rem;color:#8b949e;margin-top:4px;min-height:1em"></div>
      </div>
    </div>

  </div>
</div>

<script>
/* ═══════════════════════════════════════════════
   FIRMWARE UPDATE
   ═══════════════════════════════════════════════ */
const fwFiles={esp32:null,stm32:null};

function selectFw(target){
  const f=document.getElementById(target+'File').files[0];
  if(!f) return;
  fwFiles[target]=f;
  document.getElementById(target+'FileName').textContent=f.name+' ('+Math.round(f.size/1024)+' KB)';
  document.getElementById(target+'Btn').disabled=false;
}
function dropFw(ev,target){
  event.preventDefault();
  document.getElementById(target+'DropZone').style.borderColor='#30363d';
  const f=ev.dataTransfer.files[0];
  if(!f||!f.name.endsWith('.bin')){
    document.getElementById(target+'Msg').textContent='Only .bin files accepted.'; return;
  }
  fwFiles[target]=f;
  document.getElementById(target+'FileName').textContent=f.name+' ('+Math.round(f.size/1024)+' KB)';
  document.getElementById(target+'Btn').disabled=false;
  document.getElementById(target+'Msg').textContent='';
}

function flashESP32(){
  const f=fwFiles.esp32; if(!f) return;
  document.getElementById('esp32Btn').disabled=true;
  document.getElementById('esp32Msg').textContent='Uploading…';
  const xhr=new XMLHttpRequest();
  xhr.open('POST','/update',true);
  xhr.upload.onprogress=e=>{
    if(e.lengthComputable){
      const pct=Math.round(e.loaded/e.total*100);
      document.getElementById('esp32Bar').style.width=pct+'%';
      document.getElementById('esp32Pct').textContent=pct+'%';
    }
  };
  xhr.onload=()=>{
    if(xhr.status===200){
      document.getElementById('esp32Msg').style.color='#56d364';
      document.getElementById('esp32Msg').textContent='✓ Flash OK — ESP32 rebooting…';
      document.getElementById('esp32Bar').style.background='#56d364';
    } else {
      document.getElementById('esp32Msg').style.color='#f85149';
      document.getElementById('esp32Msg').textContent='✗ Error: '+xhr.responseText;
      document.getElementById('esp32Btn').disabled=false;
    }
  };
  xhr.onerror=()=>{
    document.getElementById('esp32Msg').style.color='#f85149';
    document.getElementById('esp32Msg').textContent='✗ Upload failed — connection lost (normal after reboot)';
    document.getElementById('esp32Btn').disabled=false;
  };
  const fd=new FormData(); fd.append('firmware',f,f.name);
  xhr.send(fd);
}

function flashSTM32(){
  const f=fwFiles.stm32; if(!f) return;
  document.getElementById('stm32Btn').disabled=true;
  document.getElementById('stm32Msg').textContent='Uploading binary to ESP32…';
  document.getElementById('stm32Bar').style.width='5%';
  const xhr=new XMLHttpRequest();
  xhr.open('POST','/update-stm32',true);
  xhr.upload.onprogress=e=>{
    if(e.lengthComputable){
      const pct=Math.round(e.loaded/e.total*50); // upload = 0-50%
      document.getElementById('stm32Bar').style.width=pct+'%';
      document.getElementById('stm32Pct').textContent=pct+'%';
    }
  };
  xhr.onload=()=>{
    if(xhr.status===200){
      document.getElementById('stm32Msg').style.color='#56d364';
      document.getElementById('stm32Msg').textContent='✓ '+xhr.responseText;
      document.getElementById('stm32Bar').style.width='100%';
      document.getElementById('stm32Bar').style.background='#56d364';
      document.getElementById('stm32Pct').textContent='100%';
    } else {
      document.getElementById('stm32Msg').style.color='#f85149';
      document.getElementById('stm32Msg').textContent='✗ Error: '+xhr.responseText;
      document.getElementById('stm32Btn').disabled=false;
    }
  };
  xhr.onerror=()=>{
    document.getElementById('stm32Msg').style.color='#f85149';
    document.getElementById('stm32Msg').textContent='✗ Connection lost during upload.';
    document.getElementById('stm32Btn').disabled=false;
  };
  // STM32 flash progress arrives as WebSocket stm32_progress messages
  const fd=new FormData(); fd.append('firmware',f,f.name);
  xhr.send(fd);
}

/* Listen for stm32_progress messages from WebSocket */
function handleFwProgress(m){
  if(m.type!=='stm32_progress') return;
  const pct=50+Math.round((m.pct||0)/2); // flashing = 50-100%
  document.getElementById('stm32Bar').style.width=pct+'%';
  document.getElementById('stm32Pct').textContent=pct+'%';
  document.getElementById('stm32Msg').textContent=m.msg||'';
}
</script>
</body></html>)HTML";



/* ═══════════════════════════════════════════════════════════════════
   C++ HELPERS
   ═══════════════════════════════════════════════════════════════════ */
static String csvField(const char *s, int idx) {
    int start = 0, field = 0;
    while (s[start]) {
        int end = start;
        while (s[end] && s[end] != ',') end++;
        if (field == idx) return String(s + start).substring(0, end - start);
        field++;
        start = end + (s[end] == ',' ? 1 : 0);
        if (!s[end]) break;
    }
    return String("");
}

static int countCommas(const char *s) {
    int n = 0; for (; *s; s++) if (*s == ',') n++; return n;
}

/* ═══════════════════════════════════════════════════════════════════
   FRAME TABLE (per-ID latest data + rate, batched to the browser)
   ═══════════════════════════════════════════════════════════════════ */
static constexpr int      FRAME_TABLE_MAX = 192;
static constexpr uint32_t FRAME_BATCH_MS  = 250;
struct FrameEnt {
    uint32_t id; uint8_t bus, ext, dlc, dirty;
    char     hex[17];
    uint32_t total, win;       /* all frames / frames in current 1 s window */
    float    rate;             /* Hz, from the last full window              */
};
static FrameEnt ftab[FRAME_TABLE_MAX];
static int      ftabN = 0;
static uint32_t busWinFrames[2], busWinBits[2], busFps[2], busBitsPs[2];

static void frameTableUpdate(uint32_t id, bool ext, uint8_t bus, uint8_t dlc, const char *hex) {
    int i = 0;
    for (; i < ftabN; i++) if (ftab[i].id == id && ftab[i].bus == bus && ftab[i].ext == ext) break;
    if (i == ftabN) {
        if (ftabN >= FRAME_TABLE_MAX) return;
        memset(&ftab[i], 0, sizeof ftab[i]);
        ftab[i].id = id; ftab[i].bus = bus; ftab[i].ext = ext; ftabN++;
    }
    FrameEnt &e = ftab[i];
    e.dlc = dlc; strlcpy(e.hex, hex, sizeof e.hex); e.total++; e.win++; e.dirty = 1;
    /* ISO 11898 frame bits: 47 (std) / 67 (ext) + 8*dlc, ~10 % stuffing */
    busWinFrames[bus & 1]++; busWinBits[bus & 1] += ((ext ? 67u : 47u) + 8u * dlc) * 11u / 10u;
}

static void frameBatchFlush() {
    static uint32_t lastBatch = 0, lastWin = 0;
    const uint32_t now = millis();
    if (now - lastWin >= 1000) {
        const float s = (now - lastWin) / 1000.0f; lastWin = now;
        for (int i = 0; i < ftabN; i++) { ftab[i].rate = ftab[i].win / s; ftab[i].win = 0; ftab[i].dirty = 1; }
        for (int b = 0; b < 2; b++) { busFps[b] = busWinFrames[b] / s; busBitsPs[b] = busWinBits[b] / s;
                                      busWinFrames[b] = busWinBits[b] = 0; }
    }
    if (now - lastBatch < FRAME_BATCH_MS || ws.connectedClients() == 0) return;
    lastBatch = now;
    static char out[12288];
    int n = snprintf(out, sizeof out,
        "{\"type\":\"frames\",\"fps\":[%u,%u],\"bps\":[%u,%u],\"total\":%u,\"locked\":%s,\"bitrate\":%u,\"l\":[",
        (unsigned)busFps[0], (unsigned)busFps[1], (unsigned)busBitsPs[0], (unsigned)busBitsPs[1],
        (unsigned)framesSeen, locked ? "true" : "false", (unsigned)bitrate);
    bool first = true;
    for (int i = 0; i < ftabN && n < (int)sizeof out - 96; i++) {
        FrameEnt &e = ftab[i];
        if (!e.dirty) continue;
        e.dirty = 0;
        n += snprintf(out + n, sizeof out - n, "%s[\"%0*X\",%u,%u,\"%s\",%u,%.1f]",
                      first ? "" : ",", e.ext ? 8 : 3, (unsigned)e.id, e.bus, e.dlc, e.hex,
                      (unsigned)e.total, (double)e.rate);
        first = false;
    }
    if (n < (int)sizeof out - 3) { out[n++] = ']'; out[n++] = '}'; out[n] = 0; ws.broadcastTXT(out, n); }
}

/* ═══════════════════════════════════════════════════════════════════
   DECODED-DATA LOGGER (LittleFS, 1 Hz) + READ-ONLY OBSERVER (TCP 2323)
   The observer port only ever writes; anything a client sends is read and
   discarded, so nothing on it can reach the STM32 or the CAN bus.
   ═══════════════════════════════════════════════════════════════════ */
static constexpr char   LOG_CUR[] = "/log.csv", LOG_OLD[] = "/log.1.csv";
static constexpr size_t LOG_ROTATE_BYTES = 512u * 1024u;   /* x2 files fits the 1.3 MB FS */
static const char LOG_HDR[] = "uptime_ms,utc,key,rpm,speed_kmh,rpm_src,acc_wire,pedal_17C_b0,fps_F,fps_B,pids\n";
static bool     logFsOk = false;
WiFiServer      obsSrv(2323);
static WiFiClient obsCli;
static struct { char key[6] = "?"; int rpm = 0; float spd = 0; char src[6] = "NONE"; int acc = 0;
                char pids[200] = ""; } snap;

/* DTC event journal: each ECU-reported DTC gets a persistent state record and
 * an independent CSV capture containing up to 60 seconds of pre-trigger data
 * plus 10 seconds after the DTC reply. The RAM ring is sampled at 1 Hz from the
 * same telemetry row used by the normal logger. */
static constexpr char EVENT_FILE[] = "/events.json";
static constexpr char EVENT_TMP[]  = "/events.tmp";
static constexpr char EVENT_BAK[]  = "/events.bak";
static constexpr size_t EVENT_MAX = 40;
struct DtcEvent {
    uint32_t id = 0;
    char code[12] = "";
    char source[24] = "";
    char bus[2] = "F";
    char ecu[9] = "";
    char status[20] = "";
    char timestamp[24] = "";
    char lastAt[24] = "";
    bool seen = false;
    uint32_t repeats = 1;
    char vehicleState[6] = "?";
    int rpm = 0;
    float speed = 0;
    char rpmSource[6] = "NONE";
    int acc = 0;
    char pids[200] = "";
    uint32_t frames = 0;
    uint16_t fpsF = 0, fpsB = 0;
    bool snapshotReady = false;
    uint32_t lastReportedMs = 0; /* monotonic dedupe window; reset on reboot */
};
static DtcEvent eventStore[EVENT_MAX];
static size_t eventCount = 0;
static uint32_t nextEventId = 1;
static constexpr size_t EVENT_PRE_SAMPLES = 60;
static constexpr uint8_t EVENT_POST_SAMPLES = 10;
static constexpr size_t EVENT_SNAPSHOT_MAX = 8; /* reserve LittleFS capacity for rolling logs */
static constexpr size_t EVENT_CAPTURE_MAX_ACTIVE = 8;
struct EventSample { char csv[400] = ""; };
struct PendingEventCapture { uint32_t id = 0; uint8_t remaining = 0; };
static EventSample eventPreRing[EVENT_PRE_SAMPLES];
static size_t eventPreHead = 0, eventPreCount = 0;
static PendingEventCapture pendingEventCaptures[EVENT_CAPTURE_MAX_ACTIVE];
static size_t pendingEventCount = 0;
struct DtcResponseBuffer {
    char ecu[9] = "";
    char hex[1025] = ""; /* bounded 512-byte DTC reply payload, represented as hex */
    size_t hexLen = 0;
    uint16_t nextOffset = 0;
    bool active = false;
};
static DtcResponseBuffer dtcResponse[2];
static bool saveEvents();
static void broadcastEvents();

static DtcEvent *findEvent(uint32_t id) {
    for (size_t i = 0; i < eventCount; ++i) if (eventStore[i].id == id) return &eventStore[i];
    return nullptr;
}

static bool eventSnapshotPath(uint32_t id, char *out, size_t cap) {
    const int n = snprintf(out, cap, "/event_%08lX.csv", (unsigned long)id);
    return n > 0 && (size_t)n < cap;
}

static bool writeEventCsvRow(File &f, uint32_t id, const char *csv, size_t n) {
    const DtcEvent *e = findEvent(id);
    if (!e) return false;
    char prefix[128];
    const int p = snprintf(prefix, sizeof prefix, "%lu,%s,%s,%s,%s,",
                           (unsigned long)e->id, e->code, e->ecu, e->bus, e->timestamp);
    return p > 0 && (size_t)p < sizeof prefix &&
           f.write((const uint8_t *)prefix, (size_t)p) == (size_t)p &&
           f.write((const uint8_t *)csv, n) == n;
}

static void pushEventPreSample(const char *csv, size_t n) {
    if (!csv || n == 0 || n >= sizeof eventPreRing[0].csv) return;
    EventSample &slot = eventPreRing[eventPreHead];
    memcpy(slot.csv, csv, n); slot.csv[n] = '\0';
    eventPreHead = (eventPreHead + 1u) % EVENT_PRE_SAMPLES;
    if (eventPreCount < EVENT_PRE_SAMPLES) ++eventPreCount;
}

static bool startEventCapture(uint32_t id) {
    if (!logFsOk || stm32FlashRunning || pendingEventCount >= EVENT_CAPTURE_MAX_ACTIVE) return false;
    size_t retained = pendingEventCount;
    for (size_t i = 0; i < eventCount; ++i) if (eventStore[i].snapshotReady) ++retained;
    if (retained >= EVENT_SNAPSHOT_MAX) {
        bool released = false;
        for (size_t i = 0; i < eventCount; ++i) {
            DtcEvent &old = eventStore[i];
            if (!old.seen || !old.snapshotReady) continue; /* never discard an unseen alarm's evidence */
            char oldPath[32];
            if (eventSnapshotPath(old.id, oldPath, sizeof oldPath)) LittleFS.remove(oldPath);
            old.snapshotReady = false; released = true;
            (void)saveEvents(); break;
        }
        if (!released) return false;
    }
    const size_t needed = (eventPreCount + EVENT_POST_SAMPLES) * sizeof eventPreRing[0].csv + strlen(LOG_HDR) + 4096u;
    const size_t total = LittleFS.totalBytes(), used = LittleFS.usedBytes();
    if (used >= total || total - used < needed) return false;
    char path[32]; if (!eventSnapshotPath(id, path, sizeof path)) return false;
    File f = LittleFS.open(path, FILE_WRITE);
    if (!f) return false;
    static const char EVENT_LOG_HDR[] = "event_id,dtc,ecu,bus,event_utc,uptime_ms,utc,key,rpm,speed_kmh,rpm_src,acc_wire,pedal_17C_b0,fps_F,fps_B,pids\n";
    bool ok = f.print(EVENT_LOG_HDR) == strlen(EVENT_LOG_HDR);
    const size_t first = (eventPreHead + EVENT_PRE_SAMPLES - eventPreCount) % EVENT_PRE_SAMPLES;
    for (size_t i = 0; ok && i < eventPreCount; ++i) {
        const EventSample &sample = eventPreRing[(first + i) % EVENT_PRE_SAMPLES];
        const size_t n = strlen(sample.csv);
        if (n && !writeEventCsvRow(f, id, sample.csv, n)) ok = false;
    }
    f.close();
    if (!ok) { LittleFS.remove(path); return false; }
    PendingEventCapture &pending = pendingEventCaptures[pendingEventCount++];
    pending.id = id; pending.remaining = EVENT_POST_SAMPLES;
    return true;
}

static void capturePostEventSample(const char *csv, size_t n) {
    for (size_t i = 0; i < pendingEventCount; ) {
        PendingEventCapture &pending = pendingEventCaptures[i];
        char path[32]; const bool pathOk = eventSnapshotPath(pending.id, path, sizeof path);
        bool ok = false;
        if (pathOk) {
            File f = LittleFS.open(path, FILE_APPEND);
            if (f) { ok = writeEventCsvRow(f, pending.id, csv, n); f.close(); }
        }
        DtcEvent *e = findEvent(pending.id);
        if (!ok || !e || pending.remaining <= 1u) {
            if (e && ok && pending.remaining <= 1u) {
                e->snapshotReady = true;
                if (saveEvents()) broadcastEvents();
                else { e->snapshotReady = false; Serial.println("EVENTS: capture completion metadata not persisted"); }
            } else if (e) {
                e->snapshotReady = false;
                Serial.printf("EVENTS: post-trigger capture failed for event %lu\n", (unsigned long)pending.id);
                (void)saveEvents(); broadcastEvents();
            }
            pendingEventCaptures[i] = pendingEventCaptures[--pendingEventCount];
            continue;
        }
        --pending.remaining;
        ++i;
    }
}

static String eventUtcNow() {
    time_t t = time(nullptr);
    if (t > 1700000000) {
        struct tm tmv; char out[24];
        gmtime_r(&t, &tmv);
        if (strftime(out, sizeof out, "%Y-%m-%dT%H:%M:%SZ", &tmv)) return String(out);
    }
    return String("UP+") + String(millis());
}

static bool saveEvents() {
    if (!logFsOk) return false;
    JsonDocument d; JsonArray a = d.to<JsonArray>();
    for (size_t i = 0; i < eventCount; ++i) {
        const DtcEvent &e = eventStore[i]; JsonObject o = a.add<JsonObject>();
        o["id"] = e.id; o["code"] = e.code; o["source"] = e.source;
        o["bus"] = e.bus; o["ecu"] = e.ecu; o["status"] = e.status;
        o["timestamp"] = e.timestamp; o["lastAt"] = e.lastAt;
        o["seen"] = e.seen; o["repeats"] = e.repeats;
        o["vehicleState"] = e.vehicleState; o["rpm"] = e.rpm;
        o["speed"] = e.speed; o["rpmSource"] = e.rpmSource;
        o["acc"] = e.acc; o["pids"] = e.pids; o["frames"] = e.frames;
        o["fpsF"] = e.fpsF; o["fpsB"] = e.fpsB; o["snapshotReady"] = e.snapshotReady;
    }
    LittleFS.remove(EVENT_TMP);
    File f = LittleFS.open(EVENT_TMP, FILE_WRITE);
    if (!f) return false;
    const size_t written = serializeJson(d, f);
    f.close();
    if (!written) { LittleFS.remove(EVENT_TMP); return false; }
    LittleFS.remove(EVENT_BAK);
    const bool hadOld = LittleFS.exists(EVENT_FILE);
    if (hadOld && !LittleFS.rename(EVENT_FILE, EVENT_BAK)) {
        LittleFS.remove(EVENT_TMP); return false;
    }
    if (!LittleFS.rename(EVENT_TMP, EVENT_FILE)) {
        if (hadOld) LittleFS.rename(EVENT_BAK, EVENT_FILE);
        LittleFS.remove(EVENT_TMP); return false;
    }
    LittleFS.remove(EVENT_BAK);
    return true;
}

static void loadEvents() {
    eventCount = 0; nextEventId = 1;
    if (!logFsOk || !LittleFS.exists(EVENT_FILE)) return;
    File f = LittleFS.open(EVENT_FILE, FILE_READ);
    if (!f) return;
    JsonDocument d;
    const DeserializationError err = deserializeJson(d, f);
    f.close();
    if (err || !d.is<JsonArray>()) {
        Serial.printf("EVENTS: journal parse failed (%s); original file left untouched\n", err.c_str());
        return;
    }
    for (JsonObjectConst o : d.as<JsonArrayConst>()) {
        if (eventCount >= EVENT_MAX) break;
        DtcEvent &e = eventStore[eventCount++];
        e.id = o["id"] | 0UL;
        strlcpy(e.code, o["code"] | "", sizeof e.code);
        strlcpy(e.source, o["source"] | "", sizeof e.source);
        strlcpy(e.bus, o["bus"] | "F", sizeof e.bus);
        strlcpy(e.ecu, o["ecu"] | "", sizeof e.ecu);
        strlcpy(e.status, o["status"] | "", sizeof e.status);
        strlcpy(e.timestamp, o["timestamp"] | "", sizeof e.timestamp);
        strlcpy(e.lastAt, o["lastAt"] | "", sizeof e.lastAt);
        e.seen = o["seen"] | false; e.repeats = o["repeats"] | 1UL;
        strlcpy(e.vehicleState, o["vehicleState"] | "?", sizeof e.vehicleState);
        e.rpm = o["rpm"] | 0; e.speed = o["speed"] | 0.0f;
        strlcpy(e.rpmSource, o["rpmSource"] | "NONE", sizeof e.rpmSource);
        e.acc = o["acc"] | 0; strlcpy(e.pids, o["pids"] | "", sizeof e.pids);
        e.frames = o["frames"] | 0UL; e.fpsF = o["fpsF"] | 0; e.fpsB = o["fpsB"] | 0;
        e.snapshotReady = (o["snapshotReady"] | false);
        e.lastReportedMs = 0;
        char snapshotPath[32];
        if (!eventSnapshotPath(e.id, snapshotPath, sizeof snapshotPath) || !LittleFS.exists(snapshotPath)) e.snapshotReady = false;
        else if (!e.snapshotReady) LittleFS.remove(snapshotPath); /* discard incomplete capture after power loss */
        if (e.id >= nextEventId) nextEventId = e.id + 1;
    }
    Serial.printf("EVENTS: loaded %u record(s), next id=%lu\n", (unsigned)eventCount, (unsigned long)nextEventId);
}

static void sendEvent(uint8_t client, const DtcEvent &e) {
    JsonDocument d; d["type"] = "event"; d["id"] = e.id;
    d["code"] = e.code; d["source"] = e.source; d["bus"] = e.bus;
    d["ecu"] = e.ecu; d["status"] = e.status; d["timestamp"] = e.timestamp;
    d["lastAt"] = e.lastAt; d["seen"] = e.seen; d["repeats"] = e.repeats;
    d["vehicleState"] = e.vehicleState; d["rpm"] = e.rpm; d["speed"] = e.speed;
    d["rpmSource"] = e.rpmSource; d["acc"] = e.acc; d["pids"] = e.pids;
    d["frames"] = e.frames; d["fpsF"] = e.fpsF; d["fpsB"] = e.fpsB; d["snapshotReady"] = e.snapshotReady;
    String out; serializeJson(d, out); ws.sendTXT(client, out);
}

static void sendAllEvents(uint8_t client) {
    ws.sendTXT(client, "{\"type\":\"events_reset\"}");
    for (size_t i = 0; i < eventCount; ++i) sendEvent(client, eventStore[i]);
}

static void broadcastEvents() {
    ws.broadcastTXT("{\"type\":\"events_reset\"}");
    for (size_t i = 0; i < eventCount; ++i) {
        JsonDocument d; const DtcEvent &e = eventStore[i];
        d["type"] = "event"; d["id"] = e.id; d["code"] = e.code;
        d["source"] = e.source; d["bus"] = e.bus; d["ecu"] = e.ecu;
        d["status"] = e.status; d["timestamp"] = e.timestamp;
        d["lastAt"] = e.lastAt; d["seen"] = e.seen; d["repeats"] = e.repeats;
        d["vehicleState"] = e.vehicleState; d["rpm"] = e.rpm; d["speed"] = e.speed;
        d["rpmSource"] = e.rpmSource; d["acc"] = e.acc; d["pids"] = e.pids;
        d["frames"] = e.frames; d["fpsF"] = e.fpsF; d["fpsB"] = e.fpsB; d["snapshotReady"] = e.snapshotReady;
        String out; serializeJson(d, out); ws.broadcastTXT(out);
    }
}

static void addDtcEvent(const char *bus, const char *ecu, const char *code,
                        const char *source, const char *status) {
    /* Repeated 30-second scans must not resurrect an alarm the owner has
     * already acknowledged. Update the newest matching record while it is
     * unseen, or while the last report is within 120 s. A later recurrence
     * after a quiet gap creates a fresh current event. */
    DtcEvent *latest = nullptr;
    for (size_t i = eventCount; i > 0; --i) {
        DtcEvent &candidate = eventStore[i - 1u];
        if (strcmp(candidate.bus, bus) == 0 && strcmp(candidate.ecu, ecu) == 0 && strcmp(candidate.code, code) == 0) {
            latest = &candidate; break;
        }
    }
    if (latest && (!latest->seen ||
        (latest->lastReportedMs != 0u && (uint32_t)(millis() - latest->lastReportedMs) < 120000u))) {
        ++latest->repeats; strlcpy(latest->source, source, sizeof latest->source);
        strlcpy(latest->status, status, sizeof latest->status);
        const String at = eventUtcNow(); strlcpy(latest->lastAt, at.c_str(), sizeof latest->lastAt);
        latest->lastReportedMs = millis();
        if (!saveEvents()) Serial.println("EVENTS: update not persisted");
        broadcastEvents(); return;
    }
    if (eventCount == EVENT_MAX) {
        size_t victim = eventCount;
        for (size_t i = 0; i < eventCount; ++i) {
            if (eventStore[i].seen) { victim = i; break; } /* oldest acknowledged event first */
        }
        if (victim == eventCount) {
            Serial.println("EVENTS: journal full; all 40 events are still unacknowledged, preserving every alarm");
            JsonDocument d; d["type"] = "diag"; d["message"] = "EVENTS,ERR,40_UNACKNOWLEDGED_CAPACITY";
            String out; serializeJson(d, out); ws.broadcastTXT(out);
            return;
        }
        const uint32_t oldId = eventStore[victim].id;
        char oldPath[32];
        if (eventSnapshotPath(oldId, oldPath, sizeof oldPath)) LittleFS.remove(oldPath);
        for (size_t i = 0; i < pendingEventCount; ) {
            if (pendingEventCaptures[i].id == oldId)
                pendingEventCaptures[i] = pendingEventCaptures[--pendingEventCount];
            else ++i;
        }
        if (victim + 1u < eventCount)
            memmove(&eventStore[victim], &eventStore[victim + 1u], sizeof(DtcEvent) * (eventCount - victim - 1u));
        --eventCount;
    }
    DtcEvent &e = eventStore[eventCount++]; e = DtcEvent{};
    e.id = nextEventId++; strlcpy(e.code, code, sizeof e.code);
    strlcpy(e.source, source, sizeof e.source); strlcpy(e.bus, bus, sizeof e.bus);
    strlcpy(e.ecu, ecu, sizeof e.ecu); strlcpy(e.status, status, sizeof e.status);
    const String now = eventUtcNow(); strlcpy(e.timestamp, now.c_str(), sizeof e.timestamp);
    strlcpy(e.lastAt, now.c_str(), sizeof e.lastAt); e.lastReportedMs = millis();
    strlcpy(e.vehicleState, snap.key, sizeof e.vehicleState); e.rpm = snap.rpm;
    e.speed = snap.spd; strlcpy(e.rpmSource, snap.src, sizeof e.rpmSource);
    e.acc = snap.acc; strlcpy(e.pids, snap.pids, sizeof e.pids);
    e.frames = framesSeen; e.fpsF = busFps[0]; e.fpsB = busFps[1];
    e.snapshotReady = false; /* becomes true only after all 10 post-trigger samples persist */
    if (!startEventCapture(e.id))
        Serial.printf("EVENTS: pre/post snapshot unavailable for event %lu\n", (unsigned long)e.id);
    if (!saveEvents()) Serial.println("EVENTS: new DTC event not persisted");
    broadcastEvents();
}

static void captureDtcReply(const String &busField, const String &ecuField, const String &payload) {
    if ((payload.length() & 1u) || payload.length() < 4) return;
    for (size_t i = 0; i < payload.length(); ++i)
        if (!isxdigit((unsigned char)payload[i])) return;
    uint8_t b[512]; size_t n = 0;
    for (size_t i = 0; i + 1 < payload.length() && n < sizeof b; i += 2)
        b[n++] = (uint8_t)strtoul(payload.substring(i, i + 2).c_str(), nullptr, 16);
    if (!n) return;
    char bus[2] = { (busField == "B") ? 'B' : 'F', 0 };
    char ecu[9]; strlcpy(ecu, ecuField.c_str(), sizeof ecu);
    uint8_t sid = b[0];
    if (sid == 0x43 || sid == 0x47 || sid == 0x4A) {
        const char *source = sid == 0x43 ? "OBD stored" : (sid == 0x47 ? "OBD pending" : "OBD permanent");
        size_t i = 1;
        if (((n - 1) & 1u) != 0) i = 2; /* skip optional OBD DTC count byte */
        for (; i + 1 < n; i += 2) {
            if ((b[i] | b[i + 1]) == 0) continue;
            char code[8]; snprintf(code, sizeof code, "%c%u%1X%02X", "PCBU"[b[i] >> 6],
                                   (unsigned)((b[i] >> 4) & 3), (unsigned)(b[i] & 15), (unsigned)b[i + 1]);
            addDtcEvent(bus, ecu, code, source, "reported by ECU");
        }
    } else if (sid == 0x59 && n >= 7 && b[1] == 0x02) {
        for (size_t i = 3; i + 3 < n; i += 4) {
            if ((b[i] | b[i + 1] | b[i + 2]) == 0) continue;
            char code[12]; snprintf(code, sizeof code, "%c%u%1X%02X-%02X", "PCBU"[b[i] >> 6],
                                    (unsigned)((b[i] >> 4) & 3), (unsigned)(b[i] & 15),
                                    (unsigned)b[i + 1], (unsigned)b[i + 2]);
            char status[20]; snprintf(status, sizeof status, "status 0x%02X", (unsigned)b[i + 3]);
            addDtcEvent(bus, ecu, code, "UDS 19/02", status);
        }
    }
}

static void captureDtcChunk(const String &busField, const String &ecuField,
                            const String &offsetField, const String &chunk) {
    const uint8_t bus = (busField == "B") ? 1u : 0u;
    DtcResponseBuffer &r = dtcResponse[bus];
    if (chunk.isEmpty() || (chunk.length() & 1u) || chunk.length() > sizeof r.hex - 1u) { r.active = false; return; }
    for (size_t i = 0; i < chunk.length(); ++i) if (!isxdigit((unsigned char)chunk[i])) { r.active = false; return; }
    const long offsetValue = offsetField.toInt();
    if (offsetValue < 0 || offsetValue > 4095) { r.active = false; return; }
    const uint16_t offset = (uint16_t)offsetValue;
    if (offset == 0u) {
        r.hexLen = 0; r.nextOffset = 0; r.active = true;
        strlcpy(r.ecu, ecuField.c_str(), sizeof r.ecu); r.hex[0] = '\0';
    }
    if (!r.active || strcmp(r.ecu, ecuField.c_str()) != 0 || offset != r.nextOffset ||
        r.hexLen + chunk.length() >= sizeof r.hex) { r.active = false; return; }
    memcpy(r.hex + r.hexLen, chunk.c_str(), chunk.length());
    r.hexLen += chunk.length(); r.hex[r.hexLen] = '\0';
    r.nextOffset = (uint16_t)(r.nextOffset + chunk.length() / 2u);
}

static void completeDtcResponse(const String &busField, const String &ecuField) {
    const uint8_t bus = (busField == "B") ? 1u : 0u;
    DtcResponseBuffer &r = dtcResponse[bus];
    if (r.active && strcmp(r.ecu, ecuField.c_str()) == 0 && r.hexLen >= 4u) {
        captureDtcReply(busField, ecuField, String(r.hex));
    }
    r.active = false; r.hexLen = 0; r.nextOffset = 0; r.hex[0] = '\0';
}

static bool markEventSeen(uint32_t id) {
    for (size_t i = 0; i < eventCount; ++i) {
        if (eventStore[i].id != id) continue;
        if (eventStore[i].seen) return true;
        eventStore[i].seen = true;
        if (!saveEvents()) { eventStore[i].seen = false; return false; }
        broadcastEvents(); return true;
    }
    return false;
}

static void snapPid(const char *pid, float v) {        /* keep "05=88.0;0C=750.0;" current */
    String out; const size_t pl = strlen(pid);
    for (const char *p = snap.pids; *p; ) {              /* copy every other PID entry */
        const char *e = strchr(p, ';'); if (!e) break;
        if (!(strncmp(p, pid, pl) == 0 && p[pl] == '=')) out.concat(p, (unsigned)(e - p + 1));
        p = e + 1;
    }
    char kv[32]; snprintf(kv, sizeof kv, "%s=%.2f;", pid, (double)v); out += kv;
    if (out.length() < sizeof snap.pids) strlcpy(snap.pids, out.c_str(), sizeof snap.pids);
}

static void obsWrite(const char *s, size_t n) {
    if (obsCli && obsCli.connected()) obsCli.write((const uint8_t *)s, n);
}

static void logService() {
    if (WiFiClient c = obsSrv.available()) {             /* one observer; newest wins */
        if (obsCli) obsCli.stop();
        obsCli = c; obsCli.setNoDelay(true);
        obsCli.write((const uint8_t *)LOG_HDR, strlen(LOG_HDR));
    }
    if (obsCli) while (obsCli.available()) obsCli.read();   /* read-only: discard input */

    static uint32_t last = 0; const uint32_t now = millis();
    if (now - last < 1000) return;
    last = now;
    char utc[24] = "";
    time_t t = time(nullptr);
    if (t > 1700000000) { struct tm tmv; gmtime_r(&t, &tmv); strftime(utc, sizeof utc, "%Y-%m-%dT%H:%M:%SZ", &tmv); }
    int pedal = -1;
    for (int i = 0; i < ftabN; i++)
        if (ftab[i].bus == 0 && !ftab[i].ext && ftab[i].id == 0x17C && strlen(ftab[i].hex) >= 2) {
            char b[3] = { ftab[i].hex[0], ftab[i].hex[1], 0 }; pedal = (int)strtol(b, nullptr, 16); }
    char line[400];
    int n = snprintf(line, sizeof line, "%lu,%s,%s,%d,%.2f,%s,%d,%d,%u,%u,%s\n",
                     (unsigned long)now, utc, snap.key, snap.rpm, (double)snap.spd, snap.src, snap.acc,
                     pedal, (unsigned)busFps[0], (unsigned)busFps[1], snap.pids);
    if (n <= 0 || n >= (int)sizeof line) return;
    obsWrite(line, n);
    pushEventPreSample(line, (size_t)n);
    if (!logFsOk || stm32FlashRunning) return;
    capturePostEventSample(line, (size_t)n);
    File f = LittleFS.open(LOG_CUR, FILE_APPEND);
    if (!f) return;
    if (f.size() == 0) f.print(LOG_HDR);
    f.write((const uint8_t *)line, n);
    const size_t sz = f.size(); f.close();
    if (sz >= LOG_ROTATE_BYTES) { LittleFS.remove(LOG_OLD); LittleFS.rename(LOG_CUR, LOG_OLD); }
}

static void handleLogGet() {
    if (!requireLogin()) return;
    const bool old = http.hasArg("historical") || http.hasArg("old");
    if (!logFsOk || !LittleFS.exists(old ? LOG_OLD : LOG_CUR)) { http.send(404, "text/plain", "no log yet"); return; }
    File f = LittleFS.open(old ? LOG_OLD : LOG_CUR, FILE_READ);
    http.sendHeader("Content-Disposition", old ? "attachment; filename=civic_historical_data.csv" : "attachment; filename=civic_log.csv");
    http.streamFile(f, "text/csv"); f.close();
}
static void handleLogClear() {
    if (!requireLogin()) return;
    if (logFsOk) { LittleFS.remove(LOG_CUR); LittleFS.remove(LOG_OLD); }
    http.send(200, "text/plain", "cleared");
}
static void handleEventCsvGet() {
    if (!requireLogin() || !logFsOk || !http.hasArg("id")) { http.send(400, "text/plain", "event id required"); return; }
    const String arg = http.arg("id");
    if (!arg.length() || arg.length() > 10) { http.send(400, "text/plain", "invalid event id"); return; }
    for (size_t i = 0; i < arg.length(); ++i) if (!isDigit(arg[i])) { http.send(400, "text/plain", "invalid event id"); return; }
    const uint32_t id = (uint32_t)strtoul(arg.c_str(), nullptr, 10);
    DtcEvent *e = findEvent(id);
    char path[32];
    if (!e || !e->snapshotReady || !eventSnapshotPath(id, path, sizeof path) || !LittleFS.exists(path)) {
        http.send(404, "text/plain", "event snapshot not ready"); return;
    }
    File f = LittleFS.open(path, FILE_READ);
    if (!f) { http.send(500, "text/plain", "snapshot open failed"); return; }
    char filename[48]; snprintf(filename, sizeof filename, "attachment; filename=civic_event_%lu.csv", (unsigned long)id);
    http.sendHeader("Content-Disposition", filename);
    http.streamFile(f, "text/csv"); f.close();
}

/* ═══════════════════════════════════════════════════════════════════
   STATUS EMITTER
   ═══════════════════════════════════════════════════════════════════ */
static void emitStatus() {
    JsonDocument d;
    d["type"]    = "status";
    d["locked"]  = locked;
    d["bitrate"] = (int)bitrate;
    d["frames"]  = (int)framesSeen;
    d["dwell"]   = (int)dwellMs;
    String out; serializeJson(d, out); ws.broadcastTXT(out);
}

/* ═══════════════════════════════════════════════════════════════════
   LINE PARSER
   ═══════════════════════════════════════════════════════════════════ */
static void parseLine(const char *raw, int len) {
    if (len == 0) return;
    if (strncmp(raw, "TX,", 3) == 0) {
        JsonDocument d; d["type"]="tx"; d["message"]=raw;
        String out; serializeJson(d,out); ws.broadcastTXT(out); return;
    }
    if (strncmp(raw, "UDS,", 4) == 0 || strncmp(raw, "CMD,", 4) == 0 ||
        strncmp(raw, "ECU,", 4) == 0 || strncmp(raw, "START,", 6) == 0) {
        if (strncmp(raw, "UDS,", 4) == 0) {
            const String busField = csvField(raw, 1), eventField = csvField(raw, 2);
            if (eventField == "DATA")
                captureDtcChunk(busField, csvField(raw, 3), csvField(raw, 4), csvField(raw, 5));
            else if (eventField == "POS")
                completeDtcResponse(busField, csvField(raw, 3));
            else if (eventField == "NEG" || eventField == "ERR" || eventField == "TXERR") {
                const uint8_t bus = (busField == "B") ? 1u : 0u;
                dtcResponse[bus].active = false; dtcResponse[bus].hexLen = 0;
            }
        }
        JsonDocument d; d["type"]="diag"; d["message"]=raw;   /* ArduinoJson escapes */
        String out; serializeJson(d,out); ws.broadcastTXT(out); return;
    }
    /* New bus-tagged status: STATUS,SYS,... (global) or STATUS,<F|B>,KIND,... */
    if (strncmp(raw, "STATUS,", 7) == 0) {
        String f1 = csvField(raw, 1);   /* SYS / F / B */
        String f2 = csvField(raw, 2);   /* kind        */
        const bool isF = (f1 == "F");
        if (f1 == "SYS" && f2 == "VEH") {   /* STATUS,SYS,VEH,<st>,<rpm>,<spd*100>,<acc>,<src> */
            JsonDocument d; d["type"]="veh"; d["state"]=csvField(raw,3);
            d["rpm"]=csvField(raw,4).toInt(); d["spd"]=csvField(raw,5).toInt()/100.0;
            d["acc"]=csvField(raw,6).toInt(); d["src"]=csvField(raw,7);
            strlcpy(snap.key, csvField(raw,3).c_str(), sizeof snap.key);
            snap.rpm = csvField(raw,4).toInt(); snap.spd = csvField(raw,5).toInt()/100.0f;
            snap.acc = csvField(raw,6).toInt(); strlcpy(snap.src, csvField(raw,7).c_str(), sizeof snap.src);
            String out; serializeJson(d,out); ws.broadcastTXT(out); return;
        }
        if (f1 == "SYS") {
            JsonDocument d; d["type"]="diag"; d["message"]=raw;
            String out; serializeJson(d,out); ws.broadcastTXT(out); return;
        }
        if (f2 == "STATS") {
            JsonDocument d; d["type"]="stats"; d["bus"]=f1; d["message"]=raw + 15; /* past "STATUS,X,STATS," */
            String out; serializeJson(d,out); ws.broadcastTXT(out); return;
        }
        if (f2 == "LOCK")    { uint32_t hz=(uint32_t)csvField(raw,3).toInt(); if(isF){bitrate=hz;locked=true;emitStatus();} }
        else if (f2=="BITRATE"){ uint32_t hz=(uint32_t)csvField(raw,3).toInt(); if(isF){bitrate=hz;locked=true;emitStatus();} }
        else if (f2=="SCAN")   { uint32_t hz=(uint32_t)csvField(raw,3).toInt(); if(isF){bitrate=hz;locked=false;emitStatus();} }
        else if (f2=="REACQUIRE"){ if(isF){locked=false;bitrate=0;emitStatus();} }
        else if (f2=="DWELL")  { if(isF){dwellMs=(uint32_t)csvField(raw,3).toInt();emitStatus();} }
        /* ARMED / DISARMED / MODE and any other per-bus status -> diag log */
        JsonDocument d; d["type"]="diag"; d["message"]=raw;
        String out; serializeJson(d,out); ws.broadcastTXT(out); return;
    }

    int commas = countCommas(raw);

    if (strncmp(raw, "PROFILE,", 8) == 0 && commas >= 5) {
        String id=csvField(raw,1); id.toUpperCase();
        String vm=csvField(raw,4); vm.toUpperCase();
        int dlc = (int)csvField(raw,2).toInt();
        float rate = atof(csvField(raw,3).c_str()) / 10.0f;
        String lbl = csvField(raw,5);
        /* P1.6: static buffer */
        static char pbuf[256];
        int pn = snprintf(pbuf, sizeof pbuf,
            "{\"type\":\"profile\",\"id\":\"%s\",\"dlc\":%d,"
            "\"rate\":%.1f,\"var_mask\":\"%s\",\"label\":\"%s\"}",
            id.c_str(), dlc, (double)rate, vm.c_str(), lbl.c_str());
        if (pn > 0 && pn < (int)sizeof pbuf) ws.broadcastTXT(pbuf, pn);
        return;
    }
    if (strncmp(raw, "SIG,", 4) == 0 && commas >= 7) {
        String id=csvField(raw,1); id.toUpperCase();
        String l=csvField(raw,3);  l.toUpperCase();
        String mn=csvField(raw,4); mn.toUpperCase();
        String mx=csvField(raw,5); mx.toUpperCase();
        int    byt  = (int)csvField(raw,2).toInt();
        float  rate = atof(csvField(raw,6).c_str()) / 10.0f;
        String lbl  = csvField(raw,7);
        /* P1.6: static buffer */
        static char sbuf[256];
        int sn = snprintf(sbuf, sizeof sbuf,
            "{\"type\":\"sig\",\"id\":\"%s\",\"byte\":%d,"
            "\"last\":\"%s\",\"min\":\"%s\",\"max\":\"%s\","
            "\"rate\":%.1f,\"label\":\"%s\"}",
            id.c_str(), byt, l.c_str(), mn.c_str(), mx.c_str(),
            (double)rate, lbl.c_str());
        if (sn > 0 && sn < (int)sizeof sbuf) ws.broadcastTXT(sbuf, sn);
        return;
    }
    if (strncmp(raw, "PID,", 4) == 0 && commas >= 4) {
        String pid=csvField(raw,1); pid.toUpperCase();
        float val = atof(csvField(raw,4).c_str());
        String lbl = csvField(raw,2);
        String unt = csvField(raw,3);
        snapPid(pid.c_str(), val);
        /* P1.6: static buffer */
        static char pidbuf[256];
        int pn = snprintf(pidbuf, sizeof pidbuf,
            "{\"type\":\"pid\",\"pid\":\"%s\",\"label\":\"%s\","
            "\"unit\":\"%s\",\"value\":%.4f}",
            pid.c_str(), lbl.c_str(), unt.c_str(), (double)val);
        if (pn > 0 && pn < (int)sizeof pidbuf) ws.broadcastTXT(pidbuf, pn);
        return;
    }
    if (commas >= 5) {
        String ts=csvField(raw,0);
        String fmt=csvField(raw,1); fmt.toUpperCase();
        String id=csvField(raw,2);  id.toUpperCase();
        String dlcS=csvField(raw,3), rtrS=csvField(raw,4);
        String pay=csvField(raw,5); pay.toUpperCase();
        String busS=csvField(raw,6);                      /* 7th field: source bus */
        if(!ts.length()||!id.length()) return;
        if(fmt!="STD"&&fmt!="EXT") return;
        int dlc=dlcS.toInt(); if(dlc<0||dlc>8) return;
        int busN=busS.toInt(); if(busN<0||busN>1) busN=0;
        bool ext=(fmt=="EXT"), rtr=(rtrS.toInt()!=0);
        framesSeen++;
        /* No per-frame WebSocket message any more: at ~1000 frames/s that
         * blocked loop(), overflowed the STM32 UART buffer and spliced lines
         * (the "0xSIG"/"0x320G" rows). Frames are folded into a per-ID table
         * and pushed as one batch every FRAME_BATCH_MS (see frameBatchFlush). */
        frameTableUpdate((uint32_t)strtoul(id.c_str(), nullptr, 16), ext, (uint8_t)busN,
                         (uint8_t)dlc, rtr ? "" : pay.c_str());

        /* Stream to a connected SavvyCAN/RealDash GVRET-over-TCP client. */
        if (gvretCli && gvretCli.connected()) {
            uint8_t d2[8] = {0};
            if (!rtr) for (int i=0;i<dlc;i++) d2[i]=(uint8_t)strtoul(pay.substring(i*2,i*2+2).c_str(),nullptr,16);
            uint8_t gb[GVRET_MAX_FRAME_BYTES];
            size_t gn = gvret_encode_frame(gb, sizeof gb, (uint8_t)busN,
                           (uint32_t)strtoul(id.c_str(),nullptr,16), ext, rtr,
                           (uint8_t)dlc, d2, (uint32_t)strtoul(ts.c_str(),nullptr,10));
            if (gn) gvretCli.write(gb, gn);
        }
        return;
    }
}

/* ═══════════════════════════════════════════════════════════════════
   GVRET-over-TCP (SavvyCAN / RealDash via Wi-Fi, port 23)
   ═══════════════════════════════════════════════════════════════════ */
static void gvretWrite(const uint8_t *b, size_t n) { if (gvretCli && gvretCli.connected()) gvretCli.write(b, n); }

static void gvretService() {
    if (!gvretReady) return;
    if (!gvretCli || !gvretCli.connected()) {
        WiFiClient c = gvretSrv.available();
        if (c) { gvretCli = c; gvret_parser_init(&gvretParser); }
    }
    if (!gvretCli || !gvretCli.connected()) return;
    for (int budget = 512; budget > 0 && gvretCli.available(); budget--) {
        uint8_t byte = (uint8_t)gvretCli.read();
        struct gvret_command gc;
        if (!gvret_parser_feed(&gvretParser, byte, &gc)) continue;
        uint8_t rb[16]; size_t rn = 0;
        switch (gc.type) {
        case GVRET_CMD_KEEPALIVE:        rn = gvret_encode_keepalive_reply(rb, sizeof rb); gvretWrite(rb, rn); break;
        case GVRET_CMD_GET_DEVICE_INFO:  rn = gvret_encode_device_info(rb, sizeof rb, 0x0001); gvretWrite(rb, rn); break;
        case GVRET_CMD_GET_NUMBUSES:     rn = gvret_encode_numbuses(rb, sizeof rb, 2); gvretWrite(rb, rn); break;
        case GVRET_CMD_TIME_SYNC:        rn = gvret_encode_time_sync(rb, sizeof rb, micros()); gvretWrite(rb, rn); break;
        case GVRET_CMD_GET_CANBUS_PARAMS: {
            uint8_t pb[16]; size_t pn = gvret_encode_canbus_params(pb, sizeof pb, true, bitrate, true, 0);
            gvretWrite(pb, pn); break;
        }
        case GVRET_CMD_SEND_FRAME: {
            /* SavvyCAN asked to transmit: map GVRET bus 0/1 -> F/B, emit a TX
             * command (honoured only while that bus is ARMed). */
            char tb[64];
            const char *b = (gc.u.frame.bus == 1) ? "B" : "F";
            if (gc.u.frame.dlc == 0) {
                snprintf(tb, sizeof tb, "TX,%s,%s,%lX,0,0,-\n", b, gc.u.frame.ext?"E":"S",
                         (unsigned long)gc.u.frame.id);
            } else {
                char hx[17]; for (int i=0;i<gc.u.frame.dlc;i++) snprintf(hx+i*2,3,"%02X",gc.u.frame.data[i]);
                snprintf(tb, sizeof tb, "TX,%s,%s,%lX,0,%u,%s\n", b, gc.u.frame.ext?"E":"S",
                         (unsigned long)gc.u.frame.id, gc.u.frame.dlc, hx);
            }
            Stm32.print(tb); break;
        }
        default: break;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════
   WS EVENT + HTTP
   ═══════════════════════════════════════════════════════════════════ */
static bool isHexStr(const String &v, size_t minLen, size_t maxLen) {
    if (v.length() < minLen || v.length() > maxLen) return false;
    for (size_t i = 0; i < v.length(); i++) if (!isxdigit((unsigned char)v[i])) return false;
    return true;
}

static void wsReply(uint8_t num, const char *type, const char *msg) {
    JsonDocument e; e["type"]=type; e["message"]=msg;
    String out; serializeJson(e,out); ws.sendTXT(num, out);
}

static void handleWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    if (num >= WEBSOCKETS_SERVER_CLIENT_MAX) return;
    /* No web login (owner request): Wi-Fi WPA2 is the only gate, so every
     * connected WebSocket client is a trusted operator. */
    if (type == WStype_CONNECTED)    { wsAuthed[num] = true; emitStatus(); sendAllEvents(num); return; }
    if (type == WStype_DISCONNECTED) {
        wsAuthed[num] = false;
        bool any = false;
        for (int i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) any = any || wsAuthed[i];
        /* No operator left: end any ARM window on both buses (the STM32 ignores
         * DISARM when it is not armed). */
        if (!any && !stm32FlashRunning) { Stm32.print("DISARM,F\n"); Stm32.print("DISARM,B\n"); }
        return;
    }
    if (type != WStype_TEXT || length == 0 || length > 1400) return;
    JsonDocument cmd;
    if (deserializeJson(cmd, payload, length)) return;
    const char *op = cmd["op"] | "";
    String busS(cmd["bus"] | "F");
    const char *bus = (busS == "B" || busS == "1") ? "B" : "F";

    char buf[260];
    if (strcmp(op, "events_get") == 0) { sendAllEvents(num); return; }
    if (strcmp(op, "event_seen") == 0) {
        const uint32_t id = cmd["id"] | 0UL;
        if (!id || !markEventSeen(id)) wsReply(num, "diag", "EVENT,ERR,NOT_FOUND_OR_SAVE_FAILED");
        return;
    }
    if (strcmp(op, "arm") == 0) {
        int secs = cmd["seconds"] | 0;
        if (secs < 1 || secs > 600) { wsReply(num, "diag", "CMD,ERR,ESP32_BAD_ARM"); return; }
        snprintf(buf, sizeof buf, "ARM,%s,%d\n", bus, secs);
        Stm32.print(buf); return;
    }
    if (strcmp(op, "disarm") == 0) { snprintf(buf, sizeof buf, "DISARM,%s\n", bus); Stm32.print(buf); return; }
    if (strcmp(op, "mode") == 0) {
        String m(cmd["mode"] | ""); m.toUpperCase();
        if (m!="SILENT"&&m!="NORMAL"&&m!="LOOPBACK"&&m!="LOOPBACKSILENT") { wsReply(num,"diag","CMD,ERR,BAD_MODE"); return; }
        snprintf(buf, sizeof buf, "MODE,%s,%s\n", bus, m.c_str()); Stm32.print(buf); return;
    }
    if (strcmp(op, "bitrate") == 0) {
        String v(cmd["value"] | ""); v.toUpperCase();
        if (v != "AUTO") { long hz = v.toInt(); if (hz < 1 || hz > 1000000) { wsReply(num,"diag","CMD,ERR,BAD_BITRATE"); return; } }
        snprintf(buf, sizeof buf, "BITRATE,%s,%s\n", bus, v.c_str()); Stm32.print(buf); return;
    }
    if (strcmp(op, "gateway") == 0) {
        bool on = cmd["on"] | false;
        snprintf(buf, sizeof buf, "GATEWAY,%s\n", on ? "ON" : "OFF"); Stm32.print(buf); return;
    }
    if (strcmp(op, "periodic") == 0) {
        int slot = cmd["slot"] | -1;
        if (slot < 0 || slot > 7) { wsReply(num,"diag","CMD,ERR,BAD_SLOT"); return; }
        if (cmd["off"] | false) { snprintf(buf, sizeof buf, "PERIODIC,%d,OFF\n", slot); Stm32.print(buf); return; }
        String fmt(cmd["format"] | ""); fmt.toUpperCase();
        String id(cmd["id"] | ""); id.toUpperCase();
        String data(cmd["data"] | ""); data.toUpperCase();
        int dlc = cmd["dlc"] | -1, per = cmd["period"] | 0;
        uint32_t maxId = (fmt == "S") ? 0x7FFu : (fmt == "E" ? 0x1FFFFFFFu : 0u);
        bool ok = (fmt=="S"||fmt=="E") && isHexStr(id,1,8) && dlc>=0 && dlc<=8 && per>=1 && per<=60000 &&
                  strtoul(id.c_str(),nullptr,16) <= maxId;
        if (ok && dlc > 0) ok = (data.length()==(size_t)(dlc*2)) && isHexStr(data,2,16);
        if (!ok) { wsReply(num,"diag","CMD,ERR,BAD_PERIODIC"); return; }
        snprintf(buf, sizeof buf, "PERIODIC,%d,%s,%s,%s,%d,%s,%d\n", slot, bus, fmt.c_str(), id.c_str(),
                 dlc, (dlc==0)?"-":data.c_str(), per);
        Stm32.print(buf); return;
    }
    if (strcmp(op, "start") == 0)        { Stm32.print("START\n"); return; }
    if (strcmp(op, "start_cancel") == 0) { Stm32.print("STARTCANCEL\n"); return; }
    /* Decoded transmission gear -> STM32 LD4/LD3/LD5/LD6 shift chase. LED-only
     * on the STM32 side (never touches a bus); value range 0..15 enforced here
     * and again on the STM32 (GEAR,ERR,BAD_GEAR). */
    if (strcmp(op, "gear") == 0) {
        int g = cmd["gear"] | -1;
        if (g < 0 || g > 15) { wsReply(num,"diag","GEAR,ERR,BAD_GEAR"); return; }
        snprintf(buf, sizeof buf, "GEAR,%d\n", g); Stm32.print(buf); return;
    }
    if (strcmp(op, "gear_clear") == 0)   { Stm32.print("GEAR,CLEAR\n"); return; }
    if (strcmp(op, "startcfg") == 0) {
        int to=cmd["timeout"]|2000, ra=cmd["armed"]|1, rn=cmd["neutral"]|1, rb=cmd["brake"]|1, ri=cmd["immob"]|1, rp=cmd["rpm"]|400;
        if (to<500) to=500; if (to>10000) to=10000; if (rp<1) rp=400;
        snprintf(buf, sizeof buf, "STARTCFG,%d,%d,%d,%d,%d,%d\n", to, ra?1:0, rn?1:0, rb?1:0, ri?1:0, rp);
        Stm32.print(buf); return;
    }
    if (strcmp(op, "startinputs") == 0) {
        int n=cmd["neutral"]|0, b=cmd["brake"]|0, i=cmd["immob"]|0;
        snprintf(buf, sizeof buf, "STARTINPUTS,%d,%d,%d\n", n?1:0, b?1:0, i?1:0); Stm32.print(buf); return;
    }
    if (strcmp(op, "startframe") == 0) {
        if (cmd["none"] | false) { Stm32.print("STARTFRAME,NONE\n"); return; }
        String fmt(cmd["format"] | ""); fmt.toUpperCase();
        String id(cmd["id"] | ""); id.toUpperCase();
        String data(cmd["data"] | ""); data.toUpperCase();
        int dlc = cmd["dlc"] | -1;
        uint32_t maxId = (fmt == "S") ? 0x7FFu : (fmt == "E" ? 0x1FFFFFFFu : 0u);
        bool ok = (fmt=="S"||fmt=="E") && isHexStr(id,1,8) && dlc>=0 && dlc<=8 &&
                  strtoul(id.c_str(),nullptr,16) <= maxId && data.length()==(size_t)(dlc*2);
        if (ok && dlc > 0) ok = isHexStr(data,2,16);
        if (!ok) { wsReply(num,"diag","CMD,ERR,BAD_FRAME"); return; }
        snprintf(buf, sizeof buf, "STARTFRAME,%s,%s,%s,%d,%s\n", bus, fmt.c_str(), id.c_str(), dlc, data.c_str());
        Stm32.print(buf); return;
    }
    if (strcmp(op, "uds_abort") == 0) { snprintf(buf, sizeof buf, "UDS,%s,ABORT\n", bus); Stm32.print(buf); return; }
    if (strcmp(op, "ecuscan_abort") == 0) { snprintf(buf, sizeof buf, "ECUSCAN,%s,ABORT\n", bus); Stm32.print(buf); return; }
    if (strcmp(op, "ecuscan") == 0) {
        String fmt(cmd["format"] | ""); fmt.toUpperCase();
        String first(cmd["first"] | ""); first.toUpperCase();
        String last(cmd["last"] | "");   last.toUpperCase();
        String req(cmd["req"] | "");     req.toUpperCase();
        int wait = cmd["wait"] | 60;
        const uint32_t maxT = (fmt == "S") ? 0x7F7u : (fmt == "E" ? 0xFFu : 0u);
        bool ok = (fmt == "S" || fmt == "E") && isHexStr(first, 1, 3) && isHexStr(last, 1, 3) &&
                  strtoul(first.c_str(), nullptr, 16) <= maxT && strtoul(last.c_str(), nullptr, 16) <= maxT &&
                  isHexStr(req, 2, 14) && (req.length() % 2) == 0 && wait >= 1 && wait <= 1000;
        if (!ok) { wsReply(num, "diag", "ECU,ERR,ESP32_VALIDATION_FAILED"); return; }
        snprintf(buf, sizeof buf, "ECUSCAN,%s,%s,%s,%s,%s,%d\n", bus, fmt.c_str(), first.c_str(), last.c_str(), req.c_str(), wait);
        Stm32.print(buf); return;
    }
    if (strcmp(op, "udspad") == 0) {
        String v(cmd["value"] | ""); v.toUpperCase();
        if (v != "NONE" && !isHexStr(v, 2, 2)) { wsReply(num, "diag", "CMD,ERR,ESP32_BAD_PAD"); return; }
        snprintf(buf, sizeof buf, "UDSPAD,%s,%s\n", bus, v.c_str());
        Stm32.print(buf); return;
    }
    if (strcmp(op, "uds") == 0) {
        String fmt(cmd["format"] | ""); fmt.toUpperCase();
        String tx(cmd["tx"] | "");  tx.toUpperCase();
        String rx(cmd["rx"] | "");  rx.toUpperCase();
        String req(cmd["req"] | ""); req.toUpperCase();
        bool confirm = cmd["confirm"] | false;
        uint32_t maxId = (fmt == "S") ? 0x7FFu : (fmt == "E" ? 0x1FFFFFFFu : 0u);
        /* req up to 512 bytes (1024 hex) so RequestDownload/TransferData blocks
         * fit; sent piecewise to avoid a huge stack buffer. */
        bool ok = (fmt == "S" || fmt == "E") && isHexStr(tx, 1, 8) && isHexStr(rx, 1, 8) &&
                  strtoul(tx.c_str(), nullptr, 16) <= maxId && strtoul(rx.c_str(), nullptr, 16) <= maxId &&
                  isHexStr(req, 2, 1024) && (req.length() % 2) == 0;
        if (!ok) { wsReply(num, "diag", "UDS,ERR,ESP32_VALIDATION_FAILED"); return; }
        Stm32.print("UDS,"); Stm32.print(bus); Stm32.print(","); Stm32.print(fmt);
        Stm32.print(","); Stm32.print(tx); Stm32.print(","); Stm32.print(rx);
        Stm32.print(","); Stm32.print(req); if (confirm) Stm32.print(",CONFIRM");
        Stm32.print("\n"); return;
    }
    if (strcmp(op, "tx") != 0) return;
    String fmt(cmd["format"]|""); fmt.toUpperCase();
    String id(cmd["id"]|"");     id.toUpperCase();
    String data(cmd["data"]|""); data.toUpperCase();
    int dlc=cmd["dlc"]|-1, rtr=cmd["rtr"]|-1;
    uint32_t maxId=(fmt=="S")?0x7FFu:(fmt=="E"?0x1FFFFFFFu:0u);
    bool ok=(fmt=="S"||fmt=="E")&&isHexStr(id,1,8)&&dlc>=0&&dlc<=8
            &&(rtr==0||rtr==1)&&strtoul(id.c_str(),nullptr,16)<=maxId;
    if(ok&&rtr==0) ok=(data.length()==(size_t)(dlc*2))&&(dlc==0||isHexStr(data,2,16));
    if(!ok){ wsReply(num, "tx", "TX,ERR,ESP32_VALIDATION_FAILED"); return; }
    /* "-" = no payload (remote frame, or data frame with DLC 0): an empty
     * field would be dropped by the STM32 tokenizer. */
    snprintf(buf,sizeof buf,"TX,%s,%s,%s,%d,%d,%s\n",bus,fmt.c_str(),id.c_str(),rtr,dlc,(rtr||dlc==0)?"-":data.c_str());
    Stm32.print(buf);
}
static void handleRoot()     { if (!requireLogin()) return; http.send_P(200,"text/html",PAGE); }
static void handleToken()    { if (!requireLogin()) return; http.sendHeader("Cache-Control","no-store"); http.send(200,"text/plain",wsToken); }
static void handleNotFound() { http.send(404,"text/plain","Not found"); }

/* ═══════════════════════════════════════════════════════════════════
   ESP32 OTA — receives .bin via HTTP POST /update
   ═══════════════════════════════════════════════════════════════════ */
static bool espOtaAuthed = false, espOtaFailed = false;

static void handleESP32Update() {
    /* Called once after the multipart upload completes */
    if (!requireLogin()) return;
    if (!espOtaAuthed || espOtaFailed || Update.hasError() || !Update.isFinished()) {
        String why = Update.hasError() ? String(Update.errorString()) : String("upload incomplete");
        http.send(500, "text/plain", "Flash failed: " + why);
        return;
    }
    http.send(200, "text/plain", "OK — image verified, rebooting");
    delay(200);
    ESP.restart();
}

static void handleESP32UpdateUpload() {
    HTTPUpload &upload = http.upload();
    if (upload.status == UPLOAD_FILE_START) {
        espOtaAuthed = true;                 /* no web login (owner request) */
        espOtaFailed = !espOtaAuthed;
        if (espOtaAuthed && !Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) espOtaFailed = true;
        Serial.printf("[OTA-ESP32] Start %s auth=%d\n", upload.filename.c_str(), espOtaAuthed);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (!espOtaFailed && Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            espOtaFailed = true;
    } else if (upload.status == UPLOAD_FILE_END) {
        /* end(true) finalises; the IDF OTA layer validates the image first */
        if (!espOtaFailed && !Update.end(true)) espOtaFailed = true;
        Serial.printf("[OTA-ESP32] End %u bytes failed=%d\n", upload.totalSize, espOtaFailed);
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (espOtaAuthed) Update.abort();
        espOtaFailed = true;
    }
}

/* ═══════════════════════════════════════════════════════════════════
   STM32 UART bootloader flash — receives .bin via HTTP POST /update-stm32
   Implements ST AN3155 UART bootloader protocol manually.
   ═══════════════════════════════════════════════════════════════════ */

/* STM32 bootloader commands */
static constexpr uint8_t BL_GET       = 0x00;
static constexpr uint8_t BL_GET_VER   = 0x01;
/* STM32F407 ROM bootloader requires Extended Erase (0x44), not Standard (0x43).
 * AN3155 §3.3: Standard Erase is NOT supported on STM32F2/F4/F7/L4/H7 devices.
 * The payload (0xFF,0xFF = global mass erase + checksum 0x00) is correct for
 * Extended Erase.  Only this command byte needed to change. */
static constexpr uint8_t BL_ERASE     = 0x44; /* Extended Erase — F407 required */
static constexpr uint8_t BL_WRITE_MEM = 0x31;
static constexpr uint8_t BL_READ_MEM  = 0x11;
static constexpr uint8_t BL_GO        = 0x21;
static constexpr uint8_t BL_ACK       = 0x79;
static constexpr uint8_t BL_NACK      = 0x1F;
static constexpr uint32_t BL_TIMEOUT_MS = 5000;

static constexpr size_t STM32_BIN_MAX = 160u * 1024u; /* ESP32 heap limit; STM32 image is ~46 KB */
static uint8_t *stm32BinBuf     = nullptr;
static size_t   stm32BinAlloc   = 0;   /* actual allocated size */
static size_t   stm32BinSize    = 0;   /* bytes received so far */

static bool blWaitAck(uint32_t timeoutMs = BL_TIMEOUT_MS) {
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        if (Stm32.available()) {
            uint8_t b = Stm32.read();
            if (b == BL_ACK) return true;
            if (b == BL_NACK) { Serial.println("[BL] NACK"); return false; }
        }
        delay(1);
        esp_task_wdt_reset(); /* feed WDT — flash takes up to ~60 s */
    }
    Serial.println("[BL] Timeout waiting for ACK");
    return false;
}

static bool blSendCmd(uint8_t cmd) {
    Stm32.write(cmd);
    Stm32.write((uint8_t)(cmd ^ 0xFF)); /* checksum = XOR complement */
    return blWaitAck();
}

static bool blSendAddr(uint32_t addr) {
    uint8_t b[5];
    b[0] = (addr >> 24) & 0xFF;
    b[1] = (addr >> 16) & 0xFF;
    b[2] = (addr >>  8) & 0xFF;
    b[3] = (addr      ) & 0xFF;
    b[4] = b[0] ^ b[1] ^ b[2] ^ b[3];
    Stm32.write(b, 5);
    return blWaitAck();
}

/* Enter ROM system-memory boot: BOOT0=HIGH, PB2/BOOT1 low via MB997 SB19. */
static bool blEnter() {
    Serial.println("[BL] Entering bootloader mode on USART3 PB10/PB11");
    Stm32.end();
    pinMode(STM32_RX, INPUT);
    pinMode(STM32_TX, INPUT);
    delay(10);
    /* SB19 ON is a physical prerequisite; there is no ESP32 PB2 control. */
    digitalWrite(STM32_BOOT0, HIGH);
    delay(10);
    digitalWrite(STM32_NRST, LOW);
    delay(10);
    digitalWrite(STM32_NRST, HIGH); /* open-drain released */
    delay(100);
    Stm32.begin(57600, SERIAL_8E1, STM32_RX, STM32_TX);
    delay(50);
    Stm32.write((uint8_t)0x7F);
    const bool ok = blWaitAck(3000);
    Serial.printf("[BL] Sync %s\n", ok ? "ACK" : "FAILED");
    return ok;
}

static bool blReadBytes(uint8_t *out, size_t n, uint32_t timeoutMs) {
    uint32_t start = millis();
    size_t got = 0;
    while (got < n && millis() - start < timeoutMs) {
        if (Stm32.available()) out[got++] = (uint8_t)Stm32.read();
        else { delay(1); esp_task_wdt_reset(); }
    }
    return got == n;
}

/* AN3155 Read Memory: cmd 0x11, address + XOR, N-1 + complement, then N bytes. */
static bool blReadMem(uint32_t addr, uint8_t *out, size_t n) {
    if (n == 0 || n > 256) return false;
    if (!blSendCmd(BL_READ_MEM)) return false;
    if (!blSendAddr(addr)) return false;
    Stm32.write((uint8_t)(n - 1));
    Stm32.write((uint8_t)((n - 1) ^ 0xFF));
    if (!blWaitAck()) return false;
    return blReadBytes(out, n, BL_TIMEOUT_MS);
}

/* Checks the image's vector table against this project's linker script
 * (STM32F407VGTX_FLASH.ld: ROM 0x08000000 1024K, RAM 0x20000000 128K,
 * CCMRAM 0x10000000 64K) before anything is erased. */
static String validateStm32Image(const uint8_t *bin, size_t size) {
    if (size < 8 || size > 1024u * 1024u) return "image size out of range";
    const uint32_t sp = bin[0] | (bin[1] << 8) | (bin[2] << 16) | ((uint32_t)bin[3] << 24);
    const uint32_t rv = bin[4] | (bin[5] << 8) | (bin[6] << 16) | ((uint32_t)bin[7] << 24);
    const bool spOk = (sp > 0x20000000u && sp <= 0x20020000u) || (sp > 0x10000000u && sp <= 0x10010000u);
    const bool rvOk = (rv & 1u) && (rv & ~1u) >= 0x08000000u && (rv & ~1u) < 0x08000000u + size;
    if (!spOk) return "initial stack pointer not in STM32 RAM — not an STM32F407 image";
    if (!rvOk) return "reset vector not inside the image — not a flash-linked .bin";
    return "";
}

/* Exit bootloader: BOOT0=LOW, pulse NRST, restore app USART3 8N1. */
static void blExit() {
    Serial.println("[BL] Exiting bootloader, resetting STM32");
    Stm32.end();
    pinMode(STM32_RX, INPUT);
    pinMode(STM32_TX, INPUT);
    delay(10);
    digitalWrite(STM32_BOOT0, LOW);
    digitalWrite(STM32_NRST, LOW);
    delay(10);
    digitalWrite(STM32_NRST, HIGH);
    delay(200);
    Stm32.begin(STM32_BAUD, SERIAL_8N1, STM32_RX, STM32_TX);
    lineLen = 0; lineDiscard = false;   /* drop any partial pre-flash line */
    Serial.println("[BL] STM32 running application firmware");
}

/* Notify dashboard clients of STM32 flash progress */
static void blProgress(int pct, const char *msg) {
    JsonDocument d;
    d["type"] = "stm32_progress";
    d["pct"]  = pct;
    d["msg"]  = msg;
    String out; serializeJson(d, out);
    ws.broadcastTXT(out);
    Serial.printf("[BL] %d%% %s\n", pct, msg);
}

/* Full STM32 flash sequence */
static String doFlashSTM32(const uint8_t *bin, size_t size) {
    const uint32_t FLASH_START = 0x08000000;
    const size_t   PAGE_SIZE   = 256; /* STM32 bootloader write page */

    const String bad = validateStm32Image(bin, size);
    if (bad.length()) return "Rejected before erase: " + bad;
    if (!blEnter()) {
        blExit();
        return "STM32 ROM bootloader did not ACK 0x7F; no erase was sent. Check BOOT0/NRST, fit MB997 SB19 (BOOT1 low), and verify PB10/PB11 wiring";
    }
    blProgress(2, "Bootloader initialised");

    /* Mass erase */
    if (!blSendCmd(BL_ERASE)) { blExit(); return "Mass erase command failed"; }
    /* Extended erase: 0xFF 0xFF = global mass erase */
    Stm32.write((uint8_t)0xFF);
    Stm32.write((uint8_t)0xFF);
    Stm32.write((uint8_t)0x00); /* checksum 0xFF^0xFF = 0x00 */
    if (!blWaitAck(BL_MASS_ERASE_TIMEOUT_MS)) { blExit(); return "Mass erase timeout"; }
    blProgress(10, "Flash erased");

    /* Write pages */
    size_t written = 0;
    while (written < size) {
        uint32_t addr = FLASH_START + written;
        size_t chunk = (size - written < PAGE_SIZE) ? (size - written) : PAGE_SIZE;
        /* Pad to multiple of 4 */
        size_t padded = (chunk + 3) & ~3u;

        if (!blSendCmd(BL_WRITE_MEM)) { blExit(); return "Write command failed at " + String(addr, HEX); }
        if (!blSendAddr(addr))         { blExit(); return "Address failed at " + String(addr, HEX); }

        /* Send: N-1, data[0..N-1], checksum */
        uint8_t ck = (uint8_t)(padded - 1);
        Stm32.write((uint8_t)(padded - 1));
        for (size_t i = 0; i < padded; i++) {
            uint8_t b = (i < chunk) ? bin[written + i] : 0xFF;
            Stm32.write(b);
            ck ^= b;
        }
        Stm32.write(ck);
        if (!blWaitAck(5000)) { blExit(); return "Write ACK failed at " + String(addr, HEX); }

        written += chunk;
        int pct = 10 + (int)((written * 80) / size);
        if (written % 4096 < PAGE_SIZE) { /* report every ~4KB */
            blProgress(pct, ("Writing... " + String(written / 1024) + "/" + String(size / 1024) + " KB").c_str());
        }
    }
    blProgress(90, "Write complete, verifying");
    static uint8_t rb[256];
    for (size_t off = 0; off < size; off += sizeof rb) {
        const size_t n = (size - off < sizeof rb) ? (size - off) : sizeof rb;
        if (!blReadMem(FLASH_START + off, rb, n)) { blExit(); return "Read-back failed at " + String(FLASH_START + off, HEX); }
        if (memcmp(rb, bin + off, n) != 0)        { blExit(); return "Verify mismatch at " + String(FLASH_START + off, HEX); }
    }
    blProgress(98, "Verified, starting application");

    /* GO command — both command and address ACKs are required for success. */
    if (!blSendCmd(BL_GO)) {
        blExit();
        return "Image read-back verified, but ROM GO command was not ACKed";
    }
    if (!blSendAddr(FLASH_START)) {
        blExit();
        return "Image read-back verified, but ROM GO address was not ACKed";
    }

    blExit();
    blProgress(100, "STM32 flashed successfully");
    return "OK — STM32 flashed " + String(size) + " bytes";
}

/* HTTP handler: receive .bin, flash STM32 */
static bool stm32OtaAuthed = false;

static void handleSTM32UpdateUpload() {
    HTTPUpload &upload = http.upload();
    if (upload.status == UPLOAD_FILE_START) {
        stm32OtaAuthed = true;               /* no web login (owner request) */
        if (stm32BinBuf) { free(stm32BinBuf); stm32BinBuf = nullptr; }
        stm32BinSize = 0; stm32BinAlloc = 0;
        if (!stm32OtaAuthed || stm32FlashRunning) return;
        /* One contiguous block: never more than the largest free block minus
         * a 24 KB reserve for Wi-Fi/LWIP (ESP32 data RAM is 328 KB, TRM 3.3.2). */
        const size_t big = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
        size_t want = (big > 24u * 1024u) ? big - 24u * 1024u : 0u;
        if (want > STM32_BIN_MAX) want = STM32_BIN_MAX;
        stm32BinBuf = want ? (uint8_t*)malloc(want) : nullptr;
        if (stm32BinBuf) stm32BinAlloc = want;
        else Serial.printf("[OTA-STM32] malloc %u bytes failed (largest free %u)\n", (unsigned)want, (unsigned)big);
        Serial.printf("[OTA-STM32] buffer %u bytes\n", (unsigned)stm32BinAlloc);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (!stm32BinBuf) return;
        if (stm32BinSize + upload.currentSize > stm32BinAlloc) {
            Serial.println("[OTA-STM32] Binary larger than the ESP32 buffer — aborting");
            free(stm32BinBuf); stm32BinBuf = nullptr; stm32BinSize = stm32BinAlloc = 0;
            return;
        }
        memcpy(stm32BinBuf + stm32BinSize, upload.buf, upload.currentSize);
        stm32BinSize += upload.currentSize;
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (stm32BinBuf) { free(stm32BinBuf); stm32BinBuf = nullptr; }
        stm32BinSize = stm32BinAlloc = 0;
    }
}

static void handleSTM32Update() {
    if (!requireLogin()) return;
    if (!stm32OtaAuthed) { http.send(401, "text/plain", "Login required"); return; }
    if (!stm32BinBuf || stm32BinSize == 0) {
        http.send(400, "text/plain", "No binary received (or larger than 160 KB / out of memory)");
        return;
    }
    if (stm32FlashRunning) {
        http.send(503, "text/plain", "Flash already in progress");
        return;
    }
    stm32FlashRunning = true;
    /* Send HTTP response first so browser doesn't time out */
    String result = doFlashSTM32(stm32BinBuf, stm32BinSize);
    stm32FlashRunning = false;
    free(stm32BinBuf);
    stm32BinBuf  = nullptr;
    stm32BinSize = 0;
    if (result.startsWith("OK")) {
        http.send(200, "text/plain", result);
    } else {
        http.send(500, "text/plain", result);
    }
}

/* ═══════════════════════════════════════════════════════════════════
   SETUP / LOOP
   ═══════════════════════════════════════════════════════════════════ */
void setup() {
    Serial.begin(115200); delay(200);
    Serial.println("\n=== Civic CAN Dashboard ===");

    Serial.printf("BOOT_DIAG reset_reason=%d\n", (int)esp_reset_reason());
    /* Keep the default 240 MHz clock: lowering it before RF init also lowers
     * the digital core voltage and made WiFi.mode() trip the brownout reset. */
    Serial.printf("WIFI_DIAG 0: CPU clock=%u MHz\n", getCpuFrequencyMhz());
    Serial.println("WIFI_DIAG 1: selecting AP-only mode");
    const bool modeOk = WiFi.mode(WIFI_AP);
    Serial.printf("WIFI_DIAG 2: WiFi.mode(AP)=%d\n", (int)modeOk);
    delay(100);
    loadDevicePass();
    Serial.println("WIFI_DIAG 3: password loaded; no STA scan or reconnect");
    Serial.println("WIFI_DIAG 4: starting access point");
    const bool ok = WiFi.softAP(AP_SSID,devicePass.c_str(),AP_CHANNEL,0,AP_MAX_CONN);
    Serial.printf("WIFI_DIAG 5: softAP=%d\n", (int)ok);
    if (ok) {
        const bool txOk = WiFi.setTxPower(WIFI_POWER_8_5dBm);
        Serial.printf("WIFI_DIAG 6: TX power cap=%d\n", (int)txOk);
    }
    wsToken = randomSecret(24); /* esp_random only after RF/AP is active */
    Serial.printf("Wi-Fi AP '%s' %s IP=%s\n",AP_SSID,ok?"UP":"FAILED",WiFi.softAPIP().toString().c_str());

    /* Initialize STM32 control pins and 2 Mbaud UART only after the ESP32 AP
     * is running; this isolates Wi-Fi startup from external-board I/O loads. */
    Serial.println("STM32: initializing control pins and UART");
    pinMode(STM32_BOOT0, OUTPUT); digitalWrite(STM32_BOOT0, LOW);
    digitalWrite(STM32_NRST, HIGH);
    pinMode(STM32_NRST, OUTPUT_OPEN_DRAIN); digitalWrite(STM32_NRST, HIGH);
    Stm32.setRxBufferSize(16384);
    Stm32.begin(STM32_BAUD, SERIAL_8N1, STM32_RX, STM32_TX);
    Serial.printf("STM32 USART3 app UART: RX=GPIO%d TX=GPIO%d baud=%u 8N1\n",
                  STM32_RX, STM32_TX, STM32_BAUD);
    Serial.printf("STM32 boot control: BOOT0=GPIO%d NRST=GPIO%d; fit MB997 SB19 (BOOT1 low)\n",
                  STM32_BOOT0, STM32_NRST);

    http.on("/",          HTTP_GET,  handleRoot);
    http.on("/token",     HTTP_GET,  handleToken);
    http.on("/update",    HTTP_POST, handleESP32Update,    handleESP32UpdateUpload);
    http.on("/update-stm32", HTTP_POST, handleSTM32Update, handleSTM32UpdateUpload);
    http.on("/log.csv",   HTTP_GET,  handleLogGet);       /* ?old=1 for the rotated file */
    http.on("/log/clear", HTTP_POST, handleLogClear);
    http.on("/event.csv", HTTP_GET,  handleEventCsvGet);
    http.onNotFound(handleNotFound);
    http.begin();

    ws.begin(); ws.onEvent(handleWsEvent);
    ws.enableHeartbeat(15000, 3000, 2);  /* lost client -> disconnect -> DISARM */

    gvretSrv.begin();                    /* SavvyCAN / RealDash over Wi-Fi (TCP 23) */
    gvretSrv.setNoDelay(true);
    gvretReady = true;
    logFsOk = LittleFS.begin(false);     /* fail closed: never erase an existing log/event journal */
    Serial.printf("LittleFS log %s\n", logFsOk ? "ready" : "UNAVAILABLE (not formatted; contents preserved)");
    if (logFsOk) loadEvents();
    configTime(0, 0, "pool.ntp.org", "time.google.com");   /* UTC stamps once STA has internet */
    obsSrv.begin(); obsSrv.setNoDelay(true);               /* read-only observer, TCP 2323 */
    Serial.println("HTTP:80  WS:81  GVRET-TCP:23  OTA:/update  STM32-OTA:/update-stm32  ready.");
}

void loop() {
    http.handleClient(); ws.loop(); gvretService(); frameBatchFlush(); logService();
    for (int budget = 8192; budget > 0 && Stm32.available(); budget--) {  /* keep HTTP/WS serviced */
        char c=(char)Stm32.read();
        if(c=='\n'){
            if(lineLen>0 && !lineDiscard){lineBuf[lineLen]='\0';parseLine(lineBuf,lineLen);}
            lineLen=0; lineDiscard=false;
        }
        else if(c!='\r' && !lineDiscard){
            if(lineLen<(uint16_t)(sizeof(lineBuf)-1)) lineBuf[lineLen++]=c;
            else { lineLen=0; lineDiscard=true; }   /* drop the rest of this line */
        }
    }
    static uint32_t last=0; uint32_t now=millis();
    if(now-last>=1000){ last=now; emitStatus(); }
}
