/*
 * gvret.c - GVRET / ESP32RET serial binary protocol codec.
 * See gvret.h for the authoritative wire framing and design notes.
 *
 * Freestanding: <stdint.h>, <stddef.h>, <stdbool.h> (all C11 freestanding)
 * plus <string.h> for memcpy/memset. No malloc, no I/O, no globals.
 */

#include "gvret.h"
#include <string.h>

/* Internal parser phases. */
enum {
    GP_IDLE = 0,    /* waiting for the 0xF1 start byte            */
    GP_CMD,         /* start seen, waiting for the selector byte  */
    GP_FRAME_HDR,   /* collecting BUILD_CAN_FRAME 6-byte header   */
    GP_FRAME_DATA,  /* collecting BUILD_CAN_FRAME payload bytes   */
    GP_SETUP        /* collecting SETUP_CANBUS 8-byte payload     */
};

/* Store a 32-bit value little-endian into b[0..3]. */
static void put_u32le(uint8_t *b, uint32_t v)
{
    b[0] = (uint8_t)(v & 0xFFu);
    b[1] = (uint8_t)((v >> 8) & 0xFFu);
    b[2] = (uint8_t)((v >> 16) & 0xFFu);
    b[3] = (uint8_t)((v >> 24) & 0xFFu);
}

/* Read a 32-bit little-endian value from b[0..3]. */
static uint32_t get_u32le(const uint8_t *b)
{
    return (uint32_t)b[0]
         | ((uint32_t)b[1] << 8)
         | ((uint32_t)b[2] << 16)
         | ((uint32_t)b[3] << 24);
}

void gvret_parser_init(struct gvret_parser *p)
{
    if (!p) {
        return;
    }
    memset(p, 0, sizeof(*p));
    p->state = GP_IDLE;
}

/* Finalise a BUILD_CAN_FRAME into *out from the parser's gathered bytes. */
static void emit_frame(const struct gvret_parser *p, struct gvret_command *out)
{
    uint32_t idword = get_u32le(p->hdr);
    uint8_t  dlc    = (p->data_total > 8u) ? 8u : p->data_total;

    out->type            = GVRET_CMD_SEND_FRAME;
    out->u.frame.ext     = (idword & 0x80000000u) != 0u;
    out->u.frame.id      = idword & 0x7FFFFFFFu; /* strip extended flag */
    out->u.frame.rtr     = false;                /* not expressed by SavvyCAN */
    out->u.frame.bus     = p->hdr[4];
    out->u.frame.dlc     = dlc;
    memcpy(out->u.frame.data, p->data, sizeof(out->u.frame.data));
}

/* Finalise a SETUP_CANBUS into *out from the parser's gathered bytes. */
static void emit_setup(const struct gvret_parser *p, struct gvret_command *out)
{
    uint32_t word = get_u32le(p->setup); /* CAN0 word; CAN1 (setup[4..7]) ignored */

    out->type            = GVRET_CMD_SETUP_CANBUS;
    out->u.setup.bus     = 0u;
    out->u.setup.enabled = (word & 0x80000000u) != 0u;
    out->u.setup.bitrate = word & 0x3FFFFFFFu;   /* strip enable + listen-only */
}

int gvret_parser_feed(struct gvret_parser *p, uint8_t byte,
                      struct gvret_command *out)
{
    if (!p || !out) {
        return 0;
    }

    switch (p->state) {
    case GP_IDLE:
        if (byte == GVRET_START_BYTE) {
            p->state = GP_CMD;
        }
        return 0;

    case GP_CMD:
        switch (byte) {
        case GVRET_CMDB_BUILD_CAN_FRAME:
            p->state = GP_FRAME_HDR;
            p->idx = 0u;
            return 0;
        case GVRET_CMDB_SETUP_CANBUS:
            p->state = GP_SETUP;
            p->idx = 0u;
            return 0;
        case GVRET_CMDB_TIME_SYNC:
            out->type = GVRET_CMD_TIME_SYNC;
            p->state = GP_IDLE;
            return 1;
        case GVRET_CMDB_GET_CANBUS_PARAMS:
            out->type = GVRET_CMD_GET_CANBUS_PARAMS;
            p->state = GP_IDLE;
            return 1;
        case GVRET_CMDB_GET_DEVICE_INFO:
            out->type = GVRET_CMD_GET_DEVICE_INFO;
            p->state = GP_IDLE;
            return 1;
        case GVRET_CMDB_KEEPALIVE:
            out->type = GVRET_CMD_KEEPALIVE;
            p->state = GP_IDLE;
            return 1;
        case GVRET_CMDB_GET_NUMBUSES:
            out->type = GVRET_CMD_GET_NUMBUSES;
            p->state = GP_IDLE;
            return 1;
        default:
            /* Unknown selector: resync. If it was itself a start byte, stay
             * primed for the next selector; otherwise drop back to idle. */
            p->state = (byte == GVRET_START_BYTE) ? GP_CMD : GP_IDLE;
            return 0;
        }

    case GP_FRAME_HDR:
        p->hdr[p->idx++] = byte;
        if (p->idx < 6u) {
            return 0;
        }
        /* Header complete: id(4), bus(1), len(1). */
        p->data_total = p->hdr[5];
        p->data_got = 0u;
        memset(p->data, 0, sizeof(p->data));
        if (p->data_total == 0u) {
            emit_frame(p, out);
            p->state = GP_IDLE;
            return 1;
        }
        p->state = GP_FRAME_DATA;
        return 0;

    case GP_FRAME_DATA:
        if (p->data_got < 8u) {
            p->data[p->data_got] = byte;
        }
        p->data_got++;
        /* Consume the full declared length even if it exceeds 8 (malformed),
         * so the stream stays aligned; only the first 8 bytes are stored. */
        if (p->data_got < p->data_total) {
            return 0;
        }
        if (p->data_total > 8u) {
            p->state = GP_IDLE;
            return 0;
        }
        emit_frame(p, out);
        p->state = GP_IDLE;
        return 1;

    case GP_SETUP:
        p->setup[p->idx++] = byte;
        if (p->idx < 8u) {
            return 0;
        }
        emit_setup(p, out);
        p->state = GP_IDLE;
        return 1;

    default:
        p->state = GP_IDLE;
        return 0;
    }
}

size_t gvret_encode_frame(uint8_t *buf, size_t cap, uint8_t bus, uint32_t id,
                          bool ext, bool rtr, uint8_t dlc,
                          const uint8_t *data, uint32_t ts_us)
{
    if (!buf || dlc > 8u) {
        return 0u;
    }

    uint8_t payload = (rtr || dlc == 0u) ? 0u : dlc;
    if (payload != 0u && !data) {
        return 0u;
    }

    size_t total = 11u + (size_t)payload + 1u; /* header + data + checksum */
    if (cap < total) {
        return 0u;
    }

    uint32_t idword = ext ? ((id & 0x1FFFFFFFu) | 0x80000000u)
                          : (id & 0x7FFFFFFFu);

    size_t i = 0u;
    buf[i++] = (uint8_t)GVRET_START_BYTE;
    buf[i++] = (uint8_t)GVRET_CMDB_BUILD_CAN_FRAME;
    put_u32le(buf + i, ts_us);
    i += 4u;
    put_u32le(buf + i, idword);
    i += 4u;
    buf[i++] = (uint8_t)((dlc & 0x0Fu) | ((uint32_t)(bus & 0x0Fu) << 4));

    for (uint8_t k = 0u; k < payload; k++) {
        buf[i++] = data[k];
    }

    uint8_t checksum = 0u;
    for (size_t j = 0u; j < i; j++) {
        checksum ^= buf[j];
    }
    buf[i++] = checksum;

    return i; /* == total */
}

size_t gvret_encode_keepalive_reply(uint8_t *buf, size_t cap)
{
    if (!buf || cap < 4u) {
        return 0u;
    }
    buf[0] = (uint8_t)GVRET_START_BYTE;
    buf[1] = (uint8_t)GVRET_CMDB_KEEPALIVE;
    buf[2] = 0xDEu;
    buf[3] = 0xADu;
    return 4u;
}

size_t gvret_encode_device_info(uint8_t *buf, size_t cap, uint16_t build)
{
    if (!buf || cap < 8u) {
        return 0u;
    }
    buf[0] = (uint8_t)GVRET_START_BYTE;
    buf[1] = (uint8_t)GVRET_CMDB_GET_DEVICE_INFO;
    buf[2] = (uint8_t)(build & 0xFFu);
    buf[3] = (uint8_t)((build >> 8) & 0xFFu);
    buf[4] = 0x20u; /* EEPROM version       */
    buf[5] = 0x01u; /* file output type     */
    buf[6] = 0x00u; /* auto-start logging   */
    buf[7] = 0x00u; /* single-wire mode     */
    return 8u;
}

size_t gvret_encode_numbuses(uint8_t *buf, size_t cap, uint8_t n)
{
    if (!buf || cap < 3u) {
        return 0u;
    }
    buf[0] = (uint8_t)GVRET_START_BYTE;
    buf[1] = (uint8_t)GVRET_CMDB_GET_NUMBUSES;
    buf[2] = n;
    return 3u;
}

size_t gvret_encode_canbus_params_lo(uint8_t *buf, size_t cap,
                                     bool en0, bool lo0, uint32_t bitrate0,
                                     bool en1, bool lo1, uint32_t bitrate1)
{
    if (!buf || cap < 12u) {
        return 0u;
    }
    buf[0] = (uint8_t)GVRET_START_BYTE;
    buf[1] = (uint8_t)GVRET_CMDB_GET_CANBUS_PARAMS;
    buf[2] = (uint8_t)((en0 ? 0x01u : 0x00u) | (lo0 ? 0x10u : 0x00u));
    put_u32le(buf + 3, bitrate0);
    buf[7] = (uint8_t)((en1 ? 0x01u : 0x00u) | (lo1 ? 0x10u : 0x00u));
    put_u32le(buf + 8, bitrate1);
    return 12u;
}

size_t gvret_encode_canbus_params(uint8_t *buf, size_t cap,
                                  bool en0, uint32_t bitrate0,
                                  bool en1, uint32_t bitrate1)
{
    return gvret_encode_canbus_params_lo(buf, cap, en0, false, bitrate0, en1, false, bitrate1);
}

size_t gvret_encode_time_sync(uint8_t *buf, size_t cap, uint32_t micros)
{
    if (!buf || cap < 6u) {
        return 0u;
    }
    buf[0] = (uint8_t)GVRET_START_BYTE;
    buf[1] = (uint8_t)GVRET_CMDB_TIME_SYNC;
    put_u32le(buf + 2, micros);
    return 6u;
}
