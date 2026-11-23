/* Host tests for ecu_scan.c */
#include "ecu_scan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

typedef struct { uint32_t id; bool ext; uint8_t d[8]; uint8_t len; } frame_t;
static frame_t sent[600];
static int nsent;
static bool refuse;

static bool fake_send(void *ctx, uint32_t id, bool ext, const uint8_t *d, uint8_t len)
{
    (void)ctx;
    if (refuse) return false;
    if (nsent < (int)(sizeof sent / sizeof sent[0])) {
        sent[nsent].id = id; sent[nsent].ext = ext; sent[nsent].len = len;
        memcpy(sent[nsent].d, d, len);
    }
    nsent++;
    return true;
}

static void test_ids(void)
{
    CHECK(ecu_scan_tx_id(false, 0x7E0) == 0x7E0u);
    CHECK(ecu_scan_rx_id(false, 0x7E0) == 0x7E8u);
    CHECK(ecu_scan_tx_id(true, 0x10) == 0x18DA10F1u);
    CHECK(ecu_scan_rx_id(true, 0x10) == 0x18DAF110u);
}

static void test_begin_validation(void)
{
    ecu_scan_t s;
    const uint8_t r[8] = {0x3E, 0x00};
    CHECK(!ecu_scan_begin(&s, false, 0x7E0, 0x7E7, r, 0, 60, true, 0));
    CHECK(!ecu_scan_begin(&s, false, 0x7E0, 0x7E7, r, 8, 60, true, 0));
    CHECK(!ecu_scan_begin(&s, false, 0x7E8, 0x7E0, r, 2, 60, true, 0));
    CHECK(!ecu_scan_begin(&s, false, 0x7E0, 0x7F8, r, 2, 60, true, 0));
    CHECK(!ecu_scan_begin(&s, true, 0x00, 0x100, r, 2, 60, true, 0));
    CHECK(!ecu_scan_begin(&s, false, 0x7E0, 0x7E7, r, 2, 0, true, 0));
    CHECK(ecu_scan_begin(&s, false, 0x7E0, 0x7E7, r, 2, 60, true, 0));
}

/* 11-bit scan, ECUs at 0x7E0 (positive) and 0x7E2 (negative response) */
static void test_scan_11bit(void)
{
    ecu_scan_t s;
    const uint8_t r[2] = {0x01, 0x00};
    nsent = 0; refuse = false;
    CHECK(ecu_scan_begin(&s, false, 0x7E0, 0x7E3, r, 2, 60, true, 0xAA));
    uint32_t t = 0;
    int found = 0, done = 0;
    uint32_t found_rx[4] = {0};
    for (int i = 0; i < 2000 && !done; i++, t++) {
        ecu_scan_event_t ev = ecu_scan_poll(&s, t, fake_send, NULL);
        if (ev == ECU_SCAN_EV_FOUND) { if (found < 4) found_rx[found] = s.found_rx; found++; }
        if (ev == ECU_SCAN_EV_DONE) done = 1;
        /* answer right after a request goes out */
        if (s.sent && s.active && !s.hit && nsent > 0) {
            const frame_t *f = &sent[nsent - 1];
            if (f->id == 0x7E0 && s.cur == 0x7E0) {
                const uint8_t a[8] = {0x06, 0x41, 0x00, 0xBE, 0x3F, 0xA8, 0x13, 0x00};
                ecu_scan_on_frame(&s, 0x7E8, false, a, 8);
            }
            if (f->id == 0x7E2 && s.cur == 0x7E2) {
                const uint8_t a[8] = {0x03, 0x7F, 0x01, 0x11, 0, 0, 0, 0};
                ecu_scan_on_frame(&s, 0x7EA, false, a, 8);
            }
        }
    }
    CHECK(done == 1);
    CHECK(found == 2);
    CHECK(found_rx[0] == 0x7E8u && found_rx[1] == 0x7EAu);
    CHECK(s.found_count == 2);
    CHECK(nsent == 4);
    CHECK(sent[0].len == 8 && sent[0].d[0] == 0x02 && sent[0].d[1] == 0x01 &&
          sent[0].d[2] == 0x00 && sent[0].d[3] == 0xAA && sent[0].d[7] == 0xAA);
    CHECK(!ecu_scan_active(&s));
}

/* A found ECU at the last address still reports FOUND, then DONE */
static void test_found_last(void)
{
    ecu_scan_t s;
    const uint8_t r[2] = {0x3E, 0x00};
    nsent = 0; refuse = false;
    CHECK(ecu_scan_begin(&s, true, 0x10, 0x11, r, 2, 50, false, 0));
    uint32_t t = 0;
    int found = 0, done = 0;
    for (int i = 0; i < 500 && !done; i++, t++) {
        ecu_scan_event_t ev = ecu_scan_poll(&s, t, fake_send, NULL);
        if (ev == ECU_SCAN_EV_FOUND) { found++; CHECK(s.found_rx == 0x18DAF111u); }
        if (ev == ECU_SCAN_EV_DONE) done = 1;
        if (s.active && s.sent && s.cur == 0x11 && !s.hit) {
            const uint8_t a[3] = {0x02, 0x7E, 0x00};
            ecu_scan_on_frame(&s, 0x18DAF111u, true, a, 3);
        }
    }
    CHECK(found == 1 && done == 1);
    CHECK(nsent == 2 && sent[0].ext && sent[0].id == 0x18DA10F1u && sent[0].len == 3);
}

/* Frames that are not answers are ignored; send refusal is retried */
static void test_ignores_and_retry(void)
{
    ecu_scan_t s;
    const uint8_t r[2] = {0x3E, 0x00};
    nsent = 0; refuse = true;
    CHECK(ecu_scan_begin(&s, false, 0x7E0, 0x7E0, r, 2, 10, true, 0));
    CHECK(ecu_scan_poll(&s, 0, fake_send, NULL) == ECU_SCAN_EV_NONE);
    CHECK(!s.sent && nsent == 0);
    refuse = false;
    CHECK(ecu_scan_poll(&s, 1, fake_send, NULL) == ECU_SCAN_EV_NONE);
    CHECK(s.sent && nsent == 1);
    const uint8_t cf[8] = {0x21, 1, 2, 3, 4, 5, 6, 7};
    const uint8_t fc[3] = {0x30, 0, 0};
    const uint8_t bad_sf[2] = {0x05, 0x41};
    ecu_scan_on_frame(&s, 0x7E8, false, cf, 8);
    ecu_scan_on_frame(&s, 0x7E8, false, fc, 3);
    ecu_scan_on_frame(&s, 0x7E8, false, bad_sf, 2);
    ecu_scan_on_frame(&s, 0x7E9, false, (const uint8_t[]){0x02, 0x7E, 0x00}, 3);
    ecu_scan_on_frame(&s, 0x7E8, true, (const uint8_t[]){0x02, 0x7E, 0x00}, 3);
    CHECK(!s.hit);
    CHECK(ecu_scan_poll(&s, 5, fake_send, NULL) == ECU_SCAN_EV_NONE);
    CHECK(ecu_scan_poll(&s, 11, fake_send, NULL) == ECU_SCAN_EV_DONE);
    CHECK(s.found_count == 0);
    /* a valid SF that does not answer the requested service is not an ECU */
    CHECK(ecu_scan_begin(&s, false, 0x7E0, 0x7E0, r, 2, 10, true, 0));
    (void)ecu_scan_poll(&s, 0, fake_send, NULL);
    ecu_scan_on_frame(&s, 0x7E8, false, (const uint8_t[]){0x03, 0x41, 0x0C, 0x10, 0, 0, 0, 0}, 8);
    ecu_scan_on_frame(&s, 0x7E8, false, (const uint8_t[]){0x03, 0x7F, 0x22, 0x11, 0, 0, 0, 0}, 8);
    ecu_scan_on_frame(&s, 0x7E8, false, (const uint8_t[]){0x02, 0x7F, 0x3E, 0, 0, 0, 0, 0}, 8);
    CHECK(!s.hit);
    ecu_scan_on_frame(&s, 0x7E8, false, (const uint8_t[]){0x03, 0x7F, 0x3E, 0x12, 0, 0, 0, 0}, 8);
    CHECK(s.hit);
    CHECK(ecu_scan_poll(&s, 1, fake_send, NULL) == ECU_SCAN_EV_FOUND);
    CHECK(ecu_scan_poll(&s, 2, fake_send, NULL) == ECU_SCAN_EV_DONE);
    /* first frame counts as an answer when it carries the positive SID */
    const uint8_t r09[2] = {0x09, 0x02};
    CHECK(ecu_scan_begin(&s, false, 0x7E0, 0x7E0, r09, 2, 10, true, 0));
    (void)ecu_scan_poll(&s, 0, fake_send, NULL);
    ecu_scan_on_frame(&s, 0x7E8, false, (const uint8_t[]){0x10, 0x14, 0x62, 0xF1, 0x90, 'J', 'H', 'M'}, 8);
    CHECK(!s.hit);
    CHECK(ecu_scan_begin(&s, false, 0x7E0, 0x7E0, r09, 2, 10, true, 0));
    (void)ecu_scan_poll(&s, 0, fake_send, NULL);
    const uint8_t ff[8] = {0x10, 0x14, 0x49, 0x02, 0x01, 'J', 'H', 'M'};
    ecu_scan_on_frame(&s, 0x7E8, false, ff, 8);
    CHECK(ecu_scan_poll(&s, 1, fake_send, NULL) == ECU_SCAN_EV_FOUND);
    CHECK(s.found_len == 8 && s.found_data[0] == 0x10);
    CHECK(ecu_scan_poll(&s, 2, fake_send, NULL) == ECU_SCAN_EV_DONE);
    /* stop */
    CHECK(ecu_scan_begin(&s, true, 0, 0xFF, r, 2, 10, true, 0));
    ecu_scan_stop(&s);
    CHECK(ecu_scan_poll(&s, 0, fake_send, NULL) == ECU_SCAN_EV_NONE);
}

/* Full 29-bit range sends 256 requests and finishes */
static void test_full_29bit(void)
{
    ecu_scan_t s;
    const uint8_t r[2] = {0x3E, 0x00};
    nsent = 0; refuse = false;
    CHECK(ecu_scan_begin(&s, true, 0x00, 0xFF, r, 2, 5, true, 0));
    int done = 0;
    for (uint32_t t = 0; t < 10000 && !done; t++)
        if (ecu_scan_poll(&s, t, fake_send, NULL) == ECU_SCAN_EV_DONE) done = 1;
    CHECK(done && nsent == 256);
    CHECK(sent[255].id == 0x18DAFFF1u);
}

/* IDs already used on the bus are never sent; scan still finishes */
static bool in_use(void *ctx, uint32_t id, bool ext)
{
    (void)ctx;
    return !ext && (id == 0x7E1u || id == 0x7E3u);
}

static void test_skip_in_use(void)
{
    ecu_scan_t s;
    const uint8_t r[2] = {0x3E, 0x00};
    nsent = 0; refuse = false;
    CHECK(ecu_scan_begin(&s, false, 0x7E0, 0x7E3, r, 2, 5, true, 0));
    ecu_scan_set_skip(&s, in_use, NULL);
    int skipped = 0, done = 0;
    uint32_t skipped_id[4] = {0};
    for (uint32_t t = 0; t < 1000 && !done; t++) {
        ecu_scan_event_t ev = ecu_scan_poll(&s, t, fake_send, NULL);
        if (ev == ECU_SCAN_EV_SKIPPED) { if (skipped < 4) skipped_id[skipped] = s.skipped_tx; skipped++; }
        if (ev == ECU_SCAN_EV_DONE) done = 1;
    }
    CHECK(done);
    CHECK(skipped == 2 && skipped_id[0] == 0x7E1u && skipped_id[1] == 0x7E3u);
    CHECK(nsent == 2 && sent[0].id == 0x7E0u && sent[1].id == 0x7E2u);
    for (int i = 0; i < nsent && i < 2; i++) CHECK(sent[i].id != 0x7E1u && sent[i].id != 0x7E3u);
    CHECK(!ecu_scan_active(&s));
}

int main(void)
{
    test_ids();
    test_begin_validation();
    test_scan_11bit();
    test_found_last();
    test_ignores_and_retry();
    test_full_29bit();
    test_skip_in_use();
    if (fails) { printf("test_ecu_scan: %d check(s) failed\n", fails); return 1; }
    printf("test_ecu_scan: all checks passed\n");
    return 0;
}
