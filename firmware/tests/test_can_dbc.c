/* Host tests for can_dbc.c + the generated Honda/ILX table. Expected values
 * are computed by hand from the DBC bit layouts. */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "can_dbc.h"

static int failures;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, m); failures++; } } while (0)

static const can_dbc_sig_t *sig(uint32_t id, const char *name)
{
    const can_dbc_msg_t *m = can_dbc_find(id);
    if (!m) return NULL;
    for (unsigned i = 0; i < m->sig_count; i++) if (strcmp(m->sigs[i].name, name) == 0) return &m->sigs[i];
    return NULL;
}
static double val(uint32_t id, const char *name, const uint8_t *d, uint8_t dlc, int64_t *raw_out)
{
    const can_dbc_sig_t *s = sig(id, name); int64_t raw = 0;
    if (!s || !can_dbc_raw(s, d, dlc, &raw)) return NAN;
    if (raw_out) *raw_out = raw;
    return can_dbc_phys(s, raw);
}

int main(void)
{
    CHECK(can_dbc_msg_count >= 30u, "table generated");
    for (unsigned i = 1; i < can_dbc_msg_count; i++) CHECK(can_dbc_msgs[i - 1].id < can_dbc_msgs[i].id, "sorted ids");
    CHECK(can_dbc_find(0x17C) && strcmp(can_dbc_find(0x17C)->name, "POWERTRAIN_DATA") == 0, "find 0x17C");
    CHECK(can_dbc_find(0x123) == NULL, "unknown id");

    /* 0x17C POWERTRAIN_DATA: ENGINE_RPM 23|16 BE = bytes 2..3 (matches the
     * firmware's existing broadcast RPM candidate), PEDAL_GAS byte 0. */
    const uint8_t pt[8] = { 0x40, 0, 0x02, 0xEE, 0x80, 0, 0, 0 };
    CHECK(val(0x17C, "ENGINE_RPM", pt, 8, NULL) == 750.0, "0x17C RPM = 750");
    CHECK(val(0x17C, "PEDAL_GAS", pt, 8, NULL) == 64.0, "pedal byte 0");
    CHECK(val(0x17C, "GAS_PRESSED", pt, 8, NULL) == 1.0, "bit 39 = byte4 bit7");
    CHECK(val(0x17C, "BRAKE_SWITCH", pt, 8, NULL) == 0.0, "bit 32 = byte4 bit0");

    /* 0x158 ENGINE_DATA: XMISSION_SPEED 7|16 x0.01 kph = bytes 0..1 (matches
     * the firmware's speed x100 candidate). */
    const uint8_t ed[8] = { 0x04, 0xD2, 0x0B, 0xB8, 0, 0, 0, 0 };
    CHECK(fabs(val(0x158, "XMISSION_SPEED", ed, 8, NULL) - 12.34) < 1e-9, "speed 12.34 kph");
    CHECK(val(0x158, "ENGINE_RPM", ed, 8, NULL) == 3000.0, "0x158 RPM 3000");

    /* 0x1A3 GEARBOX_AUTO: GEAR_SHIFTER 35|4 = byte4 bits3..0; TRANS_TARGET_GEAR 11|4 = byte1 bits3..0 */
    const uint8_t gb[8] = { 0, 0x03, 0, 0, 0x04, 0, 0, 0 };
    int64_t raw;
    CHECK(val(0x1A3, "GEAR_SHIFTER", gb, 8, &raw) == 4.0 && strcmp(can_dbc_text(sig(0x1A3, "GEAR_SHIFTER"), raw), "D") == 0, "shifter D");
    CHECK(val(0x1A3, "TRANS_TARGET_GEAR", gb, 8, &raw) == 3.0 && strcmp(can_dbc_text(sig(0x1A3, "TRANS_TARGET_GEAR"), raw), "3rd") == 0, "target gear 3rd");
    CHECK(strcmp(can_dbc_text(sig(0x1A3, "GEAR_SHIFTER"), 9), "") == 0, "unnamed value -> empty");

    /* signed BE: GAS_PEDAL_2 ENGINE_TORQUE_ESTIMATE 7|16@0- */
    const uint8_t gp[8] = { 0xFF, 0x9C, 0, 0, 0, 0, 0, 0 };
    CHECK(val(0x130, "ENGINE_TORQUE_ESTIMATE", gp, 8, NULL) == -100.0, "signed -100 Nm");

    /* short frame: signal beyond DLC is not decoded */
    CHECK(isnan(val(0x1A3, "GEAR_SHIFTER", gb, 4, NULL)), "beyond DLC rejected");

    /* little-endian (Intel) + signed, synthetic: 12 bits starting at bit 4 */
    const can_dbc_sig_t le = { "X", "", 4, 12, 1, 1, 0.5, 10.0, 0, 0 };
    const uint8_t ld[2] = { 0x30, 0xFF };          /* bits 4..15 = 0xFF3 -> -13 */
    CHECK(can_dbc_raw(&le, ld, 2, &raw) && raw == -13 && can_dbc_phys(&le, raw) == 3.5, "intel signed + scale/offset");
    /* Motorola crossing bytes: 3 bits starting at bit 1 -> byte0 bits1,0 then byte1 bit7 */
    const can_dbc_sig_t be = { "Y", "", 1, 3, 0, 0, 1.0, 0.0, 0, 0 };
    const uint8_t bd[2] = { 0x02, 0x80 };          /* 1,0,1 -> 5 */
    CHECK(can_dbc_raw(&be, bd, 2, &raw) && raw == 5, "motorola byte wrap");

    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("test_can_dbc: all checks passed (%u messages)\n", can_dbc_msg_count);
    return 0;
}
