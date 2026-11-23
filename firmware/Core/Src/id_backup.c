/* Identity backup in a 24LC512 (see id_backup.h). */
#include "id_backup.h"

#include <string.h>

uint32_t ib_crc32(uint32_t crc, const uint8_t *d, size_t n)
{
    crc = ~crc;
    while (n--) { crc ^= *d++; for (int k = 0; k < 8; k++) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u))); }
    return ~crc;
}

static void put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }
static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }

/* Header valid and payload CRC matches. */
static bool slot_valid(const ee24_t *ee, uint16_t base, uint32_t *seq, uint32_t *len)
{
    uint8_t h[IB_HDR_SIZE];
    if (!ee24_read(ee, base, h, sizeof h) || get32(h) != IB_MAGIC) return false;
    const uint32_t n = get32(h + 8);
    if (n > IB_SLOT_SIZE - IB_HDR_SIZE) return false;
    uint32_t crc = 0u; uint8_t chunk[64];
    for (uint32_t off = 0u; off < n; off += sizeof chunk) {
        const size_t c = (n - off) < sizeof chunk ? (size_t)(n - off) : sizeof chunk;
        if (!ee24_read(ee, (uint16_t)(base + IB_HDR_SIZE + off), chunk, c)) return false;
        crc = ib_crc32(crc, chunk, c);
    }
    if (crc != get32(h + 12)) return false;
    *seq = get32(h + 4); *len = n;
    return true;
}

bool ib_newest(ib_t *b, uint16_t *base, uint32_t *seq, uint32_t *len)
{
    uint32_t sa = 0u, sb = 0u, la = 0u, lb = 0u;
    const bool va = slot_valid(b->ee, 0x0000u, &sa, &la), vb = slot_valid(b->ee, (uint16_t)IB_SLOT_SIZE, &sb, &lb);
    if (!va && !vb) return false;
    const bool pick_a = va && (!vb || sa > sb);
    *base = pick_a ? 0x0000u : (uint16_t)IB_SLOT_SIZE;
    *seq = pick_a ? sa : sb; *len = pick_a ? la : lb;
    return true;
}

static bool flush_page(ib_t *b)
{
    if (b->page_n == 0u) return true;
    if (!ee24_write(b->ee, b->page_at, b->page, b->page_n)) return b->ok = false;
    b->page_at = (uint16_t)(b->page_at + b->page_n); b->page_n = 0u;
    return true;
}

static bool put_bytes(ib_t *b, const uint8_t *d, size_t n)
{
    if (b->len + n > IB_SLOT_SIZE - IB_HDR_SIZE) return b->ok = false;
    b->crc = ib_crc32(b->crc, d, n);
    b->len += (uint32_t)n;
    while (n) {
        const size_t room = EE24_PAGE - ((b->page_at + b->page_n) % EE24_PAGE);
        const size_t c = n < room ? n : room;
        memcpy(b->page + b->page_n, d, c); b->page_n = (uint8_t)(b->page_n + c);
        d += c; n -= c;
        if (((uint32_t)b->page_at + b->page_n) % EE24_PAGE == 0u && !flush_page(b)) return false;  /* page end */
    }
    return true;
}

bool ib_begin(ib_t *b, const ee24_t *ee)
{
    memset(b, 0, sizeof *b);
    b->ee = ee;
    if (!ee24_present(ee)) return false;
    uint16_t base = 0u; uint32_t seq = 0u, len = 0u;
    if (ib_newest(b, &base, &seq, &len)) b->seq = seq;
    b->base = (base == 0x0000u && b->seq) ? (uint16_t)IB_SLOT_SIZE : 0x0000u;   /* overwrite the older */
    /* invalidate the target slot first: a torn snapshot must never look valid */
    const uint8_t zero[4] = { 0, 0, 0, 0 };
    if (!ee24_write(ee, b->base, zero, sizeof zero)) return false;
    b->page_at = (uint16_t)(b->base + IB_HDR_SIZE);
    return b->ok = true;
}

bool ib_append(ib_t *b, uint8_t type, uint32_t id, const void *data, uint16_t len)
{
    if (!b->ok || (len && !data)) return false;
    uint8_t h[8] = { type, 0u, (uint8_t)len, (uint8_t)(len >> 8) };
    put32(h + 4, id);
    return put_bytes(b, h, sizeof h) && put_bytes(b, (const uint8_t *)data, len);
}

bool ib_commit(ib_t *b)
{
    if (!b->ok || !flush_page(b)) return false;
    uint8_t h[IB_HDR_SIZE];
    put32(h, IB_MAGIC); put32(h + 4, b->seq + 1u); put32(h + 8, b->len); put32(h + 12, b->crc);
    if (!ee24_write(b->ee, b->base, h, sizeof h)) return b->ok = false;
    b->seq++;
    return true;
}

int ib_restore(ib_t *b, const ee24_t *ee, ib_apply_fn apply)
{
    memset(b, 0, sizeof *b);
    b->ee = ee;
    uint16_t base; uint32_t seq, len;
    if (!apply || !ee24_present(ee) || !ib_newest(b, &base, &seq, &len)) return -1;
    b->seq = seq;
    int n = 0;
    uint8_t data[IB_REC_MAX];                         /* boot-time only: stack, not RAM */
    for (uint32_t off = 0u; off + 8u <= len; ) {
        uint8_t h[8];
        if (!ee24_read(ee, (uint16_t)(base + IB_HDR_SIZE + off), h, sizeof h)) return -1;
        const uint16_t rl = (uint16_t)(h[2] | (h[3] << 8));
        if (rl > IB_REC_MAX || off + 8u + rl > len) return -1;
        if (rl && !ee24_read(ee, (uint16_t)(base + IB_HDR_SIZE + off + 8u), data, rl)) return -1;
        apply(h[0], get32(h + 4), data, rl);
        n++;
        off += 8u + rl;
    }
    return n;
}
