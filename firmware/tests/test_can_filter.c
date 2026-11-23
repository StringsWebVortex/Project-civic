/* Host tests for bxCAN acceptance-filter bit packing (can_filter.c, RM0090 32.7.4). */
#include <assert.h>
#include <stdio.h>
#include "can_filter.h"

int main(void)
{
    can_filter_regs_t r;

    /* ---- 32-bit id packing helper -------------------------------------- */
    /* Standard 0x7E8 sits in STID[10:0] = bits[31:21]. */
    assert(can_filter_pack_id32(0x7E8u, false, false) == (0x7E8u << 21));
    assert(can_filter_pack_id32(0x7E8u, false, false) == 0xFD000000u);
    /* Extended frame: IDE (bit2) set, 29-bit id occupies bits[31:3] (id<<3). */
    assert(can_filter_pack_id32(0x12345678u, true, false) == ((0x12345678u << 3) | 0x4u));
    assert(can_filter_pack_id32(0x12345678u, true, false) == 0x91A2B3C4u);
    assert((can_filter_pack_id32(0x12345678u, true, false) & 0x4u) != 0u); /* IDE set */
    assert((can_filter_pack_id32(0x7E8u, false, false) & 0x4u) == 0u);     /* IDE clear for std */
    /* RTR is bit1. */
    assert(can_filter_pack_id32(0x100u, false, true) == ((0x100u << 21) | 0x2u));

    /* ---- 16-bit id packing helper -------------------------------------- */
    /* Standard id in bits[15:5]. */
    assert(can_filter_pack_id16(0x7E8u, false, false) == (uint16_t)(0x7E8u << 5));
    assert(can_filter_pack_id16(0x7E8u, false, false) == 0xFD00u);
    /* Extended: STID[10:0]=EXID[28:18] in bits[15:5], IDE bit3, EXID[17:15] in bits[2:0]. */
    assert(can_filter_pack_id16(0x12345678u, true, false) == 0x91A8u);
    assert((can_filter_pack_id16(0x12345678u, true, false) & 0x8u) != 0u); /* IDE */
    assert(can_filter_pack_id16(0x100u, false, true) == (uint16_t)((0x100u << 5) | 0x10u)); /* RTR bit4 */

    /* ---- accept_all: 32-bit mask, mask 0 => match everything ----------- */
    can_filter_spec_t accept = can_filter_accept_all(5u, false);
    assert(can_filter_compile(&accept, &r));
    assert(r.fr1 == 0u && r.fr2 == 0u);
    assert(r.mask_mode && r.scale32);
    assert(r.bank == 5u && !r.fifo1 && r.active);

    /* ---- single standard id 0x7E8 -------------------------------------- */
    can_filter_spec_t std = can_filter_single_std(0u, 0x7E8u, true);
    assert(can_filter_compile(&std, &r));
    assert(r.fr1 == (0x7E8u << 21));          /* id in STID field */
    assert(r.fr1 == 0xFD000000u);
    /* exact-match mask: all STID bits care + IDE=0 required, RTR don't care. */
    assert(r.fr2 == ((0x7FFu << 21) | 0x4u));
    assert(r.fr2 == 0xFFE00004u);
    assert(r.mask_mode && r.scale32 && r.fifo1);

    /* ---- single extended id 0x12345678 --------------------------------- */
    can_filter_spec_t ext = can_filter_single_ext(2u, 0x12345678u, false);
    assert(can_filter_compile(&ext, &r));
    assert(r.fr1 == ((0x12345678u << 3) | 0x4u)); /* id in bits[31:3], IDE set */
    assert(r.fr1 == 0x91A2B3C4u);
    assert((r.fr1 & 0x4u) != 0u);                 /* IDE bit set */
    assert(r.fr2 == ((0x1FFFFFFFu << 3) | 0x4u)); /* every id bit + IDE care */
    assert(r.fr2 == 0xFFFFFFFCu);

    /* ---- 32-bit identifier-list mode: two exact ids -------------------- */
    can_filter_spec_t list32 = {0};
    list32.bank = 10u;
    list32.mask_mode = false;
    list32.scale32 = true;
    list32.enabled = true;
    list32.id1 = 0x100u;          /* -> FR1 */
    list32.mask1_or_id2 = 0x200u; /* -> FR2 */
    assert(can_filter_compile(&list32, &r));
    assert(r.fr1 == (0x100u << 21) && r.fr1 == 0x20000000u);
    assert(r.fr2 == (0x200u << 21) && r.fr2 == 0x40000000u);
    assert(!r.mask_mode && r.scale32);

    /* ---- 16-bit mask mode: ids in FR1, masks in FR2 -------------------- */
    can_filter_spec_t mask16 = {0};
    mask16.bank = 12u;
    mask16.mask_mode = true;
    mask16.scale32 = false;
    mask16.enabled = true;
    mask16.fifo1 = true;
    mask16.id1 = 0x123u;           mask16.mask1_or_id2 = 0xFFE0u; /* filter 0 id + raw mask (all STID) */
    mask16.id2 = 0x456u;           mask16.mask2_or_id4 = 0xFFE0u; /* filter 1 id + raw mask */
    assert(can_filter_compile(&mask16, &r));
    /* id1 (0x2460) in low half, id2 (0x8AC0) in high half. */
    assert(r.fr1 == 0x8AC02460u);
    assert(r.fr2 == 0xFFE0FFE0u);
    assert(!r.scale32 && r.mask_mode && r.fifo1);

    /* ---- validation: invalid bank 28 ----------------------------------- */
    can_filter_spec_t badbank = can_filter_accept_all(28u, false);
    assert(!can_filter_compile(&badbank, &r));

    /* ---- validation: id out of range ----------------------------------- */
    can_filter_spec_t badstd = can_filter_single_std(0u, 0x800u, false); /* > 11 bits */
    assert(!can_filter_compile(&badstd, &r));
    can_filter_spec_t badext = can_filter_single_ext(0u, 0x20000000u, false); /* > 29 bits */
    assert(!can_filter_compile(&badext, &r));

    /* ---- validation: NULL guards --------------------------------------- */
    assert(!can_filter_compile(NULL, &r));
    assert(!can_filter_compile(&accept, NULL));

    printf("ALL CAN_FILTER TESTS PASS\n");
    /* ---- frame-format split (std -> FIFO0, ext -> FIFO1) ------------------ */
    {
        can_filter_spec_t f = can_filter_accept_format(4u, false, false);
        assert(can_filter_compile(&f, &r));
        assert(r.bank == 4u && r.mask_mode && r.scale32 && !r.fifo1 && r.active);
        assert(r.fr1 == 0x0u && r.fr2 == 0x4u);          /* IDE must be 0 */
        f = can_filter_accept_format(5u, true, true);
        assert(can_filter_compile(&f, &r));
        assert(r.fifo1 && r.fr1 == 0x4u && r.fr2 == 0x4u); /* IDE must be 1 */
    }
    /* ---- 16-bit list mode: four exact ids (RM0090 Fig. 342) ------------ */
    {
        can_filter_spec_t f = {0};
        f.bank = 7u; f.enabled = true; f.mask_mode = false; f.scale32 = false;
        f.id1 = 0x100u; f.mask1_or_id2 = 0x101u; f.id2 = 0x102u; f.mask2_or_id4 = 0x103u;
        assert(can_filter_compile(&f, &r));
        assert(!r.mask_mode && !r.scale32);
        assert(r.fr1 == (((uint32_t)(0x101u << 5) << 16) | (0x100u << 5)));
        assert(r.fr2 == (((uint32_t)(0x103u << 5) << 16) | (0x102u << 5)));
        f.mask2_or_id4 = 0x800u;                         /* 4th id out of 11-bit range */
        assert(!can_filter_compile(&f, &r));
        f.mask2_or_id4 = 0x103u; f.mask1_or_id2 = 0x900u; /* 2nd id out of range */
        assert(!can_filter_compile(&f, &r));
    }
    /* ---- 16-bit mask mode: invalid id / mask wider than 16 bits --------- */
    {
        can_filter_spec_t f = {0};
        f.bank = 8u; f.enabled = true; f.mask_mode = true; f.scale32 = false;
        f.id1 = 0x7E8u; f.id2 = 0x7E0u; f.mask1_or_id2 = 0xFFE0u; f.mask2_or_id4 = 0xFFE0u;
        assert(can_filter_compile(&f, &r));
        f.id2 = 0x800u;                                  /* bad id */
        assert(!can_filter_compile(&f, &r));
        f.id2 = 0x7E0u; f.mask2_or_id4 = 0x10000u;       /* mask > 16 bits */
        assert(!can_filter_compile(&f, &r));
    }
    /* ---- 32-bit list mode: second id out of range ----------------------- */
    {
        can_filter_spec_t f = {0};
        f.bank = 9u; f.enabled = true; f.mask_mode = false; f.scale32 = true;
        f.id1 = 0x7E8u; f.mask1_or_id2 = 0x800u;
        assert(!can_filter_compile(&f, &r));
    }
    return 0;
}
