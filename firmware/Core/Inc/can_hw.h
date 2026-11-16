#ifndef CAN_HW_H
#define CAN_HW_H
/*
 * can_hw.h - per-controller bxCAN instance for STM32F407 (CAN1 + CAN2).
 *
 * The original firmware drove a single controller (CAN1) with module-level
 * globals. This layer turns every per-controller item into fields of a
 * can_ctrl_t so the firmware can run two independent buses at once:
 *
 *   bus 0  CAN1  F-CAN (powertrain)  PD0 RX / PD1 TX (AF9), S = PD2
 *   bus 1  CAN2  B-CAN (body)        PB12 RX / PB13 TX (AF9), S = PD3
 *
 * TX is push-pull (RM0090 AF output). Each TJA1051 S input (pin 8) is driven
 * by the MCU: HIGH (transceiver silent) in every bxCAN mode except Normal,
 * LOW only in Normal. Silent is thus enforced twice: BTR.SILM and S pin.
 *
 * CAN1 and CAN2 share the 28 filter banks and one filter-master register
 * (RM0090 32.7.4): CAN2 has no registers of its own, so filter setup is done
 * once for both controllers in can_hw_filters_init(). CAN2's clock also
 * requires CAN1's clock to be enabled.
 *
 * Behaviour preserved from the single-bus firmware, now per controller:
 *   - passive silent bitrate scan over bitrate_candidates[], lock on
 *     CAN_LOCK_FRAME_COUNT frames, adaptive dwell, error-driven reacquire;
 *   - transmit only inside an ARM window, bxCAN Normal mode (after leaving
 *     controller SILM), 3-mailbox result reporting, rate limit;
 *   - UDS / ECU-scan client bound to this controller's send path.
 *
 * Transceiver: NXP TJA1051 on each bus (only supported transceiver).
 */
#include <stdbool.h>
#include <stdint.h>
#include "stm32f407xx.h"
#include "can_frame_queue.h"
#include "uds_client.h"
#include "ecu_scan.h"
#include "can_opts.h"

/* Shared scan/arm tuning (were main.c module macros; now shared by both buses). */
#define CAN_DWELL_FAST_MS         200u
#define CAN_DWELL_MEDIUM_MS       500u
#define CAN_DWELL_SLOW_MS        1000u
#define CAN_DWELL_FAST_THRESH_MS   20u
#define CAN_DWELL_MED_THRESH_MS   100u
#define CAN_LOCK_FRAME_COUNT        2u
#define CAN_REACQUIRE_ERROR_MAX    64u  /* tolerate crank brown-out error bursts */
#define TX_ARM_MAX_SECONDS        600u
#define TX_RATE_MAX_PER_SECOND    200u

/* bxCAN controller test / operating mode (RM0090 CAN_BTR LBKM/SILM). */
typedef enum {
    CAN_MODE_SILENT = 0,     /* listen only: never ACKs, never sends        */
    CAN_MODE_NORMAL,         /* full node: ACKs and can transmit            */
    CAN_MODE_LOOPBACK,       /* self-test: TX looped to RX, nothing on bus  */
    CAN_MODE_LOOPBACK_SILENT /* self-test, bus pins kept recessive          */
} can_mode_t;

/* Fixed wiring/identity of one controller. */
typedef struct {
    CAN_TypeDef  *can;            /* CAN1 or CAN2                              */
    uint8_t       bus_index;      /* 0 = CAN1/F-CAN, 1 = CAN2/B-CAN            */
    GPIO_TypeDef *gpio;           /* port carrying RX+TX                      */
    uint8_t       rx_pin, tx_pin; /* pin numbers on that port                 */
    uint8_t       af;             /* alternate function number (9 for CAN)    */
    GPIO_TypeDef *s_gpio;         /* TJA1051 S (silent) pin port, NULL = none  */
    uint8_t       s_pin;          /* TJA1051 S pin number                     */
    uint8_t       filter_bank;    /* accept-all bank index for this controller*/
    bool          is_can2;        /* true for the CAN2 filter slave           */
    const char   *label;          /* "F" or "B" for status lines              */
    uint32_t      preferred_bitrate; /* schematic nominal rate / first scan rate */
} can_cfg_t;

typedef struct {
    const can_cfg_t *cfg;

    /* bitrate scan / lock */
    volatile uint32_t rx_frame_count;
    volatile uint32_t can_error_count;
    volatile uint32_t can_lec_count;
    volatile uint32_t valid_frames_at_rate;
    volatile uint32_t bitrate_error_streak;
    volatile uint32_t reacquire_requested;
    volatile uint32_t busoff_seen;
    volatile uint32_t can_locked;
    volatile bool scan_active;          /* false while ignition is parked/off */
    volatile uint32_t current_bitrate;
    volatile uint32_t isr_min_interval_ms;
    volatile uint32_t isr_last_rx_ts_ms;
    uint32_t scan_started_ms;
    uint32_t scan_index;
    uint32_t last_good_bitrate;     /* last bitrate that locked; tried first on rescan */
    uint32_t current_dwell_ms;
    uint32_t last_dwell_report_ms;
    can_mode_t mode;
    can_opts_t opts;                    /* MCR options + SJW (can_opts.h)   */
    volatile uint32_t last_esr;         /* ESR at the last SCE interrupt   */
    volatile uint32_t fovr_count[2];    /* FIFO0/FIFO1 overruns            */
    volatile uint32_t wakeups;          /* bus-activity wake-ups (AWUM)    */
    volatile bool     asleep;
    /* TX completions recorded by the TX interrupt, consumed by tx_service */
    struct { uint32_t id; uint8_t quiet; uint8_t code; } txres[8];
    volatile uint8_t txres_head, txres_tail;
    volatile uint32_t txres_dropped;

    /* transmit / arm window */
    bool      tx_built;             /* compiled with TX support               */
    bool      tx_armed;
    uint32_t  tx_armed_until_ms;
    uint32_t  tx_mailbox_id[3];
    bool      tx_mailbox_busy[3];
    bool      tx_mailbox_quiet[3];
    uint32_t  tx_tokens;
    uint32_t  tx_tokens_ms;

    /* diagnostics bound to this controller's send path */
    uds_client_t uds;
    bool         uds_ready;
    bool         uds_pad;
    uint8_t      uds_pad_byte;
    uint16_t     uds_emit_pos;
    bool         uds_emitting;
    ecu_scan_t   ecuscan;
    uint32_t     seen_std[2048u / 32u];
    uint32_t     seen_ext_ta[256u / 32u];
} can_ctrl_t;

/* The pclk1 frequency (APB1) both controllers are clocked from; set once. */
void can_hw_set_pclk1(uint32_t hz);

/* GPIO + clock for one controller; call before any bitrate set. */
void can_hw_gpio_init(can_ctrl_t *c, const can_cfg_t *cfg);

/* Filter master setup for BOTH controllers (CAN2SB split + accept-all banks).
 * Call once after both controllers' clocks are enabled. extra: if true, a
 * second accept-all bank per controller routes to FIFO1 as well. */
void can_hw_filters_init(can_ctrl_t *can1, can_ctrl_t *can2, bool use_fifo1);

/* Set controller bitrate and (re)start it in the given mode. Returns false if
 * no bit timing is achievable for this pclk/bitrate. */
bool can_hw_set_bitrate(can_ctrl_t *c, uint32_t bitrate, can_mode_t mode);

/* Begin passive scan when the physical ignition inputs indicate ACC/ON. */
void can_hw_start_passive(can_ctrl_t *c, uint32_t now_ms);
/* Park the controller: disarm into bxCAN SILM and stop the CAN block. */
void can_hw_stop_passive(can_ctrl_t *c);

/* Advance the scan/lock state machine; emits no I/O (caller reports). Returns
 * an event code: 0 none, 1 just locked, 2 moved to next candidate. */
typedef enum { CAN_SCAN_NONE = 0, CAN_SCAN_LOCKED, CAN_SCAN_STEPPED } can_scan_ev_t;
can_scan_ev_t can_hw_scan_step(can_ctrl_t *c, uint32_t now_ms);
uint32_t can_hw_compute_dwell_ms(const can_ctrl_t *c);

/* Drain one RX FIFO (0 or 1) in interrupt context into this bus's queue,
 * tagging each frame with this controller's bus index. now_ms is the current
 * millisecond tick (this layer has no clock of its own). */
void can_hw_rx_drain(can_ctrl_t *c, can_capture_queue_t *q, uint8_t fifo, uint32_t now_ms);

/* Status/error interrupt (ESR): updates counters, requests reacquire. */
void can_hw_sce(can_ctrl_t *c);

/* True while this controller may transmit (armed window open, no bus-off). */
bool can_hw_tx_window_open(const can_ctrl_t *c, uint32_t now_ms);

/* ARM: switches the bxCAN controller from SILM to Normal for `seconds`.
 * The TJA1051 S pin is driven LOW together with bxCAN Normal.
 * Returns an error token (NULL = success). */
const char *can_hw_tx_arm(can_ctrl_t *c, uint32_t seconds, uint32_t now_ms, bool clock_ok);

/* DISARM hardware only: abort mailboxes, restore bxCAN SILM, clear arm state.
 * The caller stops UDS/ECU-scan and emits STATUS lines. */
void can_hw_tx_disarm_hw(can_ctrl_t *c);

/* Service TX mailbox results: calls report(ctrl, mailbox_id, quiet, result)
 * once per completed mailbox, where result is one of "OK","ARB_LOST","TX_ERROR",
 * "ABORTED". Also refills the rate-limit tokens and auto-disarms on timeout/
 * bus-off (returning true when it auto-disarmed, so the caller can emit). */
bool can_hw_tx_service(can_ctrl_t *c, uint32_t now_ms,
                       void (*report)(can_ctrl_t *, uint32_t id, bool quiet,
                                      const char *result),
                       const char **disarm_reason);

/* TX mailbox interrupt: record completed mailboxes (clears RQCP). */
void can_hw_tx_isr(can_ctrl_t *c);
/* Re-apply c->opts (MCR bits + SJW) keeping bitrate and mode. */
bool can_hw_apply_opts(can_ctrl_t *c);
/* Manual bus-off recovery (RM0090 32.7.6: INRQ set then cleared). */
bool can_hw_busoff_recover(can_ctrl_t *c);
/* bxCAN Sleep (low power); with AWUM the controller wakes on bus activity. */
bool can_hw_sleep(can_ctrl_t *c);
bool can_hw_wake(can_ctrl_t *c);
/* Load a free TX mailbox (spends a rate-limit token). false = none free. */
bool can_hw_load_mailbox(can_ctrl_t *c, uint32_t id, bool ext, bool rtr,
                         const uint8_t *data, uint8_t dlc, bool quiet);
bool can_hw_rate_ok(can_ctrl_t *c, uint32_t now_ms);  /* refill + check a token */
void can_hw_abort_mailboxes(can_ctrl_t *c);
bool can_hw_set_controller_mode(can_ctrl_t *c, can_mode_t mode);

/* seen-in-traffic bitmap helpers (ECU scan must not spoof a live ID). */
void can_hw_seen_clear(can_ctrl_t *c);
void can_hw_seen_mark(can_ctrl_t *c, const can_capture_frame_t *f);
bool can_hw_seen_in_traffic(can_ctrl_t *c, uint32_t id, bool ext);

#endif /* CAN_HW_H */
