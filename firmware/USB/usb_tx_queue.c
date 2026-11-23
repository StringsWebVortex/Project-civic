#include "usb_tx_queue.h"

#include <string.h>

void usb_tx_queue_init(usb_tx_queue_t *queue)
{
    if (!queue) return;
    queue->head = 0u;
    queue->tail = 0u;
}

size_t usb_tx_queue_size(const usb_tx_queue_t *queue)
{
    if (!queue) return 0u;
    return (size_t)((queue->head + USB_TX_QUEUE_CAPACITY - queue->tail) %
                    USB_TX_QUEUE_CAPACITY);
}

bool usb_tx_queue_write(usb_tx_queue_t *queue, const uint8_t *data, size_t length)
{
    if (!queue || !data || length == 0u || length >= USB_TX_QUEUE_CAPACITY)
        return false;

    const size_t used = usb_tx_queue_size(queue);
    if (length > USB_TX_QUEUE_CAPACITY - used - 1u) return false;

    for (size_t i = 0u; i < length; i++) {
        queue->data[queue->head] = data[i];
        queue->head = (uint16_t)((queue->head + 1u) % USB_TX_QUEUE_CAPACITY);
    }
    return true;
}

uint8_t *usb_tx_queue_peek(usb_tx_queue_t *queue, uint16_t *length)
{
    if (!queue || !length) return NULL;
    const size_t used = usb_tx_queue_size(queue);
    if (used == 0u) {
        *length = 0u;
        return NULL;
    }

    size_t contiguous = USB_TX_QUEUE_CAPACITY - queue->tail;
    if (contiguous > used) contiguous = used;
    *length = (uint16_t)contiguous;
    return &queue->data[queue->tail];
}

bool usb_tx_queue_consume(usb_tx_queue_t *queue, uint16_t length)
{
    if (!queue || length == 0u || length > usb_tx_queue_size(queue)) return false;
    queue->tail = (uint16_t)((queue->tail + length) % USB_TX_QUEUE_CAPACITY);
    return true;
}
