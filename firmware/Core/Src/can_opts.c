/* can_opts.c - see can_opts.h. Bit positions from RM0090 32.9. */
#include "can_opts.h"
#include <string.h>

#define MCR_TTCM (1u << 7)
#define MCR_ABOM (1u << 6)
#define MCR_AWUM (1u << 5)
#define MCR_NART (1u << 4)
#define MCR_RFLM (1u << 3)
#define MCR_TXFP (1u << 2)
#define MCR_DBF  (1u << 16)
#define BTR_SJW_POS 24u
#define BTR_TS2_POS 20u

can_opts_t can_opts_default(void)
{
    can_opts_t o = { .abom = true, .awum = true, .nart = false, .rflm = false,
                     .txfp = false, .ttcm = true, .dbf = true, .sjw = 2u };
    return o;
}

uint32_t can_opts_mcr_bits(const can_opts_t *o)
{
    uint32_t m = 0u;
    if (o->ttcm) m |= MCR_TTCM;
    if (o->abom) m |= MCR_ABOM;
    if (o->awum) m |= MCR_AWUM;
    if (o->nart) m |= MCR_NART;
    if (o->rflm) m |= MCR_RFLM;
    if (o->txfp) m |= MCR_TXFP;
    if (o->dbf)  m |= MCR_DBF;
    return m;
}

uint32_t can_opts_apply_sjw(uint32_t btr, uint8_t sjw)
{
    const uint32_t ts2 = ((btr >> BTR_TS2_POS) & 7u) + 1u;
    uint32_t j = sjw < 1u ? 1u : (sjw > 4u ? 4u : sjw);
    if (j > ts2) j = ts2;
    return (btr & ~(3u << BTR_SJW_POS)) | ((j - 1u) << BTR_SJW_POS);
}

uint8_t can_opts_btr_sjw(uint32_t btr) { return (uint8_t)(((btr >> BTR_SJW_POS) & 3u) + 1u); }

const char *can_lec_name(uint32_t lec)
{
    static const char *const n[8] = { "NONE", "STUFF", "FORM", "ACK",
                                      "BIT_RECESSIVE", "BIT_DOMINANT", "CRC", "SOFTWARE" };
    return n[lec & 7u];
}

const char *can_error_state(uint32_t esr)
{
    if (esr & (1u << 2)) return "BUS_OFF";
    if (esr & (1u << 1)) return "PASSIVE";
    if (esr & (1u << 0)) return "WARNING";
    return "ACTIVE";
}

static int parse_bool(const char *v)
{
    if (!v) return -1;
    if (!strcmp(v, "on") || !strcmp(v, "1")) return 1;
    if (!strcmp(v, "off") || !strcmp(v, "0")) return 0;
    return -1;
}

bool can_opts_set(can_opts_t *o, const char *name, const char *value)
{
    if (!o || !name || !value) return false;
    if (!strcmp(name, "SJW")) {
        if (value[0] < '1' || value[0] > '4' || value[1] != '\0') return false;
        o->sjw = (uint8_t)(value[0] - '0');
        return true;
    }
    const int b = parse_bool(value);
    if (b < 0) return false;
    bool *f = !strcmp(name, "ABOM") ? &o->abom : !strcmp(name, "AWUM") ? &o->awum :
              !strcmp(name, "NART") ? &o->nart : !strcmp(name, "RFLM") ? &o->rflm :
              !strcmp(name, "TXFP") ? &o->txfp : !strcmp(name, "TTCM") ? &o->ttcm :
              !strcmp(name, "DBF")  ? &o->dbf  : NULL;
    if (!f) return false;
    *f = (b == 1);
    return true;
}
