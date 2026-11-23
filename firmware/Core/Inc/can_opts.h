#ifndef CAN_OPTS_H
#define CAN_OPTS_H
/*
 * can_opts.h - bxCAN operating options (pure logic, RM0090 32.9.2 CAN_MCR,
 * 32.9.7 CAN_BTR, 32.9.6 CAN_ESR). Computes register bits; never touches HW.
 */
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool    abom;   /* automatic bus-off management                    */
    bool    awum;   /* automatic wake-up on bus activity                */
    bool    nart;   /* no automatic retransmission (one-shot)           */
    bool    rflm;   /* receive FIFO locked: keep oldest when full       */
    bool    txfp;   /* TX order by request (true) or by identifier      */
    bool    ttcm;   /* time-triggered mode: 16-bit hw timestamp         */
    bool    dbf;    /* freeze CAN while the core is halted by debugger  */
    uint8_t sjw;    /* resynchronisation jump width, 1..4 tq            */
} can_opts_t;

/* Defaults: ABOM, AWUM, TTCM, DBF on; retransmit on; ID priority; SJW 2. */
can_opts_t can_opts_default(void);
/* CAN_MCR option bits (INRQ/SLEEP/RESET are never included). */
uint32_t can_opts_mcr_bits(const can_opts_t *o);
/* Insert SJW into a BTR value. SJW is clamped to 1..4 and to <= TS2. */
uint32_t can_opts_apply_sjw(uint32_t btr, uint8_t sjw);
/* SJW in tq actually encoded in a BTR value (1..4). */
uint8_t can_opts_btr_sjw(uint32_t btr);
/* Name of a CAN_ESR LEC code (0..7). */
const char *can_lec_name(uint32_t lec);
/* Error state from ESR: "ACTIVE", "WARNING", "PASSIVE" or "BUS_OFF". */
const char *can_error_state(uint32_t esr);
/* Parse "on"/"off"/"1"/"0" option text; false if invalid. */
bool can_opts_set(can_opts_t *o, const char *name, const char *value);
#endif
