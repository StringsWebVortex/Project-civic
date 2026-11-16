/* Dashboard persistence on internal flash (see dash_store.h, flash_store.h). */
#include "dash_store.h"
#include "flash_store.h"
#include <string.h>

#define HDR_SIZE  16u
#define LOG_MAGIC 0x474F4C43u   /* "CLOG" */
#define EVT_MAGIC 0x54564543u   /* "CEVT" */

typedef struct { uint32_t magic, seq, rsv0, rsv1; } sect_hdr_t;

static bool hdr_valid(uint32_t base, uint32_t magic, uint32_t *seq)
{
    const sect_hdr_t *h = (const sect_hdr_t *)(const void *)fs_ptr(base);
    if (h->magic != magic || h->seq == 0xFFFFFFFFu) return false;
    if (seq) *seq = h->seq;
    return true;
}

static bool write_hdr(uint32_t base, uint32_t magic, uint32_t seq)
{
    const sect_hdr_t h = { magic, seq, 0u, 0u };
    return fs_program(base, &h, sizeof h);
}

/* ═════════════════════════════ rolling log ═════════════════════════════ */
static uint32_t s_log_cur, s_log_old;      /* sector bases; old may be 0 */
static size_t   s_log_len;                 /* bytes of data in the current sector */
static bool     s_log_ok;

/* Data never contains 0xFF (CSV text), so the end is the first 0xFF byte. */
static size_t data_len(uint32_t base)
{
    const uint8_t *d = fs_ptr(base + HDR_SIZE);
    size_t lo = 0, hi = FS_SECTOR_128K - HDR_SIZE;
    while (lo < hi) { const size_t mid = (lo + hi) / 2u; if (d[mid] == 0xFFu) hi = mid; else lo = mid + 1u; }
    return lo;
}

bool logst_init(void)
{
    uint32_t sa = 0, sb = 0;
    const bool va = hdr_valid(FS_LOG_A, LOG_MAGIC, &sa), vb = hdr_valid(FS_LOG_B, LOG_MAGIC, &sb);
    if (!va && !vb) {
        if (!fs_erase_128k(FS_LOG_A) || !write_hdr(FS_LOG_A, LOG_MAGIC, 1u)) return s_log_ok = false;
        s_log_cur = FS_LOG_A; s_log_old = 0;
    } else if (va && (!vb || sa > sb)) { s_log_cur = FS_LOG_A; s_log_old = vb ? FS_LOG_B : 0u; }
    else                               { s_log_cur = FS_LOG_B; s_log_old = va ? FS_LOG_A : 0u; }
    s_log_len = data_len(s_log_cur);
    return s_log_ok = true;
}

bool logst_ok(void) { return s_log_ok; }

static bool log_rotate(void)
{
    uint32_t seq = 0; (void)hdr_valid(s_log_cur, LOG_MAGIC, &seq);
    const uint32_t next = (s_log_cur == FS_LOG_A) ? FS_LOG_B : FS_LOG_A;
    if (!fs_erase_128k(next) || !write_hdr(next, LOG_MAGIC, seq + 1u)) { s_log_ok = false; return false; }
    s_log_old = s_log_cur; s_log_cur = next; s_log_len = 0;
    return true;
}

bool logst_append(const char *hdr, const char *line, size_t n)
{
    if (!s_log_ok) return false;
    const size_t cap = FS_SECTOR_128K - HDR_SIZE;
    const size_t hl = (s_log_len == 0u && hdr) ? strlen(hdr) : 0u;
    if (s_log_len + hl + n > cap) {
        if (!log_rotate()) return false;
        return logst_append(hdr, line, n);
    }
    uint32_t a = s_log_cur + HDR_SIZE + (uint32_t)s_log_len;
    if (hl && !fs_program(a, hdr, hl)) { s_log_ok = false; return false; }
    a += (uint32_t)hl;
    if (!fs_program(a, line, n)) { s_log_ok = false; return false; }
    s_log_len += hl + n;
    return true;
}

bool logst_get(bool old, const uint8_t **data, size_t *len)
{
    if (!s_log_ok) return false;
    const uint32_t base = old ? s_log_old : s_log_cur;
    if (!base) return false;
    const size_t l = old ? data_len(base) : s_log_len;
    if (l == 0u) return false;
    *data = fs_ptr(base + HDR_SIZE); *len = l;
    return true;
}

void logst_clear(void)
{
    if (!s_log_ok) return;
    (void)fs_erase_128k(FS_LOG_B);
    s_log_ok = fs_erase_128k(FS_LOG_A) && write_hdr(FS_LOG_A, LOG_MAGIC, 1u);
    s_log_cur = FS_LOG_A; s_log_old = 0; s_log_len = 0;
}

/* ═════════════════════════════ event journal ═══════════════════════════ */
/* record: [type][rsv][len u16][id u32][data...]; the type byte is written
 * last, so a record is valid only once fully programmed (power-loss safe). */
typedef struct { uint8_t type, rsv; uint16_t len; uint32_t id; } rec_hdr_t;

static uint32_t s_evt_cur;
static size_t   s_evt_used;               /* offset after the last record */
static bool     s_evt_ok;
static evst_live_fn      s_rewrite;
static evst_snap_live_fn s_snap_live;

static size_t rec_size(uint16_t len) { return (sizeof(rec_hdr_t) + len + 3u) & ~3u; }

/* Walk records; returns the end offset. torn records are skipped by length. */
static size_t evt_walk(uint32_t base, evst_replay_fn fn)
{
    size_t off = HDR_SIZE;
    const size_t end = FS_SECTOR_128K;
    while (off + sizeof(rec_hdr_t) <= end) {
        const rec_hdr_t *h = (const rec_hdr_t *)(const void *)fs_ptr((uint32_t)(base + off));
        if (h->type == 0xFFu && h->len == 0xFFFFu && h->id == 0xFFFFFFFFu) break;   /* free space */
        if (h->len == 0xFFFFu || off + rec_size(h->len) > end) break;               /* torn header */
        if (h->type != 0xFFu && fn) fn(h->type, h->id, (const uint8_t *)(h + 1), h->len);
        off += rec_size(h->len);
    }
    return off;
}

bool evst_init(evst_replay_fn replay, evst_live_fn rewrite_events, evst_snap_live_fn snap_live)
{
    s_rewrite = rewrite_events; s_snap_live = snap_live;
    uint32_t sa = 0, sb = 0;
    const bool va = hdr_valid(FS_EVT_A, EVT_MAGIC, &sa), vb = hdr_valid(FS_EVT_B, EVT_MAGIC, &sb);
    if (!va && !vb) {
        if (!fs_erase_128k(FS_EVT_A) || !write_hdr(FS_EVT_A, EVT_MAGIC, 1u)) return s_evt_ok = false;
        s_evt_cur = FS_EVT_A;
    } else s_evt_cur = (va && (!vb || sa > sb)) ? FS_EVT_A : FS_EVT_B;
    s_evt_used = evt_walk(s_evt_cur, replay);
    return s_evt_ok = true;
}

bool evst_ok(void) { return s_evt_ok; }
size_t evst_total_bytes(void) { return FS_SECTOR_128K - HDR_SIZE; }
size_t evst_free_bytes(void) { return s_evt_ok ? FS_SECTOR_128K - s_evt_used : 0u; }

static bool raw_append(uint32_t base, size_t *used, uint8_t type, uint32_t id, const void *data, uint16_t len)
{
    if (*used + rec_size(len) > FS_SECTOR_128K) return false;
    const uint32_t a = base + (uint32_t)*used;
    const rec_hdr_t h = { 0xFFu, 0xFFu, len, id };
    if (!fs_program(a, &h, sizeof h)) return false;
    if (len && !fs_program(a + sizeof h, data, len)) return false;
    if (!fs_program(a, &type, 1)) return false;            /* commit */
    *used += rec_size(len);
    return true;
}

static uint32_t s_compact_base; static size_t s_compact_used; static bool s_compacting;

bool evst_append(uint8_t type, uint32_t id, const void *data, uint16_t len)
{
    if (!s_evt_ok) return false;
    if (s_compacting) return raw_append(s_compact_base, &s_compact_used, type, id, data, len);
    if (raw_append(s_evt_cur, &s_evt_used, type, id, data, len)) return true;
    if (!evst_compact()) return false;
    return raw_append(s_evt_cur, &s_evt_used, type, id, data, len);
}

static uint32_t s_copy_base; static size_t *s_copy_used; static bool s_copy_ok;
static void copy_live_snap(uint8_t type, uint32_t id, const uint8_t *d, uint16_t len)
{
    if (type != EV_REC_SNAP_ROW || !s_copy_ok || !s_snap_live || !s_snap_live(id)) return;
    s_copy_ok = raw_append(s_copy_base, s_copy_used, type, id, d, len);
}

bool evst_compact(void)
{
    if (!s_evt_ok || s_compacting) return false;
    uint32_t seq = 0; (void)hdr_valid(s_evt_cur, EVT_MAGIC, &seq);
    const uint32_t next = (s_evt_cur == FS_EVT_A) ? FS_EVT_B : FS_EVT_A;
    if (!fs_erase_128k(next)) return false;
    s_compact_base = next; s_compact_used = HDR_SIZE; s_compacting = true;
    bool ok = s_rewrite ? s_rewrite() : true;            /* live event records */
    if (ok) {
        s_copy_base = next; s_copy_used = &s_compact_used; s_copy_ok = true;
        (void)evt_walk(s_evt_cur, copy_live_snap);       /* live snapshot rows */
        ok = s_copy_ok;
    }
    s_compacting = false;
    if (!ok || !write_hdr(next, EVT_MAGIC, seq + 1u)) return false;   /* header last = switch */
    s_evt_cur = next; s_evt_used = s_compact_used;
    return true;
}

static uint32_t s_q_id; static bool s_q_found, s_q_deleted;
static void q_exists(uint8_t type, uint32_t id, const uint8_t *d, uint16_t len)
{
    (void)d; (void)len;
    if (id != s_q_id) return;
    if (type == EV_REC_SNAP_ROW) { s_q_found = true; s_q_deleted = false; }
    else if (type == EV_REC_SNAP_DEL) s_q_deleted = true;
}

bool evst_snap_exists(uint32_t id)
{
    if (!s_evt_ok) return false;
    s_q_id = id; s_q_found = false; s_q_deleted = false;
    (void)evt_walk(s_evt_cur, q_exists);
    return s_q_found && !s_q_deleted;
}

bool evst_snap_next(uint32_t id, uint32_t *cursor, const uint8_t **row, uint16_t *len)
{
    if (!s_evt_ok) return false;
    size_t off = *cursor ? *cursor : HDR_SIZE;
    while (off + sizeof(rec_hdr_t) <= s_evt_used) {
        const rec_hdr_t *h = (const rec_hdr_t *)(const void *)fs_ptr((uint32_t)(s_evt_cur + off));
        if (h->len == 0xFFFFu) return false;
        off += rec_size(h->len);
        if (h->type == EV_REC_SNAP_ROW && h->id == id) {
            *row = (const uint8_t *)(h + 1); *len = h->len; *cursor = (uint32_t)off;
            return true;
        }
    }
    return false;
}
