#ifndef CAN_DBC_H
#define CAN_DBC_H
/* Broadcast-signal decoding from a constant DBC table (generated from the
 * opendbc Acura ILX 2016 / Honda Nidec definitions, the same platform as the
 * 2013 Civic). Every value is a CANDIDATE until confirmed on this car. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct { int32_t value; const char *text; } can_dbc_val_t;
typedef struct {
    const char *name, *unit;
    uint8_t start, len, little_endian, is_signed;
    double factor, offset;
    uint16_t val_index, val_count;
} can_dbc_sig_t;
typedef struct { uint16_t id; const char *name; uint8_t dlc, sig_count; const can_dbc_sig_t *sigs; } can_dbc_msg_t;

extern const can_dbc_msg_t can_dbc_msgs[];
extern const unsigned can_dbc_msg_count;
extern const can_dbc_val_t *const can_dbc_vals;

const can_dbc_msg_t *can_dbc_find(uint32_t id);                 /* standard IDs; NULL if unknown */
/* Raw integer of a signal; false if the signal lies beyond the received DLC. */
bool        can_dbc_raw(const can_dbc_sig_t *s, const uint8_t *data, uint8_t dlc, int64_t *raw);
double      can_dbc_phys(const can_dbc_sig_t *s, int64_t raw);
const char *can_dbc_text(const can_dbc_sig_t *s, int64_t raw);  /* value name or "" */

#endif
