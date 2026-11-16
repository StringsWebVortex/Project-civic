/* usb_gvret.h - bridge the USB-FS CDC virtual COM port to the GVRET codec so
 * SavvyCAN (and RealDash) can talk to the tool directly over USB.
 *
 * Built only when USB_CDC_ENABLED is defined (the `firmware-usb` image); the
 * default image does not include the USB stack. */
#ifndef USB_GVRET_H
#define USB_GVRET_H
#include <stdint.h>
#include <stdbool.h>
#include "can_frame_queue.h"
#include "gvret_bridge.h"

/* Transmit callback: the bridge calls this when the host asks to send a frame.
 * Implemented in main.c (honoured only while the target bus is ARMed). */
typedef bool (*usb_gvret_tx_fn)(uint8_t bus, uint32_t id, bool ext, bool rtr,
                                const uint8_t *data, uint8_t dlc);

typedef struct {
    uint32_t rx_packet_drops;
    uint32_t tx_queue_drops;
    uint32_t tx_transport_errors;
    gvret_bridge_stats_t bridge;
} usb_gvret_stats_t;

/* Bring up the USB device + CDC class and start it. Returns false on failure.
 * bitrate0/1 point at the live per-bus rates reported to the GVRET host. */
bool usb_gvret_init(usb_gvret_tx_fn tx, const volatile uint32_t *bitrate0,
                    const volatile uint32_t *bitrate1);

/* Stream one captured CAN frame to the host as GVRET (dropped if the CDC IN
 * endpoint is busy — a monitor tolerates gaps). */
void usb_gvret_stream_frame(const can_capture_frame_t *f);

/* Called from USB interrupt context; input is queued for main-loop processing. */
void usb_gvret_rx(const uint8_t *buf, uint32_t len);

/* Called by the CDC transfer-complete and disconnect callbacks. */
void usb_gvret_tx_complete(void);
void usb_gvret_on_disconnect(void);

/* Called from the main loop to process input and pump queued output. */
void usb_gvret_poll(void);
void usb_gvret_get_stats(usb_gvret_stats_t *stats);

#endif /* USB_GVRET_H */
