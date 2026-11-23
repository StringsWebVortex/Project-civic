/* Host tests for the log-mail policy (log_mail.c). */
#include <assert.h>
#include <string.h>
#include "log_mail.h"

static uint8_t jb[8][256]; static uint16_t jl[8]; static uint8_t jt[8]; static uint32_t ji[8]; static int jn, jfail = -1;
static bool append(uint8_t t, uint32_t id, const void *d, uint16_t l)
{
    if (jfail >= 0 && jn >= jfail) return false;
    if (l) memcpy(jb[jn], d, l);
    jl[jn] = l; jt[jn] = t; ji[jn] = id; jn++;
    return true;
}

int main(void)
{
    mail_state_t m; mail_init(&m);
    assert(mail_global() != NULL);
    assert(!strcmp(m.to, MAIL_DEFAULT_ADDR) && !strcmp(m.from, MAIL_DEFAULT_ADDR));
    assert(!mail_configured(&m));

    assert(!mail_check(&m, 699u, 3u));          /* below 70 % */
    assert(!mail_check(&m, 900u, 0u));          /* no sector  */
    assert(mail_check(&m, 700u, 3u) && m.pending_seq == 3u);
    assert(!mail_check(&m, 800u, 3u));          /* raised once */
    assert(!mail_ack(&m, 4u) && !mail_ack(&m, 0u));
    assert(mail_ack(&m, 3u) && m.pending_seq == 0u && m.sent_seq == 3u);
    assert(!mail_check(&m, 950u, 3u));          /* already sent */
    assert(mail_check(&m, 700u, 4u));           /* next sector */
    mail_request(&m, 0u); assert(m.pending_seq == 4u);
    mail_request(&m, 5u); assert(m.pending_seq == 5u);

    assert(!mail_set_cfg(&m, "no-at", "a@b", "pw"));
    assert(!mail_set_cfg(&m, "a@b", "a@b", ""));
    assert(!mail_set_cfg(&m, "a@b", "a,b@c", "pw"));
    assert(!mail_set_cfg(&m, NULL, "a@b", "pw"));
    assert(!mail_set_cfg(&m, "a@b", "a@b", "0123456789012345678901234567890123"));
    assert(!mail_set_cfg(&m, "a@b", "a@b", "p\"w"));
    assert(mail_set_cfg(&m, "x@y.z", "s@t.u", "abcd efgh ijkl mnop"));
    assert(mail_configured(&m));

    jn = 0; assert(mail_rewrite(&m, append) && jn == 2);
    mail_state_t r; mail_init(&r);
    r.pending_seq = 5u;
    for (int i = 0; i < jn; i++) mail_replay(&r, jt[i], ji[i], jb[i], jl[i]);
    assert(!strcmp(r.to, "x@y.z") && !strcmp(r.from, "s@t.u") && !strcmp(r.pass, "abcd efgh ijkl mnop"));
    assert(r.sent_seq == 3u && r.pending_seq == 5u);
    mail_replay(&r, MAIL_REC_SENT, 5u, NULL, 0u); assert(r.pending_seq == 0u && r.sent_seq == 5u);
    mail_replay(&r, MAIL_REC_CFG, 0u, jb[0], 3u);   /* bad length ignored */
    mail_replay(&r, 77, 0u, NULL, 0u);              /* unknown type */
    mail_replay(&r, MAIL_REC_SENT, 9u, jb[0], 1u);  /* bad length */
    assert(r.sent_seq == 5u);
    jn = 0; jfail = 0; assert(!mail_rewrite(&m, append));
    jn = 0; jfail = 1; assert(!mail_rewrite(&m, append));
    return 0;
}
