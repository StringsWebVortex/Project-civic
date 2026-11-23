#ifndef CIVIC_CAN_DB_H
#define CIVIC_CAN_DB_H
/*
 * civic_can_db.h - labels shown next to CAN IDs on the dashboard.
 *
 * Rule (project owner): only verified facts. Honda publishes no CAN database
 * for the 2013 Civic (Pakistan), so a broadcast ID gets a label here ONLY
 * after it has been proven on this car by reverse engineering (capture with a
 * marker line, bit change confirmed by repeating; evidence kept with
 * docs/civic_2013_pk.dbc). Community databases (opendbc etc.) and guesses are
 * not a source: the earlier opendbc-derived table was removed for that reason.
 * Until an ID is proven it is shown unlabelled.
 *
 * The only entries today are the OBD-II diagnostic addresses on CAN
 * (functional request 0x7DF, ECU #1 physical request 0x7E0 / response 0x7E8),
 * which are fixed by the OBD-on-CAN standard, not by Honda. Byte 0 is the
 * ISO-TP single-frame PCI (payload length).
 *
 * Transceiver: TJA1051 (NXP) - only supported CAN transceiver.
 */

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t    id;
    uint8_t     is_extended;          /* 1 = 29-bit, 0 = 11-bit */
    const char *frame_label;
    /* Per-byte labels; NULL means byte purpose unknown */
    const char *byte_labels[8];
} civic_can_entry_t;

static const civic_can_entry_t civic_can_db[] = {
    /* ── OBD-II on CAN diagnostic addresses (standard, not Honda) ── */
    { 0x7DF, 0, "OBD-II functional request",
      { "Len", "Mode", "PID", NULL, NULL, NULL, NULL, NULL } },
    { 0x7E0, 0, "OBD-II physical request (ECU #1)",
      { "Len", "Mode", "PID", NULL, NULL, NULL, NULL, NULL } },
    { 0x7E8, 0, "OBD-II response (ECU #1)",
      { "Len", "Mode_Resp", "PID", "A", "B", "C", "D", NULL } },
    /* ── Civic broadcast IDs: add here only after RE proof on this car ── */
};

#define CIVIC_CAN_DB_COUNT ((uint32_t)(sizeof(civic_can_db) / sizeof(civic_can_db[0])))

/* civic_can_db_lookup() - return entry for given ID, or NULL if not in table. */
static inline const civic_can_entry_t *civic_can_db_lookup(uint32_t id, uint8_t extended)
{
    for (uint32_t i = 0u; i < CIVIC_CAN_DB_COUNT; i++) {
        if (civic_can_db[i].id == id && civic_can_db[i].is_extended == extended)
            return &civic_can_db[i];
    }
    return NULL;
}

#endif /* CIVIC_CAN_DB_H */
