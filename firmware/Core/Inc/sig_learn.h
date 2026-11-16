#ifndef SIG_LEARN_H
#define SIG_LEARN_H
/* Signal learn mode: grades every DBC message definition against what this
 * car actually sends, so a wrong community definition is detected instead of
 * trusted.
 *
 *   SEED      in the DBC (opendbc-derived), not yet seen on this bus
 *   CHECKED   seen >= SL_CHECK_FRAMES times on F-CAN with the DBC's DLC
 *   VERIFIED  a signal of the message matched the official OBD-II value
 *             (SAE J1979 PID 0x0C rpm / 0x0D speed) SL_VERIFY_MATCHES times
 *             while the reference was non-trivial (engine running / moving)
 *   CONFLICT  DLC differs from the DBC, or the reference comparison failed
 *             SL_CONFLICT_MISSES times (more misses than matches)
 *
 * Grades are journaled (record SL_REC_STATE) like identity data and are
 * never deleted; a later VERIFIED overrides an earlier CHECKED, a CONFLICT is
 * sticky until the reference agrees SL_VERIFY_MATCHES more times than it
 * disagreed. */
#include <stdbool.h>
#include <stdint.h>
#include "can_dbc.h"

#define SL_MAX_MSGS        48u
#define SL_CHECK_FRAMES    20u
#define SL_VERIFY_MATCHES  10u
#define SL_CONFLICT_MISSES 10u
#define SL_REF_MAX_AGE_MS  500u
enum { SL_REC_STATE = 10 };

typedef enum { SL_SEED = 0, SL_CHECKED, SL_VERIFIED, SL_CONFLICT } sl_state_t;

typedef struct {               /* persisted part: 8 bytes */
    uint8_t  state;
    uint8_t  dlc_seen;
    uint16_t frames;           /* saturating */
    uint16_t match, miss;      /* reference comparisons, saturating */
} sl_entry_t;

typedef struct {
    sl_entry_t e[SL_MAX_MSGS];
    uint8_t    n;                       /* = DBC message count (<= SL_MAX_MSGS) */
    uint64_t   dirty;                   /* bit i: entry i changed state, persist + broadcast */
    float      ref_rpm, ref_kph;
    uint32_t   ref_rpm_ms, ref_kph_ms;
    bool       have_rpm, have_kph;
} sl_t;

void        sl_init(sl_t *s);
void        sl_on_obd(sl_t *s, uint8_t pid, float value, uint32_t now_ms);
void        sl_on_frame(sl_t *s, uint8_t bus, uint32_t id, bool ext, uint8_t dlc, const uint8_t *data, uint32_t now_ms);
/* Next changed entry (clears its dirty bit); false when none. */
bool        sl_take_change(sl_t *s, uint8_t *index);
const char *sl_state_name(sl_state_t st);
/* Journal record <-> entry (the record id is the CAN id). */
bool        sl_replay(sl_t *s, uint8_t type, uint32_t id, const uint8_t *d, uint16_t len);
typedef bool (*sl_append_fn)(uint8_t type, uint32_t id, const void *data, uint16_t len);
bool        sl_persist(const sl_t *s, uint8_t index, sl_append_fn append);
bool        sl_rewrite(const sl_t *s, sl_append_fn append);      /* compaction: every non-SEED entry */
sl_t       *sl_global(void);
#endif
