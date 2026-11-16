/* Host tests for sig_learn.c: SEED -> CHECKED -> VERIFIED / CONFLICT against
 * the OBD-II reference, persistence round trip, and guards. */
#include <stdio.h>
#include <string.h>
#include "sig_learn.h"

static int failures;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, m); failures++; } } while (0)

static int idx(uint32_t id) { for (unsigned i = 0; i < can_dbc_msg_count; i++) if (can_dbc_msgs[i].id == id) return (int)i; return -1; }

/* 0x158 ENGINE_DATA: XMISSION_SPEED bytes 0-1 (x0.01 kph), ENGINE_RPM bytes 2-3 (big-endian). */
static void engine_data(uint8_t d[8], unsigned kph100, unsigned rpm)
{
    memset(d, 0, 8);
    d[0] = (uint8_t)(kph100 >> 8); d[1] = (uint8_t)kph100; d[2] = (uint8_t)(rpm >> 8); d[3] = (uint8_t)rpm;
}

static bool rec_ok = true; static uint8_t rec_type; static uint32_t rec_id; static uint8_t rec[16]; static uint16_t rec_len; static int recs;
static bool append(uint8_t t, uint32_t id, const void *d, uint16_t n)
{ rec_type = t; rec_id = id; memcpy(rec, d, n); rec_len = n; recs++; return rec_ok; }

int main(void)
{
    sl_t s; sl_init(&s);
    CHECK(s.n == can_dbc_msg_count && s.n <= SL_MAX_MSGS, "one entry per DBC message");
    const int i158 = idx(0x158), i309 = idx(0x309), i17c = idx(0x17C);
    CHECK(i158 >= 0 && i309 >= 0 && i17c >= 0, "reference messages present");
    uint8_t d[8], k;

    /* seen with the right DLC, no reference yet -> CHECKED after 20 frames */
    engine_data(d, 0, 0);
    for (int t = 0; t < 19; t++) sl_on_frame(&s, 0, 0x158, false, 8, d, (uint32_t)t);
    CHECK(s.e[i158].state == SL_SEED, "19 frames: still SEED");
    sl_on_frame(&s, 0, 0x158, false, 8, d, 19);
    CHECK(s.e[i158].state == SL_CHECKED, "20 frames: CHECKED");
    CHECK(sl_take_change(&s, &k) && k == i158 && !sl_take_change(&s, &k), "one change reported");

    /* RPM agrees with OBD PID 0x0C ten times while running -> VERIFIED */
    for (unsigned t = 0; t < 10; t++) {
        sl_on_obd(&s, 0x0C, 1500.0f + (float)t, 1000 + t * 100);
        engine_data(d, 0, 1510 + t);
        sl_on_frame(&s, 0, 0x158, false, 8, d, 1000 + t * 100 + 50);
    }
    CHECK(s.e[i158].state == SL_VERIFIED && s.e[i158].match == 10, "RPM matched OBD: VERIFIED");
    /* stale or idle references are ignored */
    sl_on_obd(&s, 0x0C, 300.0f, 5000); sl_on_frame(&s, 0, 0x158, false, 8, d, 5001);
    sl_on_obd(&s, 0x0C, 2000.0f, 6000); sl_on_frame(&s, 0, 0x158, false, 8, d, 6000 + SL_REF_MAX_AGE_MS + 1);
    CHECK(s.e[i158].match == 10 && s.e[i158].miss == 0, "idle rpm and stale reference not compared");

    /* a definition that disagrees with OBD speed -> CONFLICT (0x309 CAR_SPEED, x0.01 kph) */
    for (unsigned t = 0; t < 10; t++) {
        sl_on_obd(&s, 0x0D, 60.0f, 20000 + t * 100);
        uint8_t c[8] = { 0x03, 0xE8 };                       /* 10.00 kph, OBD says 60 */
        sl_on_frame(&s, 0, 0x309, false, 8, c, 20000 + t * 100 + 10);
    }
    CHECK(s.e[i309].state == SL_CONFLICT, "speed disagrees with OBD: CONFLICT");
    /* the same message later agreeing much more often recovers to VERIFIED */
    for (unsigned t = 0; t < 20; t++) {
        sl_on_obd(&s, 0x0D, 60.0f, 30000 + t * 100);
        /* CAR_SPEED 60.00 kph, ROUGH_CAR_SPEED 37 mph, ROUGH_CAR_SPEED_2 37 mph, ROUGH_CAR_SPEED_3 60.00 kph */
        uint8_t c[8] = { 0x17, 0x70, 37, 37, 0x17, 0x70 };
        sl_on_frame(&s, 0, 0x309, false, 8, c, 30000 + t * 100 + 10);
    }
    CHECK(s.e[i309].state == SL_VERIFIED, "consistent agreement overrides an old conflict");

    /* wrong DLC for the DBC layout -> CONFLICT immediately */
    sl_on_frame(&s, 0, 0x17C, false, 4, d, 40000);
    CHECK(s.e[i17c].state == SL_CONFLICT && s.e[i17c].dlc_seen == 4, "DLC mismatch: CONFLICT");

    /* ignored: B-CAN, extended ids, unknown ids, NULL */
    sl_on_frame(&s, 1, 0x158, false, 8, d, 1); sl_on_frame(&s, 0, 0x158, true, 8, d, 1);
    sl_on_frame(&s, 0, 0x7FF, false, 8, d, 1); sl_on_frame(NULL, 0, 0x158, false, 8, d, 1);
    sl_on_frame(&s, 0, 0x158, false, 8, NULL, 1); sl_on_obd(&s, 0x05, 90.0f, 1);

    /* persistence round trip; compaction rewrites every learned entry */
    while (sl_take_change(&s, &k)) {}
    CHECK(sl_persist(&s, (uint8_t)i309, append) && rec_type == SL_REC_STATE && rec_id == 0x309 && rec_len == sizeof(sl_entry_t), "persist");
    sl_t r; sl_init(&r);
    CHECK(sl_replay(&r, rec_type, rec_id, rec, rec_len) && r.e[i309].state == SL_VERIFIED, "replay");
    CHECK(!sl_replay(&r, 9, 0x309, rec, rec_len) && !sl_replay(&r, SL_REC_STATE, 0x7FF, rec, rec_len) &&
          !sl_replay(&r, SL_REC_STATE, 0x309, rec, 3) && !sl_replay(&r, SL_REC_STATE, 0x309, NULL, rec_len), "replay guards");
    rec[0] = 9; CHECK(!sl_replay(&r, SL_REC_STATE, 0x309, rec, rec_len), "invalid state rejected");
    recs = 0; CHECK(sl_rewrite(&s, append) && recs == 3, "rewrite: the three graded messages");
    rec_ok = false; CHECK(!sl_rewrite(&s, append), "rewrite reports a failed append");
    CHECK(!sl_persist(&s, 200, append) && !sl_persist(NULL, 0, append) && !sl_persist(&s, 0, NULL), "persist guards");
    CHECK(!sl_take_change(NULL, &k), "take_change NULL");
    s.dirty = 1ull << 63; s.n = 2; CHECK(!sl_take_change(&s, &k) && s.dirty == 0, "stale dirty bits beyond n cleared");

    CHECK(strcmp(sl_state_name(SL_SEED), "SEED") == 0 && strcmp(sl_state_name(SL_CHECKED), "CHECKED") == 0 &&
          strcmp(sl_state_name(SL_VERIFIED), "VERIFIED") == 0 && strcmp(sl_state_name(SL_CONFLICT), "CONFLICT") == 0 &&
          strcmp(sl_state_name((sl_state_t)9), "?") == 0, "state names");
    CHECK(sl_global() == sl_global(), "global instance");
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("test_sig_learn: all checks passed\n");
    return 0;
}
