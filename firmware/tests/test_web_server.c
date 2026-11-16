/* Host tests for web_server.c: RFC 6455 handshake (spec test vector), masked
 * client frames, ping/heartbeat, HTTP routing and the streaming multipart
 * upload parser (boundary split across TCP segments). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Core/Src/web_server.c"

volatile uint32_t system_millis;
static int failures;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, m); failures++; } } while (0)

/* lwIP stubs: capture everything written per pcb */
static struct tcp_pcb pcbs[8]; static int npcb;
static char out[8][70000]; static size_t outn[8];
static tcp_accept_fn acc[100]; static uint16_t bindport;
static tcp_recv_fn rf[8]; static void *rarg[8]; static int closed[8];
u16_t tcp_sndbuf(struct tcp_pcb *pcb) { (void)pcb; return 6000; }
u16_t tcp_sndqueuelen(struct tcp_pcb *pcb) { (void)pcb; return 0; }
err_t tcp_write(struct tcp_pcb *pcb, const void *d, u16_t n, u8_t f) { (void)f; memcpy(out[pcb->id] + outn[pcb->id], d, n); outn[pcb->id] += n; return ERR_OK; }
err_t tcp_output(struct tcp_pcb *pcb) { (void)pcb; return ERR_OK; }
void  tcp_arg(struct tcp_pcb *pcb, void *a) { rarg[pcb->id] = a; }
void  tcp_recv(struct tcp_pcb *pcb, tcp_recv_fn f) { rf[pcb->id] = f; }
void  tcp_err(struct tcp_pcb *pcb, tcp_err_fn f) { (void)pcb; (void)f; }
void  tcp_sent(struct tcp_pcb *pcb, tcp_sent_fn f) { (void)pcb; (void)f; }
void  tcp_poll(struct tcp_pcb *pcb, tcp_poll_fn f, u8_t i) { (void)pcb; (void)f; (void)i; }
err_t tcp_close(struct tcp_pcb *pcb) { closed[pcb->id] = 1; return ERR_OK; }
void  tcp_abort(struct tcp_pcb *pcb) { closed[pcb->id] = 1; }
void  tcp_recved(struct tcp_pcb *pcb, u16_t len) { (void)pcb; (void)len; }
void  tcp_nagle_disable(struct tcp_pcb *pcb) { (void)pcb; }
struct tcp_pcb *tcp_new_ip_type(u8_t t) { (void)t; pcbs[npcb].id = npcb; return &pcbs[npcb++]; }
err_t tcp_bind(struct tcp_pcb *pcb, const void *ip, u16_t port) { (void)pcb; (void)ip; bindport = port; return ERR_OK; }
struct tcp_pcb *tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t b) { (void)b; return pcb; }
void  tcp_accept(struct tcp_pcb *pcb, tcp_accept_fn f) { (void)pcb; acc[bindport] = f; }
u8_t  pbuf_free(struct pbuf *p) { (void)p; return 1; }

/* dash callbacks */
static char got_text[2000]; static int connected_n, disconnected_n;
static uint8_t upload[300000]; static size_t upload_n; static int upload_end_ok = -1;
void dash_ws_connected(uint8_t n) { (void)n; connected_n++; }
void dash_ws_disconnected(uint8_t n) { (void)n; disconnected_n++; }
void dash_ws_text(uint8_t n, char *m, size_t l) { (void)n; memcpy(got_text, m, l); got_text[l] = 0; }
void dash_http_request(http_conn_t *c, const http_req_t *r)
{
    if (!strcmp(r->path, "/")) http_respond(c, 200, "text/html", "Content-Encoding: gzip\r\n", "PAGE", 4);
    else if (!strcmp(r->path, "/q")) { char v[16] = ""; http_query_get(r, "id", v, sizeof v); http_respond_text(c, 200, v); }
}
bool dash_http_upload_begin(const http_req_t *r) { (void)r; upload_n = 0; return true; }
bool dash_http_upload_data(const uint8_t *d, size_t n) { memcpy(upload + upload_n, d, n); upload_n += n; return true; }
void dash_http_upload_end(http_conn_t *c, const http_req_t *r, bool ok) { (void)r; upload_end_ok = ok; if (c) http_respond_text(c, 200, "done"); }

static struct tcp_pcb *conn(uint16_t port) { struct tcp_pcb *p = tcp_new_ip_type(0); acc[port](NULL, p, ERR_OK); return p; }
static void feed(struct tcp_pcb *p, const void *d, size_t n) { struct pbuf b = { NULL, (void *)d, (u16_t)n, (u16_t)n }; rf[p->id](rarg[p->id], p, &b, ERR_OK); }

static void test_ws(void)
{
    struct tcp_pcb *p = conn(81);
    const char *req = "GET / HTTP/1.1\r\nHost: 192.168.7.1:81\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                      "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n";
    feed(p, req, strlen(req));
    out[p->id][outn[p->id]] = 0;
    CHECK(strstr(out[p->id], "HTTP/1.1 101 Switching Protocols") != NULL, "101 response");
    CHECK(strstr(out[p->id], "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n") != NULL, "RFC 6455 accept key");
    CHECK(connected_n == 1 && ws_connected_clients() == 1, "client counted");
    /* masked text frame {"op":"start"} split across two segments */
    const char *msg = "{\"op\":\"start\"}"; const uint8_t mask[4] = { 0x11, 0x22, 0x33, 0x44 };
    uint8_t fr[64]; size_t n = 0; fr[n++] = 0x81; fr[n++] = (uint8_t)(0x80 | strlen(msg)); memcpy(fr + n, mask, 4); n += 4;
    for (size_t i = 0; i < strlen(msg); i++) fr[n++] = (uint8_t)msg[i] ^ mask[i & 3];
    feed(p, fr, 5); feed(p, fr + 5, n - 5);
    CHECK(strcmp(got_text, msg) == 0, "unmasked text delivered");
    /* server -> client text frame */
    outn[p->id] = 0; ws_broadcast_txt("hello", 5);
    CHECK(outn[p->id] == 7 && (uint8_t)out[p->id][0] == 0x81 && out[p->id][1] == 5 && !memcmp(out[p->id] + 2, "hello", 5), "server text frame");
    /* 300-byte message uses the 16-bit length form */
    char big[300]; memset(big, 'a', sizeof big); outn[p->id] = 0; ws_broadcast_txt(big, sizeof big);
    CHECK(outn[p->id] == 304 && (uint8_t)out[p->id][1] == 126 && (uint8_t)out[p->id][2] == 1 && (uint8_t)out[p->id][3] == 44, "extended length");
    /* heartbeat: ping at 15 s, unanswered x2 -> disconnect (ESP32 enableHeartbeat(15000,3000,2)) */
    outn[p->id] = 0; system_millis += 15000; web_server_poll();
    CHECK(outn[p->id] == 2 && (uint8_t)out[p->id][0] == 0x89, "ping sent at 15 s");
    system_millis += 3000; web_server_poll(); CHECK(disconnected_n == 0, "one miss tolerated");
    system_millis += 12000; web_server_poll(); system_millis += 3000; web_server_poll();
    CHECK(disconnected_n == 1 && ws_connected_clients() == 0, "two misses -> disconnect");
}

static void test_http(void)
{
    struct tcp_pcb *p = conn(80);
    feed(p, "GET / HTTP/1.1\r\nHost: x\r\n\r\n", 27);
    out[p->id][outn[p->id]] = 0;
    CHECK(strstr(out[p->id], "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Encoding: gzip\r\nContent-Length: 4\r\n") &&
          strstr(out[p->id], "\r\n\r\nPAGE"), "GET / gzip page");
    p = conn(80); feed(p, "GET /q?x=1&id=42 HTTP/1.1\r\n\r\n", 30); out[p->id][outn[p->id]] = 0;
    CHECK(strstr(out[p->id], "\r\n\r\n42") != NULL, "query parsing");
    p = conn(80); feed(p, "GET /nope HTTP/1.1\r\n\r\n", 22); out[p->id][outn[p->id]] = 0;
    CHECK(strstr(out[p->id], "404 Not Found") && strstr(out[p->id], "Not found"), "404");
}

static void test_upload(void)
{
    static uint8_t img[70000]; for (size_t i = 0; i < sizeof img; i++) img[i] = (uint8_t)(i * 7 + (i >> 8));
    /* include bytes that look like a partial boundary inside the payload */
    memcpy(img + 1000, "\r\n--XyZ", 7);
    static char body[80000]; size_t bn = 0;
    bn += (size_t)sprintf(body + bn, "--XyZb\r\nContent-Disposition: form-data; name=\"f\"; filename=\"a.bin\"\r\nContent-Type: application/octet-stream\r\n\r\n");
    memcpy(body + bn, img, sizeof img); bn += sizeof img;
    bn += (size_t)sprintf(body + bn, "\r\n--XyZb--\r\n");
    char hdr[256]; const int hn = sprintf(hdr, "POST /update-stm32 HTTP/1.1\r\nContent-Type: multipart/form-data; boundary=XyZb\r\nContent-Length: %zu\r\n\r\n", bn);
    struct tcp_pcb *p = conn(80);
    feed(p, hdr, (size_t)hn);
    for (size_t off = 0; off < bn; ) { size_t n = 1 + (off * 13) % 1460; if (off + n > bn) n = bn - off; feed(p, body + off, n); off += n; }
    CHECK(upload_end_ok == 1, "upload completed with closing boundary");
    CHECK(upload_n == sizeof img && memcmp(upload, img, sizeof img) == 0, "payload byte-exact (boundary-like data kept)");
}

/* Real-link shape: headers + body coalesced into 1460-byte TCP segments. */
static void test_upload_coalesced(void)
{
    static uint8_t img[9000]; for (size_t i = 0; i < sizeof img; i++) img[i] = (uint8_t)(i * 31 + 5);
    static char stream[12000]; size_t sn = 0;
    char body_head[200]; const int bh = sprintf(body_head, "--BB\r\nContent-Disposition: form-data; name=\"file\"; filename=\"x.bin\"\r\n\r\n");
    const size_t body_len = (size_t)bh + sizeof img + strlen("\r\n--BB--\r\n");
    sn += (size_t)sprintf(stream, "POST /update-stm32 HTTP/1.1\r\nHost: 192.168.7.1\r\nContent-Length: %zu\r\n"
                          "Content-Type: multipart/form-data; boundary=BB\r\n\r\n", body_len);
    memcpy(stream + sn, body_head, (size_t)bh); sn += (size_t)bh;
    memcpy(stream + sn, img, sizeof img); sn += sizeof img;
    sn += (size_t)sprintf(stream + sn, "\r\n--BB--\r\n");
    upload_end_ok = -1;
    struct tcp_pcb *p = conn(80);
    for (size_t off = 0; off < sn; off += 1460) feed(p, stream + off, sn - off < 1460 ? sn - off : 1460);
    CHECK(upload_end_ok == 1, "coalesced upload completes");
    CHECK(upload_n == sizeof img && memcmp(upload, img, sizeof img) == 0, "coalesced upload byte-exact");
}

int main(void)
{
    if (!web_server_init()) { printf("FAIL init\n"); return 1; }
    test_ws(); test_http(); test_upload(); test_upload_coalesced();
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("test_web_server: all checks passed\n");
    return 0;
}
