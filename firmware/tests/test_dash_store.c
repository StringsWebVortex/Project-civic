/* Host tests for dash_store.c (log rotation, event journal, compaction,
 * power-loss safety) against the NOR flash model in fake_flash.c. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dash_store.h"
#include "flash_store.h"

void fake_flash_reset(void);
extern int fake_fail_after;
extern uint32_t fake_erases;

static int failures;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, m); failures++; } } while (0)

static const char HDR[] = "uptime_ms,utc,key\n";

static void test_log_basic(void)
{
    fake_flash_reset();
    CHECK(logst_init(), "init on blank flash");
    const uint8_t *d; size_t n;
    CHECK(!logst_get(false, &d, &n), "no log yet before first row");
    CHECK(!logst_get(true, &d, &n), "no historical log yet");
    CHECK(logst_append(HDR, "1,,ON\n", 6), "append 1");
    CHECK(logst_append(HDR, "2,,ON\n", 6), "append 2");
    CHECK(logst_get(false, &d, &n) && n == strlen(HDR) + 12 && memcmp(d, HDR, strlen(HDR)) == 0, "header written once at file start");
    CHECK(memcmp(d + strlen(HDR), "1,,ON\n2,,ON\n", 12) == 0, "rows in order");
    /* reboot: state recovered from flash */
    CHECK(logst_init(), "re-init");
    CHECK(logst_get(false, &d, &n) && n == strlen(HDR) + 12, "length recovered after reboot");
    CHECK(logst_append(HDR, "3,,ON\n", 6) && logst_get(false, &d, &n) && n == strlen(HDR) + 18, "append after reboot continues");
}

static void test_log_rotation(void)
{
    fake_flash_reset();
    CHECK(logst_init(), "init");
    char row[200]; memset(row, 'x', sizeof row); row[199] = '\n';
    size_t total = 0;
    while (total < 128u * 1024u) { CHECK(logst_append(HDR, row, sizeof row), "fill"); total += sizeof row; }
    const uint8_t *d; size_t n;
    CHECK(logst_get(true, &d, &n) && n > 120000u, "rotated: historical file holds the full previous log");
    CHECK(logst_get(false, &d, &n) && memcmp(d, HDR, strlen(HDR)) == 0, "new current file starts with the header");
    CHECK(logst_init() && logst_get(true, &d, &n) && logst_get(false, &d, &n), "both files survive reboot");
    /* second rotation reuses the oldest sector */
    total = 0;
    while (total < 128u * 1024u) { CHECK(logst_append(HDR, row, sizeof row), "fill2"); total += sizeof row; }
    CHECK(logst_get(false, &d, &n), "current exists after 2nd rotation");
    logst_clear();
    CHECK(!logst_get(false, &d, &n) && !logst_get(true, &d, &n), "clear removes both files");
    CHECK(logst_append(HDR, "9\n", 2) && logst_get(false, &d, &n) && n == strlen(HDR) + 2, "logging works after clear");
}

/* ── event journal ── */
typedef struct { uint32_t id; uint32_t v; } ev_t;
static ev_t ram[16]; static int nram;
static int replays;
static void replay(uint8_t type, uint32_t id, const uint8_t *data, uint16_t len)
{
    replays++;
    if (type == EV_REC_EVENT && len == sizeof(ev_t)) {
        for (int i = 0; i < nram; i++) if (ram[i].id == id) { memcpy(&ram[i], data, len); return; }
        memcpy(&ram[nram++], data, len);
    } else if (type == EV_REC_EVENT_DEL) {
        for (int i = 0; i < nram; i++) if (ram[i].id == id) { ram[i] = ram[--nram]; return; }
    }
}
static ev_t live[16]; static int nlive; static uint32_t snap_live_id;
static bool rewrite(void) { for (int i = 0; i < nlive; i++) if (!evst_append(EV_REC_EVENT, live[i].id, &live[i], sizeof(ev_t))) return false; return true; }
static bool snap_live(uint32_t id) { return id == snap_live_id; }

static void test_journal(void)
{
    fake_flash_reset(); nram = 0;
    CHECK(evst_init(replay, rewrite, snap_live), "evst init");
    ev_t a = {1, 10}, b = {2, 20};
    CHECK(evst_append(EV_REC_EVENT, 1, &a, sizeof a) && evst_append(EV_REC_EVENT, 2, &b, sizeof b), "append events");
    a.v = 11; CHECK(evst_append(EV_REC_EVENT, 1, &a, sizeof a), "update event 1");
    CHECK(evst_append(EV_REC_EVENT_DEL, 2, NULL, 0), "delete event 2");
    CHECK(evst_append(EV_REC_SNAP_ROW, 1, "r1\n", 3) && evst_append(EV_REC_SNAP_ROW, 1, "r2\n", 3), "snap rows");
    nram = 0; CHECK(evst_init(replay, rewrite, snap_live), "reboot replay");
    CHECK(nram == 1 && ram[0].id == 1 && ram[0].v == 11, "replay: latest value of event 1, event 2 deleted");
    CHECK(evst_snap_exists(1) && !evst_snap_exists(2), "snapshot existence");
    uint32_t cur = 0; const uint8_t *row; uint16_t len; int rows = 0;
    while (evst_snap_next(1, &cur, &row, &len)) { rows++; CHECK(len == 3, "row length"); }
    CHECK(rows == 2, "two snapshot rows streamed in order");
    CHECK(evst_append(EV_REC_SNAP_DEL, 1, NULL, 0) && !evst_snap_exists(1), "snapshot delete");
}

static void test_compaction(void)
{
    fake_flash_reset(); nram = 0;
    CHECK(evst_init(replay, rewrite, snap_live), "init");
    nlive = 1; live[0] = (ev_t){7, 70}; snap_live_id = 7;
    CHECK(evst_append(EV_REC_SNAP_ROW, 7, "keep\n", 5), "live snapshot row");
    CHECK(evst_append(EV_REC_SNAP_ROW, 8, "drop\n", 5), "dead snapshot row");
    ev_t junk = {9, 0};
    const uint32_t erases0 = fake_erases;
    for (int i = 0; i < 10000; i++) { junk.v = (uint32_t)i; CHECK(evst_append(EV_REC_EVENT, 9, &junk, sizeof junk), "churn"); }
    CHECK(fake_erases > erases0, "journal compacted when full");
    CHECK(evst_snap_exists(7) && !evst_snap_exists(8), "compaction keeps live snapshots only");
    nram = 0; CHECK(evst_init(replay, rewrite, snap_live), "reboot after compaction");
    int found7 = 0; for (int i = 0; i < nram; i++) if (ram[i].id == 7 && ram[i].v == 70) found7 = 1;
    CHECK(found7, "live event rewritten into the new sector");
}

static void test_power_cut(void)
{
    fake_flash_reset(); nram = 0;
    CHECK(evst_init(replay, rewrite, snap_live), "init");
    ev_t a = {1, 1};
    CHECK(evst_append(EV_REC_EVENT, 1, &a, sizeof a), "good record");
    fake_fail_after = 6;                                   /* cut power mid-record */
    ev_t b = {2, 2};
    CHECK(!evst_append(EV_REC_EVENT, 2, &b, sizeof b), "torn write reports failure");
    fake_fail_after = -1;
    nram = 0; replays = 0;
    CHECK(evst_init(replay, rewrite, snap_live), "reboot after power cut");
    CHECK(nram == 1 && ram[0].id == 1, "torn (uncommitted) record ignored, good record kept");
    ev_t c = {3, 3};
    CHECK(evst_append(EV_REC_EVENT, 3, &c, sizeof c), "journal writable after torn record");
    nram = 0; CHECK(evst_init(replay, rewrite, snap_live) && nram == 2, "new record survives reboot");
}

int main(void)
{
    test_log_basic();
    test_log_rotation();
    test_journal();
    test_compaction();
    test_power_cut();
    CHECK(logst_ok() && evst_ok(), "stores report ok after init");
    CHECK(evst_total_bytes() > 0u && evst_free_bytes() <= evst_total_bytes(), "event store sizes");
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("test_dash_store: all checks passed\n");
    return 0;
}
