/*
 * ecu_scan.c - diagnostic controller discovery (see ecu_scan.h).
 */
#include "ecu_scan.h"
#include <string.h>

static bool due(uint32_t now, uint32_t t) { return (int32_t)(now - t) >= 0; }

uint32_t ecu_scan_tx_id(bool ext, uint32_t target)
{
    return ext ? (0x18DA0000u | ((target & 0xFFu) << 8) | ECU_SCAN_TESTER_ADDR)
               : (target & 0x7FFu);
}

uint32_t ecu_scan_rx_id(bool ext, uint32_t target)
{
    return ext ? (0x18DA0000u | (ECU_SCAN_TESTER_ADDR << 8) | (target & 0xFFu))
               : ((target + 8u) & 0x7FFu);
}

bool ecu_scan_begin(ecu_scan_t *s, bool ext, uint32_t first, uint32_t last,
                    const uint8_t *req, uint8_t len, uint32_t wait_ms,
                    bool pad, uint8_t pad_byte)
{
    const uint32_t max = ext ? 0xFFu : 0x7F7u;   /* +8 must stay 11-bit */
    if (!req || len == 0u || len > 7u || first > last || last > max) return false;
    if (wait_ms == 0u || wait_ms > 1000u) return false;
    memset(s, 0, sizeof *s);
    s->ext = ext;
    s->cur = first;
    s->last = last;
    memcpy(s->req, req, len);
    s->req_len = len;
    s->wait_ms = wait_ms;
    s->pad = pad;
    s->pad_byte = pad_byte;
    s->active = true;
    return true;
}

void ecu_scan_set_skip(ecu_scan_t *s, ecu_scan_skip_fn skip, void *ctx)
{
    s->skip = skip;
    s->skip_ctx = ctx;
}

void ecu_scan_stop(ecu_scan_t *s) { s->active = false; }
bool ecu_scan_active(const ecu_scan_t *s) { return s->active; }

void ecu_scan_on_frame(ecu_scan_t *s, uint32_t id, bool ext,
                       const uint8_t *d, uint8_t dlc)
{
    if (!s->active || !s->sent || s->hit || ext != s->ext) return;
    if (id != ecu_scan_rx_id(s->ext, s->cur) || dlc == 0u || dlc > 8u) return;
    /* The reply must answer the request's service: positive response SID =
     * request SID + 0x40, negative response = 0x7F, request SID, NRC (the
     * same rule uds_client.c applies). A periodic broadcast frame that only
     * happens to sit on the response ID is therefore not counted. */
    const uint8_t pos_sid = (uint8_t)(s->req[0] + 0x40u);
    const uint8_t type = d[0] & 0xF0u;
    if (type == 0x00u) {                         /* single frame */
        const uint8_t n = d[0] & 0x0Fu;
        if (n == 0u || n > dlc - 1u) return;
        const bool pos = d[1] == pos_sid;
        const bool neg = n >= 3u && d[1] == 0x7Fu && d[2] == s->req[0];
        if (!pos && !neg) return;
    } else if (type == 0x10u) {                  /* first frame  */
        if (dlc < 8u || d[2] != pos_sid) return; /* negative replies are SF */
    } else {
        return;                                  /* CF/FC: not an answer */
    }
    memcpy(s->hit_data, d, dlc);
    s->hit_len = dlc;
    s->hit = true;
}

static bool send_request(ecu_scan_t *s, ecu_scan_send_fn send, void *ctx)
{
    uint8_t f[8];
    uint8_t len = (uint8_t)(1u + s->req_len);
    f[0] = s->req_len;                           /* SF PCI */
    memcpy(f + 1, s->req, s->req_len);
    if (s->pad) { memset(f + len, s->pad_byte, (size_t)(8u - len)); len = 8u; }
    return send(ctx, ecu_scan_tx_id(s->ext, s->cur), s->ext, f, len);
}

ecu_scan_event_t ecu_scan_poll(ecu_scan_t *s, uint32_t now_ms,
                               ecu_scan_send_fn send, void *ctx)
{
    if (!s->active) return ECU_SCAN_EV_NONE;
    if (s->done_pending) { s->active = false; s->done_pending = false; return ECU_SCAN_EV_DONE; }

    ecu_scan_event_t ev = ECU_SCAN_EV_NONE;
    if (!s->sent && s->skip && s->skip(s->skip_ctx, ecu_scan_tx_id(s->ext, s->cur), s->ext)) {
        s->skipped_tx = ecu_scan_tx_id(s->ext, s->cur);
        s->skipped_count++;
        ev = ECU_SCAN_EV_SKIPPED;
        goto next_target;
    }
    if (!s->sent) {
        if (!send_request(s, send, ctx)) return ECU_SCAN_EV_NONE;  /* retry later */
        s->sent = true;
        s->hit = false;
        s->deadline_ms = now_ms + s->wait_ms;
        return ECU_SCAN_EV_NONE;
    }

    if (s->hit) {
        s->found_tx = ecu_scan_tx_id(s->ext, s->cur);
        s->found_rx = ecu_scan_rx_id(s->ext, s->cur);
        memcpy(s->found_data, s->hit_data, s->hit_len);
        s->found_len = s->hit_len;
        s->found_count++;
        ev = ECU_SCAN_EV_FOUND;
    } else if (!due(now_ms, s->deadline_ms)) {
        return ECU_SCAN_EV_NONE;
    }

next_target:
    s->sent = false;
    s->hit = false;
    if (s->cur >= s->last) {
        if (ev != ECU_SCAN_EV_NONE) { s->done_pending = true; return ev; }
        s->active = false;
        return ECU_SCAN_EV_DONE;
    }
    s->cur++;
    return ev;
}
