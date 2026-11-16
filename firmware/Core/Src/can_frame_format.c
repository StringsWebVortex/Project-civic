#include "can_frame_format.h"
#include <stdio.h>

/*
 * CSV format: timestamp_us,format,id,dlc,rtr,payload_hex,bus\n  (TIM2 1 us counter, wraps at 2^32)
 *
 * ID is emitted WITHOUT a "0x" prefix so it matches the format used by
 * can_signal_profiler.c (PROFILE/SIG lines also have no "0x"), allowing
 * the ESP32 dashboard to key both frame stats and profile data on the
 * same plain-hex string (e.g. "300", "7E8").
 *
 * The 7th field is the source bus (0 = CAN1/F-CAN, 1 = CAN2/B-CAN). It is
 * appended (not inserted) so a six-field reader of older captures still works;
 * the offline analyzer accepts both 6- and 7-field rows.
 *
 *   STD example: 12345,STD,7E8,8,0,0441050000000000,0\n
 *   EXT example: 67890,EXT,18DAF110,8,0,03410C3A00000000,0\n
 *   RTR example: 500,STD,7DF,8,1,,1\n
 */
size_t can_capture_frame_to_csv(const can_capture_frame_t *f, char *out, size_t capacity)
{
    if (!f || !out || !capacity || f->dlc > CAN_CAPTURE_MAX_DLC ||
        f->extended > 1u || f->remote > 1u ||
        (!f->extended && f->identifier > 0x7ffu) ||
        (f->extended  && f->identifier > 0x1fffffffu)) return 0u;

    /* ID: 3 upper-case hex digits for STD, 8 for EXT — no "0x" prefix */
    int n = snprintf(out, capacity, "%lu,%s,%0*lX,%u,%u,",
                     (unsigned long)f->timestamp_us,
                     f->extended ? "EXT" : "STD",
                     f->extended ? 8 : 3,
                     (unsigned long)f->identifier,
                     (unsigned)f->dlc,
                     (unsigned)f->remote);
    if (n < 0 || (size_t)n >= capacity) return 0u;
    size_t used = (size_t)n;

    const size_t payload_len = f->remote ? 0u : f->dlc;
    for (size_t i = 0u; i < payload_len; ++i) {
        if (used + 2u >= capacity) return 0u;
        n = snprintf(out + used, capacity - used, "%02X", (unsigned)f->data[i]);
        if (n != 2) return 0u;
        used += 2u;
    }
    n = snprintf(out + used, capacity - used, ",%u", (unsigned)(f->bus & 1u));
    if (n < 0 || (size_t)n >= capacity - used) return 0u;
    used += (size_t)n;
    if (used + 1u >= capacity) return 0u;
    out[used++] = '\n';
    out[used]   = '\0';
    return used;
}
