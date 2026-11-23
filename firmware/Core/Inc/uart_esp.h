/*
 * uart_esp.h — USART2 output to ESP32 Wi-Fi bridge
 *
 * PA2 = USART2_TX (AF7, open-drain, external 3.3 V pull-up — 3.3 V logic,
 *                  no level shift needed: ESP32 GPIO16 is 3.3 V).
 * PA3 = USART2_RX (AF7, input, no pull — not used yet; reserved for
 *                  future command input from ESP).
 *
 * Baud: PROJECT_HOST_UART_BAUD (default 2 000 000).
 * APB1 clock: 42 MHz → BRR = 42 000 000 / 2 000 000 = 21 (exact).
 *
 * Transmit: polled (no DMA, no interrupt) — called only from main-loop
 * context, same as emit().  Each line is at most 80 bytes; at 2 Mbaud
 * that is 320 µs worst case, well within the 1 ms main-loop budget.
 *
 * Thread/ISR safety: uart_esp_emit() must NOT be called from ISR context.
 * CAN frame output comes from the main loop, so this is satisfied.
 */

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * uart_esp_init() — call once after clock_init_168mhz(), before main loop.
 * Returns true on success.
 */
bool uart_esp_init(void);

/*
 * uart_esp_emit() — send a NUL-terminated string over USART2.
 * Safe to call with s == NULL or empty string (no-op).
 * Uses DMA TX — returns immediately after queueing (non-blocking).
 * Falls back to polled TX if DMA is busy.
 */
void uart_esp_emit(const char *s);

/*
 * uart_esp_emit_line() — send 'n' bytes of 's' plus a trailing '\n' in
 * one DMA transfer.  Used by ws_broadcast_txt() to mirror WS JSON data
 * to the ESP UART bridge so WiFi-connected dashboards receive it.
 */
void uart_esp_emit_line(const char *s, size_t n);

/*
 * uart_esp_rx_line() — return next complete line from ESP32 (USART2 RX).
 * Returns pointer to internal static buffer, or NULL if no complete line.
 * Call from main loop only. Lines are assembled from ISR ring buffer.
 * The returned pointer is valid until the next call.
 */
const char *uart_esp_rx_line(void);
