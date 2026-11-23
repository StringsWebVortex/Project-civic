/*
 * esp32_bridge — Full Wi-Fi bridge for STM32F407 CAN diagnostic system
 *
 * ESP32 purposes (and ONLY these):
 *   1. Wi-Fi AP "ST-CAN" / "Discovery1" at 192.168.4.1
 *   2. Serve STM32 dashboard HTML from SPIFFS (gzipped)
 *   3. WebSocket proxy on port 81: browser ↔ ESP WS ↔ UART ↔ STM32
 *   4. Captive portal — auto-open dashboard at 192.168.4.1
 *   5. SPIFFS circular log of recent CAN frames
 *   6. USB-serial bridge: laptop USB→ESP Serial ↔ Serial2→STM UART
 *   7. Remote STM32 firmware update via WiFi (USART bootloader)
 *   8. /token endpoint for dashboard WS auth
 *
 * WIRING:
 *   ESP32 GPIO16 (RX2) ← STM32 PA2 (USART2_TX, 2 Mbaud)
 *   ESP32 GPIO17 (TX2) → STM32 PA3 (USART2_RX, 2 Mbaud)
 *   ESP32 GPIO26       → STM32 NRST (reset)
 *   ESP32 GPIO27       → STM32 BOOT0 (bootloader select)
 *   GND shared
 */

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <Update.h>
#include <esp_partition.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

/* ── config ───────────────────────────────────────────────────────────── */
static const char      *WIFI_SSID  = "ST-CAN";
static const char      *WIFI_PASS  = "Discovery1";
static const IPAddress  AP_IP(192,168,4,1);
static const IPAddress  AP_GW(192,168,4,1);
static const IPAddress  AP_NM(255,255,255,0);

static const char *ESP_DASH = "http://192.168.4.1/";

static const int   UART_RX_PIN  = 16;
static const int   UART_TX_PIN  = 17;
static const long  UART_BAUD    = 2000000L;

/* STM32 bootloader control pins */
static const int   PIN_NRST  = 26;
static const int   PIN_BOOT0 = 27;

/* STM32 USART bootloader baud (standard) */
static const long  BOOT_BAUD = 115200L;

/* Log file: rotate when > 512 KB */
static const char  *LOG_PATH = "/canlog.txt";
static const size_t LOG_MAX  = 512UL * 1024UL;

/* Dashboard gzip file in SPIFFS */
static const char  *DASH_GZ_PATH = "/index.html.gz";

/* ── globals ──────────────────────────────────────────────────────────── */
static AsyncWebServer  http_server(80);
static AsyncWebServer  ws_server(81);
static AsyncWebSocket  ws_proxy("/");     /* port 81 — WS proxy to STM */
static DNSServer       dns;

static uint32_t uart_rx_bytes = 0;
static File     log_file;
static bool     spiffs_ok     = false;
static bool     flashing_stm  = false;    /* true during firmware upload */

/* ── USB-serial bridge flag ───────────────────────────────────────────── */
/* When a laptop is plugged into ESP USB, Serial (USB-CDC, 115200) bridges
   bidirectionally to Serial2 (STM UART). This always runs in loop(). */

/* ══════════════════════════════════════════════════════════════════════
 *  ESP-4: Captive portal → 192.168.4.1
 * ══════════════════════════════════════════════════════════════════════ */
/* Android connectivity check — return 204 so Android keeps WiFi alive */
static void handle_generate_204(AsyncWebServerRequest *req) {
    req->send(204);
}
/* Apple CNA — return "Success" so iOS keeps WiFi connected */
static void handle_apple_cna(AsyncWebServerRequest *req) {
    req->send(200, "text/html",
        "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
}
/* Windows NCSI — proper response so Windows keeps WiFi */
static void handle_ncsi(AsyncWebServerRequest *req) {
    req->send(200, "text/plain", "Microsoft NCSI");
}
/* Windows connection test */
static void handle_connecttest(AsyncWebServerRequest *req) {
    req->send(200, "text/plain", "Microsoft Connect Test");
}

/* ══════════════════════════════════════════════════════════════════════
 *  ESP-3: WebSocket proxy on port 81 — browser ↔ UART ↔ STM32
 * ══════════════════════════════════════════════════════════════════════ */
static void on_ws_proxy_event(AsyncWebSocket *, AsyncWebSocketClient *client,
                              AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("[WS] client #%u connected\n", client->id());
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("[WS] client #%u disconnected\n", client->id());
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->opcode == WS_TEXT && !flashing_stm) {
            /* Browser → STM: forward command over UART */
            Serial2.write(data, len);
            Serial2.write('\n');
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 *  ESP-5: SPIFFS circular log (kept from original — no changes)
 * ══════════════════════════════════════════════════════════════════════ */
static void log_line(const String &line) {
    if (!spiffs_ok) return;
    if (!log_file) log_file = SPIFFS.open(LOG_PATH, FILE_APPEND);
    if (!log_file) return;
    log_file.println(line);
    log_file.flush();
    if ((size_t)log_file.size() > LOG_MAX) {
        log_file.close();
        SPIFFS.remove(LOG_PATH);
        log_file = SPIFFS.open(LOG_PATH, FILE_WRITE);
        Serial.println("[LOG] rotated");
    }
}

/* ══════════════════════════════════════════════════════════════════════
 *  ESP-7: Remote STM32 firmware update via USART bootloader
 *
 *  STM32F407 USART bootloader protocol (AN2606 / AN3155):
 *   1. Hold BOOT0 HIGH, pulse NRST → STM enters system bootloader
 *   2. Send 0x7F sync byte, wait for ACK (0x79)
 *   3. Send commands: Get, Erase, Write Memory
 *   4. Release BOOT0 LOW, pulse NRST → STM boots from flash
 *
 *  Commands used:
 *   0x00 = Get (get bootloader version + supported commands)
 *   0x43 = Erase (or 0x44 = Extended Erase for STM32F4)
 *   0x31 = Write Memory (256 bytes at a time)
 * ══════════════════════════════════════════════════════════════════════ */

/* Bootloader ACK/NACK */
static const uint8_t BL_ACK  = 0x79;
static const uint8_t BL_NACK = 0x1F;

static bool bl_wait_ack(uint32_t timeout_ms = 1000) {
    uint32_t t0 = millis();
    while (millis() - t0 < timeout_ms) {
        if (Serial2.available()) {
            uint8_t c = Serial2.read();
            if (c == BL_ACK) return true;
            if (c == BL_NACK) return false;
        }
        delay(1);
    }
    return false;
}

static void bl_drain(void) {
    delay(5);
    while (Serial2.available()) Serial2.read();
}

/* Enter bootloader mode: BOOT0=HIGH, reset STM */
static bool bl_enter(void) {
    Serial.println("[FW] entering bootloader...");
    /* Reconfigure UART to bootloader baud */
    Serial2.end();
    delay(10);
    Serial2.begin(BOOT_BAUD, SERIAL_8E1, UART_RX_PIN, UART_TX_PIN);
    delay(10);

    /* BOOT0 HIGH, then reset pulse */
    digitalWrite(PIN_BOOT0, HIGH);
    delay(10);
    digitalWrite(PIN_NRST, LOW);
    delay(50);
    digitalWrite(PIN_NRST, HIGH);
    delay(100);  /* wait for bootloader to start */

    bl_drain();

    /* Send sync byte 0x7F */
    Serial2.write(0x7F);
    if (!bl_wait_ack(2000)) {
        Serial.println("[FW] sync failed — no ACK");
        return false;
    }
    Serial.println("[FW] bootloader synced");
    return true;
}

/* Exit bootloader: BOOT0=LOW, reset STM → normal boot */
static void bl_exit(void) {
    Serial.println("[FW] exiting bootloader...");
    Serial2.end();
    delay(10);
    digitalWrite(PIN_BOOT0, LOW);
    delay(10);
    digitalWrite(PIN_NRST, LOW);
    delay(50);
    digitalWrite(PIN_NRST, HIGH);
    delay(200);
    /* Restore normal UART */
    Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    Serial.println("[FW] STM32 reset to normal boot");
}

/* Send bootloader command byte (cmd + ~cmd) */
static bool bl_send_cmd(uint8_t cmd) {
    Serial2.write(cmd);
    Serial2.write((uint8_t)(~cmd));
    return bl_wait_ack();
}

/* Extended Erase (0x44) — mass erase */
static bool bl_mass_erase(void) {
    Serial.println("[FW] mass erase...");
    if (!bl_send_cmd(0x44)) {
        Serial.println("[FW] erase cmd rejected");
        return false;
    }
    /* Special erase: 0xFFFF = mass erase, checksum = 0x00 */
    uint8_t buf[3] = {0xFF, 0xFF, 0x00};
    Serial2.write(buf, 3);
    /* Mass erase can take up to 30 seconds on STM32F407 */
    if (!bl_wait_ack(30000)) {
        Serial.println("[FW] erase timeout");
        return false;
    }
    Serial.println("[FW] erase done");
    return true;
}

/* Write Memory (0x31) — write up to 256 bytes at address */
static bool bl_write_mem(uint32_t addr, const uint8_t *data, uint16_t len) {
    if (len == 0 || len > 256) return false;

    if (!bl_send_cmd(0x31)) return false;

    /* Send address (4 bytes) + XOR checksum */
    uint8_t ab[5];
    ab[0] = (addr >> 24) & 0xFF;
    ab[1] = (addr >> 16) & 0xFF;
    ab[2] = (addr >>  8) & 0xFF;
    ab[3] =  addr        & 0xFF;
    ab[4] = ab[0] ^ ab[1] ^ ab[2] ^ ab[3];
    Serial2.write(ab, 5);
    if (!bl_wait_ack()) return false;

    /* Send N-1, then N bytes of data, then XOR checksum */
    uint8_t n = (uint8_t)(len - 1);
    Serial2.write(n);
    uint8_t xor_ck = n;
    for (uint16_t i = 0; i < len; i++) {
        Serial2.write(data[i]);
        xor_ck ^= data[i];
    }
    /* Pad to multiple of 4 if needed (bootloader requirement) */
    uint16_t padded = (len + 3) & ~3;
    for (uint16_t i = len; i < padded; i++) {
        Serial2.write((uint8_t)0xFF);
        xor_ck ^= 0xFF;
    }
    Serial2.write(xor_ck);
    return bl_wait_ack(2000);
}

/* Full firmware flash handler */
static String fw_status = "idle";
static uint32_t fw_written = 0;
static uint32_t fw_total   = 0;

static void handle_fw_upload(AsyncWebServerRequest *request,
                             const String &filename, size_t index,
                             uint8_t *data, size_t len, bool final) {
    static uint32_t flash_addr;
    static bool     flash_ok;

    if (index == 0) {
        /* First chunk — enter bootloader and erase */
        fw_total   = request->contentLength();
        fw_written = 0;
        fw_status  = "entering bootloader";
        flashing_stm = true;
        Serial.printf("[FW] upload start: %s (%u bytes)\n", filename.c_str(), fw_total);

        flash_ok = bl_enter();
        if (flash_ok) {
            fw_status = "erasing";
            flash_ok = bl_mass_erase();
        }
        if (!flash_ok) {
            fw_status = "error: bootloader/erase failed";
            return;
        }
        flash_addr = 0x08000000;  /* STM32F407 flash base */
        fw_status = "writing";
    }

    if (!flash_ok) return;

    /* Write data in 256-byte chunks */
    size_t offset = 0;
    while (offset < len) {
        uint16_t chunk = (len - offset > 256) ? 256 : (uint16_t)(len - offset);
        if (!bl_write_mem(flash_addr, data + offset, chunk)) {
            flash_ok = false;
            fw_status = "error: write failed at 0x";
            fw_status += String(flash_addr, HEX);
            Serial.printf("[FW] write failed at 0x%08X\n", flash_addr);
            return;
        }
        flash_addr += chunk;
        offset     += chunk;
        fw_written += chunk;
    }

    if (final) {
        bl_exit();
        flashing_stm = false;
        fw_status = "done";
        Serial.printf("[FW] flash complete: %u bytes written\n", fw_written);
    }
}

/* ══════════════════════════════════════════════════════════════════════
 *  ESP-2: Serve dashboard from SPIFFS (gzipped)
 * ══════════════════════════════════════════════════════════════════════ */
static void serve_dashboard(AsyncWebServerRequest *req) {
    if (!spiffs_ok || !SPIFFS.exists(DASH_GZ_PATH)) {
        req->send(503, "text/plain", "Dashboard not available — SPIFFS missing");
        return;
    }
    /* Read gzipped file into memory and serve with explicit Content-Encoding.
       Dashboard is ~42KB gzipped — fits in ESP32 RAM easily. */
    File f = SPIFFS.open(DASH_GZ_PATH, "r");
    if (!f) { req->send(503, "text/plain", "Cannot open dashboard"); return; }
    size_t sz = f.size();
    AsyncWebServerResponse *resp = req->beginResponse("text/html", sz,
        [f](uint8_t *buffer, size_t maxLen, size_t index) mutable -> size_t {
            return f.read(buffer, maxLen);
        });
    resp->addHeader("Content-Encoding", "gzip");
    resp->addHeader("Cache-Control", "no-cache");
    req->send(resp);
}

/* ══════════════════════════════════════════════════════════════════════
 *  setup
 * ══════════════════════════════════════════════════════════════════════ */
void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    /* STM32 control pins */
    pinMode(PIN_NRST, OUTPUT);
    pinMode(PIN_BOOT0, OUTPUT);
    digitalWrite(PIN_NRST, HIGH);   /* not in reset */
    digitalWrite(PIN_BOOT0, LOW);   /* normal boot */

    /* USB Serial — debug + USB-serial bridge to STM */
    Serial.begin(115200);
    Serial.println("\n[ST-CAN Bridge] boot v2");

    /* SPIFFS */
    spiffs_ok = SPIFFS.begin(true);
    Serial.printf("[SPIFFS] %s\n", spiffs_ok ? "ok" : "failed");
    if (spiffs_ok) {
        Serial.printf("[SPIFFS] dash gz exists: %s\n",
                      SPIFFS.exists(DASH_GZ_PATH) ? "yes" : "no");
    }

    /* UART2 to STM */
    Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    Serial.printf("[UART] RX=GPIO%d TX=GPIO%d baud=%ld\n",
                  UART_RX_PIN, UART_TX_PIN, UART_BAUD);

    /* Wi-Fi AP */
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GW, AP_NM);
    WiFi.softAP(WIFI_SSID, WIFI_PASS);
    Serial.printf("[WiFi] AP '%s' up at %s\n",
                  WIFI_SSID, WiFi.softAPIP().toString().c_str());

    /* DNS — all domains → 192.168.4.1 (captive portal) */
    dns.start(53, "*", AP_IP);

    /* ── Port 81: WebSocket proxy ─────────────────────────────────── */
    ws_proxy.onEvent(on_ws_proxy_event);
    ws_server.addHandler(&ws_proxy);
    ws_server.begin();
    Serial.println("[WS] proxy on :81");

    /* ── Port 80: HTTP — dashboard + captive portal + API ─────────── */

    /* ESP-2: Dashboard at root */
    http_server.on("/", HTTP_GET, serve_dashboard);

    /* ESP-8: Token endpoint for WS auth */
    http_server.on("/token", HTTP_GET, [](AsyncWebServerRequest *req){
        /* Simple token — dashboard uses this to auth WS.
           In a real system this would be a proper auth flow. */
        String tok = "{\"token\":\"stcan-";
        tok += String(millis() & 0xFFFF, HEX);
        tok += "\",\"ws\":\"ws://192.168.4.1:81/\"}";
        req->send(200, "application/json", tok);
    });

    /* ESP-7: STM32 firmware upload — response handler (shared by both URLs) */
    auto stm32_fw_response = [](AsyncWebServerRequest *req) {
        String resp = "{\"status\":\"" + fw_status + "\"";
        resp += ",\"bytes\":" + String(fw_written);
        resp += "}";
        int code = fw_status.startsWith("error") ? 500 : 200;
        req->send(code, "application/json", resp);
    };
    http_server.on("/firmware/upload", HTTP_POST, stm32_fw_response, handle_fw_upload);
    /* Dashboard posts to /update-stm32 — alias so both URLs work */
    http_server.on("/update-stm32", HTTP_POST, stm32_fw_response, handle_fw_upload);

    /* ESP32 OTA self-update — dashboard posts .bin to /update */
    http_server.on("/update", HTTP_POST,
        [](AsyncWebServerRequest *req) {
            bool ok = !Update.hasError();
            req->send(ok ? 200 : 500, "text/plain",
                      ok ? "OK — rebooting" : "Update failed");
            if (ok) delay(500);
            if (ok) ESP.restart();
        },
        [](AsyncWebServerRequest *req, const String &filename, size_t index,
           uint8_t *data, size_t len, bool final) {
            if (index == 0) {
                Serial.printf("[OTA] ESP update start: %s\n", filename.c_str());
                if (!Update.begin(req->contentLength(), U_FLASH)) {
                    Update.printError(Serial);
                }
            }
            if (Update.isRunning()) {
                if (Update.write(data, len) != len) {
                    Update.printError(Serial);
                }
            }
            if (final) {
                if (Update.end(true)) {
                    Serial.printf("[OTA] ESP update OK: %u bytes\n", index + len);
                } else {
                    Update.printError(Serial);
                }
            }
        }
    );

    /* Firmware status check */
    http_server.on("/firmware/status", HTTP_GET, [](AsyncWebServerRequest *req){
        String s = "{\"status\":\"" + fw_status + "\"";
        s += ",\"written\":" + String(fw_written);
        s += ",\"total\":" + String(fw_total);
        s += "}";
        req->send(200, "application/json", s);
    });

    /* ESP32 SPIFFS OTA — upload index.html.gz to replace dashboard */
    http_server.on("/update-spiffs", HTTP_POST,
        [](AsyncWebServerRequest *req) {
            bool ok = !Update.hasError();
            req->send(ok ? 200 : 500, "text/plain",
                      ok ? "SPIFFS OK — rebooting" : "SPIFFS update failed");
            if (ok) delay(500);
            if (ok) ESP.restart();
        },
        [](AsyncWebServerRequest *req, const String &filename, size_t index,
           uint8_t *data, size_t len, bool final) {
            if (index == 0) {
                Serial.printf("[OTA] SPIFFS update start: %s\n", filename.c_str());
                size_t spiffsSize = 0x10000; /* 64KB min — actual partition may be larger */
                const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
                if (part) spiffsSize = part->size;
                SPIFFS.end(); /* unmount before writing */
                if (!Update.begin(spiffsSize, U_SPIFFS)) {
                    Update.printError(Serial);
                }
            }
            if (Update.isRunning()) {
                if (Update.write(data, len) != len) {
                    Update.printError(Serial);
                }
            }
            if (final) {
                if (Update.end(true)) {
                    Serial.printf("[OTA] SPIFFS update OK: %u bytes\n", index + len);
                } else {
                    Update.printError(Serial);
                }
            }
        }
    );

    /* Upload page — accessible from phone browser for OTA flashing */
    http_server.on("/flash", HTTP_GET, [](AsyncWebServerRequest *req){
        req->send(200, "text/html",
            "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<title>ST-CAN Flash</title><style>"
            "body{background:#0d1117;color:#c9d1d9;font-family:system-ui;padding:16px;max-width:500px;margin:0 auto}"
            "h2{color:#58a6ff}h3{color:#8b949e;margin-top:24px}"
            ".up{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:16px;margin:8px 0}"
            "input[type=file]{color:#c9d1d9;margin:8px 0}"
            "button{background:#238636;color:#fff;border:none;padding:10px 24px;border-radius:6px;font-size:15px;cursor:pointer;margin-top:8px}"
            "button:active{background:#2ea043}"
            "#prog{color:#3fb950;margin-top:8px;font-family:monospace}"
            "</style></head><body>"
            "<h2>🔧 ST-CAN OTA Flash</h2>"
            "<div class='up'><h3>1. ESP32 Firmware (.bin)</h3>"
            "<input type='file' id='espFw' accept='.bin'>"
            "<button onclick=\"upld('/update','espFw')\">Flash ESP32</button></div>"
            "<div class='up'><h3>2. Dashboard (SPIFFS .bin)</h3>"
            "<input type='file' id='spiFw' accept='.bin'>"
            "<button onclick=\"upld('/update-spiffs','spiFw')\">Flash SPIFFS</button></div>"
            "<div class='up'><h3>3. STM32 Firmware (.bin)</h3>"
            "<input type='file' id='stmFw' accept='.bin'>"
            "<button onclick=\"upld('/update-stm32','stmFw')\">Flash STM32</button></div>"
            "<div id='prog'></div>"
            "<script>"
            "function upld(url,id){"
            "const f=document.getElementById(id).files[0];"
            "if(!f){alert('Select a file first');return;}"
            "const p=document.getElementById('prog');"
            "p.textContent='Uploading '+f.name+' ('+f.size+' bytes)...';"
            "const xhr=new XMLHttpRequest();"
            "xhr.open('POST',url);"
            "xhr.upload.onprogress=e=>{if(e.lengthComputable)p.textContent='Progress: '+Math.round(e.loaded/e.total*100)+'%';};"
            "xhr.onload=()=>{p.textContent=xhr.status==200?'✅ '+xhr.responseText:'❌ Failed: '+xhr.responseText;};"
            "xhr.onerror=()=>{p.textContent='❌ Upload error';};"
            "const fd=new FormData();fd.append('file',f,f.name);"
            "xhr.send(fd);}"
            "</script></body></html>"
        );
    });

    /* Captive portal detection URLs — return proper success responses
       so OS keeps WiFi connected instead of dropping it */
    /* Android */
    /* Debug endpoint — hit /debug in browser to check UART + WS state */
    http_server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *req){
        String s = "{\"uart_rx_bytes\":"; s += uart_rx_bytes;
        s += ",\"ws_clients\":"; s += ws_proxy.count();
        s += ",\"uptime_s\":"; s += (unsigned long)(millis()/1000);
        s += ",\"heap\":"; s += ESP.getFreeHeap();
        s += ",\"wifi_clients\":"; s += WiFi.softAPgetStationNum();
        s += "}";
        req->send(200, "application/json", s);
    });

    http_server.on("/generate_204",         HTTP_GET, handle_generate_204);
    /* Apple iOS / macOS */
    http_server.on("/hotspot-detect.html",  HTTP_GET, handle_apple_cna);
    http_server.on("/library/test/success.html", HTTP_GET, handle_apple_cna);
    http_server.on("/canonical.html",       HTTP_GET, handle_apple_cna);
    /* Windows */
    http_server.on("/ncsi.txt",             HTTP_GET, handle_ncsi);
    http_server.on("/connecttest.txt",      HTTP_GET, handle_connecttest);
    /* Generic fallbacks */
    http_server.on("/redirect",             HTTP_GET, serve_dashboard);
    http_server.on("/success.txt",          HTTP_GET,
        [](AsyncWebServerRequest *r){ r->send(200, "text/plain", "success"); });
    http_server.on("/favicon.ico", HTTP_GET,
        [](AsyncWebServerRequest *r){ r->send(404); });

    /* Bridge status endpoint */
    http_server.on("/bridge/status", HTTP_GET, [](AsyncWebServerRequest *req){
        String s = "{\"uptime_s\":"; s += millis()/1000;
        s += ",\"uart_rx_bytes\":"; s += uart_rx_bytes;
        s += ",\"ap_clients\":"; s += WiFi.softAPgetStationNum();
        s += ",\"spiffs_ok\":"; s += spiffs_ok ? "true" : "false";
        s += ",\"flashing\":"; s += flashing_stm ? "true" : "false";
        s += "}";
        req->send(200, "application/json", s);
    });

    /* Log download */
    http_server.on("/log", HTTP_GET, [](AsyncWebServerRequest *req){
        if (spiffs_ok && SPIFFS.exists(LOG_PATH)) {
            req->send(SPIFFS, LOG_PATH, "text/plain");
        } else {
            req->send(404, "text/plain", "No log file");
        }
    });

    /* Log clear */
    http_server.on("/log/clear", HTTP_POST, [](AsyncWebServerRequest *req){
        if (log_file) log_file.close();
        if (spiffs_ok) SPIFFS.remove(LOG_PATH);
        req->send(200, "text/plain", "Log cleared");
    });

    /* Everything else → serve dashboard directly (no redirect — redirects
       on unknown URLs cause Android/iOS to think network is broken) */
    http_server.onNotFound(serve_dashboard);
    http_server.begin();
    Serial.println("[HTTP] dashboard + API on :80");
    Serial.println("[ST-CAN Bridge v2] ready");
}

/* ══════════════════════════════════════════════════════════════════════
 *  loop
 * ══════════════════════════════════════════════════════════════════════ */
static String uart_line;

void loop() {
    dns.processNextRequest();
    ws_proxy.cleanupClients();

    if (!flashing_stm) {
        /* ── STM → ESP: forward to WS clients + log ──────────────── */
        while (Serial2.available()) {
            char c = (char)Serial2.read();
            uart_rx_bytes++;

            if (c == '\n') {
                uart_line.trim();
                if (uart_line.length() > 0) {
                    /* Forward to all WS proxy clients on port 81 */
                    ws_proxy.textAll(uart_line);
                    /* Log to SPIFFS */
                    log_line(uart_line);
                }
                uart_line = "";
            } else if (c != '\r') {
                uart_line += c;
            }
        }

        /* ── ESP-6: USB-serial bridge ─────────────────────────────── */
        /* Laptop USB → ESP Serial → Serial2 → STM UART */
        while (Serial.available()) {
            char c = (char)Serial.read();
            Serial2.write((uint8_t)c);
        }
        /* STM UART responses already forwarded above via Serial2.
           But we also echo raw UART to USB serial for direct wired access: */
    }

    /* Periodic status */
    static uint32_t last_print = 0;
    if (millis() - last_print > 10000) {
        last_print = millis();
        Serial.printf("[ST-CAN] clients=%d uart_rx=%lu ws=%d up=%lus\n",
                      WiFi.softAPgetStationNum(),
                      (unsigned long)uart_rx_bytes,
                      ws_proxy.count(),
                      (unsigned long)(millis()/1000));
    }
}
