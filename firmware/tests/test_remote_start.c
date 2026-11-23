#include <string.h>
/*
 * test_remote_start.c - host tests for the remote-start starter state machine.
 * Driven by a simulated ms clock. Returns 0 and prints the pass banner on
 * success; on failure prints a reason to stderr and returns nonzero.
 */
#include <stdio.h>
#include "../Core/Inc/remote_start.h"

static int g_fails = 0;

#define CHECK(cond, msg)                                                    \
    do {                                                                    \
        if (!(cond)) {                                                      \
            fprintf(stderr, "FAIL [%s:%d]: %s\n", __func__, __LINE__, msg); \
            g_fails++;                                                      \
        }                                                                   \
    } while (0)

/* A fully-satisfied set of inputs: engine off, all interlocks OK. */
static remote_start_inputs_t ok_inputs(void)
{
    remote_start_inputs_t in = {0};
    in.rpm = 0u;
    in.rpm_fresh = true;
    in.armed = true;
    in.in_neutral_or_park = true;
    in.brake_pressed = true;
    in.immobilizer_ok = true;
    in.start_requested = false;
    in.cancel_requested = false;
    return in;
}

/* Default cfg: all interlocks required, 2000 ms / 400 rpm / 300 ms. */
static remote_start_cfg_t default_cfg(void)
{
    remote_start_cfg_t c = {0};
    c.timeout_ms = 2000u;
    c.rpm_running_threshold = 400u;
    c.min_crank_ms = 300u;
    c.require_armed = true;
    c.require_neutral_or_park = true;
    c.require_brake = true;
    c.require_immobilizer_ok = true;
    return c;
}

/* ---- Happy path: request -> crank -> RPM rises after min_crank -> STARTED */
static void test_happy_path(void)
{
    remote_start_t rs;
    remote_start_cfg_t c = default_cfg();
    remote_start_init(&rs, &c);

    remote_start_inputs_t in = ok_inputs();
    uint32_t t = 1000u;

    remote_start_event_t ev = remote_start_request(&rs, &in, t);
    CHECK(ev == RS_EV_CRANK_START, "request should start cranking");
    CHECK(remote_start_state(&rs) == RS_CRANKING, "state CRANKING after request");
    CHECK(remote_start_starter_engaged(&rs), "starter engaged while cranking");

    /* Before min_crank_ms: even a high fresh RPM must be ignored (noise). */
    t = 1200u;               /* elapsed 200 < 300 */
    in.rpm = 900u;
    ev = remote_start_poll(&rs, &in, t);
    CHECK(ev == RS_EV_NONE, "high RPM before min_crank must be ignored");
    CHECK(remote_start_state(&rs) == RS_CRANKING, "still cranking before min_crank");
    CHECK(remote_start_starter_engaged(&rs), "starter still engaged before min_crank");

    /* After min_crank_ms with fresh RPM above threshold -> STARTED. */
    t = 1400u;               /* elapsed 400 >= 300 */
    in.rpm = 850u;
    ev = remote_start_poll(&rs, &in, t);
    CHECK(ev == RS_EV_STARTED, "engine should be reported STARTED");
    CHECK(remote_start_state(&rs) == RS_RUNNING, "state RUNNING after start");
    CHECK(!remote_start_starter_engaged(&rs), "starter released in RUNNING");
    CHECK(remote_start_crank_elapsed_ms(&rs, 9999u) == 400u,
          "crank elapsed frozen at 400 ms");

    /* A further poll in RUNNING is a no-op. */
    ev = remote_start_poll(&rs, &in, t + 500u);
    CHECK(ev == RS_EV_NONE, "poll in RUNNING is a no-op");
    CHECK(remote_start_crank_elapsed_ms(&rs, 99999u) == 400u,
          "elapsed stays frozen in RUNNING");
}

/* ---- Timeout path: RPM stays 0, auto-release at exactly timeout_ms ------ */
static void test_timeout_path(void)
{
    remote_start_t rs;
    remote_start_cfg_t c = default_cfg();
    remote_start_init(&rs, &c);

    remote_start_inputs_t in = ok_inputs();
    uint32_t t0 = 5000u;

    CHECK(remote_start_request(&rs, &in, t0) == RS_EV_CRANK_START, "crank starts");

    /* Just before the timeout: still cranking. */
    remote_start_event_t ev = remote_start_poll(&rs, &in, t0 + 1999u);
    CHECK(ev == RS_EV_NONE, "no timeout at 1999 ms");
    CHECK(remote_start_starter_engaged(&rs), "starter engaged just before timeout");

    /* Exactly at timeout_ms -> TIMEOUT. */
    ev = remote_start_poll(&rs, &in, t0 + 2000u);
    CHECK(ev == RS_EV_TIMEOUT, "timeout at exactly timeout_ms");
    CHECK(remote_start_state(&rs) == RS_FAILED, "state FAILED after timeout");
    CHECK(!remote_start_starter_engaged(&rs), "starter released in FAILED");
    CHECK(remote_start_crank_elapsed_ms(&rs, 0u) == 2000u, "elapsed frozen at 2000");
}

/* ---- Each interlock individually blocks with the correct reason --------- */
static void test_interlocks(void)
{
    /* NOT_ARMED */
    {
        remote_start_t rs; remote_start_cfg_t c = default_cfg();
        remote_start_init(&rs, &c);
        remote_start_inputs_t in = ok_inputs();
        in.armed = false;
        remote_start_event_t ev = remote_start_request(&rs, &in, 10u);
        CHECK(ev == RS_EV_BLOCKED, "not armed -> blocked");
        CHECK(remote_start_state(&rs) == RS_BLOCKED, "state BLOCKED (armed)");
        CHECK(remote_start_block_reason(&rs) == RS_BLOCK_NOT_ARMED, "reason NOT_ARMED");
        CHECK(!remote_start_starter_engaged(&rs), "starter off when blocked (armed)");
    }
    /* NOT_NEUTRAL */
    {
        remote_start_t rs; remote_start_cfg_t c = default_cfg();
        remote_start_init(&rs, &c);
        remote_start_inputs_t in = ok_inputs();
        in.in_neutral_or_park = false;
        remote_start_event_t ev = remote_start_request(&rs, &in, 10u);
        CHECK(ev == RS_EV_BLOCKED, "not neutral -> blocked");
        CHECK(remote_start_block_reason(&rs) == RS_BLOCK_NOT_NEUTRAL, "reason NOT_NEUTRAL");
        CHECK(!remote_start_starter_engaged(&rs), "starter off when blocked (neutral)");
    }
    /* NO_BRAKE */
    {
        remote_start_t rs; remote_start_cfg_t c = default_cfg();
        remote_start_init(&rs, &c);
        remote_start_inputs_t in = ok_inputs();
        in.brake_pressed = false;
        remote_start_event_t ev = remote_start_request(&rs, &in, 10u);
        CHECK(ev == RS_EV_BLOCKED, "no brake -> blocked");
        CHECK(remote_start_block_reason(&rs) == RS_BLOCK_NO_BRAKE, "reason NO_BRAKE");
        CHECK(!remote_start_starter_engaged(&rs), "starter off when blocked (brake)");
    }
    /* IMMOBILIZER */
    {
        remote_start_t rs; remote_start_cfg_t c = default_cfg();
        remote_start_init(&rs, &c);
        remote_start_inputs_t in = ok_inputs();
        in.immobilizer_ok = false;
        remote_start_event_t ev = remote_start_request(&rs, &in, 10u);
        CHECK(ev == RS_EV_BLOCKED, "immobilizer -> blocked");
        CHECK(remote_start_block_reason(&rs) == RS_BLOCK_IMMOBILIZER, "reason IMMOBILIZER");
        CHECK(!remote_start_starter_engaged(&rs), "starter off when blocked (immob)");
    }

    /* A disabled interlock requirement does not block. */
    {
        remote_start_t rs; remote_start_cfg_t c = default_cfg();
        c.require_brake = false;
        remote_start_init(&rs, &c);
        remote_start_inputs_t in = ok_inputs();
        in.brake_pressed = false;        /* brake absent, but not required */
        remote_start_event_t ev = remote_start_request(&rs, &in, 10u);
        CHECK(ev == RS_EV_CRANK_START, "no brake OK when not required");
    }
}

/* ---- Already running blocks -------------------------------------------- */
static void test_already_running(void)
{
    remote_start_t rs; remote_start_cfg_t c = default_cfg();
    remote_start_init(&rs, &c);
    remote_start_inputs_t in = ok_inputs();
    in.rpm = 800u;                      /* fresh, above threshold */
    in.rpm_fresh = true;

    remote_start_event_t ev = remote_start_request(&rs, &in, 10u);
    CHECK(ev == RS_EV_BLOCKED, "already running -> blocked");
    CHECK(remote_start_block_reason(&rs) == RS_BLOCK_ALREADY_RUNNING, "reason ALREADY_RUNNING");
    CHECK(!remote_start_starter_engaged(&rs), "starter off when already running");

    /* Requesting again while mid-crank -> BUSY. */
    remote_start_init(&rs, &c);
    in = ok_inputs();
    CHECK(remote_start_request(&rs, &in, 10u) == RS_EV_CRANK_START, "crank starts");
    remote_start_event_t ev2 = remote_start_request(&rs, &in, 20u);
    CHECK(ev2 == RS_EV_BLOCKED, "second request while cranking -> blocked");
    CHECK(remote_start_block_reason(&rs) == RS_BLOCK_BUSY, "reason BUSY");
    /* First crank must be unaffected: still cranking, starter still engaged. */
    CHECK(remote_start_state(&rs) == RS_BLOCKED, "state reflects the rejected request");
    CHECK(!remote_start_starter_engaged(&rs),
          "BUSY rejection leaves machine in BLOCKED (starter off)");
}

/* ---- Cancel during crank -> ABORTED + starter off ---------------------- */
static void test_cancel(void)
{
    remote_start_t rs; remote_start_cfg_t c = default_cfg();
    remote_start_init(&rs, &c);
    remote_start_inputs_t in = ok_inputs();

    CHECK(remote_start_request(&rs, &in, 100u) == RS_EV_CRANK_START, "crank starts");
    (void)remote_start_poll(&rs, &in, 250u);     /* still cranking */
    CHECK(remote_start_starter_engaged(&rs), "engaged before cancel");

    remote_start_cancel(&rs, 400u);
    CHECK(remote_start_state(&rs) == RS_ABORTED, "state ABORTED after cancel");
    CHECK(!remote_start_starter_engaged(&rs), "starter off after cancel");
    CHECK(remote_start_crank_elapsed_ms(&rs, 9999u) == 300u, "elapsed frozen at cancel (300)");

    /* Cancel when not cranking is a no-op. */
    remote_start_t rs2; remote_start_init(&rs2, &c);
    remote_start_cancel(&rs2, 10u);
    CHECK(remote_start_state(&rs2) == RS_IDLE, "cancel when idle is a no-op");

    /* Cancel via poll edge event. */
    remote_start_t rs3; remote_start_init(&rs3, &c);
    remote_start_inputs_t in3 = ok_inputs();
    CHECK(remote_start_request(&rs3, &in3, 0u) == RS_EV_CRANK_START, "crank starts");
    in3.cancel_requested = true;
    CHECK(remote_start_poll(&rs3, &in3, 150u) == RS_EV_ABORTED, "poll cancel edge -> ABORTED");
    CHECK(!remote_start_starter_engaged(&rs3), "starter off after poll cancel");
}

/* ---- Stale/fresh RPM handling: ignore stale high RPM ------------------- */
static void test_stale_rpm(void)
{
    remote_start_t rs; remote_start_cfg_t c = default_cfg();
    remote_start_init(&rs, &c);
    remote_start_inputs_t in = ok_inputs();

    CHECK(remote_start_request(&rs, &in, 0u) == RS_EV_CRANK_START, "crank starts");

    /* Past min_crank, high RPM but STALE -> must be ignored. */
    in.rpm = 1200u;
    in.rpm_fresh = false;
    remote_start_event_t ev = remote_start_poll(&rs, &in, 500u);
    CHECK(ev == RS_EV_NONE, "stale high RPM ignored");
    CHECK(remote_start_state(&rs) == RS_CRANKING, "still cranking on stale RPM");

    /* Same reading, now fresh -> STARTED. */
    in.rpm_fresh = true;
    ev = remote_start_poll(&rs, &in, 600u);
    CHECK(ev == RS_EV_STARTED, "fresh high RPM -> STARTED");
    CHECK(remote_start_state(&rs) == RS_RUNNING, "RUNNING after fresh RPM");

    /* A stale high RPM at request time must NOT block as already-running. */
    remote_start_t rs2; remote_start_init(&rs2, &c);
    remote_start_inputs_t in2 = ok_inputs();
    in2.rpm = 2000u;
    in2.rpm_fresh = false;              /* stale -> treated as 0 */
    CHECK(remote_start_request(&rs2, &in2, 0u) == RS_EV_CRANK_START,
          "stale high RPM at request does not block");
}

/* ---- cfg clamping ------------------------------------------------------ */
static void test_cfg_clamping(void)
{
    remote_start_t rs;

    /* Too-small timeout -> default 2000. */
    {
        remote_start_cfg_t c = default_cfg();
        c.timeout_ms = 100u;
        remote_start_init(&rs, &c);
        remote_start_inputs_t in = ok_inputs();
        remote_start_request(&rs, &in, 0u);
        /* Confirm by behaviour: no timeout before 2000, timeout at 2000. */
        CHECK(remote_start_poll(&rs, &in, 1999u) == RS_EV_NONE, "100 -> 2000: no timeout at 1999");
        CHECK(remote_start_poll(&rs, &in, 2000u) == RS_EV_TIMEOUT, "100 -> 2000: timeout at 2000");
    }

    /* Too-large timeout -> clamp to 10000. */
    {
        remote_start_cfg_t c = default_cfg();
        c.timeout_ms = 99999u;
        remote_start_init(&rs, &c);
        remote_start_inputs_t in = ok_inputs();
        remote_start_request(&rs, &in, 0u);
        CHECK(remote_start_poll(&rs, &in, 9999u) == RS_EV_NONE, "99999 -> 10000: no timeout at 9999");
        CHECK(remote_start_poll(&rs, &in, 10000u) == RS_EV_TIMEOUT, "99999 -> 10000: timeout at 10000");
    }

    /* Zero threshold -> default 400 (a 200 rpm crank must not read running). */
    {
        remote_start_cfg_t c = default_cfg();
        c.rpm_running_threshold = 0u;
        remote_start_init(&rs, &c);
        remote_start_inputs_t in = ok_inputs();
        remote_start_request(&rs, &in, 0u);
        in.rpm = 200u;                  /* below default 400 */
        CHECK(remote_start_poll(&rs, &in, 500u) == RS_EV_NONE,
              "0 threshold -> 400: 200 rpm not running");
        in.rpm = 500u;                  /* above default 400 */
        CHECK(remote_start_poll(&rs, &in, 600u) == RS_EV_STARTED,
              "0 threshold -> 400: 500 rpm running");
    }

    /* Boundary values stay as-is. */
    {
        remote_start_cfg_t c = default_cfg();
        c.timeout_ms = 500u;            /* exactly min, valid */
        remote_start_init(&rs, &c);
        remote_start_inputs_t in = ok_inputs();
        remote_start_request(&rs, &in, 0u);
        CHECK(remote_start_poll(&rs, &in, 499u) == RS_EV_NONE, "min boundary: no timeout at 499");
        CHECK(remote_start_poll(&rs, &in, 500u) == RS_EV_TIMEOUT, "min boundary: timeout at 500");
    }
}

/* ---- init with NULL cfg uses safe defaults ----------------------------- */
static void test_null_cfg_defaults(void)
{
    remote_start_t rs;
    remote_start_init(&rs, NULL);
    CHECK(remote_start_state(&rs) == RS_IDLE, "NULL cfg -> IDLE");
    CHECK(!remote_start_starter_engaged(&rs), "NULL cfg -> starter off");

    /* All interlocks default to required: a missing brake must block. */
    remote_start_inputs_t in = ok_inputs();
    in.brake_pressed = false;
    CHECK(remote_start_request(&rs, &in, 0u) == RS_EV_BLOCKED,
          "defaults require brake");
    CHECK(remote_start_block_reason(&rs) == RS_BLOCK_NO_BRAKE, "default brake reason");

    /* With all interlocks satisfied, default timeout is 2000. */
    remote_start_init(&rs, NULL);
    in = ok_inputs();
    remote_start_request(&rs, &in, 0u);
    CHECK(remote_start_poll(&rs, &in, 1999u) == RS_EV_NONE, "default timeout: none at 1999");
    CHECK(remote_start_poll(&rs, &in, 2000u) == RS_EV_TIMEOUT, "default timeout at 2000");
}

/* ---- name helpers + NULL safety ---------------------------------------- */
static void test_misc(void)
{
    CHECK(remote_start_state_name(RS_CRANKING)[0] == 'C', "state name CRANKING");
    CHECK(remote_start_block_reason_name(RS_BLOCK_BUSY)[0] == 'B', "reason name BUSY");

    /* NULL-pointer guards must not crash. */
    remote_start_init(NULL, NULL);
    remote_start_set_cfg(NULL, NULL);
    remote_start_cancel(NULL, 0u);
    CHECK(remote_start_request(NULL, NULL, 0u) == RS_EV_NONE, "NULL request safe");
    CHECK(remote_start_poll(NULL, NULL, 0u) == RS_EV_NONE, "NULL poll safe");
    CHECK(!remote_start_starter_engaged(NULL), "NULL starter safe");
    CHECK(remote_start_crank_elapsed_ms(NULL, 0u) == 0u, "NULL elapsed safe");
}

int main(void)
{
    test_happy_path();
    test_timeout_path();
    test_interlocks();
    test_already_running();
    test_cancel();
    test_stale_rpm();
    test_cfg_clamping();
    test_null_cfg_defaults();
    test_misc();

    /* set_cfg, poll-driven request, names of every state/reason */
    {
        remote_start_t r; remote_start_cfg_t c = default_cfg();
        remote_start_init(&r, &c);
        c.require_armed = false; c.require_brake = false;
        c.require_neutral_or_park = false; c.require_immobilizer_ok = false;
        remote_start_set_cfg(&r, &c);
        remote_start_set_cfg(NULL, &c);
        remote_start_set_cfg(&r, NULL);
        remote_start_inputs_t in = {0};
        in.start_requested = true;
        CHECK(remote_start_poll(&r, &in, 10u) == RS_EV_CRANK_START, "coverage case");
        CHECK(remote_start_state(&r) == RS_CRANKING, "coverage case");
        CHECK(remote_start_crank_elapsed_ms(&r, 260u) == 250u, "elapsed while cranking");
        static const char *const sn[] = {"IDLE","CRANKING","RUNNING","FAILED","ABORTED","BLOCKED"};
        static const remote_start_state_t sv[] = {RS_IDLE,RS_CRANKING,RS_RUNNING,RS_FAILED,RS_ABORTED,RS_BLOCKED};
        for (unsigned i = 0; i < 6u; i++) CHECK(strcmp(remote_start_state_name(sv[i]), sn[i]) == 0, "coverage case");
        CHECK(strcmp(remote_start_state_name((remote_start_state_t)77), "?") == 0, "coverage case");
        static const char *const bn[] = {"NONE","NOT_ARMED","NOT_NEUTRAL","NO_BRAKE","IMMOBILIZER","ALREADY_RUNNING","BUSY"};
        static const remote_start_block_reason_t bv[] = {RS_BLOCK_NONE,RS_BLOCK_NOT_ARMED,RS_BLOCK_NOT_NEUTRAL,
            RS_BLOCK_NO_BRAKE,RS_BLOCK_IMMOBILIZER,RS_BLOCK_ALREADY_RUNNING,RS_BLOCK_BUSY};
        for (unsigned i = 0; i < 7u; i++) CHECK(strcmp(remote_start_block_reason_name(bv[i]), bn[i]) == 0, "coverage case");
        CHECK(strcmp(remote_start_block_reason_name((remote_start_block_reason_t)77), "?") == 0, "coverage case");
    }

    if (g_fails != 0) {
        fprintf(stderr, "REMOTE_START TESTS FAILED: %d check(s)\n", g_fails);
        return 1;
    }
    printf("ALL REMOTE_START TESTS PASS\n");
    return 0;
}
