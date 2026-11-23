#ifndef UART_TX_QUEUE_H
#define UART_TX_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* One maximum dashboard JSON message plus framing, with bounded spare space. */
#define UART_TX_QUEUE_CAPACITY 2048u

typedef struct {
    uint8_t data[UART_TX_QUEUE_CAPACITY];
    volatile uint16_t head; /* producer commits once, after copying a whole message */
    volatile uint16_t tail; /* DMA completion ISR advances this */
} uart_tx_queue_t;

void uart_tx_queue_init(uart_tx_queue_t *queue);
size_t uart_tx_queue_size(const uart_tx_queue_t *queue);
bool uart_tx_queue_write(uart_tx_queue_t *queue, const uint8_t *data, size_t length);
bool uart_tx_queue_write_line(uart_tx_queue_t *queue, const uint8_t *data, size_t length);
uint8_t *uart_tx_queue_peek(uart_tx_queue_t *queue, uint16_t *length);
bool uart_tx_queue_consume(uart_tx_queue_t *queue, uint16_t length);

#endif
