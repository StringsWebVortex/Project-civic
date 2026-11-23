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
static const size_t UART_RX_BUFFER_SIZE = 8192u;
static const size_t UART_LINE_MAX = 2048u;
static const size_t WS_COMMAND_MAX = 1400u;
static const size_t WS_PROXY_CLIENTS = 4u;

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
static uint32_t uart_rx_lines = 0;
static uint32_t uart_json_lines = 0;
static uint32_t uart_ignored_lines = 0;
static uint32_t uart_line_overflows = 0;
static uint32_t stm32_uart_tx_dropped = 0;
static uint32_t stm32_uart_tx_dma_errors = 0;
static uint32_t stm32_uart_dma_te = 0;
static uint32_t stm32_uart_dma_dme = 0;
static uint32_t stm32_uart_dma_fe = 0;
static uint32_t stm32_uart_dma_last_flags = 0;
static uint32_t stm32_uart_rx_dropped = 0;
static uint32_t stm32_gvret_frames_sent = 0;
static uint32_t stm32_gvret_frames_dropped = 0;
static uint32_t stm32_dbc_messages = 0;
static uint32_t stm32_diag_last_ms = 0;
static uint32_t stm32_diag_records = 0;
static uint32_t stm32_diag_parse_failures = 0;
static uint32_t stm32_diag_last_length = 0;
static bool stm32_gvret_client = false;
static bool stm32_diag_seen = false;
static uint32_t log_dropped_lines = 0;
static volatile uint8_t ws_proxy_clients = 0;
static uint32_t ws_last_connect_heartbeat = 0;
static String uart_line;
static bool uart_line_overflow = false;
static char ws_command[WS_PROXY_CLIENTS][WS_COMMAND_MAX];
static uint16_t ws_command_total[WS_PROXY_CLIENTS] = {0};
static bool ws_command_active[WS_PROXY_CLIENTS] = {false};
static uint32_t ws_client_ids[WS_PROXY_CLIENTS] = {
    UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX
};

static int ws_client_slot(uint32_t client_id) {
    for (size_t i = 0; i < WS_PROXY_CLIENTS; ++i)
        if (ws_client_ids[i] == client_id) return (int)i;
    return -1;
}

static int ws_client_slot_allocate(uint32_t client_id) {
    const int existing = ws_client_slot(client_id);
    if (existing >= 0) return existing;
    for (size_t i = 0; i < WS_PROXY_CLIENTS; ++i) {
        if (ws_client_ids[i] == UINT32_MAX) {
            ws_client_ids[i] = client_id;
            ws_command_active[i] = false;
            ws_command_total[i] = 0u;
            return (int)i;
        }
    }
    return -1;
}
static bool json_u32_field(const String &json, const char *key, uint32_t *value) {
    String token = "\"";
    token += key;
    token += "\":";
    int at = json.indexOf(token);
    if (at < 0) return false;
    int pos = at + token.length();
    while (pos < (int)json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= (int)json.length() || json[pos] < '0' || json[pos] > '9') return false;
    uint32_t parsed = 0u;
    while (pos < (int)json.length() && json[pos] >= '0' && json[pos] <= '9') {
        const uint32_t digit = (uint32_t)(json[pos] - '0');
        if (parsed > (UINT32_MAX - digit) / 10u) return false;
        parsed = parsed * 10u + digit;
        pos++;
    }
    if (pos < (int)json.length() && json[pos] != ',' && json[pos] != '}') return false;
    *value = parsed;
    return true;
}

static void update_pipeline_diag(const String &json) {
    if (json.indexOf("\"type\":\"pipeline_diag\"") < 0) return;
    stm32_diag_records++;
    stm32_diag_last_length = (uint32_t)json.length();
    uint32_t tx_dropped, dma_errors, dma_te, dma_dme, dma_fe, dma_flags;
    uint32_t rx_dropped, gvret_client, gvret_sent, gvret_dropped, dbc_messages;
    const bool parsed =
        json_u32_field(json, "txd", &tx_dropped) &&
        json_u32_field(json, "dma", &dma_errors) &&
        json_u32_field(json, "te", &dma_te) &&
        json_u32_field(json, "dme", &dma_dme) &&
        json_u32_field(json, "fe", &dma_fe) &&
        json_u32_field(json, "flg", &dma_flags) &&
        json_u32_field(json, "rxd", &rx_dropped) &&
        json_u32_field(json, "gv", &gvret_client) &&
        json_u32_field(json, "sent", &gvret_sent) &&
        json_u32_field(json, "drop", &gvret_dropped) &&
        json_u32_field(json, "dbc", &dbc_messages);
    if (!parsed) {
        stm32_diag_parse_failures++;
        Serial.printf("[PIPE-DIAG] parse failed len=%lu: %s\n",
                      (unsigned long)stm32_diag_last_length, json.c_str());
        return;
    }
    stm32_uart_tx_dropped = tx_dropped;
    stm32_uart_tx_dma_errors = dma_errors;
    stm32_uart_dma_te = dma_te;
    stm32_uart_dma_dme = dma_dme;
    stm32_uart_dma_fe = dma_fe;
    stm32_uart_dma_last_flags = dma_flags;
    stm32_uart_rx_dropped = rx_dropped;
    stm32_gvret_frames_sent = gvret_sent;
    stm32_gvret_frames_dropped = gvret_dropped;
    stm32_dbc_messages = dbc_messages;
    stm32_gvret_client = gvret_client != 0u;
    stm32_diag_last_ms = millis();
    stm32_diag_seen = true;
}

static char log_buffer[8192];
static size_t log_buffer_len = 0;
static uint32_t log_last_flush = 0;
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
    const uint32_t client_id = client ? client->id() : UINT32_MAX;
    if (type == WS_EVT_CONNECT) {
        const int slot = ws_client_slot_allocate(client_id);
        if (slot < 0) {
            Serial.printf("[WS] client #%u rejected: bridge slots full\n", client_id);
            if (client) client->close();
            return;
        }
        Serial.printf("[WS] client #%u connected in slot %d\n", client_id, slot);
        if (ws_proxy_clients < WS_PROXY_CLIENTS) ws_proxy_clients++;
        if (ws_proxy_clients == 1u) {
            Serial2.println("WS,CONNECT");
            ws_last_connect_heartbeat = millis();
        }
    } else if (type == WS_EVT_DISCONNECT) {
        const int slot = ws_client_slot(client_id);
        Serial.printf("[WS] client #%u disconnected\n", client_id);
        if (slot >= 0) {
            ws_command_active[slot] = false;
            ws_command_total[slot] = 0u;
            ws_client_ids[slot] = UINT32_MAX;
            if (ws_proxy_clients > 0u) ws_proxy_clients--;
        }
        if (ws_proxy_clients == 0u) Serial2.println("WS,DISCONNECT");
    } else if (type == WS_EVT_DATA && !flashing_stm) {
        const int slot = ws_client_slot(client_id);
        if (slot < 0) return;
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (!info || info->opcode != WS_TEXT || info->len > WS_COMMAND_MAX ||
            info->index + len > info->len) {
            ws_command_active[slot] = false;
            return;
        }
        if (info->index == 0u) {
            ws_command_total[slot] = (uint16_t)info->len;
            ws_command_active[slot] = true;
        }
        if (!ws_command_active[slot] || ws_command_total[slot] != info->len) return;
        memcpy(ws_command[slot] + info->index, data, len);
        if (info->final && info->index + len == info->len) {
            Serial2.write((const uint8_t *)ws_command[slot], ws_command_total[slot]);
            Serial2.write('\n');
            ws_command_active[slot] = false;
            ws_command_total[slot] = 0u;
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 *  ESP-5: SPIFFS circular log (kept from original — no changes)
 * ══════════════════════════════════════════════════════════════════════ */
static void log_line(const String &line) {
    if (!spiffs_ok || line.length() == 0u) return;
    const size_t n = line.length();
    if (n + 1u > sizeof log_buffer - log_buffer_len) {
        log_dropped_lines++;
        return; /* logging is lossy by design; never stall the live UART path */
    }
    memcpy(log_buffer + log_buffer_len, line.c_str(), n);
    log_buffer_len += n;
    log_buffer[log_buffer_len++] = '\n';
}

static void log_flush(void) {
    if (!spiffs_ok || log_buffer_len == 0u || millis() - log_last_flush < 1000u) return;
    log_last_flush = millis();
    if (!log_file) log_file = SPIFFS.open(LOG_PATH, FILE_APPEND);
    if (!log_file) return;
    const size_t written = log_file.write((const uint8_t *)log_buffer, log_buffer_len);
    if (written > 0u) {
        log_file.flush();
        if (written < log_buffer_len)
            memmove(log_buffer, log_buffer + written, log_buffer_len - written);
        log_buffer_len -= written;
    }
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
    Serial2.setRxBufferSize(UART_RX_BUFFER_SIZE);
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
    Serial2.setRxBufferSize(UART_RX_BUFFER_SIZE);
    Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    if (ws_proxy_clients > 0u) Serial2.println("WS,CONNECT");
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

    /* UART2 to STM — reserve enough RX space for a full JSON batch at 2 Mbaud. */
    uart_line.reserve(UART_LINE_MAX);
    Serial2.setRxBufferSize(UART_RX_BUFFER_SIZE);
    Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    Serial.printf("[UART] RX=GPIO%d TX=GPIO%d baud=%ld rxbuf=%u\n",
                  UART_RX_PIN, UART_TX_PIN, UART_BAUD, (unsigned)UART_RX_BUFFER_SIZE);

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
    /* Single diagnostics endpoint for the complete UART → WebSocket pipeline. */
    http_server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *req){
        String s = "{\"uart_rx_bytes\":"; s += (unsigned long)uart_rx_bytes;
        s += ",\"uart_rx_lines\":"; s += (unsigned long)uart_rx_lines;
        s += ",\"uart_json_lines\":"; s += (unsigned long)uart_json_lines;
        s += ",\"uart_ignored_lines\":"; s += (unsigned long)uart_ignored_lines;
        s += ",\"uart_line_overflows\":"; s += (unsigned long)uart_line_overflows;
        s += ",\"ws_clients\":"; s += ws_proxy.count();
        s += ",\"stm32_diag_seen\":"; s += stm32_diag_seen ? "true" : "false";
        s += ",\"stm32_diag_age_s\":"; s += stm32_diag_seen ? (long)((millis()-stm32_diag_last_ms)/1000u) : -1L;
        s += ",\"stm32_diag_records\":"; s += stm32_diag_records;
        s += ",\"stm32_diag_parse_failures\":"; s += stm32_diag_parse_failures;
        s += ",\"stm32_uart_tx_dropped\":"; s += stm32_uart_tx_dropped;
        s += ",\"stm32_uart_tx_dma_errors\":"; s += stm32_uart_tx_dma_errors;
        s += ",\"stm32_uart_dma_te\":"; s += stm32_uart_dma_te;
        s += ",\"stm32_uart_dma_dme\":"; s += stm32_uart_dma_dme;
        s += ",\"stm32_uart_dma_fe\":"; s += stm32_uart_dma_fe;
        s += ",\"stm32_uart_dma_last_flags\":"; s += stm32_uart_dma_last_flags;
        s += ",\"stm32_uart_rx_dropped\":"; s += stm32_uart_rx_dropped;
        s += ",\"gvret_client\":"; s += stm32_gvret_client ? "true" : "false";
        s += ",\"gvret_frames_sent\":"; s += stm32_gvret_frames_sent;
        s += ",\"gvret_frames_dropped\":"; s += stm32_gvret_frames_dropped;
        s += ",\"dbc_messages\":"; s += stm32_dbc_messages;
        s += ",\"uptime_s\":"; s += (unsigned long)(millis()/1000u);
        s += ",\"heap\":"; s += ESP.getFreeHeap();
        s += ",\"wifi_clients\":"; s += WiFi.softAPgetStationNum();
        s += ",\"log_dropped_lines\":"; s += (unsigned long)log_dropped_lines;
        s += ",\"log_buffered_bytes\":"; s += (unsigned long)log_buffer_len;
        s += ",\"spiffs_ok\":"; s += spiffs_ok ? "true" : "false";
        s += ",\"flashing_stm\":"; s += flashing_stm ? "true" : "false";
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
        s += ",\"stm32_diag_seen\":"; s += stm32_diag_seen ? "true" : "false";
        s += ",\"stm32_uart_tx_dropped\":"; s += stm32_uart_tx_dropped;
        s += ",\"stm32_uart_tx_dma_errors\":"; s += stm32_uart_tx_dma_errors;
        s += ",\"stm32_uart_dma_te\":"; s += stm32_uart_dma_te;
        s += ",\"stm32_uart_dma_dme\":"; s += stm32_uart_dma_dme;
        s += ",\"stm32_uart_dma_fe\":"; s += stm32_uart_dma_fe;
        s += ",\"stm32_uart_dma_last_flags\":"; s += stm32_uart_dma_last_flags;
        s += ",\"stm32_uart_rx_dropped\":"; s += stm32_uart_rx_dropped;
        s += ",\"gvret_frames_dropped\":"; s += stm32_gvret_frames_dropped;
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
void loop() {
    dns.processNextRequest();
    ws_proxy.cleanupClients();

    if (!flashing_stm) {
        /* Re-announce the virtual WS peer after an STM32 reset. STM ignores
           duplicate CONNECT markers while its virtual peer is already active. */
        if (ws_proxy_clients > 0u && millis() - ws_last_connect_heartbeat >= 5000u) {
            Serial2.println("WS,CONNECT");
            ws_last_connect_heartbeat = millis();
        }

        /* STM → ESP: only JSON is a dashboard message. Raw CSV/status lines
           remain available to the wired USB serial bridge but are not sent to
           a browser whose message contract is JSON. */
        while (Serial2.available()) {
            const int value = Serial2.read();
            if (value < 0) break;
            const char c = (char)value;
            uart_rx_bytes++;

            if (c == '\n') {
                uart_rx_lines++;
                if (!uart_line_overflow) {
                    uart_line.trim();
                    if (uart_line.length() > 0u && uart_line[0] == '{' &&
                        uart_line[uart_line.length() - 1u] == '}') {
                        update_pipeline_diag(uart_line);
                        if (ws_proxy_clients > 0u) ws_proxy.textAll(uart_line);
                        uart_json_lines++;
                        log_line(uart_line);
                    } else if (uart_line.length() > 0u) {
                        uart_ignored_lines++;
                    }
                }
                uart_line = "";
                uart_line_overflow = false;
            } else if (c != '\r' && !uart_line_overflow) {
                if (uart_line.length() < UART_LINE_MAX - 1u) {
                    uart_line += c;
                } else {
                    uart_line = "";
                    uart_line_overflow = true;
                    uart_line_overflows++;
                }
            }
        }

        /* Laptop USB → ESP Serial → Serial2 → STM UART. */
        while (Serial.available()) {
            const char c = (char)Serial.read();
            Serial2.write((uint8_t)c);
        }
        log_flush();
    }

    static uint32_t last_print = 0;
    if (millis() - last_print > 10000u) {
        last_print = millis();
        Serial.printf("[ST-CAN] clients=%d uart_rx=%lu lines=%lu json=%lu ignored=%lu overflow=%lu ws=%u log_drop=%lu up=%lus\n",
                      WiFi.softAPgetStationNum(),
                      (unsigned long)uart_rx_bytes,
                      (unsigned long)uart_rx_lines,
                      (unsigned long)uart_json_lines,
                      (unsigned long)uart_ignored_lines,
                      (unsigned long)uart_line_overflows,
                      (unsigned)ws_proxy_clients,
                      (unsigned long)log_dropped_lines,
                      (unsigned long)(millis()/1000u));
        Serial.printf("[PIPE] seen=%u records=%lu parse_fail=%lu len=%lu tx_drop=%lu dma=%lu\n",
                      (unsigned)stm32_diag_seen,
                      (unsigned long)stm32_diag_records,
                      (unsigned long)stm32_diag_parse_failures,
                      (unsigned long)stm32_diag_last_length,
                      (unsigned long)stm32_uart_tx_dropped,
                      (unsigned long)stm32_uart_tx_dma_errors);
        Serial.printf("[PIPE-DMA] TE=%lu DME=%lu FE=%lu flags=0x%08lx rx_drop=%lu gvret=%u sent=%lu drop=%lu dbc=%lu\n",
                      (unsigned long)stm32_uart_dma_te,
                      (unsigned long)stm32_uart_dma_dme,
                      (unsigned long)stm32_uart_dma_fe,
                      (unsigned long)stm32_uart_dma_last_flags,
                      (unsigned long)stm32_uart_rx_dropped,
                      (unsigned)stm32_gvret_client,
                      (unsigned long)stm32_gvret_frames_sent,
                      (unsigned long)stm32_gvret_frames_dropped,
                      (unsigned long)stm32_dbc_messages);
    }
}
