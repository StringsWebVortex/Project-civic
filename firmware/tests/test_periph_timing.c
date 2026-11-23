/* Host tests for USART BRR and bxCAN bit-timing math (periph_timing.c). */
#include <assert.h>
#include <stdio.h>
#include "periph_timing.h"

static void check_can(uint32_t pclk, uint32_t rate)
{
    uint32_t btr = 0u, actual = 0u;
    assert(can_bit_timing(pclk, rate, &btr, &actual));
    const uint32_t brp = (btr & 0x3FFu) + 1u;
    const uint32_t ts1 = ((btr >> 16) & 0xFu) + 1u;
    const uint32_t ts2 = ((btr >> 20) & 0x7u) + 1u;
    const uint32_t sjw = ((btr >> 24) & 0x3u) + 1u;
    const uint32_t tq = 1u + ts1 + ts2;
    assert(tq >= 8u && tq <= 25u && ts2 >= 2u && sjw == 1u);
    assert(pclk / (brp * tq) == actual);
    const uint32_t err = actual > rate ? actual - rate : rate - actual;
    assert((uint64_t)err * 1000u <= (uint64_t)rate * 5u);
    const uint32_t sp = (1u + ts1) * 1000u / tq;
    assert(sp >= 800u && sp <= 900u);
    printf("  pclk %lu rate %7lu -> BRP %4lu TS1 %2lu TS2 %lu tq %2lu SP %lu.%lu%% actual %lu\n",
           (unsigned long)pclk, (unsigned long)rate, (unsigned long)brp, (unsigned long)ts1,
           (unsigned long)ts2, (unsigned long)tq, (unsigned long)(sp / 10u),
           (unsigned long)(sp % 10u), (unsigned long)actual);
}

int main(void)
{
    uint32_t brr; bool over8;
    /* 84 MHz PCLK2 (168 MHz PLL, APB2 /2): 2 Mbaud -> USARTDIV 2.625 -> 0x2A */
    assert(usart_brr_for(84000000u, 2000000u, &brr, &over8) && brr == 0x2Au && !over8);
    /* 84 MHz, 5.25 Mbaud -> USARTDIV 1.0 (OVER16) -> 0x10 */
    assert(usart_brr_for(84000000u, 5250000u, &brr, &over8) && brr == 0x10u && !over8);
    /* 16 MHz HSI fallback, 2 Mbaud -> needs OVER8, USARTDIV 1.0 -> 0x10 */
    assert(usart_brr_for(16000000u, 2000000u, &brr, &over8) && brr == 0x10u && over8);
    /* 16 MHz cannot do 5.25 Mbaud at all (old firmware wrote BRR=3) */
    assert(!usart_brr_for(16000000u, 5250000u, &brr, &over8));
    assert(usart_brr_for(84000000u, 115200u, &brr, &over8) && brr == 0x2D9u);

    static const uint32_t rates[] = {1000000u, 500000u, 250000u, 125000u, 100000u,
                                     83333u, 50000u, 33333u, 20000u, 10000u};
    for (unsigned i = 0; i < sizeof rates / sizeof rates[0]; i++) check_can(42000000u, rates[i]);
    for (unsigned i = 0; i < sizeof rates / sizeof rates[0]; i++) check_can(16000000u, rates[i]);
    assert(!can_bit_timing(42000000u, 0u, &brr, &brr));
    /* TJA1051 Rev. 9 TXD dominant time-out: minimum transmit rate 20 kbit/s */
    assert(can_tx_bitrate_allowed(500000u) && can_tx_bitrate_allowed(125000u) && can_tx_bitrate_allowed(33333u) && can_tx_bitrate_allowed(20000u));
    assert(!can_tx_bitrate_allowed(19999u) && !can_tx_bitrate_allowed(10000u) && !can_tx_bitrate_allowed(0u));
    puts("PASS: periph_timing - USART BRR (OVER16/OVER8) and CAN bit timing at 42 MHz and 16 MHz");
    return 0;
}
