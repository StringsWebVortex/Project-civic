#ifndef CAN_FILTER_H
#define CAN_FILTER_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * STM32F407 bxCAN acceptance-filter compiler (pure logic, RM0090 32.7.4).
 *
 * This module only COMPUTES the FR1/FR2 bit patterns and the mode/scale/FIFO/
 * active flags for one filter bank; it never touches peripheral registers.
 * Hardware code applies the returned can_filter_regs_t to CAN_FxR1/FxR2 and
 * the matching bits of CAN_FM1R/FS1R/FFA1R/FA1R.
 *
 * CAN1 and CAN2 share 28 filter banks (0..27). The CAN_FMR.CAN2SB split is a
 * device-wide setting, not a per-bank one, so it is out of this module's scope;
 * callers pick a bank number that falls on their controller's side of the
 * split.
 *
 * Identifier layouts (RM0090 32.7.4):
 *
 *  32-bit scale (one identifier per 32-bit register):
 *      bits[31:21] = STID[10:0]       (standard id, or EXID[28:18] of an ext id)
 *      bits[20:3]  = EXID[17:0]       (extended id low part; 0 for std)
 *      bit[2]      = IDE              (1 = extended frame)
 *      bit[1]      = RTR              (1 = remote frame)
 *      bit[0]      = 0
 *  For a 29-bit extended id this is simply (id << 3) with IDE set, because
 *  STID<<21 | EXID<<3 == (id & 0x1FFFFFFF) << 3.
 *
 *  16-bit scale (two identifiers per 32-bit register):
 *      bits[15:5]  = STID[10:0]
 *      bit[4]      = RTR
 *      bit[3]      = IDE
 *      bits[2:0]   = EXID[17:15]      (only the top 3 extended-id bits fit)
 */

typedef struct {
    uint8_t bank;        /* filter bank, 0..27 */
    bool    mask_mode;   /* true = identifier-mask mode, false = identifier-list */
    bool    scale32;     /* true = single 32-bit scale, false = 16-bit scale */
    bool    fifo1;       /* true = assign matches to FIFO1, false = FIFO0 */
    bool    enabled;     /* true = filter bank active */

    /*
     * Field usage by configuration. Identifiers are given as plain CAN ids
     * (11-bit standard or 29-bit extended) and packed internally; masks are
     * given as RAW bit patterns already in the scale layout above, so the
     * caller controls exactly which bits are "care" bits.
     *
     *   32-bit mask mode   : id1(+ext1/rtr1) = identifier,  mask1_or_id2 = raw
     *                        32-bit mask.  id2/mask2 fields unused.
     *   32-bit list mode   : id1(+ext1/rtr1) and mask1_or_id2(+ext2/rtr2) are
     *                        the two exact-match identifiers.  id2/mask2 unused.
     *   16-bit mask mode   : filter 0 = id1(+ext1/rtr1) with raw 16-bit mask
     *                        mask1_or_id2; filter 1 = id2(+ext2/rtr2) with raw
     *                        16-bit mask mask2_or_id4.
     *   16-bit list mode   : four exact-match identifiers:
     *                        id1 and mask1_or_id2 go in FR1 (flags ext1/rtr1),
     *                        id2 and mask2_or_id4 go in FR2 (flags ext2/rtr2).
     */
    uint32_t id1;
    uint32_t mask1_or_id2;
    bool     ext1;
    bool     rtr1;
    uint32_t id2;
    uint32_t mask2_or_id4;
    bool     ext2;
    bool     rtr2;
} can_filter_spec_t;

typedef struct {
    uint8_t  bank;
    uint32_t fr1;
    uint32_t fr2;
    bool     mask_mode;
    bool     scale32;
    bool     fifo1;
    bool     active;
} can_filter_regs_t;

/* Pack (id, ext, rtr) into the 32-bit scale layout described above. The id is
 * masked to 29 bits (ext) or 11 bits (std); range checking is can_filter_compile's
 * job. */
uint32_t can_filter_pack_id32(uint32_t id, bool ext, bool rtr);

/* Pack (id, ext, rtr) into one 16-bit scale slot. Only EXID[17:15] of an
 * extended id is representable at this scale. */
uint16_t can_filter_pack_id16(uint32_t id, bool ext, bool rtr);

/* Compile a filter spec into register values. Returns false (and leaves *out
 * untouched) when the bank is > 27 or any field used as an identifier is out of
 * range for its frame format (std > 0x7FF, ext > 0x1FFFFFFF), or a 16-bit raw
 * mask does not fit in 16 bits. */
bool can_filter_compile(const can_filter_spec_t *spec, can_filter_regs_t *out);

/* Convenience builders. */

/* 32-bit mask, mask = 0 -> every incoming id matches. */
can_filter_spec_t can_filter_accept_all(uint8_t bank, bool fifo1);

/* 32-bit mask, exact match of one 11-bit standard id. The mask requires every
 * STID bit AND IDE = 0 (so a standard frame matches but an extended frame whose
 * top 11 bits happen to equal std_id does not); RTR is left "don't care". */
can_filter_spec_t can_filter_single_std(uint8_t bank, uint32_t std_id, bool fifo1);

/* 32-bit mask, exact match of one 29-bit extended id. The mask requires every
 * EXID/STID bit AND IDE = 1; RTR is left "don't care". */
can_filter_spec_t can_filter_single_ext(uint8_t bank, uint32_t ext_id, bool fifo1);

#endif /* CAN_FILTER_H */
