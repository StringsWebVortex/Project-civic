#ifndef WEB_SERVER_H
#define WEB_SERVER_H
/* Minimal HTTP/1.1 (port 80) + RFC 6455 WebSocket (port 81) servers on the
 * lwIP raw TCP API. Transport only; routes and messages live in dash.c. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define WS_DIRECT_CLIENTS 3u
#define WS_BRIDGE_CLIENT  3u  /* virtual WebSocket peer carried over USART2 */
#define WS_MAX_CLIENTS    4u  /* three TCP peers plus the UART bridge peer */
#define WS_MAX_MESSAGE    1400u   /* same cap the ESP32 dashboard enforced */

typedef struct http_conn http_conn_t;

typedef struct {
    char     method[8];
    char     path[64];
    char     query[96];
    uint32_t content_length;
    bool     multipart;
} http_req_t;

/* Body generator for streamed responses: fill dst (cap bytes), return bytes
 * written; 0 = end of body. */
typedef size_t (*http_gen_fn)(void *ctx, uint8_t *dst, size_t cap);

bool web_server_init(void);
void web_server_poll(void);               /* heartbeats + deferred work */

/* Responses (call once per request). `extra` = extra header lines, each
 * ending in \r\n, or NULL. Static bodies are sent without copying. */
void http_respond(http_conn_t *c, int status, const char *ctype, const char *extra,
                  const void *body, size_t len);
void http_respond_text(http_conn_t *c, int status, const char *text);
void http_respond_stream(http_conn_t *c, int status, const char *ctype, const char *extra,
                         http_gen_fn gen, void *ctx);
/* Query-string helper: copies the value of key into out; false if absent. */
bool http_query_get(const http_req_t *r, const char *key, char *out, size_t cap);

/* WebSocket output */
bool    ws_broadcast_txt(const char *s, size_t n);
void    ws_send_txt(uint8_t num, const char *s, size_t n);
bool    ws_send_txt_ok(uint8_t num, const char *s, size_t n);  /* false = window full, retry later */
uint8_t ws_connected_clients(void);

/* ── implemented by dash.c ─────────────────────────────────────────────── */
void dash_http_request(http_conn_t *c, const http_req_t *r);
bool dash_http_upload_begin(const http_req_t *r);
bool dash_http_upload_data(const uint8_t *d, size_t n);
void dash_http_upload_end(http_conn_t *c, const http_req_t *r, bool complete);
void dash_ws_connected(uint8_t num);
void dash_ws_disconnected(uint8_t num);
void dash_ws_text(uint8_t num, char *msg, size_t len);

#endif
