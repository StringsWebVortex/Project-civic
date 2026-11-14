#ifndef UART_DMA_H
#define UART_DMA_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define UART_TX_RING_SIZE 8192u

/* Initialise DMA1 Stream6 Channel4 for USART2 TX (call after USART2 init). */
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

void DMA1_Stream6_IRQHandler(void);
#endif
