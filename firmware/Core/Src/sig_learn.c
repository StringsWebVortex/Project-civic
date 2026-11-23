/* Signal learn mode (see sig_learn.h). Pure logic: no I/O, host-testable. */
#include "sig_learn.h"

#include <math.h>
#include <string.h>

static uint16_t sat_inc(uint16_t v) { return v == 0xFFFFu ? v : (uint16_t)(v + 1u); }

void sl_init(sl_t *s)
{
    memset(s, 0, sizeof *s);
    s->n = (uint8_t)(can_dbc_msg_count < SL_MAX_MSGS ? can_dbc_msg_count : SL_MAX_MSGS);
}

void sl_on_obd(sl_t *s, uint8_t pid, float value, uint32_t now_ms)
{
    if (pid == 0x0Cu)      { s->ref_rpm = value; s->ref_rpm_ms = now_ms; s->have_rpm = true; }
    else if (pid == 0x0Du) { s->ref_kph = value; s->ref_kph_ms = now_ms; s->have_kph = true; }
}

static int index_of(const sl_t *s, uint32_t id)
{
    for (uint8_t i = 0u; i < s->n; i++) if (can_dbc_msgs[i].id == id) return i;
    return -1;
}

static void set_state(sl_t *s, uint8_t i, sl_state_t st)
{
    if (s->e[i].state != (uint8_t)st) { s->e[i].state = (uint8_t)st; s->dirty |= (1ull << i); }
}

/* Reference for a signal: rpm or speed in km/h (mph converted). */
static bool reference(const sl_t *s, const can_dbc_sig_t *g, uint32_t now, float *ref, float *conv, float *tol)
{
    const bool is_rpm = strstr(g->name, "RPM") != NULL && strcmp(g->unit, "rpm") == 0;
    const bool is_kph = strstr(g->name, "SPEED") != NULL && !strstr(g->name, "CRUISE") && !strstr(g->name, "LIMIT") &&
                        !strstr(g->name, "LEAD") && !strstr(g->name, "WIPER") && !strstr(g->name, "HUD") &&
                        (strcmp(g->unit, "kph") == 0 || strcmp(g->unit, "mph") == 0);
    if (is_rpm && s->have_rpm && (uint32_t)(now - s->ref_rpm_ms) <= SL_REF_MAX_AGE_MS && s->ref_rpm >= 500.0f) {
        *ref = s->ref_rpm; *conv = 1.0f; *tol = fmaxf(75.0f, 0.05f * s->ref_rpm); return true;
    }
    if (is_kph && s->have_kph && (uint32_t)(now - s->ref_kph_ms) <= SL_REF_MAX_AGE_MS && s->ref_kph >= 10.0f) {
        *ref = s->ref_kph; *conv = strcmp(g->unit, "mph") == 0 ? 1.609344f : 1.0f;
        *tol = fmaxf(4.0f, 0.08f * s->ref_kph); return true;
    }
    return false;
}

void sl_on_frame(sl_t *s, uint8_t bus, uint32_t id, bool ext, uint8_t dlc, const uint8_t *data, uint32_t now_ms)
{
    if (!s || bus != 0u || ext || !data) return;            /* the DBC describes F-CAN 11-bit frames */
    const int k = index_of(s, id);
    if (k < 0) return;
    const uint8_t i = (uint8_t)k;
    sl_entry_t *e = &s->e[i];
    const can_dbc_msg_t *m = &can_dbc_msgs[i];
    e->frames = sat_inc(e->frames);
    e->dlc_seen = dlc;
    if (dlc != m->dlc) { set_state(s, i, SL_CONFLICT); return; }  /* layout differs from the DBC */

    /* Every signal of the message that has an OBD reference must agree:
     * one wrong signal makes the whole definition suspect. */
    uint8_t compared = 0u, agreed = 0u;
    for (uint8_t g = 0u; g < m->sig_count; g++) {
        float ref, conv, tol;
        int64_t raw;
        if (!reference(s, &m->sigs[g], now_ms, &ref, &conv, &tol) || !can_dbc_raw(&m->sigs[g], data, dlc, &raw)) continue;
        const float v = (float)can_dbc_phys(&m->sigs[g], raw) * conv;
        compared++;
        if (fabsf(v - ref) <= tol) agreed++;
    }
    if (compared) { if (agreed == compared) e->match = sat_inc(e->match); else e->miss = sat_inc(e->miss); }
    if (e->match >= SL_VERIFY_MATCHES && e->match >= (uint16_t)(e->miss + SL_VERIFY_MATCHES)) set_state(s, i, SL_VERIFIED);
    else if (e->miss >= SL_CONFLICT_MISSES && e->miss > e->match) set_state(s, i, SL_CONFLICT);
    else if (e->state == SL_SEED && e->frames >= SL_CHECK_FRAMES) set_state(s, i, SL_CHECKED);
}

bool sl_take_change(sl_t *s, uint8_t *index)
{
    if (!s || !s->dirty) return false;
    for (uint8_t i = 0u; i < s->n; i++)
        if (s->dirty & (1ull << i)) { s->dirty &= ~(1ull << i); *index = i; return true; }
    s->dirty = 0u;
    return false;
}

const char *sl_state_name(sl_state_t st)
{
    switch (st) {
    case SL_SEED:     return "SEED";
    case SL_CHECKED:  return "CHECKED";
    case SL_VERIFIED: return "VERIFIED";
    case SL_CONFLICT: return "CONFLICT";
    default:          return "?";
    }
}

bool sl_replay(sl_t *s, uint8_t type, uint32_t id, const uint8_t *d, uint16_t len)
{
    if (type != SL_REC_STATE || len != sizeof(sl_entry_t) || !d) return false;
    const int k = index_of(s, id);
    if (k < 0) return false;
    sl_entry_t e; memcpy(&e, d, sizeof e);
    if (e.state > SL_CONFLICT) return false;
    s->e[k] = e;
    return true;
}

bool sl_persist(const sl_t *s, uint8_t index, sl_append_fn append)
{
    if (!s || !append || index >= s->n) return false;
    return append(SL_REC_STATE, can_dbc_msgs[index].id, &s->e[index], (uint16_t)sizeof(sl_entry_t));
}

bool sl_rewrite(const sl_t *s, sl_append_fn append)
{
    for (uint8_t i = 0u; i < s->n; i++)
        if (s->e[i].state != SL_SEED && !sl_persist(s, i, append)) return false;
    return true;
}

sl_t *sl_global(void)
{
#ifdef FS_HOST_TEST
    static sl_t g;
#else
    static sl_t g __attribute__((section(".ccmram")));
#endif
    return &g;
}
