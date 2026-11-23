/*
 * remote_start.c - remote engine-start starter-control state machine.
 * See remote_start.h for the model and contract. Pure logic, freestanding.
 */
#include "remote_start.h"

/* Wrap-safe "has (now - t0) reached d ms?" for a free-running ms clock. */
static bool elapsed_reached(uint32_t now, uint32_t t0, uint32_t d)
{
    return (uint32_t)(now - t0) >= d;
}

/* Effective RPM: a stale/unknown sample counts as 0 (engine state unknown). */
static uint16_t eff_rpm(const remote_start_inputs_t *in)
{
    return (in != NULL && in->rpm_fresh) ? in->rpm : 0u;
}

static void apply_cfg(remote_start_t *rs, const remote_start_cfg_t *cfg)
{
    rs->cfg = *cfg;

    /* Timeout: too-small / bad values default to 2000; too-large clamps to
     * the maximum. (100 -> 2000, 99999 -> 10000.) */
    if (rs->cfg.timeout_ms < RS_TIMEOUT_MIN_MS) {
        rs->cfg.timeout_ms = RS_TIMEOUT_DEFAULT_MS;
    } else if (rs->cfg.timeout_ms > RS_TIMEOUT_MAX_MS) {
        rs->cfg.timeout_ms = RS_TIMEOUT_MAX_MS;
    }

    /* Threshold: 0 is meaningless (any crank would read "running"); default. */
    if (rs->cfg.rpm_running_threshold == 0u) {
        rs->cfg.rpm_running_threshold = (uint16_t)RS_RPM_THRESHOLD_DEFAULT;
    }

    /* min_crank_ms: 0 is accepted (take a running reading immediately). It is
     * intentionally not clamped against timeout_ms; if it exceeds the timeout
     * the crank simply always times out, which is a valid (if odd) config. */
}

void remote_start_set_cfg(remote_start_t *rs, const remote_start_cfg_t *cfg)
{
    if (rs == NULL || cfg == NULL) return;
    apply_cfg(rs, cfg);
}

void remote_start_init(remote_start_t *rs, const remote_start_cfg_t *cfg)
{
    if (rs == NULL) return;

    rs->state          = RS_IDLE;
    rs->block_reason   = RS_BLOCK_NONE;
    rs->crank_start_ms = 0u;
    rs->crank_frozen_ms = 0u;

    if (cfg != NULL) {
        apply_cfg(rs, cfg);
    } else {
        /* Safe defaults: every interlock required. */
        remote_start_cfg_t def = {
            .timeout_ms            = RS_TIMEOUT_DEFAULT_MS,
            .rpm_running_threshold = (uint16_t)RS_RPM_THRESHOLD_DEFAULT,
            .min_crank_ms          = RS_MIN_CRANK_DEFAULT_MS,
            .require_armed            = true,
            .require_neutral_or_park  = true,
            .require_brake            = true,
            .require_immobilizer_ok   = true,
        };
        apply_cfg(rs, &def);
    }
}

/* Decide whether a start is allowed right now; returns the blocking reason,
 * or RS_BLOCK_NONE if the crank may proceed. */
static remote_start_block_reason_t check_block(const remote_start_t *rs,
                                               const remote_start_inputs_t *in)
{
    /* State guards first: never engage the starter if a crank is in progress
     * or the engine is already (believed to be) running. */
    if (rs->state == RS_CRANKING) return RS_BLOCK_BUSY;
    if (rs->state == RS_RUNNING)  return RS_BLOCK_ALREADY_RUNNING;

    /* Engine already running per a fresh RPM sample. */
    if (eff_rpm(in) >= rs->cfg.rpm_running_threshold) {
        return RS_BLOCK_ALREADY_RUNNING;
    }

    /* Safety interlocks, reported in enum order. */
    if (rs->cfg.require_armed && !in->armed)
        return RS_BLOCK_NOT_ARMED;
    if (rs->cfg.require_neutral_or_park && !in->in_neutral_or_park)
        return RS_BLOCK_NOT_NEUTRAL;
    if (rs->cfg.require_brake && !in->brake_pressed)
        return RS_BLOCK_NO_BRAKE;
    if (rs->cfg.require_immobilizer_ok && !in->immobilizer_ok)
        return RS_BLOCK_IMMOBILIZER;

    return RS_BLOCK_NONE;
}

remote_start_event_t remote_start_request(remote_start_t *rs,
                                          const remote_start_inputs_t *in,
                                          uint32_t now_ms)
{
    if (rs == NULL || in == NULL) return RS_EV_NONE;

    remote_start_block_reason_t reason = check_block(rs, in);
    if (reason != RS_BLOCK_NONE) {
        rs->block_reason = reason;
        rs->state        = RS_BLOCKED;
        return RS_EV_BLOCKED;
    }

    rs->block_reason   = RS_BLOCK_NONE;
    rs->state          = RS_CRANKING;
    rs->crank_start_ms = now_ms;
    rs->crank_frozen_ms = 0u;
    return RS_EV_CRANK_START;
}

void remote_start_cancel(remote_start_t *rs, uint32_t now_ms)
{
    if (rs == NULL) return;
    if (rs->state == RS_CRANKING) {
        rs->crank_frozen_ms = (uint32_t)(now_ms - rs->crank_start_ms);
        rs->state           = RS_ABORTED;   /* starter released */
    }
}

remote_start_event_t remote_start_poll(remote_start_t *rs,
                                       const remote_start_inputs_t *in,
                                       uint32_t now_ms)
{
    if (rs == NULL || in == NULL) return RS_EV_NONE;

    /* Honour edge events so poll() can be the sole driver. Cancel wins. */
    if (in->cancel_requested && rs->state == RS_CRANKING) {
        remote_start_cancel(rs, now_ms);
        return RS_EV_ABORTED;
    }
    if (in->start_requested && rs->state != RS_CRANKING) {
        return remote_start_request(rs, in, now_ms);
    }

    if (rs->state != RS_CRANKING) return RS_EV_NONE;

    uint32_t elapsed = (uint32_t)(now_ms - rs->crank_start_ms);

    /* Accept a "running" reading only after min_crank_ms (startup RPM noise)
     * and only from a fresh sample. Checked before the timeout so a catch on
     * the very last tick still counts as STARTED. */
    if (elapsed >= rs->cfg.min_crank_ms &&
        in->rpm_fresh &&
        in->rpm >= rs->cfg.rpm_running_threshold) {
        rs->crank_frozen_ms = elapsed;
        rs->state           = RS_RUNNING;   /* starter released */
        return RS_EV_STARTED;
    }

    /* Auto-release at the timeout (reached exactly at timeout_ms). */
    if (elapsed_reached(now_ms, rs->crank_start_ms, rs->cfg.timeout_ms)) {
        rs->crank_frozen_ms = elapsed;
        rs->state           = RS_FAILED;    /* starter released */
        return RS_EV_TIMEOUT;
    }

    return RS_EV_NONE;
}

bool remote_start_starter_engaged(const remote_start_t *rs)
{
    return rs != NULL && rs->state == RS_CRANKING;
}

uint32_t remote_start_crank_elapsed_ms(const remote_start_t *rs, uint32_t now_ms)
{
    if (rs == NULL) return 0u;
    if (rs->state == RS_CRANKING) {
        return (uint32_t)(now_ms - rs->crank_start_ms);
    }
    return rs->crank_frozen_ms;
}

remote_start_state_t remote_start_state(const remote_start_t *rs)
{
    return (rs != NULL) ? rs->state : RS_IDLE;
}

remote_start_block_reason_t remote_start_block_reason(const remote_start_t *rs)
{
    return (rs != NULL) ? rs->block_reason : RS_BLOCK_NONE;
}

const char *remote_start_state_name(remote_start_state_t s)
{
    switch (s) {
        case RS_IDLE:     return "IDLE";
        case RS_CRANKING: return "CRANKING";
        case RS_RUNNING:  return "RUNNING";
        case RS_FAILED:   return "FAILED";
        case RS_ABORTED:  return "ABORTED";
        case RS_BLOCKED:  return "BLOCKED";
        default:          return "?";
    }
}

const char *remote_start_block_reason_name(remote_start_block_reason_t r)
{
    switch (r) {
        case RS_BLOCK_NONE:            return "NONE";
        case RS_BLOCK_NOT_ARMED:       return "NOT_ARMED";
        case RS_BLOCK_NOT_NEUTRAL:     return "NOT_NEUTRAL";
        case RS_BLOCK_NO_BRAKE:        return "NO_BRAKE";
        case RS_BLOCK_IMMOBILIZER:     return "IMMOBILIZER";
        case RS_BLOCK_ALREADY_RUNNING: return "ALREADY_RUNNING";
        case RS_BLOCK_BUSY:            return "BUSY";
        default:                       return "?";
    }
}
