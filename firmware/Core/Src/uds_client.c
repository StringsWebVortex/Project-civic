/*
 * uds_client.c - diagnostic request/response client on ISO-TP (see uds_client.h).
 */
#include "uds_client.h"
#include <string.h>

static bool due(uint32_t now, uint32_t t) { return (int32_t)(now - t) >= 0; }

void uds_init(uds_client_t *c, uint32_t tx_id, uint32_t rx_id, bool ext,
              isotp_send_fn send, void *ctx)
{
    memset(c, 0, sizeof *c);
    isotp_init(&c->link, tx_id, rx_id, ext, send, ctx);
    c->p2_ms = UDS_P2_DEFAULT_MS;
    c->p2_star_ms = UDS_P2_STAR_DEFAULT_MS;
    c->state = UDS_IDLE;
}

/* Services that carry a sub-function byte (bit 7 = suppressPosRspMsgIndicationBit):
 * 0x10 DiagnosticSessionControl, 0x11 ECUReset, 0x28 CommunicationControl,
 * 0x31 RoutineControl, 0x3E TesterPresent, 0x85 ControlDTCSetting. */
static bool sid_has_subfunction(uint8_t sid)
{
    return sid == 0x10u || sid == 0x11u || sid == 0x28u ||
           sid == 0x31u || sid == 0x3Eu || sid == 0x85u;
}

bool uds_request_suppresses_response(const uint8_t *req, uint16_t len)
{
    return len >= 2u && sid_has_subfunction(req[0]) && (req[1] & 0x80u) != 0u;
}

bool uds_request_needs_confirm(const uint8_t *req, uint16_t len)
{
    if (len == 0u) return true;
    switch (req[0]) {
    /* SAE J1979 read modes */
    case 0x01u: case 0x02u: case 0x03u: case 0x06u:
    case 0x07u: case 0x09u: case 0x0Au:
    /* UDS read services */
    case 0x19u:            /* ReadDTCInformation      */
    case 0x22u:            /* ReadDataByIdentifier    */
    case 0x3Eu:            /* TesterPresent           */
        return false;
    case 0x10u: {          /* only default (01) and extended (03) sessions */
        if (len < 2u) return true;
        const uint8_t sub = (uint8_t)(req[1] & 0x7Fu);
        return !(sub == 0x01u || sub == 0x03u);
    }
    default:               /* 0x04 clear DTC, 0x11 reset, 0x14, 0x27, 0x2E, 0x31, ... */
        return true;
    }
}

bool uds_busy(const uds_client_t *c) { return c->state != UDS_IDLE; }

void uds_abort(uds_client_t *c)
{
    isotp_reset(&c->link);
    c->state = UDS_IDLE;
}

uds_start_t uds_start(uds_client_t *c, const uint8_t *req, uint16_t len,
                      bool confirm, uint32_t now_ms)
{
    if (uds_busy(c)) return UDS_START_BUSY;
    if (!req || len == 0u) return UDS_START_EMPTY;
    if (len > ISOTP_MAX_PAYLOAD) return UDS_START_TOO_LONG;
    if (uds_request_needs_confirm(req, len) && !confirm) return UDS_START_CONFIRM_REQUIRED;

    isotp_status_t e;
    while (isotp_take_error(&c->link, &e)) { }        /* drop stale transport errors */
    uint8_t scratch[1]; uint16_t sl;
    while (isotp_take(&c->link, scratch, sizeof scratch, &sl)) { } /* drop stale rx */

    if (isotp_send(&c->link, req, len, now_ms) != ISOTP_OK) return UDS_START_TRANSPORT;
    c->req_sid = req[0];
    c->suppress_response = uds_request_suppresses_response(req, len);
    c->pending_count = 0u;
    c->resp_len = 0u;
    c->nrc = 0u;
    c->state = UDS_WAIT_SEND;
    return UDS_START_OK;
}

void uds_on_frame(uds_client_t *c, uint32_t id, bool ext,
                  const uint8_t *data, uint8_t dlc, uint32_t now_ms)
{
    isotp_on_frame(&c->link, id, ext, data, dlc, now_ms);
}

uds_event_t uds_poll(uds_client_t *c, uint32_t now_ms)
{
    isotp_poll(&c->link, now_ms);

    isotp_status_t terr;
    if (isotp_take_error(&c->link, &terr)) {
        if (c->state != UDS_IDLE) {
            c->transport_error = terr;
            uds_abort(c);
            return UDS_EV_TRANSPORT_ERROR;
        }
    }

    switch (c->state) {
    case UDS_IDLE:
    default:
        return UDS_EV_NONE;

    case UDS_WAIT_SEND:
        if (!c->link.tx_done) return UDS_EV_NONE;
        c->state = UDS_WAIT_RESPONSE;
        c->deadline_ms = now_ms + c->p2_ms;
        return UDS_EV_NONE;

    case UDS_WAIT_RESPONSE: {
        uint16_t len = 0u;
        while (isotp_take(&c->link, c->resp, sizeof c->resp, &len)) {
            if (len >= 3u && c->resp[0] == UDS_NEGATIVE_RESPONSE && c->resp[1] == c->req_sid) {
                if (c->resp[2] == UDS_NRC_RESPONSE_PENDING) {
                    if (++c->pending_count > UDS_MAX_PENDING) {
                        c->state = UDS_IDLE;
                        return UDS_EV_TIMEOUT;
                    }
                    c->deadline_ms = now_ms + c->p2_star_ms;
                    return UDS_EV_PENDING;
                }
                c->nrc = c->resp[2];
                c->state = UDS_IDLE;
                return UDS_EV_NEGATIVE;
            }
            if (len >= 1u && c->resp[0] == (uint8_t)(c->req_sid + 0x40u)) {
                c->resp_len = len;
                c->state = UDS_IDLE;
                return UDS_EV_POSITIVE;
            }
            /* anything else (unrelated response) is ignored */
        }
        if (due(now_ms, c->deadline_ms)) {
            c->state = UDS_IDLE;
            return c->suppress_response && c->pending_count == 0u
                       ? UDS_EV_SENT_NO_RESPONSE : UDS_EV_TIMEOUT;
        }
        return UDS_EV_NONE;
    }
    }
}
