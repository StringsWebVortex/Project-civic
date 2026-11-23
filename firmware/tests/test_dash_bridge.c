#define _GNU_SOURCE
/* Host tests for dash.c: the STM32-resident port of the ESP32 dashboard.
 * Checks that STM32 output lines become exactly the WebSocket JSON the
 * ESP32 produced (ArduinoJson formatting), that dashboard commands become
 * exactly the STM32 command lines the ESP32 sent, and the DTC journal,
 * GVRET TCP and log/observer behaviour. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dash.h"
#include "dash_store.h"
static void get(const char *method, const char *path, const char *query);
#include "log_mail.h"
#include "i2c1.h"
#include "web_server.h"
#include "lwip/tcp.h"
#include "stm32f4xx.h"

void fake_flash_reset(void);
volatile uint32_t system_millis;
shim_tim_t shim_tim2; shim_rng_t shim_rng; shim_rcc_t shim_rcc;
const uint8_t dash_page_gz[4] = { 0x1f, 0x8b, 0, 0 };
const size_t dash_page_gz_len = 4;

static int failures;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, m); failures++; } } while (0)

/* ── captured outputs ── */
#define MAXMSG 512
static char msgs[MAXMSG][1700]; static int nmsg;
static char cmds[64][1200]; static int ncmd;
static void cmd_cb(const char *line) { if (ncmd < 64) snprintf(cmds[ncmd++], sizeof cmds[0], "%s", line); }
static void clear(void) { nmsg = 0; ncmd = 0; }
static bool has_msg(const char *s) { for (int i = 0; i < nmsg; i++) if (strcmp(msgs[i], s) == 0) return true; return false; }
static bool msg_contains(const char *s) { for (int i = 0; i < nmsg; i++) if (strstr(msgs[i], s)) return true; return false; }
static bool has_cmd(const char *s) { for (int i = 0; i < ncmd; i++) if (strcmp(cmds[i], s) == 0) return true; return false; }

/* web_server stubs */
static uint8_t clients = 1;
static bool ws_fail_broadcast;
static uint32_t test_uart_tx_dropped, test_uart_tx_dma_errors, test_uart_rx_dropped;
static uint32_t test_uart_dma_te, test_uart_dma_dme, test_uart_dma_fe, test_uart_dma_last;
uint32_t uart_esp_tx_dropped(void) { return test_uart_tx_dropped; }
uint32_t uart_esp_tx_dma_errors(void) { return test_uart_tx_dma_errors; }
uint32_t uart_esp_tx_dma_transfer_errors(void) { return test_uart_dma_te; }
uint32_t uart_esp_tx_dma_direct_errors(void) { return test_uart_dma_dme; }
uint32_t uart_esp_tx_dma_fifo_errors(void) { return test_uart_dma_fe; }
uint32_t uart_esp_tx_dma_last_flags(void) { return test_uart_dma_last; }
uint32_t uart_esp_rx_dropped(void) { return test_uart_rx_dropped; }
bool ws_broadcast_txt(const char *s, size_t n)
{
    if (ws_fail_broadcast) return false;
    if (n >= sizeof msgs[0]) return false;
    if (nmsg < MAXMSG) { memcpy(msgs[nmsg], s, n); msgs[nmsg++][n] = 0; }
    return true;
}
void ws_send_txt(uint8_t num, const char *s, size_t n) { (void)num; ws_broadcast_txt(s, n); }
bool ws_send_txt_ok(uint8_t num, const char *s, size_t n) { ws_send_txt(num, s, n); return true; }
uint8_t ws_connected_clients(void) { return clients; }
bool web_server_init(void) { return true; }
void web_server_poll(void) { }
static int http_status; static char http_body[4096]; static size_t http_len;
static http_gen_fn http_gen; static void *http_ctx;
void http_respond(http_conn_t *c, int status, const char *ctype, const char *extra, const void *body, size_t len)
{ (void)c; (void)ctype; (void)extra; http_status = status; http_len = len < sizeof http_body ? len : sizeof http_body - 1; memcpy(http_body, body, http_len); http_body[http_len] = 0; http_gen = NULL; }
void http_respond_text(http_conn_t *c, int status, const char *text) { http_respond(c, status, "text/plain", NULL, text, strlen(text)); }
void http_respond_stream(http_conn_t *c, int status, const char *ctype, const char *extra, http_gen_fn gen, void *ctx)
{ (void)c; (void)ctype; (void)extra; http_status = status; http_gen = gen; http_ctx = ctx; http_len = 0;
  uint8_t buf[256]; size_t n; while ((n = gen(ctx, buf, sizeof buf)) > 0 && http_len + n < sizeof http_body) { memcpy(http_body + http_len, buf, n); http_len += n; } http_body[http_len] = 0; }
bool http_query_get(const http_req_t *r, const char *key, char *out, size_t cap)
{ const char *p = strstr(r->query, key); if (!p) return false; p += strlen(key); if (*p == '=') p++; snprintf(out, cap, "%s", p); return true; }

/* 24LC512 model on the I2C1 bus (address 0x50) */
static uint8_t ee_mem[65536]; static int ee_present = 1; static int ee_write_fail;
bool i2c1_init(void) { return true; }
uint32_t i2c1_now_ms(void) { return system_millis; }
bool i2c1_write(uint8_t a, const uint8_t *w1, size_t n1, const uint8_t *w2, size_t n2)
{
    if (!ee_present || ee_write_fail || a != 0x50) return false;
    if (n1 + n2 == 0) return true;
    const unsigned at = (unsigned)(w1[0] << 8 | w1[1]);
    for (size_t i = 0; i < n2; i++) ee_mem[(at & ~127u) | ((at + i) & 127u)] = w2[i];
    return true;
}
bool i2c1_read(uint8_t a, const uint8_t *w, size_t nw, uint8_t *r, size_t nr)
{
    if (!ee_present || a != 0x50 || nw != 2) return false;
    const unsigned at = (unsigned)(w[0] << 8 | w[1]);
    for (size_t i = 0; i < nr; i++) r[i] = ee_mem[(at + i) & 0xFFFFu];
    return true;
}

/* lwIP stubs */
static struct tcp_pcb pcbs[64]; static int npcb;
static tcp_accept_fn accept_fn[65536];
static uint16_t last_bind;
static uint8_t gv_out[4096]; static size_t gv_len; static struct tcp_pcb *gv_pcb;
static u16_t gv_sndbuf = 8000u;
static err_t gv_write_result = ERR_OK;
u16_t tcp_sndbuf(struct tcp_pcb *pcb) { (void)pcb; return gv_sndbuf; }
err_t tcp_write(struct tcp_pcb *pcb, const void *data, u16_t len, u8_t flags)
{ (void)flags; if (gv_write_result != ERR_OK) return gv_write_result; if (pcb == gv_pcb && gv_len + len < sizeof gv_out) { memcpy(gv_out + gv_len, data, len); gv_len += len; } return ERR_OK; }
err_t tcp_output(struct tcp_pcb *pcb) { (void)pcb; return ERR_OK; }
void  tcp_arg(struct tcp_pcb *pcb, void *arg) { (void)pcb; (void)arg; }
static tcp_recv_fn recv_fn[64];
void  tcp_recv(struct tcp_pcb *pcb, tcp_recv_fn f) { if (f) recv_fn[pcb->id] = f; }
static tcp_err_fn err_fn[64];
void  tcp_err(struct tcp_pcb *pcb, tcp_err_fn f) { if (f) err_fn[pcb->id] = f; }
static int close_fail;
err_t tcp_close(struct tcp_pcb *pcb) { (void)pcb; return close_fail ? ERR_VAL : ERR_OK; }
void  tcp_abort(struct tcp_pcb *pcb) { (void)pcb; }
void  tcp_recved(struct tcp_pcb *pcb, u16_t len) { (void)pcb; (void)len; }
void  tcp_nagle_disable(struct tcp_pcb *pcb) { (void)pcb; }
struct tcp_pcb *tcp_new_ip_type(u8_t type) { (void)type; pcbs[npcb].id = npcb; return &pcbs[npcb++]; }
err_t tcp_bind(struct tcp_pcb *pcb, const void *ip, u16_t port) { (void)pcb; (void)ip; last_bind = port; return ERR_OK; }
struct tcp_pcb *tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t backlog) { (void)backlog; return pcb; }
void  tcp_accept(struct tcp_pcb *pcb, tcp_accept_fn f) { (void)pcb; accept_fn[last_bind] = f; }
u8_t  pbuf_free(struct pbuf *p) { (void)p; return 1; }

static void ws(const char *json) { char b[1500]; snprintf(b, sizeof b, "%s", json); dash_ws_text(0, b, strlen(b)); }
static void tick(uint32_t ms) { for (uint32_t i = 0; i < ms; i += 50) { system_millis += 50; dash_poll(); } }

static void test_status_lines(void)
{
    clear();
    dash_on_line("STATUS,SYS,VEH,RUN,750,1234,1,OBD\r\n");
    CHECK(has_msg("{\"type\":\"veh\",\"state\":\"RUN\",\"rpm\":750,\"spd\":12.34,\"acc\":1,\"src\":\"OBD\"}"), "VEH -> veh JSON");
    clear();
    dash_on_line("STATUS,F,LOCK,500000\r\n");
    CHECK(has_msg("{\"type\":\"status\",\"locked\":true,\"bitrate\":500000,\"frames\":0,\"dwell\":0}"), "LOCK -> status JSON");
    CHECK(has_msg("{\"type\":\"diag\",\"message\":\"STATUS,F,LOCK,500000\"}"), "LOCK also logged as diag");
    clear();
    dash_on_line("STATUS,B,STATS,1,2,3\r\n");
    CHECK(has_msg("{\"type\":\"stats\",\"bus\":\"B\",\"message\":\"1,2,3\"}"), "STATS JSON");
    clear();
    dash_on_line("TX,ERR,NOT_ARMED\r\n");
    CHECK(has_msg("{\"type\":\"tx\",\"message\":\"TX,ERR,NOT_ARMED\"}"), "TX line");
    clear();
    dash_on_line("CMD,ERR,\"q\"\r\n");
    CHECK(has_msg("{\"type\":\"diag\",\"message\":\"CMD,ERR,\\\"q\\\"\"}"), "JSON escaping of quotes");
}

static void test_pid_profile_sig(void)
{
    clear();
    dash_on_line("PID,0c,Engine RPM,rpm,750.25\r\n");
    CHECK(has_msg("{\"type\":\"pid\",\"pid\":\"0C\",\"label\":\"Engine RPM\",\"unit\":\"rpm\",\"value\":750.2500}"), "PID JSON");
    clear();
    dash_on_line("PROFILE,1a0,8,250,ff00,ENG\r\n");
    CHECK(has_msg("{\"type\":\"profile\",\"id\":\"1A0\",\"dlc\":8,\"rate\":25.0,\"var_mask\":\"FF00\",\"label\":\"ENG\"}"), "PROFILE JSON");
    clear();
    dash_on_line("SIG,1a0,2,0a,00,ff,125,X\r\n");
    CHECK(has_msg("{\"type\":\"sig\",\"id\":\"1A0\",\"byte\":2,\"last\":\"0A\",\"min\":\"00\",\"max\":\"FF\",\"rate\":12.5,\"label\":\"X\"}"), "SIG JSON");
}

static void test_frames_and_gvret(void)
{
    /* SavvyCAN connects on TCP 23 */
    gv_pcb = &pcbs[npcb]; pcbs[npcb].id = npcb; npcb++;
    CHECK(accept_fn[23] && accept_fn[23](NULL, gv_pcb, ERR_OK) == ERR_OK, "GVRET client accepted");
    gv_len = 0; clear();
    dash_on_line("123456,STD,123,2,0,A1B2,0\n");
    dash_on_line("123457,EXT,18DAF10E,1,0,7F,1\n");
    CHECK(gv_len > 0 && gv_out[0] == 0xF1, "GVRET frame streamed to SavvyCAN");
    tick(1100);
    CHECK(msg_contains("{\"type\":\"frames\",") && msg_contains("[\"123\",0,2,\"A1B2\",1,") && msg_contains("[\"18DAF10E\",1,1,\"7F\",1,"),
          "frames batch with per-ID table");
    CHECK(msg_contains("\"drop\":[0,0],\"over\":0,"), "frames batch reports capture-loss counters");
    /* GVRET BUILD_CAN_FRAME from SavvyCAN -> STM32 TX command (armed-only on STM32) */
    clear();
    uint8_t pkt[] = { 0xF1, 0x00, 0x23, 0x01, 0x00, 0x00, 0x00, 0x02, 0xAA, 0xBB };
    struct pbuf p = { NULL, pkt, sizeof pkt, sizeof pkt };
    recv_fn[gv_pcb->id](NULL, gv_pcb, &p, ERR_OK);
    CHECK(ncmd == 1 && strncmp(cmds[0], "TX,F,S,123,0,2,AABB", 19) == 0, "GVRET send -> TX command");


}

static void test_uart_batch_retry(void)
{
    clear();
    dash_on_line("300000,STD,124,8,0,0102030405060708,0\n");
    ws_fail_broadcast = true;
    tick(600);
    ws_fail_broadcast = false;
    clear();
    tick(600);
    CHECK(msg_contains("[\"124\",0,8,\"0102030405060708\""),
          "frame batch remains dirty and retries after UART queue backpressure");
    clear();
    dash_on_line("300001,STD,17C,8,0,400002EE80000000,0\n");
    ws_fail_broadcast = true;
    tick(600);
    ws_fail_broadcast = false;
    clear();
    tick(600);
    CHECK(msg_contains("{\"type\":\"dec\",\"l\":[") &&
          msg_contains("ENGINE_RPM"), "decoded batch retries after UART queue backpressure");
}

static void test_dec_batch(void)
{
    clear();
    dash_on_line("200000,STD,1A3,8,0,0003000004000000,0\n");   /* GEARBOX_AUTO: target 3rd, shifter D */
    dash_on_line("200001,STD,17C,8,0,400002EE80000000,0\n");   /* POWERTRAIN_DATA: RPM 750 */
    tick(600);
    CHECK(msg_contains("{\"type\":\"dec\",\"l\":["), "dec batch sent");
    CHECK(msg_contains("[\"1A3\",\"GEARBOX_AUTO\",\"GEAR_SHIFTER\",4,\"\",\"D\"]"), "gear shifter decoded with value name");
    CHECK(msg_contains("[\"1A3\",\"GEARBOX_AUTO\",\"TRANS_TARGET_GEAR\",3,\"\",\"3rd\"]"), "target gear decoded");
    CHECK(msg_contains("[\"17C\",\"POWERTRAIN_DATA\",\"ENGINE_RPM\",750,\"rpm\",\"\"]"), "rpm decoded with unit");
    CHECK(!msg_contains("CHECKSUM") && !msg_contains("COUNTER"), "counter/checksum fields skipped");
    clear(); tick(600);
    CHECK(!msg_contains("\"type\":\"dec\""), "unchanged frames are not re-sent");

    clear();
    dash_on_line("200002,STD,17C,8,0,400002EE80000000,1\n");
    dash_on_line("200003,EXT,18DAF10E,8,0,0102030405060708,0\n");
    dash_on_line("200004,STD,123,8,0,0102030405060708,0\n");
    tick(600);
    CHECK(!msg_contains("\"type\":\"dec\""),
          "B-CAN, extended frames and undefined standard IDs remain raw-only in DBC decoding");
    clear(); dash_on_line("GEAR,BCAST,2,3,UP\r\n");
    CHECK(has_msg("{\"type\":\"diag\",\"message\":\"GEAR,BCAST,2,3,UP\"}"), "broadcast gear ack reaches the page log");
}

static void test_ws_commands(void)
{
    clear(); ws("{\"op\":\"arm\",\"bus\":\"F\",\"seconds\":30}");
    CHECK(has_cmd("ARM,F,30"), "arm");
    clear(); ws("{\"op\":\"arm\",\"bus\":\"F\",\"seconds\":0}");
    CHECK(ncmd == 0 && has_msg("{\"type\":\"diag\",\"message\":\"CMD,ERR,ESP32_BAD_ARM\"}"), "arm validation");
    clear(); ws("{\"op\":\"disarm\",\"bus\":\"B\"}");             CHECK(has_cmd("DISARM,B"), "disarm");
    clear(); ws("{\"op\":\"tx\",\"bus\":\"B\",\"format\":\"S\",\"id\":\"123\",\"dlc\":2,\"rtr\":0,\"data\":\"a1b2\"}");
    CHECK(has_cmd("TX,B,S,123,0,2,A1B2"), "tx");
    clear(); ws("{\"op\":\"tx\",\"format\":\"S\",\"id\":\"800\",\"dlc\":0,\"rtr\":0,\"data\":\"\"}");
    CHECK(ncmd == 0 && has_msg("{\"type\":\"tx\",\"message\":\"TX,ERR,ESP32_VALIDATION_FAILED\"}"), "tx id > 0x7FF rejected");
    clear(); ws("{\"op\":\"uds\",\"bus\":\"F\",\"format\":\"E\",\"tx\":\"18DA0EF1\",\"rx\":\"18DAF10E\",\"req\":\"222612\"}");
    CHECK(has_cmd("UDS,F,E,18DA0EF1,18DAF10E,222612"), "uds (DID 2612 read)");
    clear(); ws("{\"op\":\"uds\",\"bus\":\"F\",\"format\":\"S\",\"tx\":\"7E0\",\"rx\":\"7E8\",\"req\":\"1101\",\"confirm\":true}");
    CHECK(has_cmd("UDS,F,S,7E0,7E8,1101,CONFIRM"), "uds confirm");
    clear(); ws("{\"op\":\"startcfg\",\"timeout\":100}");     CHECK(has_cmd("STARTCFG,500,1,1,1,1,400"), "startcfg clamps timeout to 500 ms");
    clear(); ws("{\"op\":\"startcfg\",\"timeout\":20000,\"rpm\":600,\"brake\":0}"); CHECK(has_cmd("STARTCFG,10000,1,1,0,1,600"), "startcfg clamps to 10 s");
    clear(); ws("{\"op\":\"startinputs\",\"neutral\":1,\"brake\":0,\"immob\":1}"); CHECK(has_cmd("STARTINPUTS,1,0,1"), "startinputs");
    clear(); ws("{\"op\":\"start\"}");                          CHECK(has_cmd("START"), "start");
    clear(); ws("{\"op\":\"start_cancel\"}");                   CHECK(has_cmd("STARTCANCEL"), "start_cancel");
    clear(); ws("{\"op\":\"gear\",\"gear\":3}");                CHECK(has_cmd("GEAR,3"), "gear");
    clear(); ws("{\"op\":\"gear\",\"gear\":16}");               CHECK(ncmd == 0 && has_msg("{\"type\":\"diag\",\"message\":\"GEAR,ERR,BAD_GEAR\"}"), "gear range");
    clear(); ws("{\"op\":\"gear_bcast\",\"on\":true}");  CHECK(has_cmd("GEARBCAST,ON"), "gear_bcast on");
    clear(); ws("{\"op\":\"gear_bcast\",\"on\":false}"); CHECK(has_cmd("GEARBCAST,OFF"), "gear_bcast off");
    clear(); ws("{\"op\":\"gear_clear\"}");                     CHECK(has_cmd("GEAR,CLEAR"), "gear_clear");
    clear(); ws("{\"op\":\"mode\",\"bus\":\"F\",\"mode\":\"silent\"}"); CHECK(has_cmd("MODE,F,SILENT"), "mode");
    clear(); ws("{\"op\":\"bitrate\",\"bus\":\"B\",\"value\":\"125000\"}"); CHECK(has_cmd("BITRATE,B,125000"), "bitrate");
    clear(); ws("{\"op\":\"gateway\",\"on\":true}");            CHECK(has_cmd("GATEWAY,ON"), "gateway");
    clear(); ws("{\"op\":\"write\",\"on\":true}");              CHECK(has_cmd("WRITE,ON"), "write on");
    clear(); ws("{\"op\":\"write\",\"on\":false}");             CHECK(has_cmd("WRITE,OFF"), "write off");
    clear(); ws("{\"op\":\"can\",\"line\":\"OPT,F,NART,on\"}");  CHECK(has_cmd("OPT,F,NART,on"), "can opt");
    clear(); ws("{\"op\":\"can\",\"line\":\"FILTER,B,20,M,32,1,0,0\"}"); CHECK(has_cmd("FILTER,B,20,M,32,1,0,0"), "can filter");
    clear(); ws("{\"op\":\"can\",\"line\":\"DEVICE,SELECT,2\"}"); CHECK(has_cmd("DEVICE,SELECT,2"), "can device");
    clear(); ws("{\"op\":\"mailcfg\",\"to\":\"a@b.c\",\"from\":\"a@b.c\",\"pass\":\"pw\"}");
    CHECK(strcmp(mail_global()->pass, "pw") == 0, "mailcfg stored");
    clear(); ws("{\"op\":\"mailcfg\",\"to\":\"bad\",\"from\":\"a@b.c\",\"pass\":\"pw\"}");
    CHECK(strcmp(mail_global()->to, "a@b.c") == 0, "bad mailcfg rejected");
    clear(); ws("{\"op\":\"mailsend\"}");
    clear(); dash_on_line("DEVICE,TARGET,1,Honda Civic\r\n");
    CHECK(nmsg >= 1 && strstr(msgs[nmsg - 1], "DEVICE,TARGET,1,Honda Civic") != NULL, "DEVICE line forwarded");
    clear(); ws("{\"op\":\"can\",\"line\":\"START\"}");        CHECK(ncmd == 0, "can rejects non-ability verbs");
    clear(); ws("{\"op\":\"periodic\",\"slot\":2,\"bus\":\"F\",\"format\":\"S\",\"id\":\"7df\",\"dlc\":3,\"data\":\"02010c\",\"period\":100}");
    CHECK(has_cmd("PERIODIC,2,F,S,7DF,3,02010C,100"), "periodic");
    clear(); ws("{\"op\":\"ecuscan\",\"bus\":\"F\",\"format\":\"S\",\"first\":\"7e0\",\"last\":\"7e7\",\"req\":\"3e00\",\"wait\":60}");
    CHECK(has_cmd("ECUSCAN,F,S,7E0,7E7,3E00,60"), "ecuscan");
    clear(); ws("{\"op\":\"udspad\",\"bus\":\"F\",\"value\":\"aa\"}"); CHECK(has_cmd("UDSPAD,F,AA"), "udspad");
    clear(); ws("{\"op\":\"startframe\",\"bus\":\"F\",\"format\":\"S\",\"id\":\"1a0\",\"dlc\":1,\"data\":\"01\"}");
    CHECK(has_cmd("STARTFRAME,F,S,1A0,1,01"), "startframe");
    clear(); ws("not json");                                   CHECK(ncmd == 0, "garbage ignored");
}

static void test_frame_table_overflow_gvret(void)
{
    dash_test_reset_frame_table();
    gv_len = 0u;
    char line[96];
    for (unsigned id = 0x200u; id < 0x200u + 192u; id++) {
        (void)snprintf(line, sizeof line, "123458,STD,%03X,1,0,AA,0\n", id);
        dash_on_line(line);
    }
    const size_t raw_before = gv_len;
    (void)snprintf(line, sizeof line, "123459,STD,2C0,1,0,BB,0\n");
    dash_on_line(line);
    CHECK(gv_len > raw_before, "GVRET raw stream still carries a frame rejected by the full dashboard table");
    tick(300);
    CHECK(msg_contains("\"over\":1"), "193rd distinct frame increments dashboard table overflow");

    gv_sndbuf = 0u;
    (void)snprintf(line, sizeof line, "123460,STD,2C1,1,0,CC,0\n");
    dash_on_line(line);
    gv_sndbuf = 8000u;
    clear();
    tick(1100);
    CHECK(msg_contains("\"sent\":193"), "GVRET accepted all 193 frames before the forced backpressure event");
    CHECK(msg_contains("\"drop\":1"), "GVRET TCP backpressure is exposed in pipeline diagnostics");
    CHECK(msg_contains("\"gv\":1"), "pipeline diagnostics identify the active GVRET client");
}

static void test_pipeline_diagnostics(void)
{
    test_uart_tx_dropped = 7u;
    test_uart_tx_dma_errors = 2u;
    test_uart_dma_te = 1u;
    test_uart_dma_dme = 1u;
    test_uart_dma_fe = 0u;
    test_uart_dma_last = 0x000C0000u;
    test_uart_rx_dropped = 3u;
    clear();
    tick(1100);
    CHECK(msg_contains("\"type\":\"pipeline_diag\""), "pipeline diagnostics message is emitted periodically");
    CHECK(msg_contains("\"txd\":7"), "STM32 UART TX queue drops are exposed");
    CHECK(msg_contains("\"dma\":2"), "STM32 UART DMA errors are exposed");
    CHECK(msg_contains("\"te\":1"), "STM32 DMA transfer errors are separated");
    CHECK(msg_contains("\"dme\":1"), "STM32 DMA direct-mode errors are separated");
    CHECK(msg_contains("\"flg\":786432"), "last DMA error flags are exposed");
    CHECK(msg_contains("\"rxd\":3"), "STM32 UART RX drops are exposed");
    CHECK(msg_contains("\"dbc\":"), "DBC definition count is included in diagnostics");

    test_uart_tx_dropped = UINT32_MAX;
    test_uart_tx_dma_errors = UINT32_MAX;
    test_uart_dma_te = UINT32_MAX;
    test_uart_dma_dme = UINT32_MAX;
    test_uart_dma_fe = UINT32_MAX;
    test_uart_dma_last = UINT32_MAX;
    test_uart_rx_dropped = UINT32_MAX;
    clear();
    tick(1100);
    CHECK(msg_contains("\"flg\":4294967295"),
          "maximum-width UART counters fit in the pipeline diagnostics payload");
}

static void test_uart_bridge_peer(void)
{
    clear();
    dash_bridge_connected();
    CHECK(msg_contains("\"type\":\"status\""), "virtual UART peer receives initial status");
    clear();
    const char *command = "{\"op\":\"write\",\"on\":true}";
    dash_bridge_text((char *)command, strlen(command));
    CHECK(has_cmd("WRITE,ON"), "UART bridge JSON command uses dashboard command parser");
    clear();
    dash_ws_disconnected(0);
    CHECK(ncmd == 0, "direct peer disconnect does not disarm while bridge peer remains");
    dash_bridge_disconnected();
    CHECK(has_cmd("DISARM,F") && has_cmd("DISARM,B"), "last virtual peer disconnect disarms both buses");
    dash_ws_connected(0);
}

static void test_dtc_journal(void)
{
    clear();
    dash_on_line("UDS,F,DATA,7E8,0,5902FF01234508\r\n");
    dash_on_line("UDS,F,POS,7E8,7\r\n");
    tick(100);
    CHECK(msg_contains("\"type\":\"event\"") && msg_contains("\"code\":\"P0123-45\"") && msg_contains("\"status\":\"status 0x08\"") &&
          msg_contains("\"source\":\"UDS 19/02\""), "UDS 19 02 reply creates a DTC event");
    CHECK(has_msg("{\"type\":\"events_reset\"}"), "events_reset precedes the events");
    /* repeat while unseen: same event, repeats++ */
    clear();
    dash_on_line("UDS,F,DATA,7E8,0,5902FF01234508\r\n"); dash_on_line("UDS,F,POS,7E8,7\r\n"); tick(100);
    CHECK(msg_contains("\"repeats\":2"), "repeat report increments repeats");
    /* OBD mode 03 */
    clear();
    dash_on_line("UDS,F,DATA,7E8,0,430101710000\r\n"); dash_on_line("UDS,F,POS,7E8,6\r\n"); tick(100);
    CHECK(msg_contains("\"code\":\"P0171\"") && msg_contains("\"source\":\"OBD stored\""), "mode 03 DTC");
    /* acknowledge */
    clear(); ws("{\"op\":\"event_seen\",\"id\":1}"); tick(100);
    CHECK(msg_contains("\"id\":1,\"code\":\"P0123-45\"") && msg_contains("\"seen\":true"), "event_seen persists + rebroadcasts");
    clear(); ws("{\"op\":\"event_seen\",\"id\":999}");
    CHECK(has_msg("{\"type\":\"diag\",\"message\":\"EVENT,ERR,NOT_FOUND_OR_SAVE_FAILED\"}"), "unknown event id");
    /* snapshot completes after 10 post-trigger 1 Hz samples */
    tick(12000);
    clear(); http_req_t r; memset(&r, 0, sizeof r); strcpy(r.method, "GET"); strcpy(r.path, "/event.csv"); strcpy(r.query, "id=1");
    dash_http_request(NULL, &r);
    CHECK(http_status == 200 && strncmp(http_body, "event_id,dtc,ecu,bus,event_utc,", 31) == 0 && strstr(http_body, "\n1,P0123-45,7E8,F,UP+"),
          "event snapshot CSV downloadable");
    strcpy(r.query, "id=abc"); dash_http_request(NULL, &r);
    CHECK(http_status == 400 && strcmp(http_body, "invalid event id") == 0, "event id validation");
}

static void test_reboot_persistence(void)
{
    /* a fresh dash_init over the same flash reloads the journal */
    clear();
    CHECK(dash_init(cmd_cb, NULL, NULL), "re-init");
    dash_ws_connected(0); tick(100);
    CHECK(msg_contains("\"code\":\"P0123-45\"") && msg_contains("\"seen\":true") && msg_contains("\"code\":\"P0171\""),
          "events survive reboot and are sent to a new client");
}

static const char LOGHDR[] = "uptime_ms,utc,key,rpm,speed_kmh,rpm_src,acc_wire,pedal_17C_b0,fps_F,fps_B,pids\n";
static void test_log_http_and_disarm(void)
{
    http_req_t r; memset(&r, 0, sizeof r); strcpy(r.method, "GET"); strcpy(r.path, "/log.csv");
    dash_http_request(NULL, &r);
    CHECK(http_status == 200 && strncmp(http_body, LOGHDR, strlen(LOGHDR)) == 0,
          "log.csv starts with the ESP32 header");
    CHECK(strstr(http_body, ",RUN,750,12.34,OBD,1,") != NULL, "log rows carry the vehicle snapshot");
    strcpy(r.method, "POST"); strcpy(r.path, "/log/clear"); dash_http_request(NULL, &r);
    CHECK(http_status == 200 && strcmp(http_body, "cleared") == 0, "log clear");
    strcpy(r.method, "GET"); strcpy(r.path, "/log.csv"); dash_http_request(NULL, &r);
    CHECK(http_status == 404 && strcmp(http_body, "no log yet") == 0, "no log after clear");
    strcpy(r.path, "/token"); dash_http_request(NULL, &r);
    CHECK(http_status == 200 && strlen(http_body) == 24, "token is 24 chars");
    get("GET", "/generate_204", "");
    CHECK(http_status == 302 && strstr(http_body, "Redirecting") != NULL,
          "captive portal connectivity check redirects to the dashboard");
    get("GET", "/ncsi.txt", "");
    CHECK(http_status == 302, "second captive portal probe redirects to the dashboard");
    strcpy(r.path, "/"); dash_http_request(NULL, &r);
    CHECK(http_status == 200 && (uint8_t)http_body[0] == 0x1f, "GET / serves the gzip page");
    clear(); dash_ws_disconnected(0);
    CHECK(has_cmd("DISARM,F") && has_cmd("DISARM,B"), "last client leaving disarms both buses");
}

static void test_stm32_update_validation(void)
{
    http_req_t r; memset(&r, 0, sizeof r); strcpy(r.method, "POST"); strcpy(r.path, "/update"); r.multipart = true;
    CHECK(!dash_http_upload_begin(&r), "ESP32 OTA refused");
    dash_http_upload_end((http_conn_t *)1, &r, true);
    CHECK(http_status == 500 && strstr(http_body, "no ESP32"), "ESP32 OTA explains single-board build");
    strcpy(r.path, "/update-stm32"); r.content_length = 64;
    CHECK(dash_http_upload_begin(&r), "STM32 upload begins (staging erased)");
    uint8_t bad[16] = { 0 };
    CHECK(dash_http_upload_data(bad, sizeof bad), "data staged");
    dash_http_upload_end((http_conn_t *)1, &r, true);
    CHECK(http_status == 500 && strstr(http_body, "Rejected before erase: initial stack pointer"), "invalid image rejected, app untouched");
}

extern int fake_erase_fail; extern int fake_fail_after;
static struct tcp_pcb *newpcb(void) { pcbs[npcb].id = npcb; return &pcbs[npcb++]; }
static void get(const char *method, const char *path, const char *query)
{
    http_req_t r; memset(&r, 0, sizeof r); strcpy(r.method, method); strcpy(r.path, path); strcpy(r.query, query);
    http_status = 0; dash_http_request((http_conn_t *)1, &r);
}
static void dtc(unsigned n)       /* one UDS 19 02 report with a distinct DTC */
{
    char l[96]; snprintf(l, sizeof l, "UDS,F,DATA,7E8,0,5902FF%02X%02X0108\r\n", 0x10u + n / 200u, n % 200u);
    dash_on_line(l); dash_on_line("UDS,F,POS,7E8,7\r\n");
}

static void test_parser_and_json_edges(void)
{
    clear();
    dash_on_line("CMD,ERR,a\\b\b\f\tz\rq\nw\r\n");
    CHECK(msg_contains("a\\\\b\\b\\f\\tz\\rq"), "JSON escapes for backslash and control characters");
    clear(); dash_on_line("UDS,F,NEG,7E8,31\r\n"); dash_on_line("UDS,B,ERR,TIMEOUT\r\n"); dash_on_line("UDS,F,TXERR\r\n");
    CHECK(msg_contains("UDS,F,NEG"), "UDS negative/error lines reset the DTC buffer and are logged");
    clear();
    dash_on_line("STATUS,F,BITRATE,250000\r\n");
    CHECK(msg_contains("\"locked\":true,\"bitrate\":250000"), "BITRATE status");
    dash_on_line("STATUS,F,SCAN,125000\r\n");
    CHECK(msg_contains("\"locked\":false,\"bitrate\":125000"), "SCAN status");
    dash_on_line("STATUS,F,DWELL,300\r\n");
    CHECK(msg_contains("\"dwell\":300"), "DWELL status");
    dash_on_line("STATUS,F,REACQUIRE\r\n");
    CHECK(msg_contains("\"locked\":false,\"bitrate\":0"), "REACQUIRE status");
    dash_on_line("STATUS,F,LOCK,500000\r\n");
    clear(); ncmd = 0;
    ws("{\"op\":\"can\",\"line\":\"OPT,F\\n\\t\\r\\b\\f\\u0041\\\"\"}");
    CHECK(ncmd == 1 && strcmp(cmds[0], "OPT,F\n\t\r\b\f?\"") == 0, "JSON string escapes decoded");
    clear(); ncmd = 0;
    ws("{\"op\":\"write\",\"on\":1}");
    CHECK(has_cmd("WRITE,OFF"), "non-boolean 'on' falls back to the default");
    /* PID values refresh the snapshot list in place */
    dash_on_line("PID,0C,RPM,rpm,800\r\n"); dash_on_line("PID,0D,Speed,kmh,20\r\n"); dash_on_line("PID,0C,RPM,rpm,900\r\n");
}

static void test_gvret_and_observer(void)
{
    /* GVRET: replies for keepalive, device info, numbuses, time sync, bus params, setup, dlc 0 */
    gv_len = 0;
    static const uint8_t cmds_in[] = { 0xF1, 0x09, 0xF1, 0x07, 0xF1, 0x0C, 0xF1, 0x01, 0xF1, 0x06,
                                       0xF1, 0x05, 1, 2, 3, 4, 5, 6, 7, 8,
                                       0xF1, 0x00, 0x23, 0x01, 0x00, 0x00, 0x01, 0x00 };
    struct pbuf p = { NULL, (void *)cmds_in, sizeof cmds_in, sizeof cmds_in };
    clear(); recv_fn[gv_pcb->id](NULL, gv_pcb, &p, ERR_OK);
    CHECK(gv_len > 20 && has_cmd("TX,B,S,123,0,0,-"), "GVRET commands answered, dlc-0 frame forwarded");
    CHECK(memmem(gv_out, gv_len, "\xF1\x06\x10", 3) != NULL, "bus params from the weak default (disabled)");
    struct tcp_pcb *second = newpcb();
    CHECK(accept_fn[23](NULL, second, ERR_OK) == ERR_ABRT, "one GVRET client at a time");
    CHECK(accept_fn[23](NULL, NULL, ERR_OK) == ERR_VAL, "accept error");
    recv_fn[gv_pcb->id](NULL, gv_pcb, NULL, ERR_OK);               /* peer closed */
    struct tcp_pcb *g2 = newpcb();
    CHECK(accept_fn[23](NULL, g2, ERR_OK) == ERR_OK, "slot free after close");
    err_fn[g2->id](NULL, ERR_ABRT);                                  /* reset by peer */
    struct tcp_pcb *g3 = newpcb(); close_fail = 1;
    CHECK(accept_fn[23](NULL, g3, ERR_OK) == ERR_OK, "slot free after error");
    recv_fn[g3->id](NULL, g3, NULL, ERR_OK); close_fail = 0;        /* close fails -> abort */
    gv_pcb = newpcb(); CHECK(accept_fn[23](NULL, gv_pcb, ERR_OK) == ERR_OK, "re-accept");

    /* observer on 2323: newest wins, input discarded, close and error */
    struct tcp_pcb *o1 = newpcb(), *o2 = newpcb();
    CHECK(accept_fn[2323](NULL, o1, ERR_OK) == ERR_OK, "observer accepted");
    CHECK(accept_fn[2323](NULL, o2, ERR_OK) == ERR_OK, "newest observer replaces the old one");
    CHECK(accept_fn[2323](NULL, NULL, ERR_OK) == ERR_VAL, "observer accept error");
    uint8_t junk[3] = { 1, 2, 3 }; struct pbuf jp = { NULL, junk, 3, 3 };
    recv_fn[o2->id](NULL, o2, &jp, ERR_OK);
    recv_fn[o1->id](NULL, o1, NULL, ERR_OK);                         /* stale pcb closes */
    recv_fn[o2->id](NULL, o2, NULL, ERR_OK);
    struct tcp_pcb *o3 = newpcb();
    CHECK(accept_fn[2323](NULL, o3, ERR_OK) == ERR_OK, "observer re-accept");
    err_fn[o3->id](NULL, ERR_ABRT);
    tick(1100);
}

static void test_mail_http(void)
{
    get("POST", "/mail/ack", "");
    CHECK(http_status == 400, "ack needs seq");
    get("POST", "/mail/ack", "seq=12345");
    CHECK(http_status == 409, "ack of a non-pending seq");
    clear(); get("POST", "/mail/send", "");
    CHECK(http_status == 200 && has_msg("{\"type\":\"diag\",\"message\":\"MAIL,PENDING\"}"), "manual send queued");
    get("GET", "/mail/status", "");
    CHECK(http_status == 200 && strstr(http_body, "\"pending\":true") != NULL, "status shows pending");
    const char *s = strstr(http_body, "\"seq\":"); unsigned long seq = s ? strtoul(s + 6, NULL, 10) : 0ul;
    char q[32]; snprintf(q, sizeof q, "seq=%lu", seq);
    clear(); get("POST", "/mail/ack", q);
    CHECK(http_status == 200 && has_msg("{\"type\":\"diag\",\"message\":\"MAIL,SENT\"}"), "ack marks the log as sent");
    get("GET", "/event.csv", "id=999");
    CHECK(http_status == 404, "unknown event id");
}

static void test_event_capacity_and_compaction(void)
{
    tick(65000);                       /* > 60 one-second rows: pre-trigger ring drops the oldest */
    /* fill the journal to 40 unacknowledged events, each with a snapshot */
    for (unsigned n = 0; n < 45u; n++) { dtc(n); tick(11000); }
    clear(); dtc(300); tick(100);
    CHECK(has_msg("{\"type\":\"diag\",\"message\":\"EVENTS,ERR,40_UNACKNOWLEDGED_CAPACITY\"}"),
          "40 unseen events: new reports refused, evidence kept");
    /* acknowledge everything: new reports evict the oldest seen events */
    for (unsigned id = 1; id < 200u; id++) { char b[64]; snprintf(b, sizeof b, "{\"op\":\"event_seen\",\"id\":%u}", id); ws(b); }
    dtc(301); dtc(302); tick(3000);
    CHECK(msg_contains("\"type\":\"event\""), "eviction keeps the journal running");
    /* seen event reported again within 120 s counts as a repeat */
    for (unsigned id = 1; id < 200u; id++) { char b[64]; snprintf(b, sizeof b, "{\"op\":\"event_seen\",\"id\":%u}", id); ws(b); }
    tick(3000); clear(); dtc(302);
    bool rep = false; for (int t = 0; t < 40 && !rep; t++) { tick(50); rep = msg_contains("\"repeats\":2"); }
    CHECK(rep, "seen event repeated within 120 s");
    /* a deletion record is replayed on boot */
    CHECK(dash_init(cmd_cb, NULL, NULL), "re-init with deletion records");
    /* post-trigger row write fails: the capture is dropped, the event kept */
    dtc(310); tick(1500); fake_fail_after = 0; tick(1100); fake_fail_after = -1;
    /* keep capturing snapshots until the journal sector compacts */
    for (unsigned n = 320; n < 360u; n++) {
        dtc(n); tick(11000);
        char b[64]; for (unsigned id = 1; id < 400u; id++) { snprintf(b, sizeof b, "{\"op\":\"event_seen\",\"id\":%u}", id); ws(b); }
    }
    /* the journal survives a reboot after compaction */
    clear();
    CHECK(dash_init(cmd_cb, NULL, NULL), "re-init after compaction");
    dash_ws_connected(0); tick(100);
    CHECK(msg_contains("\"type\":\"event\""), "events replayed after compaction");
    const uint32_t delete_id = dash_test_latest_event_id();
    CHECK(delete_id != 0u && evst_append(EV_REC_EVENT_DEL, delete_id, NULL, 0u),
          "event deletion record appended");
    clear(); CHECK(dash_init(cmd_cb, NULL, NULL), "re-init with event deletion record");
    dash_ws_connected(0); tick(100);
    char deleted_id[32]; snprintf(deleted_id, sizeof deleted_id, "\"id\":%lu,", (unsigned long)delete_id);
    CHECK(!msg_contains(deleted_id), "deleted event is removed during journal replay");
}

static void test_label_paths_and_replay(void)
{
    clear();
    ws("{\"op\":\"set_label\",\"id\":\"158\",\"label\":\"ENGINE_CUSTOM\"}");
    CHECK(msg_contains("label_update") && msg_contains("ENGINE_CUSTOM"),
          "dashboard set_label command updates the frame label");
    clear();
    dash_on_line("LABEL,158,CSV_CUSTOM\r\n");
    CHECK(msg_contains("label_update") && msg_contains("CSV_CUSTOM"),
          "UART LABEL line updates the frame label");

    clear();
    CHECK(dash_init(cmd_cb, NULL, NULL), "label journal replay init");
    dash_ws_connected(0);
    CHECK(msg_contains("label_update") && msg_contains("CSV_CUSTOM"),
          "saved label is replayed before the first CAN frame");
    dash_on_line("300100,STD,158,8,0,000005DC00000000,0\n");
    tick(300);
    CHECK(msg_contains("[\"158\",0,8,\"000005DC00000000\"" ) &&
          msg_contains("CSV_CUSTOM"), "first post-reboot frame inherits its saved label");

    static const char pending_label[] = "PLACEHOLDER";
    CHECK(evst_append(EV_REC_LABEL, 0x6AAu, pending_label, sizeof pending_label),
          "persist a label before its CAN identifier has been observed");
    clear(); CHECK(dash_init(cmd_cb, NULL, NULL), "replay pending label");
    dash_on_line("300101,STD,6AA,8,0,0102030405060708,1\n");
    tick(300);
    CHECK(msg_contains("PLACEHOLDER") && msg_contains("[\"6AA\",1,8,"),
          "replayed placeholder label attaches to the first matching bus-1 frame");
}

static void test_snapshot_completion_save_failure(void)
{
    clear();
    dtc(401);
    tick(100);
    const uint32_t id = dash_test_latest_event_id();
    CHECK(id != 0u, "test event created for snapshot failure");
    dash_test_fail_save_events(true);
    for (unsigned sample = 0; sample < 20u; sample++) dash_test_capture_post_event_sample();
    dash_test_fail_save_events(false);
    CHECK(!dash_test_snapshot_ready(id), "snapshot completion metadata failure leaves the snapshot incomplete");
}

static void test_snapshot_limit_backpressure(void)
{
    fake_flash_reset();
    CHECK(dash_init(cmd_cb, NULL, NULL), "isolated snapshot-limit journal init");
    dash_ws_connected(0);
    uint32_t first_id = 0u;
    for (unsigned i = 0; i < 8u; i++) {
        dtc(500u + i);
        tick(50);
        const uint32_t id = dash_test_latest_event_id();
        CHECK(id != 0u, "snapshot-capacity event created");
        if (i == 0u) first_id = id;
        for (unsigned sample = 0; sample < 20u; sample++) dash_test_capture_post_event_sample();
        CHECK(dash_test_snapshot_ready(id), "post-event samples complete the snapshot");
    }

    char seen[64]; snprintf(seen, sizeof seen, "{\"op\":\"event_seen\",\"id\":%lu}", (unsigned long)first_id);
    ws(seen);
    dtc(508u); tick(50);
    uint32_t ninth_id = dash_test_latest_event_id();
    for (unsigned sample = 0; sample < 20u; sample++) dash_test_capture_post_event_sample();
    CHECK(dash_test_snapshot_ready(ninth_id), "one seen snapshot is evicted to admit a new capture");
    CHECK(!dash_test_snapshot_ready(first_id), "oldest seen snapshot is released at the configured limit");

    dtc(509u); tick(50);
    const uint32_t tenth_id = dash_test_latest_event_id();
    CHECK(!dash_test_snapshot_ready(tenth_id), "capture is refused when all retained snapshots are unseen");
}

static void test_backup_write_failure(void)
{
    ee_present = 1;
    ee_write_fail = 0;
    CHECK(dash_init(cmd_cb, NULL, NULL), "reinitialize present EEPROM before failure test");
    ee_write_fail = 1;
    for (unsigned attempt = 0; attempt < 3u; attempt++) {
        clear();
        dash_identity_changed();
        dash_test_run_backup_service();
        CHECK(msg_contains("BACKUP,ERR,WRITE_FAILED"), "EEPROM write failure is reported");
    }
    CHECK(msg_contains("BACKUP,STATUS,ABSENT"), "three EEPROM failures disable backup until reboot");
    ee_write_fail = 0;
}

static void test_event_compaction_rewrite_failure(void)
{
    ee_present = 0;
    fake_flash_reset();
    CHECK(dash_init(cmd_cb, NULL, NULL), "isolated journal init");
    dtc(400);
    tick(100);
    CHECK(dash_test_latest_event_id() != 0u, "event exists before journal compaction");
    dash_test_fail_event_rewrite(true);
    CHECK(!evst_compact(), "compaction aborts when an event record cannot be rewritten");
    dash_test_fail_event_rewrite(false);
    CHECK(dash_init(cmd_cb, NULL, NULL), "old journal remains bootable after interrupted compaction");
    dash_ws_connected(0);
    CHECK(msg_contains("\"type\":\"event\""), "event data survives interrupted compaction");
    ee_present = 1;
}

static void test_mail_at_70_percent(void)
{
    bool pending = false;
    for (int k = 0; k < 90 && !pending; k++) {
        clear();
        for (int t = 0; t < 60 && !pending; t++) { tick(1000); pending = has_msg("{\"type\":\"diag\",\"message\":\"MAIL,PENDING\"}"); }
    }
    CHECK(pending, "log reaching 70 % requests the e-mail");
}

static void test_upload_paths(void)
{
    http_req_t r; memset(&r, 0, sizeof r); strcpy(r.method, "POST"); r.multipart = true;
    strcpy(r.path, "/elsewhere");
    CHECK(!dash_http_upload_begin(&r), "unknown upload path refused");
    dash_http_upload_end((http_conn_t *)1, &r, true);
    CHECK(http_status == 404, "end of unknown upload -> 404");
    strcpy(r.path, "/update-stm32"); r.content_length = 300000;
    fake_erase_fail = 1;
    CHECK(!dash_http_upload_begin(&r), "staging erase failure refused");
    fake_erase_fail = 0;
    dash_http_upload_end((http_conn_t *)1, &r, false);
    CHECK(http_status == 400 && strstr(http_body, "staging write failed"), "erase failure reported");
    CHECK(dash_http_upload_begin(&r), "begin");
    CHECK(!dash_http_upload_begin(&r), "second upload refused while one runs");
    static uint8_t chunk[8192]; memset(chunk, 0, sizeof chunk);
    for (int i = 0; i < 40; i++) (void)dash_http_upload_data(chunk, sizeof chunk);   /* > 256 KB */
    dash_http_upload_end((http_conn_t *)1, &r, true);
    CHECK(http_status == 400, "oversized image refused");
    r.content_length = 0; CHECK(dash_http_upload_begin(&r), "begin");
    fake_fail_after = 0; CHECK(!dash_http_upload_data(chunk, 16), "program failure"); fake_fail_after = -1;
    dash_http_upload_end(NULL, &r, true);
    /* stack pointer good, reset vector outside the image */
    r.content_length = 8192; CHECK(dash_http_upload_begin(&r), "begin");
    static uint8_t img[8192]; memset(img, 0, sizeof img);
    img[0] = 0x00; img[1] = 0x00; img[2] = 0x02; img[3] = 0x20;    /* SP 0x20020000 */
    img[4] = 0x01; img[5] = 0x00; img[6] = 0x10; img[7] = 0x08;    /* RV 0x08100001 */
    CHECK(dash_http_upload_data(img, 4096) && dash_http_upload_data(img + 4096, 4096), "staged");
    dash_http_upload_end((http_conn_t *)1, &r, true);
    CHECK(http_status == 500 && strstr(http_body, "reset vector"), "reset vector check");
    /* a valid image is accepted; applying it (reset) is the next poll, not run here */
    CHECK(dash_http_upload_begin(&r), "begin");
    img[4] = 0x01; img[5] = 0x01; img[6] = 0x00; img[7] = 0x08;    /* RV 0x08000101 */
    CHECK(dash_http_upload_data(img, sizeof img), "staged");
    dash_http_upload_end((http_conn_t *)1, &r, true);
    CHECK(http_status == 200 && strstr(http_body, "STM32 flashed 8192 bytes"), "valid image accepted");
}

static void test_signal_learning(void)
{
    /* OBD RPM 1500 and ENGINE_DATA (0x158) frames carrying 1500 rpm: CHECKED then VERIFIED */
    clear();
    for (int t = 0; t < 25; t++) {
        dash_on_line("PID,0C,RPM,rpm,1500\r\n");
        dash_on_line("123,STD,158,8,0,000005DC00000000,0\n");
        tick(50);
    }
    CHECK(msg_contains("\"type\":\"sigstate\",\"id\":\"158\",\"name\":\"ENGINE_DATA\",\"state\":\"VERIFIED\""),
          "learn mode verifies ENGINE_DATA against OBD RPM");
    dash_on_line("124,STD,17C,4,0,00000000,0\n"); tick(50);
    CHECK(msg_contains("\"id\":\"17C\",\"name\":\"POWERTRAIN_DATA\",\"state\":\"CONFLICT\""), "DLC mismatch reported as CONFLICT");
    /* grades survive a reboot and are sent in chunks to a new client */
    clear();
    CHECK(dash_init(cmd_cb, NULL, NULL), "re-init");
    dash_ws_connected(0);
    CHECK(msg_contains("\"type\":\"sigstates\",\"reset\":true") && msg_contains("[\"158\",\"ENGINE_DATA\",\"VERIFIED\"") &&
          msg_contains("[\"516\",\"ODOMETER\",\"SEED\""), "grades replayed and every DBC message listed");
    clear(); ws("{\"op\":\"sigstates\"}");
    CHECK(msg_contains("[\"17C\",\"POWERTRAIN_DATA\",\"CONFLICT\""), "sigstates on request");
    /* identity mirrored to the 24LC512 once the bus is quiet */
    clear(); ws("{\"op\":\"backup\",\"now\":true}"); tick(2500);
    CHECK(msg_contains("BACKUP,STATUS,PRESENT,"), "EEPROM backup written");
    /* full flash erase (ST-LINK mass erase): grades come back from the EEPROM */
    fake_flash_reset(); clear();
    CHECK(dash_init(cmd_cb, NULL, NULL), "boot on an erased flash");
    dash_ws_connected(0);
    CHECK(msg_contains("[\"158\",\"ENGINE_DATA\",\"VERIFIED\"") && msg_contains("BACKUP,STATUS,PRESENT,") &&
          !msg_contains("BACKUP,STATUS,PRESENT,0,-1"), "identity restored from the EEPROM after a mass erase");
    /* the restored identity was written back into the flash journal */
    clear(); CHECK(dash_init(cmd_cb, NULL, NULL), "second boot"); dash_ws_connected(0);
    CHECK(msg_contains("[\"158\",\"ENGINE_DATA\",\"VERIFIED\"") && msg_contains(",-1\"}"), "journal holds it again (no restore needed)");
    /* EEPROM absent: backups are skipped, nothing breaks */
    ee_present = 0; clear(); CHECK(dash_init(cmd_cb, NULL, NULL), "boot without EEPROM"); dash_ws_connected(0);
    CHECK(msg_contains("BACKUP,STATUS,ABSENT"), "absent EEPROM reported");
    ee_present = 1; CHECK(dash_init(cmd_cb, NULL, NULL), "EEPROM back");
}

int main(void)
{
    fake_flash_reset();
    if (!dash_init(cmd_cb, NULL, NULL)) { printf("FAIL dash_init\n"); return 1; }
    dash_ws_connected(0);
    test_status_lines();
    test_pid_profile_sig();
    test_frames_and_gvret();
    test_uart_batch_retry();
    test_dec_batch();
    test_ws_commands();
    test_uart_bridge_peer();
    test_pipeline_diagnostics();
    test_dtc_journal();
    test_reboot_persistence();
    test_log_http_and_disarm();
    test_stm32_update_validation();
    test_parser_and_json_edges();
    test_gvret_and_observer();
    test_mail_http();
    test_event_capacity_and_compaction();
    test_signal_learning();
    test_label_paths_and_replay();
    test_snapshot_completion_save_failure();
    test_mail_at_70_percent();
    test_snapshot_limit_backpressure();
    test_backup_write_failure();
    test_event_compaction_rewrite_failure();
    test_frame_table_overflow_gvret();
    test_upload_paths();             /* last: a valid image schedules the apply/reset */
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("test_dash_bridge: all checks passed\n");
    return 0;
}
