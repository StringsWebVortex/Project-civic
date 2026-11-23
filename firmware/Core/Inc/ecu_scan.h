#ifndef ECU_SCAN_H
#define ECU_SCAN_H
/*
 * ecu_scan.h - finds the diagnostic controllers (ECUs) on the bus by sending
 * one single-frame request to each physical request address and listening on
 * the matching response address. Own implementation.
 *
 * Addressing (ISO 15765-4 as summarised by SAE J1979 / python-udsoncan and the
 * Linux can-utils isotpscan tool, github.com/linux-can/can-utils isotpscan.c):
 *   11-bit: request 0x7E0..0x7E7, response = request + 8 (0x7E8..0x7EF).
 *           Other 11-bit ranges are scanned with the same +8 rule.
 *   29-bit normal fixed addressing: request 0x18DA<TA>F1, response 0x18DAF1<TA>
 *           (tester address F1), TA = 0x00..0xFF.
 *
 * A frame on the response ID counts as "found" only when it has a valid
 * ISO-TP single or first frame PCI AND answers the request's service
 * (positive SID = request SID + 0x40, or negative 0x7F <request SID> <NRC>):
 * a negative response still proves an ECU is there, while a broadcast frame
 * that happens to use the response ID is not counted.
 * The request must fit a single frame (1..7 bytes) and should be read-only
 * (e.g. 3E 00 TesterPresent or 01 00 OBD supported PIDs).
 */
#include <stdbool.h>
#include <stdint.h>

#define ECU_SCAN_TESTER_ADDR   0xF1u
#define ECU_SCAN_DEFAULT_WAIT  60u     /* ms per address */

typedef bool (*ecu_scan_send_fn)(void *ctx, uint32_t id, bool ext,
                                 const uint8_t *data, uint8_t len);

/* true = do not send on this request ID (e.g. the ID is already used by
 * another node in the bus traffic). The target is skipped without a frame. */
typedef bool (*ecu_scan_skip_fn)(void *ctx, uint32_t id, bool ext);

typedef enum {
    ECU_SCAN_EV_NONE = 0,
    ECU_SCAN_EV_FOUND,      /* found_tx/found_rx/found_data valid */
    ECU_SCAN_EV_DONE,       /* scan finished; found_count valid   */
    ECU_SCAN_EV_SKIPPED     /* skipped_tx not sent (skip fn said so) */
} ecu_scan_event_t;

typedef struct {
    bool     active, ext, sent, pad, done_pending;
    uint8_t  pad_byte;
    uint32_t cur, last;               /* 11-bit request ID, or 29-bit TA   */
    uint8_t  req[7], req_len;
    uint32_t wait_ms, deadline_ms;
    bool     hit;                     /* reply seen for the current target */
    uint8_t  hit_data[8], hit_len;
    uint16_t found_count;
    /* last FOUND event */
    uint32_t found_tx, found_rx;
    uint8_t  found_data[8], found_len;
    /* last SKIPPED event */
    uint32_t skipped_tx;
    uint16_t skipped_count;
    ecu_scan_skip_fn skip;
    void    *skip_ctx;
} ecu_scan_t;

/* Request / response IDs for one target (cur) */
uint32_t ecu_scan_tx_id(bool ext, uint32_t target);
uint32_t ecu_scan_rx_id(bool ext, uint32_t target);

/* false when the arguments are invalid (range, request length). */
bool ecu_scan_begin(ecu_scan_t *s, bool ext, uint32_t first, uint32_t last,
                    const uint8_t *req, uint8_t len, uint32_t wait_ms,
                    bool pad, uint8_t pad_byte);
/* Optional; call after ecu_scan_begin (begin clears it). */
void ecu_scan_set_skip(ecu_scan_t *s, ecu_scan_skip_fn skip, void *ctx);
void ecu_scan_stop(ecu_scan_t *s);
bool ecu_scan_active(const ecu_scan_t *s);

void ecu_scan_on_frame(ecu_scan_t *s, uint32_t id, bool ext,
                       const uint8_t *data, uint8_t dlc);

/* Call often. Sends the next request when due; one event per call. */
ecu_scan_event_t ecu_scan_poll(ecu_scan_t *s, uint32_t now_ms,
                               ecu_scan_send_fn send, void *ctx);
#endif
