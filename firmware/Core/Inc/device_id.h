#ifndef DEVICE_ID_H
#define DEVICE_ID_H
/*
 * device_id.h - identify which device/network is connected, from passive
 * traffic only (bitrates + the set of 11-bit identifiers seen per bus), and
 * remember the user's target device across boots.
 *
 * Boot behaviour:
 *   target == 0  -> DETECT: observe traffic, then report the best matching
 *                   known profile (or NEW) and keep reporting it on every
 *                   boot until the user selects a device.
 *   target != 0  -> boot straight into that device.
 *
 * Profiles are identity data: they are never deleted, only added or renamed.
 * Persistence is a record journal (dash_store); this module only encodes and
 * decodes records, so it is pure logic and host-testable.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEV_MAX_PROFILES   12u
#define DEV_MAX_IDS        64u     /* identifiers kept per fingerprint      */
#define DEV_NAME_MAX       40u     /* including the terminating NUL         */
#define DEV_MATCH_PERMILLE 700u    /* score needed to call it the same device */
#define DEV_FLAG_SEED      0x01u   /* built in from old/DBC data, unverified */
#define DEV_FLAG_VERIFIED  0x02u   /* matched live traffic at least once     */

/* identifier key: bit15 = bus (0 F/CAN1, 1 B/CAN2), bits 10..0 = std id */
typedef struct {
    uint32_t pid;                  /* profile id, never 0                   */
    char     name[DEV_NAME_MAX];
    uint32_t bitrate[2];           /* 0 = bus not seen                      */
    uint16_t nids;
    uint16_t ids[DEV_MAX_IDS];     /* sorted ascending                      */
    uint8_t  flags;
} dev_profile_t;

typedef enum { DEV_IDLE = 0, DEV_OBSERVING, DEV_DONE } dev_state_t;
typedef enum { DEV_EV_NONE = 0, DEV_EV_DETECTED, DEV_EV_NEW } dev_event_t;
typedef enum { DEV_ADD_OK = 0, DEV_ADD_EXISTS, DEV_ADD_FULL, DEV_ADD_NO_DATA, DEV_ADD_BAD_NAME } dev_add_t;

typedef struct {
    dev_profile_t prof[DEV_MAX_PROFILES];
    uint8_t  count;
    uint32_t target;               /* selected profile, 0 = detect mode     */
    uint32_t next_pid;
    /* live observation */
    dev_state_t state;
    uint16_t obs[DEV_MAX_IDS];
    uint16_t nobs;
    uint32_t obs_bitrate[2];
    uint32_t obs_frames;
    uint32_t obs_start_ms;
    uint32_t best_pid;             /* result of the last detection          */
    uint16_t best_score;
} dev_db_t;

/* Record types in the dash_store event journal. */
enum { DEV_REC_PROFILE = 6, DEV_REC_TARGET = 7 };

/* Empty database plus the built-in seed profile(s); detect mode. */
void dev_init(dev_db_t *db);
/* Start a new observation window. */
void dev_observe_start(dev_db_t *db, uint32_t now_ms);
/* Feed one received frame (any thread that owns db). */
void dev_observe(dev_db_t *db, uint8_t bus, uint32_t id, bool ext);
void dev_set_bitrate(dev_db_t *db, uint8_t bus, uint32_t bitrate);
/* Advance detection; returns an event once per observation window. */
dev_event_t dev_step(dev_db_t *db, uint32_t now_ms);
/* Similarity of the current observation with a profile, 0..1000. */
uint16_t dev_score(const dev_db_t *db, const dev_profile_t *p);
const dev_profile_t *dev_find(const dev_db_t *db, uint32_t pid);
/* Save the current observation as a new profile. If it matches an existing
 * profile, returns DEV_ADD_EXISTS with *pid = that profile. */
dev_add_t dev_add(dev_db_t *db, const char *name, uint32_t *pid);
bool dev_select(dev_db_t *db, uint32_t pid);   /* fixed target from now on */
void dev_detect_mode(dev_db_t *db);            /* target = 0              */
bool dev_rename(dev_db_t *db, uint32_t pid, const char *name);
/* Suggested name for an unknown network, e.g. "Unknown network F500k/B125k 23 IDs". */
void dev_suggest_name(const dev_db_t *db, char *out, size_t cap);

/* Persistence: encode/decode journal records. */
size_t dev_encode_profile(const dev_profile_t *p, uint8_t *buf, size_t cap);
bool   dev_decode_profile(const uint8_t *buf, size_t len, dev_profile_t *out);
/* Apply one replayed journal record (unknown types are ignored). */
void dev_replay(dev_db_t *db, uint8_t type, uint32_t id, const uint8_t *data, uint16_t len);
/* Rewrite the whole state through append() (journal compaction). */
typedef bool (*dev_append_fn)(uint8_t type, uint32_t id, const void *data, uint16_t len);
bool dev_rewrite(const dev_db_t *db, dev_append_fn append);
/* Persist one profile / the target through append(). */
bool dev_persist_profile(const dev_db_t *db, uint32_t pid, dev_append_fn append);
bool dev_persist_target(const dev_db_t *db, dev_append_fn append);

/* Global instance shared by main.c and dash.c. */
dev_db_t *dev_global(void);
#endif
