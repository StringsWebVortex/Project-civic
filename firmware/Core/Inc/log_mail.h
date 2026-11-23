#ifndef LOG_MAIL_H
#define LOG_MAIL_H
/*
 * log_mail.h - "email the logs when they are 70 % full" policy (pure logic).
 *
 * The board decides WHEN logs must be mailed and keeps the request pending
 * (persisted) until a sender confirms delivery. The SMTP/TLS transfer itself
 * runs on the internet-connected host (tools/log_mailer.py): Gmail accepts
 * only TLS, and a TLS session does not fit next to the CAN buffers in the
 * STM32F407's 128 KB main RAM. If there is no internet the request simply
 * stays pending and is sent as soon as the host is online.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAIL_ADDR_MAX     64u
#define MAIL_PASS_MAX     32u
#define MAIL_TRIGGER_PERMILLE 700u
#define MAIL_DEFAULT_ADDR "stringswebvortex@gmail.com"

typedef struct {
    char     to[MAIL_ADDR_MAX];
    char     from[MAIL_ADDR_MAX];
    char     pass[MAIL_PASS_MAX];   /* Gmail App Password (set in Settings) */
    uint32_t sent_seq;              /* last log sector sequence mailed      */
    uint32_t pending_seq;           /* 0 = nothing pending                  */
} mail_state_t;

enum { MAIL_REC_CFG = 8, MAIL_REC_SENT = 9 };

void mail_init(mail_state_t *m);
/* Call after every log append: fill = current sector use in permille,
 * seq = current sector sequence. Returns true when a NEW request was raised. */
bool mail_check(mail_state_t *m, uint32_t fill_permille, uint32_t seq);
/* Manual "send now". */
void mail_request(mail_state_t *m, uint32_t seq);
/* Sender confirms delivery of seq. false if seq is not the pending one. */
bool mail_ack(mail_state_t *m, uint32_t seq);
bool mail_set_cfg(mail_state_t *m, const char *to, const char *from, const char *pass);
bool mail_configured(const mail_state_t *m);

/* Persistence (dash_store journal). */
typedef bool (*mail_append_fn)(uint8_t type, uint32_t id, const void *data, uint16_t len);
void mail_replay(mail_state_t *m, uint8_t type, uint32_t id, const uint8_t *data, uint16_t len);
bool mail_rewrite(const mail_state_t *m, mail_append_fn append);
bool mail_persist_cfg(const mail_state_t *m, mail_append_fn append);
bool mail_persist_sent(const mail_state_t *m, mail_append_fn append);

mail_state_t *mail_global(void);
#endif
