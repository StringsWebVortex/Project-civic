/* device_id.c - see device_id.h. */
#include "device_id.h"
#include <stdio.h>
#include <string.h>

#define OBS_MIN_MS        3000u
#define OBS_MIN_FRAMES     200u
#define OBS_MAX_MS       10000u
#define OBS_MAX_MIN_FRAMES  20u
#define PROFILE_REC_HDR (4u + DEV_NAME_MAX + 8u + 2u + 1u)

/* Built-in seed: 2013 Honda Civic (9th gen, R18Z1) F-CAN 500 kbit/s.
 * Identifier set from the Honda DBC shipped in firmware/dbc (old project
 * data, unverified until it matches live traffic). */
static const uint16_t s_civic_fcan[] = {
    145, 148, 228, 304, 316, 342, 344, 380, 388, 399, 401, 419, 420, 422, 427,
    432, 450, 464, 487, 490, 506, 547, 597, 660, 773, 777, 780, 804, 829, 862,
    884, 891, 892, 927, 1029, 1302
};

static dev_db_t s_db;
dev_db_t *dev_global(void) { return &s_db; }

static void sort_u16(uint16_t *a, uint16_t n)
{
    for (uint16_t i = 1u; i < n; i++) {
        const uint16_t v = a[i]; uint16_t j = i;
        while (j > 0u && a[j - 1u] > v) { a[j] = a[j - 1u]; j--; }
        a[j] = v;
    }
}

static bool name_ok(const char *n)
{
    if (!n || !n[0] || strlen(n) >= DEV_NAME_MAX) return false;
    for (const char *p = n; *p; p++) if (*p == ',' || *p == '\r' || *p == '\n' || *p == '"') return false;
    return true;
}

void dev_init(dev_db_t *db)
{
    memset(db, 0, sizeof *db);
    db->next_pid = 2u;
    dev_profile_t *p = &db->prof[db->count++];
    p->pid = 1u;
    (void)snprintf(p->name, sizeof p->name, "Honda Civic 2013 9th gen R18Z1");
    p->bitrate[0] = 500000u;
    p->nids = (uint16_t)(sizeof s_civic_fcan / sizeof s_civic_fcan[0]);
    memcpy(p->ids, s_civic_fcan, sizeof s_civic_fcan);
    p->flags = DEV_FLAG_SEED;
}

void dev_observe_start(dev_db_t *db, uint32_t now_ms)
{
    db->state = DEV_OBSERVING;
    db->nobs = 0u;
    db->obs_frames = 0u;
    db->obs_start_ms = now_ms;
    db->best_pid = 0u;
    db->best_score = 0u;
}

void dev_observe(dev_db_t *db, uint8_t bus, uint32_t id, bool ext)
{
    if (db->state != DEV_OBSERVING || bus > 1u) return;
    db->obs_frames++;
    if (ext || id > 0x7FFu) return;
    const uint16_t key = (uint16_t)((bus ? 0x8000u : 0u) | id);
    for (uint16_t i = 0u; i < db->nobs; i++) if (db->obs[i] == key) return;
    if (db->nobs < DEV_MAX_IDS) db->obs[db->nobs++] = key;
}

void dev_set_bitrate(dev_db_t *db, uint8_t bus, uint32_t bitrate)
{
    if (bus <= 1u) db->obs_bitrate[bus] = bitrate;
}

uint16_t dev_score(const dev_db_t *db, const dev_profile_t *p)
{
    if (!p || db->nobs == 0u || p->nids == 0u) return 0u;
    uint32_t inter = 0u, on_bus = 0u;
    for (uint16_t i = 0u; i < db->nobs; i++) {
        const uint8_t b = (db->obs[i] & 0x8000u) ? 1u : 0u;
        if (p->bitrate[b]) on_bus++;
        for (uint16_t j = 0u; j < p->nids; j++)
            if (db->obs[i] == p->ids[j]) { inter++; break; }
    }
    uint32_t s;
    if (p->flags & DEV_FLAG_SEED) {
        /* seed lists every known message; live traffic shows a subset:
         * score = share of observed ids (on the seed's buses) it knows. */
        if (on_bus == 0u) return 0u;
        s = (inter * 1000u) / on_bus;
    } else {
        const uint32_t uni = (uint32_t)db->nobs + p->nids - inter;
        s = (inter * 1000u) / uni;
    }
    for (uint8_t b = 0u; b < 2u; b++)
        if (p->bitrate[b] && db->obs_bitrate[b] && p->bitrate[b] != db->obs_bitrate[b]) s /= 2u;
    return (uint16_t)s;
}

const dev_profile_t *dev_find(const dev_db_t *db, uint32_t pid)
{
    for (uint8_t i = 0u; i < db->count; i++) if (db->prof[i].pid == pid) return &db->prof[i];
    return NULL;
}

static void best_match(dev_db_t *db)
{
    db->best_pid = 0u; db->best_score = 0u;
    for (uint8_t i = 0u; i < db->count; i++) {
        const uint16_t s = dev_score(db, &db->prof[i]);
        if (s > db->best_score) { db->best_score = s; db->best_pid = db->prof[i].pid; }
    }
}

dev_event_t dev_step(dev_db_t *db, uint32_t now_ms)
{
    if (db->state != DEV_OBSERVING) return DEV_EV_NONE;
    const uint32_t el = now_ms - db->obs_start_ms;
    const bool enough = (el >= OBS_MIN_MS && db->obs_frames >= OBS_MIN_FRAMES) ||
                        (el >= OBS_MAX_MS && db->obs_frames >= OBS_MAX_MIN_FRAMES);
    if (!enough) return DEV_EV_NONE;
    db->state = DEV_DONE;
    best_match(db);
    if (db->best_score >= DEV_MATCH_PERMILLE) {
        for (uint8_t i = 0u; i < db->count; i++)
            if (db->prof[i].pid == db->best_pid) db->prof[i].flags |= DEV_FLAG_VERIFIED;
        return DEV_EV_DETECTED;
    }
    return DEV_EV_NEW;
}

dev_add_t dev_add(dev_db_t *db, const char *name, uint32_t *pid)
{
    if (!name_ok(name)) return DEV_ADD_BAD_NAME;
    if (db->nobs == 0u) return DEV_ADD_NO_DATA;
    best_match(db);
    if (db->best_score >= DEV_MATCH_PERMILLE) { *pid = db->best_pid; return DEV_ADD_EXISTS; }
    if (db->count >= DEV_MAX_PROFILES) return DEV_ADD_FULL;
    dev_profile_t *p = &db->prof[db->count++];
    memset(p, 0, sizeof *p);
    p->pid = db->next_pid++;
    (void)snprintf(p->name, sizeof p->name, "%s", name);
    p->bitrate[0] = db->obs_bitrate[0];
    p->bitrate[1] = db->obs_bitrate[1];
    p->nids = db->nobs;
    memcpy(p->ids, db->obs, (size_t)db->nobs * sizeof db->obs[0]);
    sort_u16(p->ids, p->nids);
    p->flags = DEV_FLAG_VERIFIED;
    *pid = p->pid;
    return DEV_ADD_OK;
}

bool dev_select(dev_db_t *db, uint32_t pid)
{
    if (!dev_find(db, pid)) return false;
    db->target = pid;
    return true;
}

void dev_detect_mode(dev_db_t *db) { db->target = 0u; }

bool dev_rename(dev_db_t *db, uint32_t pid, const char *name)
{
    if (!name_ok(name)) return false;
    for (uint8_t i = 0u; i < db->count; i++)
        if (db->prof[i].pid == pid) { (void)snprintf(db->prof[i].name, DEV_NAME_MAX, "%s", name); return true; }
    return false;
}

void dev_suggest_name(const dev_db_t *db, char *out, size_t cap)
{
    (void)snprintf(out, cap, "Unknown network F%luk/B%luk %u IDs",
                   (unsigned long)(db->obs_bitrate[0] / 1000u),
                   (unsigned long)(db->obs_bitrate[1] / 1000u), (unsigned)db->nobs);
}

static void put32(uint8_t *b, uint32_t v) { b[0]=(uint8_t)v; b[1]=(uint8_t)(v>>8); b[2]=(uint8_t)(v>>16); b[3]=(uint8_t)(v>>24); }
static uint32_t get32(const uint8_t *b) { return (uint32_t)b[0] | ((uint32_t)b[1]<<8) | ((uint32_t)b[2]<<16) | ((uint32_t)b[3]<<24); }

size_t dev_encode_profile(const dev_profile_t *p, uint8_t *buf, size_t cap)
{
    const size_t n = PROFILE_REC_HDR + (size_t)p->nids * 2u;
    if (cap < n || p->nids > DEV_MAX_IDS) return 0u;
    size_t k = 0u;
    put32(buf + k, p->pid); k += 4u;
    memcpy(buf + k, p->name, DEV_NAME_MAX); k += DEV_NAME_MAX;
    put32(buf + k, p->bitrate[0]); k += 4u;
    put32(buf + k, p->bitrate[1]); k += 4u;
    buf[k++] = (uint8_t)p->nids; buf[k++] = (uint8_t)(p->nids >> 8);
    buf[k++] = p->flags;
    for (uint16_t i = 0u; i < p->nids; i++) { buf[k++] = (uint8_t)p->ids[i]; buf[k++] = (uint8_t)(p->ids[i] >> 8); }
    return k;
}

bool dev_decode_profile(const uint8_t *buf, size_t len, dev_profile_t *out)
{
    if (!buf || len < PROFILE_REC_HDR) return false;
    dev_profile_t p; memset(&p, 0, sizeof p);
    size_t k = 0u;
    p.pid = get32(buf + k); k += 4u;
    memcpy(p.name, buf + k, DEV_NAME_MAX); k += DEV_NAME_MAX;
    p.name[DEV_NAME_MAX - 1u] = '\0';
    p.bitrate[0] = get32(buf + k); k += 4u;
    p.bitrate[1] = get32(buf + k); k += 4u;
    p.nids = (uint16_t)(buf[k] | (buf[k + 1u] << 8)); k += 2u;
    p.flags = buf[k++];
    if (p.pid == 0u || p.nids > DEV_MAX_IDS || len != PROFILE_REC_HDR + (size_t)p.nids * 2u) return false;
    for (uint16_t i = 0u; i < p.nids; i++, k += 2u) p.ids[i] = (uint16_t)(buf[k] | (buf[k + 1u] << 8));
    *out = p;
    return true;
}

void dev_replay(dev_db_t *db, uint8_t type, uint32_t id, const uint8_t *data, uint16_t len)
{
    if (type == DEV_REC_TARGET && len == 4u) {
        const uint32_t t = get32(data);
        if (t == 0u || dev_find(db, t)) db->target = t;
        return;
    }
    if (type != DEV_REC_PROFILE) return;
    dev_profile_t p;
    if (!dev_decode_profile(data, len, &p) || p.pid != id) return;
    for (uint8_t i = 0u; i < db->count; i++)
        if (db->prof[i].pid == p.pid) { db->prof[i] = p; return; }
    if (db->count >= DEV_MAX_PROFILES) return;
    db->prof[db->count++] = p;
    if (p.pid >= db->next_pid) db->next_pid = p.pid + 1u;
}

bool dev_persist_profile(const dev_db_t *db, uint32_t pid, dev_append_fn append)
{
    const dev_profile_t *p = dev_find(db, pid);
    uint8_t buf[PROFILE_REC_HDR + DEV_MAX_IDS * 2u];
    const size_t n = p ? dev_encode_profile(p, buf, sizeof buf) : 0u;
    return n != 0u && append(DEV_REC_PROFILE, pid, buf, (uint16_t)n);
}

bool dev_persist_target(const dev_db_t *db, dev_append_fn append)
{
    uint8_t b[4]; put32(b, db->target);
    return append(DEV_REC_TARGET, 0u, b, 4u);
}

bool dev_rewrite(const dev_db_t *db, dev_append_fn append)
{
    for (uint8_t i = 0u; i < db->count; i++)
        if (!dev_persist_profile(db, db->prof[i].pid, append)) return false;
    return dev_persist_target(db, append);
}
