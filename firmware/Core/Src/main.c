#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f407xx.h"
#include "can_frame_queue.h"
#include "can_frame_format.h"
#include "can_signal_profiler.h"
#include "obd2_decoder.h"
#include "uart_dma.h"
#include "periph_timing.h"
#include "uds_client.h"
#include "ecu_scan.h"

/*
 * Project Civic: STM32F407VGT6 (STM32F4DISCOVERY) CAN capture + controlled TX.
 *
 * Transceiver: NXP TJA1051 (plain part, pin 5 n.c.).
 *   PD0 = CAN1_RX  <- TJA1051 pin 4 RXD   (AF9)
 *   PD1 = CAN1_TX  -> TJA1051 pin 1 TXD   (AF9, open-drain)
 *   PD2 = GPIO     -> TJA1051 pin 8 S     (open-drain; released = S pulled
 *                     HIGH externally = transceiver SILENT, LOW only while armed)
 *   TJA1051 pin 7 CANH / pin 6 CANL -> vehicle DLC pins 6 / 14 (twisted pair).
 *   Pins: CAN1 on PD0/PD1 AF9 as in ST STM32CubeF4 CAN_Networking (STM324xG_EVAL)
 *   and ST STM32_open_pin_data (GPIO_AF9_CAN1). PD0/PD1/PD2 are "Free I/O" on
 *   the Discovery (UM1472 Rev 9 Table 7); PB9 is NOT (audio DAC SDA).
 *   TJA1051: TXD/S VIH = 0.7*VIO (VIO = VCC on the plain part), VCC 4.5..5.5 V;
 *   TXD has an internal pull-up, S an internal pull-down (NXP TJA1051 product
 *   data sheet Rev. 02, docs/TJA1051.PDF: Section 6.2.2, Table 6). The
 *   3 V Discovery cannot drive 0.7*VCC, so TXD and S are open-drain here and
 *   are pulled up to 5 V by the external interface circuit (docs/master-plan.md).
 *   PD0/PD1/PD2 are FT (5 V tolerant) I/Os (DS8626 Rev 12 Table 7, LQFP100
 *   pins 81/82/83); an FT pin may sit above VDD+0.3 V only with its internal
 *   pull-up/pull-down disabled (DS8626 Table 48 note 5), so PUPDR is 00 on all
 *   three. RXD (up to VCC = 5 V) goes straight to PD0.
 *
 * Safety model for transmit (build flag PROJECT_CAN_TX_CONFIRMED=1 required):
 *   - Boots and stays in bxCAN silent mode (BTR.SILM): never ACKs, never sends.
 *   - "ARM,<seconds>" (1..600) switches to Normal mode for that window only.
 *   - "DISARM", window expiry, re-acquisition or bus-off -> back to silent.
 *   - TX only while armed, with strict field validation and a rate limit.
 *   - Authentication of the human operator is done by the ESP32 dashboard.
 *
 * Wire protocol to ESP32 (USART2, PROJECT_HOST_UART_BAUD 8N1, PA2=TX, PA3=RX,
 *   AF7; TX by DMA1 Stream6 Channel4, as ST's STM32F4-Discovery
 *   UART_TwoBoards_ComDMA example. PA9/PA10 are the board's USB VBUS/ID lines,
 *   UM1472 Rev 9 Section 7.5 / Table 7, so USART1 is not used):
 *   STATUS,SCAN,<hz> | STATUS,LOCK,<hz> | STATUS,REACQUIRE | STATUS,DWELL,<ms>
 *   STATUS,ARMED,<seconds_left> | STATUS,DISARMED,<reason>
 *   STATUS,RESET,<IWDG|WWDG|LOWPOWER|SOFTWARE|BROWNOUT_OR_POWER_ON|PIN|UNKNOWN>
 *   STATUS,WATCHDOG,FAILED  (IWDG registers did not update)
 *   STATUS,STATS,<rx>,<queue_drop>,<uart_line_drop>,<can_err>,<lec_err>,<tec>,<rec>,
 *       <ACTIVE|WARNING|PASSIVE|BUS_OFF>   (CAN_ESR error counters/state)
 *   <ts_us>,STD/EXT,<id_hex>,<dlc>,<rtr>,<payload_hex>      (no "0x")
 *   PID,... / PROFILE,... / SIG,...
 *   TX,QUEUED,<id> | TX,OK,<id> | TX,ERR,<reason>[,<id>]
 *   UDS,DATA,<rx_id>,<offset>,<hex>  (response bytes, chunks of 32)
 *   UDS,POS,<rx_id>,<length> | UDS,NEG,<rx_id>,<sid>,<nrc> | UDS,PENDING,<rx_id>
 *   UDS,SENT,<rx_id> (response suppressed) | UDS,ERR,<reason>
 *   UDS,TXERR,<id>,<ARB_LOST|TX_ERROR|ABORTED>  (ISO-TP frame not sent)
 * Commands from ESP32 (one per line, max HOST_LINE_CAPACITY-1 chars):
 *   ARM,<seconds> | DISARM | TX,<S|E>,<id_hex>,<rtr 0|1>,<dlc>,<hex|->
 *       ("-" = no payload: remote frame or data frame with DLC 0)
 *   UDS,<S|E>,<tx_id_hex>,<rx_id_hex>,<request_hex>[,CONFIRM]   (armed only;
 *       CONFIRM is required for every non-read service, see uds_client.c)
 *   UDS,ABORT | UDSPAD,<hex byte|NONE>   (ISO-TP padding, default 00)
 *   ECUSCAN,<S|E>,<first_hex>,<last_hex>,<request_hex>[,<wait_ms>]  (armed;
 *       read-only single-frame request; S: request IDs, response = +8;
 *       E: 29-bit target addresses 00..FF, 0x18DA<TA>F1 / 0x18DAF1<TA>)
 *   ECUSCAN,ABORT
 * ECU scan output:
 *   ECU,FOUND,<tx_id>,<rx_id>,<first reply frame hex> | ECU,DONE,<count>
 *   ECU,SKIP,<tx_id>,IN_USE  (request ID already seen in bus traffic: never sent)
 *   ECU,ERR,<BAD_FORMAT|NOT_ARMED|BUSY|NOT_READ_ONLY|ABORTED|DISARMED>
 */

/* ── build-time gates ─────────────────────────────────────────────── */
#ifndef PROJECT_TRANSCEIVER_SILENT_CONFIRMED
#define PROJECT_TRANSCEIVER_SILENT_CONFIRMED 0u
#endif
#ifndef PROJECT_CAN_TX_CONFIRMED
#define PROJECT_CAN_TX_CONFIRMED 0u
#endif
#ifndef PROJECT_HOST_UART_BAUD
#define PROJECT_HOST_UART_BAUD 2000000u
#endif
#if PROJECT_HOST_UART_BAUD > 5000000u
#error "ESP32 UART is specified up to 5 MBaud (ESP32 Series Datasheet v5.3 Section 4.8.3)"
#endif

/* ── tuning ───────────────────────────────────────────────────────── */
#define CAN_DWELL_FAST_MS         200u
#define CAN_DWELL_MEDIUM_MS       500u
#define CAN_DWELL_SLOW_MS        1000u
#define CAN_DWELL_FAST_THRESH_MS   20u
#define CAN_DWELL_MED_THRESH_MS   100u
#define CAN_LOCK_FRAME_COUNT        2u
#define CAN_REACQUIRE_ERROR_MAX     8u
#define HOST_RX_CAPACITY          256u
#define HOST_LINE_CAPACITY        192u
#define TX_ARM_MAX_SECONDS        600u
#define TX_RATE_MAX_PER_SECOND    200u
#define STATS_PERIOD_MS          1000u

/* Independent watchdog (IWDG), clocked by LSI. Sequence follows ST's
 * HAL_IWDG_Init (stm32f4xx-hal-driver, Src/stm32f4xx_hal_iwdg.c): KR=0xCCCC,
 * KR=0x5555, PR, RLR, wait SR=0, KR=0xAAAA. Prescaler /64 (PR=4), reload 1000.
 * Timeout = 64*1000/f_LSI: 2.0 s at the typical 32 kHz; f_LSI is 17..47 kHz
 * (DS8626 Rev 12 Table 35), so 1.36..3.76 s. The main loop's longest busy
 * wait (bxCAN mode or mailbox-abort loops, 1e6 iterations) is far shorter. */
#define IWDG_PR_DIV64               4u
#define IWDG_RELOAD              1000u
#define IWDG_SR_WAIT_MS            50u

/* ── state ─────────────────────────────────────────────────────────── */
static can_capture_queue_t capture_queue;
static can_profiler_t      profiler;
static volatile uint32_t   can_error_count;
static volatile uint32_t   can_lec_count;
static volatile uint32_t   rx_frame_count;
static volatile uint32_t   system_millis;
static volatile uint32_t   valid_frames_at_rate;
static volatile uint32_t   bitrate_error_streak;
static volatile uint32_t   reacquire_requested;
static volatile uint32_t   busoff_seen;
static volatile uint32_t   can_locked;
static volatile uint32_t   current_bitrate;
static volatile uint32_t   scan_started_ms;
static volatile uint32_t   scan_index;
static volatile uint32_t   host_rx_overflow;
static volatile uint16_t   host_rx_head, host_rx_tail;
static volatile uint8_t    host_rx_ring[HOST_RX_CAPACITY];
static uint32_t            pclk1_hz, pclk2_hz, tim2_clk_hz;
static bool                clock_ok;        /* false = HSE/PLL failed, running on HSI */

static uint32_t current_dwell_ms = CAN_DWELL_SLOW_MS;
static volatile uint32_t isr_min_interval_ms;
static volatile uint32_t isr_last_rx_ts_ms;

/* Candidate list for the passive bitrate scan. The vehicle's real bitrates
 * are NOT taken from any document (Honda publishes none); they are found by
 * this scan, which runs in silent mode and cannot disturb the bus. */
static const uint32_t bitrate_candidates[] = {
    500000u, 1000000u, 250000u, 125000u, 100000u,
    83333u,  50000u,   33333u,  20000u,  10000u
};
#define BITRATE_CANDIDATE_COUNT \
    ((uint32_t)(sizeof(bitrate_candidates)/sizeof(bitrate_candidates[0])))

/* ══════════════════════════════════════════════════════════════════════
 * Clock: HSE 8 MHz (Discovery, UM1472 Section 7.11.1) -> PLL 168 MHz.
 * Requires HSE_VALUE=8000000 (set in Makefile) so SystemCoreClockUpdate()
 * reports the true frequency. Values match ST's STM32CubeF4 Discovery
 * examples (PLLM 8, PLLN 336, PLLP 2, PLLQ 7, 5 wait states).
 * ══════════════════════════════════════════════════════════════════════ */
static bool clock_init_168mhz(void)
{
    RCC->CR |= RCC_CR_HSEON;
    uint32_t to = 1000000u;
    while (!(RCC->CR & RCC_CR_HSERDY) && --to) { }
    if (!to) return false;

    FLASH->ACR = FLASH_ACR_LATENCY_5WS | FLASH_ACR_PRFTEN |
                 FLASH_ACR_ICEN | FLASH_ACR_DCEN;
    /* RM0090 3.5.1 "Increasing the CPU frequency" step 2: check LATENCY */
    to = 1000u;
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_5WS && --to) { }
    if (!to) return false;

    /* Reserved bits 31:28, 23, 21:18, 15 kept at reset value (RM0090 7.3.2) */
    RCC->PLLCFGR = (RCC->PLLCFGR & 0xF0BC8000u)
                 | (8u   << RCC_PLLCFGR_PLLM_Pos)
                 | (336u << RCC_PLLCFGR_PLLN_Pos)
                 | (0u   << RCC_PLLCFGR_PLLP_Pos)     /* 00 = /2 */
                 | RCC_PLLCFGR_PLLSRC_HSE
                 | (7u   << RCC_PLLCFGR_PLLQ_Pos);

    RCC->CR |= RCC_CR_PLLON;
    to = 1000000u;
    while (!(RCC->CR & RCC_CR_PLLRDY) && --to) { }
    if (!to) return false;

    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE2 | RCC_CFGR_PPRE1))
              | RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_PPRE1_DIV4;

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    to = 1000000u;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL && --to) { }
    if (!to) {
        /* Still on HSI: undo the APB prescalers so PCLK1 = 16 MHz and the
         * 2 Mbaud host UART stays reachable (RM0090 7.3.3, 30.3.4 Eq. 1). */
        RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
        return false;
    }
    return true;
}

/* APB prescaler field (3 bits): 0xx = /1, 100 = /2, 101 = /4, 110 = /8, 111 = /16 */
static uint32_t apb_divider(uint32_t field)
{
    return (field & 4u) ? (2u << (field & 3u)) : 1u;
}

/* Derive bus clocks from the real RCC configuration, so every peripheral
 * divider below is correct whether the PLL started or the chip stayed on
 * HSI 16 MHz. AHB prescaler is never changed by this firmware (/1). */
static void bus_clocks_update(void)
{
    SystemCoreClockUpdate();
    const uint32_t cfgr = RCC->CFGR;
    const uint32_t d1 = apb_divider((cfgr & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos);
    const uint32_t d2 = apb_divider((cfgr & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos);
    pclk1_hz   = SystemCoreClock / d1;
    pclk2_hz   = SystemCoreClock / d2;
    /* Timer clock = PCLK1 if APB1 prescaler is 1, else 2 x PCLK1 */
    tim2_clk_hz = (d1 == 1u) ? pclk1_hz : (2u * pclk1_hz);
}

/* TIM2: free-running 32-bit counter at 1 MHz -> 1 us timestamps. */
static void tim2_us_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    (void)RCC->APB1ENR;
    TIM2->PSC = (tim2_clk_hz / 1000000u) - 1u;
    TIM2->ARR = 0xFFFFFFFFu;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 = TIM_CR1_CEN;
}

void SysTick_Handler(void) { system_millis++; }

/* ══════════════════════════════════════════════════════════════════════
 * TJA1051 S (pin 8) on PD2, open-drain output. Released (ODR = 1) lets the
 * external 5 V pull-up hold S HIGH: transceiver in Silent mode, transmitter
 * disabled, RXD still active (NXP TJA1051 Rev. 02, Table 3 operating modes). The pin is
 * driven LOW (Normal mode) only inside an ARM window. While the MCU is in
 * reset, being flashed or hung, PD2 is an input and S stays HIGH (silent).
 * ══════════════════════════════════════════════════════════════════════ */
#define S_PIN 2u
static void transceiver_s_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;
    GPIOD->BSRR    = (1u << S_PIN);                  /* ODR = 1 before output */
    GPIOD->OTYPER |= (1u << S_PIN);                  /* open-drain            */
    GPIOD->PUPDR  &= ~(3u << (2u * S_PIN));
    GPIOD->OSPEEDR &= ~(3u << (2u * S_PIN));
    GPIOD->MODER   = (GPIOD->MODER & ~(3u << (2u * S_PIN))) | (1u << (2u * S_PIN));
}

#if PROJECT_TRANSCEIVER_SILENT_CONFIRMED == 1u

/* ══════════════════════════════════════════════════════════════════════
 * USART2 (PA2 TX / PA3 RX, AF7, APB1). BRR formulas follow ST's HAL macros
 * UART_BRR_SAMPLING16 / UART_BRR_SAMPLING8 (stm32f4xx_hal_uart.h, HAL 1.8.5).
 * ══════════════════════════════════════════════════════════════════════ */

static bool host_stream_init(void)
{
    uint32_t brr; bool over8;
    if (!usart_brr_for(pclk1_hz, PROJECT_HOST_UART_BAUD, &brr, &over8)) {
        can_error_count++;
        return false;
    }
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;
    GPIOA->MODER   = (GPIOA->MODER   & ~((3u<<4)|(3u<<6))) | (2u<<4)|(2u<<6);
    GPIOA->OTYPER &= ~((1u<<2)|(1u<<3));
    GPIOA->PUPDR   = (GPIOA->PUPDR   & ~((3u<<4)|(3u<<6))) | (1u<<6);  /* RX pull-up */
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~((3u<<4)|(3u<<6))) | (2u<<4)|(2u<<6);
    GPIOA->AFR[0]  = (GPIOA->AFR[0]  & ~((15u<<8)|(15u<<12))) | (7u<<8)|(7u<<12);
    USART2->CR1 = 0u;
    USART2->BRR = brr;
    USART2->CR2 = 0u;
    USART2->CR3 = 0u;
    USART2->CR1 = (over8 ? USART_CR1_OVER8 : 0u) |
                  USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_UE;
    NVIC_EnableIRQ(USART2_IRQn);
    return true;
}

void USART2_IRQHandler(void)
{
    const uint32_t sr = USART2->SR;
    if (sr & (USART_SR_RXNE | USART_SR_ORE)) {
        const uint8_t ch = (uint8_t)USART2->DR;   /* SR then DR read clears ORE too */
        if (sr & USART_SR_ORE) host_rx_overflow++;
        const uint16_t nxt = (uint16_t)((host_rx_head + 1u) % HOST_RX_CAPACITY);
        if (nxt != host_rx_tail) { host_rx_ring[host_rx_head] = ch; host_rx_head = nxt; }
        else { host_rx_overflow++; }
    }
}

static bool host_rx_pop(uint8_t *out)
{
    if (host_rx_tail == host_rx_head) return false;
    *out = host_rx_ring[host_rx_tail];
    host_rx_tail = (uint16_t)((host_rx_tail + 1u) % HOST_RX_CAPACITY);
    return true;
}

/* ── CAN1 GPIO: PD0 RX, PD1 TX (AF9). TX is open-drain: the bus side
 *    recessive level comes from the 5 V pull-up of the interface circuit. ── */
static void gpio_can1_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;
    GPIOD->PUPDR  &= ~((3u<<0)|(3u<<2));
    GPIOD->OTYPER  = (GPIOD->OTYPER & ~(1u<<0)) | (1u<<1);
    GPIOD->OSPEEDR = (GPIOD->OSPEEDR & ~((3u<<0)|(3u<<2))) | (3u<<0)|(3u<<2);
    GPIOD->AFR[0]  = (GPIOD->AFR[0]  & ~((15u<<0)|(15u<<4))) | (9u<<0)|(9u<<4);
    GPIOD->MODER   = (GPIOD->MODER   & ~((3u<<0)|(3u<<2))) | (2u<<0)|(2u<<2);
}

/* ══════════════════════════════════════════════════════════════════════
 * bxCAN mode helpers. Sequence follows ST HAL_CAN_Init (stm32f4xx_hal_can.c,
 * HAL 1.8.5): set INRQ, wait INAK; clear SLEEP, wait SLAK=0. After reset the
 * controller is in Sleep mode, so SLEEP must be cleared or it never runs.
 * ══════════════════════════════════════════════════════════════════════ */
static bool can1_enter_init(void)
{
    /* RM0090 32.4.3: INRQ set while in Sleep mode must also clear SLEEP */
    CAN1->MCR = (CAN1->MCR & ~CAN_MCR_SLEEP) | CAN_MCR_INRQ;
    uint32_t to = 1000000u;
    while (!(CAN1->MSR & CAN_MSR_INAK) && --to) { }
    if (!to) return false;
    to = 1000000u;
    while ((CAN1->MSR & CAN_MSR_SLAK) && --to) { }
    return to != 0u;
}

static bool can1_leave_init(void)
{
    CAN1->MCR &= ~CAN_MCR_INRQ;
    uint32_t to = 1000000u;
    while ((CAN1->MSR & CAN_MSR_INAK) && --to) { }
    return to != 0u;
}


static bool can1_set_bitrate(uint32_t bitrate, bool silent)
{
    uint32_t btr, actual;
    if (!can_bit_timing(pclk1_hz, bitrate, &btr, &actual)) return false;

    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN; (void)RCC->APB1ENR;
    CAN1->IER &= ~CAN_IER_FMPIE0;                  /* no RX counting while switching */
    if (!can1_enter_init()) { can_error_count++; return false; }

    CAN1->MCR |= CAN_MCR_ABOM;
    CAN1->BTR = btr | (silent ? CAN_BTR_SILM : 0u);

    /* Filter bank 0: 32-bit mask, all-zero mask = accept every STD/EXT frame */
    CAN1->FMR |= CAN_FMR_FINIT;
    CAN1->FA1R &= ~1u; CAN1->FM1R &= ~1u; CAN1->FS1R |= 1u; CAN1->FFA1R &= ~1u;
    CAN1->sFilterRegister[0].FR1 = 0u;
    CAN1->sFilterRegister[0].FR2 = 0u;
    CAN1->FA1R |= 1u;
    CAN1->FMR &= ~CAN_FMR_FINIT;

    /* Drop anything still in FIFO0 from the previous bitrate */
    while (CAN1->RF0R & CAN_RF0R_FMP0) CAN1->RF0R = CAN_RF0R_RFOM0;

    current_bitrate      = actual;
    valid_frames_at_rate = 0u;
    bitrate_error_streak = 0u;
    scan_started_ms      = system_millis;
    isr_last_rx_ts_ms    = 0u;
    isr_min_interval_ms  = 0u;

    if (!can1_leave_init()) { can_error_count++; return false; }

    CAN1->ESR = 0u;                                /* clear LEC (HAL clears it the same way) */
    CAN1->IER |= CAN_IER_FMPIE0 | CAN_IER_FOVIE0 |
                 CAN_IER_ERRIE | CAN_IER_LECIE | CAN_IER_BOFIE | CAN_IER_EPVIE;
    NVIC_EnableIRQ(CAN1_RX0_IRQn);
    NVIC_EnableIRQ(CAN1_SCE_IRQn);
    return true;
}

/* Profiler output may use only the upper half of the UART ring, so a
 * PROFILE/SIG cycle never crowds out live frame lines; the profiler resumes
 * the cycle on the next loop pass (can_signal_profiler.h). */
static bool profiler_write(const char *line)
{
    const uint32_t len = (uint32_t)strlen(line);
    if (uart_dma_free() < UART_TX_RING_SIZE / 2u + len) return false;
    return uart_dma_put_line(line);
}

static void status_line(const char *fmt, unsigned long v)
{
    char s[64];
    (void)snprintf(s, sizeof s, fmt, v);
    (void)uart_dma_put_line(s);
}

static void can1_start_passive(void)
{
    can_locked = 0u; scan_index = 0u;
    if (can1_set_bitrate(bitrate_candidates[0], true))
        status_line("STATUS,SCAN,%lu\r\n", current_bitrate);
}

static uint32_t compute_dwell_ms(void)
{
    const uint32_t mi = isr_min_interval_ms;
    if (mi == 0u)                         return CAN_DWELL_SLOW_MS;
    if (mi <= CAN_DWELL_FAST_THRESH_MS)   return CAN_DWELL_FAST_MS;
    if (mi <= CAN_DWELL_MED_THRESH_MS)    return CAN_DWELL_MEDIUM_MS;
    return CAN_DWELL_SLOW_MS;
}

static void can1_scan_step(void)
{
    if (can_locked) return;
    if (valid_frames_at_rate >= CAN_LOCK_FRAME_COUNT) {
        can_locked = 1u;
        status_line("STATUS,LOCK,%lu\r\n", current_bitrate);
        status_line("STATUS,DWELL,%lu\r\n", current_dwell_ms);
        return;
    }
    if (valid_frames_at_rate == 1u &&
        (uint32_t)(system_millis - scan_started_ms) < current_dwell_ms * 3u) return;
    if ((uint32_t)(system_millis - scan_started_ms) < current_dwell_ms) return;

    for (uint32_t tries = 0u; tries < BITRATE_CANDIDATE_COUNT; tries++) {
        scan_index = (scan_index + 1u) % BITRATE_CANDIDATE_COUNT;
        if (can1_set_bitrate(bitrate_candidates[scan_index], true)) {
            status_line("STATUS,SCAN,%lu\r\n", current_bitrate);
            return;
        }
    }
    scan_started_ms = system_millis;   /* nothing configurable: retry later */
}

/* ── strict number parsing ───────────────────────────────────────── */
#if PROJECT_CAN_TX_CONFIRMED == 1u
static bool parse_u32(const char *s, int base, uint32_t max, uint32_t *out)
{
    if (!s || !*s) return false;
    for (const char *p = s; *p; p++) {
        const char c = *p;
        const bool ok = (c >= '0' && c <= '9') ||
                        (base == 16 && ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')));
        if (!ok) return false;
    }
    if (strlen(s) > (base == 16 ? 8u : 10u)) return false;
    char *end = NULL;
    const unsigned long v = strtoul(s, &end, base);
    if (!end || *end != '\0' || v > max) return false;
    *out = (uint32_t)v;
    return true;
}
#endif

/* ══════════════════════════════════════════════════════════════════════
 * Transmit: armed window, mailbox result reporting, rate limit.
 * TSR bit meanings: CMSIS stm32f407xx.h CAN_TSR_* (RQCPx, TXOKx, ALSTx, TERRx).
 * ══════════════════════════════════════════════════════════════════════ */
#if PROJECT_CAN_TX_CONFIRMED == 1u
static uint32_t tx_armed_until_ms;      /* 0 = disarmed */
static bool     tx_armed;
static uint32_t tx_mailbox_id[3];
static bool     tx_mailbox_busy[3];
static bool     tx_mailbox_quiet[3];    /* ISO-TP frame: report errors only */
static uint32_t tx_tokens = TX_RATE_MAX_PER_SECOND;
static uint32_t tx_tokens_ms;

static void transceiver_set_silent(bool silent)
{
    GPIOD->BSRR = silent ? (1u << S_PIN) : (1u << (S_PIN + 16u));
}

/* Abort every pending TX mailbox (CAN_TSR ABRQx) and wait until all three are
 * empty, so no queued frame survives a DISARM and goes out at the next ARM.
 * The aborted mailboxes are still reported by tx_service (TX,ERR,ABORTED). */
static void can1_abort_mailboxes(void)
{
    const uint32_t all = CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2;
    CAN1->TSR = CAN_TSR_ABRQ0 | CAN_TSR_ABRQ1 | CAN_TSR_ABRQ2;
    uint32_t to = 1000000u;
    while ((CAN1->TSR & all) != all && --to) { }
    if (!to) can_error_count++;
}

static bool can1_set_controller_silent(bool silent)
{
    if (!can1_enter_init()) return false;
    if (silent) CAN1->BTR |= CAN_BTR_SILM; else CAN1->BTR &= ~CAN_BTR_SILM;
    return can1_leave_init();
}

static void uds_stop(const char *reason);
static void ecuscan_stop(const char *reason);

static void tx_disarm(const char *reason)
{
    if (!tx_armed) return;
    transceiver_set_silent(true);                  /* hardware silent first */
    tx_armed = false;
    tx_armed_until_ms = 0u;
    can1_abort_mailboxes();
    uds_stop("DISARMED");
    ecuscan_stop("DISARMED");
    if (!can1_set_controller_silent(true)) can_error_count++;
    char s[48];
    (void)snprintf(s, sizeof s, "STATUS,DISARMED,%s\r\n", reason);
    (void)uart_dma_put_line(s);
}

static void tx_arm(uint32_t seconds)
{
    if (!clock_ok) { (void)uart_dma_put_line("TX,ERR,CLOCK_FALLBACK\r\n"); return; }
    if (!can_locked) { (void)uart_dma_put_line("TX,ERR,NO_BITRATE_LOCK\r\n"); return; }
    if (!can_tx_bitrate_allowed(current_bitrate)) {        /* TJA1051 min 40 kbit/s */
        (void)uart_dma_put_line("TX,ERR,BITRATE_BELOW_TRANSCEIVER_MIN\r\n"); return;
    }
    if (!tx_armed && !can1_set_controller_silent(false)) {
        (void)uart_dma_put_line("TX,ERR,CAN_MODE_SWITCH_FAILED\r\n"); return;
    }
    transceiver_set_silent(false);
    tx_armed = true;
    tx_armed_until_ms = system_millis + seconds * 1000u;
    if (tx_armed_until_ms == 0u) tx_armed_until_ms = 1u;
    status_line("STATUS,ARMED,%lu\r\n", seconds);
}

/* True only while the ARM window is open and no bus-off is pending. Checked
 * at every transmit, so a command handled before tx_service() in the same
 * loop pass cannot send after the window ended or after a bus-off. */
static bool tx_window_open(void)
{
    if (!tx_armed || busoff_seen) return false;
    return (int32_t)(system_millis - tx_armed_until_ms) < 0;
}

static void tx_service(void)
{
    /* rate-limit token refill */
    const uint32_t now = system_millis;
    const uint32_t el = now - tx_tokens_ms;
    if (el >= 1000u / TX_RATE_MAX_PER_SECOND) {
        const uint32_t add = el / (1000u / TX_RATE_MAX_PER_SECOND);
        tx_tokens = (tx_tokens + add > TX_RATE_MAX_PER_SECOND) ? TX_RATE_MAX_PER_SECOND : tx_tokens + add;
        tx_tokens_ms = now;
    }
    if (tx_armed && (int32_t)(now - tx_armed_until_ms) >= 0) tx_disarm("TIMEOUT");
    if (busoff_seen) { busoff_seen = 0u; tx_disarm("BUS_OFF"); }

    static const uint32_t rqcp[3] = { CAN_TSR_RQCP0, CAN_TSR_RQCP1, CAN_TSR_RQCP2 };
    static const uint32_t txok[3] = { CAN_TSR_TXOK0, CAN_TSR_TXOK1, CAN_TSR_TXOK2 };
    static const uint32_t alst[3] = { CAN_TSR_ALST0, CAN_TSR_ALST1, CAN_TSR_ALST2 };
    static const uint32_t terr[3] = { CAN_TSR_TERR0, CAN_TSR_TERR1, CAN_TSR_TERR2 };
    const uint32_t tsr = CAN1->TSR;
    for (uint32_t mb = 0u; mb < 3u; mb++) {
        if (!tx_mailbox_busy[mb] || !(tsr & rqcp[mb])) continue;
        char s[48];
        if (tx_mailbox_quiet[mb]) {
            if (!(tsr & txok[mb])) {
                const char *why = (tsr & alst[mb]) ? "ARB_LOST" :
                                  (tsr & terr[mb]) ? "TX_ERROR" : "ABORTED";
                (void)snprintf(s, sizeof s, "UDS,TXERR,%lX,%s\r\n",
                               (unsigned long)tx_mailbox_id[mb], why);
                (void)uart_dma_put_line(s);
            }
        } else {
            const char *res = (tsr & txok[mb]) ? "TX,OK" :
                              (tsr & alst[mb]) ? "TX,ERR,ARB_LOST" :
                              (tsr & terr[mb]) ? "TX,ERR,TX_ERROR" : "TX,ERR,ABORTED";
            (void)snprintf(s, sizeof s, "%s,%lX\r\n", res, (unsigned long)tx_mailbox_id[mb]);
            (void)uart_dma_put_line(s);
        }
        CAN1->TSR = rqcp[mb];                       /* write 1 clears RQCP/TXOK/ALST/TERR */
        tx_mailbox_busy[mb] = false;
    }
}

static int hex_nibble(char c)
{
    if (c>='0'&&c<='9') return c-'0';
    if (c>='a'&&c<='f') return c-'a'+10;
    if (c>='A'&&c<='F') return c-'A'+10;
    return -1;
}

/* Loads a free TX mailbox; spends one rate-limit token. false = none free. */
static bool can1_load_mailbox(uint32_t id, bool ext, bool rtr,
                              const uint8_t *data, uint8_t dlc, bool quiet)
{
    const uint32_t tsr = CAN1->TSR;
    uint32_t mb;
    /* A mailbox is reused only after tx_service reported its result. */
    if      ((tsr & CAN_TSR_TME0) && !tx_mailbox_busy[0]) mb = 0u;
    else if ((tsr & CAN_TSR_TME1) && !tx_mailbox_busy[1]) mb = 1u;
    else if ((tsr & CAN_TSR_TME2) && !tx_mailbox_busy[2]) mb = 2u;
    else return false;
    uint8_t d[8] = {0};
    if (!rtr) memcpy(d, data, dlc);
    tx_tokens--;
    CAN1->sTxMailBox[mb].TIR  = 0u;
    CAN1->sTxMailBox[mb].TDTR = dlc;
    CAN1->sTxMailBox[mb].TDLR = (uint32_t)d[0] | ((uint32_t)d[1]<<8) |
                                ((uint32_t)d[2]<<16) | ((uint32_t)d[3]<<24);
    CAN1->sTxMailBox[mb].TDHR = (uint32_t)d[4] | ((uint32_t)d[5]<<8) |
                                ((uint32_t)d[6]<<16) | ((uint32_t)d[7]<<24);
    uint32_t tir = ext ? ((id << 3) | CAN_TI0R_IDE) : (id << 21);
    if (rtr) tir |= CAN_TI0R_RTR;
    tx_mailbox_id[mb] = id;
    tx_mailbox_busy[mb] = true;
    tx_mailbox_quiet[mb] = quiet;
    CAN1->sTxMailBox[mb].TIR = tir | CAN_TI0R_TXRQ;
    return true;
}

static void tx_frame(const char *fmt, const char *id_t, const char *rtr_t,
                     const char *dlc_t, const char *hex_t)
{
    if (!tx_window_open()) { (void)uart_dma_put_line("TX,ERR,NOT_ARMED\r\n"); return; }
    const bool ext = strcmp(fmt, "E") == 0;
    uint32_t id, rtr, dlc;
    if (!parse_u32(id_t, 16, ext ? 0x1FFFFFFFu : 0x7FFu, &id) ||
        !parse_u32(rtr_t, 10, 1u, &rtr) || !parse_u32(dlc_t, 10, 8u, &dlc) ||
        ((rtr || dlc == 0u) ? strcmp(hex_t, "-") != 0 : strlen(hex_t) != dlc * 2u)) {
        (void)uart_dma_put_line("TX,ERR,BAD_FIELDS\r\n"); return;
    }
    uint8_t data[8] = {0};
    for (uint32_t i = 0; i < dlc && !rtr; i++) {
        const int hi = hex_nibble(hex_t[i*2]), lo = hex_nibble(hex_t[i*2+1]);
        if (hi < 0 || lo < 0) { (void)uart_dma_put_line("TX,ERR,BAD_HEX\r\n"); return; }
        data[i] = (uint8_t)((hi << 4) | lo);
    }
    if (tx_tokens == 0u) { (void)uart_dma_put_line("TX,ERR,RATE_LIMIT\r\n"); return; }
    if (!can1_load_mailbox(id, ext, rtr != 0u, data, (uint8_t)dlc, false)) {
        (void)uart_dma_put_line("TX,ERR,MAILBOX_BUSY\r\n"); return;
    }
    char s[32];
    (void)snprintf(s, sizeof s, "TX,QUEUED,%lX\r\n", (unsigned long)id);
    (void)uart_dma_put_line(s);
}
/* ══════════════════════════════════════════════════════════════════════
 * Diagnostics: UDS / OBD-II requests over ISO-TP (own code, armed only).
 * ══════════════════════════════════════════════════════════════════════ */
#define UDS_MAX_REQUEST   64u
#define UDS_CHUNK_BYTES   32u

static uds_client_t uds;
static bool     uds_ready;              /* uds_init done at least once   */
static bool     uds_pad = true;
static uint8_t  uds_pad_byte = 0x00u;
static uint16_t uds_emit_pos;           /* positive response being sent */
static bool     uds_emitting;

static bool uds_can_send(void *ctx, uint32_t id, bool ext, const uint8_t *d, uint8_t len)
{
    (void)ctx;
    if (!tx_window_open() || tx_tokens == 0u) return false;
    return can1_load_mailbox(id, ext, false, d, len, true);
}

static void uds_line(const char *s) { (void)uart_dma_put_line(s); }

/* ── ECU discovery (ecu_scan.c), shares the armed send path ── */
static ecu_scan_t ecuscan;

/* IDs seen in received bus traffic since the last bitrate lock. The scan never
 * sends on one of them: a request on an ID another node already uses would be
 * a spoofed frame (and a collision when both send at once). 11-bit: all 2048
 * IDs; 29-bit: the 0x18DA<TA>F1 request pattern the scan uses, per TA. */
static volatile uint32_t seen_std[2048u / 32u];     /* set in CAN1_RX0_IRQHandler */
static volatile uint32_t seen_ext_ta[256u / 32u];

static void seen_clear(void)
{
    for (uint32_t i = 0u; i < 2048u / 32u; i++) seen_std[i] = 0u;
    for (uint32_t i = 0u; i < 256u / 32u; i++)  seen_ext_ta[i] = 0u;
}

static void seen_mark(const can_capture_frame_t *f)
{
    if (!f->extended) {
        seen_std[(f->identifier & 0x7FFu) >> 5] |= 1u << (f->identifier & 31u);
    } else if ((f->identifier & 0x1FFF00FFu) == (0x18DA0000u | ECU_SCAN_TESTER_ADDR)) {
        const uint32_t ta = (f->identifier >> 8) & 0xFFu;
        seen_ext_ta[ta >> 5] |= 1u << (ta & 31u);
    }
}

static bool seen_in_traffic(void *ctx, uint32_t id, bool ext)
{
    (void)ctx;
    if (!ext) return (seen_std[(id & 0x7FFu) >> 5] >> (id & 31u)) & 1u;
    if ((id & 0x1FFF00FFu) != (0x18DA0000u | ECU_SCAN_TESTER_ADDR)) return false;
    const uint32_t ta = (id >> 8) & 0xFFu;
    return (seen_ext_ta[ta >> 5] >> (ta & 31u)) & 1u;
}

static void ecuscan_stop(const char *reason)
{
    if (!ecu_scan_active(&ecuscan)) return;
    ecu_scan_stop(&ecuscan);
    char s[40];
    (void)snprintf(s, sizeof s, "ECU,ERR,%s\r\n", reason);
    uds_line(s);
}

static void uds_stop(const char *reason)
{
    if (!uds_ready || (!uds_busy(&uds) && !uds_emitting)) return;
    uds_abort(&uds);
    uds_emitting = false;
    char s[40];
    (void)snprintf(s, sizeof s, "UDS,ERR,%s\r\n", reason);
    uds_line(s);
}

static bool hex_to_bytes(const char *hex, uint8_t *out, uint16_t cap, uint16_t *len)
{
    const size_t n = strlen(hex);
    if (n == 0u || (n & 1u) || n / 2u > cap) return false;
    for (size_t i = 0; i < n / 2u; i++) {
        const int hi = hex_nibble(hex[2u*i]), lo = hex_nibble(hex[2u*i + 1u]);
        if (hi < 0 || lo < 0) return false;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    *len = (uint16_t)(n / 2u);
    return true;
}

static void uds_command(char **save)
{
    char *a = strtok_r(NULL, ",", save);
    if (a && strcmp(a, "ABORT") == 0) { uds_stop("ABORTED"); return; }
    char *tx_t = strtok_r(NULL, ",", save), *rx_t = strtok_r(NULL, ",", save),
         *req_t = strtok_r(NULL, ",", save), *conf_t = strtok_r(NULL, ",", save);
    if (!a || !tx_t || !rx_t || !req_t || strtok_r(NULL, ",", save) ||
        (strcmp(a, "S") != 0 && strcmp(a, "E") != 0) ||
        (conf_t && strcmp(conf_t, "CONFIRM") != 0)) {
        uds_line("UDS,ERR,BAD_FORMAT\r\n"); return;
    }
    if (!tx_armed) { uds_line("UDS,ERR,NOT_ARMED\r\n"); return; }
    if ((uds_ready && (uds_busy(&uds) || uds_emitting)) || ecu_scan_active(&ecuscan)) {
        uds_line("UDS,ERR,BUSY\r\n"); return;
    }
    const bool ext = a[0] == 'E';
    uint32_t tx_id, rx_id;
    uint8_t req[UDS_MAX_REQUEST]; uint16_t len;
    if (!parse_u32(tx_t, 16, ext ? 0x1FFFFFFFu : 0x7FFu, &tx_id) ||
        !parse_u32(rx_t, 16, ext ? 0x1FFFFFFFu : 0x7FFu, &rx_id) ||
        !hex_to_bytes(req_t, req, sizeof req, &len)) {
        uds_line("UDS,ERR,BAD_FIELDS\r\n"); return;
    }
    if (!uds_ready || uds.link.tx_id != tx_id || uds.link.rx_id != rx_id || uds.link.ext != ext) {
        uds_init(&uds, tx_id, rx_id, ext, uds_can_send, NULL);
        uds_ready = true;
    }
    isotp_set_padding(&uds.link, uds_pad, uds_pad_byte);
    switch (uds_start(&uds, req, len, conf_t != NULL, system_millis)) {
    case UDS_START_OK:               return;
    case UDS_START_CONFIRM_REQUIRED: uds_line("UDS,ERR,CONFIRM_REQUIRED\r\n"); return;
    case UDS_START_BUSY:             uds_line("UDS,ERR,BUSY\r\n"); return;
    default:                         uds_line("UDS,ERR,BAD_REQUEST\r\n"); return;
    }
}

static void udspad_command(char **save)
{
    char *a = strtok_r(NULL, ",", save);
    uint32_t v;
    if (a && strtok_r(NULL, ",", save) == NULL) {
        if (strcmp(a, "NONE") == 0) { uds_pad = false; uds_line("UDS,PAD,NONE\r\n"); return; }
        if (strlen(a) == 2u && parse_u32(a, 16, 0xFFu, &v)) {
            uds_pad = true; uds_pad_byte = (uint8_t)v;
            char s[24]; (void)snprintf(s, sizeof s, "UDS,PAD,%02X\r\n", (unsigned)v); uds_line(s);
            return;
        }
    }
    uds_line("UDS,ERR,BAD_PAD\r\n");
}

/* Streams a positive response as UDS,DATA chunks while the UART ring has room. */
static void uds_emit_service(void)
{
    char s[24 + 2u * UDS_CHUNK_BYTES + 8u];
    while (uds_emitting) {
        if (uds_emit_pos >= uds.resp_len) {
            (void)snprintf(s, sizeof s, "UDS,POS,%lX,%u\r\n",
                           (unsigned long)uds.link.rx_id, (unsigned)uds.resp_len);
            if (uart_dma_free() < strlen(s)) return;
            uds_line(s);
            uds_emitting = false;
            return;
        }
        uint16_t n = (uint16_t)(uds.resp_len - uds_emit_pos);
        if (n > UDS_CHUNK_BYTES) n = UDS_CHUNK_BYTES;
        int k = snprintf(s, sizeof s, "UDS,DATA,%lX,%u,",
                         (unsigned long)uds.link.rx_id, (unsigned)uds_emit_pos);
        for (uint16_t i = 0; i < n; i++)
            k += snprintf(s + k, sizeof s - (size_t)k, "%02X", uds.resp[uds_emit_pos + i]);
        (void)snprintf(s + k, sizeof s - (size_t)k, "\r\n");
        if (uart_dma_free() < strlen(s)) return;      /* try again next loop */
        uds_line(s);
        uds_emit_pos = (uint16_t)(uds_emit_pos + n);
    }
}

static void uds_service(void)
{
    if (!uds_ready) return;
    if (!tx_armed && uds_busy(&uds)) { uds_stop("DISARMED"); return; }
    uds_emit_service();
    char s[48];
    const unsigned long rx = (unsigned long)uds.link.rx_id;
    switch (uds_poll(&uds, system_millis)) {
    case UDS_EV_NONE: return;
    case UDS_EV_POSITIVE:
        uds_emit_pos = 0u; uds_emitting = true; uds_emit_service(); return;
    case UDS_EV_NEGATIVE:
        (void)snprintf(s, sizeof s, "UDS,NEG,%lX,%02X,%02X\r\n", rx, uds.req_sid, uds.nrc); break;
    case UDS_EV_PENDING:
        (void)snprintf(s, sizeof s, "UDS,PENDING,%lX\r\n", rx); break;
    case UDS_EV_SENT_NO_RESPONSE:
        (void)snprintf(s, sizeof s, "UDS,SENT,%lX\r\n", rx); break;
    case UDS_EV_TIMEOUT:
        (void)snprintf(s, sizeof s, "UDS,ERR,TIMEOUT\r\n"); break;
    case UDS_EV_TRANSPORT_ERROR:
        (void)snprintf(s, sizeof s, "UDS,ERR,%s\r\n", isotp_status_name(uds.transport_error)); break;
    default: return;
    }
    uds_line(s);
}

static void uds_feed(const can_capture_frame_t *f)
{
    if (f->remote) return;
    if (ecu_scan_active(&ecuscan))
        ecu_scan_on_frame(&ecuscan, f->identifier, f->extended != 0u, f->data, f->dlc);
    if (!uds_ready || !uds_busy(&uds)) return;
    uds_on_frame(&uds, f->identifier, f->extended != 0u, f->data, f->dlc, system_millis);
}

static void ecuscan_command(char **save)
{
    char *a = strtok_r(NULL, ",", save);
    if (a && strcmp(a, "ABORT") == 0) { ecuscan_stop("ABORTED"); return; }
    char *first_t = strtok_r(NULL, ",", save), *last_t = strtok_r(NULL, ",", save),
         *req_t = strtok_r(NULL, ",", save), *wait_t = strtok_r(NULL, ",", save);
    uint32_t first, last, wait = ECU_SCAN_DEFAULT_WAIT;
    uint8_t req[7]; uint16_t len;
    const bool ext = a && strcmp(a, "E") == 0;
    if (!a || (strcmp(a, "S") != 0 && !ext) || !first_t || !last_t || !req_t ||
        !parse_u32(first_t, 16, ext ? 0xFFu : 0x7F7u, &first) ||
        !parse_u32(last_t, 16, ext ? 0xFFu : 0x7F7u, &last) ||
        !hex_to_bytes(req_t, req, sizeof req, &len) ||
        (wait_t && !parse_u32(wait_t, 10, 1000u, &wait)) ||
        strtok_r(NULL, ",", save) != NULL) {
        uds_line("ECU,ERR,BAD_FORMAT\r\n"); return;
    }
    if (!tx_armed) { uds_line("ECU,ERR,NOT_ARMED\r\n"); return; }
    if (ecu_scan_active(&ecuscan) || (uds_ready && (uds_busy(&uds) || uds_emitting))) {
        uds_line("ECU,ERR,BUSY\r\n"); return;
    }
    if (uds_request_needs_confirm(req, len)) { uds_line("ECU,ERR,NOT_READ_ONLY\r\n"); return; }
    if (!ecu_scan_begin(&ecuscan, ext, first, last, req, (uint8_t)len, wait,
                        uds_pad, uds_pad_byte)) {
        uds_line("ECU,ERR,BAD_FORMAT\r\n"); return;
    }
    ecu_scan_set_skip(&ecuscan, seen_in_traffic, NULL);
}

static void ecuscan_service(void)
{
    if (!ecu_scan_active(&ecuscan)) return;
    if (!tx_armed) { ecuscan_stop("DISARMED"); return; }
    if (uart_dma_free() < 64u) return;          /* keep room for the report */
    char s[64];
    switch (ecu_scan_poll(&ecuscan, system_millis, uds_can_send, NULL)) {
    case ECU_SCAN_EV_FOUND: {
        int k = snprintf(s, sizeof s, "ECU,FOUND,%lX,%lX,",
                         (unsigned long)ecuscan.found_tx, (unsigned long)ecuscan.found_rx);
        for (uint8_t i = 0; i < ecuscan.found_len && k > 0 && (size_t)k < sizeof s - 3u; i++)
            k += snprintf(s + k, sizeof s - (size_t)k, "%02X", ecuscan.found_data[i]);
        (void)snprintf(s + k, sizeof s - (size_t)k, "\r\n");
        uds_line(s);
        return;
    }
    case ECU_SCAN_EV_DONE:
        (void)snprintf(s, sizeof s, "ECU,DONE,%u\r\n", (unsigned)ecuscan.found_count);
        uds_line(s);
        return;
    case ECU_SCAN_EV_SKIPPED:
        (void)snprintf(s, sizeof s, "ECU,SKIP,%lX,IN_USE\r\n", (unsigned long)ecuscan.skipped_tx);
        uds_line(s);
        return;
    default:
        return;
    }
}
#endif /* PROJECT_CAN_TX_CONFIRMED */

/* ── Host command parser ─────────────────────────────────────────── */
static void host_command(char *lb)
{
    char *save = NULL;
    char *cmd = strtok_r(lb, ",", &save);
    if (!cmd) return;
#if PROJECT_CAN_TX_CONFIRMED == 1u
    if (strcmp(cmd, "DISARM") == 0) { tx_disarm("COMMAND"); return; }
    if (strcmp(cmd, "ARM") == 0) {
        uint32_t secs;
        char *a = strtok_r(NULL, ",", &save);
        if (!parse_u32(a, 10, TX_ARM_MAX_SECONDS, &secs) || secs == 0u ||
            strtok_r(NULL, ",", &save) != NULL) {
            (void)uart_dma_put_line("TX,ERR,BAD_ARM\r\n"); return;
        }
        tx_arm(secs);
        return;
    }
    if (strcmp(cmd, "UDS") == 0)    { uds_command(&save); return; }
    if (strcmp(cmd, "UDSPAD") == 0) { udspad_command(&save); return; }
    if (strcmp(cmd, "ECUSCAN") == 0) { ecuscan_command(&save); return; }
#else
    if (strcmp(cmd, "UDS") == 0 || strcmp(cmd, "UDSPAD") == 0 || strcmp(cmd, "ECUSCAN") == 0 ||
        strcmp(cmd, "ARM") == 0 || strcmp(cmd, "DISARM") == 0) {
        (void)uart_dma_put_line("UDS,ERR,TX_NOT_BUILT\r\n"); return;
    }
#endif
    if (strcmp(cmd, "TX") != 0) { (void)uart_dma_put_line("CMD,ERR,UNKNOWN\r\n"); return; }
    char *fmt = strtok_r(NULL, ",", &save), *id_t = strtok_r(NULL, ",", &save),
         *rtr_t = strtok_r(NULL, ",", &save), *dlc_t = strtok_r(NULL, ",", &save),
         *hex_t = strtok_r(NULL, ",", &save);
    if (!fmt || !id_t || !rtr_t || !dlc_t || !hex_t || strtok_r(NULL, ",", &save) ||
        (strcmp(fmt, "S") != 0 && strcmp(fmt, "E") != 0)) {
        (void)uart_dma_put_line("TX,ERR,BAD_FORMAT\r\n"); return;
    }
#if PROJECT_CAN_TX_CONFIRMED != 1u
    (void)uart_dma_put_line("TX,ERR,TX_NOT_BUILT\r\n");
#else
    tx_frame(fmt, id_t, rtr_t, dlc_t, hex_t);
#endif
}

static void host_command_poll(void)
{
    static char lb[HOST_LINE_CAPACITY];
    static uint32_t used = 0u;
    static bool discarding = false;     /* inside an over-long line */
    uint8_t ch;
    while (host_rx_pop(&ch)) {
        if (ch == '\r') continue;
        if (ch == '\n') {
            if (!discarding && used) { lb[used] = '\0'; host_command(lb); }
            used = 0u; discarding = false;
        } else if (discarding) {
            /* drop until end of line */
        } else if (used + 1u < sizeof lb) {
            lb[used++] = (char)ch;
        } else {
            used = 0u; discarding = true;
            (void)uart_dma_put_line("CMD,ERR,LINE_TOO_LONG\r\n");
        }
    }
}

#endif /* PROJECT_TRANSCEIVER_SILENT_CONFIRMED */

/* ══════════════════════════════════════════════════════════════════════
 * CAN interrupt handlers
 * ══════════════════════════════════════════════════════════════════════ */
void CAN1_RX0_IRQHandler(void)
{
    while (CAN1->RF0R & CAN_RF0R_FMP0) {
        const uint32_t rir  = CAN1->sFIFOMailBox[0].RIR;
        const uint32_t rdtr = CAN1->sFIFOMailBox[0].RDTR;
        const uint32_t low  = CAN1->sFIFOMailBox[0].RDLR;
        const uint32_t high = CAN1->sFIFOMailBox[0].RDHR;

        can_capture_frame_t f = {0};
        f.timestamp_ms = system_millis;
        f.timestamp_us = TIM2->CNT;
        f.extended = (rir & CAN_RI0R_IDE) ? 1u : 0u;
        f.remote   = (rir & CAN_RI0R_RTR) ? 1u : 0u;
        f.identifier = f.extended ? ((rir>>3)&0x1fffffffu) : ((rir>>21)&0x7ffu);
        f.dlc = (uint8_t)(rdtr & 0x0fu); if (f.dlc>8u) f.dlc=8u;
        if (!f.remote) {
            for (uint32_t i=0;i<f.dlc&&i<4;i++) f.data[i]=(uint8_t)(low >>(i*8));
            for (uint32_t i=4;i<f.dlc&&i<8u;i++) f.data[i]=(uint8_t)(high>>((i-4u)*8u));
        }

        const uint32_t now_ms = system_millis;
        if (isr_last_rx_ts_ms != 0u) {
            const uint32_t intv = now_ms - isr_last_rx_ts_ms;
            if (intv > 0u && (isr_min_interval_ms == 0u || intv < isr_min_interval_ms))
                isr_min_interval_ms = intv;
        }
        isr_last_rx_ts_ms = now_ms;

#if PROJECT_TRANSCEIVER_SILENT_CONFIRMED == 1u && PROJECT_CAN_TX_CONFIRMED == 1u
        seen_mark(&f);      /* before the queue: ECU scan never sends on it */
#endif
        rx_frame_count++;
        valid_frames_at_rate++;
        bitrate_error_streak = 0u;
        (void)can_capture_queue_push_isr(&capture_queue, &f); /* overflow counted in queue */
        CAN1->RF0R = CAN_RF0R_RFOM0;
    }
    if (CAN1->RF0R & CAN_RF0R_FOVR0) { can_error_count++; CAN1->RF0R = CAN_RF0R_FOVR0; }
}

void CAN1_SCE_IRQHandler(void)
{
    const uint32_t esr = CAN1->ESR;
    if (esr & (CAN_ESR_BOFF|CAN_ESR_EPVF|CAN_ESR_EWGF)) can_error_count++;
    if (esr & CAN_ESR_LEC) {
        can_lec_count++;
        bitrate_error_streak++;
        if (bitrate_error_streak >= CAN_REACQUIRE_ERROR_MAX && can_locked)
            reacquire_requested = 1u;
        CAN1->ESR = 0u;                    /* clear LEC so the next error is seen */
    }
    if (esr & CAN_ESR_BOFF) {
        busoff_seen = 1u;
        if (can_locked) reacquire_requested = 1u;
    }
    CAN1->MSR = CAN_MSR_ERRI;              /* rc_w1: clear error interrupt flag */
}

/* ══════════════════════════════════════════════════════════════════════
 * Public API
 * ══════════════════════════════════════════════════════════════════════ */
bool     project_capture_pop(can_capture_frame_t *out) { return can_capture_queue_pop(&capture_queue, out); }
uint32_t project_capture_dropped(void)  { return capture_queue.dropped; }
uint32_t project_capture_errors(void)   { return can_error_count; }
uint32_t project_capture_millis(void)   { return system_millis; }

/* ══════════════════════════════════════════════════════════════════════
 * Watchdog and reset cause
 * ══════════════════════════════════════════════════════════════════════ */
static bool iwdg_start(void)
{
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP;   /* halted debugger: no reset */
    IWDG->KR  = 0xCCCCu;                             /* start; LSI turns on by hardware */
    IWDG->KR  = 0x5555u;                             /* unlock PR/RLR */
    IWDG->PR  = IWDG_PR_DIV64;
    IWDG->RLR = IWDG_RELOAD;
    const uint32_t t0 = system_millis;
    while ((IWDG->SR & (IWDG_SR_PVU | IWDG_SR_RVU)) != 0u) {
        if ((uint32_t)(system_millis - t0) > IWDG_SR_WAIT_MS) return false;
    }
    IWDG->KR = 0xAAAAu;                              /* reload */
    return true;
}

static inline void iwdg_feed(void) { IWDG->KR = 0xAAAAu; }

/* Reads and clears RCC_CSR reset flags; returns the most specific cause. */
static const char *reset_cause_take(void)
{
    const uint32_t csr = RCC->CSR;
    const char *cause = "UNKNOWN";
    if      (csr & RCC_CSR_IWDGRSTF) cause = "IWDG";
    else if (csr & RCC_CSR_WWDGRSTF) cause = "WWDG";
    else if (csr & RCC_CSR_LPWRRSTF) cause = "LOWPOWER";
    else if (csr & RCC_CSR_SFTRSTF)  cause = "SOFTWARE";
    else if (csr & RCC_CSR_BORRSTF)  cause = "BROWNOUT_OR_POWER_ON";
    else if (csr & RCC_CSR_PINRSTF)  cause = "PIN";
    RCC->CSR |= RCC_CSR_RMVF;
    return cause;
}

/* ══════════════════════════════════════════════════════════════════════
 * User LEDs (UM1472 Rev 9 Section 7.5; MB997 rev E schematic sheet 4: each
 * PD pin drives its LED through a resistor to GND, so HIGH = LED on).
 *   LD4 green  PD12: heartbeat, toggles every 500 ms while the firmware runs
 *   LD3 orange PD13: on = CAN bitrate locked, off = still scanning
 *   LD5 red    PD14: on = CAN error passive / bus-off or HSI clock fallback;
 *                    200 ms flash on every new CAN error
 *   LD6 blue   PD15: on = ARM window open (transmit allowed)
 * ══════════════════════════════════════════════════════════════════════ */
#if PROJECT_TRANSCEIVER_SILENT_CONFIRMED == 1u
#define LED_GREEN  12u
#define LED_ORANGE 13u
#define LED_RED    14u
#define LED_BLUE   15u

static void leds_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;
    const uint32_t pins = (1u << LED_GREEN) | (1u << LED_ORANGE) |
                          (1u << LED_RED)   | (1u << LED_BLUE);
    GPIOD->BSRR = pins << 16u;                                   /* all off */
    GPIOD->OTYPER &= ~pins;                                      /* push-pull */
    GPIOD->PUPDR  &= ~((3u << (2u*LED_GREEN)) | (3u << (2u*LED_ORANGE)) |
                       (3u << (2u*LED_RED))   | (3u << (2u*LED_BLUE)));
    GPIOD->OSPEEDR &= ~((3u << (2u*LED_GREEN)) | (3u << (2u*LED_ORANGE)) |
                        (3u << (2u*LED_RED))   | (3u << (2u*LED_BLUE)));
    GPIOD->MODER = (GPIOD->MODER & ~((3u << (2u*LED_GREEN)) | (3u << (2u*LED_ORANGE)) |
                                     (3u << (2u*LED_RED))   | (3u << (2u*LED_BLUE))))
                 | (1u << (2u*LED_GREEN)) | (1u << (2u*LED_ORANGE))
                 | (1u << (2u*LED_RED))   | (1u << (2u*LED_BLUE));
}

static void led_set(uint32_t pin, bool on)
{
    GPIOD->BSRR = on ? (1u << pin) : (1u << (pin + 16u));
}

static void leds_update(bool armed)
{
    static uint32_t last_err, red_until;
    const uint32_t now = system_millis;
    led_set(LED_GREEN, ((now / 500u) & 1u) != 0u);
    led_set(LED_ORANGE, can_locked != 0u);
    const uint32_t errs = can_error_count + can_lec_count;
    if (errs != last_err) { last_err = errs; red_until = now + 200u; }
    const uint32_t esr = CAN1->ESR;
    const bool red = !clock_ok || (esr & (CAN_ESR_BOFF | CAN_ESR_EPVF)) != 0u ||
                     (int32_t)(now - red_until) < 0;
    led_set(LED_RED, red);
    led_set(LED_BLUE, armed);
}
#endif /* PROJECT_TRANSCEIVER_SILENT_CONFIRMED */

/* ══════════════════════════════════════════════════════════════════════
 * main()
 * ══════════════════════════════════════════════════════════════════════ */
int main(void)
{
    transceiver_s_init();            /* S released = silent, before anything else */
    clock_ok = clock_init_168mhz();  /* on failure the chip stays on HSI 16 MHz */
    bus_clocks_update();             /* all dividers derived from real RCC state */
    can_capture_queue_init(&capture_queue);
    can_profiler_init(&profiler);
    if (SysTick_Config(SystemCoreClock / 1000u) != 0u) for (;;) { }
    tim2_us_init();

#if PROJECT_TRANSCEIVER_SILENT_CONFIRMED == 1u
    if (!host_stream_init()) for (;;) { __WFI(); }   /* no valid UART divider */
    uart_dma_init();
    {
        char s[48];
        (void)snprintf(s, sizeof s, "STATUS,RESET,%s\r\n", reset_cause_take());
        (void)uart_dma_put_line(s);
    }
    /* HSI is only +-4 % (DS8626 Table 34): CAN timing not trusted, TX refused */
    if (!clock_ok) (void)uart_dma_put_line("STATUS,CLOCK,HSI_FALLBACK\r\n");
    gpio_can1_init();
    leds_init();
    can1_start_passive();
    if (!iwdg_start()) (void)uart_dma_put_line("STATUS,WATCHDOG,FAILED\r\n");

    uint32_t last_dwell_report_ms = 0u, last_stats_ms = 0u;

    for (;;) {
        iwdg_feed();
        host_command_poll();
#if PROJECT_CAN_TX_CONFIRMED == 1u
        tx_service();
        uds_service();
        ecuscan_service();
#endif
        if (reacquire_requested) {
            reacquire_requested  = 0u;
#if PROJECT_CAN_TX_CONFIRMED == 1u
            tx_disarm("REACQUIRE");
            seen_clear();
#endif
            can_locked           = 0u;
            scan_index           = 0u;
            can_profiler_init(&profiler);
            if (can1_set_bitrate(bitrate_candidates[0], true))
                (void)uart_dma_put_line("STATUS,REACQUIRE\r\n");
        }

        {
            const uint32_t nd = compute_dwell_ms();
            if (nd != current_dwell_ms) {
                current_dwell_ms = nd;
                if ((system_millis - last_dwell_report_ms) >= 5000u) {
                    last_dwell_report_ms = system_millis;
                    status_line("STATUS,DWELL,%lu\r\n", current_dwell_ms);
                }
            }
        }

        can1_scan_step();
#if PROJECT_CAN_TX_CONFIRMED == 1u
        leds_update(tx_armed);
#else
        leds_update(false);
#endif

        {
            can_capture_frame_t frame;
            while (can_capture_queue_pop(&capture_queue, &frame)) {
                char csv[72];
                if (can_capture_frame_to_csv(&frame, csv, sizeof csv))
                    (void)uart_dma_put_line(csv);
                char pid[OBD2_LINE_CAPACITY];
                if (obd2_decode(&frame, pid, sizeof pid))
                    (void)uart_dma_put_line(pid);
                can_profiler_update(&profiler, &frame);
#if PROJECT_CAN_TX_CONFIRMED == 1u
                uds_feed(&frame);
#endif
            }
        }

        can_profiler_emit(&profiler, system_millis, profiler_write);

        if ((uint32_t)(system_millis - last_stats_ms) >= STATS_PERIOD_MS) {
            last_stats_ms = system_millis;
            /* ESR (RM0090 CAN_ESR): REC 31:24, TEC 23:16, BOFF/EPVF/EWGF 2:0 */
            const uint32_t esr = CAN1->ESR;
            const char *st = (esr & CAN_ESR_BOFF) ? "BUS_OFF" :
                             (esr & CAN_ESR_EPVF) ? "PASSIVE" :
                             (esr & CAN_ESR_EWGF) ? "WARNING" : "ACTIVE";
            char s[128];
            (void)snprintf(s, sizeof s, "STATUS,STATS,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%s\r\n",
                           (unsigned long)rx_frame_count,
                           (unsigned long)capture_queue.dropped,
                           (unsigned long)uart_dma_dropped_lines(),
                           (unsigned long)can_error_count,
                           (unsigned long)can_lec_count,
                           (unsigned long)((esr & CAN_ESR_TEC) >> CAN_ESR_TEC_Pos),
                           (unsigned long)((esr & CAN_ESR_REC) >> CAN_ESR_REC_Pos), st);
            (void)uart_dma_put_line(s);
        }
    }
#else
    for (;;) { __WFI(); }
#endif
}
