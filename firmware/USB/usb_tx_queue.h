#ifndef USB_TX_QUEUE_H
#define USB_TX_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define USB_TX_QUEUE_CAPACITY 1024u

typedef struct {
    uint8_t data[USB_TX_QUEUE_CAPACITY];
    uint16_t head;
    uint16_t tail;
} usb_tx_queue_t;

void usb_tx_queue_init(usb_tx_queue_t *queue);
bool usb_tx_queue_write(usb_tx_queue_t *queue, const uint8_t *data, size_t length);
uint8_t *usb_tx_queue_peek(usb_tx_queue_t *queue, uint16_t *length);
bool usb_tx_queue_consume(usb_tx_queue_t *queue, uint16_t length);
size_t usb_tx_queue_size(const usb_tx_queue_t *queue);

#endif
