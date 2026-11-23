#ifndef UDS_CLIENT_H
#define UDS_CLIENT_H
/*
 * uds_client.h - request/response client for ISO 14229 (UDS) and SAE J1979
 * (OBD-II modes) diagnostic services, on top of isotp.h. Own implementation.
 *
 * Service IDs and negative response codes match python-udsoncan
 * (github.com/pylessard/python-udsoncan, udsoncan/ResponseCode.py and
 * services/ package). Positive response = request SID + 0x40; negative response =
 * 0x7F, request SID, NRC. NRC 0x78 (RequestCorrectlyReceived_ResponsePending)
 * extends the wait to P2*. Default P2 = 1000 ms and P2* = 5000 ms, the same
 * client defaults as udsoncan configs.py. The ISO 14229 text is not public and
 * was not read.
 *
 * Safety: only read-type services are allowed without confirmation.
 * Everything else (DTC clear, ECU reset, routines, writes, security access,
 * communication control, programming, ...) needs confirm = true. Arming and
 * the operator confirmation itself are enforced by the caller.
 */
#include <stdbool.h>
#include <stdint.h>
#include "isotp.h"

#define UDS_P2_DEFAULT_MS        1000u
#define UDS_P2_STAR_DEFAULT_MS   5000u
#define UDS_MAX_PENDING          20u    /* NRC 0x78 repeats accepted per request */
#define UDS_NRC_RESPONSE_PENDING 0x78u
#define UDS_NEGATIVE_RESPONSE    0x7Fu

typedef enum {
    UDS_IDLE = 0,
    UDS_WAIT_SEND,          /* ISO-TP still sending the request          */
    UDS_WAIT_RESPONSE
} uds_state_t;

typedef enum {
    UDS_EV_NONE = 0,
    UDS_EV_POSITIVE,        /* resp/resp_len hold the full response     */
    UDS_EV_NEGATIVE,        /* nrc holds the code                       */
    UDS_EV_PENDING,         /* NRC 0x78 received, still waiting         */
    UDS_EV_SENT_NO_RESPONSE,/* suppressPosRspMsgIndicationBit was set   */
    UDS_EV_TIMEOUT,         /* P2 / P2* expired                         */
    UDS_EV_TRANSPORT_ERROR  /* ISO-TP aborted: see transport_error      */
} uds_event_t;

typedef enum {
    UDS_START_OK = 0,
    UDS_START_BUSY,
    UDS_START_EMPTY,
    UDS_START_TOO_LONG,
    UDS_START_CONFIRM_REQUIRED,
    UDS_START_TRANSPORT
} uds_start_t;

typedef struct {
    isotp_link_t   link;
    uds_state_t    state;
    uint8_t        req_sid;
    bool           suppress_response;
    uint32_t       p2_ms, p2_star_ms;
    uint32_t       deadline_ms;
    uint32_t       pending_count;
    uint8_t        resp[ISOTP_MAX_PAYLOAD];
    uint16_t       resp_len;
    uint8_t        nrc;
    isotp_status_t transport_error;
} uds_client_t;

void uds_init(uds_client_t *c, uint32_t tx_id, uint32_t rx_id, bool ext,
              isotp_send_fn send, void *ctx);

/* true when the request needs an explicit operator confirmation. */
bool uds_request_needs_confirm(const uint8_t *req, uint16_t len);

/* true when byte 1 carries suppressPosRspMsgIndicationBit for this SID. */
bool uds_request_suppresses_response(const uint8_t *req, uint16_t len);

uds_start_t uds_start(uds_client_t *c, const uint8_t *req, uint16_t len,
                      bool confirm, uint32_t now_ms);

void uds_on_frame(uds_client_t *c, uint32_t id, bool ext,
                  const uint8_t *data, uint8_t dlc, uint32_t now_ms);

/* Call often. Returns one event per call (UDS_EV_NONE when nothing new). */
uds_event_t uds_poll(uds_client_t *c, uint32_t now_ms);

void uds_abort(uds_client_t *c);
bool uds_busy(const uds_client_t *c);
#endif
