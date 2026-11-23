#include "uart_tx_queue.h"

static bool write_parts(uart_tx_queue_t *queue,
                        const uint8_t *first, size_t first_length,
                        const uint8_t *second, size_t second_length)
{
    if (!queue || (first_length && !first) || (second_length && !second)) return false;
    if (first_length == 0u && second_length == 0u) return false;
    if (first_length >= UART_TX_QUEUE_CAPACITY ||
        second_length >= UART_TX_QUEUE_CAPACITY ||
        first_length + second_length >= UART_TX_QUEUE_CAPACITY) return false;

    const uint16_t head = queue->head;
    const uint16_t tail = queue->tail;
    const size_t used = head >= tail
        ? (size_t)(head - tail)
        : (size_t)(UART_TX_QUEUE_CAPACITY - tail + head);
    const size_t total = first_length + second_length;
    if (total > UART_TX_QUEUE_CAPACITY - used - 1u) return false;

    uint16_t next = head;
    for (size_t i = 0u; i < first_length; ++i) {
        queue->data[next] = first[i];
        next = (uint16_t)((next + 1u) % UART_TX_QUEUE_CAPACITY);
    }
    for (size_t i = 0u; i < second_length; ++i) {
        queue->data[next] = second[i];
        next = (uint16_t)((next + 1u) % UART_TX_QUEUE_CAPACITY);
    }

    /* Single 16-bit commit: the DMA ISR never observes a partial line. */
    queue->head = next;
    return true;
}

void uart_tx_queue_init(uart_tx_queue_t *queue)
{
    if (!queue) return;
    queue->head = 0u;
    queue->tail = 0u;
}

size_t uart_tx_queue_size(const uart_tx_queue_t *queue)
{
    if (!queue) return 0u;
    const uint16_t head = queue->head;
    const uint16_t tail = queue->tail;
    return head >= tail
        ? (size_t)(head - tail)
        : (size_t)(UART_TX_QUEUE_CAPACITY - tail + head);
}

bool uart_tx_queue_write(uart_tx_queue_t *queue, const uint8_t *data, size_t length)
{
    return write_parts(queue, data, length, NULL, 0u);
}

bool uart_tx_queue_write_line(uart_tx_queue_t *queue, const uint8_t *data, size_t length)
{
    static const uint8_t newline = '\n';
    if (length == 0u) return false;
    return write_parts(queue, data, length, &newline, 1u);
}

uint8_t *uart_tx_queue_peek(uart_tx_queue_t *queue, uint16_t *length)
{
    if (!queue || !length) return NULL;
    const size_t used = uart_tx_queue_size(queue);
    if (used == 0u) {
        *length = 0u;
        return NULL;
    }

    size_t contiguous = UART_TX_QUEUE_CAPACITY - queue->tail;
    if (contiguous > used) contiguous = used;
    *length = (uint16_t)contiguous;
    return &queue->data[queue->tail];
}

bool uart_tx_queue_consume(uart_tx_queue_t *queue, uint16_t length)
{
    if (!queue || length == 0u || length > uart_tx_queue_size(queue)) return false;
    queue->tail = (uint16_t)((queue->tail + length) % UART_TX_QUEUE_CAPACITY);
    return true;
}
