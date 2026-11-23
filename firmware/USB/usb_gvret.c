/* USB-FS transport for GVRET. USB callbacks only enqueue input; parsing,
 * CAN transmission, and USB IN submission stay in the main loop. */
#include "usb_gvret.h"

#include "stm32f407xx.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usb_tx_queue.h"

#include <string.h>

#define USB_RX_PACKET_MAX 64u
#define USB_RX_PACKET_SLOTS 4u

typedef struct {
    uint8_t length;
    uint8_t data[USB_RX_PACKET_MAX];
} usb_rx_packet_t;

typedef struct {
    usb_gvret_tx_fn transmit;
    const volatile uint32_t *bitrate[2];
} usb_bridge_context_t;

USBD_HandleTypeDef hUsbDeviceFS;

static gvret_bridge_t s_bridge;
static usb_bridge_context_t s_bridge_context;
static usb_rx_packet_t s_rx_packets[USB_RX_PACKET_SLOTS];
static volatile uint8_t s_rx_head;
static volatile uint8_t s_rx_tail;
static volatile uint8_t s_rx_reset_pending;
static volatile uint8_t s_tx_complete;
static volatile uint32_t s_rx_packet_drops;
static usb_tx_queue_t s_tx_queue;
static uint16_t s_tx_pending_length;
static uint32_t s_tx_queue_drops;
static uint32_t s_tx_transport_errors;
static bool s_tx_pending;
static bool s_initialized;

static uint8_t rx_next(uint8_t index)
{
    return (uint8_t)((index + 1u) % USB_RX_PACKET_SLOTS);
}

static bool queue_output(const uint8_t *data, size_t length)
{
    if (!usb_tx_queue_write(&s_tx_queue, data, length)) {
        s_tx_queue_drops++;
        return false;
    }
    return true;
}

static bool bridge_transmit(void *context, const struct gvret_frame *frame)
{
    usb_bridge_context_t *bridge_context = context;
    return bridge_context->transmit &&
           bridge_context->transmit(frame->bus, frame->id, frame->ext,
                                    frame->rtr, frame->data, frame->dlc);
}

static bool bridge_send(void *context, const uint8_t *data, size_t length)
{
    (void)context;
    return queue_output(data, length);
}

static uint32_t bridge_micros(void *context)
{
    (void)context;
    return TIM2->CNT;
}

static void reset_transport(void)
{
    if (s_tx_pending || usb_tx_queue_size(&s_tx_queue) != 0u)
        s_tx_transport_errors++;
    s_rx_head = 0u;
    s_rx_tail = 0u;
    usb_tx_queue_init(&s_tx_queue);
    s_tx_pending_length = 0u;
    s_tx_pending = false;
    s_tx_complete = 0u;
    s_rx_reset_pending = 0u;
    gvret_bridge_reset(&s_bridge);
}

static void poll_rx(void)
{
    uint8_t tail = s_rx_tail;
    uint8_t head = s_rx_head;
    while (tail != head) {
        const usb_rx_packet_t *packet = &s_rx_packets[tail];
        gvret_bridge_receive(&s_bridge, packet->data, packet->length);
        tail = rx_next(tail);
        s_rx_tail = tail;
        head = s_rx_head;
    }
}

static void poll_tx(void)
{
    if (s_tx_pending) {
        if (s_tx_complete == 0u) return;
        if (!usb_tx_queue_consume(&s_tx_queue, s_tx_pending_length)) {
            s_tx_transport_errors++;
            usb_tx_queue_init(&s_tx_queue);
        }
        s_tx_pending_length = 0u;
        s_tx_pending = false;
        s_tx_complete = 0u;
    }

    uint16_t length = 0u;
    uint8_t *data = usb_tx_queue_peek(&s_tx_queue, &length);
    if (!data) return;

    s_tx_pending_length = length;
    s_tx_pending = true;
    s_tx_complete = 0u;
    const uint8_t result = CDC_Transmit_FS(data, s_tx_pending_length);
    if (result == USBD_OK || result == USBD_BUSY) {
        if (result == USBD_BUSY) {
            s_tx_pending = false;
            s_tx_pending_length = 0u;
        }
        return;
    }

    s_tx_pending = false;
    if (!usb_tx_queue_consume(&s_tx_queue, s_tx_pending_length)) {
        usb_tx_queue_init(&s_tx_queue);
    }
    s_tx_pending_length = 0u;
    s_tx_transport_errors++;
}

bool usb_gvret_init(usb_gvret_tx_fn tx, const volatile uint32_t *bitrate0,
                    const volatile uint32_t *bitrate1)
{
    if (!tx || !bitrate0 || !bitrate1) return false;

    s_initialized = false;
    s_rx_head = 0u;
    s_rx_tail = 0u;
    s_rx_reset_pending = 0u;
    s_rx_packet_drops = 0u;
    usb_tx_queue_init(&s_tx_queue);
    s_tx_pending_length = 0u;
    s_tx_queue_drops = 0u;
    s_tx_transport_errors = 0u;
    s_tx_pending = false;
    s_tx_complete = 0u;

    s_bridge_context.transmit = tx;
    s_bridge_context.bitrate[0] = bitrate0;
    s_bridge_context.bitrate[1] = bitrate1;
    if (!gvret_bridge_init(&s_bridge, &s_bridge_context, bridge_transmit,
                           bridge_send, bridge_micros))
        return false;

    if (USBD_Init(&hUsbDeviceFS, &CDC_Desc, DEVICE_FS) != USBD_OK) return false;
    if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK) return false;
    if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
        return false;
    if (USBD_Start(&hUsbDeviceFS) != USBD_OK) return false;

    s_initialized = true;
    return true;
}

void usb_gvret_stream_frame(const can_capture_frame_t *frame)
{
    if (s_initialized) (void)gvret_bridge_stream_frame(&s_bridge, frame);
}

void usb_gvret_rx(const uint8_t *buf, uint32_t len)
{
    if (!buf || len == 0u) return;
    if (len > USB_RX_PACKET_MAX) {
        s_rx_packet_drops++;
        return;
    }

    const uint8_t head = s_rx_head;
    const uint8_t next = rx_next(head);
    if (next == s_rx_tail) {
        s_rx_packet_drops++;
        return;
    }

    usb_rx_packet_t *packet = &s_rx_packets[head];
    memcpy(packet->data, buf, len);
    packet->length = (uint8_t)len;
    __DMB();
    s_rx_head = next;
}

void usb_gvret_tx_complete(void)
{
    s_tx_complete = 1u;
}

void usb_gvret_on_disconnect(void)
{
    s_rx_reset_pending = 1u;
}

void usb_gvret_poll(void)
{
    if (!s_initialized) return;
    if (s_rx_reset_pending != 0u) reset_transport();

    gvret_bridge_set_bitrates(&s_bridge,
        s_bridge_context.bitrate[0] ? *s_bridge_context.bitrate[0] : 0u,
        s_bridge_context.bitrate[1] ? *s_bridge_context.bitrate[1] : 0u);
    poll_rx();
    poll_tx();
}

void usb_gvret_get_stats(usb_gvret_stats_t *stats)
{
    if (!stats) return;
    stats->rx_packet_drops = s_rx_packet_drops;
    stats->tx_queue_drops = s_tx_queue_drops;
    stats->tx_transport_errors = s_tx_transport_errors;
    stats->bridge = s_bridge.stats;
}
