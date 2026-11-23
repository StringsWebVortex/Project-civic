/* log_mail.c - see log_mail.h. */
#include "log_mail.h"
#include <stdio.h>
#include <string.h>

#ifdef STM32F407xx
static mail_state_t s_mail __attribute__((section(".ccmram")));   /* CPU-only data */
#else
static mail_state_t s_mail;
#endif
mail_state_t *mail_global(void) { return &s_mail; }

void mail_init(mail_state_t *m)
{
    memset(m, 0, sizeof *m);
    (void)snprintf(m->to, sizeof m->to, "%s", MAIL_DEFAULT_ADDR);
    (void)snprintf(m->from, sizeof m->from, "%s", MAIL_DEFAULT_ADDR);
}

bool mail_check(mail_state_t *m, uint32_t fill_permille, uint32_t seq)
{
    if (fill_permille < MAIL_TRIGGER_PERMILLE || seq == 0u) return false;
    if (m->pending_seq == seq || m->sent_seq == seq) return false;
    m->pending_seq = seq;
    return true;
}

void mail_request(mail_state_t *m, uint32_t seq) { if (seq) m->pending_seq = seq; }

bool mail_ack(mail_state_t *m, uint32_t seq)
{
    if (seq == 0u || seq != m->pending_seq) return false;
    m->sent_seq = seq;
    m->pending_seq = 0u;
    return true;
}

static bool field_ok(const char *s, size_t cap, bool email)
{
    if (!s || !s[0] || strlen(s) >= cap) return false;
    for (const char *p = s; *p; p++) if (*p == ',' || *p == '"' || *p == '\r' || *p == '\n' || *p == '\\') return false;
    return !email || (strchr(s, '@') != NULL);
}

bool mail_set_cfg(mail_state_t *m, const char *to, const char *from, const char *pass)
{
    if (!field_ok(to, MAIL_ADDR_MAX, true) || !field_ok(from, MAIL_ADDR_MAX, true) ||
        !field_ok(pass, MAIL_PASS_MAX, false)) return false;
    (void)snprintf(m->to, sizeof m->to, "%s", to);
    (void)snprintf(m->from, sizeof m->from, "%s", from);
    (void)snprintf(m->pass, sizeof m->pass, "%s", pass);
    return true;
}

bool mail_configured(const mail_state_t *m) { return m->pass[0] != '\0'; }

#define CFG_LEN (MAIL_ADDR_MAX * 2u + MAIL_PASS_MAX)

void mail_replay(mail_state_t *m, uint8_t type, uint32_t id, const uint8_t *data, uint16_t len)
{
    if (type == MAIL_REC_CFG && len == CFG_LEN) {
        memcpy(m->to, data, MAIL_ADDR_MAX);                       m->to[MAIL_ADDR_MAX - 1u] = '\0';
        memcpy(m->from, data + MAIL_ADDR_MAX, MAIL_ADDR_MAX);     m->from[MAIL_ADDR_MAX - 1u] = '\0';
        memcpy(m->pass, data + 2u * MAIL_ADDR_MAX, MAIL_PASS_MAX); m->pass[MAIL_PASS_MAX - 1u] = '\0';
    } else if (type == MAIL_REC_SENT && len == 0u) {
        m->sent_seq = id;
        if (m->pending_seq == id) m->pending_seq = 0u;
    }
}

bool mail_persist_cfg(const mail_state_t *m, mail_append_fn append)
{
    uint8_t b[CFG_LEN];
    memcpy(b, m->to, MAIL_ADDR_MAX);
    memcpy(b + MAIL_ADDR_MAX, m->from, MAIL_ADDR_MAX);
    memcpy(b + 2u * MAIL_ADDR_MAX, m->pass, MAIL_PASS_MAX);
    return append(MAIL_REC_CFG, 0u, b, (uint16_t)sizeof b);
}

bool mail_persist_sent(const mail_state_t *m, mail_append_fn append)
{
    return append(MAIL_REC_SENT, m->sent_seq, NULL, 0u);
}

bool mail_rewrite(const mail_state_t *m, mail_append_fn append)
{
    return mail_persist_cfg(m, append) && mail_persist_sent(m, append);
}
