/*
 * test_gvret.c - host test for the GVRET / SavvyCAN serial binary codec.
 *
 * Returns 0 and prints "ALL GVRET TESTS PASS" on success; on any failure it
 * prints a reason and returns a nonzero status. Uses only the public API in
 * gvret.h.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "gvret.h"

static int failures = 0;

#define FAIL(msg) do { \
    printf("FAIL (%s:%d): %s\n", __func__, __LINE__, (msg)); \
    failures++; \
} while (0)

#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); } } while (0)

static int bytes_eq(const uint8_t *a, const uint8_t *b, size_t n)
{
    return memcmp(a, b, n) == 0;
}

static void dump(const char *label, const uint8_t *b, size_t n)
{
    printf("  %s:", label);
    for (size_t i = 0; i < n; i++) {
        printf(" %02X", b[i]);
    }
    printf("\n");
}

/* --- encode: known standard frame, exact bytes incl. checksum --- */
static void test_encode_standard(void)
{
    const uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    /* bus=0, id=0x7E8 std, dlc=8, ts=0x12345678 */
    const uint8_t expect[20] = {
        0xF1, 0x00,                     /* start + BUILD_CAN_FRAME             */
        0x78, 0x56, 0x34, 0x12,         /* ts_us LE                            */
        0xE8, 0x07, 0x00, 0x00,         /* id LE (std, no bit31)               */
        0x08,                           /* (dlc & 0x0F) | (bus << 4)           */
        0x01, 0x02, 0x03, 0x04,         /* data                                */
        0x05, 0x06, 0x07, 0x08,
        0x16                            /* XOR checksum of bytes 0..18         */
    };
    uint8_t out[32];
    size_t n = gvret_encode_frame(out, sizeof out, 0u, 0x7E8u, false, false,
                                  8u, data, 0x12345678u);
    CHECK(n == 20u, "standard frame length should be 20");
    if (n == 20u && !bytes_eq(out, expect, 20u)) {
        dump("got   ", out, 20u);
        dump("expect", expect, 20u);
        FAIL("standard frame bytes mismatch");
    }
}

/* --- encode: known extended frame, exact bytes incl. checksum --- */
static void test_encode_extended(void)
{
    const uint8_t data[3] = {0x03, 0x41, 0x0C};
    /* bus=1, id=0x18DAF110 ext, dlc=3, ts=1 */
    const uint8_t expect[15] = {
        0xF1, 0x00,
        0x01, 0x00, 0x00, 0x00,         /* ts_us LE = 1                        */
        0x10, 0xF1, 0xDA, 0x98,         /* id LE with bit31 set (0x98DAF110)   */
        0x13,                           /* dlc 3 | bus 1 << 4                  */
        0x03, 0x41, 0x0C,               /* data                                */
        0x0E                            /* XOR checksum                        */
    };
    uint8_t out[32];
    size_t n = gvret_encode_frame(out, sizeof out, 1u, 0x18DAF110u, true, false,
                                  3u, data, 1u);
    CHECK(n == 15u, "extended frame length should be 15");
    if (n == 15u && !bytes_eq(out, expect, 15u)) {
        dump("got   ", out, 15u);
        dump("expect", expect, 15u);
        FAIL("extended frame bytes mismatch");
    }
}

/* --- encode: self-checking checksum == XOR of all preceding bytes --- */
static void test_encode_checksum_property(void)
{
    const uint8_t data[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x11, 0x22, 0x33};
    uint8_t out[32];
    size_t n = gvret_encode_frame(out, sizeof out, 3u, 0x123u, false, false,
                                  8u, data, 0xCAFEBABEu);
    CHECK(n == 20u, "frame length should be 20");
    uint8_t xsum = 0u;
    for (size_t i = 0; i + 1u < n; i++) {
        xsum ^= out[i];
    }
    CHECK(xsum == out[n - 1u], "checksum must equal XOR of preceding bytes");
}

/* --- encode: remote frame omits data but keeps dlc nibble --- */
static void test_encode_remote(void)
{
    uint8_t out[32];
    /* rtr=true, dlc=8 -> no data section, 11 header + 1 checksum = 12 */
    size_t n = gvret_encode_frame(out, sizeof out, 0u, 0x7DFu, false, true,
                                  8u, NULL, 0u);
    CHECK(n == 12u, "remote frame should be 12 bytes (no payload)");
    CHECK(out[10] == 0x08u, "remote frame dlc nibble should still be 8");
}

/* --- encode: buffer-too-small returns 0 without overflow (2-byte cap) --- */
static void test_encode_too_small(void)
{
    const uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t guarded[4] = {0xAA, 0xAA, 0xAA, 0xAA};
    /* cap = 2 is far below the 20 bytes needed. */
    size_t n = gvret_encode_frame(guarded, 2u, 0u, 0x100u, false, false,
                                  8u, data, 0u);
    CHECK(n == 0u, "too-small buffer must return 0");
    CHECK(guarded[0] == 0xAAu && guarded[1] == 0xAAu &&
          guarded[2] == 0xAAu && guarded[3] == 0xAAu,
          "too-small buffer must not be written");

    /* Also exercise the other encoders at cap below their minimum. */
    uint8_t tiny[1] = {0xAA};
    CHECK(gvret_encode_keepalive_reply(tiny, sizeof tiny) == 0u, "ka too small");
    CHECK(gvret_encode_device_info(tiny, sizeof tiny, 123u) == 0u, "info small");
    CHECK(gvret_encode_numbuses(tiny, sizeof tiny, 2u) == 0u, "numbus small");
    CHECK(gvret_encode_time_sync(tiny, sizeof tiny, 9u) == 0u, "tsync small");
    CHECK(gvret_encode_canbus_params(tiny, sizeof tiny, true, 500000u,
                                     false, 0u) == 0u, "params small");
    CHECK(tiny[0] == 0xAAu, "tiny buffer must not be written");

    /* dlc out of range must be rejected. */
    CHECK(gvret_encode_frame(guarded, sizeof guarded, 0u, 1u, false, false,
                             9u, data, 0u) == 0u, "dlc>8 must return 0");
    /* non-empty payload with NULL data must be rejected. */
    CHECK(gvret_encode_frame(guarded, sizeof guarded, 0u, 1u, false, false,
                             4u, NULL, 0u) == 0u, "null data must return 0");
}

/* Feed a whole buffer at once; expect exactly one decoded command. */
static int feed_all(struct gvret_parser *p, const uint8_t *b, size_t n,
                    struct gvret_command *out)
{
    int hits = 0;
    for (size_t i = 0; i < n; i++) {
        if (gvret_parser_feed(p, b[i], out)) {
            hits++;
        }
    }
    return hits;
}

/* --- decode: BUILD_CAN_FRAME standard --- */
static void test_decode_build_frame_std(void)
{
    struct gvret_parser p;
    struct gvret_command cmd;
    gvret_parser_init(&p);

    /* id=0x1F0 std, bus=0, len=2, data AB CD */
    const uint8_t in[] = {
        0xF1, 0x00,
        0xF0, 0x01, 0x00, 0x00,   /* id LE = 0x000001F0 */
        0x00,                     /* bus */
        0x02,                     /* len */
        0xAB, 0xCD                /* data */
    };
    int hits = feed_all(&p, in, sizeof in, &cmd);
    CHECK(hits == 1, "exactly one command expected");
    CHECK(cmd.type == GVRET_CMD_SEND_FRAME, "type should be SEND_FRAME");
    CHECK(cmd.u.frame.id == 0x1F0u, "id should be 0x1F0");
    CHECK(cmd.u.frame.ext == false, "frame should be standard");
    CHECK(cmd.u.frame.rtr == false, "rtr should default false");
    CHECK(cmd.u.frame.bus == 0u, "bus should be 0");
    CHECK(cmd.u.frame.dlc == 2u, "dlc should be 2");
    CHECK(cmd.u.frame.data[0] == 0xABu && cmd.u.frame.data[1] == 0xCDu,
          "payload bytes mismatch");
    CHECK(cmd.u.frame.data[2] == 0x00u, "bytes beyond dlc should be zero");
}

/* --- decode: BUILD_CAN_FRAME extended (bit31 set) --- */
static void test_decode_build_frame_ext(void)
{
    struct gvret_parser p;
    struct gvret_command cmd;
    gvret_parser_init(&p);

    /* id word = 0x98DAF110 (ext flag + 0x18DAF110), bus=1, len=0 */
    const uint8_t in[] = {
        0xF1, 0x00,
        0x10, 0xF1, 0xDA, 0x98,
        0x01,
        0x00
    };
    int hits = feed_all(&p, in, sizeof in, &cmd);
    CHECK(hits == 1, "exactly one command expected (ext)");
    CHECK(cmd.type == GVRET_CMD_SEND_FRAME, "type should be SEND_FRAME (ext)");
    CHECK(cmd.u.frame.ext == true, "frame should be extended");
    CHECK(cmd.u.frame.id == 0x18DAF110u, "ext id should strip bit31");
    CHECK(cmd.u.frame.dlc == 0u, "dlc should be 0");
    CHECK(cmd.u.frame.bus == 1u, "bus should be 1");
}

/* --- decode: byte-fragmented feed reassembles correctly --- */
static void test_fragmented_feed(void)
{
    struct gvret_parser p;
    struct gvret_command cmd;
    gvret_parser_init(&p);

    const uint8_t in[] = {
        0xF1, 0x00,
        0x34, 0x12, 0x00, 0x00,   /* id LE = 0x1234 */
        0x02,                     /* bus */
        0x03,                     /* len */
        0x11, 0x22, 0x33          /* data */
    };
    const size_t n = sizeof in;

    /* Feed one byte at a time: no completion until the final byte. */
    for (size_t i = 0; i < n; i++) {
        int r = gvret_parser_feed(&p, in[i], &cmd);
        if (i < n - 1u) {
            CHECK(r == 0, "no completion before last byte");
        } else {
            CHECK(r == 1, "completion exactly on last byte");
        }
    }
    CHECK(cmd.type == GVRET_CMD_SEND_FRAME, "fragmented type SEND_FRAME");
    CHECK(cmd.u.frame.id == 0x1234u, "fragmented id mismatch");
    CHECK(cmd.u.frame.bus == 2u, "fragmented bus mismatch");
    CHECK(cmd.u.frame.dlc == 3u, "fragmented dlc mismatch");
    CHECK(cmd.u.frame.data[0] == 0x11u && cmd.u.frame.data[1] == 0x22u &&
          cmd.u.frame.data[2] == 0x33u, "fragmented payload mismatch");
}

/* --- decode: leading garbage is ignored, then a command is found --- */
static void test_noise_resync(void)
{
    struct gvret_parser p;
    struct gvret_command cmd;
    gvret_parser_init(&p);

    const uint8_t in[] = {
        0x00, 0xFF, 0x7E, 0x42,   /* noise */
        0xF1, 0x09                /* keepalive */
    };
    int hits = feed_all(&p, in, sizeof in, &cmd);
    CHECK(hits == 1, "noise then keepalive -> one command");
    CHECK(cmd.type == GVRET_CMD_KEEPALIVE, "resynced command is keepalive");
}

/* --- decode: the no-payload commands --- */
static void test_decode_simple_commands(void)
{
    struct gvret_parser p;
    struct gvret_command cmd;

    const struct { uint8_t sel; enum gvret_cmd_type type; const char *name; }
    cases[] = {
        { 0x01, GVRET_CMD_TIME_SYNC,        "time sync"   },
        { 0x06, GVRET_CMD_GET_CANBUS_PARAMS,"canbus params" },
        { 0x07, GVRET_CMD_GET_DEVICE_INFO,  "device info" },
        { 0x09, GVRET_CMD_KEEPALIVE,        "keepalive"   },
        { 0x0C, GVRET_CMD_GET_NUMBUSES,     "numbuses"    },
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        gvret_parser_init(&p);
        cmd.type = GVRET_CMD_NONE;
        int r0 = gvret_parser_feed(&p, 0xF1u, &cmd);
        int r1 = gvret_parser_feed(&p, cases[i].sel, &cmd);
        CHECK(r0 == 0, "start byte must not complete a command");
        CHECK(r1 == 1, cases[i].name);
        CHECK(cmd.type == cases[i].type, cases[i].name);
    }
}

/* --- decode: SETUP_CANBUS --- */
static void test_decode_setup_canbus(void)
{
    struct gvret_parser p;
    struct gvret_command cmd;
    gvret_parser_init(&p);

    /* CAN0 = 500000 (0x0007A120) with enable bit31 set -> 0x8007A120,
     * CAN1 = 0 (disabled). */
    const uint8_t in[] = {
        0xF1, 0x05,
        0x20, 0xA1, 0x07, 0x80,   /* CAN0 LE */
        0x00, 0x00, 0x00, 0x00    /* CAN1 LE */
    };
    int hits = feed_all(&p, in, sizeof in, &cmd);
    CHECK(hits == 1, "setup canbus -> one command");
    CHECK(cmd.type == GVRET_CMD_SETUP_CANBUS, "type SETUP_CANBUS");
    CHECK(cmd.u.setup.bus == 0u, "setup bus 0");
    CHECK(cmd.u.setup.enabled == true, "setup enabled");
    CHECK(cmd.u.setup.bitrate == 500000u, "setup bitrate 500000");
}

/* --- decode: two commands back to back stay aligned --- */
static void test_decode_back_to_back(void)
{
    struct gvret_parser p;
    struct gvret_command cmd;
    gvret_parser_init(&p);

    const uint8_t in[] = {
        0xF1, 0x09,               /* keepalive */
        0xF1, 0x00,               /* build frame ... */
        0x05, 0x00, 0x00, 0x00,   /* id 0x05 */
        0x00, 0x01, 0x99          /* bus 0, len 1, data 0x99 */
    };
    int hits = feed_all(&p, in, sizeof in, &cmd);
    CHECK(hits == 2, "two commands expected back to back");
    /* cmd holds the last one decoded. */
    CHECK(cmd.type == GVRET_CMD_SEND_FRAME, "last decoded is a frame");
    CHECK(cmd.u.frame.id == 0x05u && cmd.u.frame.dlc == 1u &&
          cmd.u.frame.data[0] == 0x99u, "second frame contents");
}

static void test_decode_oversized_frame_is_discarded(void)
{
    struct gvret_parser p;
    struct gvret_command cmd;
    gvret_parser_init(&p);

    uint8_t in[] = { 0xF1, 0x00, 0x23, 0x01, 0x00, 0x00, 0x00, 0x09,
                     1, 2, 3, 4, 5, 6, 7, 8, 9 };
    CHECK(feed_all(&p, in, sizeof in, &cmd) == 0,
          "oversized CAN frame must not be emitted");

    const uint8_t keepalive[] = { 0xF1, 0x09 };
    CHECK(feed_all(&p, keepalive, sizeof keepalive, &cmd) == 1 &&
          cmd.type == GVRET_CMD_KEEPALIVE,
          "parser must resynchronize after oversized frame");
}

/* --- encode: keepalive reply equals F1 09 DE AD --- */
static void test_encode_keepalive(void)
{
    const uint8_t expect[4] = {0xF1, 0x09, 0xDE, 0xAD};
    uint8_t out[8];
    size_t n = gvret_encode_keepalive_reply(out, sizeof out);
    CHECK(n == 4u, "keepalive reply length 4");
    if (n == 4u && !bytes_eq(out, expect, 4u)) {
        dump("got   ", out, 4u);
        FAIL("keepalive reply must be F1 09 DE AD");
    }
}

/* --- encode: device info, numbuses, canbus params, time sync --- */
static void test_encode_other_replies(void)
{
    uint8_t out[16];

    size_t n = gvret_encode_device_info(out, sizeof out, 0x0203u);
    CHECK(n == 8u, "device info length 8");
    const uint8_t di[8] = {0xF1, 0x07, 0x03, 0x02, 0x20, 0x01, 0x00, 0x00};
    CHECK(n == 8u && bytes_eq(out, di, 8u), "device info bytes");

    n = gvret_encode_numbuses(out, sizeof out, 2u);
    const uint8_t nb[3] = {0xF1, 0x0C, 0x02};
    CHECK(n == 3u && bytes_eq(out, nb, 3u), "numbuses bytes");

    n = gvret_encode_time_sync(out, sizeof out, 0x11223344u);
    const uint8_t ts[6] = {0xF1, 0x01, 0x44, 0x33, 0x22, 0x11};
    CHECK(n == 6u && bytes_eq(out, ts, 6u), "time sync bytes");

    n = gvret_encode_canbus_params(out, sizeof out, true, 500000u, false, 0u);
    const uint8_t cp[12] = {
        0xF1, 0x06,
        0x01, 0x20, 0xA1, 0x07, 0x00,   /* bus0 enabled, 500000 LE */
        0x00, 0x00, 0x00, 0x00, 0x00    /* bus1 disabled, 0 */
    };
    CHECK(n == 12u && bytes_eq(out, cp, 12u), "canbus params bytes");
}

/* --- round trip: decode a built frame, re-encode in device->host stream --- */
static void test_round_trip(void)
{
    struct gvret_parser p;
    struct gvret_command cmd;
    gvret_parser_init(&p);

    const uint8_t data[4] = {0xA1, 0xB2, 0xC3, 0xD4};
    const uint8_t in[] = {
        0xF1, 0x00,
        0x10, 0xF1, 0xDA, 0x98,   /* ext id 0x18DAF110 */
        0x01,                     /* bus 1 */
        0x04,                     /* len 4 */
        0xA1, 0xB2, 0xC3, 0xD4
    };
    int hits = feed_all(&p, in, sizeof in, &cmd);
    CHECK(hits == 1, "round trip decode one frame");

    uint8_t out[GVRET_MAX_FRAME_BYTES];
    size_t n = gvret_encode_frame(out, sizeof out, cmd.u.frame.bus,
                                  cmd.u.frame.id, cmd.u.frame.ext,
                                  cmd.u.frame.rtr, cmd.u.frame.dlc,
                                  cmd.u.frame.data, 7u);
    const uint8_t expect[15] = {       /* 15 bytes before the checksum */
        0xF1, 0x00,
        0x07, 0x00, 0x00, 0x00,   /* ts = 7 */
        0x10, 0xF1, 0xDA, 0x98,   /* re-set ext bit31 */
        0x14,                     /* dlc 4 | bus 1 << 4 */
        0xA1, 0xB2, 0xC3, 0xD4
    };
    uint8_t xsum = 0u;
    for (size_t i = 0; i < 15u; i++) {
        xsum ^= expect[i];
    }
    CHECK(n == 16u, "round trip re-encode length");
    CHECK(n == 16u && bytes_eq(out, expect, 15u) && out[15] == xsum,
          "round trip re-encode bytes");
    (void)data;
}

/* --- robustness: NULL arguments are handled gracefully --- */
static void test_null_args(void)
{
    struct gvret_parser p;
    struct gvret_command cmd;
    gvret_parser_init(&p);
    gvret_parser_init(NULL); /* must not crash */

    CHECK(gvret_parser_feed(NULL, 0xF1u, &cmd) == 0, "null parser returns 0");
    CHECK(gvret_parser_feed(&p, 0xF1u, NULL) == 0, "null out returns 0");
    CHECK(gvret_encode_frame(NULL, 20u, 0u, 1u, false, false, 0u, NULL, 0u)
          == 0u, "null buf frame returns 0");
}

int main(void)
{
    test_encode_standard();
    test_encode_extended();
    test_encode_checksum_property();
    test_encode_remote();
    test_encode_too_small();
    test_decode_build_frame_std();
    test_decode_build_frame_ext();
    test_fragmented_feed();
    test_noise_resync();
    test_decode_simple_commands();
    test_decode_setup_canbus();
    test_decode_back_to_back();
    test_decode_oversized_frame_is_discarded();
    test_encode_keepalive();
    test_encode_other_replies();
    test_round_trip();
    test_null_args();

    if (failures != 0) {
        printf("GVRET TESTS FAILED: %d failure(s)\n", failures);
        return 1;
    }
    printf("ALL GVRET TESTS PASS\n");
    return 0;
}
