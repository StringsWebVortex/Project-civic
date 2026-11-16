/* Single-board dashboard: C port of esp32-dashboard/src/main.cpp running on
 * the STM32F407 itself. Function names follow the ESP32 originals so the two
 * can be compared side by side. The ESP32-only pieces (Wi-Fi AP, ESP32 OTA,
 * STM32 ROM-bootloader-over-UART) are replaced by USB networking and an
 * in-place update of the STM32's own flash (see flash_store.c). */
#include "dash.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h"
#include "lwip/tcp.h"
#include "web_server.h"
#include "dash_store.h"
#include "flash_store.h"
#include "gvret.h"
#include "can_dbc.h"
#define JSMN_STATIC
#include "jsmn.h"

extern volatile uint32_t system_millis;
static uint32_t millis(void) { return system_millis; }

static dash_cmd_fn s_cmd;
static void stm32_print(const char *line) { if (s_cmd) s_cmd(line); }

/* ═══════════════════════ JSON writer (ArduinoJson-compatible output) ═══ */
typedef struct { char *b; size_t cap, n; bool first, err; } jb_t;

static void jb_raw(jb_t *j, const char *s, size_t n)
{
    if (j->err || j->n + n >= j->cap) { j->err = true; return; }
    memcpy(j->b + j->n, s, n); j->n += n; j->b[j->n] = '\0';
}
static void jb_c(jb_t *j, char c) { jb_raw(j, &c, 1); }
static void jb_begin(jb_t *j, char *buf, size_t cap) { j->b = buf; j->cap = cap; j->n = 0; j->first = true; j->err = false; jb_c(j, '{'); }
static void jb_escaped(jb_t *j, const char *s)
{
    jb_c(j, '"');
    for (; *s; s++) {
        const unsigned char ch = (unsigned char)*s;
        switch (ch) {
        case '"': jb_raw(j, "\\\"", 2); break;
        case '\\': jb_raw(j, "\\\\", 2); break;
        case '\b': jb_raw(j, "\\b", 2); break;
        case '\f': jb_raw(j, "\\f", 2); break;
        case '\n': jb_raw(j, "\\n", 2); break;
        case '\r': jb_raw(j, "\\r", 2); break;
        case '\t': jb_raw(j, "\\t", 2); break;
        default:
            if (ch < 0x20u) { char u[8]; snprintf(u, sizeof u, "\\u%04x", ch); jb_raw(j, u, 6); }
            else jb_c(j, (char)ch);
        }
    }
    jb_c(j, '"');
}
static void jb_key(jb_t *j, const char *k) { if (!j->first) jb_c(j, ','); j->first = false; jb_escaped(j, k); jb_c(j, ':'); }
static void jb_str(jb_t *j, const char *k, const char *v) { jb_key(j, k); jb_escaped(j, v); }
static void jb_int(jb_t *j, const char *k, long v) { char t[24]; jb_key(j, k); jb_raw(j, t, (size_t)snprintf(t, sizeof t, "%ld", v)); }
static void jb_uint(jb_t *j, const char *k, unsigned long v) { char t[24]; jb_key(j, k); jb_raw(j, t, (size_t)snprintf(t, sizeof t, "%lu", v)); }
static void jb_bool(jb_t *j, const char *k, bool v) { jb_key(j, k); if (v) jb_raw(j, "true", 4); else jb_raw(j, "false", 5); }
static void jb_num(jb_t *j, const char *k, double v, int digits)
{
    char t[40]; jb_key(j, k);
    if (v != v) { jb_raw(j, "null", 4); return; }
    jb_raw(j, t, (size_t)snprintf(t, sizeof t, "%.*g", digits, v));
}
static size_t jb_end(jb_t *j) { jb_c(j, '}'); return j->err ? 0u : j->n; }

static char s_jbuf[1600];          /* one JSON message at a time (main loop only) */

static void ws_broadcast_doc(jb_t *j) { const size_t n = jb_end(j); if (n) ws_broadcast_txt(j->b, n); }
static void ws_send_doc(uint8_t num, jb_t *j) { const size_t n = jb_end(j); if (n) ws_send_txt(num, j->b, n); }

static void broadcast_type_msg(const char *type, const char *msg)
{
    jb_t j; jb_begin(&j, s_jbuf, sizeof s_jbuf);
    jb_str(&j, "type", type); jb_str(&j, "message", msg);
    ws_broadcast_doc(&j);
}

/* ═══════════════════════════════ C helpers ═════════════════════════════ */
static const char *csvField(const char *s, int idx, char *out, size_t cap)
{
    int field = 0; const char *p = s;
    out[0] = '\0';
    for (;;) {
        const char *e = strchr(p, ',');
        const size_t l = e ? (size_t)(e - p) : strlen(p);
        if (field == idx) { const size_t c = l < cap - 1u ? l : cap - 1u; memcpy(out, p, c); out[c] = '\0'; return out; }
        if (!e) return out;
        p = e + 1; field++;
    }
}
static long fieldInt(const char *s, int idx) { char b[32]; return strtol(csvField(s, idx, b, sizeof b), NULL, 10); }
static int countCommas(const char *s) { int n = 0; for (; *s; s++) if (*s == ',') n++; return n; }
static void upper(char *s) { for (; *s; s++) *s = (char)toupper((unsigned char)*s); }

/* strlcpy semantics (what the ESP32 code used): always NUL-terminates. */
static void copyz(char *dst, size_t cap, const char *src)
{
    size_t i = 0;
    if (!cap) return;
    for (; i + 1u < cap && src[i]; i++) dst[i] = src[i];
    dst[i] = '\0';
}
#define STRLCPY(dst, src) copyz((dst), sizeof(dst), (src))

static uint32_t framesSeen, bitrate, dwellMs;
static bool     locked;
static bool     stm32FlashRunning;

/* ══════════════ FRAME TABLE (per-ID latest data + rate, batched) ═══════ */
#define FRAME_TABLE_MAX 192
#define FRAME_BATCH_MS  250u
typedef struct { uint32_t id; uint8_t bus, ext, dlc, dirty, decdirty; char hex[17]; uint32_t total, win; float rate; } FrameEnt;
static FrameEnt ftab[FRAME_TABLE_MAX];
static int      ftabN;
static uint32_t ftabOverflow;                 /* frames whose ID did not fit the table */
static can_capture_queue_t *s_capq[2];      /* for the drop counters shown on the page */
static uint32_t busWinFrames[2], busWinBits[2], busFps[2], busBitsPs[2];

static void frameTableUpdate(uint32_t id, bool ext, uint8_t bus, uint8_t dlc, const char *hex)
{
    int i = 0;
    for (; i < ftabN; i++) if (ftab[i].id == id && ftab[i].bus == bus && ftab[i].ext == ext) break;
    if (i == ftabN) {
        if (ftabN >= FRAME_TABLE_MAX) { ftabOverflow++; return; }
        memset(&ftab[i], 0, sizeof ftab[i]);
        ftab[i].id = id; ftab[i].bus = bus; ftab[i].ext = ext; ftabN++;
    }
    FrameEnt *e = &ftab[i];
    e->dlc = dlc; STRLCPY(e->hex, hex);
    e->total++; e->win++; e->dirty = 1; e->decdirty = 1;
    busWinFrames[bus & 1]++; busWinBits[bus & 1] += ((ext ? 67u : 47u) + 8u * dlc) * 11u / 10u;
}

static void frameBatchFlush(void)
{
    static uint32_t lastBatch, lastWin;
    const uint32_t now = millis();
    if (now - lastWin >= 1000u) {
        const float s = (float)(now - lastWin) / 1000.0f; lastWin = now;
        for (int i = 0; i < ftabN; i++) { ftab[i].rate = (float)ftab[i].win / s; ftab[i].win = 0; ftab[i].dirty = 1; }
        for (int b = 0; b < 2; b++) { busFps[b] = (uint32_t)((float)busWinFrames[b] / s); busBitsPs[b] = (uint32_t)((float)busWinBits[b] / s);
                                      busWinFrames[b] = busWinBits[b] = 0; }
    }
    if (now - lastBatch < FRAME_BATCH_MS || ws_connected_clients() == 0u) return;
    lastBatch = now;
    /* Smaller than the ESP32's 12 KB buffer: entries that do not fit stay
     * dirty and go out in the next 250 ms batch, exactly as on the ESP32. */
    static char out[2048];
    int n = snprintf(out, sizeof out,
        "{\"type\":\"frames\",\"fps\":[%u,%u],\"bps\":[%u,%u],\"total\":%u,\"locked\":%s,\"bitrate\":%u,"
        "\"drop\":[%lu,%lu],\"over\":%lu,\"l\":[",
        (unsigned)busFps[0], (unsigned)busFps[1], (unsigned)busBitsPs[0], (unsigned)busBitsPs[1],
        (unsigned)framesSeen, locked ? "true" : "false", (unsigned)bitrate,
        (unsigned long)(s_capq[0] ? s_capq[0]->dropped : 0u), (unsigned long)(s_capq[1] ? s_capq[1]->dropped : 0u),
        (unsigned long)ftabOverflow);
    bool first = true;
    for (int i = 0; i < ftabN && n < (int)sizeof out - 96; i++) {
        FrameEnt *e = &ftab[i];
        if (!e->dirty) continue;
        e->dirty = 0;
        n += snprintf(out + n, sizeof out - (size_t)n, "%s[\"%0*X\",%u,%u,\"%s\",%u,%.1f]",
                      first ? "" : ",", e->ext ? 8 : 3, (unsigned)e->id, e->bus, e->dlc, e->hex,
                      (unsigned)e->total, (double)e->rate);
        first = false;
    }
    if (n < (int)sizeof out - 3) { out[n++] = ']'; out[n++] = '}'; out[n] = 0; ws_broadcast_txt(out, (size_t)n); }
}

/* ═══ DECODED BROADCAST SIGNALS (DBC table, F-CAN standard IDs, 500 ms) ═══
 * Uses the latest bytes already held in the frame table: no extra per-signal
 * state. Message: {"type":"dec","l":[[id,msg,sig,value,unit,text],...]}.  */
#define DEC_BATCH_MS 500u
static void decBatchFlush(void)
{
    static uint32_t lastDec; static int cursor;
    const uint32_t now = millis();
    if (now - lastDec < DEC_BATCH_MS || ws_connected_clients() == 0u) return;
    lastDec = now;
    static char out[2048];
    int n = snprintf(out, sizeof out, "{\"type\":\"dec\",\"l\":[");
    bool first = true, full = false;
    for (int k = 0; k < ftabN && !full; k++) {
        const int i = (cursor + k) % ftabN;
        FrameEnt *e = &ftab[i];
        if (!e->decdirty || e->bus != 0u || e->ext) continue;
        const can_dbc_msg_t *m = can_dbc_find(e->id);
        if (!m) { e->decdirty = 0; continue; }
        uint8_t d[8] = {0}; const uint8_t dlc = e->dlc;
        for (uint8_t b = 0; b < dlc && b < 8u && e->hex[b * 2u]; b++) {
            const char t[3] = { e->hex[b * 2u], e->hex[b * 2u + 1u], 0 };
            d[b] = (uint8_t)strtoul(t, NULL, 16);
        }
        const int mark = n; const bool firstMark = first;
        for (unsigned s = 0; s < m->sig_count; s++) {
            int64_t raw;
            if (!can_dbc_raw(&m->sigs[s], d, dlc, &raw)) continue;
            n += snprintf(out + n, sizeof out - (size_t)n, "%s[\"%03X\",\"%s\",\"%s\",%.6g,\"%s\",\"%s\"]",
                          first ? "" : ",", (unsigned)m->id, m->name, m->sigs[s].name,
                          can_dbc_phys(&m->sigs[s], raw), m->sigs[s].unit, can_dbc_text(&m->sigs[s], raw));
            first = false;
            if (n >= (int)sizeof out - 8) { n = mark; first = firstMark; full = true; cursor = i; break; }
        }
        if (!full) e->decdirty = 0;
    }
    if (first) return;                                     /* nothing decoded */
    out[n++] = ']'; out[n++] = '}'; out[n] = 0;
    ws_broadcast_txt(out, (size_t)n);
}

/* ═════════ DECODED-DATA LOGGER (1 Hz) + READ-ONLY OBSERVER (TCP 2323) ══ */
static const char LOG_HDR[] = "uptime_ms,utc,key,rpm,speed_kmh,rpm_src,acc_wire,pedal_17C_b0,fps_F,fps_B,pids\n";
static bool logFsOk;
static struct { char key[6]; int rpm; float spd; char src[6]; int acc; char pids[200]; } snap = { "?", 0, 0, "NONE", 0, "" };

/* DTC event journal (same rules as the ESP32 build). */
#define EVENT_MAX 40u
typedef struct {
    uint32_t id; char code[12]; char source[24]; char bus[2]; char ecu[9]; char status[20];
    char timestamp[24]; char lastAt[24]; bool seen; uint32_t repeats; char vehicleState[6];
    int rpm; float speed; char rpmSource[6]; int acc; char pids[200]; uint32_t frames;
    uint16_t fpsF, fpsB; bool snapshotReady; uint32_t lastReportedMs;
} DtcEvent;
static DtcEvent eventStore[EVENT_MAX] __attribute__((section(".ccmram")));
static size_t   eventCount;
static uint32_t nextEventId = 1;
#define EVENT_PRE_SAMPLES 60u
#define EVENT_POST_SAMPLES 10u
#define EVENT_SNAPSHOT_MAX 8u
#define EVENT_CAPTURE_MAX_ACTIVE 8u
#define EVENT_SAMPLE_MAX 400u
typedef struct { uint32_t id; uint8_t remaining; } PendingEventCapture;
static PendingEventCapture pendingEventCaptures[EVENT_CAPTURE_MAX_ACTIVE];
static size_t pendingEventCount;
typedef struct { char ecu[9]; char hex[1025]; size_t hexLen; uint16_t nextOffset; bool active; } DtcResponseBuffer;
static DtcResponseBuffer dtcResponse[2];

/* Pre-trigger ring: the last 60 one-second rows, stored back to back in a
 * byte ring (rows are ~100-250 bytes, not the ESP32's fixed 400). If the ring
 * is ever too small for 60 rows the oldest rows are dropped first. */
#define PRE_RING_BYTES 8192u
static char     preRing[PRE_RING_BYTES];
static uint16_t preLen[EVENT_PRE_SAMPLES];
static size_t   preHeadByte, preTailByte, preUsed, preFirstIdx, eventPreCount;

static void pre_drop_oldest(void)
{
    const size_t l = preLen[preFirstIdx];
    preTailByte = (preTailByte + l) % PRE_RING_BYTES; preUsed -= l;
    preFirstIdx = (preFirstIdx + 1u) % EVENT_PRE_SAMPLES; eventPreCount--;
}

static void pushEventPreSample(const char *csv, size_t n)
{
    if (!csv || n == 0u || n >= EVENT_SAMPLE_MAX) return;
    if (eventPreCount == EVENT_PRE_SAMPLES) pre_drop_oldest();
    while (preUsed + n > PRE_RING_BYTES && eventPreCount) pre_drop_oldest();
    for (size_t i = 0; i < n; i++) preRing[(preHeadByte + i) % PRE_RING_BYTES] = csv[i];
    preHeadByte = (preHeadByte + n) % PRE_RING_BYTES; preUsed += n;
    preLen[(preFirstIdx + eventPreCount) % EVENT_PRE_SAMPLES] = (uint16_t)n;
    eventPreCount++;
}

static DtcEvent *findEvent(uint32_t id)
{
    for (size_t i = 0; i < eventCount; ++i) if (eventStore[i].id == id) return &eventStore[i];
    return NULL;
}

static bool writeEventCsvRow(uint32_t id, const char *csv, size_t n)
{
    const DtcEvent *e = findEvent(id);
    if (!e) return false;
    char row[128 + EVENT_SAMPLE_MAX];
    const int p = snprintf(row, 128, "%lu,%s,%s,%s,%s,", (unsigned long)e->id, e->code, e->ecu, e->bus, e->timestamp);
    if (p <= 0 || p >= 128 || n > EVENT_SAMPLE_MAX) return false;
    memcpy(row + p, csv, n);
    return evst_append(EV_REC_SNAP_ROW, id, row, (uint16_t)((size_t)p + n));
}

/* persistence: one record per changed event, one per removed event */
static uint32_t savedId[EVENT_MAX], savedCrc[EVENT_MAX]; static size_t savedN;

static uint32_t crc32(const void *d, size_t n)
{
    const uint8_t *p = (const uint8_t *)d; uint32_t c = 0xFFFFFFFFu;
    while (n--) { c ^= *p++; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u))); }
    return ~c;
}
static uint32_t eventCrc(const DtcEvent *e) { return crc32(e, offsetof(DtcEvent, lastReportedMs)); }

static bool saveEvents(void)
{
    if (!logFsOk) return false;
    bool ok = true;
    for (size_t s = 0; s < savedN; s++) {                  /* removed events */
        if (findEvent(savedId[s])) continue;
        ok = ok && evst_append(EV_REC_EVENT_DEL, savedId[s], NULL, 0);
    }
    uint32_t newId[EVENT_MAX], newCrc[EVENT_MAX];
    for (size_t i = 0; i < eventCount; ++i) {
        const DtcEvent *e = &eventStore[i];
        const uint32_t c = eventCrc(e);
        bool same = false;
        for (size_t s = 0; s < savedN; s++) if (savedId[s] == e->id) { same = savedCrc[s] == c; break; }
        if (!same) ok = ok && evst_append(EV_REC_EVENT, e->id, e, (uint16_t)offsetof(DtcEvent, lastReportedMs));
        newId[i] = e->id; newCrc[i] = c;
    }
    if (!ok) return false;
    memcpy(savedId, newId, sizeof(uint32_t) * eventCount); memcpy(savedCrc, newCrc, sizeof(uint32_t) * eventCount);
    savedN = eventCount;
    return true;
}

/* compaction callbacks */
static bool rewriteEvents(void)
{
    for (size_t i = 0; i < eventCount; ++i)
        if (!evst_append(EV_REC_EVENT, eventStore[i].id, &eventStore[i], (uint16_t)offsetof(DtcEvent, lastReportedMs)))
            return false;
    return true;
}
static bool snapLive(uint32_t id)
{
    for (size_t i = 0; i < pendingEventCount; i++) if (pendingEventCaptures[i].id == id) return true;
    const DtcEvent *e = findEvent(id);
    return e && e->snapshotReady;
}

static void replayRecord(uint8_t type, uint32_t id, const uint8_t *d, uint16_t len)
{
    if (type == EV_REC_EVENT && len == offsetof(DtcEvent, lastReportedMs)) {
        DtcEvent *e = findEvent(id);
        if (!e) { if (eventCount >= EVENT_MAX) return; e = &eventStore[eventCount++]; }
        memcpy(e, d, len); e->lastReportedMs = 0;
    } else if (type == EV_REC_EVENT_DEL) {
        for (size_t i = 0; i < eventCount; i++) if (eventStore[i].id == id) {
            memmove(&eventStore[i], &eventStore[i + 1u], sizeof(DtcEvent) * (eventCount - i - 1u));
            eventCount--; break;
        }
    }
}

static bool startEventCapture(uint32_t id)
{
    if (!logFsOk || stm32FlashRunning || pendingEventCount >= EVENT_CAPTURE_MAX_ACTIVE) return false;
    size_t retained = pendingEventCount;
    for (size_t i = 0; i < eventCount; ++i) if (eventStore[i].snapshotReady) ++retained;
    if (retained >= EVENT_SNAPSHOT_MAX) {
        bool released = false;
        for (size_t i = 0; i < eventCount; ++i) {
            DtcEvent *old = &eventStore[i];
            if (!old->seen || !old->snapshotReady) continue;   /* never discard an unseen alarm's evidence */
            (void)evst_append(EV_REC_SNAP_DEL, old->id, NULL, 0);
            old->snapshotReady = false; released = true;
            (void)saveEvents(); break;
        }
        if (!released) return false;
    }
    const size_t needed = (eventPreCount + EVENT_POST_SAMPLES) * EVENT_SAMPLE_MAX + sizeof LOG_HDR + 4096u;
    if (evst_free_bytes() < needed) { (void)evst_compact(); if (evst_free_bytes() < needed) return false; }
    bool ok = true;
    for (size_t i = 0, idx = preFirstIdx, off = preTailByte; ok && i < eventPreCount; ++i) {
        const size_t n = preLen[idx];
        char row[EVENT_SAMPLE_MAX];
        for (size_t k = 0; k < n; k++) row[k] = preRing[(off + k) % PRE_RING_BYTES];
        if (n && !writeEventCsvRow(id, row, n)) ok = false;
        off = (off + n) % PRE_RING_BYTES; idx = (idx + 1u) % EVENT_PRE_SAMPLES;
    }
    if (!ok) { (void)evst_append(EV_REC_SNAP_DEL, id, NULL, 0); return false; }
    PendingEventCapture *pending = &pendingEventCaptures[pendingEventCount++];
    pending->id = id; pending->remaining = EVENT_POST_SAMPLES;
    return true;
}

static void broadcastEvents(void);

static void capturePostEventSample(const char *csv, size_t n)
{
    for (size_t i = 0; i < pendingEventCount; ) {
        PendingEventCapture *pending = &pendingEventCaptures[i];
        const bool ok = writeEventCsvRow(pending->id, csv, n);
        DtcEvent *e = findEvent(pending->id);
        if (!ok || !e || pending->remaining <= 1u) {
            if (e && ok && pending->remaining <= 1u) {
                e->snapshotReady = true;
                if (saveEvents()) broadcastEvents();
                else e->snapshotReady = false;          /* EVENTS: capture completion metadata not persisted */
            } else if (e) {
                e->snapshotReady = false;               /* EVENTS: post-trigger capture failed */
                (void)evst_append(EV_REC_SNAP_DEL, pending->id, NULL, 0);
                (void)saveEvents(); broadcastEvents();
            }
            pendingEventCaptures[i] = pendingEventCaptures[--pendingEventCount];
            continue;
        }
        --pending->remaining;
        ++i;
    }
}

static void eventUtcNow(char *out, size_t cap)
{
    /* No NTP source (the ESP32 AP had none either): uptime stamp. */
    snprintf(out, cap, "UP+%lu", (unsigned long)millis());
}

static void loadEvents(void)
{
    /* records were replayed by evst_init(); apply the ESP32 load rules */
    nextEventId = 1;
    for (size_t i = 0; i < eventCount; ++i) {
        DtcEvent *e = &eventStore[i];
        const bool exists = evst_snap_exists(e->id);
        if (!exists) e->snapshotReady = false;
        else if (!e->snapshotReady) (void)evst_append(EV_REC_SNAP_DEL, e->id, NULL, 0); /* incomplete capture */
        if (e->id >= nextEventId) nextEventId = e->id + 1u;
        savedId[i] = e->id; savedCrc[i] = eventCrc(e);
    }
    savedN = eventCount;
}

static size_t eventJson(const DtcEvent *e, jb_t *j)
{
    jb_begin(j, s_jbuf, sizeof s_jbuf);
    jb_str(j, "type", "event"); jb_uint(j, "id", e->id);
    jb_str(j, "code", e->code); jb_str(j, "source", e->source); jb_str(j, "bus", e->bus);
    jb_str(j, "ecu", e->ecu); jb_str(j, "status", e->status); jb_str(j, "timestamp", e->timestamp);
    jb_str(j, "lastAt", e->lastAt); jb_bool(j, "seen", e->seen); jb_uint(j, "repeats", e->repeats);
    jb_str(j, "vehicleState", e->vehicleState); jb_int(j, "rpm", e->rpm); jb_num(j, "speed", e->speed, 7);
    jb_str(j, "rpmSource", e->rpmSource); jb_int(j, "acc", e->acc); jb_str(j, "pids", e->pids);
    jb_uint(j, "frames", e->frames); jb_uint(j, "fpsF", e->fpsF); jb_uint(j, "fpsB", e->fpsB);
    jb_bool(j, "snapshotReady", e->snapshotReady);
    return jb_end(j);
}

/* Event delivery: a full "events_reset + every event" burst is up to ~20 KB,
 * more than one TCP send window, so each client has a resend cursor that the
 * poll loop advances as the window opens (same messages, same order). */
static int16_t evCursor[WS_MAX_CLIENTS] = { -2, -2, -2, -2 };   /* -2 idle, -1 reset pending, k next event */

static void sendAllEvents(uint8_t client) { if (client < WS_MAX_CLIENTS) evCursor[client] = -1; }
static void broadcastEvents(void) { for (uint8_t i = 0; i < WS_MAX_CLIENTS; i++) evCursor[i] = -1; }

static void eventsPump(void)
{
    for (uint8_t c = 0; c < WS_MAX_CLIENTS; c++) {
        for (int guard = 0; evCursor[c] != -2 && guard < 8; guard++) {
            if (evCursor[c] == -1) {
                static const char reset[] = "{\"type\":\"events_reset\"}";
                if (!ws_send_txt_ok(c, reset, sizeof reset - 1u)) break;
                evCursor[c] = 0;
            } else if ((size_t)evCursor[c] >= eventCount) { evCursor[c] = -2; }
            else {
                jb_t j; const size_t n = eventJson(&eventStore[evCursor[c]], &j);
                if (n && !ws_send_txt_ok(c, s_jbuf, n)) break;
                evCursor[c]++;
            }
        }
    }
}

static void addDtcEvent(const char *bus, const char *ecu, const char *code, const char *source, const char *status)
{
    DtcEvent *latest = NULL;
    for (size_t i = eventCount; i > 0; --i) {
        DtcEvent *candidate = &eventStore[i - 1u];
        if (strcmp(candidate->bus, bus) == 0 && strcmp(candidate->ecu, ecu) == 0 && strcmp(candidate->code, code) == 0) {
            latest = candidate; break;
        }
    }
    if (latest && (!latest->seen ||
        (latest->lastReportedMs != 0u && (uint32_t)(millis() - latest->lastReportedMs) < 120000u))) {
        ++latest->repeats;
        STRLCPY(latest->source, source);
        STRLCPY(latest->status, status);
        eventUtcNow(latest->lastAt, sizeof latest->lastAt);
        latest->lastReportedMs = millis();
        (void)saveEvents();
        broadcastEvents(); return;
    }
    if (eventCount == EVENT_MAX) {
        size_t victim = eventCount;
        for (size_t i = 0; i < eventCount; ++i) if (eventStore[i].seen) { victim = i; break; }
        if (victim == eventCount) {
            jb_t j; jb_begin(&j, s_jbuf, sizeof s_jbuf);
            jb_str(&j, "type", "diag"); jb_str(&j, "message", "EVENTS,ERR,40_UNACKNOWLEDGED_CAPACITY");
            ws_broadcast_doc(&j);
            return;
        }
        const uint32_t oldId = eventStore[victim].id;
        (void)evst_append(EV_REC_SNAP_DEL, oldId, NULL, 0);
        for (size_t i = 0; i < pendingEventCount; ) {
            if (pendingEventCaptures[i].id == oldId) pendingEventCaptures[i] = pendingEventCaptures[--pendingEventCount];
            else ++i;
        }
        if (victim + 1u < eventCount)
            memmove(&eventStore[victim], &eventStore[victim + 1u], sizeof(DtcEvent) * (eventCount - victim - 1u));
        --eventCount;
    }
    DtcEvent *e = &eventStore[eventCount++]; memset(e, 0, sizeof *e);
    e->id = nextEventId++; STRLCPY(e->code, code);
    STRLCPY(e->source, source); STRLCPY(e->bus, bus);
    STRLCPY(e->ecu, ecu); STRLCPY(e->status, status);
    eventUtcNow(e->timestamp, sizeof e->timestamp);
    memcpy(e->lastAt, e->timestamp, sizeof e->lastAt); e->lastReportedMs = millis(); e->repeats = 1;
    STRLCPY(e->vehicleState, snap.key); e->rpm = snap.rpm;
    e->speed = snap.spd; STRLCPY(e->rpmSource, snap.src);
    e->acc = snap.acc; STRLCPY(e->pids, snap.pids);
    e->frames = framesSeen; e->fpsF = (uint16_t)busFps[0]; e->fpsB = (uint16_t)busFps[1];
    e->snapshotReady = false;
    (void)startEventCapture(e->id);
    (void)saveEvents();
    broadcastEvents();
}

static void captureDtcReply(const char *busField, const char *ecuField, const char *payload)
{
    const size_t pl = strlen(payload);
    if ((pl & 1u) || pl < 4u) return;
    for (size_t i = 0; i < pl; ++i) if (!isxdigit((unsigned char)payload[i])) return;
    uint8_t b[512]; size_t n = 0;
    for (size_t i = 0; i + 1u < pl && n < sizeof b; i += 2u) {
        char t[3] = { payload[i], payload[i + 1u], 0 }; b[n++] = (uint8_t)strtoul(t, NULL, 16);
    }
    if (!n) return;
    char bus[2] = { strcmp(busField, "B") == 0 ? 'B' : 'F', 0 };
    char ecu[9]; STRLCPY(ecu, ecuField);
    const uint8_t sid = b[0];
    if (sid == 0x43u || sid == 0x47u || sid == 0x4Au) {
        const char *source = sid == 0x43u ? "OBD stored" : (sid == 0x47u ? "OBD pending" : "OBD permanent");
        size_t i = 1;
        if (((n - 1u) & 1u) != 0u) i = 2;
        for (; i + 1u < n; i += 2u) {
            if ((b[i] | b[i + 1u]) == 0u) continue;
            char code[8]; snprintf(code, sizeof code, "%c%u%1X%02X", "PCBU"[b[i] >> 6],
                                   (unsigned)((b[i] >> 4) & 3u), (unsigned)(b[i] & 15u), (unsigned)b[i + 1u]);
            addDtcEvent(bus, ecu, code, source, "reported by ECU");
        }
    } else if (sid == 0x59u && n >= 7u && b[1] == 0x02u) {
        for (size_t i = 3; i + 3u < n; i += 4u) {
            if ((b[i] | b[i + 1u] | b[i + 2u]) == 0u) continue;
            char code[12]; snprintf(code, sizeof code, "%c%u%1X%02X-%02X", "PCBU"[b[i] >> 6],
                                    (unsigned)((b[i] >> 4) & 3u), (unsigned)(b[i] & 15u),
                                    (unsigned)b[i + 1u], (unsigned)b[i + 2u]);
            char status[20]; snprintf(status, sizeof status, "status 0x%02X", (unsigned)b[i + 3u]);
            addDtcEvent(bus, ecu, code, "UDS 19/02", status);
        }
    }
}

static void captureDtcChunk(const char *busField, const char *ecuField, const char *offsetField, const char *chunk)
{
    const uint8_t bus = strcmp(busField, "B") == 0 ? 1u : 0u;
    DtcResponseBuffer *r = &dtcResponse[bus];
    const size_t cl = strlen(chunk);
    if (cl == 0u || (cl & 1u) || cl > sizeof r->hex - 1u) { r->active = false; return; }
    for (size_t i = 0; i < cl; ++i) if (!isxdigit((unsigned char)chunk[i])) { r->active = false; return; }
    const long offsetValue = strtol(offsetField, NULL, 10);
    if (offsetValue < 0 || offsetValue > 4095) { r->active = false; return; }
    const uint16_t offset = (uint16_t)offsetValue;
    if (offset == 0u) {
        r->hexLen = 0; r->nextOffset = 0; r->active = true;
        STRLCPY(r->ecu, ecuField); r->hex[0] = '\0';
    }
    if (!r->active || strcmp(r->ecu, ecuField) != 0 || offset != r->nextOffset ||
        r->hexLen + cl >= sizeof r->hex) { r->active = false; return; }
    memcpy(r->hex + r->hexLen, chunk, cl);
    r->hexLen += cl; r->hex[r->hexLen] = '\0';
    r->nextOffset = (uint16_t)(r->nextOffset + cl / 2u);
}

static void completeDtcResponse(const char *busField, const char *ecuField)
{
    const uint8_t bus = strcmp(busField, "B") == 0 ? 1u : 0u;
    DtcResponseBuffer *r = &dtcResponse[bus];
    if (r->active && strcmp(r->ecu, ecuField) == 0 && r->hexLen >= 4u) {
        captureDtcReply(busField, ecuField, r->hex);   /* parsed before the buffer is reset below */
    }
    r->active = false; r->hexLen = 0; r->nextOffset = 0; r->hex[0] = '\0';
}

static bool markEventSeen(uint32_t id)
{
    for (size_t i = 0; i < eventCount; ++i) {
        if (eventStore[i].id != id) continue;
        if (eventStore[i].seen) return true;
        eventStore[i].seen = true;
        if (!saveEvents()) { eventStore[i].seen = false; return false; }
        broadcastEvents(); return true;
    }
    return false;
}

static void snapPid(const char *pid, float v)
{
    char out[sizeof snap.pids + 32]; size_t on = 0; const size_t pl = strlen(pid);
    out[0] = '\0';
    for (const char *p = snap.pids; *p; ) {
        const char *e = strchr(p, ';'); if (!e) break;
        if (!(strncmp(p, pid, pl) == 0 && p[pl] == '=')) {
            const size_t l = (size_t)(e - p + 1);
            if (on + l < sizeof out) { memcpy(out + on, p, l); on += l; out[on] = '\0'; }
        }
        p = e + 1;
    }
    char kv[32]; const int k = snprintf(kv, sizeof kv, "%s=%.2f;", pid, (double)v);
    if (k > 0 && on + (size_t)k < sizeof out) { memcpy(out + on, kv, (size_t)k + 1u); on += (size_t)k; }
    if (on < sizeof snap.pids) memcpy(snap.pids, out, on + 1u);
}

/* ── TCP 2323 observer: write-only, newest client wins, input discarded ── */
static struct tcp_pcb *obsCli;
static void obs_close(void) { if (obsCli) { tcp_arg(obsCli, NULL); tcp_recv(obsCli, NULL); tcp_err(obsCli, NULL); if (tcp_close(obsCli) != ERR_OK) tcp_abort(obsCli); obsCli = NULL; } }
static void obsWrite(const char *s, size_t n)
{
    if (obsCli && tcp_sndbuf(obsCli) >= n) { (void)tcp_write(obsCli, s, (u16_t)n, TCP_WRITE_FLAG_COPY); tcp_output(obsCli); }
}
static err_t obs_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    (void)arg; (void)err;
    if (!p) { if (pcb == obsCli) obs_close(); else tcp_close(pcb); return ERR_OK; }
    tcp_recved(pcb, p->tot_len); pbuf_free(p);               /* read-only: discard input */
    return ERR_OK;
}
static void obs_err(void *arg, err_t err) { (void)arg; (void)err; obsCli = NULL; }
static err_t obs_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    (void)arg;
    if (err != ERR_OK || !pcb) return ERR_VAL;
    obs_close();                                             /* one observer; newest wins */
    obsCli = pcb; tcp_nagle_disable(pcb);
    tcp_recv(pcb, obs_recv); tcp_err(pcb, obs_err);
    obsWrite(LOG_HDR, strlen(LOG_HDR));
    return ERR_OK;
}

static void logService(void)
{
    static uint32_t last; const uint32_t now = millis();
    if (now - last < 1000u) return;
    last = now;
    const char *utc = "";
    int pedal = -1;
    for (int i = 0; i < ftabN; i++)
        if (ftab[i].bus == 0 && !ftab[i].ext && ftab[i].id == 0x17Cu && strlen(ftab[i].hex) >= 2u) {
            char b[3] = { ftab[i].hex[0], ftab[i].hex[1], 0 }; pedal = (int)strtol(b, NULL, 16); }
    char line[400];
    const int n = snprintf(line, sizeof line, "%lu,%s,%s,%d,%.2f,%s,%d,%d,%u,%u,%s\n",
                           (unsigned long)now, utc, snap.key, snap.rpm, (double)snap.spd, snap.src, snap.acc,
                           pedal, (unsigned)busFps[0], (unsigned)busFps[1], snap.pids);
    if (n <= 0 || n >= (int)sizeof line) return;
    obsWrite(line, (size_t)n);
    pushEventPreSample(line, (size_t)n);
    if (!logFsOk || stm32FlashRunning) return;
    capturePostEventSample(line, (size_t)n);
    (void)logst_append(LOG_HDR, line, (size_t)n);
}

/* ═══════════════════════════════ STATUS ════════════════════════════════ */
static void emitStatus(void)
{
    jb_t j; jb_begin(&j, s_jbuf, sizeof s_jbuf);
    jb_str(&j, "type", "status"); jb_bool(&j, "locked", locked);
    jb_int(&j, "bitrate", (int)bitrate); jb_int(&j, "frames", (int)framesSeen); jb_int(&j, "dwell", (int)dwellMs);
    ws_broadcast_doc(&j);
}

/* ═════════════════════ GVRET over TCP (SavvyCAN / RealDash, port 23) ═══ */
static struct tcp_pcb *gvretCli;
static struct gvret_parser gvretParser;

static void gvretWrite(const uint8_t *b, size_t n)
{
    if (gvretCli && n && tcp_sndbuf(gvretCli) >= n) (void)tcp_write(gvretCli, b, (u16_t)n, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
}

/* Live controller state for GVRET; main.c provides the real one. */
__attribute__((weak)) bool project_bus_state(uint8_t bus, uint32_t *bit_rate, bool *listen_only)
{
    (void)bus; *bit_rate = 0u; *listen_only = true;
    return false;
}

static void gvretHandle(const uint8_t *data, size_t len)
{
    for (size_t k = 0; k < len; k++) {
        struct gvret_command gc;
        if (!gvret_parser_feed(&gvretParser, data[k], &gc)) continue;
        uint8_t rb[16]; size_t rn = 0;
        switch (gc.type) {
        case GVRET_CMD_KEEPALIVE:        rn = gvret_encode_keepalive_reply(rb, sizeof rb); gvretWrite(rb, rn); break;
        case GVRET_CMD_GET_DEVICE_INFO:  rn = gvret_encode_device_info(rb, sizeof rb, 0x0001); gvretWrite(rb, rn); break;
        case GVRET_CMD_GET_NUMBUSES:     rn = gvret_encode_numbuses(rb, sizeof rb, 2); gvretWrite(rb, rn); break;
        case GVRET_CMD_TIME_SYNC:        rn = gvret_encode_time_sync(rb, sizeof rb, TIM2->CNT); gvretWrite(rb, rn); break;
        case GVRET_CMD_GET_CANBUS_PARAMS: {
            uint32_t b0, b1; bool l0, l1;
            const bool e0 = project_bus_state(0u, &b0, &l0), e1 = project_bus_state(1u, &b1, &l1);
            uint8_t pb[16]; const size_t pn = gvret_encode_canbus_params_lo(pb, sizeof pb, e0, l0, b0, e1, l1, b1);
            gvretWrite(pb, pn); break;
        }
        case GVRET_CMD_SEND_FRAME: {
            char tb[64];
            const char *b = (gc.u.frame.bus == 1) ? "B" : "F";
            if (gc.u.frame.dlc == 0) {
                snprintf(tb, sizeof tb, "TX,%s,%s,%lX,0,0,-", b, gc.u.frame.ext ? "E" : "S", (unsigned long)gc.u.frame.id);
            } else {
                char hx[17]; for (int i = 0; i < gc.u.frame.dlc; i++) snprintf(hx + i * 2, 3, "%02X", gc.u.frame.data[i]);
                snprintf(tb, sizeof tb, "TX,%s,%s,%lX,0,%u,%s", b, gc.u.frame.ext ? "E" : "S",
                         (unsigned long)gc.u.frame.id, gc.u.frame.dlc, hx);
            }
            stm32_print(tb); break;
        }
        default: break;
        }
    }
    if (gvretCli) tcp_output(gvretCli);
}

static void gvret_close(void) { if (gvretCli) { tcp_arg(gvretCli, NULL); tcp_recv(gvretCli, NULL); tcp_err(gvretCli, NULL); if (tcp_close(gvretCli) != ERR_OK) tcp_abort(gvretCli); gvretCli = NULL; } }
static err_t gvret_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    (void)arg; (void)err;
    if (!p) { gvret_close(); return ERR_OK; }
    tcp_recved(pcb, p->tot_len);
    for (struct pbuf *q = p; q; q = q->next) gvretHandle((const uint8_t *)q->payload, q->len);
    pbuf_free(p);
    return ERR_OK;
}
static void gvret_err(void *arg, err_t err) { (void)arg; (void)err; gvretCli = NULL; }
static err_t gvret_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    (void)arg;
    if (err != ERR_OK || !pcb) return ERR_VAL;
    if (gvretCli) { tcp_abort(pcb); return ERR_ABRT; }          /* one GVRET client at a time */
    gvretCli = pcb; gvret_parser_init(&gvretParser);
    tcp_nagle_disable(pcb);
    tcp_recv(pcb, gvret_recv); tcp_err(pcb, gvret_err);
    return ERR_OK;
}

/* ═══════════════════════════════ LINE PARSER ═══════════════════════════ */
static void diag(const char *raw) { broadcast_type_msg("diag", raw); }

static void diag(const char *raw);
static void parseLine(const char *raw, int len)
{
    char f1[24], f2[24], f3[48], f4[48], f5[1100];
    if (len == 0) return;
    if (strncmp(raw, "TX,", 3) == 0) { broadcast_type_msg("tx", raw); return; }
    if (strncmp(raw, "GEAR,", 5) == 0) { diag(raw); return; }       /* LED chase acks / broadcast gear */
    if (strncmp(raw, "UDS,", 4) == 0 || strncmp(raw, "CMD,", 4) == 0 ||
        strncmp(raw, "ECU,", 4) == 0 || strncmp(raw, "START,", 6) == 0) {
        if (strncmp(raw, "UDS,", 4) == 0) {
            csvField(raw, 1, f1, sizeof f1); csvField(raw, 2, f2, sizeof f2);
            if (strcmp(f2, "DATA") == 0)
                captureDtcChunk(f1, csvField(raw, 3, f3, sizeof f3), csvField(raw, 4, f4, sizeof f4), csvField(raw, 5, f5, sizeof f5));
            else if (strcmp(f2, "POS") == 0)
                completeDtcResponse(f1, csvField(raw, 3, f3, sizeof f3));
            else if (strcmp(f2, "NEG") == 0 || strcmp(f2, "ERR") == 0 || strcmp(f2, "TXERR") == 0) {
                const uint8_t bus = strcmp(f1, "B") == 0 ? 1u : 0u;
                dtcResponse[bus].active = false; dtcResponse[bus].hexLen = 0;
            }
        }
        diag(raw); return;
    }
    if (strncmp(raw, "STATUS,", 7) == 0) {
        csvField(raw, 1, f1, sizeof f1); csvField(raw, 2, f2, sizeof f2);
        const bool isF = strcmp(f1, "F") == 0;
        if (strcmp(f1, "SYS") == 0 && strcmp(f2, "VEH") == 0) {
            jb_t j; jb_begin(&j, s_jbuf, sizeof s_jbuf);
            jb_str(&j, "type", "veh"); jb_str(&j, "state", csvField(raw, 3, f3, sizeof f3));
            jb_int(&j, "rpm", fieldInt(raw, 4)); jb_num(&j, "spd", (double)fieldInt(raw, 5) / 100.0, 15);
            jb_int(&j, "acc", fieldInt(raw, 6)); jb_str(&j, "src", csvField(raw, 7, f4, sizeof f4));
            STRLCPY(snap.key, f3);
            snap.rpm = (int)fieldInt(raw, 4); snap.spd = (float)fieldInt(raw, 5) / 100.0f;
            snap.acc = (int)fieldInt(raw, 6); STRLCPY(snap.src, f4);
            ws_broadcast_doc(&j); return;
        }
        if (strcmp(f1, "SYS") == 0) { diag(raw); return; }
        if (strcmp(f2, "STATS") == 0) {
            jb_t j; jb_begin(&j, s_jbuf, sizeof s_jbuf);
            jb_str(&j, "type", "stats"); jb_str(&j, "bus", f1); jb_str(&j, "message", len > 15 ? raw + 15 : "");
            ws_broadcast_doc(&j); return;
        }
        if (strcmp(f2, "LOCK") == 0)           { const uint32_t hz = (uint32_t)fieldInt(raw, 3); if (isF) { bitrate = hz; locked = true; emitStatus(); } }
        else if (strcmp(f2, "BITRATE") == 0)   { const uint32_t hz = (uint32_t)fieldInt(raw, 3); if (isF) { bitrate = hz; locked = true; emitStatus(); } }
        else if (strcmp(f2, "SCAN") == 0)      { const uint32_t hz = (uint32_t)fieldInt(raw, 3); if (isF) { bitrate = hz; locked = false; emitStatus(); } }
        else if (strcmp(f2, "REACQUIRE") == 0) { if (isF) { locked = false; bitrate = 0; emitStatus(); } }
        else if (strcmp(f2, "DWELL") == 0)     { if (isF) { dwellMs = (uint32_t)fieldInt(raw, 3); emitStatus(); } }
        diag(raw); return;
    }

    const int commas = countCommas(raw);
    if (strncmp(raw, "PROFILE,", 8) == 0 && commas >= 5) {
        char id[16], vm[32], lbl[64];
        csvField(raw, 1, id, sizeof id); upper(id); csvField(raw, 4, vm, sizeof vm); upper(vm);
        const int dlc = (int)fieldInt(raw, 2);
        const float rate = (float)atof(csvField(raw, 3, f3, sizeof f3)) / 10.0f;
        csvField(raw, 5, lbl, sizeof lbl);
        static char pbuf[256];
        const int pn = snprintf(pbuf, sizeof pbuf, "{\"type\":\"profile\",\"id\":\"%s\",\"dlc\":%d,"
                                "\"rate\":%.1f,\"var_mask\":\"%s\",\"label\":\"%s\"}", id, dlc, (double)rate, vm, lbl);
        if (pn > 0 && pn < (int)sizeof pbuf) ws_broadcast_txt(pbuf, (size_t)pn);
        return;
    }
    if (strncmp(raw, "SIG,", 4) == 0 && commas >= 7) {
        char id[16], l[16], mn[16], mx[16], lbl[64];
        csvField(raw, 1, id, sizeof id); upper(id); csvField(raw, 3, l, sizeof l); upper(l);
        csvField(raw, 4, mn, sizeof mn); upper(mn); csvField(raw, 5, mx, sizeof mx); upper(mx);
        const int byt = (int)fieldInt(raw, 2);
        const float rate = (float)atof(csvField(raw, 6, f3, sizeof f3)) / 10.0f;
        csvField(raw, 7, lbl, sizeof lbl);
        static char sbuf[256];
        const int sn = snprintf(sbuf, sizeof sbuf, "{\"type\":\"sig\",\"id\":\"%s\",\"byte\":%d,"
                                "\"last\":\"%s\",\"min\":\"%s\",\"max\":\"%s\",\"rate\":%.1f,\"label\":\"%s\"}",
                                id, byt, l, mn, mx, (double)rate, lbl);
        if (sn > 0 && sn < (int)sizeof sbuf) ws_broadcast_txt(sbuf, (size_t)sn);
        return;
    }
    if (strncmp(raw, "PID,", 4) == 0 && commas >= 4) {
        char pid[16], lbl[64], unt[24];
        csvField(raw, 1, pid, sizeof pid); upper(pid);
        const float val = (float)atof(csvField(raw, 4, f3, sizeof f3));
        csvField(raw, 2, lbl, sizeof lbl); csvField(raw, 3, unt, sizeof unt);
        snapPid(pid, val);
        static char pidbuf[256];
        const int pn = snprintf(pidbuf, sizeof pidbuf, "{\"type\":\"pid\",\"pid\":\"%s\",\"label\":\"%s\","
                                "\"unit\":\"%s\",\"value\":%.4f}", pid, lbl, unt, (double)val);
        if (pn > 0 && pn < (int)sizeof pidbuf) ws_broadcast_txt(pidbuf, (size_t)pn);
        return;
    }
    if (commas >= 5) {
        char ts[16], fmt[8], id[16], dlcS[8], rtrS[8], pay[24], busS[8];
        csvField(raw, 0, ts, sizeof ts); csvField(raw, 1, fmt, sizeof fmt); upper(fmt);
        csvField(raw, 2, id, sizeof id); upper(id); csvField(raw, 3, dlcS, sizeof dlcS); csvField(raw, 4, rtrS, sizeof rtrS);
        csvField(raw, 5, pay, sizeof pay); upper(pay); csvField(raw, 6, busS, sizeof busS);
        if (!ts[0] || !id[0]) return;
        if (strcmp(fmt, "STD") != 0 && strcmp(fmt, "EXT") != 0) return;
        const int dlc = atoi(dlcS); if (dlc < 0 || dlc > 8) return;
        int busN = atoi(busS); if (busN < 0 || busN > 1) busN = 0;
        const bool ext = strcmp(fmt, "EXT") == 0, rtr = atoi(rtrS) != 0;
        framesSeen++;
        const uint32_t idv = (uint32_t)strtoul(id, NULL, 16);
        frameTableUpdate(idv, ext, (uint8_t)busN, (uint8_t)dlc, rtr ? "" : pay);
        if (gvretCli) {
            uint8_t d2[8] = {0};
            if (!rtr) for (int i = 0; i < dlc && (size_t)(i * 2 + 1) < strlen(pay); i++) {
                char t[3] = { pay[i * 2], pay[i * 2 + 1], 0 }; d2[i] = (uint8_t)strtoul(t, NULL, 16); }
            uint8_t gb[GVRET_MAX_FRAME_BYTES];
            const size_t gn = gvret_encode_frame(gb, sizeof gb, (uint8_t)busN, idv, ext, rtr, (uint8_t)dlc, d2,
                                                 (uint32_t)strtoul(ts, NULL, 10));
            gvretWrite(gb, gn);
        }
        return;
    }
}

void dash_on_line(const char *line)
{
    static char buf[1224];
    size_t n = strlen(line);
    while (n && (line[n - 1u] == '\n' || line[n - 1u] == '\r')) n--;
    if (n == 0u || n >= sizeof buf) return;                  /* over-long lines are dropped (as on ESP32) */
    if (__get_IPSR() != 0u) return;                          /* main-loop context only */
    memcpy(buf, line, n); buf[n] = '\0';
    parseLine(buf, (int)n);
}

/* ═══════════════════════════ WS command handler ════════════════════════ */
static jsmntok_t s_tok[48]; static int s_ntok; static const char *s_js;

static int tok_skip(int i)
{
    int n = 1;
    if (s_tok[i].type == JSMN_OBJECT) { int k = i + 1; for (int c = 0; c < s_tok[i].size; c++) { k += tok_skip(k); k += tok_skip(k); } return k - i; }
    if (s_tok[i].type == JSMN_ARRAY)  { int k = i + 1; for (int c = 0; c < s_tok[i].size; c++) k += tok_skip(k); return k - i; }
    return n;
}
static int jfind(const char *key)
{
    if (s_ntok < 1 || s_tok[0].type != JSMN_OBJECT) return -1;
    int i = 1; const size_t kl = strlen(key);
    int found = -1;
    for (int c = 0; c < s_tok[0].size && i < s_ntok; c++) {
        const jsmntok_t *k = &s_tok[i];
        if (k->type == JSMN_STRING && (size_t)(k->end - k->start) == kl && strncmp(s_js + k->start, key, kl) == 0) found = i + 1;
        i += tok_skip(i); if (i >= s_ntok) break;
        i += tok_skip(i);
    }
    return found;            /* last duplicate wins, like ArduinoJson */
}
static const char *jstr(const char *key, const char *def, char *out, size_t cap)
{
    const int t = jfind(key);
    if (t < 0 || s_tok[t].type != JSMN_STRING) { strncpy(out, def, cap - 1u); out[cap - 1u] = '\0'; return out; }
    size_t o = 0;
    for (int i = s_tok[t].start; i < s_tok[t].end && o + 1u < cap; i++) {
        char ch = s_js[i];
        if (ch == '\\' && i + 1 < s_tok[t].end) {
            ch = s_js[++i];
            if (ch == 'n') ch = '\n'; else if (ch == 't') ch = '\t'; else if (ch == 'r') ch = '\r';
            else if (ch == 'b') ch = '\b'; else if (ch == 'f') ch = '\f';
            else if (ch == 'u') { i += 4; ch = '?'; }
        }
        out[o++] = ch;
    }
    out[o] = '\0';
    return out;
}
static bool jprim(int t, const char **p, size_t *n) { if (t < 0 || s_tok[t].type != JSMN_PRIMITIVE) return false; *p = s_js + s_tok[t].start; *n = (size_t)(s_tok[t].end - s_tok[t].start); return true; }
static long jint(const char *key, long def)
{
    const char *p; size_t n;
    if (!jprim(jfind(key), &p, &n) || n == 0u || n > 20u) return def;
    for (size_t i = 0; i < n; i++) if (!(isdigit((unsigned char)p[i]) || (i == 0 && p[i] == '-'))) return def;
    char b[24]; memcpy(b, p, n); b[n] = '\0';
    return strtol(b, NULL, 10);
}
static bool jbool(const char *key, bool def)
{
    const char *p; size_t n;
    if (!jprim(jfind(key), &p, &n)) return def;
    if (n == 4u && strncmp(p, "true", 4) == 0) return true;
    if (n == 5u && strncmp(p, "false", 5) == 0) return false;
    return def;
}

static bool isHexStr(const char *v, size_t minLen, size_t maxLen)
{
    const size_t l = strlen(v);
    if (l < minLen || l > maxLen) return false;
    for (size_t i = 0; i < l; i++) if (!isxdigit((unsigned char)v[i])) return false;
    return true;
}

static void wsReply(uint8_t num, const char *type, const char *msg)
{
    jb_t j; jb_begin(&j, s_jbuf, sizeof s_jbuf);
    jb_str(&j, "type", type); jb_str(&j, "message", msg);
    ws_send_doc(num, &j);
}

static bool wsAuthed[WS_MAX_CLIENTS];

void dash_ws_connected(uint8_t num)
{
    if (num >= WS_MAX_CLIENTS) return;
    wsAuthed[num] = true; emitStatus(); sendAllEvents(num);
}

void dash_ws_disconnected(uint8_t num)
{
    if (num >= WS_MAX_CLIENTS) return;
    wsAuthed[num] = false; evCursor[num] = -2;
    bool any = false;
    for (int i = 0; i < (int)WS_MAX_CLIENTS; i++) any = any || wsAuthed[i];
    /* No operator left: end any ARM window on both buses. */
    if (!any && !stm32FlashRunning) { stm32_print("DISARM,F"); stm32_print("DISARM,B"); }
}

void dash_ws_text(uint8_t num, char *payload, size_t length)
{
    if (num >= WS_MAX_CLIENTS || length == 0u || length > WS_MAX_MESSAGE) return;
    jsmn_parser jp; jsmn_init(&jp);
    s_js = payload;
    s_ntok = jsmn_parse(&jp, payload, length, s_tok, sizeof s_tok / sizeof s_tok[0]);
    if (s_ntok < 1 || s_tok[0].type != JSMN_OBJECT) return;

    char op[24], busS[8], fmt[8], id[16], data[24], v[24];
    jstr("op", "", op, sizeof op);
    jstr("bus", "F", busS, sizeof busS);
    const char *bus = (strcmp(busS, "B") == 0 || strcmp(busS, "1") == 0) ? "B" : "F";
    char buf[260];

    if (strcmp(op, "events_get") == 0) { sendAllEvents(num); return; }
    if (strcmp(op, "event_seen") == 0) {
        const long idv = jint("id", 0);
        if (idv <= 0 || !markEventSeen((uint32_t)idv)) wsReply(num, "diag", "EVENT,ERR,NOT_FOUND_OR_SAVE_FAILED");
        return;
    }
    if (strcmp(op, "arm") == 0) {
        const long secs = jint("seconds", 0);
        if (secs < 1 || secs > 600) { wsReply(num, "diag", "CMD,ERR,ESP32_BAD_ARM"); return; }
        snprintf(buf, sizeof buf, "ARM,%s,%ld", bus, secs); stm32_print(buf); return;
    }
    if (strcmp(op, "disarm") == 0) { snprintf(buf, sizeof buf, "DISARM,%s", bus); stm32_print(buf); return; }
    if (strcmp(op, "mode") == 0) {
        char m[24]; jstr("mode", "", m, sizeof m); upper(m);
        if (strcmp(m, "SILENT") && strcmp(m, "NORMAL") && strcmp(m, "LOOPBACK") && strcmp(m, "LOOPBACKSILENT")) { wsReply(num, "diag", "CMD,ERR,BAD_MODE"); return; }
        snprintf(buf, sizeof buf, "MODE,%s,%s", bus, m); stm32_print(buf); return;
    }
    if (strcmp(op, "bitrate") == 0) {
        jstr("value", "", v, sizeof v); upper(v);
        if (strcmp(v, "AUTO") != 0) { const long hz = strtol(v, NULL, 10); if (hz < 1 || hz > 1000000) { wsReply(num, "diag", "CMD,ERR,BAD_BITRATE"); return; } }
        snprintf(buf, sizeof buf, "BITRATE,%s,%s", bus, v); stm32_print(buf); return;
    }
    if (strcmp(op, "write") == 0) {
        stm32_print(jbool("on", false) ? "WRITE,ON" : "WRITE,OFF"); return;
    }
    if (strcmp(op, "can") == 0) {
        /* bxCAN ability command line; only these verbs pass through. */
        static const char *const ok[] = { "OPT,", "FILTER,", "CAN2SB,", "ERRSTATE,",
                                          "BUSOFF,", "SLEEP,", "WAKE,", "WRITE," };
        char line[96]; jstr("line", "", line, sizeof line);
        for (size_t k = 0; k < sizeof ok / sizeof ok[0]; k++)
            if (strncmp(line, ok[k], strlen(ok[k])) == 0) { stm32_print(line); return; }
        wsReply(num, "diag", "CMD,ERR,UNKNOWN"); return;
    }
    if (strcmp(op, "gateway") == 0) {
        snprintf(buf, sizeof buf, "GATEWAY,%s", jbool("on", false) ? "ON" : "OFF"); stm32_print(buf); return;
    }
    if (strcmp(op, "periodic") == 0) {
        const long slot = jint("slot", -1);
        if (slot < 0 || slot > 7) { wsReply(num, "diag", "CMD,ERR,BAD_SLOT"); return; }
        if (jbool("off", false)) { snprintf(buf, sizeof buf, "PERIODIC,%ld,OFF", slot); stm32_print(buf); return; }
        jstr("format", "", fmt, sizeof fmt); upper(fmt); jstr("id", "", id, sizeof id); upper(id);
        jstr("data", "", data, sizeof data); upper(data);
        const long dlc = jint("dlc", -1), per = jint("period", 0);
        const uint32_t maxId = strcmp(fmt, "S") == 0 ? 0x7FFu : (strcmp(fmt, "E") == 0 ? 0x1FFFFFFFu : 0u);
        bool ok = (!strcmp(fmt, "S") || !strcmp(fmt, "E")) && isHexStr(id, 1, 8) && dlc >= 0 && dlc <= 8 && per >= 1 && per <= 60000 &&
                  strtoul(id, NULL, 16) <= maxId;
        if (ok && dlc > 0) ok = strlen(data) == (size_t)(dlc * 2) && isHexStr(data, 2, 16);
        if (!ok) { wsReply(num, "diag", "CMD,ERR,BAD_PERIODIC"); return; }
        snprintf(buf, sizeof buf, "PERIODIC,%ld,%s,%s,%s,%ld,%s,%ld", slot, bus, fmt, id, dlc, dlc == 0 ? "-" : data, per);
        stm32_print(buf); return;
    }
    if (strcmp(op, "start") == 0)        { stm32_print("START"); return; }
    if (strcmp(op, "start_cancel") == 0) { stm32_print("STARTCANCEL"); return; }
    if (strcmp(op, "gear") == 0) {
        const long g = jint("gear", -1);
        if (g < 0 || g > 15) { wsReply(num, "diag", "GEAR,ERR,BAD_GEAR"); return; }
        snprintf(buf, sizeof buf, "GEAR,%ld", g); stm32_print(buf); return;
    }
    if (strcmp(op, "gear_clear") == 0)   { stm32_print("GEAR,CLEAR"); return; }
    if (strcmp(op, "gear_bcast") == 0)   { stm32_print(jbool("on", false) ? "GEARBCAST,ON" : "GEARBCAST,OFF"); return; }
    if (strcmp(op, "startcfg") == 0) {
        long to = jint("timeout", 2000), rp = jint("rpm", 400);
        const long ra = jint("armed", 1), rn = jint("neutral", 1), rb = jint("brake", 1), ri = jint("immob", 1);
        if (to < 500) to = 500;
        if (to > 10000) to = 10000;
        if (rp < 1) rp = 400;
        snprintf(buf, sizeof buf, "STARTCFG,%ld,%d,%d,%d,%d,%ld", to, ra ? 1 : 0, rn ? 1 : 0, rb ? 1 : 0, ri ? 1 : 0, rp);
        stm32_print(buf); return;
    }
    if (strcmp(op, "startinputs") == 0) {
        const long n = jint("neutral", 0), b = jint("brake", 0), i = jint("immob", 0);
        snprintf(buf, sizeof buf, "STARTINPUTS,%d,%d,%d", n ? 1 : 0, b ? 1 : 0, i ? 1 : 0); stm32_print(buf); return;
    }
    if (strcmp(op, "startframe") == 0) {
        if (jbool("none", false)) { stm32_print("STARTFRAME,NONE"); return; }
        jstr("format", "", fmt, sizeof fmt); upper(fmt); jstr("id", "", id, sizeof id); upper(id);
        jstr("data", "", data, sizeof data); upper(data);
        const long dlc = jint("dlc", -1);
        const uint32_t maxId = strcmp(fmt, "S") == 0 ? 0x7FFu : (strcmp(fmt, "E") == 0 ? 0x1FFFFFFFu : 0u);
        bool ok = (!strcmp(fmt, "S") || !strcmp(fmt, "E")) && isHexStr(id, 1, 8) && dlc >= 0 && dlc <= 8 &&
                  strtoul(id, NULL, 16) <= maxId && strlen(data) == (size_t)(dlc * 2);
        if (ok && dlc > 0) ok = isHexStr(data, 2, 16);
        if (!ok) { wsReply(num, "diag", "CMD,ERR,BAD_FRAME"); return; }
        snprintf(buf, sizeof buf, "STARTFRAME,%s,%s,%s,%ld,%s", bus, fmt, id, dlc, data);
        stm32_print(buf); return;
    }
    if (strcmp(op, "uds_abort") == 0) { snprintf(buf, sizeof buf, "UDS,%s,ABORT", bus); stm32_print(buf); return; }
    if (strcmp(op, "ecuscan_abort") == 0) { snprintf(buf, sizeof buf, "ECUSCAN,%s,ABORT", bus); stm32_print(buf); return; }
    if (strcmp(op, "ecuscan") == 0) {
        char first[8], last[8], req[24];
        jstr("format", "", fmt, sizeof fmt); upper(fmt); jstr("first", "", first, sizeof first); upper(first);
        jstr("last", "", last, sizeof last); upper(last); jstr("req", "", req, sizeof req); upper(req);
        const long wait = jint("wait", 60);
        const uint32_t maxT = strcmp(fmt, "S") == 0 ? 0x7F7u : (strcmp(fmt, "E") == 0 ? 0xFFu : 0u);
        const bool ok = (!strcmp(fmt, "S") || !strcmp(fmt, "E")) && isHexStr(first, 1, 3) && isHexStr(last, 1, 3) &&
                        strtoul(first, NULL, 16) <= maxT && strtoul(last, NULL, 16) <= maxT &&
                        isHexStr(req, 2, 14) && (strlen(req) % 2u) == 0u && wait >= 1 && wait <= 1000;
        if (!ok) { wsReply(num, "diag", "ECU,ERR,ESP32_VALIDATION_FAILED"); return; }
        snprintf(buf, sizeof buf, "ECUSCAN,%s,%s,%s,%s,%s,%ld", bus, fmt, first, last, req, wait);
        stm32_print(buf); return;
    }
    if (strcmp(op, "udspad") == 0) {
        jstr("value", "", v, sizeof v); upper(v);
        if (strcmp(v, "NONE") != 0 && !isHexStr(v, 2, 2)) { wsReply(num, "diag", "CMD,ERR,ESP32_BAD_PAD"); return; }
        snprintf(buf, sizeof buf, "UDSPAD,%s,%s", bus, v); stm32_print(buf); return;
    }
    if (strcmp(op, "uds") == 0) {
        static char line[1100];
        char *req = line + 48;                     /* request hex lives inside the line buffer */
        char tx[16], rx[16];
        jstr("format", "", fmt, sizeof fmt); upper(fmt); jstr("tx", "", tx, sizeof tx); upper(tx);
        jstr("rx", "", rx, sizeof rx); upper(rx); jstr("req", "", req, 1032); upper(req);
        const bool confirm = jbool("confirm", false);
        const uint32_t maxId = strcmp(fmt, "S") == 0 ? 0x7FFu : (strcmp(fmt, "E") == 0 ? 0x1FFFFFFFu : 0u);
        const bool ok = (!strcmp(fmt, "S") || !strcmp(fmt, "E")) && isHexStr(tx, 1, 8) && isHexStr(rx, 1, 8) &&
                        strtoul(tx, NULL, 16) <= maxId && strtoul(rx, NULL, 16) <= maxId &&
                        isHexStr(req, 2, 1024) && (strlen(req) % 2u) == 0u;
        if (!ok) { wsReply(num, "diag", "UDS,ERR,ESP32_VALIDATION_FAILED"); return; }
        char pre[48]; const int pl = snprintf(pre, sizeof pre, "UDS,%s,%s,%s,%s,", bus, fmt, tx, rx);
        const size_t rl = strlen(req);
        memmove(line + pl, req, rl + 1u); memcpy(line, pre, (size_t)pl);
        if (confirm) strcat(line, ",CONFIRM");
        stm32_print(line); return;
    }
    if (strcmp(op, "tx") != 0) return;
    jstr("format", "", fmt, sizeof fmt); upper(fmt); jstr("id", "", id, sizeof id); upper(id);
    jstr("data", "", data, sizeof data); upper(data);
    const long dlc = jint("dlc", -1), rtr = jint("rtr", -1);
    const uint32_t maxId = strcmp(fmt, "S") == 0 ? 0x7FFu : (strcmp(fmt, "E") == 0 ? 0x1FFFFFFFu : 0u);
    bool ok = (!strcmp(fmt, "S") || !strcmp(fmt, "E")) && isHexStr(id, 1, 8) && dlc >= 0 && dlc <= 8 &&
              (rtr == 0 || rtr == 1) && strtoul(id, NULL, 16) <= maxId;
    if (ok && rtr == 0) ok = strlen(data) == (size_t)(dlc * 2) && (dlc == 0 || isHexStr(data, 2, 16));
    if (!ok) { wsReply(num, "tx", "TX,ERR,ESP32_VALIDATION_FAILED"); return; }
    snprintf(buf, sizeof buf, "TX,%s,%s,%s,%ld,%ld,%s", bus, fmt, id, rtr, dlc, (rtr || dlc == 0) ? "-" : data);
    stm32_print(buf);
}

/* ═══════════════════════════════ HTTP routes ═══════════════════════════ */
extern const uint8_t dash_page_gz[];
extern const size_t  dash_page_gz_len;
static char wsToken[25];

static void randomSecret(char *out, size_t n)
{
    static const char al[] = "abcdefghjkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    for (size_t i = 0; i < n; i++) {
        uint32_t r = TIM2->CNT ^ (i * 2654435761u);
        if (RNG->CR & RNG_CR_RNGEN) { for (int g = 0; g < 1000 && !(RNG->SR & RNG_SR_DRDY); g++) { } if (RNG->SR & RNG_SR_DRDY) r = RNG->DR; }
        out[i] = al[r % (sizeof al - 1u)];
    }
    out[n] = '\0';
}

/* snapshot stream state */
static struct { uint32_t id, cursor; bool hdr_done; } s_snapStream;
static size_t snapGen(void *ctx, uint8_t *dst, size_t cap)
{
    (void)ctx;
    static const char EVENT_LOG_HDR[] = "event_id,dtc,ecu,bus,event_utc,uptime_ms,utc,key,rpm,speed_kmh,rpm_src,acc_wire,pedal_17C_b0,fps_F,fps_B,pids\n";
    if (!s_snapStream.hdr_done) {
        s_snapStream.hdr_done = true;
        const size_t n = sizeof EVENT_LOG_HDR - 1u < cap ? sizeof EVENT_LOG_HDR - 1u : cap;
        memcpy(dst, EVENT_LOG_HDR, n); return n;
    }
    const uint8_t *row; uint16_t len;
    if (!evst_snap_next(s_snapStream.id, &s_snapStream.cursor, &row, &len)) return 0;
    const size_t n = len < cap ? len : cap;
    memcpy(dst, row, n);
    return n;
}

static bool is_get(const http_req_t *r) { return strcmp(r->method, "GET") == 0; }
static bool is_post(const http_req_t *r) { return strcmp(r->method, "POST") == 0; }

void dash_http_request(http_conn_t *c, const http_req_t *r)
{
    if (is_get(r) && strcmp(r->path, "/") == 0) {
        http_respond(c, 200, "text/html", "Content-Encoding: gzip\r\n", dash_page_gz, dash_page_gz_len); return;
    }
    if (is_get(r) && strcmp(r->path, "/token") == 0) {
        http_respond(c, 200, "text/plain", "Cache-Control: no-store\r\n", wsToken, strlen(wsToken)); return;
    }
    if (is_get(r) && strcmp(r->path, "/log.csv") == 0) {
        char tmp[8];
        const bool old = http_query_get(r, "historical", tmp, sizeof tmp) || http_query_get(r, "old", tmp, sizeof tmp);
        const uint8_t *d; size_t n;
        if (!logFsOk || !logst_get(old, &d, &n)) { http_respond_text(c, 404, "no log yet"); return; }
        http_respond(c, 200, "text/csv", old ? "Content-Disposition: attachment; filename=civic_historical_data.csv\r\n"
                                             : "Content-Disposition: attachment; filename=civic_log.csv\r\n", d, n);
        return;
    }
    if (is_post(r) && strcmp(r->path, "/log/clear") == 0) {
        if (logFsOk) logst_clear();
        http_respond_text(c, 200, "cleared"); return;
    }
    if (is_get(r) && strcmp(r->path, "/event.csv") == 0) {
        char arg[16];
        if (!logFsOk || !http_query_get(r, "id", arg, sizeof arg)) { http_respond_text(c, 400, "event id required"); return; }
        const size_t al = strlen(arg);
        if (!al || al > 10u) { http_respond_text(c, 400, "invalid event id"); return; }
        for (size_t i = 0; i < al; ++i) if (!isdigit((unsigned char)arg[i])) { http_respond_text(c, 400, "invalid event id"); return; }
        const uint32_t idv = (uint32_t)strtoul(arg, NULL, 10);
        DtcEvent *e = findEvent(idv);
        if (!e || !e->snapshotReady || !evst_snap_exists(idv)) { http_respond_text(c, 404, "event snapshot not ready"); return; }
        static char disp[80];
        snprintf(disp, sizeof disp, "Content-Disposition: attachment; filename=civic_event_%lu.csv\r\n", (unsigned long)idv);
        s_snapStream.id = idv; s_snapStream.cursor = 0; s_snapStream.hdr_done = false;
        http_respond_stream(c, 200, "text/csv", disp, snapGen, NULL);
        return;
    }
    /* anything else: web_server answers 404 "Not found" */
}

/* ── uploads: /update (ESP32 OTA, no ESP32 any more) and /update-stm32 ── */
enum { UP_NONE, UP_ESP32, UP_STM32 };
static uint8_t  s_up;
static bool     s_upFail;
static uint32_t s_upSize, s_upExpect, s_upLastPct;
static uint32_t s_applyAt;       /* millis() when the staged image is applied */
static uint32_t s_applySize;

static void blProgress(int pct, const char *msg)
{
    jb_t j; jb_begin(&j, s_jbuf, sizeof s_jbuf);
    jb_str(&j, "type", "stm32_progress"); jb_int(&j, "pct", pct); jb_str(&j, "msg", msg);
    ws_broadcast_doc(&j);
}

bool dash_http_upload_begin(const http_req_t *r)
{
    s_up = UP_NONE; s_upFail = false; s_upSize = 0; s_upLastPct = 0;
    if (strcmp(r->path, "/update") == 0) { s_up = UP_ESP32; return false; }
    if (strcmp(r->path, "/update-stm32") != 0) return false;
    if (stm32FlashRunning) return false;
    s_up = UP_STM32; stm32FlashRunning = true; s_upExpect = r->content_length;
    blProgress(2, "Upload started");
    if (!fs_erase_128k(FS_STAGING_ADDR) || !fs_erase_128k(FS_STAGING_ADDR + FS_SECTOR_128K)) {
        s_upFail = true; return false;
    }
    blProgress(10, "Staging area erased");
    return true;
}

bool dash_http_upload_data(const uint8_t *d, size_t n)
{
    if (s_up != UP_STM32 || s_upFail) return false;
    if (s_upSize + n > FS_APP_MAX_SIZE) { s_upFail = true; return false; }
    if (!fs_program(FS_STAGING_ADDR + s_upSize, d, n)) { s_upFail = true; return false; }
    s_upSize += (uint32_t)n;
    if (s_upExpect && s_upSize - s_upLastPct >= 4096u) {
        s_upLastPct = s_upSize;
        char m[48]; snprintf(m, sizeof m, "Writing... %lu/%lu KB", (unsigned long)(s_upSize / 1024u), (unsigned long)(s_upExpect / 1024u));
        blProgress(10 + (int)((uint64_t)s_upSize * 80u / s_upExpect), m);
    }
    return true;
}

static const char *validateStm32Image(const uint8_t *bin, size_t size)
{
    if (size < 8u || size > FS_APP_MAX_SIZE) return "image size out of range";
    const uint32_t sp = bin[0] | (bin[1] << 8) | (bin[2] << 16) | ((uint32_t)bin[3] << 24);
    const uint32_t rv = bin[4] | (bin[5] << 8) | (bin[6] << 16) | ((uint32_t)bin[7] << 24);
    const bool spOk = (sp > 0x20000000u && sp <= 0x20020000u) || (sp > 0x10000000u && sp <= 0x10010000u);
    const bool rvOk = (rv & 1u) && (rv & ~1u) >= 0x08000000u && (rv & ~1u) < 0x08000000u + size;
    if (!spOk) return "initial stack pointer not in STM32 RAM — not an STM32F407 image";
    if (!rvOk) return "reset vector not inside the image — not a flash-linked .bin";
    return "";
}

void dash_http_upload_end(http_conn_t *c, const http_req_t *r, bool complete)
{
    (void)r;
    if (s_up == UP_ESP32) {
        s_up = UP_NONE;
        if (c) http_respond_text(c, 500, "Flash failed: this single-board build has no ESP32 (the STM32 serves the dashboard itself)");
        return;
    }
    if (s_up != UP_STM32) { if (c) http_respond_text(c, 404, "Not found"); return; }
    s_up = UP_NONE;
    if (!c || !complete || s_upFail || s_upSize == 0u) {
        stm32FlashRunning = false;
        if (c) http_respond_text(c, 400, s_upFail ? "No binary received (or larger than 256 KB / staging write failed)"
                                                  : "No binary received (or larger than 256 KB / out of memory)");
        return;
    }
    blProgress(90, "Write complete, verifying");
    const char *bad = validateStm32Image(fs_ptr(FS_STAGING_ADDR), s_upSize);
    if (bad[0]) {
        stm32FlashRunning = false;
        static char msg[160]; snprintf(msg, sizeof msg, "Rejected before erase: %s", bad);
        http_respond_text(c, 500, msg); return;
    }
    blProgress(98, "Verified, starting application");
    static char ok[80];
    snprintf(ok, sizeof ok, "OK — STM32 flashed %lu bytes", (unsigned long)s_upSize);
    http_respond_text(c, 200, ok);
    blProgress(100, "STM32 flashed successfully");
    /* give lwIP/USB time to deliver the reply, then copy staging -> app */
    s_applySize = s_upSize; s_applyAt = millis() + 700u; if (!s_applyAt) s_applyAt = 1u;
}

/* ═════════════════════════════════ init / poll ═════════════════════════ */
static struct tcp_pcb *listen_port(uint16_t port, tcp_accept_fn fn)
{
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (!pcb) return NULL;
    if (tcp_bind(pcb, IP_ANY_TYPE, port) != ERR_OK) { tcp_close(pcb); return NULL; }
    struct tcp_pcb *l = tcp_listen_with_backlog(pcb, 2);
    if (!l) { tcp_close(pcb); return NULL; }
    tcp_accept(l, fn);
    return l;
}

bool dash_init(dash_cmd_fn cmd, can_capture_queue_t *q_fcan, can_capture_queue_t *q_bcan)
{
    s_cmd = cmd;
    s_capq[0] = q_fcan; s_capq[1] = q_bcan;
    fs_init(q_fcan, q_bcan);
    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN; (void)RCC->AHB2ENR; RNG->CR |= RNG_CR_RNGEN;
    randomSecret(wsToken, 24);
    for (unsigned i = 0; i < WS_MAX_CLIENTS; i++) evCursor[i] = -2;
    logFsOk = logst_init() && evst_init(replayRecord, rewriteEvents, snapLive);
    if (logFsOk) loadEvents();
    bool ok = web_server_init();
    ok = (listen_port(23, gvret_accept) != NULL) && ok;          /* SavvyCAN / RealDash */
    ok = (listen_port(2323, obs_accept) != NULL) && ok;          /* read-only observer  */
    return ok && logFsOk;
}

void dash_poll(void)
{
    web_server_poll();
    frameBatchFlush();
    decBatchFlush();
    logService();
    eventsPump();
    if (gvretCli) tcp_output(gvretCli);
    static uint32_t last; const uint32_t now = millis();
    if (now - last >= 1000u) { last = now; emitStatus(); }
    if (s_applyAt && (int32_t)(now - s_applyAt) >= 0) fs_apply_staged_image(s_applySize);
}
