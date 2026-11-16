#ifndef REMOTE_START_H
#define REMOTE_START_H
/*
 * remote_start.h - remote engine-start starter-control state machine.
 *
 * Models the starter behaviour of a keyed-ignition car whose key is already
 * physically at RUN (position II): a request engages the starter motor (crank)
 * and the machine RELEASES it the instant the engine is confirmed running
 * (RPM above a threshold) - exactly like a key springing back from START to
 * RUN - OR auto-releases after a configurable timeout if the engine never
 * caught. Safety interlocks are enforced before any crank.
 *
 * PURE LOGIC: no hardware register access, no I/O, no malloc, no OS. All state
 * lives in a caller-owned remote_start_t. The caller reads
 * remote_start_starter_engaged() each tick and wires it to the starter
 * relay / CAN message, and feeds live inputs (time, RPM, interlocks) in.
 *
 * Typical use:
 *   remote_start_t rs;
 *   remote_start_init(&rs, NULL);               // NULL -> safe defaults
 *   ... on dashboard button edge:
 *       remote_start_request(&rs, &inputs, now_ms);
 *   ... every poll tick:
 *       remote_start_poll(&rs, &inputs, now_ms);
 *       drive_starter_relay(remote_start_starter_engaged(&rs));
 *
 * Freestanding headers only.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ---- Defaults and accepted ranges ------------------------------------- */
#define RS_TIMEOUT_DEFAULT_MS   2000u
#define RS_TIMEOUT_MIN_MS        500u
#define RS_TIMEOUT_MAX_MS      10000u
#define RS_RPM_THRESHOLD_DEFAULT 400u
#define RS_MIN_CRANK_DEFAULT_MS  300u

/* ---- States ----------------------------------------------------------- */
typedef enum {
    RS_IDLE = 0,    /* no start in progress; starter off                    */
    RS_CRANKING,    /* starter engaged, waiting for RPM or timeout          */
    RS_RUNNING,     /* engine confirmed running, starter released           */
    RS_FAILED,      /* timed out without catching, starter released         */
    RS_ABORTED,     /* cancelled by caller, starter released                */
    RS_BLOCKED      /* an interlock prevented the start, starter never on    */
} remote_start_state_t;

/* ---- Events returned by request()/poll() ------------------------------ */
typedef enum {
    RS_EV_NONE = 0,
    RS_EV_CRANK_START,  /* transitioned IDLE/terminal -> CRANKING            */
    RS_EV_STARTED,      /* engine caught; released starter, -> RUNNING       */
    RS_EV_TIMEOUT,      /* timed out; released starter, -> FAILED            */
    RS_EV_BLOCKED,      /* interlock refused the start, -> BLOCKED           */
    RS_EV_ABORTED       /* cancelled; released starter, -> ABORTED           */
} remote_start_event_t;

/* ---- Reason a request was blocked ------------------------------------- */
typedef enum {
    RS_BLOCK_NONE = 0,
    RS_BLOCK_NOT_ARMED,        /* require_armed and !armed                   */
    RS_BLOCK_NOT_NEUTRAL,      /* require_neutral_or_park and !in_neutral... */
    RS_BLOCK_NO_BRAKE,         /* require_brake and !brake_pressed           */
    RS_BLOCK_IMMOBILIZER,      /* require_immobilizer_ok and !immobilizer_ok */
    RS_BLOCK_ALREADY_RUNNING,  /* engine already running at request time     */
    RS_BLOCK_BUSY              /* a crank is already in progress             */
} remote_start_block_reason_t;

/* ---- Configuration (caller sets; init/set_cfg validate & clamp) ------- */
typedef struct {
    uint32_t timeout_ms;            /* max crank time; clamped to [500,10000],
                                     * bad/too-small values default to 2000  */
    uint16_t rpm_running_threshold; /* RPM above which engine is "running";
                                     * 0 defaults to 400                      */
    uint32_t min_crank_ms;          /* minimum crank before a "running" RPM
                                     * reading is accepted (ignore startup
                                     * RPM noise); default 300                */
    bool require_armed;             /* interlock: system armed               */
    bool require_neutral_or_park;   /* interlock: gearbox in neutral/park    */
    bool require_brake;             /* interlock: brake pressed              */
    bool require_immobilizer_ok;    /* interlock: immobilizer cleared        */
} remote_start_cfg_t;

/* ---- Runtime inputs, sampled each tick -------------------------------- */
typedef struct {
    uint16_t rpm;                   /* latest engine RPM                     */
    bool     rpm_fresh;             /* false -> RPM stale/unknown, treat as 0 */
    bool     armed;
    bool     in_neutral_or_park;
    bool     brake_pressed;
    bool     immobilizer_ok;
    /* Edge events. request()/cancel() are the usual way to signal these, but
     * poll() also honours them so a caller may drive everything through poll()
     * alone: poll() acts on cancel_requested, then start_requested, before
     * watching RPM/timeout. */
    bool     start_requested;
    bool     cancel_requested;
} remote_start_inputs_t;

/* ---- State ------------------------------------------------------------ */
typedef struct {
    remote_start_cfg_t          cfg;
    remote_start_state_t        state;
    remote_start_block_reason_t block_reason;
    uint32_t crank_start_ms;        /* when the current crank began          */
    uint32_t crank_frozen_ms;       /* crank duration frozen at release       */
} remote_start_t;

/* ---- API -------------------------------------------------------------- */

/* Initialise to RS_IDLE, starter off. cfg==NULL applies safe defaults
 * (2000 ms, 400 rpm, 300 ms min crank, every interlock required). A non-NULL
 * cfg is validated and clamped exactly as remote_start_set_cfg() does. */
void remote_start_init(remote_start_t *rs, const remote_start_cfg_t *cfg);

/* Replace configuration, validating/clamping timeout and threshold.
 * Does not change the current state. cfg==NULL is ignored. */
void remote_start_set_cfg(remote_start_t *rs, const remote_start_cfg_t *cfg);

/* Edge handler for a start request. Checks interlocks against *in; on success
 * engages the starter (-> RS_CRANKING) and returns RS_EV_CRANK_START, else
 * goes RS_BLOCKED and returns RS_EV_BLOCKED (block reason via
 * remote_start_block_reason()). */
remote_start_event_t remote_start_request(remote_start_t *rs,
                                          const remote_start_inputs_t *in,
                                          uint32_t now_ms);

/* Edge handler for a cancel request. If a crank is in progress, releases the
 * starter and goes RS_ABORTED; otherwise has no effect. */
void remote_start_cancel(remote_start_t *rs, uint32_t now_ms);

/* Periodic tick. Honours edge events in *in, then while cranking watches RPM
 * and the timeout: emits RS_EV_STARTED (release, -> RUNNING) once the engine
 * is confirmed running, or RS_EV_TIMEOUT (release, -> FAILED) at the timeout.
 * Returns RS_EV_NONE when nothing changed. */
remote_start_event_t remote_start_poll(remote_start_t *rs,
                                       const remote_start_inputs_t *in,
                                       uint32_t now_ms);

/* true only while cranking - the caller drives the starter relay/CAN from
 * this. Guaranteed false in RUNNING, FAILED, ABORTED, BLOCKED and IDLE. */
bool remote_start_starter_engaged(const remote_start_t *rs);

/* Milliseconds spent cranking: live while RS_CRANKING, otherwise the frozen
 * duration of the last crank (0 if none has run). */
uint32_t remote_start_crank_elapsed_ms(const remote_start_t *rs, uint32_t now_ms);

remote_start_state_t        remote_start_state(const remote_start_t *rs);
remote_start_block_reason_t remote_start_block_reason(const remote_start_t *rs);

const char *remote_start_state_name(remote_start_state_t s);
const char *remote_start_block_reason_name(remote_start_block_reason_t r);

#endif /* REMOTE_START_H */
