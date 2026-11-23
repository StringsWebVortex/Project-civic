/* Host tests for bxCAN option bits (can_opts.c, RM0090 32.9). */
#include <assert.h>
#include <string.h>
#include "can_opts.h"

int main(void)
{
    can_opts_t o = can_opts_default();
    assert(o.abom && o.awum && o.ttcm && o.dbf && !o.nart && !o.rflm && !o.txfp && o.sjw == 2u);
    /* TTCM bit7, ABOM bit6, AWUM bit5, DBF bit16 */
    assert(can_opts_mcr_bits(&o) == ((1u<<7)|(1u<<6)|(1u<<5)|(1u<<16)));
    can_opts_t z = {0};
    assert(can_opts_mcr_bits(&z) == 0u);
    z.nart = z.rflm = z.txfp = true;
    assert(can_opts_mcr_bits(&z) == ((1u<<4)|(1u<<3)|(1u<<2)));

    /* SJW: BTR with TS2 = 2 tq (field 1) */
    const uint32_t btr = (1u << 20) | (10u << 16) | 5u;
    assert(can_opts_btr_sjw(can_opts_apply_sjw(btr, 1u)) == 1u);
    assert(can_opts_btr_sjw(can_opts_apply_sjw(btr, 2u)) == 2u);
    assert(can_opts_btr_sjw(can_opts_apply_sjw(btr, 4u)) == 2u);   /* clamped to TS2 */
    assert(can_opts_btr_sjw(can_opts_apply_sjw(btr, 0u)) == 1u);   /* clamped up */
    const uint32_t btr8 = (7u << 20) | 5u;                         /* TS2 = 8 tq */
    assert(can_opts_btr_sjw(can_opts_apply_sjw(btr8, 9u)) == 4u);  /* clamped to 4 */
    assert((can_opts_apply_sjw(btr8 | (3u << 24), 1u) & (3u << 24)) == 0u);
    assert((can_opts_apply_sjw(btr, 2u) & ~(3u << 24)) == btr);    /* rest untouched */

    assert(!strcmp(can_lec_name(0), "NONE"));
    assert(!strcmp(can_lec_name(3), "ACK"));
    assert(!strcmp(can_lec_name(6), "CRC"));
    assert(!strcmp(can_lec_name(7), "SOFTWARE"));
    assert(!strcmp(can_lec_name(0x13), "ACK"));

    assert(!strcmp(can_error_state(0), "ACTIVE"));
    assert(!strcmp(can_error_state(1), "WARNING"));
    assert(!strcmp(can_error_state(3), "PASSIVE"));
    assert(!strcmp(can_error_state(7), "BUS_OFF"));

    can_opts_t s = can_opts_default();
    assert(can_opts_set(&s, "NART", "on") && s.nart);
    assert(can_opts_set(&s, "NART", "0") && !s.nart);
    assert(can_opts_set(&s, "TXFP", "1") && s.txfp);
    assert(can_opts_set(&s, "RFLM", "on") && s.rflm);
    assert(can_opts_set(&s, "ABOM", "off") && !s.abom);
    assert(can_opts_set(&s, "AWUM", "off") && !s.awum);
    assert(can_opts_set(&s, "TTCM", "off") && !s.ttcm);
    assert(can_opts_set(&s, "DBF", "off") && !s.dbf);
    assert(can_opts_set(&s, "SJW", "3") && s.sjw == 3u);
    assert(!can_opts_set(&s, "SJW", "5"));
    assert(!can_opts_set(&s, "SJW", "0"));
    assert(!can_opts_set(&s, "SJW", "12"));
    assert(!can_opts_set(&s, "NART", "yes"));
    assert(!can_opts_set(&s, "BOGUS", "on"));
    assert(!can_opts_set(NULL, "NART", "on"));
    assert(!can_opts_set(&s, NULL, "on"));
    assert(!can_opts_set(&s, "NART", NULL));
    return 0;
}
