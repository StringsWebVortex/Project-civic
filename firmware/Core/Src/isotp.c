/*
 * isotp.c - ISO 15765-2 transport, normal addressing, classic CAN (8 bytes).
 * Behaviour reference: Linux kernel net/can/isotp.c (see isotp.h).
 */
#include "isotp.h"
#include <string.h>

#define PCI_SF 0x00u
#define PCI_FF 0x10u
#define PCI_CF 0x20u
#define PCI_FC 0x30u
#define FC_CTS   0u
#define FC_WAIT  1u
#define FC_OVFLW 2u

static bool due(uint32_t now, uint32_t t) { return (int32_t)(now - t) >= 0; }

static bool emit(isotp_link_t *l, const uint8_t *bytes, uint8_t n)
{
    uint8_t f[8];
    memcpy(f, bytes, n);
    uint8_t len = n;
    if (l->pad) { memset(f + n, l->pad_byte, (size_t)(8u - n)); len = 8u; }
    return l->send(l->ctx, l->tx_id, l->ext, f, len);
}

static void abort_tx(isotp_link_t *l, isotp_status_t e)
{
    l->tx_state = ISOTP_TX_IDLE;
    l->last_error = e;
    l->error_pending = true;
}

static void abort_rx(isotp_link_t *l, isotp_status_t e)
{
    l->rx_state = ISOTP_RX_IDLE;
    l->last_error = e;
    l->error_pending = true;
}

static uint32_t stmin_to_ms(uint8_t st)
{
    if (st >= 0xF1u && st <= 0xF9u) return 1u;        /* 100..900 us -> 1 ms (round up) */
    if (st > 0x7Fu) st = 0x7Fu;                       /* invalid -> 127 ms              */
    return st;
}

void isotp_reset(isotp_link_t *l)
{
    l->tx_state = ISOTP_TX_IDLE;
    l->rx_state = ISOTP_RX_IDLE;
    l->rx_ready = false;
    l->tx_done = false;
    l->error_pending = false;
    l->last_error = ISOTP_OK;
}

void isotp_init(isotp_link_t *l, uint32_t tx_id, uint32_t rx_id, bool ext,
                isotp_send_fn send, void *ctx)
{
    memset(l, 0, sizeof *l);
    l->tx_id = tx_id;
    l->rx_id = rx_id;
    l->ext = ext;
    l->send = send;
    l->ctx = ctx;
    l->pad = true;
    l->pad_byte = 0x00u;
    l->rx_bs = 0u;
    l->rx_stmin = 0u;
    isotp_reset(l);
}

void isotp_set_padding(isotp_link_t *l, bool pad, uint8_t pad_byte)
{
    l->pad = pad;
    l->pad_byte = pad_byte;
}

bool isotp_tx_busy(const isotp_link_t *l) { return l->tx_state != ISOTP_TX_IDLE; }

isotp_status_t isotp_send(isotp_link_t *l, const uint8_t *data, uint16_t len, uint32_t now_ms)
{
    if (isotp_tx_busy(l)) return ISOTP_ERR_BUSY;
    if (len == 0u || len > ISOTP_MAX_PAYLOAD || !data) return ISOTP_ERR_LENGTH;
    memcpy(l->tx_buf, data, len);
    l->tx_len = len;
    l->tx_pos = 0u;
    l->tx_done = false;
    l->tx_wait_count = 0u;
    l->tx_state = (len <= 7u) ? ISOTP_TX_SF : ISOTP_TX_FF;
    l->tx_deadline_ms = now_ms + ISOTP_TIMEOUT_MS;    /* frame must get a mailbox */
    isotp_poll(l, now_ms);
    return ISOTP_OK;
}

static void handle_fc(isotp_link_t *l, const uint8_t *d, uint8_t dlc, uint32_t now)
{
    if (l->tx_state != ISOTP_TX_WAIT_FC || dlc < 3u) return;
    switch (d[0] & 0x0Fu) {
    case FC_CTS:
        l->tx_bs = d[1];
        l->tx_bs_count = 0u;
        l->tx_wait_count = 0u;
        l->tx_gap_ms = stmin_to_ms(d[2]);
        l->tx_next_ms = now;
        l->tx_deadline_ms = now + ISOTP_TIMEOUT_MS;
        l->tx_state = ISOTP_TX_CF;
        isotp_poll(l, now);
        break;
    case FC_WAIT:
        if (++l->tx_wait_count > ISOTP_MAX_WAIT_FRAMES) abort_tx(l, ISOTP_ERR_WAIT_LIMIT);
        else l->tx_deadline_ms = now + ISOTP_TIMEOUT_MS;
        break;
    case FC_OVFLW:
        abort_tx(l, ISOTP_ERR_OVERFLOW);
        break;
    default:
        abort_tx(l, ISOTP_ERR_BAD_FC);
        break;
    }
}

void isotp_on_frame(isotp_link_t *l, uint32_t id, bool ext,
                    const uint8_t *d, uint8_t dlc, uint32_t now)
{
    if (id != l->rx_id || ext != l->ext || dlc == 0u || dlc > 8u) return;
    const uint8_t type = d[0] & 0xF0u;

    if (type == PCI_FC) { handle_fc(l, d, dlc, now); return; }

    if (type == PCI_SF) {
        const uint8_t n = d[0] & 0x0Fu;
        if (n == 0u || n > 7u || n > dlc - 1u) return;            /* invalid SF */
        memcpy(l->rx_buf, d + 1, n);
        l->rx_len = n;
        l->rx_state = ISOTP_RX_IDLE;
        l->rx_ready = true;
        return;
    }

    if (type == PCI_FF) {
        if (dlc < 8u) return;                                     /* FF must be full */
        const uint16_t n = (uint16_t)(((d[0] & 0x0Fu) << 8) | d[1]);
        if (n == 0u) {
            l->rx_state = ISOTP_RX_SEND_OVFLW;
            l->rx_deadline_ms = now + ISOTP_TIMEOUT_MS;
            isotp_poll(l, now);
            return;
        }
        if (n < 8u) return;                                       /* would fit in SF */
        memcpy(l->rx_buf, d + 2, 6u);
        l->rx_len = n;
        l->rx_pos = 6u;
        l->rx_sn = 1u;
        l->rx_bs_count = 0u;
        l->rx_ready = false;
        l->rx_state = ISOTP_RX_SEND_FC;
        l->rx_deadline_ms = now + ISOTP_TIMEOUT_MS;
        isotp_poll(l, now);
        return;
    }

    if (type == PCI_CF) {
        if (l->rx_state != ISOTP_RX_CF) return;
        if ((d[0] & 0x0Fu) != l->rx_sn) { abort_rx(l, ISOTP_ERR_WRONG_SN); return; }
        uint16_t n = (uint16_t)(l->rx_len - l->rx_pos);
        if (n > 7u) n = 7u;
        if (n > dlc - 1u) { abort_rx(l, ISOTP_ERR_WRONG_SN); return; }  /* short CF */
        memcpy(l->rx_buf + l->rx_pos, d + 1, n);
        l->rx_pos = (uint16_t)(l->rx_pos + n);
        l->rx_sn = (uint8_t)((l->rx_sn + 1u) & 0x0Fu);
        if (l->rx_pos >= l->rx_len) {
            l->rx_state = ISOTP_RX_IDLE;
            l->rx_ready = true;
            return;
        }
        l->rx_deadline_ms = now + ISOTP_TIMEOUT_MS;
        if (l->rx_bs != 0u && ++l->rx_bs_count >= l->rx_bs) {
            l->rx_bs_count = 0u;
            l->rx_state = ISOTP_RX_SEND_FC;
            isotp_poll(l, now);
        }
    }
}

static void poll_tx(isotp_link_t *l, uint32_t now)
{
    uint8_t f[8];
    switch (l->tx_state) {
    case ISOTP_TX_IDLE:
        return;
    case ISOTP_TX_SF:
        f[0] = (uint8_t)(PCI_SF | l->tx_len);
        memcpy(f + 1, l->tx_buf, l->tx_len);
        if (emit(l, f, (uint8_t)(1u + l->tx_len))) { l->tx_state = ISOTP_TX_IDLE; l->tx_done = true; }
        else if (due(now, l->tx_deadline_ms)) abort_tx(l, ISOTP_ERR_TIMEOUT_TX);
        return;
    case ISOTP_TX_FF:
        f[0] = (uint8_t)(PCI_FF | ((l->tx_len >> 8) & 0x0Fu));
        f[1] = (uint8_t)(l->tx_len & 0xFFu);
        memcpy(f + 2, l->tx_buf, 6u);
        if (emit(l, f, 8u)) {
            l->tx_pos = 6u;
            l->tx_sn = 1u;
            l->tx_state = ISOTP_TX_WAIT_FC;
            l->tx_deadline_ms = now + ISOTP_TIMEOUT_MS;
        } else if (due(now, l->tx_deadline_ms)) {
            abort_tx(l, ISOTP_ERR_TIMEOUT_TX);
        }
        return;
    case ISOTP_TX_WAIT_FC:
        if (due(now, l->tx_deadline_ms)) abort_tx(l, ISOTP_ERR_TIMEOUT_FC);
        return;
    case ISOTP_TX_CF:
        for (;;) {
            if (!due(now, l->tx_next_ms)) return;
            uint16_t n = (uint16_t)(l->tx_len - l->tx_pos);
            if (n > 7u) n = 7u;
            f[0] = (uint8_t)(PCI_CF | l->tx_sn);
            memcpy(f + 1, l->tx_buf + l->tx_pos, n);
            if (!emit(l, f, (uint8_t)(1u + n))) {                /* retry next poll */
                if (due(now, l->tx_deadline_ms)) abort_tx(l, ISOTP_ERR_TIMEOUT_TX);
                return;
            }
            l->tx_pos = (uint16_t)(l->tx_pos + n);
            l->tx_sn = (uint8_t)((l->tx_sn + 1u) & 0x0Fu);
            /* The ms clock can tick right after a send, so "now + gap" may come
             * after only gap-1 ms of real time. +1 keeps at least STmin. */
            l->tx_next_ms = now + l->tx_gap_ms + (l->tx_gap_ms != 0u ? 1u : 0u);
            l->tx_deadline_ms = l->tx_next_ms + ISOTP_TIMEOUT_MS;  /* next CF send window */
            if (l->tx_pos >= l->tx_len) { l->tx_state = ISOTP_TX_IDLE; l->tx_done = true; return; }
            if (l->tx_bs != 0u && ++l->tx_bs_count >= l->tx_bs) {
                l->tx_state = ISOTP_TX_WAIT_FC;
                l->tx_deadline_ms = now + ISOTP_TIMEOUT_MS;
                return;
            }
            if (l->tx_gap_ms != 0u) return;
        }
    }
}

static void poll_rx(isotp_link_t *l, uint32_t now)
{
    uint8_t f[3];
    switch (l->rx_state) {
    case ISOTP_RX_SEND_FC:
        f[0] = (uint8_t)(PCI_FC | FC_CTS); f[1] = l->rx_bs; f[2] = l->rx_stmin;
        if (emit(l, f, 3u)) { l->rx_state = ISOTP_RX_CF; l->rx_deadline_ms = now + ISOTP_TIMEOUT_MS; }
        else if (due(now, l->rx_deadline_ms)) abort_rx(l, ISOTP_ERR_TIMEOUT_TX);
        return;
    case ISOTP_RX_SEND_OVFLW:
        f[0] = (uint8_t)(PCI_FC | FC_OVFLW); f[1] = 0u; f[2] = 0u;
        if (emit(l, f, 3u) || due(now, l->rx_deadline_ms)) abort_rx(l, ISOTP_ERR_OVERFLOW);
        return;
    case ISOTP_RX_CF:
        if (due(now, l->rx_deadline_ms)) abort_rx(l, ISOTP_ERR_TIMEOUT_CF);
        return;
    case ISOTP_RX_IDLE:
        return;
    }
}

void isotp_poll(isotp_link_t *l, uint32_t now_ms)
{
    poll_rx(l, now_ms);
    poll_tx(l, now_ms);
}

bool isotp_take(isotp_link_t *l, uint8_t *out, uint16_t cap, uint16_t *len)
{
    if (!l->rx_ready) return false;
    const uint16_t n = (l->rx_len < cap) ? l->rx_len : cap;
    memcpy(out, l->rx_buf, n);
    *len = l->rx_len;
    l->rx_ready = false;
    return true;
}

bool isotp_take_error(isotp_link_t *l, isotp_status_t *err)
{
    if (!l->error_pending) return false;
    *err = l->last_error;
    l->error_pending = false;
    return true;
}

const char *isotp_status_name(isotp_status_t s)
{
    switch (s) {
    case ISOTP_OK:             return "OK";
    case ISOTP_ERR_BUSY:       return "BUSY";
    case ISOTP_ERR_LENGTH:     return "LENGTH";
    case ISOTP_ERR_TIMEOUT_FC: return "TIMEOUT_FC";
    case ISOTP_ERR_TIMEOUT_CF: return "TIMEOUT_CF";
    case ISOTP_ERR_WRONG_SN:   return "WRONG_SN";
    case ISOTP_ERR_OVERFLOW:   return "OVERFLOW";
    case ISOTP_ERR_WAIT_LIMIT: return "WAIT_LIMIT";
    case ISOTP_ERR_BAD_FC:     return "BAD_FC";
    case ISOTP_ERR_TIMEOUT_TX: return "TIMEOUT_TX";
    }
    return "UNKNOWN";
}
