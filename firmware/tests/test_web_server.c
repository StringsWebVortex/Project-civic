/* Host tests for web_server.c: RFC 6455 handshake (spec test vector), masked
 * client frames, ping/heartbeat, HTTP routing and the streaming multipart
 * upload parser (boundary split across TCP segments). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Core/Src/web_server.c"

static char uart_mirror[70000];
static size_t uart_mirror_len;
static unsigned uart_mirror_calls;
bool uart_esp_emit_line(const char *s, size_t n)
{
    if (!s || n + 1u > sizeof uart_mirror - uart_mirror_len) return false;
    memcpy(uart_mirror + uart_mirror_len, s, n);
    uart_mirror_len += n;
    uart_mirror[uart_mirror_len++] = '\n';
    uart_mirror_calls++;
    return true;
}

volatile uint32_t system_millis;
static int failures;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, m); failures++; } } while (0)

/* lwIP stubs: capture everything written per pcb */
static struct tcp_pcb pcbs[64]; static int npcb;
static char out[64][70000]; static size_t outn[64];
static tcp_accept_fn acc[100]; static uint16_t bindport;
static tcp_recv_fn rf[64]; static void *rarg[64]; static int closed[64];
static tcp_sent_fn sf[64]; static tcp_poll_fn pf[64]; static tcp_err_fn ef[64];
static u16_t sndbuf = 6000; static int write_fail;
u16_t tcp_sndbuf(struct tcp_pcb *pcb) { (void)pcb; return sndbuf; }
u16_t tcp_sndqueuelen(struct tcp_pcb *pcb) { (void)pcb; return 0; }
err_t tcp_write(struct tcp_pcb *pcb, const void *d, u16_t n, u8_t f) { (void)f; if (write_fail) return ERR_MEM; if (outn[pcb->id] + n > sizeof out[0]) outn[pcb->id] = 0; memcpy(out[pcb->id] + outn[pcb->id], d, n); outn[pcb->id] += n; return ERR_OK; }
err_t tcp_output(struct tcp_pcb *pcb) { (void)pcb; return ERR_OK; }
void  tcp_arg(struct tcp_pcb *pcb, void *a) { rarg[pcb->id] = a; }
void  tcp_recv(struct tcp_pcb *pcb, tcp_recv_fn f) { rf[pcb->id] = f; }
void  tcp_err(struct tcp_pcb *pcb, tcp_err_fn f) { if (f) ef[pcb->id] = f; }
void  tcp_sent(struct tcp_pcb *pcb, tcp_sent_fn f) { if (f) sf[pcb->id] = f; }
void  tcp_poll(struct tcp_pcb *pcb, tcp_poll_fn f, u8_t i) { (void)i; if (f) pf[pcb->id] = f; }
static int close_fail;
err_t tcp_close(struct tcp_pcb *pcb) { closed[pcb->id] = 1; return close_fail ? ERR_MEM : ERR_OK; }
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
static int gen_calls; static http_conn_t *held;
static size_t gen3(void *ctx, uint8_t *dst, size_t cap)
{   /* three 100-byte chunks, then end */
    (void)ctx; (void)cap;
    if (gen_calls >= 3) return 0u;
    memset(dst, 'A' + gen_calls++, 100u);
    return 100u;
}
void dash_http_request(http_conn_t *c, const http_req_t *r)
{
    if (!strcmp(r->path, "/")) http_respond(c, 200, "text/html", "Content-Encoding: gzip\r\n", "PAGE", 4);
    else if (!strcmp(r->path, "/q")) { char v[16] = ""; http_query_get(r, "id", v, sizeof v); http_respond_text(c, 200, v); }
    else if (!strcmp(r->path, "/miss")) { char v[16] = "x"; http_respond_text(c, 200, http_query_get(r, "zz", v, sizeof v) ? "found" : "absent"); }
    else if (!strcmp(r->path, "/s")) http_respond_stream(c, 200, "text/csv", NULL, gen3, NULL);
    else if (!strcmp(r->path, "/e500")) http_respond_text(c, 500, "boom");
    else if (!strcmp(r->path, "/e503")) http_respond_text(c, 503, "busy");
    else if (!strcmp(r->path, "/e299")) http_respond_text(c, 299, "odd");
    else if (!strcmp(r->path, "/hold")) { held = c; }
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
    outn[p->id] = 0; uart_mirror_len = 0; uart_mirror_calls = 0;
    ws_broadcast_txt("hello", 5);
    CHECK(outn[p->id] == 7 && (uint8_t)out[p->id][0] == 0x81 && out[p->id][1] == 5 && !memcmp(out[p->id] + 2, "hello", 5), "server text frame");
    CHECK(uart_mirror_calls == 1u && uart_mirror_len == 6u &&
          memcmp(uart_mirror, "hello\n", 6u) == 0, "broadcast is mirrored as one UART line");
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

static char *text(struct tcp_pcb *p) { out[p->id][outn[p->id]] = 0; return out[p->id]; }
static void eof(struct tcp_pcb *p) { rf[p->id](rarg[p->id], p, NULL, ERR_OK); }
static struct tcp_pcb *ws_open(void)
{
    struct tcp_pcb *p = conn(81);
    const char *req = "GET / HTTP/1.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                      "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n";
    feed(p, req, strlen(req));
    outn[p->id] = 0;
    return p;
}
static void ws_frame(struct tcp_pcb *p, uint8_t op, const char *pl)
{
    uint8_t fr[64]; size_t n = 0, l = strlen(pl);
    fr[n++] = (uint8_t)(0x80 | op); fr[n++] = (uint8_t)(0x80 | l);
    fr[n++] = 0; fr[n++] = 0; fr[n++] = 0; fr[n++] = 0;   /* zero mask */
    memcpy(fr + n, pl, l); n += l;
    feed(p, fr, n);
}

static void test_http_more(void)
{
    struct tcp_pcb *p;
    p = conn(80); feed(p, "GET /miss?a=1 HTTP/1.1\r\n\r\n", 26);
    CHECK(strstr(text(p), "\r\n\r\nabsent") != NULL, "missing query key");
    gen_calls = 0; p = conn(80); feed(p, "GET /s HTTP/1.1\r\n\r\n", 19);
    CHECK(strstr(text(p), "Content-Type: text/csv") && strstr(text(p), "AAAA") && strstr(text(p), "CCCC") && closed[p->id],
          "streamed response drained and closed");
    p = conn(80); feed(p, "GET /e500 HTTP/1.1\r\n\r\n", 22); CHECK(strstr(text(p), "500 Internal Server Error") != NULL, "500");
    p = conn(80); feed(p, "GET /e503 HTTP/1.1\r\n\r\n", 22); CHECK(strstr(text(p), "503 Service Unavailable") != NULL, "503");
    p = conn(80); feed(p, "GET /e299 HTTP/1.1\r\n\r\n", 22); CHECK(strstr(text(p), "299 OK") != NULL, "unknown status text");
    /* header without terminator that fills the buffer -> 400 */
    static char junk[1100]; memset(junk, 'h', sizeof junk);
    p = conn(80); feed(p, junk, sizeof junk); CHECK(strstr(text(p), "400 Bad Request") != NULL, "header too large");
    /* path longer than the request path buffer -> 414 */
    static char lp[300]; size_t ln = (size_t)sprintf(lp, "GET /"); memset(lp + ln, 'p', 150); ln += 150;
    ln += (size_t)sprintf(lp + ln, " HTTP/1.1\r\n\r\n");
    p = conn(80); feed(p, lp, ln); CHECK(strstr(text(p), "414 URI Too Long") != NULL, "uri too long");
    /* slow sender: room < 64 stops the pump; http_sent resumes; tcp_close failure aborts */
    sndbuf = 10; gen_calls = 0;
    p = conn(80); feed(p, "GET /s HTTP/1.1\r\n\r\n", 19);
    CHECK(!closed[p->id], "pump waits for send room");
    sndbuf = 6000; close_fail = 1;
    sf[p->id](rarg[p->id], p, 10);
    close_fail = 0;
    CHECK(closed[p->id] && strstr(text(p), "CCCC"), "sent callback resumes the stream");
    /* write failure mid-stream: retried from the poll callback */
    write_fail = 1; gen_calls = 0;
    p = conn(80); feed(p, "GET /s HTTP/1.1\r\n\r\n", 19);
    write_fail = 0;
    pf[p->id](rarg[p->id], p);
    CHECK(closed[p->id], "poll pumps the pending response");
    /* idle connection: poll after 30 s frees it; error callback frees a slot */
    held = NULL; p = conn(80); feed(p, "GET /hold HTTP/1.1\r\n\r\n", 22);
    system_millis += 31000; pf[p->id](rarg[p->id], p);
    CHECK(closed[p->id], "idle connection closed by poll");
    p = conn(80); ef[p->id](rarg[p->id], ERR_ABRT);
    ef[p->id](NULL, ERR_ABRT); sf[p->id](NULL, p, 0); pf[p->id](NULL, p);
    p = conn(80); eof(p); CHECK(closed[p->id], "peer close frees the connection");
}

static void up_hdr(struct tcp_pcb *p, size_t clen)
{
    char h[200]; int n = sprintf(h, "POST /update-stm32 HTTP/1.1\r\nContent-Type: multipart/form-data; boundary=QQ\r\nContent-Length: %zu\r\n\r\n", clen);
    feed(p, h, (size_t)n);
}

static void test_upload_more(void)
{
    /* whole small upload inside the first segment (body in header buffer) */
    const char *part = "--QQ\r\nContent-Disposition: form-data; name=\"f\"\r\n\r\nHELLO\r\n--QQ--\r\n";
    char all[400]; int an = sprintf(all, "POST /update-stm32 HTTP/1.1\r\nContent-Type: multipart/form-data; boundary=QQ\r\nContent-Length: %zu\r\n\r\n%s", strlen(part), part);
    upload_end_ok = -1; struct tcp_pcb *p = conn(80); feed(p, all, (size_t)an);
    CHECK(upload_end_ok == 1 && upload_n == 5 && !memcmp(upload, "HELLO", 5), "upload inside first segment");
    /* one oversized segment: headers fill hdr[], the rest of the body follows in the same pbuf */
    static char big[3000]; static char body[2600];
    size_t bn = (size_t)sprintf(body, "--QQ\r\nContent-Disposition: form-data; name=\"f\"\r\n\r\n");
    memset(body + bn, 'z', 2000); bn += 2000; bn += (size_t)sprintf(body + bn, "\r\n--QQ--\r\n");
    int hn = sprintf(big, "POST /update-stm32 HTTP/1.1\r\nContent-Type: multipart/form-data; boundary=QQ\r\nContent-Length: %zu\r\n\r\n", bn);
    memcpy(big + hn, body, bn);
    upload_end_ok = -1; p = conn(80); feed(p, big, (size_t)hn + bn);
    CHECK(upload_end_ok == 1 && upload_n == 2000, "upload completing in the header segment");
    /* part headers larger than the buffer -> upload fails */
    static char ph[1500]; size_t pn = (size_t)sprintf(ph, "--QQ\r\nX: "); memset(ph + pn, 'x', 1200); pn += 1200;
    upload_end_ok = -1; p = conn(80); up_hdr(p, pn + 10); feed(p, ph, pn); feed(p, "0123456789", 10);
    CHECK(upload_end_ok == 0, "oversized part headers rejected");
    /* second upload while one is running -> 503; peer close of the first aborts it */
    upload_end_ok = -1; p = conn(80); up_hdr(p, 5000);
    struct tcp_pcb *q = conn(80); up_hdr(q, 10);
    CHECK(strstr(text(q), "503 Service Unavailable") != NULL, "single flash at a time");
    eof(p); CHECK(upload_end_ok == 0, "peer close aborts the upload");
    /* error and timeout during an upload */
    upload_end_ok = -1; p = conn(80); up_hdr(p, 5000);
    ef[p->id](rarg[p->id], ERR_RST); CHECK(upload_end_ok == 0, "tcp error aborts the upload");
    upload_end_ok = -1; p = conn(80); up_hdr(p, 5000);
    system_millis += 31000; pf[p->id](rarg[p->id], p); CHECK(upload_end_ok == 0, "stalled upload times out");
}

static void test_ws_more(void)
{
    struct tcp_pcb *p = ws_open();
    ws_frame(p, 0x9, "hi");
    CHECK((uint8_t)out[p->id][0] == 0x8A && out[p->id][1] == 2 && !memcmp(out[p->id] + 2, "hi", 2), "ping answered with pong");
    outn[p->id] = 0; system_millis += 15000; web_server_poll();
    ws_frame(p, 0xA, "");
    system_millis += 3000; web_server_poll(); system_millis += 3000; web_server_poll();
    CHECK(ws_connected_clients() == 1, "pong clears the missed-ping count");
    outn[p->id] = 0; ws_send_txt(0, "abc", 3); ws_send_txt(9, "abc", 3);
    CHECK(outn[p->id] == 5, "unicast text");
    CHECK(ws_send_txt_ok(0, "abc", 3) && ws_send_txt_ok(9, "x", 1), "send_ok");
    static char huge[70000]; memset(huge, 'b', sizeof huge);
    CHECK(!ws_send_txt_ok(0, huge, sizeof huge), "frames over 64 KiB refused");
    const int d0 = disconnected_n;
    ws_frame(p, 0x8, "");
    CHECK(disconnected_n == d0 + 1 && ws_connected_clients() == 0, "close frame");
    p = ws_open();
    ef[p->id](rarg[p->id], ERR_RST); ef[p->id](NULL, ERR_RST);
    CHECK(disconnected_n == d0 + 2, "tcp error disconnects the client");
}

int main(void)
{
    if (!web_server_init()) { printf("FAIL init\n"); return 1; }
    test_ws(); test_http(); test_upload(); test_upload_coalesced();
    test_http_more(); test_upload_more(); test_ws_more();
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("test_web_server: all checks passed\n");
    return 0;
}
