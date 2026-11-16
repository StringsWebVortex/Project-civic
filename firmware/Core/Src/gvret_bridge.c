#include "gvret_bridge.h"

#include <string.h>

static void send_reply(gvret_bridge_t *bridge, const uint8_t *data, size_t length)
{
    if (length != 0u && !bridge->send(bridge->context, data, length))
        bridge->stats.output_drops++;
}

static void handle_command(gvret_bridge_t *bridge,
                           const struct gvret_command *command)
{
    uint8_t reply[12];
    size_t length = 0u;

    switch (command->type) {
    case GVRET_CMD_SEND_FRAME: {
        const struct gvret_frame *frame = &command->u.frame;
        const uint32_t max_id = frame->ext ? 0x1FFFFFFFu : 0x7FFu;
        if (frame->bus > 1u || frame->id > max_id ||
            !bridge->transmit(bridge->context, frame))
            bridge->stats.rejected_frames++;
        return;
    }
    case GVRET_CMD_TIME_SYNC:
        length = gvret_encode_time_sync(reply, sizeof reply,
                                        bridge->micros(bridge->context));
        break;
    case GVRET_CMD_GET_CANBUS_PARAMS:
        length = gvret_encode_canbus_params(reply, sizeof reply, true,
                                             bridge->bitrate[0], true,
                                             bridge->bitrate[1]);
        break;
    case GVRET_CMD_GET_DEVICE_INFO:
        length = gvret_encode_device_info(reply, sizeof reply, 0x0001u);
        break;
    case GVRET_CMD_KEEPALIVE:
        length = gvret_encode_keepalive_reply(reply, sizeof reply);
        break;
    case GVRET_CMD_GET_NUMBUSES:
        length = gvret_encode_numbuses(reply, sizeof reply, 2u);
        break;
    case GVRET_CMD_SETUP_CANBUS:
        /* Passive discovery and the explicit BITRATE command own bus speed. */
        bridge->stats.ignored_setup_commands++;
        /* fall through */
    case GVRET_CMD_NONE:
    default:
        return;
    }

    send_reply(bridge, reply, length);
}

bool gvret_bridge_init(gvret_bridge_t *bridge, void *context,
                       gvret_bridge_tx_fn transmit, gvret_bridge_send_fn send,
                       gvret_bridge_micros_fn micros)
{
    if (!bridge || !transmit || !send || !micros) return false;

    memset(bridge, 0, sizeof *bridge);
    bridge->context = context;
    bridge->transmit = transmit;
    bridge->send = send;
    bridge->micros = micros;
    gvret_parser_init(&bridge->parser);
    return true;
}

void gvret_bridge_reset(gvret_bridge_t *bridge)
{
    if (bridge) gvret_parser_init(&bridge->parser);
}

void gvret_bridge_set_bitrates(gvret_bridge_t *bridge, uint32_t bitrate0,
                               uint32_t bitrate1)
{
    if (!bridge) return;
    bridge->bitrate[0] = bitrate0;
    bridge->bitrate[1] = bitrate1;
}

void gvret_bridge_receive(gvret_bridge_t *bridge, const uint8_t *data,
                          size_t length)
{
    if (!bridge || (!data && length != 0u)) return;

    for (size_t i = 0u; i < length; i++) {
        struct gvret_command command;
        if (gvret_parser_feed(&bridge->parser, data[i], &command))
            handle_command(bridge, &command);
    }
}

bool gvret_bridge_stream_frame(gvret_bridge_t *bridge,
                               const can_capture_frame_t *frame)
{
    if (!bridge || !frame) return false;

    uint8_t encoded[GVRET_MAX_FRAME_BYTES];
    const size_t length = gvret_encode_frame(encoded, sizeof encoded, frame->bus,
        frame->identifier, frame->extended != 0u, frame->remote != 0u,
        frame->dlc, frame->data, frame->timestamp_us);
    if (length == 0u) {
        bridge->stats.output_drops++;
        return false;
    }

    if (!bridge->send(bridge->context, encoded, length)) {
        bridge->stats.output_drops++;
        return false;
    }
    return true;
}
