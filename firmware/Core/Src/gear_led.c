/*
 * gear_led.c - transmission shift indication on the four MB997 user LEDs.
 * See gear_led.h for the model and contract. Pure logic, freestanding,
 * integer-only.
 */
#include "gear_led.h"
#include <stddef.h>

/* Quarter-wave sine, sin(i*pi/128) * 32767 for i = 0..64. */
static const int16_t SIN_Q[65] = {
        0,   804,  1608,  2410,  3212,  4011,  4808,  5602,
     6393,  7179,  7962,  8739,  9512, 10278, 11039, 11793,
    12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530,
    18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
    23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
    27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971,
    32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
    32767
};

/* sin(2*pi*phase/256) in Q15 for phase 0..255, by quarter-wave symmetry. */
static int32_t sin_q15(uint8_t phase)
{
    const uint8_t quadrant = (uint8_t)(phase >> 6);        /* 0..3 */
    const uint8_t i        = (uint8_t)(phase & 0x3Fu);     /* 0..63 */
    switch (quadrant) {
        case 0:  return  SIN_Q[i];
        case 1:  return  SIN_Q[64u - i];
        case 2:  return -SIN_Q[i];
        default: return -SIN_Q[64u - i];
    }
}

uint8_t gear_led_envelope(uint8_t phase, uint8_t harmonics)
{
    if (harmonics == 0u) harmonics = 1u;
    if (harmonics > GEAR_LED_MAX_HARMONICS) harmonics = (uint8_t)GEAR_LED_MAX_HARMONICS;

    /* Partial sum S = sum_{k odd <= 2H-1} sin(k*theta)/k, in Q15.
     * The square-wave series converges to +-pi/4 so |S| < ~0.93 for the H
     * used here (Gibbs overshoot ~1.18*pi/4 at the edges); scale by (2/pi)
     * then offset to 0..1: b = 1/2 + (2/pi)*S. */
    int32_t s = 0;
    for (uint32_t h = 0; h < harmonics; h++) {
        const uint32_t k = 2u * h + 1u;
        const uint8_t  p = (uint8_t)((k * (uint32_t)phase) & 0xFFu);
        s += sin_q15(p) / (int32_t)k;
    }
    /* (2/pi) = 0.63662 ~= 20861/32768 */
    int32_t b = 16384 + (int32_t)(((int64_t)s * 20861) >> 15);   /* Q15 0..1 */
    if (b < 0)      b = 0;
    if (b > 32767)  b = 32767;
    return (uint8_t)(b >> 7);                                   /* 0..255 */
}

void gear_led_init(gear_led_t *g)
{
    if (g == NULL) return;
    g->known = false; g->gear = 0u; g->active = false; g->up = true;
    g->harmonics = 1u; g->start_ms = 0u; g->shifts = 0u;
}

void gear_led_clear(gear_led_t *g)
{
    if (g == NULL) return;
    g->known = false; g->active = false;
}

gear_led_event_t gear_led_report(gear_led_t *g, uint8_t gear, uint32_t now_ms)
{
    if (g == NULL || gear > GEAR_LED_GEAR_MAX) return GEAR_LED_IGNORED;

    if (!g->known) {                      /* first value: learn, no animation */
        g->known = true; g->gear = gear;
        return GEAR_LED_IGNORED;
    }
    if (gear == g->gear) return GEAR_LED_IGNORED;

    const bool up = gear > g->gear;
    g->gear      = gear;
    g->up        = up;
    g->harmonics = (uint8_t)(gear == 0u ? 1u :
                             (gear > GEAR_LED_MAX_HARMONICS ? GEAR_LED_MAX_HARMONICS : gear));
    g->start_ms  = now_ms;
    g->active    = true;                  /* a new shift restarts the chase  */
    g->shifts++;
    return up ? GEAR_LED_UPSHIFT : GEAR_LED_DOWNSHIFT;
}

bool gear_led_active(gear_led_t *g, uint32_t now_ms)
{
    if (g == NULL || !g->active) return false;
    if ((uint32_t)(now_ms - g->start_ms) >= GEAR_LED_TOTAL_MS) g->active = false;
    return g->active;
}

uint8_t gear_led_mask(gear_led_t *g, uint32_t now_ms)
{
    if (!gear_led_active(g, now_ms)) return 0u;

    const uint32_t t    = (uint32_t)(now_ms - g->start_ms);
    const uint32_t step = t / GEAR_LED_ENVELOPE_MS;                 /* 0..3 */
    const uint32_t in   = t % GEAR_LED_ENVELOPE_MS;
    const uint8_t  led  = (uint8_t)(g->up ? step : (GEAR_LED_COUNT - 1u - step));
    const uint8_t  ph   = (uint8_t)((in * 256u) / GEAR_LED_ENVELOPE_MS);
    const uint8_t  duty = gear_led_envelope(ph, g->harmonics);

    /* Software PWM from the ms clock: on for the first duty/256 of each
     * GEAR_LED_PWM_PERIOD_MS window. */
    const uint32_t win = now_ms % GEAR_LED_PWM_PERIOD_MS;
    const uint32_t on  = ((uint32_t)duty * GEAR_LED_PWM_PERIOD_MS + 128u) / 256u;
    return (win < on) ? (uint8_t)(1u << led) : 0u;
}

const char *gear_led_event_name(gear_led_event_t e)
{
    switch (e) {
        case GEAR_LED_UPSHIFT:   return "UP";
        case GEAR_LED_DOWNSHIFT: return "DOWN";
        default:                 return "SAME";
    }
}
