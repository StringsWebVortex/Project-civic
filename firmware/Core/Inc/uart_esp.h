#ifndef UART_ESP_H
#define UART_ESP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* USART2 PA2/PA3 ↔ ESP32 GPIO16/GPIO17, 8N1 at 2 Mbaud. */
bool uart_esp_init(void);

/* Main-loop producers only. TX is queued atomically and drained by DMA. */
void uart_esp_emit(const char *s);
bool uart_esp_emit_line(const char *s, size_t n);

/* Main-loop RX: returns the next complete line or NULL when none is ready. */
const char *uart_esp_rx_line(void);

/* Monotonic diagnostic counters; reading them does not clear them. */
uint32_t uart_esp_tx_dropped(void);
uint32_t uart_esp_tx_dma_errors(void);
uint32_t uart_esp_tx_dma_transfer_errors(void);
uint32_t uart_esp_tx_dma_direct_errors(void);
uint32_t uart_esp_tx_dma_fifo_errors(void);
uint32_t uart_esp_tx_dma_last_flags(void);
uint32_t uart_esp_rx_dropped(void);

#endif
