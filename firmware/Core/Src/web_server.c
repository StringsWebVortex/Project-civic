/* HTTP (80) + WebSocket (81) transport on lwIP raw TCP. See web_server.h. */
#include "web_server.h"

#include <stdio.h>
#include <string.h>
#include "lwip/tcp.h"
#include "uart_esp.h"

extern volatile uint32_t system_millis;

/* ═════════════════════════ SHA-1 + base64 (WS handshake) ═══════════════ */
typedef struct { uint32_t h[5]; uint8_t buf[64]; uint32_t n; uint64_t total; } sha1_t;

static uint32_t rol(uint32_t v, unsigned s) { return (v << s) | (v >> (32u - s)); }

static void sha1_block(sha1_t *c, const uint8_t *p)
{
    uint32_t w[80];
    for (unsigned i = 0; i < 16u; i++)
        w[i] = ((uint32_t)p[4*i] << 24) | ((uint32_t)p[4*i+1] << 16) | ((uint32_t)p[4*i+2] << 8) | p[4*i+3];
    for (unsigned i = 16; i < 80u; i++) w[i] = rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    uint32_t a = c->h[0], b = c->h[1], cc = c->h[2], d = c->h[3], e = c->h[4];
    for (unsigned i = 0; i < 80u; i++) {
        uint32_t f, k;
        if (i < 20u)      { f = (b & cc) | (~b & d);            k = 0x5A827999u; }
        else if (i < 40u) { f = b ^ cc ^ d;                     k = 0x6ED9EBA1u; }
        else if (i < 60u) { f = (b & cc) | (b & d) | (cc & d);  k = 0x8F1BBCDCu; }
        else              { f = b ^ cc ^ d;                     k = 0xCA62C1D6u; }
        const uint32_t t = rol(a, 5) + f + e + k + w[i];
        e = d; d = cc; cc = rol(b, 30); b = a; a = t;
    }
    c->h[0] += a; c->h[1] += b; c->h[2] += cc; c->h[3] += d; c->h[4] += e;
}

static void sha1_init(sha1_t *c)
{
    c->h[0] = 0x67452301u; c->h[1] = 0xEFCDAB89u; c->h[2] = 0x98BADCFEu;
    c->h[3] = 0x10325476u; c->h[4] = 0xC3D2E1F0u; c->n = 0; c->total = 0;
}

static void sha1_update(sha1_t *c, const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    c->total += len;
    while (len--) { c->buf[c->n++] = *p++; if (c->n == 64u) { sha1_block(c, c->buf); c->n = 0; } }
}

static void sha1_final(sha1_t *c, uint8_t out[20])
{
    const uint64_t bits = c->total * 8u;
    const uint8_t pad = 0x80u, zero = 0u;
    sha1_update(c, &pad, 1);
    while (c->n != 56u) sha1_update(c, &zero, 1);
    uint8_t len[8];
    for (unsigned i = 0; i < 8u; i++) len[i] = (uint8_t)(bits >> (56u - 8u * i));
    sha1_update(c, len, 8);
    for (unsigned i = 0; i < 20u; i++) out[i] = (uint8_t)(c->h[i / 4u] >> (24u - 8u * (i % 4u)));
}

static size_t base64(const uint8_t *in, size_t n, char *out)
{
    static const char t[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t o = 0;
    for (size_t i = 0; i < n; i += 3) {
        const uint32_t v = ((uint32_t)in[i] << 16) | (i + 1 < n ? (uint32_t)in[i+1] << 8 : 0u) |
                           (i + 2 < n ? in[i+2] : 0u);
        out[o++] = t[(v >> 18) & 63u]; out[o++] = t[(v >> 12) & 63u];
        out[o++] = (i + 1 < n) ? t[(v >> 6) & 63u] : '=';
        out[o++] = (i + 2 < n) ? t[v & 63u] : '=';
    }
    out[o] = '\0';
    return o;
}

/* ═════════════════════════════ HTTP ════════════════════════════════════ */
#define HTTP_SLOTS     2u
#define HTTP_HDR_MAX   1024u

enum { H_FREE = 0, H_HEADERS, H_UPLOAD, H_SENDING };
enum { M_PREAMBLE = 0, M_DATA, M_END };

struct http_conn {
    struct tcp_pcb *pcb;
    uint8_t  state;
    char     hdr[HTTP_HDR_MAX];
    uint16_t hlen;
    http_req_t req;
    /* response body */
    const uint8_t *body; size_t body_len, body_off;
    http_gen_fn gen; void *gen_ctx; bool gen_done;
    uint8_t  chunk[256]; size_t chunk_len, chunk_off;
    uint32_t last_ms;
    /* upload */
    uint32_t received;
    uint8_t  mstate;
    char     delim[80]; uint8_t dlen;
    uint8_t  tail[80];  uint8_t tlen;
    uint8_t  out[256];  uint16_t olen;
    bool     upload_ok;
};

static http_conn_t s_http[HTTP_SLOTS];
static uint8_t s_upload_owner = 0xFFu;

static void http_free(http_conn_t *c)
{
    if (c->pcb) {
        tcp_arg(c->pcb, NULL); tcp_recv(c->pcb, NULL); tcp_sent(c->pcb, NULL);
        tcp_err(c->pcb, NULL); tcp_poll(c->pcb, NULL, 0);
        if (tcp_close(c->pcb) != ERR_OK) tcp_abort(c->pcb);
    }
    if (s_upload_owner == (uint8_t)(c - s_http)) s_upload_owner = 0xFFu;
    c->pcb = NULL; c->state = H_FREE;
}

static void http_pump(http_conn_t *c)
{
    if (!c->pcb || c->state != H_SENDING) return;
    for (;;) {
        u16_t room = tcp_sndbuf(c->pcb);
        if (room < 64u) break;
        if (c->body && c->body_off < c->body_len) {
            size_t n = c->body_len - c->body_off; if (n > room) n = room; if (n > TCP_MSS) n = TCP_MSS;
            if (tcp_write(c->pcb, c->body + c->body_off, (u16_t)n, TCP_WRITE_FLAG_COPY) != ERR_OK) break;
            c->body_off += n;
            continue;
        }
        if (c->gen && !c->gen_done) {
            if (c->chunk_off >= c->chunk_len) {
                c->chunk_len = c->gen(c->gen_ctx, c->chunk, sizeof c->chunk); c->chunk_off = 0;
                if (c->chunk_len == 0u) { c->gen_done = true; continue; }
            }
            size_t n = c->chunk_len - c->chunk_off; if (n > room) n = room;
            if (tcp_write(c->pcb, c->chunk + c->chunk_off, (u16_t)n, TCP_WRITE_FLAG_COPY) != ERR_OK) break;
            c->chunk_off += n;
            continue;
        }
        /* everything queued: graceful close once lwIP has sent it */
        tcp_output(c->pcb);
        http_free(c);
        return;
    }
    tcp_output(c->pcb);
}

static const char *status_text(int s)
{
    switch (s) {
    case 200: return "OK"; case 302: return "Found"; case 400: return "Bad Request"; case 401: return "Unauthorized";
    case 404: return "Not Found"; case 410: return "Gone"; case 413: return "Payload Too Large"; case 414: return "URI Too Long";
    case 500: return "Internal Server Error"; case 503: return "Service Unavailable";
    default:  return "OK";
    }
}

static void http_start(http_conn_t *c, int status, const char *ctype, const char *extra, long len)
{
    char h[384];
    int n = snprintf(h, sizeof h, "HTTP/1.1 %d %s\r\nContent-Type: %s\r\n%s", status, status_text(status),
                     ctype, extra ? extra : "");
    if (len >= 0) n += snprintf(h + n, sizeof h - (size_t)n, "Content-Length: %ld\r\n", len);
    n += snprintf(h + n, sizeof h - (size_t)n, "Connection: close\r\n\r\n");
    if (n > 0 && (size_t)n < sizeof h) (void)tcp_write(c->pcb, h, (u16_t)n, TCP_WRITE_FLAG_COPY);
    c->state = H_SENDING;
}

void http_respond(http_conn_t *c, int status, const char *ctype, const char *extra,
                  const void *body, size_t len)
{
    if (!c || !c->pcb) return;
    http_start(c, status, ctype, extra, (long)len);
    c->body = (const uint8_t *)body; c->body_len = len; c->body_off = 0; c->gen = NULL;
    http_pump(c);
}

void http_respond_text(http_conn_t *c, int status, const char *text)
{
    http_respond(c, status, "text/plain", NULL, text, strlen(text));
}

void http_respond_stream(http_conn_t *c, int status, const char *ctype, const char *extra,
                         http_gen_fn gen, void *ctx)
{
    if (!c || !c->pcb) return;
    http_start(c, status, ctype, extra, -1);
    c->body = NULL; c->gen = gen; c->gen_ctx = ctx; c->gen_done = false; c->chunk_len = c->chunk_off = 0;
    http_pump(c);
}

bool http_query_get(const http_req_t *r, const char *key, char *out, size_t cap)
{
    const size_t kl = strlen(key);
    const char *p = r->query;
    while (*p) {
        const char *amp = strchr(p, '&'); const size_t seg = amp ? (size_t)(amp - p) : strlen(p);
        if (seg >= kl && strncmp(p, key, kl) == 0 && (seg == kl || p[kl] == '=')) {
            const char *v = (seg == kl) ? p + kl : p + kl + 1;
            size_t vl = seg - (size_t)(v - p); if (vl >= cap) vl = cap - 1u;
            memcpy(out, v, vl); out[vl] = '\0';
            return true;
        }
        if (!amp) break;
        p = amp + 1;
    }
    return false;
}

static bool header_value(const char *hdr, const char *name, char *out, size_t cap)
{
    const size_t nl = strlen(name);
    for (const char *p = hdr; (p = strstr(p, "\r\n")) != NULL; ) {
        p += 2;
        if (strncasecmp(p, name, nl) == 0 && p[nl] == ':') {
            const char *v = p + nl + 1; while (*v == ' ') v++;
            const char *e = strstr(v, "\r\n"); size_t l = e ? (size_t)(e - v) : strlen(v);
            if (l >= cap) l = cap - 1u;
            memcpy(out, v, l); out[l] = '\0';
            return true;
        }
    }
    return false;
}

static void upload_flush(http_conn_t *c)
{
    if (c->olen && c->upload_ok) c->upload_ok = dash_http_upload_data(c->out, c->olen);
    c->olen = 0;
}

static void upload_feed(http_conn_t *c, const uint8_t *d, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (c->mstate == M_END) return;
        if (c->mstate == M_PREAMBLE) {
            /* skip "--boundary\r\n<part headers>\r\n\r\n" */
            if (c->hlen + 1u < sizeof c->hdr) { c->hdr[c->hlen++] = (char)d[i]; c->hdr[c->hlen] = '\0'; }
            else { c->upload_ok = false; c->mstate = M_END; return; }
            if (c->hlen >= 4u && memcmp(c->hdr + c->hlen - 4u, "\r\n\r\n", 4) == 0) c->mstate = M_DATA;
            continue;
        }
        /* M_DATA: hold back dlen bytes to detect "\r\n--boundary" */
        c->tail[c->tlen++] = d[i];
        if (c->tlen == c->dlen) {
            if (memcmp(c->tail, c->delim, c->dlen) == 0) { upload_flush(c); c->mstate = M_END; return; }
            c->out[c->olen++] = c->tail[0];
            memmove(c->tail, c->tail + 1, --c->tlen);
            if (c->olen == sizeof c->out) upload_flush(c);
        }
    }
}

static void http_handle_headers(http_conn_t *c)
{
    char *end = strstr(c->hdr, "\r\n\r\n");
    if (!end) {
        if (c->hlen + 1u >= sizeof c->hdr) { http_respond_text(c, 400, "Header too large"); }
        return;
    }
    const size_t head_len = (size_t)(end - c->hdr) + 4u;
    http_req_t *r = &c->req;
    memset(r, 0, sizeof *r);
    char target[160] = "";
    if (sscanf(c->hdr, "%7s %159s", r->method, target) != 2) { http_respond_text(c, 400, "Bad request"); return; }
    char *q = strchr(target, '?');
    if (q) *q++ = '\0';
    if (strlen(target) >= sizeof r->path || (q && strlen(q) >= sizeof r->query)) {
        http_respond_text(c, 414, "URI too long"); return;
    }
    memcpy(r->path, target, strlen(target) + 1u);
    if (q) memcpy(r->query, q, strlen(q) + 1u);
    char v[128];
    if (header_value(c->hdr, "Content-Length", v, sizeof v)) r->content_length = (uint32_t)strtoul(v, NULL, 10);
    if (header_value(c->hdr, "Content-Type", v, sizeof v) && strncasecmp(v, "multipart/form-data", 19) == 0) {
        const char *b = strstr(v, "boundary=");
        if (b) {
            b += 9; if (*b == '"') b++;
            size_t bl = strcspn(b, "\";");
            if (bl + 4u < sizeof c->delim) {
                c->delim[0] = '\r'; c->delim[1] = '\n'; c->delim[2] = '-'; c->delim[3] = '-';
                memcpy(c->delim + 4, b, bl); c->dlen = (uint8_t)(bl + 4u);
                r->multipart = true;
            }
        }
    }
    if (strcmp(r->method, "POST") == 0 && r->multipart) {
        if (s_upload_owner != 0xFFu) { http_respond_text(c, 503, "Flash already in progress"); return; }
        s_upload_owner = (uint8_t)(c - s_http);
        c->upload_ok = dash_http_upload_begin(r);
        c->state = H_UPLOAD; c->mstate = M_PREAMBLE; c->tlen = 0; c->olen = 0; c->received = 0;
        /* body bytes already in the header buffer */
        const size_t extra = c->hlen - head_len;
        uint8_t tmp[HTTP_HDR_MAX];
        memcpy(tmp, c->hdr + head_len, extra);
        c->hlen = 0; c->hdr[0] = '\0';
        c->received = (uint32_t)extra;
        upload_feed(c, tmp, extra);
        if (c->received >= r->content_length) {
            upload_flush(c);
            dash_http_upload_end(c, r, c->upload_ok && c->mstate == M_END);
            s_upload_owner = 0xFFu;
        }
        return;
    }
    dash_http_request(c, r);
    if (c->state == H_HEADERS) http_respond_text(c, 404, "Not found");
}

static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    http_conn_t *c = (http_conn_t *)arg;
    (void)err;
    if (!c) { if (p) pbuf_free(p); return ERR_OK; }
    if (!p) {
        if (c->state == H_UPLOAD) { dash_http_upload_end(NULL, &c->req, false); s_upload_owner = 0xFFu; }
        http_free(c);
        return ERR_OK;
    }
    tcp_recved(pcb, p->tot_len);
    c->last_ms = system_millis;
    for (struct pbuf *q = p; q; q = q->next) {
        const uint8_t *d = (const uint8_t *)q->payload;
        if (c->state == H_HEADERS) {
            size_t n = q->len;
            if (c->hlen + n >= sizeof c->hdr) n = sizeof c->hdr - 1u - c->hlen;
            memcpy(c->hdr + c->hlen, d, n); c->hlen = (uint16_t)(c->hlen + n); c->hdr[c->hlen] = '\0';
            http_handle_headers(c);
            /* Headers and the start of the body often share one TCP segment:
             * bytes of this pbuf beyond what fitted in hdr[] are body bytes. */
            if (c->state == H_UPLOAD && n < q->len) {
                c->received += (uint32_t)(q->len - n);
                upload_feed(c, d + n, q->len - n);
                if (c->received >= c->req.content_length) {
                    upload_flush(c);
                    dash_http_upload_end(c, &c->req, c->upload_ok && c->mstate == M_END);
                    s_upload_owner = 0xFFu;
                }
            }
        } else if (c->state == H_UPLOAD) {
            c->received += q->len;
            upload_feed(c, d, q->len);
            if (c->received >= c->req.content_length) {
                upload_flush(c);
                dash_http_upload_end(c, &c->req, c->upload_ok && c->mstate == M_END);
                s_upload_owner = 0xFFu;
            }
        }
        if (c->state == H_FREE) break;
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t http_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    (void)pcb; (void)len;
    http_conn_t *c = (http_conn_t *)arg;
    if (c) { c->last_ms = system_millis; http_pump(c); }
    return ERR_OK;
}

static void http_err(void *arg, err_t err)
{
    (void)err;
    http_conn_t *c = (http_conn_t *)arg;
    if (!c) return;
    if (c->state == H_UPLOAD) { dash_http_upload_end(NULL, &c->req, false); }
    if (s_upload_owner == (uint8_t)(c - s_http)) s_upload_owner = 0xFFu;
    c->pcb = NULL; c->state = H_FREE;      /* pcb already freed by lwIP */
}

static err_t http_poll(void *arg, struct tcp_pcb *pcb)
{
    (void)pcb;
    http_conn_t *c = (http_conn_t *)arg;
    if (c && (uint32_t)(system_millis - c->last_ms) > 30000u) {
        if (c->state == H_UPLOAD) dash_http_upload_end(NULL, &c->req, false);
        http_free(c);
    }
    else if (c) http_pump(c);
    return ERR_OK;
}

static err_t http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    (void)arg;
    if (err != ERR_OK || !pcb) return ERR_VAL;
    http_conn_t *c = NULL;
    for (unsigned i = 0; i < HTTP_SLOTS; i++) if (s_http[i].state == H_FREE) { c = &s_http[i]; break; }
    if (!c) { tcp_abort(pcb); return ERR_ABRT; }
    memset(c, 0, offsetof(http_conn_t, hdr));
    c->pcb = pcb; c->state = H_HEADERS; c->hlen = 0; c->hdr[0] = '\0'; c->last_ms = system_millis;
    c->body = NULL; c->gen = NULL;
    tcp_arg(pcb, c); tcp_recv(pcb, http_recv); tcp_sent(pcb, http_sent);
    tcp_err(pcb, http_err); tcp_poll(pcb, http_poll, 2);
    return ERR_OK;
}

/* ═════════════════════════════ WebSocket ═══════════════════════════════ */
typedef struct {
    struct tcp_pcb *pcb;
    bool     open;            /* handshake done */
    uint8_t  rx[WS_MAX_MESSAGE + 16u];
    uint16_t rlen;
    uint32_t last_ping_ms, ping_sent_ms;
    bool     ping_pending;
    uint8_t  missed;
} ws_client_t;

static ws_client_t s_ws[WS_MAX_CLIENTS];

static void ws_drop(ws_client_t *w, bool notify)
{
    const uint8_t num = (uint8_t)(w - s_ws);
    const bool was_open = w->open;
    if (w->pcb) {
        tcp_arg(w->pcb, NULL); tcp_recv(w->pcb, NULL); tcp_sent(w->pcb, NULL);
        tcp_err(w->pcb, NULL); tcp_poll(w->pcb, NULL, 0);
        if (tcp_close(w->pcb) != ERR_OK) tcp_abort(w->pcb);
    }
    w->pcb = NULL; w->open = false; w->rlen = 0;
    if (notify && was_open) dash_ws_disconnected(num);
}

static bool ws_write_frame(ws_client_t *w, uint8_t opcode, const void *data, size_t n)
{
    if (!w->pcb) return false;
    uint8_t h[4]; size_t hl;
    h[0] = (uint8_t)(0x80u | opcode);
    if (n < 126u) { h[1] = (uint8_t)n; hl = 2; }
    else if (n <= 0xFFFFu) { h[1] = 126u; h[2] = (uint8_t)(n >> 8); h[3] = (uint8_t)n; hl = 4; }
    else return false;
    if (tcp_sndbuf(w->pcb) < n + hl || tcp_sndqueuelen(w->pcb) + 4u > TCP_SND_QUEUELEN) return false;
    if (tcp_write(w->pcb, h, (u16_t)hl, TCP_WRITE_FLAG_COPY | (n ? TCP_WRITE_FLAG_MORE : 0)) != ERR_OK) return false;
    if (n && tcp_write(w->pcb, data, (u16_t)n, TCP_WRITE_FLAG_COPY) != ERR_OK) return false;
    tcp_output(w->pcb);
    return true;
}

void ws_send_txt(uint8_t num, const char *s, size_t n)
{
    if (num < WS_MAX_CLIENTS && s_ws[num].open) (void)ws_write_frame(&s_ws[num], 0x1u, s, n);
}

bool ws_send_txt_ok(uint8_t num, const char *s, size_t n)
{
    if (num >= WS_MAX_CLIENTS || !s_ws[num].open) return true;    /* nothing to deliver to */
    return ws_write_frame(&s_ws[num], 0x1u, s, n);
}

void ws_broadcast_txt(const char *s, size_t n)
{
    /* lwIP WS clients (USB-CDC-ECM direct connection) */
    for (uint8_t i = 0; i < WS_MAX_CLIENTS; i++) if (s_ws[i].open) (void)ws_write_frame(&s_ws[i], 0x1u, s, n);

    /* Mirror to ESP32 UART bridge — ESP forwards to WiFi WS clients.
     * uart_esp_emit_line() sends data + '\n' in one DMA transfer so
     * the ESP can reassemble line-delimited JSON on the other end. */
    uart_esp_emit_line(s, n);
}

uint8_t ws_connected_clients(void)
{
    uint8_t n = 0;
    for (unsigned i = 0; i < WS_MAX_CLIENTS; i++) if (s_ws[i].open) n++;
    return n;
}

static void ws_handshake(ws_client_t *w)
{
    char *end = strstr((char *)w->rx, "\r\n\r\n");
    if (!end) { if (w->rlen >= sizeof w->rx - 1u) ws_drop(w, false); return; }
    char key[64];
    if (!header_value((char *)w->rx, "Sec-WebSocket-Key", key, sizeof key)) { ws_drop(w, false); return; }
    sha1_t s; uint8_t dig[20]; char acc[32];
    sha1_init(&s); sha1_update(&s, key, strlen(key));
    sha1_update(&s, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36); sha1_final(&s, dig);
    base64(dig, 20, acc);
    char resp[160];
    const int n = snprintf(resp, sizeof resp, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n"
                           "Connection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n", acc);
    if (tcp_write(w->pcb, resp, (u16_t)n, TCP_WRITE_FLAG_COPY) != ERR_OK) { ws_drop(w, false); return; }
    tcp_output(w->pcb);
    const size_t used = (size_t)(end - (char *)w->rx) + 4u;
    memmove(w->rx, w->rx + used, w->rlen - used); w->rlen = (uint16_t)(w->rlen - used);
    w->open = true; w->last_ping_ms = system_millis; w->missed = 0; w->ping_pending = false;
    dash_ws_connected((uint8_t)(w - s_ws));
}

static void ws_parse(ws_client_t *w)
{
    while (w->pcb && w->open && w->rlen >= 2u) {
        const uint8_t op = w->rx[0] & 0x0Fu;
        const bool masked = (w->rx[1] & 0x80u) != 0u;
        size_t len = w->rx[1] & 0x7Fu, hl = 2;
        if (len == 126u) { if (w->rlen < 4u) return; len = ((size_t)w->rx[2] << 8) | w->rx[3]; hl = 4; }
        else if (len == 127u) { ws_drop(w, true); return; }
        if (masked) hl += 4u;
        if (hl + len > sizeof w->rx - 1u) { ws_drop(w, true); return; }    /* > 1400: refuse */
        if (w->rlen < hl + len) return;
        uint8_t *pl = w->rx + hl;
        if (masked) { const uint8_t *m = w->rx + hl - 4u; for (size_t i = 0; i < len; i++) pl[i] ^= m[i & 3u]; }
        if (op == 0x1u) {
            char save = (char)pl[len]; pl[len] = '\0';
            dash_ws_text((uint8_t)(w - s_ws), (char *)pl, len);
            pl[len] = (uint8_t)save;
        } else if (op == 0x8u) { (void)ws_write_frame(w, 0x8u, NULL, 0); ws_drop(w, true); return; }
        else if (op == 0x9u) { (void)ws_write_frame(w, 0xAu, pl, len); }
        else if (op == 0xAu) { w->ping_pending = false; w->missed = 0; }
        if (!w->pcb) return;
        memmove(w->rx, w->rx + hl + len, w->rlen - (hl + len));
        w->rlen = (uint16_t)(w->rlen - (hl + len));
    }
}

static err_t ws_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    ws_client_t *w = (ws_client_t *)arg;
    (void)err;
    if (!w) { if (p) pbuf_free(p); return ERR_OK; }
    if (!p) { ws_drop(w, true); return ERR_OK; }
    tcp_recved(pcb, p->tot_len);
    for (struct pbuf *q = p; q && w->pcb; q = q->next) {
        size_t n = q->len;
        if (w->rlen + n > sizeof w->rx - 1u) { pbuf_free(p); ws_drop(w, true); return ERR_OK; }
        memcpy(w->rx + w->rlen, q->payload, n); w->rlen = (uint16_t)(w->rlen + n); w->rx[w->rlen] = 0;
        if (!w->open) ws_handshake(w);
        if (w->open) ws_parse(w);
    }
    pbuf_free(p);
    return ERR_OK;
}

static void ws_err(void *arg, err_t err)
{
    (void)err;
    ws_client_t *w = (ws_client_t *)arg;
    if (!w) return;
    const bool was_open = w->open;
    w->pcb = NULL; w->open = false; w->rlen = 0;
    if (was_open) dash_ws_disconnected((uint8_t)(w - s_ws));
}

static err_t ws_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    (void)arg;
    if (err != ERR_OK || !pcb) return ERR_VAL;
    ws_client_t *w = NULL;
    for (unsigned i = 0; i < WS_MAX_CLIENTS; i++) if (!s_ws[i].pcb) { w = &s_ws[i]; break; }
    if (!w) { tcp_abort(pcb); return ERR_ABRT; }
    w->pcb = pcb; w->open = false; w->rlen = 0;
    tcp_nagle_disable(pcb);
    tcp_arg(pcb, w); tcp_recv(pcb, ws_recv); tcp_err(pcb, ws_err);
    return ERR_OK;
}

/* Heartbeat identical to the ESP32 WebSocketsServer::enableHeartbeat(15000, 3000, 2):
 * ping every 15 s, a ping unanswered for 3 s counts as missed, 2 missed -> disconnect. */
void web_server_poll(void)
{
    const uint32_t now = system_millis;
    for (unsigned i = 0; i < WS_MAX_CLIENTS; i++) {
        ws_client_t *w = &s_ws[i];
        if (!w->open) continue;
        if (w->ping_pending && (uint32_t)(now - w->ping_sent_ms) >= 3000u) {
            w->ping_pending = false;
            if (++w->missed >= 2u) { ws_drop(w, true); continue; }
        }
        if ((uint32_t)(now - w->last_ping_ms) >= 15000u) {
            w->last_ping_ms = now;
            if (ws_write_frame(w, 0x9u, NULL, 0)) { w->ping_pending = true; w->ping_sent_ms = now; }
        }
    }
}

static struct tcp_pcb *listen_on(uint16_t port, tcp_accept_fn fn)
{
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (!pcb) return NULL;
    if (tcp_bind(pcb, IP_ANY_TYPE, port) != ERR_OK) { tcp_close(pcb); return NULL; }
    struct tcp_pcb *l = tcp_listen_with_backlog(pcb, 4);
    if (!l) { tcp_close(pcb); return NULL; }
    tcp_accept(l, fn);
    return l;
}

bool web_server_init(void)
{
    return listen_on(80, http_accept) != NULL && listen_on(81, ws_accept) != NULL;
}
