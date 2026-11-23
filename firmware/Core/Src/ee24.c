/* 24LC512 page writes and reads (see ee24.h). */
#include "ee24.h"

bool ee24_present(const ee24_t *e)
{
    return e && e->write && e->write(e->addr7, NULL, 0u, NULL, 0u);
}

bool ee24_read(const ee24_t *e, uint16_t mem, uint8_t *buf, size_t n)
{
    if (!e || !e->read || !buf || n == 0u || (size_t)mem + n > EE24_SIZE) return false;
    const uint8_t a[2] = { (uint8_t)(mem >> 8), (uint8_t)mem };
    return e->read(e->addr7, a, 2u, buf, n);                 /* sequential read (DS 8.3) */
}

/* DS 7.0: after the stop the device is busy for up to TWC and does not
 * acknowledge its address; poll until it does. */
static bool ack_poll(const ee24_t *e)
{
    const uint32_t t0 = e->now_ms();
    do {
        if (e->write(e->addr7, NULL, 0u, NULL, 0u)) return true;
    } while ((uint32_t)(e->now_ms() - t0) <= EE24_TWC_MS + 5u);
    return false;
}

bool ee24_write(const ee24_t *e, uint16_t mem, const uint8_t *buf, size_t n)
{
    if (!e || !e->write || !e->now_ms || !buf || (size_t)mem + n > EE24_SIZE) return false;
    size_t done = 0u;
    while (done < n) {
        const uint32_t at = (uint32_t)mem + done;
        size_t chunk = EE24_PAGE - (at % EE24_PAGE);          /* never cross a page */
        if (chunk > n - done) chunk = n - done;
        const uint8_t a[2] = { (uint8_t)(at >> 8), (uint8_t)at };
        if (!e->write(e->addr7, a, 2u, buf + done, chunk) || !ack_poll(e)) return false;
        done += chunk;
    }
    return true;
}
