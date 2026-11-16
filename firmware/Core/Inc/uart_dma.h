#ifndef UART_DMA_H
#define UART_DMA_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef NET_DASHBOARD
/* No ESP32 reader on USART3 any more: a small ring keeps the line alive. */
#define UART_TX_RING_SIZE 1024u
#else
#define UART_TX_RING_SIZE 8192u
#endif

/* Initialise DMA1 Stream3 Channel4 for USART3 TX (call after USART3 init). */
void uart_dma_init(void);

/* Queue a whole line or nothing. Returns false (and counts a dropped line)
 * when the ring has no room for the complete string. */
bool uart_dma_put_line(const char *str);

/* Same as uart_dma_put_line, without the result (profiler callback type). */
void uart_dma_puts(const char *str);

uint32_t uart_dma_dropped_lines(void);
/* Bytes that can be queued right now without dropping a line. */
uint32_t uart_dma_free(void);
uint32_t uart_dma_errors(void);

void DMA1_Stream3_IRQHandler(void);
#endif
