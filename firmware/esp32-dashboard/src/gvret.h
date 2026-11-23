#ifndef GVRET_H
#define GVRET_H
/*
 * gvret.h - GVRET / ESP32RET serial binary protocol codec (the protocol
 * SavvyCAN speaks over serial/TCP).
 *
 * Freestanding: depends only on <stdint.h>, <stddef.h>, <stdbool.h>
 * (<stdbool.h> is a C11 freestanding header) and, in the .c, <string.h>.
 * No malloc, no printf, no OS calls, no mutable globals - the caller owns
 * all state through the structs defined here.
 *
 * ------------------------------------------------------------------
 * Wire framing implemented by THIS module
 * ------------------------------------------------------------------
 * Every command/stream record begins with the start byte 0xF1, followed by
 * a selector byte that names the command.
 *
 * Host -> device commands decoded by the parser:
 *   0x00 BUILD_CAN_FRAME    transmit a frame (canonical GVRET layout)
 *   0x01 TIME_SYNC          request current micros (no payload)
 *   0x05 SETUP_CANBUS       set bus bitrate/enable
 *   0x06 GET_CANBUS_PARAMS  request bus params (no payload)
 *   0x07 GET_DEVICE_INFO    request device info (no payload)
 *   0x09 KEEPALIVE          device must reply 0xF1 0x09 0xDE 0xAD
 *   0x0C GET_NUMBUSES       request number of buses (no payload)
 *
 * BUILD_CAN_FRAME payload (host -> device), canonical GVRET layout:
 *   0xF1 0x00, id(4 LE; bit31 set => extended), bus(1), len(1), data[len].
 *   SavvyCAN does not express RTR here, so decoded rtr defaults to false.
 *   bit31 of the id word is the extended flag; the remaining 31 bits are the
 *   identifier (an 11-bit value for standard frames, 29-bit for extended).
 *
 * SETUP_CANBUS payload (host -> device):
 *   SavvyCAN sends the canonical 8-byte payload: CAN0 setting (4 LE) then
 *   CAN1 setting (4 LE). In each 32-bit word bit31 = enabled, bit30 =
 *   listen-only, the low 30 bits = bitrate. This module decodes CAN0 into the
 *   command (bus 0, enabled, bitrate); the CAN1 word is consumed to keep the
 *   byte stream in sync but is not surfaced (listen-only is not surfaced).
 *
 * Device -> host CAN frame stream (gvret_encode_frame), this project's framing:
 *   byte0  = 0xF1
 *   byte1  = 0x00
 *   byte2..5  = timestamp microseconds, little-endian (uint32)
 *   byte6..9  = CAN id, little-endian (uint32); extended => id | 0x80000000
 *   byte10    = (dlc & 0x0F) | ((bus & 0x0F) << 4)
 *   byte11..  = data[0..dlc-1]  (omitted for remote frames or dlc 0)
 *   last byte = checksum = XOR of every preceding byte (from byte0)
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Protocol literals on the wire. */
#define GVRET_START_BYTE             0xF1u
#define GVRET_CMDB_BUILD_CAN_FRAME   0x00u
#define GVRET_CMDB_TIME_SYNC         0x01u
#define GVRET_CMDB_SETUP_CANBUS      0x05u
#define GVRET_CMDB_GET_CANBUS_PARAMS 0x06u
#define GVRET_CMDB_GET_DEVICE_INFO   0x07u
#define GVRET_CMDB_KEEPALIVE         0x09u
#define GVRET_CMDB_GET_NUMBUSES      0x0Cu

/* Maximum bytes a single encoded device->host frame can occupy:
 * 11 header + 8 data + 1 checksum. */
#define GVRET_MAX_FRAME_BYTES 20u

/* Kind of command produced by the parser. */
enum gvret_cmd_type {
    GVRET_CMD_NONE = 0,
    GVRET_CMD_SEND_FRAME,       /* BUILD_CAN_FRAME, carries .u.frame   */
    GVRET_CMD_TIME_SYNC,
    GVRET_CMD_SETUP_CANBUS,     /* carries .u.setup                    */
    GVRET_CMD_GET_DEVICE_INFO,
    GVRET_CMD_GET_CANBUS_PARAMS,
    GVRET_CMD_KEEPALIVE,
    GVRET_CMD_GET_NUMBUSES
};

/* A CAN frame the host asked the device to transmit (BUILD_CAN_FRAME). */
struct gvret_frame {
    uint32_t id;        /* identifier, 31 bits (bit31 flag stripped)        */
    bool     ext;       /* true => extended (29-bit) identifier             */
    bool     rtr;       /* remote frame; SavvyCAN never sets it => false    */
    uint8_t  bus;       /* target bus index                                 */
    uint8_t  dlc;       /* data length, 0..8                                */
    uint8_t  data[8];   /* payload; bytes beyond dlc are zero               */
};

/* A SETUP_CANBUS request (CAN0 word, see header notes). */
struct gvret_setup {
    uint8_t  bus;       /* always 0 (CAN0 word)                             */
    uint32_t bitrate;   /* requested bitrate in bit/s                       */
    bool     enabled;   /* enable flag (word bit31)                         */
};

/* A fully decoded command. Inspect .type, then the matching union member. */
struct gvret_command {
    enum gvret_cmd_type type;
    union {
        struct gvret_frame frame;
        struct gvret_setup setup;
    } u;
};

/* Byte-at-a-time parser state. Caller owns the storage; zero-init via
 * gvret_parser_init(). Opaque in spirit - do not rely on field layout. */
struct gvret_parser {
    uint8_t state;        /* internal state machine phase */
    uint8_t idx;          /* bytes gathered in the current fixed phase */
    uint8_t data_total;   /* BUILD_CAN_FRAME declared data length */
    uint8_t data_got;     /* BUILD_CAN_FRAME data bytes consumed so far */
    uint8_t hdr[6];       /* BUILD_CAN_FRAME header: id(4),bus(1),len(1) */
    uint8_t setup[8];     /* SETUP_CANBUS 8-byte payload */
    uint8_t data[8];      /* BUILD_CAN_FRAME payload (stored, capped at 8) */
};

/* Reset a parser to the idle state. Safe to call on any time. */
void gvret_parser_init(struct gvret_parser *p);

/*
 * Feed one received byte. Bytes may arrive fragmented across any number of
 * calls. Returns 1 and fills *out when a complete command is decoded, else 0.
 * Stray bytes before a 0xF1 start byte are ignored; an unknown command
 * selector resynchronises to the next start byte.
 */
int gvret_parser_feed(struct gvret_parser *p, uint8_t byte,
                      struct gvret_command *out);

/*
 * Encode a device->host CAN frame stream record (framing above). Returns the
 * number of bytes written, or 0 if buf is NULL, cap is too small, dlc > 8, or
 * a non-empty payload was requested with data == NULL. Never writes past cap.
 * For remote frames or dlc 0 the data section is omitted.
 */
size_t gvret_encode_frame(uint8_t *buf, size_t cap, uint8_t bus, uint32_t id,
                          bool ext, bool rtr, uint8_t dlc,
                          const uint8_t *data, uint32_t ts_us);

/* Encode the keepalive reply 0xF1 0x09 0xDE 0xAD (4 bytes). 0 if too small. */
size_t gvret_encode_keepalive_reply(uint8_t *buf, size_t cap);

/*
 * Encode the GET_DEVICE_INFO reply (8 bytes):
 *   0xF1, 0x07, build(2 LE), 0x20 (EEPROM version), 0x01 (file output type),
 *   0x00 (auto-start logging off), 0x00 (single-wire mode off).
 * The four trailing bytes are the fixed descriptor SavvyCAN consumes after the
 * build number. Returns 8, or 0 if too small.
 */
size_t gvret_encode_device_info(uint8_t *buf, size_t cap, uint16_t build);

/* Encode the GET_NUMBUSES reply 0xF1 0x0C <n> (3 bytes). 0 if too small. */
size_t gvret_encode_numbuses(uint8_t *buf, size_t cap, uint8_t n);

/*
 * Encode the GET_CANBUS_PARAMS reply (12 bytes):
 *   0xF1, 0x06,
 *   bus0: enabled(1; listen-only bit4 left 0), bitrate(4 LE),
 *   bus1: enabled(1; listen-only bit4 left 0), bitrate(4 LE).
 * Returns 12, or 0 if too small.
 */
size_t gvret_encode_canbus_params(uint8_t *buf, size_t cap,
                                  bool en0, uint32_t bitrate0,
                                  bool en1, uint32_t bitrate1);

/* Encode the TIME_SYNC reply 0xF1 0x01 <micros 4 LE> (6 bytes). 0 if small. */
size_t gvret_encode_time_sync(uint8_t *buf, size_t cap, uint32_t micros);

#endif /* GVRET_H */
