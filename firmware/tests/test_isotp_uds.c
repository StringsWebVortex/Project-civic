/* Host tests for isotp.c and uds_client.c. Prints one FAIL line and exits 1
 * on the first failed check, so the recorder captures the exact reason. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "isotp.h"
#include "uds_client.h"

#define CHECK(cond) do { if (!(cond)) { \
    printf("FAIL %s:%d in %s: %s\n", __FILE__, __LINE__, __func__, #cond); exit(1); } } while (0)

/* ── mock CAN bus ───────────────────────────────────────────────── */
typedef struct { uint32_t id; bool ext; uint8_t d[8]; uint8_t len; } frame_t;
static frame_t sent[700];
static unsigned nsent;
static bool bus_busy;

static bool mock_send(void *ctx, uint32_t id, bool ext, const uint8_t *d, uint8_t len)
{
    (void)ctx;
    if (bus_busy) return false;
    CHECK(nsent < sizeof sent / sizeof sent[0]);
    CHECK(len <= 8u);
    sent[nsent].id = id; sent[nsent].ext = ext; sent[nsent].len = len;
    memcpy(sent[nsent].d, d, len);
    nsent++;
    return true;
}

static void reset_bus(void) { nsent = 0; bus_busy = false; memset(sent, 0, sizeof sent); }

static isotp_link_t L;    /* large: keep off the stack */
static uds_client_t U;

#define TX 0x7E0u
#define RX 0x7E8u

static void fc(isotp_link_t *l, uint8_t fs, uint8_t bs, uint8_t st, uint32_t now)
{
    const uint8_t f[8] = { (uint8_t)(0x30u | fs), bs, st, 0, 0, 0, 0, 0 };
    isotp_on_frame(l, RX, false, f, 8, now);
}

static void test_single_frame_padded_and_unpadded(void)
{
    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    const uint8_t p[3] = { 0x22, 0xF1, 0x90 };
    CHECK(isotp_send(&L, p, 3, 0) == ISOTP_OK);
    CHECK(nsent == 1 && sent[0].id == TX && sent[0].len == 8);
    CHECK(sent[0].d[0] == 0x03 && sent[0].d[1] == 0x22 && sent[0].d[3] == 0x90 && sent[0].d[7] == 0x00);
    CHECK(L.tx_done && !isotp_tx_busy(&L));

    reset_bus();
    isotp_set_padding(&L, false, 0);
    CHECK(isotp_send(&L, p, 3, 0) == ISOTP_OK);
    CHECK(nsent == 1 && sent[0].len == 4);

    reset_bus();
    isotp_set_padding(&L, true, 0xAA);
    CHECK(isotp_send(&L, p, 3, 0) == ISOTP_OK);
    CHECK(sent[0].d[4] == 0xAA && sent[0].d[7] == 0xAA);
}

static void test_length_limits(void)
{
    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    static uint8_t big[ISOTP_MAX_PAYLOAD + 1];
    CHECK(isotp_send(&L, big, 0, 0) == ISOTP_ERR_LENGTH);
    CHECK(isotp_send(&L, big, ISOTP_MAX_PAYLOAD + 1, 0) == ISOTP_ERR_LENGTH);
    CHECK(nsent == 0);
}

static void test_multi_frame_send_bs0(void)
{
    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    uint8_t p[20];
    for (unsigned i = 0; i < sizeof p; i++) p[i] = (uint8_t)i;
    CHECK(isotp_send(&L, p, 20, 0) == ISOTP_OK);
    CHECK(nsent == 1 && sent[0].d[0] == 0x10 && sent[0].d[1] == 20 && sent[0].d[7] == 5);
    CHECK(isotp_send(&L, p, 3, 0) == ISOTP_ERR_BUSY);
    fc(&L, 0, 0, 0, 5);
    CHECK(nsent == 3);
    CHECK(sent[1].d[0] == 0x21 && sent[1].d[1] == 6 && sent[1].d[7] == 12);
    CHECK(sent[2].d[0] == 0x22 && sent[2].d[1] == 13 && sent[2].d[7] == 19);
    CHECK(L.tx_done && !isotp_tx_busy(&L));
}

static void test_block_size_and_stmin(void)
{
    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    uint8_t p[20] = {0};
    CHECK(isotp_send(&L, p, 20, 0) == ISOTP_OK);
    fc(&L, 0, 1, 10, 100);                 /* BS 1, STmin 10 ms */
    CHECK(nsent == 2);                     /* FF + one CF, then wait for FC */
    isotp_poll(&L, 200);
    CHECK(nsent == 2);
    fc(&L, 0, 1, 10, 200);
    CHECK(nsent == 3 && L.tx_done);

    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    uint8_t q[30] = {0};                   /* FF + 4 CFs */
    CHECK(isotp_send(&L, q, 30, 0) == ISOTP_OK);
    fc(&L, 0, 0, 10, 0);
    CHECK(nsent == 2);
    isotp_poll(&L, 10);
    CHECK(nsent == 2);                     /* STmin + 1 tick not yet elapsed */
    isotp_poll(&L, 11);
    CHECK(nsent == 3);                     /* >= 10 ms real time guaranteed */
    isotp_poll(&L, 21);
    CHECK(nsent == 3);
    isotp_poll(&L, 22); isotp_poll(&L, 33);
    CHECK(nsent == 5 && L.tx_done);

    /* 0xF5 (500 us) is rounded up to 1 ms; 0x80 (invalid) becomes 127 ms */
    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    CHECK(isotp_send(&L, q, 30, 0) == ISOTP_OK);
    fc(&L, 0, 0, 0xF5, 0);
    CHECK(L.tx_gap_ms == 1u);
    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    CHECK(isotp_send(&L, q, 30, 0) == ISOTP_OK);
    fc(&L, 0, 0, 0x80, 0);
    CHECK(L.tx_gap_ms == 127u);
}

static void test_fc_timeout_wait_overflow(void)
{
    isotp_status_t e;
    uint8_t p[20] = {0};

    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    CHECK(isotp_send(&L, p, 20, 0) == ISOTP_OK);
    isotp_poll(&L, 999);
    CHECK(!isotp_take_error(&L, &e));
    isotp_poll(&L, 1000);
    CHECK(isotp_take_error(&L, &e) && e == ISOTP_ERR_TIMEOUT_FC && !isotp_tx_busy(&L));

    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    CHECK(isotp_send(&L, p, 20, 0) == ISOTP_OK);
    for (uint32_t i = 1; i <= ISOTP_MAX_WAIT_FRAMES; i++) fc(&L, 1, 0, 0, i * 500u);
    CHECK(!isotp_take_error(&L, &e) && isotp_tx_busy(&L));     /* each WAIT restarts timer */
    fc(&L, 1, 0, 0, 6000);
    CHECK(isotp_take_error(&L, &e) && e == ISOTP_ERR_WAIT_LIMIT);

    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    CHECK(isotp_send(&L, p, 20, 0) == ISOTP_OK);
    fc(&L, 2, 0, 0, 1);
    CHECK(isotp_take_error(&L, &e) && e == ISOTP_ERR_OVERFLOW);

    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    CHECK(isotp_send(&L, p, 20, 0) == ISOTP_OK);
    fc(&L, 7, 0, 0, 1);
    CHECK(isotp_take_error(&L, &e) && e == ISOTP_ERR_BAD_FC);
}

static void test_busy_bus_retries(void)
{
    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    const uint8_t p[2] = { 0x3E, 0x00 };
    bus_busy = true;
    CHECK(isotp_send(&L, p, 2, 0) == ISOTP_OK);
    CHECK(nsent == 0 && isotp_tx_busy(&L));
    bus_busy = false;
    isotp_poll(&L, 1);
    CHECK(nsent == 1 && L.tx_done);
}

/* A frame that never gets a mailbox ends the transfer with TIMEOUT_TX after
 * ISOTP_TIMEOUT_MS (single frame, first frame and consecutive frame), and a
 * UDS request reports it instead of waiting forever. */
static void test_send_timeout(void)
{
    isotp_status_t e;
    const uint8_t sfp[2] = { 0x3E, 0x00 };
    uint8_t big[20]; memset(big, 0x11, sizeof big);

    reset_bus(); bus_busy = true;
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    CHECK(isotp_send(&L, sfp, 2, 100) == ISOTP_OK);
    isotp_poll(&L, 100 + ISOTP_TIMEOUT_MS - 1u);
    CHECK(isotp_tx_busy(&L) && !isotp_take_error(&L, &e));
    isotp_poll(&L, 100 + ISOTP_TIMEOUT_MS);
    CHECK(!isotp_tx_busy(&L) && isotp_take_error(&L, &e) && e == ISOTP_ERR_TIMEOUT_TX);

    reset_bus(); bus_busy = true;
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    CHECK(isotp_send(&L, big, sizeof big, 0) == ISOTP_OK);
    isotp_poll(&L, ISOTP_TIMEOUT_MS);
    CHECK(isotp_take_error(&L, &e) && e == ISOTP_ERR_TIMEOUT_TX);

    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    CHECK(isotp_send(&L, big, sizeof big, 0) == ISOTP_OK);   /* FF out */
    CHECK(nsent == 1);
    bus_busy = true;
    fc(&L, 0, 0, 0, 10);                                     /* CTS, CF blocked */
    isotp_poll(&L, 10 + ISOTP_TIMEOUT_MS - 1u);
    CHECK(isotp_tx_busy(&L));
    isotp_poll(&L, 10 + ISOTP_TIMEOUT_MS);
    CHECK(!isotp_tx_busy(&L) && isotp_take_error(&L, &e) && e == ISOTP_ERR_TIMEOUT_TX);

    reset_bus(); bus_busy = true;
    uds_init(&U, TX, RX, false, mock_send, NULL);
    CHECK(uds_start(&U, sfp, 2, false, 0) == UDS_START_OK);
    uds_event_t ev = UDS_EV_NONE;
    for (uint32_t t = 1; t <= ISOTP_TIMEOUT_MS + 5u && ev == UDS_EV_NONE; t++) ev = uds_poll(&U, t);
    CHECK(ev == UDS_EV_TRANSPORT_ERROR && U.transport_error == ISOTP_ERR_TIMEOUT_TX && !uds_busy(&U));
    CHECK(strcmp(isotp_status_name(ISOTP_ERR_TIMEOUT_TX), "TIMEOUT_TX") == 0);
    bus_busy = false;
}

static void test_sequence_number_wrap_and_max_length(void)
{
    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    static uint8_t p[ISOTP_MAX_PAYLOAD];
    for (unsigned i = 0; i < sizeof p; i++) p[i] = (uint8_t)(i * 7u);
    CHECK(isotp_send(&L, p, ISOTP_MAX_PAYLOAD, 0) == ISOTP_OK);
    CHECK(sent[0].d[0] == 0x1F && sent[0].d[1] == 0xFF);
    fc(&L, 0, 0, 0, 1);
    const unsigned cfs = (ISOTP_MAX_PAYLOAD - 6u + 6u) / 7u;     /* ceil((4095-6)/7) = 585 */
    CHECK(nsent == 1u + cfs && L.tx_done);
    for (unsigned i = 0; i < cfs; i++) CHECK((sent[1 + i].d[0] & 0x0F) == ((i + 1u) & 0x0Fu));
    /* last CF holds the remaining (4095-6) % 7 = 1 byte, then padding */
    CHECK(sent[cfs].d[1] == p[ISOTP_MAX_PAYLOAD - 1] && sent[cfs].d[2] == 0x00);
}

static void rx_frame(isotp_link_t *l, const uint8_t *f, uint8_t dlc, uint32_t now)
{
    isotp_on_frame(l, RX, false, f, dlc, now);
}

static void test_receive_single_and_multi(void)
{
    uint8_t out[64]; uint16_t n = 0; isotp_status_t e;

    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    const uint8_t sf[8] = { 0x03, 0x62, 0xF1, 0x90, 0, 0, 0, 0 };
    rx_frame(&L, sf, 8, 0);
    CHECK(isotp_take(&L, out, sizeof out, &n) && n == 3 && out[0] == 0x62);
    CHECK(!isotp_take(&L, out, sizeof out, &n));

    /* other ID, extended mismatch, invalid SF length: ignored */
    isotp_on_frame(&L, 0x7E9, false, sf, 8, 0);
    isotp_on_frame(&L, RX, true, sf, 8, 0);
    const uint8_t badsf[2] = { 0x05, 0x62 };
    rx_frame(&L, badsf, 2, 0);
    CHECK(!isotp_take(&L, out, sizeof out, &n));

    /* 20-byte message: FF + 2 CF, we answer FC CTS BS 0 STmin 0 */
    const uint8_t ff[8]  = { 0x10, 20, 0, 1, 2, 3, 4, 5 };
    const uint8_t cf1[8] = { 0x21, 6, 7, 8, 9, 10, 11, 12 };
    const uint8_t cf2[8] = { 0x22, 13, 14, 15, 16, 17, 18, 19 };
    rx_frame(&L, ff, 8, 10);
    CHECK(nsent == 1 && sent[0].id == TX && sent[0].d[0] == 0x30 && sent[0].d[1] == 0 && sent[0].d[2] == 0);
    rx_frame(&L, cf1, 8, 11);
    CHECK(!isotp_take(&L, out, sizeof out, &n));
    rx_frame(&L, cf2, 8, 12);
    CHECK(isotp_take(&L, out, sizeof out, &n) && n == 20);
    for (unsigned i = 0; i < 20; i++) CHECK(out[i] == i);

    /* wrong sequence number */
    rx_frame(&L, ff, 8, 20);
    rx_frame(&L, cf2, 8, 21);
    CHECK(isotp_take_error(&L, &e) && e == ISOTP_ERR_WRONG_SN);

    /* consecutive frame timeout */
    rx_frame(&L, ff, 8, 30);
    isotp_poll(&L, 1029);
    CHECK(!isotp_take_error(&L, &e));
    isotp_poll(&L, 1030);
    CHECK(isotp_take_error(&L, &e) && e == ISOTP_ERR_TIMEOUT_CF);

    /* FF_DL = 0 escape (> 4095 bytes): answered with FC OVERFLOW */
    reset_bus();
    const uint8_t ffx[8] = { 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0, 0 };
    rx_frame(&L, ffx, 8, 40);
    CHECK(nsent == 1 && sent[0].d[0] == 0x32);
    CHECK(isotp_take_error(&L, &e) && e == ISOTP_ERR_OVERFLOW);

    /* stray CF while idle is ignored */
    rx_frame(&L, cf1, 8, 50);
    CHECK(!isotp_take(&L, out, sizeof out, &n) && !isotp_take_error(&L, &e));
}

static void test_receive_block_size(void)
{
    uint8_t out[64]; uint16_t n = 0;
    reset_bus();
    isotp_init(&L, TX, RX, false, mock_send, NULL);
    L.rx_bs = 1;
    const uint8_t ff[8]  = { 0x10, 20, 0, 1, 2, 3, 4, 5 };
    const uint8_t cf1[8] = { 0x21, 6, 7, 8, 9, 10, 11, 12 };
    const uint8_t cf2[8] = { 0x22, 13, 14, 15, 16, 17, 18, 19 };
    rx_frame(&L, ff, 8, 0);
    rx_frame(&L, cf1, 8, 1);
    CHECK(nsent == 2 && sent[1].d[0] == 0x30 && sent[1].d[1] == 1);   /* FC after each block */
    rx_frame(&L, cf2, 8, 2);
    CHECK(isotp_take(&L, out, sizeof out, &n) && n == 20);
}

static void test_extended_ids(void)
{
    uint8_t out[8]; uint16_t n;
    reset_bus();
    isotp_init(&L, 0x18DA10F1u, 0x18DAF110u, true, mock_send, NULL);
    const uint8_t p[2] = { 0x3E, 0x00 };
    CHECK(isotp_send(&L, p, 2, 0) == ISOTP_OK);
    CHECK(sent[0].id == 0x18DA10F1u && sent[0].ext);
    const uint8_t sf[8] = { 0x02, 0x7E, 0x00, 0, 0, 0, 0, 0 };
    isotp_on_frame(&L, 0x18DAF110u, true, sf, 8, 0);
    CHECK(isotp_take(&L, out, sizeof out, &n) && n == 2 && out[0] == 0x7E);
}

/* ── UDS client ─────────────────────────────────────────────────── */
static void uds_rx(const uint8_t *f, uint32_t now) { uds_on_frame(&U, RX, false, f, 8, now); }

static void test_confirm_rules(void)
{
    const uint8_t r22[3] = { 0x22, 0xF1, 0x90 }, r19[3] = { 0x19, 0x02, 0xFF };
    const uint8_t r14[4] = { 0x14, 0xFF, 0xFF, 0xFF }, r04[1] = { 0x04 };
    const uint8_t r11[2] = { 0x11, 0x01 }, r31[4] = { 0x31, 0x01, 0x02, 0x03 };
    const uint8_t r1001[2] = { 0x10, 0x01 }, r1003[2] = { 0x10, 0x83 }, r1002[2] = { 0x10, 0x02 };
    const uint8_t r27[2] = { 0x27, 0x01 }, r2e[4] = { 0x2E, 0xF1, 0x90, 0x00 };
    const uint8_t r01[2] = { 0x01, 0x0C }, r09[2] = { 0x09, 0x02 }, r3e[2] = { 0x3E, 0x80 };
    const uint8_t r10[1] = { 0x10 };
    CHECK(!uds_request_needs_confirm(r22, 3) && !uds_request_needs_confirm(r19, 3));
    CHECK(!uds_request_needs_confirm(r01, 2) && !uds_request_needs_confirm(r09, 2));
    CHECK(!uds_request_needs_confirm(r3e, 2));
    CHECK(!uds_request_needs_confirm(r1001, 2) && !uds_request_needs_confirm(r1003, 2));
    CHECK(uds_request_needs_confirm(r1002, 2) && uds_request_needs_confirm(r10, 1));
    CHECK(uds_request_needs_confirm(r14, 4) && uds_request_needs_confirm(r04, 1));
    CHECK(uds_request_needs_confirm(r11, 2) && uds_request_needs_confirm(r31, 4));
    CHECK(uds_request_needs_confirm(r27, 2) && uds_request_needs_confirm(r2e, 4));
    CHECK(uds_request_suppresses_response(r3e, 2) && uds_request_suppresses_response(r1003, 2));
    CHECK(!uds_request_suppresses_response(r22, 3) && !uds_request_suppresses_response(r11, 2));
}

static void test_uds_positive_negative_pending_timeout(void)
{
    const uint8_t req[3] = { 0x22, 0xF1, 0x90 };

    reset_bus();
    uds_init(&U, TX, RX, false, mock_send, NULL);
    const uint8_t clr[4] = { 0x14, 0xFF, 0xFF, 0xFF };
    CHECK(uds_start(&U, clr, 4, false, 0) == UDS_START_CONFIRM_REQUIRED && nsent == 0);
    CHECK(uds_start(&U, req, 0, false, 0) == UDS_START_EMPTY);

    CHECK(uds_start(&U, req, 3, false, 0) == UDS_START_OK);
    CHECK(nsent == 1 && sent[0].d[0] == 0x03 && sent[0].d[1] == 0x22);
    CHECK(uds_start(&U, req, 3, false, 0) == UDS_START_BUSY);
    CHECK(uds_poll(&U, 1) == UDS_EV_NONE);
    const uint8_t other[8] = { 0x03, 0x7F, 0x10, 0x31, 0, 0, 0, 0 };   /* not ours */
    uds_rx(other, 2);
    CHECK(uds_poll(&U, 2) == UDS_EV_NONE);
    const uint8_t pos[8] = { 0x04, 0x62, 0xF1, 0x90, 0x41, 0, 0, 0 };
    uds_rx(pos, 3);
    CHECK(uds_poll(&U, 3) == UDS_EV_POSITIVE && U.resp_len == 4 && U.resp[3] == 0x41);
    CHECK(!uds_busy(&U));

    CHECK(uds_start(&U, req, 3, false, 10) == UDS_START_OK);
    CHECK(uds_poll(&U, 11) == UDS_EV_NONE);
    const uint8_t neg[8] = { 0x03, 0x7F, 0x22, 0x31, 0, 0, 0, 0 };
    uds_rx(neg, 12);
    CHECK(uds_poll(&U, 12) == UDS_EV_NEGATIVE && U.nrc == 0x31);

    CHECK(uds_start(&U, clr, 4, true, 100) == UDS_START_OK);
    CHECK(uds_poll(&U, 100) == UDS_EV_NONE);
    const uint8_t pend[8] = { 0x03, 0x7F, 0x14, 0x78, 0, 0, 0, 0 };
    uds_rx(pend, 200);
    CHECK(uds_poll(&U, 200) == UDS_EV_PENDING);
    CHECK(uds_poll(&U, 1500) == UDS_EV_NONE);         /* P2* = 5000 ms now applies */
    const uint8_t pos14[8] = { 0x01, 0x54, 0, 0, 0, 0, 0, 0 };
    uds_rx(pos14, 4000);
    CHECK(uds_poll(&U, 4000) == UDS_EV_POSITIVE && U.resp_len == 1);

    CHECK(uds_start(&U, req, 3, false, 5000) == UDS_START_OK);
    CHECK(uds_poll(&U, 5000) == UDS_EV_NONE);
    CHECK(uds_poll(&U, 5999) == UDS_EV_NONE);
    CHECK(uds_poll(&U, 6000) == UDS_EV_TIMEOUT && !uds_busy(&U));
}

static void test_uds_suppressed_and_multiframe(void)
{
    reset_bus();
    uds_init(&U, TX, RX, false, mock_send, NULL);
    const uint8_t tp[2] = { 0x3E, 0x80 };
    CHECK(uds_start(&U, tp, 2, false, 0) == UDS_START_OK);
    CHECK(uds_poll(&U, 0) == UDS_EV_NONE);
    CHECK(uds_poll(&U, 1000) == UDS_EV_SENT_NO_RESPONSE);

    /* VIN-style 20-byte response over FF + CF */
    reset_bus();
    const uint8_t req[3] = { 0x22, 0xF1, 0x90 };
    CHECK(uds_start(&U, req, 3, false, 0) == UDS_START_OK);
    CHECK(uds_poll(&U, 0) == UDS_EV_NONE);
    const uint8_t ff[8]  = { 0x10, 20, 0x62, 0xF1, 0x90, 'A', 'B', 'C' };
    const uint8_t cf1[8] = { 0x21, 'D', 'E', 'F', 'G', 'H', 'I', 'J' };
    const uint8_t cf2[8] = { 0x22, 'K', 'L', 'M', 'N', 'O', 'P', 'Q' };
    uds_rx(ff, 5);
    CHECK(nsent == 2 && sent[1].d[0] == 0x30);
    uds_rx(cf1, 6);
    uds_rx(cf2, 7);
    CHECK(uds_poll(&U, 7) == UDS_EV_POSITIVE && U.resp_len == 20 && U.resp[19] == 'Q');

    /* long request with no flow control: transport error */
    reset_bus();
    uint8_t wr[12] = { 0x2E, 0xF1, 0x90 };
    CHECK(uds_start(&U, wr, sizeof wr, true, 100) == UDS_START_OK);
    CHECK(uds_poll(&U, 500) == UDS_EV_NONE);
    CHECK(uds_poll(&U, 1100) == UDS_EV_TRANSPORT_ERROR && U.transport_error == ISOTP_ERR_TIMEOUT_FC);
    CHECK(!uds_busy(&U));

    /* repeated NRC 0x78 beyond the limit ends as timeout */
    reset_bus();
    const uint8_t rst[2] = { 0x11, 0x01 };
    CHECK(uds_start(&U, rst, 2, true, 0) == UDS_START_OK);
    CHECK(uds_poll(&U, 0) == UDS_EV_NONE);
    const uint8_t pend[8] = { 0x03, 0x7F, 0x11, 0x78, 0, 0, 0, 0 };
    uds_event_t ev = UDS_EV_NONE;
    for (uint32_t i = 1; i <= UDS_MAX_PENDING + 1u; i++) { uds_rx(pend, i * 100u); ev = uds_poll(&U, i * 100u); }
    CHECK(ev == UDS_EV_TIMEOUT && !uds_busy(&U));
}

int main(void)
{
    test_single_frame_padded_and_unpadded();
    test_length_limits();
    test_multi_frame_send_bs0();
    test_block_size_and_stmin();
    test_fc_timeout_wait_overflow();
    test_busy_bus_retries();
    test_send_timeout();
    test_sequence_number_wrap_and_max_length();
    test_receive_single_and_multi();
    test_receive_block_size();
    test_extended_ids();
    test_confirm_rules();
    test_uds_positive_negative_pending_timeout();
    test_uds_suppressed_and_multiframe();
    printf("PASS test_isotp_uds\n");
    return 0;
}
