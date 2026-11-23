/*
 * test_gear_led.c - host tests for the gear-shift LED chase / Fourier envelope.
 * Simulated ms clock. Returns 0 on success; prints failures to stderr.
 */
#include <stdio.h>
#include <string.h>
#include "../Core/Inc/gear_led.h"

static int g_fails = 0;
#define CHECK(cond, msg)                                                    \
    do {                                                                    \
        if (!(cond)) {                                                      \
            fprintf(stderr, "FAIL [%s:%d]: %s\n", __func__, __LINE__, msg); \
            g_fails++;                                                      \
        }                                                                   \
    } while (0)

/* Which LED index is lit at time t (-1 = none, -2 = more than one). */
static int lit_led(gear_led_t *g, uint32_t t)
{
    uint8_t m = gear_led_mask(g, t);
    if (m == 0u) return -1;
    int idx = -1;
    for (int i = 0; i < 4; i++) if (m & (1u << i)) { if (idx >= 0) return -2; idx = i; }
    return idx;
}

/* Scan a whole envelope window and report the single LED index driven in it
 * (the PWM is off part of the time, so look for any lit sample). */
static int led_in_window(gear_led_t *g, uint32_t start, uint32_t ms)
{
    int seen = -1;
    for (uint32_t t = start; t < start + ms; t++) {
        int l = lit_led(g, t);
        if (l == -2) return -2;
        if (l >= 0) { if (seen >= 0 && seen != l) return -2; seen = l; }
    }
    return seen;
}

static void test_first_report_learns_only(void)
{
    gear_led_t g; gear_led_init(&g);
    CHECK(gear_led_report(&g, 1u, 0u) == GEAR_LED_IGNORED, "first report must not animate");
    CHECK(!gear_led_active(&g, 1u), "idle after first report");
    CHECK(gear_led_report(&g, 1u, 10u) == GEAR_LED_IGNORED, "same gear ignored");
    CHECK(gear_led_mask(&g, 10u) == 0u, "mask 0 while idle");
}

static void test_upshift_chase_light_to_dark(void)
{
    gear_led_t g; gear_led_init(&g);
    (void)gear_led_report(&g, 1u, 0u);
    CHECK(gear_led_report(&g, 2u, 1000u) == GEAR_LED_UPSHIFT, "1->2 is an upshift");
    CHECK(g.harmonics == 2u, "gear 2 -> 2 harmonics");
    CHECK(gear_led_active(&g, 1000u), "active right after shift");
    for (uint32_t i = 0; i < GEAR_LED_COUNT; i++) {
        int l = led_in_window(&g, 1000u + i * GEAR_LED_ENVELOPE_MS, GEAR_LED_ENVELOPE_MS);
        CHECK(l == (int)i, "upshift chases index 0 (green) -> 3 (blue)");
    }
    CHECK(!gear_led_active(&g, 1000u + GEAR_LED_TOTAL_MS), "finished after TOTAL_MS");
    CHECK(gear_led_mask(&g, 1000u + GEAR_LED_TOTAL_MS + 5u) == 0u, "mask 0 after finish");
    CHECK(g.shifts == 1u, "one shift counted");
}

static void test_downshift_chase_dark_to_light(void)
{
    gear_led_t g; gear_led_init(&g);
    (void)gear_led_report(&g, 4u, 0u);
    CHECK(gear_led_report(&g, 3u, 500u) == GEAR_LED_DOWNSHIFT, "4->3 is a downshift");
    for (uint32_t i = 0; i < GEAR_LED_COUNT; i++) {
        int l = led_in_window(&g, 500u + i * GEAR_LED_ENVELOPE_MS, GEAR_LED_ENVELOPE_MS);
        CHECK(l == (int)(GEAR_LED_COUNT - 1u - i), "downshift chases 3 (blue) -> 0 (green)");
    }
}

static void test_never_two_leds_at_once(void)
{
    gear_led_t g; gear_led_init(&g);
    (void)gear_led_report(&g, 2u, 0u);
    (void)gear_led_report(&g, 5u, 100u);
    for (uint32_t t = 100u; t < 100u + GEAR_LED_TOTAL_MS; t++)
        CHECK(lit_led(&g, t) != -2, "at most one LED lit per instant");
}

static void test_new_shift_restarts(void)
{
    gear_led_t g; gear_led_init(&g);
    (void)gear_led_report(&g, 1u, 0u);
    (void)gear_led_report(&g, 2u, 0u);
    CHECK(gear_led_report(&g, 3u, 200u) == GEAR_LED_UPSHIFT, "shift during animation accepted");
    CHECK(g.start_ms == 200u, "animation restarted at the new shift");
    CHECK(led_in_window(&g, 200u, GEAR_LED_ENVELOPE_MS) == 0, "restart begins at index 0");
}

static void test_bad_values_and_clear(void)
{
    gear_led_t g; gear_led_init(&g);
    (void)gear_led_report(&g, 1u, 0u);
    CHECK(gear_led_report(&g, 200u, 10u) == GEAR_LED_IGNORED, "value > 15 refused");
    CHECK(g.gear == 1u, "refused value does not change known gear");
    CHECK(gear_led_report(&g, 2u, 20u) == GEAR_LED_UPSHIFT, "valid shift after refusal");
    gear_led_clear(&g);
    CHECK(!gear_led_active(&g, 30u), "clear stops animation");
    CHECK(gear_led_report(&g, 5u, 40u) == GEAR_LED_IGNORED, "after clear the next value only learns");
    CHECK(gear_led_report(&g, 5u, 40u) == GEAR_LED_IGNORED, "NULL-safe / same gear");
    CHECK(gear_led_report(NULL, 1u, 0u) == GEAR_LED_IGNORED, "NULL object ignored");
    CHECK(gear_led_mask(NULL, 0u) == 0u, "NULL mask is 0");
}

static void test_harmonic_clamp(void)
{
    gear_led_t g; gear_led_init(&g);
    (void)gear_led_report(&g, 3u, 0u);
    (void)gear_led_report(&g, 0u, 1u);  CHECK(g.harmonics == 1u, "gear 0 -> 1 harmonic");
    (void)gear_led_report(&g, 9u, 2u);  CHECK(g.harmonics == GEAR_LED_MAX_HARMONICS, "gear 9 clamps to max harmonics");
}

/* Fourier envelope: 1 harmonic is a plain raised sine (mid at phase 0, max
 * near 64, min near 192); more harmonics flatten the top (approach a square
 * wave) and keep the sum within 0..255 everywhere. */
static void test_envelope_shape(void)
{
    uint8_t e0 = gear_led_envelope(0u, 1u), e64 = gear_led_envelope(64u, 1u),
            e192 = gear_led_envelope(192u, 1u);
    CHECK(e0 >= 120u && e0 <= 136u, "1 harmonic: phase 0 is mid-scale");
    CHECK(e64 > 230u, "1 harmonic: phase 64 near max");
    CHECK(e192 < 25u, "1 harmonic: phase 192 near min");
    /* With H harmonics the plateau is flatter: value at phase 32 (1/8 cycle)
     * must be closer to the peak than with 1 harmonic. */
    CHECK(gear_led_envelope(32u, 5u) > gear_led_envelope(32u, 1u), "more harmonics -> squarer edge");
    /* Half-wave symmetry: b(p) + b(p+128) == 1 (within rounding). */
    for (uint32_t h = 1; h <= GEAR_LED_MAX_HARMONICS; h++)
        for (uint32_t p = 0; p < 256u; p++) {
            int sum = (int)gear_led_envelope((uint8_t)p, (uint8_t)h)
                    + (int)gear_led_envelope((uint8_t)(p + 128u), (uint8_t)h);
            CHECK(sum >= 250 && sum <= 258, "half-wave symmetric envelope");
        }
    CHECK(gear_led_envelope(64u, 0u) == gear_led_envelope(64u, 1u), "0 harmonics treated as 1");
}

/* PWM: the mean lit fraction over one envelope period must track the mean
 * envelope (1/2), i.e. the LED is really being dimmed, not just toggled. */
static void test_pwm_average(void)
{
    gear_led_t g; gear_led_init(&g);
    (void)gear_led_report(&g, 1u, 0u);
    (void)gear_led_report(&g, 2u, 0u);
    uint32_t on = 0;
    for (uint32_t t = 0; t < GEAR_LED_ENVELOPE_MS; t++) if (gear_led_mask(&g, t)) on++;
    CHECK(on > GEAR_LED_ENVELOPE_MS * 35u / 100u && on < GEAR_LED_ENVELOPE_MS * 65u / 100u,
          "mean duty over one envelope period is about 50%");
}

static void test_names(void)
{
    CHECK(strcmp(gear_led_event_name(GEAR_LED_UPSHIFT), "UP") == 0, "UP name");
    CHECK(strcmp(gear_led_event_name(GEAR_LED_DOWNSHIFT), "DOWN") == 0, "DOWN name");
    CHECK(strcmp(gear_led_event_name(GEAR_LED_IGNORED), "SAME") == 0, "SAME name");
}

int main(void)
{
    test_first_report_learns_only();
    test_upshift_chase_light_to_dark();
    test_downshift_chase_dark_to_light();
    test_never_two_leds_at_once();
    test_new_shift_restarts();
    test_bad_values_and_clear();
    test_harmonic_clamp();
    test_envelope_shape();
    test_pwm_average();
    test_names();
    if (g_fails != 0) {
        fprintf(stderr, "GEAR_LED TESTS FAILED: %d check(s)\n", g_fails);
        return 1;
    }
    printf("ALL GEAR_LED TESTS PASS\n");
    return 0;
}
