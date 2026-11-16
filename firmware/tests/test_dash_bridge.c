/* Host tests for dash.c: the STM32-resident port of the ESP32 dashboard.
 * Checks that STM32 output lines become exactly the WebSocket JSON the
 * ESP32 produced (ArduinoJson formatting), that dashboard commands become
 * exactly the STM32 command lines the ESP32 sent, and the DTC journal,
 * GVRET TCP and log/observer behaviour. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dash.h"
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
void ws_broadcast_txt(const char *s, size_t n) { if (nmsg < MAXMSG) { memcpy(msgs[nmsg], s, n); msgs[nmsg++][n] = 0; } }
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

/* lwIP stubs */
static struct tcp_pcb pcbs[16]; static int npcb;
static tcp_accept_fn accept_fn[65536];
static uint16_t last_bind;
static uint8_t gv_out[4096]; static size_t gv_len; static struct tcp_pcb *gv_pcb;
u16_t tcp_sndbuf(struct tcp_pcb *pcb) { (void)pcb; return 8000; }
err_t tcp_write(struct tcp_pcb *pcb, const void *data, u16_t len, u8_t flags)
{ (void)flags; if (pcb == gv_pcb && gv_len + len < sizeof gv_out) { memcpy(gv_out + gv_len, data, len); gv_len += len; } return ERR_OK; }
err_t tcp_output(struct tcp_pcb *pcb) { (void)pcb; return ERR_OK; }
void  tcp_arg(struct tcp_pcb *pcb, void *arg) { (void)pcb; (void)arg; }
static tcp_recv_fn recv_fn[16];
void  tcp_recv(struct tcp_pcb *pcb, tcp_recv_fn f) { recv_fn[pcb->id] = f; }
void  tcp_err(struct tcp_pcb *pcb, tcp_err_fn f) { (void)pcb; (void)f; }
err_t tcp_close(struct tcp_pcb *pcb) { (void)pcb; return ERR_OK; }
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

int main(void)
{
    fake_flash_reset();
    if (!dash_init(cmd_cb, NULL, NULL)) { printf("FAIL dash_init\n"); return 1; }
    dash_ws_connected(0);
    test_status_lines();
    test_pid_profile_sig();
    test_frames_and_gvret();
    test_dec_batch();
    test_ws_commands();
    test_dtc_journal();
    test_reboot_persistence();
    test_log_http_and_disarm();
    test_stm32_update_validation();
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("test_dash_bridge: all checks passed\n");
    return 0;
}
