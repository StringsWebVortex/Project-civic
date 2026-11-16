#ifndef PERIPH_TIMING_H
#define PERIPH_TIMING_H
#include <stdbool.h>
#include <stdint.h>

/* Target CAN sample point in 1/1000 of a bit. */
#define CAN_SAMPLE_POINT_PERMILLE 875u

/* USART BRR value for pclk/baud, using ST HAL formulas UART_BRR_SAMPLING16
 * (preferred) or UART_BRR_SAMPLING8 (when pclk < 16 x baud). Fails if the
 * achieved rate is off by more than 1 % or the divider is out of range. */
bool usart_brr_for(uint32_t pclk, uint32_t baud, uint32_t *brr, bool *over8);

/* bxCAN BTR timing fields (BRP, TS1, TS2; SJW = 1 tq) for pclk/bitrate.
 * Fails if no setting is within 0.5 %. */
bool can_bit_timing(uint32_t pclk, uint32_t bitrate, uint32_t *btr, uint32_t *actual);

/* Lowest bitrate the TJA1051 can transmit at. NXP TJA1051 data sheet Rev. 9,
 * 28-Nov-2017, Section 7.2.1 states the TXD dominant-timeout function defines
 * a 20 kbit/s minimum (the owner's specified TJA1051 marking with pin 5 NC; pin 5 NC).
 * Keep ARM blocked below the published device limit.
 * The older Rev. 02 PDF bundled in docs/TJA1051.PDF incorrectly/previously
 * reports 40 kbit/s; use the newer NXP Rev. 9 datasheet specification. */
#define TJA1051_MIN_TX_BITRATE 20000u
bool can_tx_bitrate_allowed(uint32_t bitrate);
#endif
