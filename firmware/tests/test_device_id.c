/* Host tests for passive device identification (device_id.c). */
#include <assert.h>
#include <string.h>
#include "device_id.h"

static uint8_t jbuf[64][300]; static uint16_t jlen[64]; static uint8_t jtype[64]; static uint32_t jid[64];
static int jn; static int jfail_after = -1;
static bool append(uint8_t type, uint32_t id, const void *data, uint16_t len)
{
    if (jfail_after >= 0 && jn >= jfail_after) return false;
    assert(jn < 64 && len <= sizeof jbuf[0]);
    memcpy(jbuf[jn], data, len); jlen[jn] = len; jtype[jn] = type; jid[jn] = id; jn++;
    return true;
}

static void feed(dev_db_t *db, uint8_t bus, const uint16_t *ids, int n, int reps)
{
    for (int r = 0; r < reps; r++) for (int i = 0; i < n; i++) dev_observe(db, bus, ids[i], false);
}

int main(void)
{
    static dev_db_t db;
    dev_init(&db);
    assert(db.count == 1u && db.target == 0u && db.prof[0].pid == 1u);
    assert(db.prof[0].flags & DEV_FLAG_SEED);
    assert(dev_find(&db, 1u) && !dev_find(&db, 9u));
    assert(dev_global() != NULL);

    /* not observing: ignored */
    dev_observe(&db, 0, 0x123, false);
    assert(db.nobs == 0u && dev_step(&db, 100000u) == DEV_EV_NONE);
    assert(dev_score(&db, &db.prof[0]) == 0u);

    /* Civic-like F-CAN traffic: subset of seed ids at 500k -> DETECTED */
    const uint16_t civic[] = { 145, 148, 228, 304, 316, 342, 344, 380, 388, 399, 401, 419, 420 };
    dev_observe_start(&db, 1000u);
    dev_set_bitrate(&db, 0, 500000u);
    dev_set_bitrate(&db, 5, 1u);                    /* bad bus ignored */
    feed(&db, 0, civic, 13, 20);
    dev_observe(&db, 7, 1, false);                  /* bad bus ignored */
    dev_observe(&db, 0, 0x18DAF110u, true);         /* ext counted, not kept */
    assert(db.nobs == 13u);
    assert(dev_step(&db, 2000u) == DEV_EV_NONE);    /* too early */
    assert(dev_step(&db, 4500u) == DEV_EV_DETECTED);
    assert(db.best_pid == 1u && db.best_score == 1000u);
    assert(db.prof[0].flags & DEV_FLAG_VERIFIED);
    assert(dev_step(&db, 5000u) == DEV_EV_NONE);    /* once per window */

    /* adding the same network reports EXISTS */
    uint32_t pid = 0;
    assert(dev_add(&db, "My Civic", &pid) == DEV_ADD_EXISTS && pid == 1u);

    /* wrong bitrate halves the score */
    dev_set_bitrate(&db, 0, 250000u);
    assert(dev_score(&db, &db.prof[0]) == 500u);

    /* unknown network on both buses -> NEW, then add, select, persist */
    const uint16_t other[] = { 0x10, 0x20, 0x30, 0x40, 0x50 };
    dev_observe_start(&db, 0u);
    dev_set_bitrate(&db, 0, 250000u); dev_set_bitrate(&db, 1, 125000u);
    feed(&db, 1, other, 5, 5);                      /* 25 frames on B only */
    assert(dev_score(&db, &db.prof[0]) == 0u);      /* seed has no B bus */
    assert(dev_step(&db, 5000u) == DEV_EV_NONE);    /* < 200 frames, < 10 s */
    assert(dev_step(&db, 10000u) == DEV_EV_NEW);
    char nm[64]; dev_suggest_name(&db, nm, sizeof nm);
    assert(strcmp(nm, "Unknown network F250k/B125k 5 IDs") == 0);
    assert(dev_add(&db, "", &pid) == DEV_ADD_BAD_NAME);
    assert(dev_add(&db, "a,b", &pid) == DEV_ADD_BAD_NAME);
    assert(dev_add(&db, NULL, &pid) == DEV_ADD_BAD_NAME);
    assert(dev_add(&db, "0123456789012345678901234567890123456789", &pid) == DEV_ADD_BAD_NAME);
    assert(dev_add(&db, "Bench rig", &pid) == DEV_ADD_OK && pid == 2u);
    const dev_profile_t *p = dev_find(&db, 2u);
    assert(p && p->nids == 5u && p->ids[0] == (0x8000u | 0x10u) && p->bitrate[1] == 125000u);
    assert(dev_add(&db, "Bench rig again", &pid) == DEV_ADD_EXISTS && pid == 2u);
    assert(dev_select(&db, 2u) && db.target == 2u);
    assert(!dev_select(&db, 77u) && db.target == 2u);
    assert(dev_rename(&db, 2u, "Rig") && strcmp(dev_find(&db, 2u)->name, "Rig") == 0);
    assert(!dev_rename(&db, 99u, "X") && !dev_rename(&db, 2u, "bad\"name"));

    /* no data */
    static dev_db_t e; dev_init(&e); dev_observe_start(&e, 0u);
    assert(dev_add(&e, "X", &pid) == DEV_ADD_NO_DATA);

    /* persistence round trip through the journal */
    jn = 0;
    assert(dev_rewrite(&db, append));
    assert(jn == 3 && jtype[0] == DEV_REC_PROFILE && jtype[2] == DEV_REC_TARGET);
    static dev_db_t r; dev_init(&r);
    for (int i = 0; i < jn; i++) dev_replay(&r, jtype[i], jid[i], jbuf[i], jlen[i]);
    assert(r.count == 2u && r.target == 2u && r.next_pid == 3u);
    assert(strcmp(dev_find(&r, 2u)->name, "Rig") == 0 && dev_find(&r, 2u)->nids == 5u);
    /* replay edge cases */
    dev_replay(&r, 99, 0, jbuf[0], jlen[0]);                       /* unknown type */
    dev_replay(&r, DEV_REC_PROFILE, 5u, jbuf[1], jlen[1]);          /* id mismatch */
    dev_replay(&r, DEV_REC_PROFILE, 2u, jbuf[1], 3u);               /* short */
    uint8_t t9[4] = { 9, 0, 0, 0 }; dev_replay(&r, DEV_REC_TARGET, 0, t9, 4); /* unknown pid */
    assert(r.target == 2u);
    uint8_t t0[4] = { 0 }; dev_replay(&r, DEV_REC_TARGET, 0, t0, 4);
    assert(r.target == 0u);
    dev_replay(&r, DEV_REC_TARGET, 0, t0, 3);                       /* bad length */
    assert(r.count == 2u);
    /* encode/decode limits */
    uint8_t small[8];
    assert(dev_encode_profile(&db.prof[0], small, sizeof small) == 0u);
    dev_profile_t bad = db.prof[1]; bad.nids = DEV_MAX_IDS + 1u;
    uint8_t big[400]; assert(dev_encode_profile(&bad, big, sizeof big) == 0u);
    dev_profile_t out;
    assert(!dev_decode_profile(NULL, 10, &out));
    size_t n = dev_encode_profile(&db.prof[1], big, sizeof big);
    assert(n && dev_decode_profile(big, n, &out) && out.pid == 2u);
    assert(!dev_decode_profile(big, n - 1u, &out));
    big[0] = big[1] = big[2] = big[3] = 0; assert(!dev_decode_profile(big, n, &out)); /* pid 0 */
    /* persist failures */
    jn = 0; jfail_after = 0;
    assert(!dev_rewrite(&db, append));
    assert(!dev_persist_profile(&db, 42u, append));
    jfail_after = 2; jn = 0;
    assert(!dev_rewrite(&db, append));                              /* target write fails */
    jfail_after = -1;

    /* full table */
    static dev_db_t f; dev_init(&f);
    for (uint32_t k = 0; k < DEV_MAX_PROFILES; k++) {
        dev_observe_start(&f, 0u);
        uint16_t one = (uint16_t)(0x100u + k); feed(&f, 1, &one, 1, 1);
        uint32_t q; dev_add_t a = dev_add(&f, "N", &q);
        assert(k < DEV_MAX_PROFILES - 1u ? a == DEV_ADD_OK : a == DEV_ADD_FULL);
    }
    /* replay into a full table is ignored */
    dev_replay(&f, DEV_REC_PROFILE, 2u, jbuf[0], 0);
    n = dev_encode_profile(&db.prof[1], big, sizeof big); big[0] = 200;
    dev_replay(&f, DEV_REC_PROFILE, 200u, big, (uint16_t)n);
    assert(f.count == DEV_MAX_PROFILES);
    /* detect mode + observation id cap */
    dev_detect_mode(&f); assert(f.target == 0u);
    dev_observe_start(&f, 0u);
    for (uint16_t i = 0; i < 200u; i++) dev_observe(&f, 0, i, false);
    assert(f.nobs == DEV_MAX_IDS);
    return 0;
}
