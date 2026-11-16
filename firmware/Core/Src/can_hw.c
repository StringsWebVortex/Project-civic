/*
 * can_hw.c - per-controller bxCAN implementation (CAN1 + CAN2) for STM32F407.
 * See can_hw.h. The scan/lock/TX/arm logic is the single-bus firmware's
 * behaviour, made per-controller. Register sequences follow RM0090 chapter 32
 * and match ST HAL_CAN_Init ordering (the original main.c cited the same).
 */
#include "can_hw.h"
#include "can_filter.h"
#include "periph_timing.h"
#include <string.h>

/* APB1 clock both controllers run from (set once from main). */
static uint32_t s_pclk1_hz;

/* Passive scan candidates. Each controller starts at its schematic nominal
 * bitrate (F-CAN 500 kbit/s; B-CAN 125 kbit/s), then scans independently. */
static const uint32_t s_bitrate_candidates[] = {
    500000u, 1000000u, 250000u, 125000u, 100000u,
    83333u,  50000u,   33333u,  20000u,  10000u
};
#define BITRATE_CANDIDATE_COUNT \
    ((uint32_t)(sizeof(s_bitrate_candidates)/sizeof(s_bitrate_candidates[0])))

void can_hw_set_pclk1(uint32_t hz) { s_pclk1_hz = hz; }

/* ── GPIO ─────────────────────────────────────────────────────────────── */
static void gpio_clock_enable(GPIO_TypeDef *g)
{
    if      (g == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (g == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (g == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    else if (g == GPIOD) RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    else if (g == GPIOE) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    (void)RCC->AHB1ENR;
}

static void pin_af(GPIO_TypeDef *g, uint8_t pin, uint8_t af)
{
    g->PUPDR   &= ~(3u << (2u * pin));                         /* no pull: FT pin above VDD */
    g->OTYPER  &= ~(1u << pin);                                /* push-pull */
    g->OSPEEDR |= (3u << (2u * pin));                          /* high speed */
    const uint8_t reg = pin >> 3, sh = (uint8_t)((pin & 7u) * 4u);
    g->AFR[reg] = (g->AFR[reg] & ~(15u << sh)) | ((uint32_t)af << sh);
    g->MODER    = (g->MODER & ~(3u << (2u * pin))) | (2u << (2u * pin)); /* AF mode */
}

void can_hw_gpio_init(can_ctrl_t *c, const can_cfg_t *cfg)
{
    memset(c, 0, sizeof *c);
    c->cfg = cfg;
    c->mode = CAN_MODE_SILENT;
    c->current_dwell_ms = CAN_DWELL_SLOW_MS;
    c->tx_tokens = TX_RATE_MAX_PER_SECOND;
    c->uds_pad = true;
    gpio_clock_enable(cfg->gpio);
    /* TJA1051 RXD is a 5 V output: RX pins are FT (DS8626). TXD push-pull. */
    if (cfg->s_gpio) {
        gpio_clock_enable(cfg->s_gpio);
        cfg->s_gpio->BSRR = (1u << cfg->s_pin);                /* S HIGH = silent first */
        cfg->s_gpio->OTYPER &= ~(1u << cfg->s_pin);
        cfg->s_gpio->PUPDR  &= ~(3u << (2u * cfg->s_pin));
        cfg->s_gpio->MODER   = (cfg->s_gpio->MODER & ~(3u << (2u * cfg->s_pin))) |
                               (1u << (2u * cfg->s_pin));
    }
    pin_af(cfg->gpio, cfg->rx_pin, cfg->af);
    pin_af(cfg->gpio, cfg->tx_pin, cfg->af);
}

/* TJA1051 S: LOW (Normal) only for bxCAN Normal, HIGH (silent) otherwise. */
static void transceiver_follow_mode(const can_cfg_t *cfg, can_mode_t m)
{
    if (!cfg->s_gpio) return;
    cfg->s_gpio->BSRR = (m == CAN_MODE_NORMAL) ? (1u << (cfg->s_pin + 16u))
                                               : (1u << cfg->s_pin);
}

/* ── controller init/mode ────────────────────────────────────────────── */
static bool can_enter_init(CAN_TypeDef *can)
{
    can->MCR = (can->MCR & ~CAN_MCR_SLEEP) | CAN_MCR_INRQ;
    uint32_t to = 1000000u;
    while (!(can->MSR & CAN_MSR_INAK) && --to) { }
    if (!to) return false;
    to = 1000000u;
    while ((can->MSR & CAN_MSR_SLAK) && --to) { }
    return to != 0u;
}

static bool can_leave_init(CAN_TypeDef *can)
{
    can->MCR &= ~CAN_MCR_INRQ;
    uint32_t to = 1000000u;
    while ((can->MSR & CAN_MSR_INAK) && --to) { }
    return to != 0u;
}

static uint32_t mode_btr_bits(can_mode_t m)
{
    switch (m) {
    case CAN_MODE_NORMAL:          return 0u;
    case CAN_MODE_LOOPBACK:        return CAN_BTR_LBKM;
    case CAN_MODE_LOOPBACK_SILENT: return CAN_BTR_LBKM | CAN_BTR_SILM;
    case CAN_MODE_SILENT:
    default:                       return CAN_BTR_SILM;
    }
}

static void clocks_for(const can_cfg_t *cfg)
{
    /* CAN2 is a slave of CAN1: its registers and the shared filters need
     * CAN1's clock too (RM0090 32.9). */
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    if (cfg->is_can2) RCC->APB1ENR |= RCC_APB1ENR_CAN2EN;
    (void)RCC->APB1ENR;
}

static void nvic_enable_for(const can_cfg_t *cfg)
{
    if (cfg->is_can2) {
        NVIC_EnableIRQ(CAN2_RX0_IRQn);
        NVIC_EnableIRQ(CAN2_RX1_IRQn);
        NVIC_EnableIRQ(CAN2_SCE_IRQn);
    } else {
        NVIC_EnableIRQ(CAN1_RX0_IRQn);
        NVIC_EnableIRQ(CAN1_RX1_IRQn);
        NVIC_EnableIRQ(CAN1_SCE_IRQn);
    }
}

bool can_hw_set_bitrate(can_ctrl_t *c, uint32_t bitrate, can_mode_t mode)
{
    if (!c || !c->cfg || c->tx_armed) return false;
    if (mode == CAN_MODE_NORMAL &&
        (!c->can_locked || !can_tx_bitrate_allowed(bitrate))) return false;
    CAN_TypeDef *can = c->cfg->can;
    uint32_t btr, actual;
    if (!can_bit_timing(s_pclk1_hz, bitrate, &btr, &actual)) return false;

    clocks_for(c->cfg);
    can->IER &= ~(CAN_IER_FMPIE0 | CAN_IER_FMPIE1);
    if (!can_enter_init(can)) { c->can_error_count++; return false; }

    can->MCR |= CAN_MCR_ABOM;                       /* auto bus-off recovery */
    can->BTR = btr | mode_btr_bits(mode);

    while (can->RF0R & CAN_RF0R_FMP0) can->RF0R = CAN_RF0R_RFOM0;
    while (can->RF1R & CAN_RF1R_FMP1) can->RF1R = CAN_RF1R_RFOM1;

    c->current_bitrate      = actual;
    c->mode                 = mode;
    transceiver_follow_mode(c->cfg, mode);
    c->valid_frames_at_rate = 0u;
    c->bitrate_error_streak = 0u;
    c->isr_last_rx_ts_ms    = 0u;
    c->isr_min_interval_ms  = 0u;

    if (!can_leave_init(can)) { c->can_error_count++; return false; }

    can->ESR = 0u;
    can->IER |= CAN_IER_FMPIE0 | CAN_IER_FMPIE1 | CAN_IER_FOVIE0 | CAN_IER_FOVIE1 |
                CAN_IER_ERRIE | CAN_IER_LECIE | CAN_IER_BOFIE | CAN_IER_EPVIE;
    nvic_enable_for(c->cfg);
    return true;
}

/* ── filters (shared bank array in CAN1; CAN2SB splits the 28 banks) ───── */
static void apply_filter(const can_filter_regs_t *r)
{
    const uint32_t bit = 1u << r->bank;
    CAN1->FA1R &= ~bit;                             /* deactivate while editing */
    if (r->mask_mode) CAN1->FM1R &= ~bit; else CAN1->FM1R |= bit;
    if (r->scale32)   CAN1->FS1R |= bit;  else CAN1->FS1R &= ~bit;
    if (r->fifo1)     CAN1->FFA1R |= bit; else CAN1->FFA1R &= ~bit;
    CAN1->sFilterRegister[r->bank].FR1 = r->fr1;
    CAN1->sFilterRegister[r->bank].FR2 = r->fr2;
    if (r->active) CAN1->FA1R |= bit;
}

void can_hw_filters_init(can_ctrl_t *can1, can_ctrl_t *can2, bool use_fifo1)
{
    (void)use_fifo1;
    /* Filters are configured at boot while both bxCAN instances are held in
     * listen-only initialization and both TJA1051 S pins are HIGH. */
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN | RCC_APB1ENR_CAN2EN;
    (void)RCC->APB1ENR;
    const uint8_t can2_start = 14u;                 /* banks 0..13 CAN1, 14..27 CAN2 */
    CAN1->FMR |= CAN_FMR_FINIT;
    CAN1->FMR = (CAN1->FMR & ~CAN_FMR_CAN2SB) | ((uint32_t)can2_start << 8);

    can_filter_regs_t r;
    can_filter_spec_t s1 = can_filter_accept_all(can1->cfg->filter_bank, false);
    if (can_filter_compile(&s1, &r)) apply_filter(&r);
    if (can2 && can2->cfg) {
        can_filter_spec_t s2 = can_filter_accept_all(can2->cfg->filter_bank, false);
        if (can_filter_compile(&s2, &r)) apply_filter(&r);
    }
    CAN1->FMR &= ~CAN_FMR_FINIT;
}

/* ── passive scan / lock ─────────────────────────────────────────────── */
void can_hw_start_passive(can_ctrl_t *c, uint32_t now_ms)
{
    if (!c || !c->cfg || c->tx_armed) return;
    c->scan_active = true;
    c->can_locked = 0u;
    c->scan_index = 0u;
    /* Sticky lock first; otherwise start at this bus's schematic nominal rate. */
    const uint32_t first_rate = c->last_good_bitrate != 0u
                              ? c->last_good_bitrate : c->cfg->preferred_bitrate;
    for (uint32_t i = 0u; i < BITRATE_CANDIDATE_COUNT; i++)
        if (s_bitrate_candidates[i] == first_rate) { c->scan_index = i; break; }
    if (!can_hw_set_bitrate(c, s_bitrate_candidates[c->scan_index], CAN_MODE_SILENT)) {
        c->scan_active = false;
        return;
    }
    c->scan_started_ms = now_ms;
}

void can_hw_stop_passive(can_ctrl_t *c)
{
    if (!c || !c->cfg) return;
    transceiver_follow_mode(c->cfg, CAN_MODE_SILENT);
    can_hw_abort_mailboxes(c);
    CAN_TypeDef *can = c->cfg->can;
    if (c->scan_active && can_enter_init(can)) {
        can->IER = 0u;
        /* Leave init first, then request the peripheral sleep state. */
        if (can_leave_init(can)) {
            can->MCR |= CAN_MCR_SLEEP;
            uint32_t timeout = 100000u;
            while (!(can->MSR & CAN_MSR_SLAK) && --timeout) { }
        }
    }
    c->scan_active = false;
    c->can_locked = 0u;
    c->valid_frames_at_rate = 0u;
    c->reacquire_requested = 0u;
    c->current_bitrate = 0u;
}

uint32_t can_hw_compute_dwell_ms(const can_ctrl_t *c)
{
    const uint32_t mi = c->isr_min_interval_ms;
    if (mi == 0u)                       return CAN_DWELL_SLOW_MS;
    if (mi <= CAN_DWELL_FAST_THRESH_MS) return CAN_DWELL_FAST_MS;
    if (mi <= CAN_DWELL_MED_THRESH_MS)  return CAN_DWELL_MEDIUM_MS;
    return CAN_DWELL_SLOW_MS;
}

can_scan_ev_t can_hw_scan_step(can_ctrl_t *c, uint32_t now_ms)
{
    if (!c->scan_active || c->can_locked) return CAN_SCAN_NONE;
    if (c->valid_frames_at_rate >= CAN_LOCK_FRAME_COUNT) {
        c->can_locked = 1u;
        c->last_good_bitrate = c->current_bitrate;
        return CAN_SCAN_LOCKED;
    }
    if (c->valid_frames_at_rate == 1u &&
        (uint32_t)(now_ms - c->scan_started_ms) < c->current_dwell_ms * 3u)
        return CAN_SCAN_NONE;
    if ((uint32_t)(now_ms - c->scan_started_ms) < c->current_dwell_ms)
        return CAN_SCAN_NONE;

    for (uint32_t tries = 0u; tries < BITRATE_CANDIDATE_COUNT; tries++) {
        c->scan_index = (c->scan_index + 1u) % BITRATE_CANDIDATE_COUNT;
        if (can_hw_set_bitrate(c, s_bitrate_candidates[c->scan_index], CAN_MODE_SILENT)) {
            c->scan_started_ms = now_ms;
            return CAN_SCAN_STEPPED;
        }
    }
    c->scan_started_ms = now_ms;
    return CAN_SCAN_NONE;
}

/* ── RX drain (interrupt context) ────────────────────────────────────── */
void can_hw_rx_drain(can_ctrl_t *c, can_capture_queue_t *q, uint8_t fifo, uint32_t now_ms)
{
    CAN_TypeDef *can = c->cfg->can;
    volatile uint32_t *rfr = (fifo == 0u) ? &can->RF0R : &can->RF1R;
    const uint32_t fmp = (fifo == 0u) ? CAN_RF0R_FMP0 : CAN_RF1R_FMP1;
    const uint32_t rfom = (fifo == 0u) ? CAN_RF0R_RFOM0 : CAN_RF1R_RFOM1;
    const uint32_t fovr = (fifo == 0u) ? CAN_RF0R_FOVR0 : CAN_RF1R_FOVR1;

    while (*rfr & fmp) {
        const uint32_t rir  = can->sFIFOMailBox[fifo].RIR;
        const uint32_t rdtr = can->sFIFOMailBox[fifo].RDTR;
        const uint32_t low  = can->sFIFOMailBox[fifo].RDLR;
        const uint32_t high = can->sFIFOMailBox[fifo].RDHR;

        can_capture_frame_t f = {0};
        f.timestamp_ms = now_ms;
        f.timestamp_us = TIM2->CNT;
        f.bus      = c->cfg->bus_index;
        f.extended = (rir & CAN_RI0R_IDE) ? 1u : 0u;
        f.remote   = (rir & CAN_RI0R_RTR) ? 1u : 0u;
        f.identifier = f.extended ? ((rir >> 3) & 0x1FFFFFFFu) : ((rir >> 21) & 0x7FFu);
        f.dlc = (uint8_t)(rdtr & 0x0Fu); if (f.dlc > 8u) f.dlc = 8u;
        if (!f.remote) {
            for (uint32_t i = 0; i < f.dlc && i < 4u; i++) f.data[i] = (uint8_t)(low >> (i*8u));
            for (uint32_t i = 4; i < f.dlc && i < 8u; i++) f.data[i] = (uint8_t)(high >> ((i-4u)*8u));
        }
        /* fastest inter-frame interval drives the adaptive scan dwell */
        if (c->isr_last_rx_ts_ms != 0u) {
            const uint32_t intv = now_ms - c->isr_last_rx_ts_ms;
            if (intv > 0u && (c->isr_min_interval_ms == 0u || intv < c->isr_min_interval_ms))
                c->isr_min_interval_ms = intv;
        }
        c->isr_last_rx_ts_ms = now_ms;

        can_hw_seen_mark(c, &f);
        c->rx_frame_count++;
        c->valid_frames_at_rate++;
        c->bitrate_error_streak = 0u;
        (void)can_capture_queue_push_isr(q, &f);
        *rfr = rfom;
    }
    if (*rfr & fovr) { c->can_error_count++; *rfr = fovr; }
}

void can_hw_sce(can_ctrl_t *c)
{
    CAN_TypeDef *can = c->cfg->can;
    const uint32_t esr = can->ESR;
    if (esr & (CAN_ESR_BOFF | CAN_ESR_EPVF | CAN_ESR_EWGF)) c->can_error_count++;
    if (esr & CAN_ESR_LEC) {
        c->can_lec_count++;
        c->bitrate_error_streak++;
        if (c->bitrate_error_streak >= CAN_REACQUIRE_ERROR_MAX && c->can_locked)
            c->reacquire_requested = 1u;
        can->ESR = 0u;
    }
    if (esr & CAN_ESR_BOFF) {
        c->busoff_seen = 1u;
        if (c->can_locked) c->reacquire_requested = 1u;
    }
    can->MSR = CAN_MSR_ERRI;
}

/* ── seen-in-traffic bitmap ───────────────────────────────────────────── */
void can_hw_seen_clear(can_ctrl_t *c)
{
    for (uint32_t i = 0u; i < 2048u/32u; i++) c->seen_std[i] = 0u;
    for (uint32_t i = 0u; i < 256u/32u; i++)  c->seen_ext_ta[i] = 0u;
}

void can_hw_seen_mark(can_ctrl_t *c, const can_capture_frame_t *f)
{
    if (!f->extended) {
        c->seen_std[(f->identifier & 0x7FFu) >> 5] |= 1u << (f->identifier & 31u);
    } else if ((f->identifier & 0x1FFF00FFu) == (0x18DA0000u | ECU_SCAN_TESTER_ADDR)) {
        const uint32_t ta = (f->identifier >> 8) & 0xFFu;
        c->seen_ext_ta[ta >> 5] |= 1u << (ta & 31u);
    }
}

bool can_hw_seen_in_traffic(can_ctrl_t *c, uint32_t id, bool ext)
{
    if (!ext) return (c->seen_std[(id & 0x7FFu) >> 5] >> (id & 31u)) & 1u;
    if ((id & 0x1FFF00FFu) != (0x18DA0000u | ECU_SCAN_TESTER_ADDR)) return false;
    const uint32_t ta = (id >> 8) & 0xFFu;
    return (c->seen_ext_ta[ta >> 5] >> (ta & 31u)) & 1u;
}

/* ── transmit / arm ───────────────────────────────────────────────────── */
bool can_hw_set_controller_mode(can_ctrl_t *c, can_mode_t mode)
{
    if (!c || !c->cfg) return false;
    if (mode == CAN_MODE_NORMAL &&
        (!c->tx_armed || !c->can_locked ||
         !can_tx_bitrate_allowed(c->current_bitrate))) return false;
    CAN_TypeDef *can = c->cfg->can;
    if (!can_enter_init(can)) return false;
    if (mode != CAN_MODE_NORMAL) transceiver_follow_mode(c->cfg, mode);  /* silence first */
    can->BTR = (can->BTR & ~(CAN_BTR_SILM | CAN_BTR_LBKM)) | mode_btr_bits(mode);
    c->mode = mode;
    if (mode == CAN_MODE_NORMAL) transceiver_follow_mode(c->cfg, mode);
    return can_leave_init(can);
}

void can_hw_abort_mailboxes(can_ctrl_t *c)
{
    CAN_TypeDef *can = c->cfg->can;
    const uint32_t all = CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2;
    can->TSR = CAN_TSR_ABRQ0 | CAN_TSR_ABRQ1 | CAN_TSR_ABRQ2;
    uint32_t to = 1000000u;
    while ((can->TSR & all) != all && --to) { }
    if (!to) c->can_error_count++;
}

const char *can_hw_tx_arm(can_ctrl_t *c, uint32_t seconds, uint32_t now_ms, bool clock_ok)
{
    if (!c || !c->cfg)                         return "BAD_CONTROLLER";
    if (seconds == 0u || seconds > TX_ARM_MAX_SECONDS) return "BAD_ARM_DURATION";
    if (!clock_ok)                              return "CLOCK_FALLBACK";
    if (!c->can_locked)                         return "NO_BITRATE_LOCK";
    if (!can_tx_bitrate_allowed(c->current_bitrate)) return "BITRATE_BELOW_TRANSCEIVER_MIN";
    c->tx_armed = true; /* set before NORMAL mode guard checks the ARM gate */
    if (c->mode != CAN_MODE_NORMAL &&
        !can_hw_set_controller_mode(c, CAN_MODE_NORMAL)) {
        c->tx_armed = false;
        c->tx_armed_until_ms = 0u;
        (void)can_hw_set_controller_mode(c, CAN_MODE_SILENT);
        return "CAN_MODE_SWITCH_FAILED";
    }
    c->tx_armed_until_ms = now_ms + seconds * 1000u;
    if (c->tx_armed_until_ms == 0u) c->tx_armed_until_ms = 1u;
    return NULL;
}

void can_hw_tx_disarm_hw(can_ctrl_t *c)
{
    c->tx_armed = false;
    c->tx_armed_until_ms = 0u;
    can_hw_abort_mailboxes(c);
    if (!can_hw_set_controller_mode(c, CAN_MODE_SILENT)) c->can_error_count++;
}

bool can_hw_tx_window_open(const can_ctrl_t *c, uint32_t now_ms)
{
    if (!c || !c->cfg || !c->tx_armed || c->busoff_seen ||
        c->mode != CAN_MODE_NORMAL ||
        !can_tx_bitrate_allowed(c->current_bitrate)) return false;
    return (int32_t)(now_ms - c->tx_armed_until_ms) < 0;
}

bool can_hw_rate_ok(can_ctrl_t *c, uint32_t now_ms)
{
    const uint32_t el = now_ms - c->tx_tokens_ms;
    if (el >= 1000u / TX_RATE_MAX_PER_SECOND) {
        const uint32_t add = el / (1000u / TX_RATE_MAX_PER_SECOND);
        c->tx_tokens = (c->tx_tokens + add > TX_RATE_MAX_PER_SECOND)
                       ? TX_RATE_MAX_PER_SECOND : c->tx_tokens + add;
        c->tx_tokens_ms = now_ms;
    }
    return c->tx_tokens > 0u;
}

bool can_hw_load_mailbox(can_ctrl_t *c, uint32_t id, bool ext, bool rtr,
                         const uint8_t *data, uint8_t dlc, bool quiet)
{
    if (!c || !c->cfg || !c->tx_armed || c->mode != CAN_MODE_NORMAL ||
        !can_tx_bitrate_allowed(c->current_bitrate) || dlc > 8u ||
        (!rtr && dlc != 0u && !data)) return false;
    CAN_TypeDef *can = c->cfg->can;
    const uint32_t tsr = can->TSR;
    uint32_t mb;
    if      ((tsr & CAN_TSR_TME0) && !c->tx_mailbox_busy[0]) mb = 0u;
    else if ((tsr & CAN_TSR_TME1) && !c->tx_mailbox_busy[1]) mb = 1u;
    else if ((tsr & CAN_TSR_TME2) && !c->tx_mailbox_busy[2]) mb = 2u;
    else return false;
    if (c->tx_tokens == 0u) return false;
    uint8_t d[8] = {0};
    if (!rtr) memcpy(d, data, dlc);
    c->tx_tokens--;
    can->sTxMailBox[mb].TDTR = dlc;
    can->sTxMailBox[mb].TDLR = (uint32_t)d[0] | ((uint32_t)d[1]<<8) |
                               ((uint32_t)d[2]<<16) | ((uint32_t)d[3]<<24);
    can->sTxMailBox[mb].TDHR = (uint32_t)d[4] | ((uint32_t)d[5]<<8) |
                               ((uint32_t)d[6]<<16) | ((uint32_t)d[7]<<24);
    uint32_t tir = ext ? ((id << 3) | CAN_TI0R_IDE) : (id << 21);
    if (rtr) tir |= CAN_TI0R_RTR;
    c->tx_mailbox_id[mb] = id;
    c->tx_mailbox_busy[mb] = true;
    c->tx_mailbox_quiet[mb] = quiet;
    can->sTxMailBox[mb].TIR = tir | CAN_TI0R_TXRQ;
    return true;
}

bool can_hw_tx_service(can_ctrl_t *c, uint32_t now_ms,
                       void (*report)(can_ctrl_t *, uint32_t id, bool quiet,
                                      const char *result),
                       const char **disarm_reason)
{
    CAN_TypeDef *can = c->cfg->can;
    bool auto_disarmed = false;
    *disarm_reason = NULL;

    (void)can_hw_rate_ok(c, now_ms);                /* refill tokens */

    if (c->tx_armed && (int32_t)(now_ms - c->tx_armed_until_ms) >= 0) {
        *disarm_reason = "TIMEOUT"; auto_disarmed = true;
    }
    if (c->busoff_seen) { c->busoff_seen = 0u; *disarm_reason = "BUS_OFF"; auto_disarmed = true; }

    static const uint32_t rqcp[3] = { CAN_TSR_RQCP0, CAN_TSR_RQCP1, CAN_TSR_RQCP2 };
    static const uint32_t txok[3] = { CAN_TSR_TXOK0, CAN_TSR_TXOK1, CAN_TSR_TXOK2 };
    static const uint32_t alst[3] = { CAN_TSR_ALST0, CAN_TSR_ALST1, CAN_TSR_ALST2 };
    static const uint32_t terr[3] = { CAN_TSR_TERR0, CAN_TSR_TERR1, CAN_TSR_TERR2 };
    const uint32_t tsr = can->TSR;
    for (uint32_t mb = 0u; mb < 3u; mb++) {
        if (!c->tx_mailbox_busy[mb] || !(tsr & rqcp[mb])) continue;
        const char *res = (tsr & txok[mb]) ? "OK" :
                          (tsr & alst[mb]) ? "ARB_LOST" :
                          (tsr & terr[mb]) ? "TX_ERROR" : "ABORTED";
        if (report) report(c, c->tx_mailbox_id[mb], c->tx_mailbox_quiet[mb], res);
        can->TSR = rqcp[mb];
        c->tx_mailbox_busy[mb] = false;
    }
    return auto_disarmed;
}
