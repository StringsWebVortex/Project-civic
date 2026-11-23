#ifndef ID_BACKUP_H
#define ID_BACKUP_H
/* Identity backup in an external 24LC512: device profiles, the selected
 * target and learned signal grades survive a full flash erase (ST-LINK mass
 * erase / reflash), which wipes the on-chip journal.
 *
 * Two 32 KB slots (A 0x0000, B 0x8000). A snapshot is streamed record by
 * record ([type][0][len u16][id u32][data]) after a 16-byte header slot, and
 * the header {magic, seq, len, crc32} is written last: a power cut leaves the
 * older slot as the valid one. The mail password is deliberately NOT backed
 * up (it stays only in the MCU). */
#include <stdbool.h>
#include <stdint.h>
#include "ee24.h"

#define IB_SLOT_SIZE   0x8000u
#define IB_HDR_SIZE    16u
#define IB_MAGIC       0x31444943u        /* "CID1" */
#define IB_REC_MAX     512u               /* largest record restored */

typedef struct {
    const ee24_t *ee;
    uint16_t base;                         /* slot being written */
    uint32_t len, crc;
    uint16_t page_at;                      /* EEPROM address of page[0] */
    uint8_t  page_n;
    uint8_t  page[EE24_PAGE];
    bool     ok;
    uint32_t seq;                          /* newest valid snapshot seq (0 = none) */
} ib_t;

/* Newest valid slot: returns true and its base/seq/len. */
bool ib_newest(ib_t *b, uint16_t *base, uint32_t *seq, uint32_t *len);
bool ib_begin(ib_t *b, const ee24_t *ee);                     /* picks the older slot */
bool ib_append(ib_t *b, uint8_t type, uint32_t id, const void *data, uint16_t len);
bool ib_commit(ib_t *b);
typedef void (*ib_apply_fn)(uint8_t type, uint32_t id, const uint8_t *data, uint16_t len);
/* Replays the newest valid snapshot; returns records applied, -1 if none. */
int  ib_restore(ib_t *b, const ee24_t *ee, ib_apply_fn apply);
uint32_t ib_crc32(uint32_t crc, const uint8_t *d, size_t n);  /* crc starts at 0 */
#endif
