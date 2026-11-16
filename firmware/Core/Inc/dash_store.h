#ifndef DASH_STORE_H
#define DASH_STORE_H
/* Persistent storage for the dashboard (replaces ESP32 LittleFS files):
 *   log.csv / log.1.csv  -> two alternating 128 KB sectors (rotation at 128 KB)
 *   events.json + event_<id>.csv snapshots -> an append-only record journal in
 *   two ping-pong 128 KB sectors, compacted when full.                     */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ── rolling log ── */
bool logst_init(void);
bool logst_ok(void);
bool logst_append(const char *hdr, const char *line, size_t n);   /* hdr written into an empty file */
bool logst_get(bool old, const uint8_t **data, size_t *len);      /* false = "file does not exist" */
void logst_clear(void);
/* Current sector fill in permille and its sequence number (0 if none). */
uint32_t logst_fill_permille(void);
uint32_t logst_seq(void);

/* ── event journal ── */
enum { EV_REC_EVENT = 2, EV_REC_EVENT_DEL = 3, EV_REC_SNAP_ROW = 4, EV_REC_SNAP_DEL = 5 };

typedef void (*evst_replay_fn)(uint8_t type, uint32_t id, const uint8_t *data, uint16_t len);
/* Rewrites the live state into a fresh sector during compaction. */
typedef bool (*evst_live_fn)(void);
typedef bool (*evst_snap_live_fn)(uint32_t id);

bool     evst_init(evst_replay_fn replay, evst_live_fn rewrite_events, evst_snap_live_fn snap_live);
bool     evst_ok(void);
bool     evst_append(uint8_t type, uint32_t id, const void *data, uint16_t len);
size_t   evst_free_bytes(void);
size_t   evst_total_bytes(void);
bool     evst_compact(void);
bool     evst_snap_exists(uint32_t id);
/* Iterate snapshot rows of id: *cursor starts at 0; returns false at end. */
bool     evst_snap_next(uint32_t id, uint32_t *cursor, const uint8_t **row, uint16_t *len);

#endif
