/* Host shim: the lwIP raw-TCP surface used by dash.c. */
#ifndef HOST_SHIM_LWIP_TCP_H
#define HOST_SHIM_LWIP_TCP_H
#include <stdint.h>
typedef int8_t err_t; typedef uint16_t u16_t; typedef uint8_t u8_t;
#define ERR_OK 0
#define ERR_VAL (-6)
#define ERR_ABRT (-13)
#define ERR_MEM (-1)
#define ERR_RST (-14)
#define TCP_WRITE_FLAG_COPY 1
#define TCP_WRITE_FLAG_MORE 2
#define IPADDR_TYPE_V4 0
#define IP_ANY_TYPE NULL
struct tcp_pcb { int id; };
struct pbuf { struct pbuf *next; void *payload; u16_t len, tot_len; };
typedef err_t (*tcp_accept_fn)(void *arg, struct tcp_pcb *newpcb, err_t err);
typedef err_t (*tcp_recv_fn)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
typedef void  (*tcp_err_fn)(void *arg, err_t err);
u16_t tcp_sndbuf(struct tcp_pcb *pcb);
err_t tcp_write(struct tcp_pcb *pcb, const void *data, u16_t len, u8_t flags);
err_t tcp_output(struct tcp_pcb *pcb);
void  tcp_arg(struct tcp_pcb *pcb, void *arg);
void  tcp_recv(struct tcp_pcb *pcb, tcp_recv_fn f);
void  tcp_err(struct tcp_pcb *pcb, tcp_err_fn f);
err_t tcp_close(struct tcp_pcb *pcb);
void  tcp_abort(struct tcp_pcb *pcb);
void  tcp_recved(struct tcp_pcb *pcb, u16_t len);
void  tcp_nagle_disable(struct tcp_pcb *pcb);
struct tcp_pcb *tcp_new_ip_type(u8_t type);
err_t tcp_bind(struct tcp_pcb *pcb, const void *ip, u16_t port);
struct tcp_pcb *tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t backlog);
void  tcp_accept(struct tcp_pcb *pcb, tcp_accept_fn f);
u8_t  pbuf_free(struct pbuf *p);
typedef err_t (*tcp_sent_fn)(void *arg, struct tcp_pcb *tpcb, u16_t len);
typedef err_t (*tcp_poll_fn)(void *arg, struct tcp_pcb *tpcb);
void  tcp_sent(struct tcp_pcb *pcb, tcp_sent_fn f);
void  tcp_poll(struct tcp_pcb *pcb, tcp_poll_fn f, u8_t interval);
u16_t tcp_sndqueuelen(struct tcp_pcb *pcb);
#define TCP_MSS 1460
#define TCP_SND_QUEUELEN 16
#endif
