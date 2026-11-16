#include "can_filter.h"

#define CAN_FILTER_MAX_BANK   27u
#define CAN_STD_ID_MAX        0x7FFu        /* 11-bit */
#define CAN_EXT_ID_MAX        0x1FFFFFFFu   /* 29-bit */

/* 32-bit scale bit positions (RM0090 32.7.4). */
#define F32_IDE               (1u << 2)
#define F32_RTR               (1u << 1)

/* 16-bit scale bit positions. */
#define F16_RTR               (1u << 4)
#define F16_IDE               (1u << 3)

uint32_t can_filter_pack_id32(uint32_t id, bool ext, bool rtr)
{
    uint32_t v;
    if (ext) {
        /* STID[10:0]<<21 | EXID[17:0]<<3 == (29-bit id)<<3 */
        v = (id & CAN_EXT_ID_MAX) << 3;
        v |= F32_IDE;
    } else {
        v = (id & CAN_STD_ID_MAX) << 21;
    }
    if (rtr) {
        v |= F32_RTR;
    }
    return v;
}

uint16_t can_filter_pack_id16(uint32_t id, bool ext, bool rtr)
{
    uint32_t v;
    if (ext) {
        const uint32_t stid = (id >> 18) & CAN_STD_ID_MAX; /* EXID[28:18] */
        const uint32_t exid_top = (id >> 15) & 0x7u;       /* EXID[17:15]  */
        v = (stid << 5) | F16_IDE | exid_top;
    } else {
        v = (id & CAN_STD_ID_MAX) << 5;
    }
    if (rtr) {
        v |= F16_RTR;
    }
    return (uint16_t)v;
}

static bool id_in_range(uint32_t id, bool ext)
{
    return ext ? (id <= CAN_EXT_ID_MAX) : (id <= CAN_STD_ID_MAX);
}

bool can_filter_compile(const can_filter_spec_t *spec, can_filter_regs_t *out)
{
    if (spec == NULL || out == NULL) {
        return false;
    }
    if (spec->bank > CAN_FILTER_MAX_BANK) {
        return false;
    }

    uint32_t fr1;
    uint32_t fr2;

    if (spec->scale32) {
        if (spec->mask_mode) {
            /* One 32-bit filter: FR1 = identifier, FR2 = raw mask. */
            if (!id_in_range(spec->id1, spec->ext1)) {
                return false;
            }
            fr1 = can_filter_pack_id32(spec->id1, spec->ext1, spec->rtr1);
            fr2 = spec->mask1_or_id2;
        } else {
            /* Two 32-bit identifiers (list mode). */
            if (!id_in_range(spec->id1, spec->ext1) ||
                !id_in_range(spec->mask1_or_id2, spec->ext2)) {
                return false;
            }
            fr1 = can_filter_pack_id32(spec->id1, spec->ext1, spec->rtr1);
            fr2 = can_filter_pack_id32(spec->mask1_or_id2, spec->ext2, spec->rtr2);
        }
    } else {
        if (spec->mask_mode) {
            /* Two 16-bit (id, mask) pairs: ids in FR1, masks in FR2. */
            if (!id_in_range(spec->id1, spec->ext1) ||
                !id_in_range(spec->id2, spec->ext2)) {
                return false;
            }
            if (spec->mask1_or_id2 > 0xFFFFu || spec->mask2_or_id4 > 0xFFFFu) {
                return false;
            }
            const uint16_t id_lo = can_filter_pack_id16(spec->id1, spec->ext1, spec->rtr1);
            const uint16_t id_hi = can_filter_pack_id16(spec->id2, spec->ext2, spec->rtr2);
            fr1 = ((uint32_t)id_hi << 16) | (uint32_t)id_lo;
            fr2 = (spec->mask2_or_id4 << 16) | (spec->mask1_or_id2 & 0xFFFFu);
        } else {
            /* Four 16-bit identifiers (list mode). */
            if (!id_in_range(spec->id1, spec->ext1) ||
                !id_in_range(spec->mask1_or_id2, spec->ext1) ||
                !id_in_range(spec->id2, spec->ext2) ||
                !id_in_range(spec->mask2_or_id4, spec->ext2)) {
                return false;
            }
            const uint16_t f0 = can_filter_pack_id16(spec->id1, spec->ext1, spec->rtr1);
            const uint16_t f1 = can_filter_pack_id16(spec->mask1_or_id2, spec->ext1, spec->rtr1);
            const uint16_t f2 = can_filter_pack_id16(spec->id2, spec->ext2, spec->rtr2);
            const uint16_t f3 = can_filter_pack_id16(spec->mask2_or_id4, spec->ext2, spec->rtr2);
            fr1 = ((uint32_t)f1 << 16) | (uint32_t)f0;
            fr2 = ((uint32_t)f3 << 16) | (uint32_t)f2;
        }
    }

    out->bank = spec->bank;
    out->fr1 = fr1;
    out->fr2 = fr2;
    out->mask_mode = spec->mask_mode;
    out->scale32 = spec->scale32;
    out->fifo1 = spec->fifo1;
    out->active = spec->enabled;
    return true;
}

can_filter_spec_t can_filter_accept_all(uint8_t bank, bool fifo1)
{
    can_filter_spec_t s = {0};
    s.bank = bank;
    s.mask_mode = true;
    s.scale32 = true;
    s.fifo1 = fifo1;
    s.enabled = true;
    s.id1 = 0u;
    s.mask1_or_id2 = 0u; /* mask 0 => all id bits don't care => match everything */
    return s;
}

can_filter_spec_t can_filter_single_std(uint8_t bank, uint32_t std_id, bool fifo1)
{
    can_filter_spec_t s = {0};
    s.bank = bank;
    s.mask_mode = true;
    s.scale32 = true;
    s.fifo1 = fifo1;
    s.enabled = true;
    s.id1 = std_id;
    s.ext1 = false;
    /* care about all 11 STID bits and require IDE = 0 (standard frame). */
    s.mask1_or_id2 = ((uint32_t)CAN_STD_ID_MAX << 21) | F32_IDE;
    return s;
}

can_filter_spec_t can_filter_single_ext(uint8_t bank, uint32_t ext_id, bool fifo1)
{
    can_filter_spec_t s = {0};
    s.bank = bank;
    s.mask_mode = true;
    s.scale32 = true;
    s.fifo1 = fifo1;
    s.enabled = true;
    s.id1 = ext_id;
    s.ext1 = true;
    /* care about all 29 id bits and require IDE = 1 (extended frame). */
    s.mask1_or_id2 = ((uint32_t)CAN_EXT_ID_MAX << 3) | F32_IDE;
    return s;
}
