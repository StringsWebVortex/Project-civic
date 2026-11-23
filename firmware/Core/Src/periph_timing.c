#include "periph_timing.h"

bool usart_brr_for(uint32_t pclk, uint32_t baud, uint32_t *brr, bool *over8)
{
    if (!baud) return false;
    uint32_t mant, frac;
    if (pclk >= 16u * baud) {
        const uint32_t div = (uint32_t)(((uint64_t)pclk * 25u) / (4u * (uint64_t)baud));
        mant = div / 100u;
        frac = (((div - mant * 100u) * 16u) + 50u) / 100u;
        *brr = (mant << 4) + (frac & 0xF0u) + (frac & 0x0Fu);
        *over8 = false;
    } else if (pclk >= 8u * baud) {
        const uint32_t div = (uint32_t)(((uint64_t)pclk * 25u) / (2u * (uint64_t)baud));
        mant = div / 100u;
        frac = (((div - mant * 100u) * 8u) + 50u) / 100u;
        *brr = (mant << 4) + ((frac & 0xF8u) << 1) + (frac & 0x07u);
        *over8 = true;
    } else {
        return false;
    }
    /* Verify the achieved rate is within 1 % */
    const uint32_t m = *brr >> 4;
    const uint32_t f = *over8 ? (*brr & 0x7u) : (*brr & 0xFu);
    const uint32_t scale = *over8 ? 8u : 16u;
    const uint64_t div_x = (uint64_t)m * scale + f;          /* USARTDIV * scale */
    if (div_x == 0u) return false;
    const uint64_t actual = (uint64_t)pclk / div_x;
    const uint64_t delta = actual > baud ? actual - baud : baud - actual;
    return delta * 100u <= baud;
}

/* Find bit timing: tq per bit 8..25, TS1 1..16, TS2 1..8, SJW 1.
 * Picks the exact (or <=0.5 % error) prescaler whose sample point is
 * closest to CAN_SAMPLE_POINT_PERMILLE. */
bool can_bit_timing(uint32_t pclk, uint32_t bitrate, uint32_t *btr, uint32_t *actual)
{
    if (!bitrate) return false;
    uint32_t best_err = 0xFFFFFFFFu, best_sp_diff = 0xFFFFFFFFu, best_btr = 0u, best_rate = 0u;
    for (uint32_t tq = 25u; tq >= 8u; tq--) {
        const uint64_t denom = (uint64_t)bitrate * tq;
        uint32_t presc = (uint32_t)(((uint64_t)pclk + denom / 2u) / denom);
        if (presc < 1u || presc > 1024u) continue;
        const uint32_t rate = pclk / (presc * tq);
        const uint32_t err = rate > bitrate ? rate - bitrate : bitrate - rate;
        if ((uint64_t)err * 1000u > (uint64_t)bitrate * 5u) continue;   /* > 0.5 % */
        uint32_t ts2 = (tq * (1000u - CAN_SAMPLE_POINT_PERMILLE) + 500u) / 1000u;
        if (ts2 < 2u) ts2 = 2u;
        if (ts2 > 8u) ts2 = 8u;
        const uint32_t ts1 = tq - 1u - ts2;
        if (ts1 < 1u || ts1 > 16u) continue;
        const uint32_t sp = ((1u + ts1) * 1000u) / tq;
        const uint32_t sp_diff = sp > CAN_SAMPLE_POINT_PERMILLE ?
                                 sp - CAN_SAMPLE_POINT_PERMILLE : CAN_SAMPLE_POINT_PERMILLE - sp;
        if (err < best_err || (err == best_err && sp_diff < best_sp_diff)) {
            best_err = err; best_sp_diff = sp_diff; best_rate = rate;
            best_btr = ((presc - 1u) & 0x3FFu) | ((ts1 - 1u) << 16) | ((ts2 - 1u) << 20);
        }
    }
    if (best_rate == 0u) return false;
    *btr = best_btr;           /* SJW field 0 -> 1 tq */
    *actual = best_rate;
    return true;
}

bool can_tx_bitrate_allowed(uint32_t bitrate)
{
    return bitrate >= TJA1051_MIN_TX_BITRATE;
}
