/* Online per-identifier MLP for CAN payloads. See frame_nn.h.
 *
 * Math is plain float: the STM32F407 has a single-precision FPU. No libm calls
 * (softsign replaces tanh, output is linear), so the module links with the
 * firmware's existing float support and nothing else. */
#include "frame_nn.h"
#include <string.h>

/* CPU-only state (no DMA): kept in the 64 KB CCM RAM, as the other CPU-only modules are. */
static fnn_t g_nn __attribute__((section(".ccmram")));
static bool  g_nn_ready;

fnn_t *fnn_global(void)
{
    if (!g_nn_ready) { fnn_init(&g_nn); g_nn_ready = true; }
    return &g_nn;
}

/* Deterministic small-weight init (LCG), so every boot starts identically. */
static uint32_t lcg_next(uint32_t *s) { *s = *s * 1664525u + 1013904223u; return *s; }

void fnn_init(fnn_t *n)
{
    memset(n, 0, sizeof *n);
    uint32_t seed = 0x2F6E2B1u;
    for (unsigned i = 0; i < FNN_HID; i++)
        for (unsigned j = 0; j < FNN_IN; j++)
            n->w1[i][j] = ((float)(lcg_next(&seed) >> 8) / 16777216.0f - 0.5f) * 0.4f;
    for (unsigned i = 0; i < FNN_OUT; i++)
        for (unsigned j = 0; j < FNN_HID; j++)
            n->w2[i][j] = ((float)(lcg_next(&seed) >> 8) / 16777216.0f - 0.5f) * 0.4f;
}

static fnn_slot_t *slot_find(fnn_t *n, uint32_t id, bool ext, bool create)
{
    fnn_slot_t *free_s = NULL;
    for (unsigned i = 0; i < FNN_SLOTS; i++) {
        fnn_slot_t *s = &n->slot[i];
        if (s->used && s->id == id && s->ext == ext) return s;
        if (!s->used && !free_s) free_s = s;
    }
    if (!create || !free_s) return NULL;
    memset(free_s, 0, sizeof *free_s);
    free_s->used = true; free_s->id = id; free_s->ext = ext;
    return free_s;
}

static const fnn_slot_t *slot_find_const(const fnn_t *n, uint32_t id, bool ext)
{
    for (unsigned i = 0; i < FNN_SLOTS; i++) {
        const fnn_slot_t *s = &n->slot[i];
        if (s->used && s->id == id && s->ext == ext) return s;
    }
    return NULL;
}

/* Softsign: x / (1 + |x|). Bounded, smooth, no libm. */
static float absf(float v) { return v < 0.0f ? -v : v; }
static float softsign(float x) { return x / (1.0f + absf(x)); }
/* Derivative of softsign expressed through its output y: (1 - |y|)^2. */
static float softsign_d(float y) { const float a = absf(y); return (1.0f - a) * (1.0f - a); }

/* Forward pass. h[] receives hidden activations, out[] the predicted bytes
 * (linear). Input is the previous payload scaled to [0,1] plus a bias. */
static void forward(const fnn_t *n, const uint8_t prev[8], float h[FNN_HID], float out[FNN_OUT])
{
    float x[FNN_IN];
    for (unsigned j = 0; j < 8; j++) x[j] = (float)prev[j] / 255.0f;
    x[8] = 1.0f;
    for (unsigned i = 0; i < FNN_HID; i++) {
        float a = 0.0f;
        for (unsigned j = 0; j < FNN_IN; j++) a += n->w1[i][j] * x[j];
        h[i] = softsign(a);
    }
    for (unsigned i = 0; i < FNN_OUT; i++) {
        float a = n->b2[i];
        for (unsigned j = 0; j < FNN_HID; j++) a += n->w2[i][j] * h[j];
        out[i] = a;
    }
}

static void payload_norm(uint8_t dst[8], uint8_t dlc, const uint8_t d[8])
{
    memset(dst, 0, 8);
    if (dlc > 8) dlc = 8;
    memcpy(dst, d, dlc);
}

float fnn_score(const fnn_t *n, uint32_t id, bool ext, uint8_t dlc, const uint8_t d[8])
{
    const fnn_slot_t *s = slot_find_const(n, id, ext);
    if (!s || !s->have_prev || dlc == 0u || dlc > 8u) return -1.0f;
    float h[FNN_HID], out[FNN_OUT];
    forward(n, s->prev, h, out);
    uint8_t cur[8]; payload_norm(cur, dlc, d);
    float err = 0.0f;
    for (unsigned j = 0; j < dlc; j++) {
        float p = out[j] * 255.0f;
        if (p < 0.0f) p = 0.0f;
        if (p > 255.0f) p = 255.0f;
        err += absf(p - (float)cur[j]) / 255.0f;
    }
    return err / (float)dlc;
}

float fnn_on_frame(fnn_t *n, uint32_t id, bool ext, uint8_t dlc, const uint8_t d[8])
{
    if (dlc == 0u || dlc > 8u) return -1.0f;
    fnn_slot_t *s = slot_find(n, id, ext, true);
    if (!s) return -1.0f;                       /* table full: frame ignored */

    float score = -1.0f;
    if (s->have_prev) {
        score = fnn_score(n, id, ext, dlc, d);
        /* One SGD step: predict this payload from the previous payload. */
        float h[FNN_HID], out[FNN_OUT];
        forward(n, s->prev, h, out);
        uint8_t cur[8]; payload_norm(cur, dlc, d);
        float dout[FNN_OUT];
        for (unsigned i = 0; i < FNN_OUT; i++) {
            if (i >= dlc) { dout[i] = 0.0f; continue; }
            dout[i] = out[i] - (float)cur[i] / 255.0f;
        }
        float dh[FNN_HID];
        for (unsigned j = 0; j < FNN_HID; j++) {
            float a = 0.0f;
            for (unsigned i = 0; i < FNN_OUT; i++) a += n->w2[i][j] * dout[i];
            dh[j] = a * softsign_d(h[j]);
        }
        for (unsigned i = 0; i < FNN_OUT; i++) {
            n->b2[i] -= FNN_LR * dout[i];
            for (unsigned j = 0; j < FNN_HID; j++) n->w2[i][j] -= FNN_LR * dout[i] * h[j];
        }
        float x[FNN_IN];
        for (unsigned j = 0; j < 8; j++) x[j] = (float)s->prev[j] / 255.0f;
        x[8] = 1.0f;
        for (unsigned i = 0; i < FNN_HID; i++)
            for (unsigned j = 0; j < FNN_IN; j++) n->w1[i][j] -= FNN_LR * dh[i] * x[j];

        n->ema_err = n->frames == 0u ? score : n->ema_err * 0.99f + score * 0.01f;
        if (n->frames < 0xFFFFFFFFu) n->frames++;
    }

    payload_norm(s->prev, dlc, d);
    s->dlc = dlc;
    s->have_prev = true;
    return score;
}
