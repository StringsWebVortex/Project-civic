/* DBC signal extraction (Vector DBC conventions). See can_dbc.h. */
#include "can_dbc.h"

const can_dbc_msg_t *can_dbc_find(uint32_t id)
{
    unsigned lo = 0, hi = can_dbc_msg_count;          /* table is sorted by id */
    while (lo < hi) {
        const unsigned mid = (lo + hi) / 2u;
        if (can_dbc_msgs[mid].id == id) return &can_dbc_msgs[mid];
        if (can_dbc_msgs[mid].id < id) lo = mid + 1u; else hi = mid;
    }
    return NULL;
}

bool can_dbc_raw(const can_dbc_sig_t *s, const uint8_t *data, uint8_t dlc, int64_t *raw)
{
    if (s->len == 0u || s->len > 64u || dlc > 8u) return false;
    uint64_t v = 0;
    if (s->little_endian) {
        /* Intel: start = LSB, bits ascend through the bytes. */
        for (unsigned i = 0; i < s->len; i++) {
            const unsigned bit = s->start + i, byte = bit / 8u;
            if (byte >= dlc) return false;
            v |= (uint64_t)((data[byte] >> (bit % 8u)) & 1u) << i;
        }
    } else {
        /* Motorola: start = MSB; next bit is one lower, wrapping to bit 7 of
         * the following byte after bit 0 ("sawtooth" numbering). */
        unsigned bit = s->start;
        for (unsigned i = 0; i < s->len; i++) {
            const unsigned byte = bit / 8u;
            if (byte >= dlc) return false;
            v = (v << 1) | ((data[byte] >> (bit % 8u)) & 1u);
            bit = (bit % 8u == 0u) ? (byte + 1u) * 8u + 7u : bit - 1u;
        }
    }
    if (s->is_signed && s->len < 64u && (v >> (s->len - 1u)) & 1u) v |= ~((1ull << s->len) - 1u);
    *raw = (int64_t)v;
    return true;
}

double can_dbc_phys(const can_dbc_sig_t *s, int64_t raw) { return (double)raw * s->factor + s->offset; }

const char *can_dbc_text(const can_dbc_sig_t *s, int64_t raw)
{
    for (unsigned i = 0; i < s->val_count; i++)
        if (can_dbc_vals[s->val_index + i].value == raw) return can_dbc_vals[s->val_index + i].text;
    return "";
}
