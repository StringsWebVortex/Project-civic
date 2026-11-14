#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../Core/Inc/can_signal_profiler.h"
#include "../Core/Inc/can_frame_queue.h"

static int emit_count = 0;
static char last_line[128];
static bool test_write(const char *line)
{
    emit_count++;
    strncpy(last_line, line, sizeof last_line - 1);
    last_line[sizeof last_line - 1] = '\0';
    return true;
}

/* Output with room for only `room` lines per call; records every line. */
static int room;
static char seen_lines[4096][64];
static int nseen;
static bool limited_write(const char *line)
{
    if (room <= 0) return false;
    room--;
    assert(nseen < 4096);
    strncpy(seen_lines[nseen], line, 63);
    seen_lines[nseen][63] = '\0';
    nseen++;
    return true;
}

static can_capture_frame_t make_frame(uint32_t id, uint32_t ts, const uint8_t *data, uint8_t dlc)
{
    can_capture_frame_t f = {0};
    f.identifier   = id;
    f.extended     = 0u;
    f.remote       = 0u;
    f.dlc          = dlc;
    f.timestamp_ms = ts;
    if (data) {
        for (uint8_t i = 0u; i < dlc && i < 8u; i++) f.data[i] = data[i];
    }
    return f;
}

int main(void)
{
    can_profiler_t p;

    /* ── Init ─────────────────────────────────────────────────────── */
    can_profiler_init(&p);
    assert(p.count == 0u);
    assert(can_profiler_min_interval_ms(&p) == 0u);

    /* ── Single frame: min/max should equal the byte value ─────────── */
    {
        uint8_t d0[] = {0xAB, 0x00, 0xFF, 0x55, 0x00, 0x00, 0x00, 0x00};
        can_capture_frame_t f = make_frame(0x300, 100u, d0, 8u);
        can_profiler_update(&p, &f);
        assert(p.count == 1u);
        can_id_profile_t *e = &p.entries[0];
        assert(e->id == 0x300u);
        assert(e->frame_count == 1u);
        assert(e->byte_min[0] == 0xABu);
        assert(e->byte_max[0] == 0xABu);
        assert(e->byte_last[0] == 0xABu);
        assert(e->var_mask == 0u); /* no change yet on first frame */
    }

    /* ── Second frame with different byte 0: var_mask bit 0 set ────── */
    {
        uint8_t d1[] = {0x31, 0x00, 0xFF, 0x55, 0x00, 0x00, 0x00, 0x00};
        can_capture_frame_t f = make_frame(0x300, 110u, d1, 8u);
        can_profiler_update(&p, &f);
        can_id_profile_t *e = &p.entries[0];
        assert(e->frame_count == 2u);
        assert(e->byte_min[0] == 0x31u);
        assert(e->byte_max[0] == 0xABu);
        assert(e->byte_last[0] == 0x31u);
        assert(e->var_mask & 0x01u); /* bit 0 = byte 0 changed */
        /* byte 1 constant (0x00) — should NOT be in var_mask */
        assert(!(e->var_mask & 0x02u));
    }

    /* ── New ID creates a second entry ─────────────────────────────── */
    {
        uint8_t d[] = {0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        can_capture_frame_t f = make_frame(0x1A0, 200u, d, 8u);
        can_profiler_update(&p, &f);
        assert(p.count == 2u);
        assert(p.entries[1].id == 0x1A0u);
    }

    /* ── Inter-frame interval tracking ─────────────────────────────── */
    {
        /* Send 3 more frames for 0x300 at 10 ms intervals */
        uint8_t d[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        for (uint32_t t = 120u; t <= 140u; t += 10u) {
            can_capture_frame_t f = make_frame(0x300, t, d, 8u);
            can_profiler_update(&p, &f);
        }
        /* min_interval_ms should be ≤ 10 */
        assert(can_profiler_min_interval_ms(&p) <= 10u);
    }

    /* ── Emit: no output before interval elapsed ────────────────────── */
    {
        emit_count = 0;
        can_profiler_emit(&p, 100u, test_write); /* first call sets last_emit_ms */
        /* second call immediately: 0 ms elapsed — should emit because last_emit_ms was 0 */
        emit_count = 0;
        can_profiler_emit(&p, 150u, test_write); /* 50 ms elapsed, < 2000 — no emit */
        assert(emit_count == 0);
    }

    /* ── Emit: output after PROFILER_EMIT_INTERVAL_MS elapsed ─────── */
    {
        emit_count = 0;
        can_profiler_emit(&p, 100u + PROFILER_EMIT_INTERVAL_MS + 1u, test_write);
        assert(emit_count > 0); /* must have emitted at least one PROFILE line */
        /* last_line should contain a PROFILE, or SIG */
        assert(strncmp(last_line, "PROFILE,", 8) == 0 ||
               strncmp(last_line, "SIG,", 4) == 0);
    }

    /* ── Table full: 129th ID is silently dropped ─────────────────── */
    {
        can_profiler_init(&p);
        uint8_t d[] = {0x01};
        for (uint32_t id = 0u; id < PROFILER_MAX_IDS; id++) {
            can_capture_frame_t f = make_frame(id, id * 10u, d, 1u);
            can_profiler_update(&p, &f);
        }
        assert(p.count == PROFILER_MAX_IDS);
        /* 129th ID — should not crash and count stays at max */
        {
            can_capture_frame_t f = make_frame(0xFFFFu, 9999u, d, 1u);
            can_profiler_update(&p, &f);
            assert(p.count == PROFILER_MAX_IDS);
        }
    }

    /* ── Burst larger than the output: resumed, nothing lost or repeated ── */
    {
        can_profiler_init(&p);
        for (uint32_t id = 0u; id < 100u; id++) {
            uint8_t a[8] = {0, 1, 2, 3, 4, 5, 6, 7};
            uint8_t b[8] = {9, 9, 9, 9, 9, 9, 9, 9};
            can_capture_frame_t f = make_frame(0x100u + id, 10u, a, 8u);
            can_profiler_update(&p, &f);
            f = make_frame(0x100u + id, 20u, b, 8u);
            can_profiler_update(&p, &f);
        }
        /* full output (unlimited) = 100 x (1 PROFILE + 8 SIG) = 900 lines */
        nseen = 0; room = 1000000;
        can_profiler_emit(&p, 100u, limited_write);
        assert(nseen == 900);
        char ref[900][64];
        memcpy(ref, seen_lines, sizeof ref);
        /* now only 7 lines fit per call */
        room = 0;
        can_profiler_emit(&p, 100u + PROFILER_EMIT_INTERVAL_MS, limited_write); /* room 0 */
        assert(p.emit_active == 1u);
        nseen = 0;
        uint32_t t = 100u + PROFILER_EMIT_INTERVAL_MS;
        for (int i = 0; i < 1000 && p.emit_active; i++) { room = 7; can_profiler_emit(&p, ++t, limited_write); }
        assert(!p.emit_active);
        assert(nseen == 900);
        for (int i = 0; i < 900; i++) assert(strcmp(ref[i], seen_lines[i]) == 0);
        assert(strncmp(seen_lines[899], "SIG,163,7,", 10) == 0);
        /* next cycle only after the interval */
        nseen = 0; room = 1000000;
        can_profiler_emit(&p, t + 1u, limited_write);
        assert(nseen == 0);
    }

    /* ── NULL safety ──────────────────────────────────────────────── */
    can_profiler_init(NULL); /* no crash */
    can_profiler_update(NULL, NULL); /* no crash */
    can_profiler_emit(NULL, 0u, test_write); /* no crash */
    {
        can_profiler_t p2;
        can_profiler_init(&p2);
        can_profiler_update(&p2, NULL); /* no crash */
    }

    puts("PASS: can_signal_profiler — init, update, min/max, var_mask, rate, emit, table-full, nulls");
    return 0;
}
