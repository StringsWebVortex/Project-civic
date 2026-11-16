#include <stdio.h>
#include <string.h>

#include "gvret_bridge.h"

typedef struct {
    struct gvret_frame last_frame;
    uint8_t output[32];
    size_t output_length;
    uint32_t now_us;
    bool accept_frame;
    bool accept_output;
    unsigned frame_calls;
} test_context_t;

static bool send_frame(void *context, const struct gvret_frame *frame)
{
    test_context_t *test = context;
    test->last_frame = *frame;
    test->frame_calls++;
    return test->accept_frame;
}

static bool send_bytes(void *context, const uint8_t *data, size_t length)
{
    test_context_t *test = context;
    if (!test->accept_output || length > sizeof test->output) return false;
    memcpy(test->output, data, length);
    test->output_length = length;
    return true;
}

static uint32_t get_micros(void *context)
{
    return ((test_context_t *)context)->now_us;
}

static void feed(gvret_bridge_t *bridge, const uint8_t *data, size_t length)
{
    gvret_bridge_receive(bridge, data, length);
}

static int fail(const char *message)
{
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(void)
{
    test_context_t test = { .now_us = 0x12345678u, .accept_frame = true,
                            .accept_output = true };
    gvret_bridge_t bridge;
    if (!gvret_bridge_init(&bridge, &test, send_frame, send_bytes, get_micros))
        return fail("bridge initialization");
    gvret_bridge_set_bitrates(&bridge, 500000u, 125000u);

    const uint8_t keepalive[] = { GVRET_START_BYTE, GVRET_CMDB_KEEPALIVE };
    feed(&bridge, keepalive, sizeof keepalive);
    const uint8_t keepalive_reply[] = { 0xF1, 0x09, 0xDE, 0xAD };
    if (test.output_length != sizeof keepalive_reply ||
        memcmp(test.output, keepalive_reply, sizeof keepalive_reply) != 0)
        return fail("keepalive response");

    const uint8_t get_buses[] = { GVRET_START_BYTE, GVRET_CMDB_GET_NUMBUSES };
    feed(&bridge, get_buses, sizeof get_buses);
    if (test.output_length != 3u || test.output[0] != GVRET_START_BYTE ||
        test.output[1] != GVRET_CMDB_GET_NUMBUSES || test.output[2] != 2u)
        return fail("bus count response");

    const uint8_t get_params[] = { GVRET_START_BYTE, GVRET_CMDB_GET_CANBUS_PARAMS };
    feed(&bridge, get_params, sizeof get_params);
    const uint8_t params_reply[] = {
        0xF1, 0x06, 0x01, 0x20, 0xA1, 0x07, 0x00,
        0x01, 0x48, 0xE8, 0x01, 0x00
    };
    if (test.output_length != sizeof params_reply ||
        memcmp(test.output, params_reply, sizeof params_reply) != 0)
        return fail("CAN bitrate response");

    const uint8_t time_sync[] = { GVRET_START_BYTE, GVRET_CMDB_TIME_SYNC };
    feed(&bridge, time_sync, sizeof time_sync);
    const uint8_t time_reply[] = { 0xF1, 0x01, 0x78, 0x56, 0x34, 0x12 };
    if (test.output_length != sizeof time_reply ||
        memcmp(test.output, time_reply, sizeof time_reply) != 0)
        return fail("time-sync response");

    const uint8_t build_frame[] = {
        0xF1, 0x00, 0x23, 0x01, 0x00, 0x00, 0x01, 0x02, 0xAA, 0x55
    };
    feed(&bridge, build_frame, sizeof build_frame);
    if (test.frame_calls != 1u || test.last_frame.bus != 1u ||
        test.last_frame.id != 0x123u || test.last_frame.dlc != 2u ||
        test.last_frame.data[0] != 0xAAu || test.last_frame.data[1] != 0x55u)
        return fail("CAN transmit dispatch");

    test.accept_frame = false;
    feed(&bridge, build_frame, sizeof build_frame);
    if (bridge.stats.rejected_frames != 1u)
        return fail("rejected CAN transmit counted");

    const uint8_t invalid_bus[] = {
        0xF1, 0x00, 0x23, 0x01, 0x00, 0x00, 0x02, 0x00
    };
    feed(&bridge, invalid_bus, sizeof invalid_bus);
    if (test.frame_calls != 2u || bridge.stats.rejected_frames != 2u)
        return fail("invalid bus rejected");

    const uint8_t invalid_standard_id[] = {
        0xF1, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00
    };
    feed(&bridge, invalid_standard_id, sizeof invalid_standard_id);
    if (test.frame_calls != 2u || bridge.stats.rejected_frames != 3u)
        return fail("out-of-range standard identifier rejected");

    const uint8_t setup[] = {
        0xF1, 0x05, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00
    };
    feed(&bridge, setup, sizeof setup);
    if (bridge.stats.ignored_setup_commands != 1u)
        return fail("automatic bitrate setup remains ignored");

    test.accept_output = false;
    feed(&bridge, keepalive, sizeof keepalive);
    if (bridge.stats.output_drops != 1u)
        return fail("failed output counted");

    can_capture_frame_t captured = {0};
    captured.bus = 1u;
    captured.identifier = 0x321u;
    captured.dlc = 2u;
    captured.data[0] = 0x10u;
    captured.data[1] = 0x20u;
    captured.timestamp_us = 42u;
    test.accept_output = true;
    if (!gvret_bridge_stream_frame(&bridge, &captured) ||
        test.output_length != 14u || test.output[0] != GVRET_START_BYTE ||
        test.output[10] != 0x12u || test.output[11] != 0x10u ||
        test.output[12] != 0x20u)
        return fail("captured frame stream");

    /* device info reply, reset of a half-parsed command */
    test.accept_output = true; test.output_length = 0u;
    const uint8_t info[] = { GVRET_START_BYTE, GVRET_CMDB_GET_DEVICE_INFO };
    feed(&bridge, info, sizeof info);
    if (test.output_length == 0u || test.output[1] != GVRET_CMDB_GET_DEVICE_INFO)
        return fail("device info reply");
    feed(&bridge, info, 1u);                 /* start byte only */
    gvret_bridge_reset(&bridge);
    gvret_bridge_reset(NULL);
    test.output_length = 0u;
    feed(&bridge, keepalive + 1, 1u);        /* selector alone after reset: ignored */
    if (test.output_length != 0u) return fail("reset drops the half command");

    /* stream: encoder rejects dlc > 8; transport refusal counts a drop */
    const uint32_t drops = bridge.stats.output_drops;
    can_capture_frame_t bad = captured;
    bad.dlc = 9u;
    if (gvret_bridge_stream_frame(&bridge, &bad) || bridge.stats.output_drops != drops + 1u)
        return fail("oversized frame dropped");
    test.accept_output = false;
    if (gvret_bridge_stream_frame(&bridge, &captured) || bridge.stats.output_drops != drops + 2u)
        return fail("send refusal dropped");

    puts("ALL GVRET BRIDGE TESTS PASS");
    return 0;
}
