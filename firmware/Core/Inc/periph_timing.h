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

/* Lowest bitrate the TJA1051 can transmit at: its TXD dominant time-out
 * (tto(dom)TXD min 0.3 ms) "defines the minimum possible bit rate of
 * 40 kbit/s" (NXP TJA1051 data sheet Rev. 02, Section 6.2.1 and Table 7,
 * docs/TJA1051.PDF). Below it the transceiver can cut a dominant phase in the
 * middle of a frame, so the firmware never leaves silent mode there.
 * Receiving at lower rates (silent scan) is not affected. */
#define TJA1051_MIN_TX_BITRATE 40000u
bool can_tx_bitrate_allowed(uint32_t bitrate);
#endif
