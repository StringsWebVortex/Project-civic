#ifndef GVRET_BRIDGE_H
#define GVRET_BRIDGE_H

#include "can_frame_queue.h"
#include "gvret.h"

typedef bool (*gvret_bridge_tx_fn)(void *context, const struct gvret_frame *frame);
typedef bool (*gvret_bridge_send_fn)(void *context, const uint8_t *data, size_t length);
typedef uint32_t (*gvret_bridge_micros_fn)(void *context);

typedef struct {
    uint32_t output_drops;
    uint32_t rejected_frames;
    uint32_t ignored_setup_commands;
} gvret_bridge_stats_t;

typedef struct {
    struct gvret_parser parser;
    void *context;
    gvret_bridge_tx_fn transmit;
    gvret_bridge_send_fn send;
    gvret_bridge_micros_fn micros;
    uint32_t bitrate[2];
    gvret_bridge_stats_t stats;
} gvret_bridge_t;

bool gvret_bridge_init(gvret_bridge_t *bridge, void *context,
                       gvret_bridge_tx_fn transmit, gvret_bridge_send_fn send,
                       gvret_bridge_micros_fn micros);
void gvret_bridge_reset(gvret_bridge_t *bridge);
void gvret_bridge_set_bitrates(gvret_bridge_t *bridge, uint32_t bitrate0,
                               uint32_t bitrate1);
void gvret_bridge_receive(gvret_bridge_t *bridge, const uint8_t *data,
                          size_t length);
bool gvret_bridge_stream_frame(gvret_bridge_t *bridge,
                               const can_capture_frame_t *frame);

#endif
