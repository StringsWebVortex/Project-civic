/*  Civic CAN Dashboard — ESP32 firmware (ESP-32S NodeMCU v1.1)
 *  Wi-Fi AP  : Civic-CAN-Monitor, WPA2 password CANmonitor26 (fixed, owner request).
 *  HTTP      : port 80, no web login (owner request).
 *  WebSocket : port 81, live JSON telemetry. Commands (TX, ARM/DISARM, UDS)
 *              are accepted from a client that sent the session token from
 *              GET /token (no web login; Wi-Fi WPA2 is the gate).
 *  HTTP POST : /update       → ESP32 OTA (multipart, image verified by Update.end)
 *  HTTP POST : /update-stm32 → STM32 flash via ROM UART bootloader (ST AN3155):
 *              vector table checked before erase, read-back verified after write.
 *  STM32 UART: HardwareSerial Stm32(2)  RX=GPIO16 (← STM32 PA2 USART2_TX)
 *              TX=GPIO17 (→ STM32 PA3 USART2_RX)  2 Mbaud 8N1
 *              (PA9/PA10 are the Discovery USB VBUS/ID lines, UM1472 Rev 9)
 *  Safety    : when the last logged-in dashboard client disconnects, the
 *              ESP32 sends DISARM, so the bus never stays armed unattended.
 *  STM32 boot: GPIO4 → BOOT0, GPIO5 → NRST, GPIO27 → PB2/BOOT1, bootloader UART
 *              GPIO25 → PB11, GPIO26 ← PB10 (see the comment at STM32_BOOT0)
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

static constexpr char     AP_SSID[]  = "Civic-CAN-Monitor";
static constexpr char     WEB_USER[] = "admin";
static constexpr int      AP_CHANNEL = 6;
static constexpr int      AP_MAX_CONN= 4;
static constexpr int      STM32_RX   = 16;
static constexpr int      STM32_TX   = 17;
static constexpr uint32_t STM32_BAUD = 2000000;  // must equal STM32 PROJECT_HOST_UART_BAUD; ESP32 UART max 5 MBaud (ESP32 datasheet v5.3 4.8.3)

/* STM32 ROM bootloader wiring (STM32F4DISCOVERY MB997 rev E schematic,
 * UM1472 Rev 9 Table 7, DS8626 Rev 12 Section 3.13, AN2606 STM32F40xxx/41xxx):
 *  - BOOT0 (P2-21) is held LOW by R30 510 ohm + SB18 to GND (R31 10k to VDD):
 *    the ESP32 drives it HIGH push-pull (about 6.5 mA into 510 ohm; BOOT0
 *    VIH min 0.17*VDD+0.7 V, DS8626 Table 48).
 *  - BOOT1 = PB2 (P1-24) is held HIGH by R34 10k (SB19 not fitted). BOOT0=1
 *    with BOOT1=1 boots from SRAM, so the ESP32 pulls PB2 LOW (open-drain)
 *    around the reset that enters the bootloader (system memory = BOOT1 0,
 *    BOOT0 1: RM0090 Table 2).
 *  - The application UART is USART2 PA2/PA3, which is not a bootloader port.
 *    The bootloader listens on USART3 PB10 (TX, P1-34) / PB11 (RX, P1-35):
 *    the ESP32 moves its UART2 to GPIO26 (RX) / GPIO25 (TX) only while
 *    flashing. PB10 also drives the on-board MEMS microphone CLK input
 *    (UM1472 Table 7); PB11 is a free I/O. The application never configures
 *    PB2/PB10/PB11, so they stay inputs while it runs. */
static constexpr int STM32_BOOT0 = 4;   /* push-pull: HIGH = ROM bootloader on next reset */
static constexpr int STM32_NRST  = 5;   /* open-drain: LOW = reset, HIGH = released */
static constexpr int STM32_BOOT1 = 27;  /* open-drain: LOW = PB2 low, HIGH = released */
static constexpr int STM32_BL_RX = 26;  /* <- STM32 PB10 USART3_TX (bootloader only) */
static constexpr int STM32_BL_TX = 25;  /* -> STM32 PB11 USART3_RX (bootloader only) */
/* Mass erase: DS8626 Rev 12 Table 40 tME max 32 s at x8 parallelism. */
static constexpr uint32_t BL_MASS_ERASE_TIMEOUT_MS = 40000;

WebServer        http(80);
WebSocketsServer ws(81);
HardwareSerial   Stm32(2);

static char     lineBuf[221];
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
body{font:13px/1.5 system-ui,sans-serif;background:#0d1117;color:#c9d1d9;min-height:100vh;padding:10px 12px}
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
<h1>Vehicle Telemetry Dashboard</h1>
<div id="statusBar">Connecting…</div>

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
<div class="card full">
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
<div class="card full">
  <div class="card-head">
    <span class="card-title">Signal Explorer — All ECU Signals</span>
    <span class="card-sub" id="sigCnt">0 signals</span>
  </div>
  <div style="max-height:380px;overflow-y:auto" id="sigExp"><p style="color:#8b949e;padding:8px">Waiting for PROFILE/SIG data…</p></div>
</div>

<!-- ══ FULL WIDTH: Controllers & bus health ══ -->
<div class="card full">
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
<div class="card full">
  <div class="card-head"><span class="card-title">Fault Codes &amp; Live Sensors</span><span class="card-sub" id="dtcSum">—</span></div>
  <div style="font-size:.65rem;color:#8b949e;margin-bottom:6px">Needs ARM. Reads go to every controller found by the scan (or the engine ECU at 7E0 if no scan was run).</div>
  <div style="display:flex;gap:6px;flex-wrap:wrap;margin-bottom:6px">
    <button class="btn btn-sm" onclick="readDtcs()">Read all DTCs</button>
    <button class="btn btn-sm" onclick="clearDtcs()">Clear all DTCs</button>
    <select id="rstEcu" style="min-width:160px"></select>
    <select id="rstType"><option value="01">Hard reset (01)</option><option value="03">Soft reset (03)</option><option value="02">Key off/on (02)</option></select>
    <button class="btn btn-sm" onclick="ecuReset()">ECU reset</button>
    <button class="btn btn-sm" id="pidBtn" onclick="togglePids()">Start live PIDs</button>
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
  <div style="display:grid;grid-template-columns:1fr 1fr;gap:10px">
    <div class="can-wrap" style="max-height:240px;overflow:auto"><table><thead><tr><th>Controller</th><th>DTC</th><th>Source</th><th>Status</th></tr></thead>
      <tbody id="dtcRows"><tr><td colspan="4" style="color:#8b949e">Not read yet.</td></tr></tbody></table></div>
    <div class="can-wrap" style="max-height:240px;overflow:auto"><table><thead><tr><th>PID</th><th>Sensor</th><th>Value</th></tr></thead>
      <tbody id="pidRows"><tr><td colspan="3" style="color:#8b949e">—</td></tr></tbody></table></div>
  </div>
</div>

<!-- ══ FULL WIDTH: Operating Mode ══ -->
<div class="card full">
  <div class="card-head"><span class="card-title">Operating Mode</span></div>
  <div class="mode-tabs">
    <button class="mtab on" id="tabR" onclick="setMode('r')">Read Mode</button>
    <button class="mtab"    id="tabW" onclick="setMode('w')">Write Mode</button>
  </div>
  <div id="pnlR"><p style="font-size:.75rem;color:#8b949e">Passive monitor — no CAN frames are transmitted. All data is receive-only.</p></div>
  <div id="pnlW" style="display:none">
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
function setMode(m){
  const w=m==='w';
  $('pnlW').style.display=w?'block':'none';
  $('pnlR').style.display=w?'none':'block';
  $('tabW').className='mtab'+(w?' on':'');
  $('tabR').className='mtab'+(w?'':' on');
}

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

/* ── authenticated command channel ── */
let authOk=false, armedLeft=0, udsBuf='';
function wsSend(o){
  if(!sock||sock.readyState!==1||!authOk){ diagLog('Not connected or not logged in.'); return false; }
  sock.send(JSON.stringify(o)); return true;
}
function diagLog(t){
  const el=$('udsLog'); el.textContent=(new Date().toLocaleTimeString()+'  '+t+'\n'+el.textContent).slice(0,6000);
}
function showArm(){
  $('armState').textContent=armedLeft>0?('ARMED — '+armedLeft+' s left (CAN controller active)'):'Disarmed (silent, receive-only)';
  $('armState').style.color=armedLeft>0?'#f85149':'#8b949e';
}
setInterval(()=>{ if(armedLeft>0){armedLeft--; showArm();} },1000);
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
  let f=t.split(',');
  if(t.startsWith('ECU,')){ onEcu(t); return; }
  if(idCur && t.startsWith('UDS,')){
    if(t.startsWith('UDS,DATA,')){ udsBuf+=f[4]||''; return; }
    if(t.startsWith('UDS,POS,')){ const h=udsBuf; udsBuf=''; idDone(h); return; }
    if(t.startsWith('UDS,NEG,')){ idDone(null,'NRC 0x'+(f[4]||'')); return; }
    if(t.startsWith('UDS,SENT,')){ idDone('00','SENT'); return; }
    if(t.startsWith('UDS,ERR,')){ idDone(null,f[2]||'ERR'); return; }
    if(t.startsWith('UDS,PENDING,')) return;
  }
  if(t.startsWith('STATUS,ARMED,')){ armedLeft=parseInt(f[2],10)||0; showArm(); diagLog(t); return; }
  if(t.startsWith('STATUS,DISARMED,')){ armedLeft=0; showArm(); diagLog(t); return; }
  if(t.startsWith('UDS,DATA,')){ udsBuf+=f[4]||''; return; }
  if(t.startsWith('UDS,POS,')){ diagLog('← '+f[2]+' positive, '+f[3]+' bytes: '+udsBuf); udsBuf=''; return; }
  if(t.startsWith('UDS,NEG,')){ diagLog('← '+f[2]+' negative: service 0x'+f[3]+' NRC 0x'+f[4]); return; }
  diagLog(t);
}

/* ── controllers & bus health ── */
let lastBitrate=0;
const ecus={};          // rx id → {tx,rx,fmt,reply,info:{}}
let idq=[], idCur=null, idTimer=null;
const esc=v=>String(v).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
function updBus(){
  const ids=Object.keys(frm$); let fps=0, bps=0;
  ids.forEach(id=>{ const s=frm$[id], r=s.rate||0, ext=id.length>3;
    fps+=r; bps+=r*((ext?67:47)+8*(s.dlc||0))*1.1; });   // ~10% stuff bits
  $('bhIds').textContent=ids.length;
  $('bhFps').textContent=fps.toFixed(0);
  $('bhLoad').textContent=lastBitrate?Math.min(100,100*bps/lastBitrate).toFixed(1)+' %':'—';
  $('busSum').textContent=Object.keys(ecus).length+' diagnostic controller(s) found';
}
setInterval(updBus,1000);
function onStats(t){
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
function diagReq(e,req,cb,confirmed){ idq.push({e,req,cb,confirmed:!!confirmed}); idNext(); }
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
  setTimeout(idNext,20);
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
let pidList=[], pidOn=false, pidVals={};
function pidEcu(){ const k=Object.keys(ecus).filter(r=>ecus[r].fmt==='S'); return k.length?ecus[k[0]]:{tx:'7E0',rx:'7E8',fmt:'S',info:{}}; }
function discoverPids(cb){
  pidList=[]; const e=pidEcu();
  const step=(base)=>diagReq(e,'01'+base.toString(16).padStart(2,'0').toUpperCase(),(hex)=>{
    if(!hex){ cb(); return; } const b=hexBytes(hex).slice(2);
    for(let i=0;i<32;i++) if(b[i>>3]&(0x80>>(i&7))){ const p=base+i+1; if((p&0x1F)!==0) pidList.push(p); }
    if((b[3]&1)&&base<0xC0) step(base+0x20); else cb(); });
  step(0);
}
function pollPids(){
  if(!pidOn) return;
  if(!pidList.length){ diagLog('No supported PIDs reported.'); pidOn=false; $('pidBtn').textContent='Start live PIDs'; return; }
  const e=pidEcu(); let left=pidList.length;
  pidList.forEach(p=>diagReq(e,'01'+p.toString(16).padStart(2,'0').toUpperCase(),(hex)=>{
    if(hex){ const b=hexBytes(hex); if(b[1]===p){ const d=b.slice(2), f=PIDS[p];
      pidVals[p]=f?[f[0],(Math.round(f[2](d)*100)/100)+' '+f[1]]:['PID 0x'+p.toString(16).toUpperCase(),d.map(x=>x.toString(16).padStart(2,'0')).join(' ')]; } }
    if(--left===0){ drawPids(); setTimeout(pollPids,200); } }));
}
function drawPids(){
  const k=Object.keys(pidVals).map(Number).sort((a,b)=>a-b);
  $('pidRows').innerHTML=k.map(p=>`<tr><td>0x${p.toString(16).toUpperCase().padStart(2,'0')}</td><td>${esc(pidVals[p][0])}</td><td><b>${esc(pidVals[p][1])}</b></td></tr>`).join('')||'<tr><td colspan="3" style="color:#8b949e">—</td></tr>';
}
function togglePids(){
  pidOn=!pidOn; $('pidBtn').textContent=pidOn?'Stop live PIDs':'Start live PIDs';
  if(pidOn) discoverPids(()=>{ diagLog(pidList.length+' supported PIDs'); pollPids(); });
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
  if(t.startsWith('ECU,FOUND,')){
    const rx=f[3]; ecus[rx]={tx:f[2],rx,fmt:f[2].length>3?'E':'S',reply:f[4]||'',info:(ecus[rx]&&ecus[rx].info)||{}};
    drawEcus(); fillRstEcus(); diagLog(t); return;
  }
  if(t.startsWith('ECU,DONE,')){ diagLog('ECU scan finished: '+f[2]+' controller(s) answered'); drawEcus(); return; }
  diagLog(t);
}

/* ── message handler ── */
function onMsg(m){
  if(m.type==='stats'){ onStats(String(m.message||'')); return; }
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

/* ── websocket ── */
function connect(){
  $('statusBar').textContent='Connecting…';
  try{ sock=new WebSocket('ws://'+location.hostname+':81/'); }
  catch(e){ $('statusBar').textContent='WS error: '+e.message; setTimeout(connect,2000); return; }
  sock.onopen =()=>{
    $('statusBar').textContent='Connected — waiting for STM32…';
    authOk=false;
    fetch('/token',{cache:'no-store'}).then(r=>r.ok?r.text():Promise.reject(r.status))
      .then(t=>sock.send(JSON.stringify({op:'auth',token:t.trim()})))
      .catch(e=>diagLog('Could not get command token: '+e));
  };
  sock.onclose=()=>{ $('statusBar').textContent='Disconnected — reconnecting…'; setTimeout(connect,1500); };
  sock.onerror=()=>$('statusBar').textContent='WebSocket error';
  sock.onmessage=e=>{ try{onMsg(JSON.parse(e.data));}catch{} };
}

/* ── 1s redraw loop ── */
setInterval(()=>{ drawRpm(); drawTemp(); drawScat(); updQuality(); updCAN(); updSigs(); }, 1000);
window.addEventListener('resize',()=>{ drawRpm(); drawTemp(); drawScat(); });
connect();
</script>

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
      <div style="font-size:.68rem;color:#8b949e;margin-bottom:4px">Upload a <code style="background:#21262d;padding:1px 4px;border-radius:3px">.bin</code> built with <code style="background:#21262d;padding:1px 4px;border-radius:3px">make firmware</code>. Requires <b>GPIO4→BOOT0</b> and <b>GPIO5→NRST</b> wired. <b style="color:#f85149">Not yet tested on the boards:</b> uses the ROM bootloader on USART3 PB10/PB11; the on-board ST-LINK remains the primary flashing path.</div>
      <div style="font-size:.65rem;background:#1c2128;border:1px solid #30363d;border-radius:4px;padding:6px;margin-bottom:8px;color:#8b949e">
        <b style="color:#e3b341">⚠ Wiring required:</b> ESP32 GPIO4 → BOOT0 (P2-21) &nbsp;|&nbsp; GPIO5 → NRST (P1-6) &nbsp;|&nbsp; GPIO27 → PB2/BOOT1 (P1-24) &nbsp;|&nbsp; GPIO25 → PB11 (P1-35) &nbsp;|&nbsp; GPIO26 ← PB10 (P1-34)
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
    if (strncmp(raw, "UDS,", 4) == 0 || strncmp(raw, "CMD,", 4) == 0 || strncmp(raw, "ECU,", 4) == 0 ||
        strncmp(raw, "STATUS,ARMED,", 13) == 0 || strncmp(raw, "STATUS,DISARMED,", 16) == 0 ||
        strncmp(raw, "STATUS,RESET,", 13) == 0 || strncmp(raw, "STATUS,WATCHDOG,", 16) == 0) {
        JsonDocument d; d["type"]="diag"; d["message"]=raw;   /* ArduinoJson escapes */
        String out; serializeJson(d,out); ws.broadcastTXT(out); return;
    }
    if (strncmp(raw, "STATUS,STATS,", 13) == 0) {
        JsonDocument d; d["type"]="stats"; d["message"]=raw + 13;
        String out; serializeJson(d,out); ws.broadcastTXT(out); return;
    }
    if (strncmp(raw, "STATUS,LOCK,", 12) == 0)     { bitrate=(uint32_t)atol(raw+12); locked=true;  emitStatus(); return; }
    if (strncmp(raw, "STATUS,SCAN,", 12) == 0)     { bitrate=(uint32_t)atol(raw+12); locked=false; emitStatus(); return; }
    if (strncmp(raw, "STATUS,REACQUIRE", 16) == 0) { locked=false; bitrate=0;        emitStatus(); return; }
    if (strncmp(raw, "STATUS,DWELL,", 13) == 0)    { dwellMs=(uint32_t)atol(raw+13); emitStatus(); return; }
    if (strncmp(raw, "STATUS,", 7) == 0) return;   /* other status lines: not a frame */

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
        if(!ts.length()||!id.length()) return;
        if(fmt!="STD"&&fmt!="EXT") return;
        int dlc=dlcS.toInt(); if(dlc<0||dlc>8) return;
        framesSeen++;
        /* P1.6: static buffer replaces StaticJsonDocument heap allocation */
        static char fbuf[200];
        const char *fmtS = fmt.c_str();
        int fn = snprintf(fbuf, sizeof fbuf,
            "{\"type\":\"frame\",\"ts\":\"%s\",\"id\":\"%s\",\"format\":\"%s\","
            "\"rtr\":%s,\"dlc\":%d,\"data\":\"%s\",\"frames\":%d,"
            "\"locked\":%s,\"bitrate\":%d}",
            ts.c_str(), id.c_str(), fmtS,
            (rtrS.toInt()!=0)?"true":"false", dlc, pay.c_str(),
            (int)framesSeen, locked?"true":"false", (int)bitrate);
        if (fn > 0 && fn < (int)sizeof fbuf) ws.broadcastTXT(fbuf, fn);
        return;
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
    if (type == WStype_CONNECTED)    { wsAuthed[num] = false; emitStatus(); return; }
    if (type == WStype_DISCONNECTED) {
        const bool was = wsAuthed[num];
        wsAuthed[num] = false;
        bool any = false;
        for (int i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) any = any || wsAuthed[i];
        /* No operator left: end any ARM window (the STM32 ignores DISARM when
         * it is not armed). */
        if (was && !any && !stm32FlashRunning) Stm32.print("DISARM\n");
        return;
    }
    if (type != WStype_TEXT || length == 0 || length > 300) return;
    JsonDocument cmd;
    if (deserializeJson(cmd, payload, length)) return;
    const char *op = cmd["op"] | "";

    if (strcmp(op, "auth") == 0) {
        String t(cmd["token"] | "");
        wsAuthed[num] = wsToken.length() > 0 && t == wsToken;
        JsonDocument r; r["type"]="auth"; r["ok"]=wsAuthed[num];
        String out; serializeJson(r,out); ws.sendTXT(num, out);
        return;
    }
    if (!wsAuthed[num]) { wsReply(num, "diag", "CMD,ERR,NOT_AUTHENTICATED"); return; }

    char buf[200];
    if (strcmp(op, "arm") == 0) {
        int secs = cmd["seconds"] | 0;
        if (secs < 1 || secs > 600) { wsReply(num, "diag", "CMD,ERR,ESP32_BAD_ARM"); return; }
        snprintf(buf, sizeof buf, "ARM,%d\n", secs);
        Stm32.print(buf); return;
    }
    if (strcmp(op, "disarm") == 0) { Stm32.print("DISARM\n"); return; }
    if (strcmp(op, "uds_abort") == 0) { Stm32.print("UDS,ABORT\n"); return; }
    if (strcmp(op, "ecuscan_abort") == 0) { Stm32.print("ECUSCAN,ABORT\n"); return; }
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
        snprintf(buf, sizeof buf, "ECUSCAN,%s,%s,%s,%s,%d\n", fmt.c_str(), first.c_str(), last.c_str(), req.c_str(), wait);
        Stm32.print(buf); return;
    }
    if (strcmp(op, "udspad") == 0) {
        String v(cmd["value"] | ""); v.toUpperCase();
        if (v != "NONE" && !isHexStr(v, 2, 2)) { wsReply(num, "diag", "CMD,ERR,ESP32_BAD_PAD"); return; }
        snprintf(buf, sizeof buf, "UDSPAD,%s\n", v.c_str());
        Stm32.print(buf); return;
    }
    if (strcmp(op, "uds") == 0) {
        String fmt(cmd["format"] | ""); fmt.toUpperCase();
        String tx(cmd["tx"] | "");  tx.toUpperCase();
        String rx(cmd["rx"] | "");  rx.toUpperCase();
        String req(cmd["req"] | ""); req.toUpperCase();
        bool confirm = cmd["confirm"] | false;
        uint32_t maxId = (fmt == "S") ? 0x7FFu : (fmt == "E" ? 0x1FFFFFFFu : 0u);
        bool ok = (fmt == "S" || fmt == "E") && isHexStr(tx, 1, 8) && isHexStr(rx, 1, 8) &&
                  strtoul(tx.c_str(), nullptr, 16) <= maxId && strtoul(rx.c_str(), nullptr, 16) <= maxId &&
                  isHexStr(req, 2, 128) && (req.length() % 2) == 0;
        if (!ok) { wsReply(num, "diag", "UDS,ERR,ESP32_VALIDATION_FAILED"); return; }
        snprintf(buf, sizeof buf, "UDS,%s,%s,%s,%s%s\n", fmt.c_str(), tx.c_str(), rx.c_str(),
                 req.c_str(), confirm ? ",CONFIRM" : "");
        Stm32.print(buf); return;
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
    snprintf(buf,sizeof buf,"TX,%s,%s,%d,%d,%s\n",fmt.c_str(),id.c_str(),rtr,dlc,(rtr||dlc==0)?"-":data.c_str());
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

/* Enter STM32 bootloader: BOOT0=HIGH, pulse NRST. Returns true on 0x7F sync ACK. */
static bool blEnter() {
    Serial.println("[BL] Entering bootloader mode");
    Stm32.end(); /* release the application UART pins */
    pinMode(STM32_RX, INPUT); pinMode(STM32_TX, INPUT);
    delay(10);
    digitalWrite(STM32_BOOT0, HIGH);
    digitalWrite(STM32_BOOT1, LOW);   /* PB2 low: system memory, not SRAM */
    delay(10);
    /* pulse NRST: LOW ~10ms then released; boot pins are sampled at reset */
    digitalWrite(STM32_NRST, LOW);
    delay(10);
    digitalWrite(STM32_NRST, HIGH);
    delay(100); /* STM32 ROM starts */
    digitalWrite(STM32_BOOT1, HIGH);  /* release PB2 */
    /* Bootloader port USART3 PB10/PB11, 8 data bits + even parity (AN3155) */
    Stm32.begin(57600, SERIAL_8E1, STM32_BL_RX, STM32_BL_TX);
    delay(50);
    /* Send init byte 0x7F — bootloader auto-detects baud */
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

/* Exit bootloader: BOOT0=LOW, pulse NRST, restore normal UART */
static void blExit() {
    Serial.println("[BL] Exiting bootloader, resetting STM32");
    Stm32.end();
    pinMode(STM32_BL_RX, INPUT); pinMode(STM32_BL_TX, INPUT);
    delay(10);
    digitalWrite(STM32_BOOT1, HIGH);
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
    if (!blEnter()) { blExit(); return "STM32 bootloader did not answer the 0x7F sync (check BOOT0/BOOT1/NRST and PB10/PB11 wiring)"; }
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

    /* GO command — jump to flash start */
    if (blSendCmd(BL_GO)) {
        blSendAddr(FLASH_START);
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

    /* STM32 bootloader control pins */
    pinMode(STM32_BOOT0, OUTPUT); digitalWrite(STM32_BOOT0, LOW);
    /* NRST is open-drain: the ESP32 only ever pulls it LOW; HIGH = released
     * (pin floats, the STM32's own NRST pull-up and the Discovery reset button
     * / ST-LINK keep working, and internal resets such as IWDG can still drive
     * the pin). A push-pull HIGH would fight the reset button and ST-LINK. */
    digitalWrite(STM32_NRST, HIGH);
    pinMode(STM32_NRST, OUTPUT_OPEN_DRAIN); digitalWrite(STM32_NRST, HIGH);
    digitalWrite(STM32_BOOT1, HIGH);
    pinMode(STM32_BOOT1, OUTPUT_OPEN_DRAIN); digitalWrite(STM32_BOOT1, HIGH);
    pinMode(STM32_BL_RX, INPUT); pinMode(STM32_BL_TX, INPUT);

    Stm32.setRxBufferSize(16384);
    Stm32.begin(STM32_BAUD,SERIAL_8N1,STM32_RX,STM32_TX);
    Serial.printf("STM32 UART2: RX=GPIO%d TX=GPIO%d baud=%u\n",STM32_RX,STM32_TX,STM32_BAUD);
    Serial.printf("STM32 BOOT0: GPIO%d  NRST: GPIO%d\n",STM32_BOOT0,STM32_NRST);

    WiFi.mode(WIFI_AP);                 /* RF on before secrets are generated */
    loadDevicePass();
    wsToken = randomSecret(24);
    Serial.printf("Wi-Fi password: %s\n", devicePass.c_str());
    bool ok=WiFi.softAP(AP_SSID,devicePass.c_str(),AP_CHANNEL,0,AP_MAX_CONN);
    Serial.printf("Wi-Fi AP '%s' %s  IP=%s\n",AP_SSID,ok?"UP":"FAILED",WiFi.softAPIP().toString().c_str());

    http.on("/",          HTTP_GET,  handleRoot);
    http.on("/token",     HTTP_GET,  handleToken);
    http.on("/update",    HTTP_POST, handleESP32Update,    handleESP32UpdateUpload);
    http.on("/update-stm32", HTTP_POST, handleSTM32Update, handleSTM32UpdateUpload);
    http.onNotFound(handleNotFound);
    http.begin();

    ws.begin(); ws.onEvent(handleWsEvent);
    ws.enableHeartbeat(15000, 3000, 2);  /* lost client -> disconnect -> DISARM */
    Serial.println("HTTP:80  WS:81  OTA:/update  STM32-OTA:/update-stm32  ready.");
}

void loop() {
    http.handleClient(); ws.loop();
    for (int budget = 2048; budget > 0 && Stm32.available(); budget--) {  /* keep HTTP/WS serviced */
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
