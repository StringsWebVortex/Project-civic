#ifndef GEAR_LED_H
#define GEAR_LED_H
/*
 * gear_led.h - transmission shift indication on the four MB997 user LEDs.
 *
 * Hardware reality (UM1472 / MB997 E-01 schematic): the STM32F4DISCOVERY has
 * four SEPARATE single-colour user LEDs on PD12..PD15 (LD4 green, LD3 orange,
 * LD5 red, LD6 blue). There is no 4-colour user LED; LD1 is the ST-LINK COM
 * LED and LD2 is the 3 V power LED, neither is MCU-controllable. The requested
 * "light -> dark colour sequence" is therefore mapped onto the four LEDs
 * ordered from the lightest to the darkest colour:
 *
 *     index 0 = GREEN (LD4)  lightest
 *     index 1 = ORANGE (LD3)
 *     index 2 = RED (LD5)
 *     index 3 = BLUE (LD6)   darkest
 *
 * An UPSHIFT (gear n -> n+1, low to high) chases index 0 -> 3 (light to dark);
 * a DOWNSHIFT (high to low) chases 3 -> 0 (dark to light). Each LED of the
 * chase is driven for one envelope period with a brightness envelope that is
 * the truncated Fourier series of a square wave,
 *
 *     b(t) = 1/2 + (2/pi) * sum_{k odd, k <= 2H-1} sin(k*w*t) / k
 *
 * with H = number of harmonics = the TARGET gear (1..GEAR_LED_MAX_HARMONICS).
 * Higher gears therefore show a "squarer" (harder-edged) pulse, lower gears a
 * softer, nearly sinusoidal one, so the pattern itself encodes the gear the
 * box moved into. Brightness is realised as software PWM from the millisecond
 * clock (GEAR_LED_PWM_PERIOD_MS period), which the caller samples through
 * gear_led_mask() each main-loop pass.
 *
 * PURE LOGIC: no I/O, no floating point (fixed-point sine table), no malloc.
 * The caller maps the returned 4-bit mask onto its GPIO pins and must restore
 * its normal LED behaviour when gear_led_active() returns false.
 *
 * Gear value semantics: the raw 8-bit value of a decoded gear DID/frame. The
 * caller decides what counts as a "gear"; values above GEAR_LED_GEAR_MAX are
 * refused (returned as GEAR_LED_IGNORED). A first report after init (no known
 * previous gear) or an unchanged report does not animate.
 */
#include <stdint.h>
#include <stdbool.h>

#define GEAR_LED_COUNT            4u
#define GEAR_LED_GEAR_MAX         15u   /* raw gear values accepted: 0..15 */
#define GEAR_LED_MAX_HARMONICS    5u    /* H clamp; gear 0 uses 1 harmonic   */
#define GEAR_LED_ENVELOPE_MS      160u  /* one Fourier-envelope period/LED   */
#define GEAR_LED_PWM_PERIOD_MS    16u   /* 62.5 Hz software PWM, 16 levels   */
#define GEAR_LED_TOTAL_MS         (GEAR_LED_ENVELOPE_MS * GEAR_LED_COUNT)

typedef enum {
    GEAR_LED_IGNORED = 0,   /* bad value, same gear, or no previous gear     */
    GEAR_LED_UPSHIFT,       /* animation started: light -> dark chase        */
    GEAR_LED_DOWNSHIFT      /* animation started: dark -> light chase        */
} gear_led_event_t;

typedef struct {
    bool     known;         /* a gear has been reported since init/clear     */
    uint8_t  gear;          /* last accepted gear                            */
    bool     active;        /* animation running                              */
    bool     up;            /* direction of the running animation            */
    uint8_t  harmonics;     /* H of the running animation (1..MAX)           */
    uint32_t start_ms;      /* animation start                                */
    uint32_t shifts;        /* accepted shift count (diagnostics)            */
} gear_led_t;

void             gear_led_init(gear_led_t *g);
/* Forget the known gear (e.g. key OFF) and stop any animation. */
void             gear_led_clear(gear_led_t *g);
/* Report a decoded gear. Starts an animation on an actual change. */
gear_led_event_t gear_led_report(gear_led_t *g, uint8_t gear, uint32_t now_ms);
/* true while an animation owns the four LEDs. Finishes itself by time. */
bool             gear_led_active(gear_led_t *g, uint32_t now_ms);
/* 4-bit LED mask (bit i = LED index i above) for this instant, 0 if idle. */
uint8_t          gear_led_mask(gear_led_t *g, uint32_t now_ms);
/* Fourier-envelope brightness 0..255 at phase 0..255 with H harmonics.
 * Exposed for tests. */
uint8_t          gear_led_envelope(uint8_t phase, uint8_t harmonics);
const char      *gear_led_event_name(gear_led_event_t e);

#endif /* GEAR_LED_H */
