/* Host tests for ee24.c + id_backup.c against a 24LC512 model: page writes
 * wrap inside their 128-byte page (DS20001754 6.2), the device NACKs while
 * busy after a write, A/B slots with header-last commit survive power cuts. */
#include <stdio.h>
#include <string.h>
#include "id_backup.h"

static int failures;
#define CHECK(c, m) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, m); failures++; } } while (0)

static uint8_t mem[EE24_SIZE];
static int busy, present = 1, fail_write_after = -1, fail_read, stuck;
static uint32_t now;
static uint32_t now_ms(void) { return now++; }

static bool m_write(uint8_t a, const uint8_t *w1, size_t n1, const uint8_t *w2, size_t n2)
{
    if (!present || a != 0x50) return false;
    if (busy > 0) { busy--; return false; }               /* write cycle in progress: NACK */
    if (n1 + n2 == 0) return true;                        /* address probe */
    if (fail_write_after == 0) return false;
    if (fail_write_after > 0) fail_write_after--;
    const uint16_t at = (uint16_t)(w1[0] << 8 | w1[1]);
    for (size_t i = 0; i < n2; i++)                       /* wrap inside the page */
        mem[(at & ~(EE24_PAGE - 1u)) | ((at + i) & (EE24_PAGE - 1u))] = w2[i];
    busy = stuck ? 100000 : 3;
    return true;
}
static bool m_read(uint8_t a, const uint8_t *w, size_t nw, uint8_t *r, size_t nr)
{
    if (!present || a != 0x50 || busy > 0 || nw != 2 || fail_read) return false;
    const uint16_t at = (uint16_t)(w[0] << 8 | w[1]);
    for (size_t i = 0; i < nr; i++) r[i] = mem[(uint16_t)(at + i)];
    return true;
}
static const ee24_t ee = { m_write, m_read, now_ms, 0x50 };

static int napplied; static uint32_t last_id; static uint8_t last_type; static uint16_t last_len; static uint8_t last_data[IB_REC_MAX];
static void apply(uint8_t t, uint32_t id, const uint8_t *d, uint16_t n) { napplied++; last_type = t; last_id = id; last_len = n; memcpy(last_data, d, n); }

static bool snapshot(ib_t *b, int records, uint8_t fill)
{
    if (!ib_begin(b, &ee)) return false;
    uint8_t d[300]; memset(d, fill, sizeof d);
    for (int i = 0; i < records; i++) if (!ib_append(b, (uint8_t)(6 + i % 2), (uint32_t)i, d, (uint16_t)(50 + i * 7))) return false;
    return ib_commit(b);
}

int main(void)
{
    memset(mem, 0xFF, sizeof mem);
    ib_t b;
    /* ee24: page split across boundaries is byte-exact */
    uint8_t w[300], r[300];
    for (int i = 0; i < 300; i++) w[i] = (uint8_t)(i * 13 + 1);
    CHECK(ee24_present(&ee), "present");
    CHECK(ee24_write(&ee, 100, w, sizeof w) && ee24_read(&ee, 100, r, sizeof r) && !memcmp(w, r, sizeof w), "write across 3 pages");
    CHECK(!ee24_write(&ee, 65500, w, 100) && !ee24_read(&ee, 65500, r, 100), "beyond 64 KB refused");
    CHECK(!ee24_read(&ee, 0, NULL, 4) && !ee24_read(&ee, 0, r, 0) && !ee24_write(NULL, 0, w, 1) && !ee24_present(NULL), "guards");
    busy = 1000; CHECK(!ee24_write(&ee, 0, w, 4), "device busy: write refused"); busy = 0;
    stuck = 1; CHECK(!ee24_write(&ee, 0, w, 4), "never finishes its write cycle: ack polling times out"); stuck = 0; busy = 0;

    /* no snapshot yet */
    CHECK(ib_restore(&b, &ee, apply) == -1, "empty EEPROM: nothing to restore");
    /* snapshot 1 -> slot A; restore replays every record in order */
    CHECK(snapshot(&b, 12, 0xA1) && b.seq == 1 && b.base == 0x0000, "first snapshot in slot A");
    napplied = 0;
    CHECK(ib_restore(&b, &ee, apply) == 12 && napplied == 12 && last_id == 11 && last_type == 7 && last_len == 50 + 11 * 7 &&
          last_data[0] == 0xA1, "restore replays all records");
    /* snapshot 2 -> slot B (A stays as the older copy) */
    CHECK(snapshot(&b, 5, 0xB2) && b.seq == 2 && b.base == IB_SLOT_SIZE, "second snapshot in slot B");
    napplied = 0; CHECK(ib_restore(&b, &ee, apply) == 5 && last_data[0] == 0xB2 && b.seq == 2, "newest slot wins");
    /* power cut during snapshot 3 (slot A): header never written -> slot B still restored */
    fail_write_after = 4; CHECK(!snapshot(&b, 12, 0xC3), "write failure reported"); fail_write_after = -1;
    napplied = 0; CHECK(ib_restore(&b, &ee, apply) == 5 && last_data[0] == 0xB2, "torn snapshot ignored, previous one kept");
    /* bit rot in slot B payload -> CRC rejects it, nothing else valid (A was invalidated) */
    mem[IB_SLOT_SIZE + IB_HDR_SIZE + 20] ^= 0x40;
    CHECK(ib_restore(&b, &ee, apply) == -1, "CRC mismatch rejected");
    mem[IB_SLOT_SIZE + IB_HDR_SIZE + 20] ^= 0x40;
    /* next snapshot goes to A again and becomes newest */
    CHECK(snapshot(&b, 3, 0xD4) && b.base == 0x0000 && b.seq == 3, "alternates back to slot A");
    napplied = 0; CHECK(ib_restore(&b, &ee, apply) == 3 && last_data[0] == 0xD4, "restore newest after alternation");

    /* limits and guards */
    CHECK(ib_begin(&b, &ee), "begin");
    uint8_t big[IB_REC_MAX + 1]; memset(big, 1, sizeof big);
    for (int i = 0; i < 70 && ib_append(&b, 6, 1, big, sizeof big); i++) {}
    CHECK(!b.ok && !ib_commit(&b), "slot overflow refused");
    CHECK(!ib_append(&b, 6, 1, big, 4), "append after failure refused");
    CHECK(ib_begin(&b, &ee) && ib_append(&b, 6, 1, big, sizeof big) && ib_commit(&b), "oversized record stored");
    CHECK(ib_restore(&b, &ee, apply) == -1, "record larger than the restore buffer rejected");
    CHECK(ib_begin(&b, &ee) && !ib_append(&b, 6, 1, NULL, 4), "NULL data refused");
    CHECK(ib_append(&b, 7, 0, NULL, 0) && ib_commit(&b), "empty record ok");
    CHECK(ib_restore(&b, &ee, NULL) == -1, "restore needs an apply function");
    fail_read = 1; CHECK(ib_restore(&b, &ee, apply) == -1, "read failure"); fail_read = 0;
    present = 0;
    CHECK(!ib_begin(&b, &ee) && ib_restore(&b, &ee, apply) == -1 && !ee24_present(&ee), "absent EEPROM");
    present = 1;
    /* header claims a length beyond the slot */
    uint8_t h[16] = { 0x43, 0x49, 0x44, 0x31, 9, 0, 0, 0, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0 };
    CHECK(ee24_write(&ee, 0, h, sizeof h) && ee24_write(&ee, IB_SLOT_SIZE, h, 4), "corrupt headers written");
    CHECK(ib_restore(&b, &ee, apply) == -1, "oversized length rejected");
    /* a record whose length runs past the payload */
    CHECK(ib_begin(&b, &ee) && ib_append(&b, 6, 1, big, 20) && ib_commit(&b), "valid");
    uint16_t base; uint32_t seq, len; CHECK(ib_newest(&b, &base, &seq, &len), "newest");
    uint8_t rh[8] = { 6, 0, 0xF0, 0x00, 1, 0, 0, 0 };     /* len 240 > payload */
    uint8_t pay[28]; CHECK(ee24_read(&ee, (uint16_t)(base + IB_HDR_SIZE), pay, sizeof pay), "read payload");
    memcpy(pay, rh, 8);
    uint8_t nh[16]; CHECK(ee24_read(&ee, base, nh, 16), "hdr");
    const uint32_t crc = ib_crc32(0, pay, sizeof pay);
    nh[12] = (uint8_t)crc; nh[13] = (uint8_t)(crc >> 8); nh[14] = (uint8_t)(crc >> 16); nh[15] = (uint8_t)(crc >> 24);
    CHECK(ee24_write(&ee, (uint16_t)(base + IB_HDR_SIZE), pay, sizeof pay) && ee24_write(&ee, base, nh, 16), "rewrite");
    CHECK(ib_restore(&b, &ee, apply) == -1, "record overrunning the payload rejected");
    CHECK(ib_crc32(0, (const uint8_t *)"123456789", 9) == 0xCBF43926u, "CRC-32 check value");

    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("test_id_backup: all checks passed\n");
    return 0;
}
