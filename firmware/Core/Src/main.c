/*
 * Project Civic - STM32F407VGT6 (STM32F4DISCOVERY) dual-bus CAN diagnostic core.
 *
 * Pin map (verified against DS8626 pin/AF tables and UM1472 MB997 pin table:
 * every pin below is FT and has no on-board function on MB997):
 *   CAN1 F-CAN: PD0 RX / PD1 TX AF9 (push-pull), TJA1051 #1 S = PD2
 *   CAN2 B-CAN: PB12 RX / PB13 TX AF9 (push-pull), TJA1051 #2 S = PD3
 *   S pin HIGH (transceiver silent) in every mode except bxCAN Normal, so the
 *   bus is never driven unless the controller is a full node (RM0090: in
 *   loop-back mode CANTX still carries the frames).
 *   Starter relay PE4, ACC PE5, IGN2 PE6. CS43L22 held in reset by PD4.
 *   No serial host link / no ESP32: all lines go to the USB dashboard.
 *
 * While ignition is OFF, both bxCAN blocks are parked in SILM. PE5/PE6
 * ignition inputs start independent passive bitrate scans (listen-only).
 * Each bus has its own capture queue and status/error counters.
 * Frames stream to the USB dashboard as:
 *     <ts_us>,STD|EXT,<id_hex>,<dlc>,<rtr>,<payload_hex>,<bus>
 * plus PID / PROFILE / SIG lines (PROFILE/SIG carry bus as the last field).
 *
 * Write features (CAN transmit, UDS/OBD requests, ECU scan, relearn, flashing,
 * remote engine start, gateway, periodic TX) run per bus behind an ARM window;
 * every non-read UDS service additionally needs CONFIRM. Operator authentication
 * is done by the ESP32 dashboard (Wi-Fi WPA2).
 *
 * Host protocol (one command per line; <B> = F|B|0|1 selects the bus):
 *   ARM,<B>,<seconds>                 DISARM,<B>
 *   MODE,<B>,<SILENT|NORMAL|LOOPBACK|LOOPBACKSILENT>
 *   BITRATE,<B>,<AUTO|hz>
 *   TX,<B>,<S|E>,<id_hex>,<rtr 0|1>,<dlc>,<hex|->
 *   UDS,<B>,<S|E>,<tx_hex>,<rx_hex>,<req_hex>[,CONFIRM]   UDS,<B>,ABORT
 *   UDSPAD,<B>,<hex byte|NONE>
 *   ECUSCAN,<B>,<S|E>,<first_hex>,<last_hex>,<req_hex>[,<wait_ms>]  ECUSCAN,<B>,ABORT
 *   PERIODIC,<slot 0..7>,<B>,<S|E>,<id_hex>,<dlc>,<hex|->,<period_ms>   PERIODIC,<slot>,OFF
 *   GATEWAY,<ON|OFF>
 *   STARTCFG,<timeout_s>,<req_armed>,<req_neutral>,<req_brake>,<req_immob>,<rpm_thresh>
 *   STARTINPUTS,<neutral 0|1>,<brake 0|1>,<immob 0|1>
 *   STARTFRAME,<B>,<S|E>,<id_hex>,<dlc>,<hex>|NONE
 *   START            STARTCANCEL
 *   GEAR,<raw 0..15> | GEAR,CLEAR    decoded gear -> LD4/LD3/LD5/LD6 shift chase
 * Status/stream out:
 *   STATUS,SYS,RESET,<cause> | STATUS,SYS,CLOCK,HSI_FALLBACK | STATUS,SYS,WATCHDOG,FAILED
 *   STATUS,<F|B>,SCAN,<hz> | LOCK,<hz> | REACQUIRE | DWELL,<ms> | ARMED,<s> | DISARMED,<reason>
 *   STATUS,<F|B>,MODE,<mode> | BITRATE,<hz>
 *   STATUS,<F|B>,STATS,<rx>,<qdrop>,<uartdrop>,<canerr>,<lecerr>,<tec>,<rec>,<state>
 *   TX,<F|B>,QUEUED|OK|ERR,...   UDS,<F|B>,...   ECU,<F|B>,...
 *   START,<CRANK|STARTED|TIMEOUT|BLOCKED|ABORTED|CANCEL>[,<detail>]
 *
 * Transceiver: NXP TJA1051 on each bus (only supported transceiver).
 */
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
#include "periph_timing.h"
#include "uds_client.h"
#include "ecu_scan.h"
#include "can_hw.h"
#include "device_id.h"
#include "log_mail.h"
#include "remote_start.h"
#include "gear_led.h"
#include "uart_esp.h"
#ifdef NET_DASHBOARD
#include "net_usb.h"
#include "dash.h"
#include "dash_store.h"
#endif
#ifdef USB_CDC_ENABLED
#include "usb_gvret.h"
#endif

/* Larger line/ring so UDS programming blocks (RequestDownload 0x34 /
 * TransferData 0x36) carry useful payloads: a 512-byte UDS request = ~1024 hex
 * chars + framing. ISO-TP itself handles up to 4095 bytes; the dashboard
 * chunks TransferData to <= UDS_MAX_REQUEST per frame. */
#define HOST_LINE_CAPACITY       1152u
#define STATS_PERIOD_MS          1000u
#define UDS_MAX_REQUEST          512u
#define UDS_CHUNK_BYTES           32u
#define PERIODIC_SLOTS             8u
#define RPM_FRESH_MS             500u

/* Independent watchdog (IWDG), LSI-clocked. PR=/64, reload 1000 => ~2.0 s. */
#define IWDG_PR_DIV64               4u
#define IWDG_RELOAD              1000u
#define IWDG_SR_WAIT_MS            50u

/* ── state ─────────────────────────────────────────────────────────────── */
/* CPU/ISR-only (no DMA): live in the 64 KB CCM RAM, zeroed by startup. */
static can_capture_queue_t capture_queue[2] __attribute__((section(".ccmram"))); /* independent RX queue per bxCAN instance */
static can_profiler_t      profiler[2];
volatile uint32_t   system_millis;   /* shared with net_usb/lwIP/dashboard */
static uint32_t            pclk1_hz, pclk2_hz, tim2_clk_hz;
static bool                clock_ok;
static volatile uint32_t   processed_can_frames;
static volatile uint32_t   out_lines_emitted;
static volatile uint32_t   out_last_activity_ms;
static volatile uint32_t   processed_last_activity_ms;
static uint32_t            measured_vdd_mv = 3300u;
static bool                vdd_low;

static can_cfg_t  s_cfg[2];
static can_ctrl_t s_can[2];

/* remote engine start */
static remote_start_t      rs;
static remote_start_inputs_t rs_in;        /* operator-asserted interlocks + rpm */
static uint32_t            rpm_last_ms;
static bool                start_frame_set;
static uint8_t             start_frame_bus;
static bool                start_frame_ext;
static uint32_t            start_frame_id;
static uint8_t             start_frame_dlc;
static uint8_t             start_frame_data[8];
#define STARTER_PORT  GPIOE
#define STARTER_PIN   4u

/* gear-shift LED chase (LD4 green -> LD3 orange -> LD5 red -> LD6 blue);
 * fed by the dashboard's decoded Honda DID 2612 value via GEAR,<n>. */
static gear_led_t          gear_led;
static bool    gear_bcast_on = true;              /* 0x1A3 passive gear source */
static uint8_t gear_bcast_last = 0xFFu, gear_bcast_same;

/* periodic TX scheduler */
typedef struct {
    bool     active;
    uint8_t  bus, dlc;
    bool     ext, rtr;
    uint32_t id, period_ms, next_ms;
    uint8_t  data[8];
} periodic_slot_t;
static periodic_slot_t periodic[PERIODIC_SLOTS];

static bool gateway_on;

/* ── clock (unchanged from the single-bus firmware) ─────────────────────── */
void SysTick_Handler(void) { system_millis++; }

static bool clock_init_168mhz(void)
{
    RCC->CR |= RCC_CR_HSEON;
    uint32_t to = 1000000u;
    while (!(RCC->CR & RCC_CR_HSERDY) && --to) { }
    if (!to) return false;
    FLASH->ACR = FLASH_ACR_LATENCY_5WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
    to = 1000u;
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_5WS && --to) { }
    if (!to) return false;
    RCC->PLLCFGR = (RCC->PLLCFGR & 0xF0BC8000u)
                 | (8u << RCC_PLLCFGR_PLLM_Pos) | (336u << RCC_PLLCFGR_PLLN_Pos)
                 | (0u << RCC_PLLCFGR_PLLP_Pos) | RCC_PLLCFGR_PLLSRC_HSE
                 | (7u << RCC_PLLCFGR_PLLQ_Pos);
    RCC->CR |= RCC_CR_PLLON;
    to = 1000000u;
    while (!(RCC->CR & RCC_CR_PLLRDY) && --to) { }
    if (!to) return false;
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE2 | RCC_CFGR_PPRE1))
              | RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_PPRE1_DIV4;
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    to = 1000000u;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL && --to) { }
    if (!to) { RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2); return false; }
    return true;
}

static uint32_t apb_divider(uint32_t field) { return (field & 4u) ? (2u << (field & 3u)) : 1u; }

static void bus_clocks_update(void)
{
    SystemCoreClockUpdate();
    const uint32_t cfgr = RCC->CFGR;
    const uint32_t d1 = apb_divider((cfgr & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos);
    const uint32_t d2 = apb_divider((cfgr & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos);
    pclk1_hz = SystemCoreClock / d1;
    pclk2_hz = SystemCoreClock / d2;
    tim2_clk_hz = (d1 == 1u) ? pclk1_hz : (2u * pclk1_hz);
}

static void tim2_us_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    (void)RCC->APB1ENR;
    TIM2->PSC = (tim2_clk_hz / 1000000u) - 1u;
    TIM2->ARR = 0xFFFFFFFFu;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 = TIM_CR1_CEN;
}


static bool profiler_write(const char *line)
{
    const uint32_t len = (uint32_t)strlen(line);
#ifdef NET_DASHBOARD
    dash_on_line(line);
#endif
    (void)len;
    return true;
}

static void emit(const char *s)
{
#ifdef NET_DASHBOARD
    dash_on_line(s);              /* the dashboard parses every STM32 line */
#endif
    uart_esp_emit(s);             /* mirror every line to ESP32 over USART2 */
    out_lines_emitted++;
    out_last_activity_ms = system_millis;
}
static void emit_bus_line(const can_ctrl_t *c, const char *rest)
{ char s[72]; (void)snprintf(s, sizeof s, "STATUS,%s,%s", c->cfg->label, rest); emit(s); }

/* ── parsing helpers ───────────────────────────────────────────────────── */
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

static int hex_nibble(char c)
{
    if (c>='0'&&c<='9') return c-'0';
    if (c>='a'&&c<='f') return c-'a'+10;
    if (c>='A'&&c<='F') return c-'A'+10;
    return -1;
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

/* bus selector: "F"/"0" -> &s_can[0], "B"/"1" -> &s_can[1], else NULL */
static can_ctrl_t *sel_bus(const char *t)
{
    if (!t) return NULL;
    if ((t[0]=='F'||t[0]=='0') && t[1]=='\0') return &s_can[0];
    if ((t[0]=='B'||t[0]=='1') && t[1]=='\0') return &s_can[1];
    return NULL;
}
static uint8_t ctrl_index(const can_ctrl_t *c) { return (c == &s_can[1]) ? 1u : 0u; }

/* ── diagnostics: UDS / ECU-scan, per controller ───────────────────────── */
static bool uds_can_send(void *ctx, uint32_t id, bool ext, const uint8_t *d, uint8_t len)
{
    can_ctrl_t *c = (can_ctrl_t *)ctx;
    if (!can_hw_tx_window_open(c, system_millis) || c->tx_tokens == 0u) return false;
    return can_hw_load_mailbox(c, id, ext, false, d, len, true);
}

static void uds_line_bus(const can_ctrl_t *c, const char *rest)
{
    char s[64];
    (void)snprintf(s, sizeof s, "UDS,%s,%s", c->cfg->label, rest);
    emit(s);
}

static void ecuscan_stop(can_ctrl_t *c, const char *reason)
{
    if (!ecu_scan_active(&c->ecuscan)) return;
    ecu_scan_stop(&c->ecuscan);
    char s[40]; (void)snprintf(s, sizeof s, "ERR,%s\r\n", reason);
    char t[64]; (void)snprintf(t, sizeof t, "ECU,%s,%s", c->cfg->label, s);
    emit(t);
}

static void uds_stop(can_ctrl_t *c, const char *reason)
{
    if (!c->uds_ready || (!uds_busy(&c->uds) && !c->uds_emitting)) return;
    uds_abort(&c->uds);
    c->uds_emitting = false;
    char s[48]; (void)snprintf(s, sizeof s, "ERR,%s\r\n", reason);
    uds_line_bus(c, s);
}

static void disarm(can_ctrl_t *c, const char *reason)
{
    const bool was_armed = c->tx_armed;
    /* Always force SILM, even after an already-disarmed MODE change or error. */
    uds_stop(c, "DISARMED");
    ecuscan_stop(c, "DISARMED");
    can_hw_tx_disarm_hw(c);
    if (was_armed) {
        char s[48]; (void)snprintf(s, sizeof s, "DISARMED,%s\r\n", reason);
        emit_bus_line(c, s);
    }
}

/* TX mailbox result callback from can_hw_tx_service */
static void tx_report(can_ctrl_t *c, uint32_t id, bool quiet, const char *result)
{
    char s[48];
    if (quiet) {
        if (strcmp(result, "OK") != 0)
            (void)snprintf(s, sizeof s, "UDS,%s,TXERR,%lX,%s\r\n", c->cfg->label,
                           (unsigned long)id, result);
        else return;
    } else {
        if (strcmp(result, "OK") == 0)
            (void)snprintf(s, sizeof s, "TX,%s,OK,%lX\r\n", c->cfg->label, (unsigned long)id);
        else
            (void)snprintf(s, sizeof s, "TX,%s,ERR,%s,%lX\r\n", c->cfg->label, result,
                           (unsigned long)id);
    }
    emit(s);
}

static void tx_frame(can_ctrl_t *c, const char *fmt, const char *id_t,
                     const char *rtr_t, const char *dlc_t, const char *hex_t)
{
    char pfx[8]; (void)snprintf(pfx, sizeof pfx, "TX,%s,", c->cfg->label);
    if (!can_hw_tx_window_open(c, system_millis)) {
        char s[32]; (void)snprintf(s, sizeof s, "%sERR,NOT_ARMED\r\n", pfx); emit(s); return; }
    const bool ext = strcmp(fmt, "E") == 0;
    uint32_t id, rtr, dlc;
    char s[40];
    if (!parse_u32(id_t, 16, ext ? 0x1FFFFFFFu : 0x7FFu, &id) ||
        !parse_u32(rtr_t, 10, 1u, &rtr) || !parse_u32(dlc_t, 10, 8u, &dlc) ||
        ((rtr || dlc == 0u) ? strcmp(hex_t, "-") != 0 : strlen(hex_t) != dlc * 2u)) {
        (void)snprintf(s, sizeof s, "%sERR,BAD_FIELDS\r\n", pfx); emit(s); return;
    }
    uint8_t data[8] = {0};
    for (uint32_t i = 0; i < dlc && !rtr; i++) {
        const int hi = hex_nibble(hex_t[i*2]), lo = hex_nibble(hex_t[i*2+1]);
        if (hi < 0 || lo < 0) { (void)snprintf(s, sizeof s, "%sERR,BAD_HEX\r\n", pfx); emit(s); return; }
        data[i] = (uint8_t)((hi << 4) | lo);
    }
    if (c->tx_tokens == 0u) { (void)snprintf(s, sizeof s, "%sERR,RATE_LIMIT\r\n", pfx); emit(s); return; }
    if (!can_hw_load_mailbox(c, id, ext, rtr != 0u, data, (uint8_t)dlc, false)) {
        (void)snprintf(s, sizeof s, "%sERR,MAILBOX_BUSY\r\n", pfx); emit(s); return;
    }
    (void)snprintf(s, sizeof s, "%sQUEUED,%lX\r\n", pfx, (unsigned long)id); emit(s);
}

static void uds_command(can_ctrl_t *c, char **save)
{
    char *a = strtok_r(NULL, ",", save);
    if (a && strcmp(a, "ABORT") == 0) { uds_stop(c, "ABORTED"); return; }
    char *tx_t = strtok_r(NULL, ",", save), *rx_t = strtok_r(NULL, ",", save),
         *req_t = strtok_r(NULL, ",", save), *conf_t = strtok_r(NULL, ",", save);
    if (!a || !tx_t || !rx_t || !req_t || strtok_r(NULL, ",", save) ||
        (strcmp(a, "S") != 0 && strcmp(a, "E") != 0) ||
        (conf_t && strcmp(conf_t, "CONFIRM") != 0)) { uds_line_bus(c, "ERR,BAD_FORMAT\r\n"); return; }
    if (!c->tx_armed) { uds_line_bus(c, "ERR,NOT_ARMED\r\n"); return; }
    if ((c->uds_ready && (uds_busy(&c->uds) || c->uds_emitting)) || ecu_scan_active(&c->ecuscan)) {
        uds_line_bus(c, "ERR,BUSY\r\n"); return; }
    const bool ext = a[0] == 'E';
    uint32_t tx_id, rx_id; uint8_t req[UDS_MAX_REQUEST]; uint16_t len;
    if (!parse_u32(tx_t, 16, ext ? 0x1FFFFFFFu : 0x7FFu, &tx_id) ||
        !parse_u32(rx_t, 16, ext ? 0x1FFFFFFFu : 0x7FFu, &rx_id) ||
        !hex_to_bytes(req_t, req, sizeof req, &len)) { uds_line_bus(c, "ERR,BAD_FIELDS\r\n"); return; }
    if (!c->uds_ready || c->uds.link.tx_id != tx_id || c->uds.link.rx_id != rx_id || c->uds.link.ext != ext) {
        uds_init(&c->uds, tx_id, rx_id, ext, uds_can_send, c);
        c->uds_ready = true;
    }
    isotp_set_padding(&c->uds.link, c->uds_pad, c->uds_pad_byte);
    switch (uds_start(&c->uds, req, len, conf_t != NULL, system_millis)) {
    case UDS_START_OK:               return;
    case UDS_START_CONFIRM_REQUIRED: uds_line_bus(c, "ERR,CONFIRM_REQUIRED\r\n"); return;
    case UDS_START_BUSY:             uds_line_bus(c, "ERR,BUSY\r\n"); return;
    default:                         uds_line_bus(c, "ERR,BAD_REQUEST\r\n"); return;
    }
}

static void udspad_command(can_ctrl_t *c, char **save)
{
    char *a = strtok_r(NULL, ",", save);
    uint32_t v;
    if (a && strtok_r(NULL, ",", save) == NULL) {
        if (strcmp(a, "NONE") == 0) { c->uds_pad = false; uds_line_bus(c, "PAD,NONE\r\n"); return; }
        if (strlen(a) == 2u && parse_u32(a, 16, 0xFFu, &v)) {
            c->uds_pad = true; c->uds_pad_byte = (uint8_t)v;
            char s[24]; (void)snprintf(s, sizeof s, "PAD,%02X\r\n", (unsigned)v); uds_line_bus(c, s); return;
        }
    }
    uds_line_bus(c, "ERR,BAD_PAD\r\n");
}

static void uds_emit_service(can_ctrl_t *c)
{
    char s[24 + 2u * UDS_CHUNK_BYTES + 16u];
    while (c->uds_emitting) {
        if (c->uds_emit_pos >= c->uds.resp_len) {
            (void)snprintf(s, sizeof s, "UDS,%s,POS,%lX,%u\r\n", c->cfg->label,
                           (unsigned long)c->uds.link.rx_id, (unsigned)c->uds.resp_len);
            emit(s); c->uds_emitting = false; return;
        }
        uint16_t n = (uint16_t)(c->uds.resp_len - c->uds_emit_pos);
        if (n > UDS_CHUNK_BYTES) n = UDS_CHUNK_BYTES;
        int k = snprintf(s, sizeof s, "UDS,%s,DATA,%lX,%u,", c->cfg->label,
                         (unsigned long)c->uds.link.rx_id, (unsigned)c->uds_emit_pos);
        for (uint16_t i = 0; i < n; i++)
            k += snprintf(s + k, sizeof s - (size_t)k, "%02X", c->uds.resp[c->uds_emit_pos + i]);
        (void)snprintf(s + k, sizeof s - (size_t)k, "\r\n");
        emit(s);
        c->uds_emit_pos = (uint16_t)(c->uds_emit_pos + n);
    }
}

static void uds_service(can_ctrl_t *c)
{
    if (!c->uds_ready) return;
    if (!c->tx_armed && uds_busy(&c->uds)) { uds_stop(c, "DISARMED"); return; }
    uds_emit_service(c);
    char s[48];
    const unsigned long rx = (unsigned long)c->uds.link.rx_id;
    switch (uds_poll(&c->uds, system_millis)) {
    case UDS_EV_NONE: return;
    case UDS_EV_POSITIVE: c->uds_emit_pos = 0u; c->uds_emitting = true; uds_emit_service(c); return;
    case UDS_EV_NEGATIVE:
        (void)snprintf(s, sizeof s, "NEG,%lX,%02X,%02X\r\n", rx, c->uds.req_sid, c->uds.nrc); break;
    case UDS_EV_PENDING:  (void)snprintf(s, sizeof s, "PENDING,%lX\r\n", rx); break;
    case UDS_EV_SENT_NO_RESPONSE: (void)snprintf(s, sizeof s, "SENT,%lX\r\n", rx); break;
    case UDS_EV_TIMEOUT:  (void)snprintf(s, sizeof s, "ERR,TIMEOUT\r\n"); break;
    case UDS_EV_TRANSPORT_ERROR:
        (void)snprintf(s, sizeof s, "ERR,%s\r\n", isotp_status_name(c->uds.transport_error)); break;
    default: return;
    }
    uds_line_bus(c, s);
}

static void ecuscan_command(can_ctrl_t *c, char **save)
{
    char *a = strtok_r(NULL, ",", save);
    if (a && strcmp(a, "ABORT") == 0) { ecuscan_stop(c, "ABORTED"); return; }
    char *first_t = strtok_r(NULL, ",", save), *last_t = strtok_r(NULL, ",", save),
         *req_t = strtok_r(NULL, ",", save), *wait_t = strtok_r(NULL, ",", save);
    uint32_t first, last, wait = ECU_SCAN_DEFAULT_WAIT;
    uint8_t req[7]; uint16_t len;
    const bool ext = a && strcmp(a, "E") == 0;
    char pfx[8]; (void)snprintf(pfx, sizeof pfx, "ECU,%s,", c->cfg->label);
    char s[40];
    if (!a || (strcmp(a, "S") != 0 && !ext) || !first_t || !last_t || !req_t ||
        !parse_u32(first_t, 16, ext ? 0xFFu : 0x7F7u, &first) ||
        !parse_u32(last_t, 16, ext ? 0xFFu : 0x7F7u, &last) ||
        !hex_to_bytes(req_t, req, sizeof req, &len) ||
        (wait_t && !parse_u32(wait_t, 10, 1000u, &wait)) ||
        strtok_r(NULL, ",", save) != NULL) {
        (void)snprintf(s, sizeof s, "%sERR,BAD_FORMAT\r\n", pfx); emit(s); return; }
    if (!c->tx_armed) { (void)snprintf(s, sizeof s, "%sERR,NOT_ARMED\r\n", pfx); emit(s); return; }
    if (ecu_scan_active(&c->ecuscan) || (c->uds_ready && (uds_busy(&c->uds) || c->uds_emitting))) {
        (void)snprintf(s, sizeof s, "%sERR,BUSY\r\n", pfx); emit(s); return; }
    if (uds_request_needs_confirm(req, len)) {
        (void)snprintf(s, sizeof s, "%sERR,NOT_READ_ONLY\r\n", pfx); emit(s); return; }
    if (!ecu_scan_begin(&c->ecuscan, ext, first, last, req, (uint8_t)len, wait, c->uds_pad, c->uds_pad_byte)) {
        (void)snprintf(s, sizeof s, "%sERR,BAD_FORMAT\r\n", pfx); emit(s); return; }
    /* caller attaches the per-ctrl "seen in traffic" skip filter after begin */
}

/* skip wrapper closures need ctx = ctrl; ecu_scan_skip_fn signature matches */
static bool ecuscan_skip(void *ctx, uint32_t id, bool ext)
{
    return can_hw_seen_in_traffic((can_ctrl_t *)ctx, id, ext);
}

static void ecuscan_service(can_ctrl_t *c)
{
    if (!ecu_scan_active(&c->ecuscan)) return;
    if (!c->tx_armed) { ecuscan_stop(c, "DISARMED"); return; }
    char s[80];
    switch (ecu_scan_poll(&c->ecuscan, system_millis, uds_can_send, c)) {
    case ECU_SCAN_EV_FOUND: {
        int k = snprintf(s, sizeof s, "ECU,%s,FOUND,%lX,%lX,", c->cfg->label,
                         (unsigned long)c->ecuscan.found_tx, (unsigned long)c->ecuscan.found_rx);
        for (uint8_t i = 0; i < c->ecuscan.found_len && k > 0 && (size_t)k < sizeof s - 3u; i++)
            k += snprintf(s + k, sizeof s - (size_t)k, "%02X", c->ecuscan.found_data[i]);
        (void)snprintf(s + k, sizeof s - (size_t)k, "\r\n"); emit(s); return; }
    case ECU_SCAN_EV_DONE:
        (void)snprintf(s, sizeof s, "ECU,%s,DONE,%u\r\n", c->cfg->label, (unsigned)c->ecuscan.found_count);
        emit(s); return;
    case ECU_SCAN_EV_SKIPPED:
        (void)snprintf(s, sizeof s, "ECU,%s,SKIP,%lX,IN_USE\r\n", c->cfg->label,
                       (unsigned long)c->ecuscan.skipped_tx); emit(s); return;
    default: return;
    }
}

/* ── remote engine start ────────────────────────────────────────────────── */
static void starter_gpio_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; (void)RCC->AHB1ENR;
    STARTER_PORT->BSRR = (1u << (STARTER_PIN + 16u));          /* off (low) */
    STARTER_PORT->OTYPER  &= ~(1u << STARTER_PIN);
    STARTER_PORT->PUPDR   &= ~(3u << (2u * STARTER_PIN));
    STARTER_PORT->OSPEEDR &= ~(3u << (2u * STARTER_PIN));
    STARTER_PORT->MODER    = (STARTER_PORT->MODER & ~(3u << (2u * STARTER_PIN)))
                           | (1u << (2u * STARTER_PIN));
}
static void starter_drive(bool on)
{
    STARTER_PORT->BSRR = on ? (1u << STARTER_PIN) : (1u << (STARTER_PIN + 16u));
}

static void remote_start_apply(remote_start_event_t ev)
{
    char s[64];
    switch (ev) {
    case RS_EV_CRANK_START: emit("START,CRANK\r\n"); break;
    case RS_EV_STARTED:
        (void)snprintf(s, sizeof s, "START,STARTED,%lu\r\n",
                       (unsigned long)remote_start_crank_elapsed_ms(&rs, system_millis)); emit(s); break;
    case RS_EV_TIMEOUT:  emit("START,TIMEOUT\r\n"); break;
    case RS_EV_ABORTED:  emit("START,ABORTED\r\n"); break;
    case RS_EV_BLOCKED:
        (void)snprintf(s, sizeof s, "START,BLOCKED,%s\r\n",
                       remote_start_block_reason_name(remote_start_block_reason(&rs))); emit(s); break;
    default: break;
    }
    starter_drive(remote_start_starter_engaged(&rs));
}

/* crank CAN frame (optional, once the user reverse-engineers the BCM start msg) */
static void start_crank_can(void)
{
    if (!start_frame_set || !remote_start_starter_engaged(&rs)) return;
    can_ctrl_t *c = &s_can[start_frame_bus];
    if (!can_hw_tx_window_open(c, system_millis) || c->tx_tokens == 0u) return;
    (void)can_hw_load_mailbox(c, start_frame_id, start_frame_ext, false,
                              start_frame_data, start_frame_dlc, true);
}

static bool veh_key_on(void);
static bool veh_parked(void);

/* ── write mode: one switch for every transmitting protocol ───────────────
 * WRITE,ON keeps both buses armed (bxCAN Normal + TJA1051 S LOW) until
 * WRITE,OFF; read-only (SILM + S HIGH) is the boot default. */
static bool write_mode;
static uint32_t write_retry_ms;
static void write_service(void)
{
    if (!write_mode) return;
    for (uint32_t i = 0u; i < 2u; i++) {
        can_ctrl_t *c = &s_can[i];
        if (c->tx_armed) {
            c->tx_armed_until_ms = system_millis + TX_ARM_MAX_SECONDS * 1000u;
            if (c->tx_armed_until_ms == 0u) c->tx_armed_until_ms = 1u;
        } else if (c->can_locked && veh_key_on() &&
                   (uint32_t)(system_millis - write_retry_ms) >= 1000u) {
            write_retry_ms = system_millis;
            const char *err = can_hw_tx_arm(c, TX_ARM_MAX_SECONDS, system_millis, clock_ok);
            char s[56];
            if (err) (void)snprintf(s, sizeof s, "STATUS,%s,WRITE,WAIT,%s\r\n", c->cfg->label, err);
            else     (void)snprintf(s, sizeof s, "STATUS,%s,ARMED,WRITE\r\n", c->cfg->label);
            emit(s);
        }
    }
}

static void report_opts(const can_ctrl_t *c)
{
    const can_opts_t *o = &c->opts;
    char s[112];
    (void)snprintf(s, sizeof s, "STATUS,%s,OPTS,ABOM=%u,AWUM=%u,NART=%u,RFLM=%u,TXFP=%u,TTCM=%u,DBF=%u,SJW=%u\r\n",
                   c->cfg->label, o->abom, o->awum, o->nart, o->rflm, o->txfp, o->ttcm, o->dbf, o->sjw);
    emit(s);
}

static void report_err(const can_ctrl_t *c)
{
    const uint32_t esr = c->cfg->can->ESR;
    char s[112];
    (void)snprintf(s, sizeof s, "STATUS,%s,ERRSTATE,%s,TEC=%lu,REC=%lu,LEC=%s,FOVR0=%lu,FOVR1=%lu,WAKE=%lu\r\n",
                   c->cfg->label, can_error_state(esr), (unsigned long)((esr >> 16) & 0xFFu),
                   (unsigned long)(esr >> 24), can_lec_name((esr >> 4) & 7u),
                   (unsigned long)c->fovr_count[0], (unsigned long)c->fovr_count[1],
                   (unsigned long)c->wakeups);
    emit(s);
}

/* bxCAN ability commands. Returns true when cmd was handled. */
static bool can_ability_command(const char *cmd, char **save)
{
    if (strcmp(cmd, "WRITE") == 0) {
        char *a = strtok_r(NULL, ",", save);
        if (a && strcmp(a, "ON") == 0) {
            write_mode = true; write_retry_ms = system_millis - 1000u;
            emit("STATUS,SYS,WRITE,ON\r\n");
            write_service();
        } else if (a && strcmp(a, "OFF") == 0) {
            write_mode = false;
            for (uint32_t i = 0u; i < 2u; i++) if (s_can[i].tx_armed) disarm(&s_can[i], "WRITE_OFF");
            emit("STATUS,SYS,WRITE,OFF\r\n");
        } else if (a && strcmp(a, "?") == 0) {
            emit(write_mode ? "STATUS,SYS,WRITE,ON\r\n" : "STATUS,SYS,WRITE,OFF\r\n");
        } else emit("STATUS,ERR,BAD_WRITE\r\n");
        return true;
    }
    if (strcmp(cmd, "OPT") == 0) {   /* OPT,<bus>[,<NAME>,<value>] */
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", save));
        char *n = strtok_r(NULL, ",", save), *v = strtok_r(NULL, ",", save);
        if (!c) { emit("STATUS,ERR,BAD_OPT\r\n"); return true; }
        if (n) {
            can_opts_t o = c->opts;
            if (!v || !can_opts_set(&o, n, v)) { emit("STATUS,ERR,BAD_OPT\r\n"); return true; }
            c->opts = o;
            if (c->current_bitrate && !can_hw_apply_opts(c)) {
                char s[48]; (void)snprintf(s, sizeof s, "STATUS,%s,ERR,OPT_APPLY\r\n", c->cfg->label); emit(s);
                return true;
            }
        }
        report_opts(c);
        return true;
    }
    if (strcmp(cmd, "FILTER") == 0) {
        /* FILTER,<bus>,<bank>,<M|L>,<32|16>,<fifo 0|1>,<fr1 hex>,<fr2 hex>
         * FILTER,<bus>,<bank>,OFF     FILTER,<bus>,DEFAULT */
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", save));
        char *b = strtok_r(NULL, ",", save);
        if (!c || !b) { emit("STATUS,ERR,BAD_FILTER\r\n"); return true; }
        if (strcmp(b, "DEFAULT") == 0) {
            can_filter_regs_t r; const uint8_t fb = c->cfg->filter_bank;
            can_filter_spec_t sf = can_filter_accept_format(fb, false, false);
            bool ok = can_filter_compile(&sf, &r) && can_hw_filter_program(c, &r);
            sf = can_filter_accept_format((uint8_t)(fb + 1u), true, true);
            ok = ok && can_filter_compile(&sf, &r) && can_hw_filter_program(c, &r);
            char s[48]; (void)snprintf(s, sizeof s, "STATUS,%s,FILTER,DEFAULT,%s\r\n", c->cfg->label, ok ? "OK" : "ERR"); emit(s);
            return true;
        }
        uint32_t bank;
        if (!parse_u32(b, 10, 27u, &bank)) { emit("STATUS,ERR,BAD_FILTER\r\n"); return true; }
        char *m = strtok_r(NULL, ",", save);
        if (m && strcmp(m, "OFF") == 0) {
            const bool ok = can_hw_filter_disable(c, (uint8_t)bank);
            char s[48]; (void)snprintf(s, sizeof s, "STATUS,%s,FILTER,%lu,%s\r\n", c->cfg->label, (unsigned long)bank, ok ? "OFF" : "ERR"); emit(s);
            return true;
        }
        char *sc = strtok_r(NULL, ",", save), *ff = strtok_r(NULL, ",", save),
             *r1 = strtok_r(NULL, ",", save), *r2 = strtok_r(NULL, ",", save);
        uint32_t fifo, fr1, fr2;
        if (!m || !sc || (strcmp(m, "M") && strcmp(m, "L")) || (strcmp(sc, "32") && strcmp(sc, "16")) ||
            !parse_u32(ff, 10, 1u, &fifo) || !parse_u32(r1, 16, 0xFFFFFFFFu, &fr1) ||
            !parse_u32(r2, 16, 0xFFFFFFFFu, &fr2)) { emit("STATUS,ERR,BAD_FILTER\r\n"); return true; }
        can_filter_regs_t r = { .bank = (uint8_t)bank, .fr1 = fr1, .fr2 = fr2,
                                .mask_mode = m[0] == 'M', .scale32 = sc[0] == '3',
                                .fifo1 = fifo == 1u, .active = true };
        const bool ok = can_hw_filter_program(c, &r);
        char s[48]; (void)snprintf(s, sizeof s, "STATUS,%s,FILTER,%lu,%s\r\n", c->cfg->label, (unsigned long)bank, ok ? "OK" : "NOT_OWNED"); emit(s);
        return true;
    }
    if (strcmp(cmd, "CAN2SB") == 0) {
        uint32_t sb; char *a = strtok_r(NULL, ",", save);
        if (a && strcmp(a, "?") == 0) sb = can_hw_can2sb();
        else if (!parse_u32(a, 10, 27u, &sb) || !can_hw_set_can2sb((uint8_t)sb)) { emit("STATUS,ERR,BAD_CAN2SB\r\n"); return true; }
        char s[40]; (void)snprintf(s, sizeof s, "STATUS,SYS,CAN2SB,%lu\r\n", (unsigned long)sb); emit(s);
        return true;
    }
    if (strcmp(cmd, "ERRSTATE") == 0) {
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", save));
        if (c) report_err(c); else emit("STATUS,ERR,BAD_BUS\r\n");
        return true;
    }
    if (strcmp(cmd, "BUSOFF") == 0) {   /* BUSOFF,<bus>,RECOVER */
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", save));
        char *a = strtok_r(NULL, ",", save);
        if (!c || !a || strcmp(a, "RECOVER")) { emit("STATUS,ERR,BAD_BUSOFF\r\n"); return true; }
        const bool ok = can_hw_busoff_recover(c);
        char s[48]; (void)snprintf(s, sizeof s, "STATUS,%s,BUSOFF,%s\r\n", c->cfg->label, ok ? "RECOVERED" : "ERR"); emit(s);
        return true;
    }
    if (strcmp(cmd, "SLEEP") == 0 || strcmp(cmd, "WAKE") == 0) {
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", save));
        if (!c) { emit("STATUS,ERR,BAD_BUS\r\n"); return true; }
        const bool sl = cmd[0] == 'S';
        const bool ok = sl ? can_hw_sleep(c) : can_hw_wake(c);
        char s[48]; (void)snprintf(s, sizeof s, "STATUS,%s,%s,%s\r\n", c->cfg->label, sl ? "SLEEP" : "WAKE", ok ? "OK" : "ERR"); emit(s);
        return true;
    }
    return false;
}

/* ── connected-device identification / target selection ───────────────── */
#ifdef NET_DASHBOARD
static bool dev_append(uint8_t type, uint32_t id, const void *data, uint16_t len)
{ const bool ok = evst_append(type, id, data, len); dash_identity_changed(); return ok; }
#else
static bool dev_append(uint8_t type, uint32_t id, const void *data, uint16_t len)
{ (void)type; (void)id; (void)data; (void)len; return false; }
#endif

static void dev_announce(void)
{
    dev_db_t *db = dev_global();
    char s[112];
    const dev_profile_t *p = dev_find(db, db->target);
    if (p) (void)snprintf(s, sizeof s, "DEVICE,TARGET,%lu,%s\r\n", (unsigned long)p->pid, p->name);
    else   (void)snprintf(s, sizeof s, "DEVICE,DETECTING\r\n");
    emit(s);
}

static void dev_service(void)
{
    dev_db_t *db = dev_global();
    const dev_event_t ev = dev_step(db, system_millis);
    char s[112];
    if (ev == DEV_EV_DETECTED) {
        const dev_profile_t *p = dev_find(db, db->best_pid);
        (void)snprintf(s, sizeof s, "DEVICE,DETECTED,%lu,%s,%u\r\n", (unsigned long)db->best_pid,
                       p ? p->name : "?", (unsigned)db->best_score);
        emit(s);
        if (p && (p->flags & DEV_FLAG_SEED)) (void)dev_persist_profile(db, p->pid, dev_append);
    } else if (ev == DEV_EV_NEW) {
        char nm[DEV_NAME_MAX + 24u]; dev_suggest_name(db, nm, sizeof nm);
        (void)snprintf(s, sizeof s, "DEVICE,NEW,%s,%u\r\n", nm, (unsigned)db->best_score);
        emit(s);
    }
}

static bool device_command(const char *cmd, char **save)
{
    if (strcmp(cmd, "DEVICE") != 0) return false;
    dev_db_t *db = dev_global();
    char *a = strtok_r(NULL, ",", save);
    char s[112];
    if (!a) { emit("DEVICE,ERR,BAD_REQUEST\r\n"); return true; }
    if (strcmp(a, "STATUS") == 0) { dev_announce(); return true; }
    if (strcmp(a, "LIST") == 0) {
        for (uint8_t i = 0u; i < db->count; i++) {
            const dev_profile_t *p = &db->prof[i];
            (void)snprintf(s, sizeof s, "DEVICE,ITEM,%lu,%s,%lu,%lu,%u,%s\r\n", (unsigned long)p->pid, p->name,
                           (unsigned long)p->bitrate[0], (unsigned long)p->bitrate[1], (unsigned)p->nids,
                           (p->flags & DEV_FLAG_VERIFIED) ? "VERIFIED" : "SEED");
            emit(s);
        }
        emit("DEVICE,LIST,END\r\n");
        return true;
    }
    if (strcmp(a, "OBSERVE") == 0) {   /* re-run identification now */
        dev_observe_start(db, system_millis);
        for (uint8_t b = 0u; b < 2u; b++) dev_set_bitrate(db, b, s_can[b].can_locked ? s_can[b].current_bitrate : 0u);
        emit("DEVICE,OBSERVING\r\n");
        return true;
    }
    if (strcmp(a, "DETECT") == 0) {    /* add/detect new device: detect on every boot */
        dev_detect_mode(db);
        (void)dev_persist_target(db, dev_append);
        dev_observe_start(db, system_millis);
        for (uint8_t b = 0u; b < 2u; b++) dev_set_bitrate(db, b, s_can[b].can_locked ? s_can[b].current_bitrate : 0u);
        dev_announce();
        return true;
    }
    if (strcmp(a, "SELECT") == 0) {
        uint32_t pid;
        if (!parse_u32(strtok_r(NULL, ",", save), 10, 0xFFFFFFFFu, &pid) || !dev_select(db, pid)) {
            emit("DEVICE,ERR,NOT_FOUND\r\n"); return true; }
        const bool ok = dev_persist_target(db, dev_append);
        dev_announce();
        if (!ok) emit("DEVICE,ERR,SAVE_FAILED\r\n");
        return true;
    }
    if (strcmp(a, "ADD") == 0) {
        uint32_t pid = 0u;
        const dev_add_t r = dev_add(db, strtok_r(NULL, ",", save), &pid);
        const dev_profile_t *p = dev_find(db, pid);
        if (r == DEV_ADD_OK) {
            const bool ok = dev_persist_profile(db, pid, dev_append);
            (void)snprintf(s, sizeof s, "DEVICE,ADDED,%lu,%s\r\n", (unsigned long)pid, p ? p->name : "?");
            emit(s);
            if (!ok) emit("DEVICE,ERR,SAVE_FAILED\r\n");
        } else if (r == DEV_ADD_EXISTS) {
            (void)snprintf(s, sizeof s, "DEVICE,EXISTS,%lu,%s\r\n", (unsigned long)pid, p ? p->name : "?");
            emit(s);
        } else {
            emit(r == DEV_ADD_FULL ? "DEVICE,ERR,FULL\r\n" : r == DEV_ADD_NO_DATA ? "DEVICE,ERR,NO_TRAFFIC\r\n"
                                   : "DEVICE,ERR,BAD_NAME\r\n");
        }
        return true;
    }
    if (strcmp(a, "RENAME") == 0) {
        uint32_t pid;
        if (!parse_u32(strtok_r(NULL, ",", save), 10, 0xFFFFFFFFu, &pid) ||
            !dev_rename(db, pid, strtok_r(NULL, ",", save))) { emit("DEVICE,ERR,BAD_RENAME\r\n"); return true; }
        if (!dev_persist_profile(db, pid, dev_append)) emit("DEVICE,ERR,SAVE_FAILED\r\n");
        else emit("DEVICE,RENAMED\r\n");
        return true;
    }
    emit("DEVICE,ERR,BAD_REQUEST\r\n");
    return true;
}

/* ── host command parser ────────────────────────────────────────────────── */
static void host_command(char *lb)
{
    char *save = NULL;
    char *cmd = strtok_r(lb, ",", &save);
    if (!cmd) return;
    if (can_ability_command(cmd, &save)) return;
    if (device_command(cmd, &save)) return;

    if (strcmp(cmd, "ARM") == 0) {
        if (!veh_key_on()) { emit("TX,ERR,KEY_NOT_ON\r\n"); return; }
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", &save));
        uint32_t secs; char *a = strtok_r(NULL, ",", &save);
        if (!c || !parse_u32(a, 10, TX_ARM_MAX_SECONDS, &secs) || secs == 0u ||
            strtok_r(NULL, ",", &save)) { emit("TX,ERR,BAD_ARM\r\n"); return; }
        const char *err = can_hw_tx_arm(c, secs, system_millis, clock_ok);
        if (err) { char s[48]; (void)snprintf(s, sizeof s, "TX,%s,ERR,%s\r\n", c->cfg->label, err); emit(s); }
        else { char s[32]; (void)snprintf(s, sizeof s, "STATUS,%s,ARMED,%lu\r\n", c->cfg->label, (unsigned long)secs); emit(s); }
        return;
    }
    if (strcmp(cmd, "DISARM") == 0) {
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", &save));
        if (c) disarm(c, "COMMAND");
        return;
    }
    if (strcmp(cmd, "MODE") == 0) {
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", &save));
        char *m = strtok_r(NULL, ",", &save);
        if (!c || !m) { emit("STATUS,ERR,BAD_MODE\r\n"); return; }
        can_mode_t md;
        if      (strcmp(m,"SILENT")==0)         md = CAN_MODE_SILENT;
        else if (strcmp(m,"NORMAL")==0)         md = CAN_MODE_NORMAL;
        else if (strcmp(m,"LOOPBACK")==0)       md = CAN_MODE_LOOPBACK;
        else if (strcmp(m,"LOOPBACKSILENT")==0) md = CAN_MODE_LOOPBACK_SILENT;
        else { emit("STATUS,ERR,BAD_MODE\r\n"); return; }
        if (md == CAN_MODE_NORMAL && !c->tx_armed) {   /* loopback: S stays HIGH */
            char s[48]; (void)snprintf(s,sizeof s,"STATUS,%s,MODE,REQUIRES_ARM\r\n",c->cfg->label); emit(s); return; }
        if (can_hw_set_controller_mode(c, md)) {
            char s[48]; (void)snprintf(s,sizeof s,"STATUS,%s,MODE,%s\r\n",c->cfg->label,m); emit(s);
        }
        return;
    }
    if (strcmp(cmd, "BITRATE") == 0) {
        if (veh_parked()) { emit("STATUS,ERR,KEY_OFF\r\n"); return; }
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", &save));
        char *b = strtok_r(NULL, ",", &save); uint32_t hz;
        if (!c || !b) { emit("STATUS,ERR,BAD_BITRATE\r\n"); return; }
        if (c->tx_armed) {
            char s[48]; (void)snprintf(s, sizeof s, "STATUS,%s,ERR,DISARM_REQUIRED\r\n", c->cfg->label);
            emit(s); return;
        }
        if (strcmp(b,"AUTO")==0) { can_hw_start_passive(c, system_millis);
            char s[48]; (void)snprintf(s,sizeof s,"STATUS,%s,SCAN,%lu\r\n",c->cfg->label,(unsigned long)c->current_bitrate); emit(s); return; }
        if (parse_u32(b,10,1000000u,&hz) && can_hw_set_bitrate(c, hz, CAN_MODE_SILENT)) {
            c->can_locked = 1u;
            char s[48]; (void)snprintf(s,sizeof s,"STATUS,%s,BITRATE,%lu\r\n",c->cfg->label,(unsigned long)c->current_bitrate); emit(s);
        } else emit("STATUS,ERR,BAD_BITRATE\r\n");
        return;
    }
    if (strcmp(cmd, "TX") == 0) {
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", &save));
        char *fmt = strtok_r(NULL, ",", &save), *id_t = strtok_r(NULL, ",", &save),
             *rtr_t = strtok_r(NULL, ",", &save), *dlc_t = strtok_r(NULL, ",", &save),
             *hex_t = strtok_r(NULL, ",", &save);
        if (!c || !fmt || !id_t || !rtr_t || !dlc_t || !hex_t || strtok_r(NULL, ",", &save) ||
            (strcmp(fmt,"S")!=0 && strcmp(fmt,"E")!=0)) { emit("TX,ERR,BAD_FORMAT\r\n"); return; }
        tx_frame(c, fmt, id_t, rtr_t, dlc_t, hex_t); return;
    }
    if (strcmp(cmd, "UDS") == 0) {
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", &save));
        if (c) uds_command(c, &save);
        else   emit("UDS,ERR,BAD_BUS\r\n");
        return;
    }
    if (strcmp(cmd, "UDSPAD") == 0) {
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", &save));
        if (c) udspad_command(c, &save);
        else   emit("UDS,ERR,BAD_BUS\r\n");
        return;
    }
    if (strcmp(cmd, "ECUSCAN") == 0) {
        can_ctrl_t *c = sel_bus(strtok_r(NULL, ",", &save));
        if (c) {
            ecuscan_command(c, &save);
            if (ecu_scan_active(&c->ecuscan)) ecu_scan_set_skip(&c->ecuscan, ecuscan_skip, c);
        } else {
            emit("ECU,ERR,BAD_BUS\r\n");
        }
        return;
    }

    if (strcmp(cmd, "GATEWAY") == 0) {
        char *a = strtok_r(NULL, ",", &save);
        if (a && strcmp(a,"ON")==0)  { gateway_on = true;  emit("STATUS,SYS,GATEWAY,ON\r\n"); }
        else if (a && strcmp(a,"OFF")==0) { gateway_on = false; emit("STATUS,SYS,GATEWAY,OFF\r\n"); }
        else emit("STATUS,ERR,BAD_GATEWAY\r\n");
        return;
    }
    if (strcmp(cmd, "PERIODIC") == 0) {
        uint32_t slot;
        char *slot_t = strtok_r(NULL, ",", &save);
        if (!parse_u32(slot_t, 10, PERIODIC_SLOTS-1u, &slot)) { emit("STATUS,ERR,BAD_PERIODIC\r\n"); return; }
        char *a = strtok_r(NULL, ",", &save);
        if (a && strcmp(a,"OFF")==0) { periodic[slot].active=false;
            char s[32]; (void)snprintf(s,sizeof s,"STATUS,SYS,PERIODIC,%lu,OFF\r\n",(unsigned long)slot); emit(s); return; }
        can_ctrl_t *c = sel_bus(a);
        char *fmt=strtok_r(NULL,",",&save),*id_t=strtok_r(NULL,",",&save),
             *dlc_t=strtok_r(NULL,",",&save),*hex_t=strtok_r(NULL,",",&save),*per_t=strtok_r(NULL,",",&save);
        uint32_t id,dlc,per;
        const bool ext = fmt && strcmp(fmt,"E")==0;
        if (!c || !fmt || (strcmp(fmt,"S")!=0 && !ext) || !id_t || !dlc_t || !hex_t || !per_t ||
            !parse_u32(id_t,16,ext?0x1FFFFFFFu:0x7FFu,&id) || !parse_u32(dlc_t,10,8u,&dlc) ||
            !parse_u32(per_t,10,60000u,&per) || per==0u ||
            (dlc==0u ? strcmp(hex_t,"-")!=0 : strlen(hex_t)!=dlc*2u)) { emit("STATUS,ERR,BAD_PERIODIC\r\n"); return; }
        uint8_t data[8]={0}; uint16_t n=0;
        if (dlc>0u && !hex_to_bytes(hex_t,data,8,&n)) { emit("STATUS,ERR,BAD_PERIODIC\r\n"); return; }
        periodic[slot].active=true; periodic[slot].bus=ctrl_index(c); periodic[slot].ext=ext;
        periodic[slot].rtr=false; periodic[slot].id=id; periodic[slot].dlc=(uint8_t)dlc;
        periodic[slot].period_ms=per; periodic[slot].next_ms=system_millis+per;
        memcpy(periodic[slot].data,data,8);
        char s[40]; (void)snprintf(s,sizeof s,"STATUS,SYS,PERIODIC,%lu,ON\r\n",(unsigned long)slot); emit(s);
        return;
    }

    if (strcmp(cmd, "STARTCFG") == 0) {
        char *to=strtok_r(NULL,",",&save),*ra=strtok_r(NULL,",",&save),*rn=strtok_r(NULL,",",&save),
             *rb=strtok_r(NULL,",",&save),*ri=strtok_r(NULL,",",&save),*rp=strtok_r(NULL,",",&save);
        uint32_t t,a1,n1,b1,i1,p1;
        if (!parse_u32(to,10,10000u,&t)||!parse_u32(ra,10,1u,&a1)||!parse_u32(rn,10,1u,&n1)||
            !parse_u32(rb,10,1u,&b1)||!parse_u32(ri,10,1u,&i1)||!parse_u32(rp,10,20000u,&p1)) {
            emit("START,ERR,BAD_CFG\r\n"); return; }
        remote_start_cfg_t cfg = { .timeout_ms=t, .rpm_running_threshold=(uint16_t)p1,
            .min_crank_ms=RS_MIN_CRANK_DEFAULT_MS, .require_armed=a1!=0u,
            .require_neutral_or_park=n1!=0u, .require_brake=b1!=0u, .require_immobilizer_ok=i1!=0u };
        remote_start_set_cfg(&rs, &cfg); emit("START,CFG,OK\r\n"); return;
    }
    if (strcmp(cmd, "STARTINPUTS") == 0) {
        char *n=strtok_r(NULL,",",&save),*b=strtok_r(NULL,",",&save),*i=strtok_r(NULL,",",&save);
        uint32_t n1,b1,i1;
        if (!parse_u32(n,10,1u,&n1)||!parse_u32(b,10,1u,&b1)||!parse_u32(i,10,1u,&i1)) { emit("START,ERR,BAD_INPUTS\r\n"); return; }
        rs_in.in_neutral_or_park=n1!=0u; rs_in.brake_pressed=b1!=0u; rs_in.immobilizer_ok=i1!=0u;
        emit("START,INPUTS,OK\r\n"); return;
    }
    if (strcmp(cmd, "STARTFRAME") == 0) {
        char *a = strtok_r(NULL, ",", &save);
        if (a && strcmp(a,"NONE")==0) { start_frame_set=false; emit("START,FRAME,NONE\r\n"); return; }
        can_ctrl_t *c = sel_bus(a);
        char *fmt=strtok_r(NULL,",",&save),*id_t=strtok_r(NULL,",",&save),
             *dlc_t=strtok_r(NULL,",",&save),*hex_t=strtok_r(NULL,",",&save);
        uint32_t id,dlc; const bool ext = fmt && strcmp(fmt,"E")==0;
        if (!c||!fmt||(strcmp(fmt,"S")!=0&&!ext)||!id_t||!dlc_t||!hex_t||
            !parse_u32(id_t,16,ext?0x1FFFFFFFu:0x7FFu,&id)||!parse_u32(dlc_t,10,8u,&dlc)||
            strlen(hex_t)!=dlc*2u) { emit("START,ERR,BAD_FRAME\r\n"); return; }
        uint8_t data[8]={0}; uint16_t n=0;
        if (dlc>0u && !hex_to_bytes(hex_t,data,8,&n)) { emit("START,ERR,BAD_FRAME\r\n"); return; }
        start_frame_set=true; start_frame_bus=ctrl_index(c); start_frame_ext=ext;
        start_frame_id=id; start_frame_dlc=(uint8_t)dlc; memcpy(start_frame_data,data,8);
        emit("START,FRAME,OK\r\n"); return;
    }
    if (strcmp(cmd, "START") == 0) {
        /* key must be at II (ON): F-CAN alive. Key I / OFF never cranks. */
        if (!veh_key_on()) { emit("START,BLOCKED,KEY_NOT_ON\r\n"); return; }
        rs_in.armed = s_can[0].tx_armed;   /* require_armed checks F-CAN arm */
        remote_start_apply(remote_start_request(&rs, &rs_in, system_millis));
        return;
    }
    if (strcmp(cmd, "STARTCANCEL") == 0) {
        remote_start_cancel(&rs, system_millis); remote_start_apply(RS_EV_ABORTED); return;
    }
    if (strcmp(cmd, "GEARBCAST") == 0) {
        char *a = strtok_r(NULL, ",", &save);
        if (a && strcmp(a, "ON") == 0)       { gear_bcast_on = true;  emit("GEAR,BCAST,ON\r\n"); }
        else if (a && strcmp(a, "OFF") == 0) { gear_bcast_on = false; gear_bcast_same = 0u; gear_bcast_last = 0xFFu; emit("GEAR,BCAST,OFF\r\n"); }
        else emit("GEAR,ERR,BAD_BCAST\r\n");
        return;
    }
    if (strcmp(cmd, "GEAR") == 0) {
        /* GEAR,<raw 0..15>  decoded gear from the dashboard (Honda DID 2612).
         * GEAR,CLEAR        forget the known gear.  Only drives LEDs: never
         * touches the buses, so it is accepted in any key state. */
        char *a = strtok_r(NULL, ",", &save);
        if (a && strcmp(a, "CLEAR") == 0) { gear_led_clear(&gear_led); emit("GEAR,CLEAR\r\n"); return; }
        uint32_t gr;
        if (!parse_u32(a, 10, GEAR_LED_GEAR_MAX, &gr)) { emit("GEAR,ERR,BAD_GEAR\r\n"); return; }
        const uint8_t prev = gear_led.known ? gear_led.gear : 0xFFu;
        const gear_led_event_t ev = gear_led_report(&gear_led, (uint8_t)gr, system_millis);
        char s[48];
        (void)snprintf(s, sizeof s, "GEAR,ACK,%u,%lu,%s\r\n",
                       (unsigned)prev, (unsigned long)gr, gear_led_event_name(ev));
        emit(s);
        return;
    }

    emit("CMD,ERR,UNKNOWN\r\n");
}

#ifdef NET_DASHBOARD
/* Dashboard commands (WebSocket) enter here. */
static void dash_command(const char *line)
{
    static char lb[HOST_LINE_CAPACITY];
    const size_t n = strlen(line);
    if (n == 0u) return;
    if (n + 1u > sizeof lb) { emit("CMD,ERR,LINE_TOO_LONG\r\n"); return; }
    memcpy(lb, line, n + 1u);
    host_command(lb);
}
#endif


#ifdef NET_DASHBOARD
bool project_bus_state(uint8_t bus, uint32_t *bitrate, bool *listen_only)
{
    if (bus > 1u) return false;
    const can_ctrl_t *c = &s_can[bus];
    *bitrate = c->current_bitrate;
    *listen_only = c->mode != CAN_MODE_NORMAL;
    return c->current_bitrate != 0u && !c->asleep;
}
#endif

/* ── CAN interrupt handlers ─────────────────────────────────────────────── */
void CAN1_RX0_IRQHandler(void) { can_hw_rx_drain(&s_can[0], &capture_queue[0], 0u, system_millis); }
void CAN1_RX1_IRQHandler(void) { can_hw_rx_drain(&s_can[0], &capture_queue[0], 1u, system_millis); }
void CAN1_SCE_IRQHandler(void) { can_hw_sce(&s_can[0]); }
void CAN1_TX_IRQHandler(void)  { can_hw_tx_isr(&s_can[0]); }
void CAN2_RX0_IRQHandler(void) { can_hw_rx_drain(&s_can[1], &capture_queue[1], 0u, system_millis); }
void CAN2_RX1_IRQHandler(void) { can_hw_rx_drain(&s_can[1], &capture_queue[1], 1u, system_millis); }
void CAN2_SCE_IRQHandler(void) { can_hw_sce(&s_can[1]); }
void CAN2_TX_IRQHandler(void)  { can_hw_tx_isr(&s_can[1]); }

/* ── watchdog / reset cause (unchanged) ─────────────────────────────────── */
static bool iwdg_start(void)
{
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP;
    IWDG->KR = 0xCCCCu; IWDG->KR = 0x5555u; IWDG->PR = IWDG_PR_DIV64; IWDG->RLR = IWDG_RELOAD;
    const uint32_t t0 = system_millis;
    while ((IWDG->SR & (IWDG_SR_PVU | IWDG_SR_RVU)) != 0u)
        if ((uint32_t)(system_millis - t0) > IWDG_SR_WAIT_MS) return false;
    IWDG->KR = 0xAAAAu; return true;
}
static inline void iwdg_feed(void) { IWDG->KR = 0xAAAAu; }

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

/* CS43L22 audio codec is unused: hold its reset (PD4) low so it stays idle. */
static void audio_codec_hold_reset_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;
    GPIOD->BSRR = (1u << (4u + 16u)); /* drive Audio_RST / PD4 low first */
    GPIOD->OTYPER &= ~(1u << 4u);
    GPIOD->PUPDR &= ~(3u << (2u * 4u));
    GPIOD->OSPEEDR &= ~(3u << (2u * 4u));
    GPIOD->MODER = (GPIOD->MODER & ~(3u << (2u * 4u))) | (1u << (2u * 4u));
}

/* ── MB997 user LEDs: LD3=PD13, LD4=PD12, LD5=PD14, LD6=PD15 ────────── */
#define LED_GREEN 12u  /* LD4: main-loop / frame-processing throughput */
#define LED_ORANGE 13u /* LD3: telemetry line queued to ESP32 UART DMA */
#define LED_RED 14u    /* LD5: STM32 VDD below safe warning threshold */
#define LED_BLUE 15u   /* LD6: CAN receive traffic-rate indicator */
#define VDD_WARN_MV 2900u
#define VDD_HYST_MV 100u
static void leds_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; (void)RCC->AHB1ENR;
    const uint32_t pins = (1u<<LED_GREEN)|(1u<<LED_ORANGE)|(1u<<LED_RED)|(1u<<LED_BLUE);
    GPIOD->BSRR = pins << 16u;
    GPIOD->OTYPER &= ~pins;
    GPIOD->PUPDR  &= ~((3u<<(2u*LED_GREEN))|(3u<<(2u*LED_ORANGE))|(3u<<(2u*LED_RED))|(3u<<(2u*LED_BLUE)));
    GPIOD->OSPEEDR &= ~((3u<<(2u*LED_GREEN))|(3u<<(2u*LED_ORANGE))|(3u<<(2u*LED_RED))|(3u<<(2u*LED_BLUE)));
    GPIOD->MODER = (GPIOD->MODER & ~((3u<<(2u*LED_GREEN))|(3u<<(2u*LED_ORANGE))|(3u<<(2u*LED_RED))|(3u<<(2u*LED_BLUE))))
                 | (1u<<(2u*LED_GREEN))|(1u<<(2u*LED_ORANGE))|(1u<<(2u*LED_RED))|(1u<<(2u*LED_BLUE));
}
static void led_set(uint32_t pin, bool on) { GPIOD->BSRR = on ? (1u<<pin) : (1u<<(pin+16u)); }

/* STM32F407 VREFINT is approximately 1.21 V. This gives a useful VDD
 * warning estimate without an external divider; it is not a precision
 * calibrated voltmeter. ADC channel 17 is the internal reference. */
static void vdd_monitor_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    (void)RCC->APB2ENR;
    ADC->CCR |= ADC_CCR_TSVREFE;
    ADC1->CR1 = 0u;
    ADC1->CR2 = ADC_CR2_ADON;
    ADC1->SMPR1 = (ADC1->SMPR1 & ~(7u << 21u)) | (7u << 21u); /* CH17: 480 cycles */
    ADC1->SQR1 = 0u;
    ADC1->SQR2 = 0u;
    ADC1->SQR3 = 17u;
}
static void vdd_monitor_update(uint32_t now)
{
    static uint32_t last_sample;
    if ((uint32_t)(now - last_sample) < 250u) return;
    last_sample = now;
    ADC1->CR2 |= ADC_CR2_SWSTART;
    uint32_t timeout = 100000u;
    while (!(ADC1->SR & ADC_SR_EOC) && --timeout) { }
    if (!timeout) return;
    const uint32_t raw = ADC1->DR & 0x0FFFu;
    if (raw == 0u) return;
    measured_vdd_mv = (1210u * 4095u + raw / 2u) / raw;
    if (!vdd_low && measured_vdd_mv < VDD_WARN_MV) vdd_low = true;
    else if (vdd_low && measured_vdd_mv > VDD_WARN_MV + VDD_HYST_MV) vdd_low = false;
}

static uint32_t visible_blink_period(uint32_t frames_per_second)
{
    if (frames_per_second == 0u) return 0u;
    uint32_t p = 2000u / frames_per_second; /* one visible on/off cycle per traffic group */
    if (p < 240u) p = 240u; /* prevent imperceptible high-speed flicker */
    if (p > 1000u) p = 1000u;
    return p;
}
static void leds_update(void)
{
    static uint32_t last_sample_ms, last_rx[2], last_processed;
    static uint32_t can_fps, cpu_fps;
    const uint32_t now = system_millis;
    if ((uint32_t)(now - last_sample_ms) >= 250u) {
        const uint32_t elapsed = (uint32_t)(now - last_sample_ms);
        const uint32_t rx0 = s_can[0].rx_frame_count, rx1 = s_can[1].rx_frame_count;
        const uint32_t rx_delta = (rx0 - last_rx[0]) + (rx1 - last_rx[1]);
        const uint32_t proc = processed_can_frames;
        can_fps = (rx_delta * 1000u) / elapsed;
        cpu_fps = ((proc - last_processed) * 1000u) / elapsed;
        last_rx[0] = rx0; last_rx[1] = rx1; last_processed = proc;
        last_sample_ms = now;
    }
    vdd_monitor_update(now);

    /* Gear-shift chase owns all four LEDs for GEAR_LED_TOTAL_MS after a
     * decoded transmission shift (light -> dark on upshift, reverse on
     * downshift, Fourier-series brightness envelope). The health indicators
     * below resume automatically when the chase ends. Mask bit order is the
     * light-to-dark order: 0 green LD4, 1 orange LD3, 2 red LD5, 3 blue LD6. */
    if (gear_led_active(&gear_led, now)) {
        const uint8_t m = gear_led_mask(&gear_led, now);
        led_set(LED_GREEN,  (m & 1u) != 0u);
        led_set(LED_ORANGE, (m & 2u) != 0u);
        led_set(LED_RED,    (m & 4u) != 0u);
        led_set(LED_BLUE,   (m & 8u) != 0u);
        return;
    }

    /* LD6: CAN bus receive activity. Faster frame rates produce faster
     * blinking, with a 240 ms minimum cycle so activity remains visible. */
    const uint32_t traffic_period = visible_blink_period(can_fps);
    const bool traffic_recent = (uint32_t)(now - s_can[0].isr_last_rx_ts_ms) < 500u ||
                                (uint32_t)(now - s_can[1].isr_last_rx_ts_ms) < 500u;
    led_set(LED_BLUE, traffic_period && traffic_recent && ((now % traffic_period) < (traffic_period / 2u)));

    /* LD4: main-loop processing throughput, derived from processed frames. */
    const uint32_t cpu_period = visible_blink_period(cpu_fps);
    const bool cpu_recent = (uint32_t)(now - processed_last_activity_ms) < 500u;
    led_set(LED_GREEN, cpu_period && cpu_recent && ((now % cpu_period) < (cpu_period / 2u)));

    /* LD3: each line emitted to the dashboard triggers a short pulse. */
    led_set(LED_ORANGE, (uint32_t)(now - out_last_activity_ms) < 45u && out_lines_emitted != 0u);

    /* LD5: distinctive 4 Hz warning pattern while measured MCU VDD is low. */
    led_set(LED_RED, vdd_low && ((now % 250u) < 125u));
}

/* ── per-bus periodic work: scan/lock reporting, stats, TX service ──────── */
static void bus_periodic(can_ctrl_t *c, uint32_t *last_stats_ms, uint32_t *last_dwell_ms)
{
    if (veh_parked() || !c->scan_active) return;
    const char *dr = NULL;
    if (can_hw_tx_service(c, system_millis, tx_report, &dr) && dr) disarm(c, dr);

    if (c->reacquire_requested) {
        c->reacquire_requested = 0u;
        disarm(c, "REACQUIRE");
        can_hw_seen_clear(c);
        can_profiler_init(&profiler[ctrl_index(c)]);
        can_profiler_set_bus(&profiler[ctrl_index(c)], ctrl_index(c));
        can_hw_start_passive(c, system_millis);
        char s[48]; (void)snprintf(s,sizeof s,"STATUS,%s,REACQUIRE\r\n",c->cfg->label); emit(s);
    }

    const uint32_t nd = can_hw_compute_dwell_ms(c);
    if (nd != c->current_dwell_ms) {
        c->current_dwell_ms = nd;
        if ((system_millis - *last_dwell_ms) >= 5000u) {
            *last_dwell_ms = system_millis;
            char s[48]; (void)snprintf(s,sizeof s,"STATUS,%s,DWELL,%lu\r\n",c->cfg->label,(unsigned long)nd); emit(s);
        }
    }

    /* Parked: hold on the last locked bitrate instead of cycling through wrong
     * rates (which only piles up receive errors on a silent bus). */
    if (veh_parked() && !c->can_locked && c->last_good_bitrate != 0u &&
        c->current_bitrate == c->last_good_bitrate && c->valid_frames_at_rate == 0u)
        c->scan_started_ms = system_millis;

    switch (can_hw_scan_step(c, system_millis)) {
    case CAN_SCAN_LOCKED: {
        dev_set_bitrate(dev_global(), c->cfg->bus_index, c->current_bitrate);
        char s[48]; (void)snprintf(s,sizeof s,"STATUS,%s,LOCK,%lu\r\n",c->cfg->label,(unsigned long)c->current_bitrate); emit(s);
        break; }
    case CAN_SCAN_STEPPED: {
        char s[48]; (void)snprintf(s,sizeof s,"STATUS,%s,SCAN,%lu\r\n",c->cfg->label,(unsigned long)c->current_bitrate); emit(s);
        break; }
    default: break;
    }

    if ((uint32_t)(system_millis - *last_stats_ms) >= STATS_PERIOD_MS) {
        *last_stats_ms = system_millis;
        const uint32_t esr = c->cfg->can->ESR;
        const char *st = (esr & CAN_ESR_BOFF) ? "BUS_OFF" : (esr & CAN_ESR_EPVF) ? "PASSIVE" :
                         (esr & CAN_ESR_EWGF) ? "WARNING" : "ACTIVE";
        char s[128];
        (void)snprintf(s, sizeof s, "STATUS,%s,STATS,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%s\r\n",
                       c->cfg->label,
                       (unsigned long)c->rx_frame_count,
                       (unsigned long)capture_queue[c->cfg->bus_index].dropped,
                       0ul, (unsigned long)c->can_error_count,
                       (unsigned long)c->can_lec_count,
                       (unsigned long)((esr & CAN_ESR_TEC) >> CAN_ESR_TEC_Pos),
                       (unsigned long)((esr & CAN_ESR_REC) >> CAN_ESR_REC_Pos), st);
        emit(s);
    }
}

static void periodic_tx_service(void)
{
    for (uint32_t i = 0; i < PERIODIC_SLOTS; i++) {
        if (!periodic[i].active) continue;
        if ((int32_t)(system_millis - periodic[i].next_ms) < 0) continue;
        periodic[i].next_ms = system_millis + periodic[i].period_ms;
        can_ctrl_t *c = &s_can[periodic[i].bus];
        if (!can_hw_tx_window_open(c, system_millis) || c->tx_tokens == 0u) continue;
        (void)can_hw_load_mailbox(c, periodic[i].id, periodic[i].ext, periodic[i].rtr,
                                  periodic[i].data, periodic[i].dlc, true);
    }
}

/* Extract engine RPM from an OBD-II mode-01 PID 0x0C response (7E8..7EF). */
static void rpm_from_frame(const can_capture_frame_t *f)
{
    if (f->extended || f->remote || f->dlc < 5u) return;
    if ((f->identifier & 0x7F8u) != 0x7E8u) return;   /* 0x7E8..0x7EF */
    if (f->data[1] == 0x41u && f->data[2] == 0x0Cu) {
        rs_in.rpm = (uint16_t)(((uint16_t)f->data[3] << 8 | f->data[4]) / 4u);
        rpm_last_ms = system_millis;
    }
}

/* ── vehicle power state (physical ignition inputs are authoritative) ─────
 * PE5 = ACC/key position I; PE6 = IGN2/key position II. Each input must be
 * driven by its own automotive-protected, isolated 12 V -> 3.3 V interface.
 * CAN traffic is deliberately NOT used to infer key state: body modules may
 * chatter after key-off, and a quiet bus cannot wake an unpowered scanner.
 * OFF disarms both bxCAN blocks into SILM; TJA1051 S pins go HIGH (silent).
 * The ESP32 remains alive and can reset STM32 into its ROM bootloader. */
#define ACC_PORT GPIOE
#define ACC_PIN  5u
#define VEH_UP_DEBOUNCE_MS   100u
#define VEH_OFF_DEBOUNCE_MS 2000u
#define VEH_DOWN_DEBOUNCE_MS 200u
#define VEH_RUN_RPM          400u
typedef enum { VEH_OFF = 0, VEH_ACC, VEH_ON, VEH_RUN } veh_state_t;
static const char *const VEH_NAME[] = { "OFF", "ACC", "ON", "RUN" };
static veh_state_t veh = VEH_OFF, veh_cand = VEH_OFF;
static uint32_t    veh_cand_ms, veh_emit_ms;
static uint32_t    bus_last_rx_ms[2];
/* Honda broadcast candidates (Honda "Nidec" layout as published in comma.ai
 * opendbc honda_*_common DBCs: 0x17C POWERTRAIN_DATA RPM = bytes 2-3 BE;
 * 0x158 ENGINE_DATA XMISSION_SPEED = bytes 0-1 BE x0.01 km/h). NOT yet proven
 * on this car -> not in civic_can_db.h; cross-check against OBD 01 0C/0D. */
static uint16_t bc_rpm, bc_speed_x100;
static uint32_t bc_rpm_ms;

/* Ignition inputs are ACTIVE-HIGH 3.3 V logic from separate automotive-
 * protected/isolated 12 V input stages. Never connect vehicle 12 V directly.
 * MB997 header P2 exposes PE5 and PE6; PE5 was the earlier ACC-only input. */
#define IGN2_PORT GPIOE
#define IGN2_PIN  6u
static void ignition_inputs_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; (void)RCC->AHB1ENR;
    const uint32_t mask = (3u << (2u * ACC_PIN)) | (3u << (2u * IGN2_PIN));
    ACC_PORT->MODER &= ~mask; /* PE5=ACC, PE6=IGN2: inputs */
    ACC_PORT->PUPDR = (ACC_PORT->PUPDR & ~mask) |
                      (2u << (2u * ACC_PIN)) | (2u << (2u * IGN2_PIN)); /* pull-down */
}

/* Passive gear source: F-CAN 0x1A3 GEARBOX_AUTO TRANS_TARGET_GEAR (DBC
 * 11|4@0 = byte 1 bits 3..0; Honda Nidec/ILX layout, candidate for this car).
 * Two identical consecutive frames with the key ON drive the same LED chase
 * as the GEAR command. LED-only: never transmits. GEARBCAST,ON|OFF toggles. */
static bool veh_key_on(void);
static void gear_bcast_sample(uint8_t g)
{
    if (!gear_bcast_on || !veh_key_on()) { gear_bcast_same = 0u; gear_bcast_last = 0xFFu; return; }
    if (g != gear_bcast_last) { gear_bcast_last = g; gear_bcast_same = 1u; return; }
    if (gear_bcast_same >= 2u) return;                       /* already reported */
    gear_bcast_same = 2u;
    if (gear_led.known && gear_led.gear == g) return;
    const uint8_t prev = gear_led.known ? gear_led.gear : 0xFFu;
    const gear_led_event_t ev = gear_led_report(&gear_led, g, system_millis);
    char s[48];
    (void)snprintf(s, sizeof s, "GEAR,BCAST,%u,%u,%s\r\n", (unsigned)prev, (unsigned)g, gear_led_event_name(ev));
    emit(s);
}

static void broadcast_decode(const can_capture_frame_t *f)
{
    if (f->bus != 0u || f->extended || f->remote) return;
    if (f->identifier == 0x17Cu && f->dlc >= 4u) {
        bc_rpm = (uint16_t)((uint16_t)f->data[2] << 8 | f->data[3]);
        bc_rpm_ms = system_millis;
    } else if (f->identifier == 0x158u && f->dlc >= 2u) {
        bc_speed_x100 = (uint16_t)((uint16_t)f->data[0] << 8 | f->data[1]);
    } else if (f->identifier == 0x1A3u && f->dlc >= 2u) {
        gear_bcast_sample((uint8_t)(f->data[1] & 0x0Fu));
    }
}

static void veh_update(void)
{
    const uint32_t now = system_millis;
    const bool acc  = ((ACC_PORT->IDR >> ACC_PIN) & 1u) != 0u;
    const bool ign2 = ((IGN2_PORT->IDR >> IGN2_PIN) & 1u) != 0u;
    const uint16_t rpm = rs_in.rpm_fresh ? rs_in.rpm : 0u;
    /* IGN2 is the authority for position II. ACC alone means position I.
     * CAN frames never promote a parked vehicle to ACC/ON. */
    const veh_state_t n = ign2 ? (rpm >= VEH_RUN_RPM ? VEH_RUN : VEH_ON)
                               : (acc ? VEH_ACC : VEH_OFF);
    if (n != veh_cand) { veh_cand = n; veh_cand_ms = now; }
    const uint32_t stable_ms = (veh_cand > veh) ? VEH_UP_DEBOUNCE_MS :
                               (veh_cand == VEH_OFF ? VEH_OFF_DEBOUNCE_MS : VEH_DOWN_DEBOUNCE_MS);
    const bool change = (veh_cand != veh) &&
                        ((uint32_t)(now - veh_cand_ms) >= stable_ms);
    if (change) {
        const veh_state_t old = veh;
        veh = veh_cand;
        if (old == VEH_OFF && veh != VEH_OFF) {
            /* Key moved to ACC/ON: both passive scans already run from power-up;
             * (re)start only a controller that is not listening (keeps a lock). */
            for (uint32_t i = 0; i < 2u; i++) if (!s_can[i].scan_active) can_hw_start_passive(&s_can[i], now);
            emit("STATUS,SYS,POWER,ACTIVE\r\n");
        }
        if (old >= VEH_ON && veh < VEH_ON) {
            /* Leaving position II: immediately revoke every transmit path. */
            for (uint32_t i = 0; i < 2u; i++) {
                disarm(&s_can[i], "KEY_LEFT_II");
                uds_stop(&s_can[i], "KEY_LEFT_II");
                ecuscan_stop(&s_can[i], "KEY_LEFT_II");
            }
            for (uint32_t i = 0; i < PERIODIC_SLOTS; i++) periodic[i].active = false;
            gateway_on = false;
            if (remote_start_starter_engaged(&rs)) {
                remote_start_cancel(&rs, now); remote_start_apply(RS_EV_ABORTED);
            }
        }
        if (old != VEH_OFF && veh == VEH_OFF) {
            /* Key removed/OFF: end every transmit-capable activity, but keep both
             * buses in listen-only capture until the car itself goes quiet
             * (owner requirement: capture from the first frame until locked). */
            for (uint32_t i = 0; i < 2u; i++) {
                disarm(&s_can[i], "KEY_OFF");
                uds_stop(&s_can[i], "KEY_OFF");
                ecuscan_stop(&s_can[i], "KEY_OFF");
                /* passive listening continues (bxCAN SILM: cannot ACK or transmit) */
            }
            for (uint32_t i = 0; i < PERIODIC_SLOTS; i++) periodic[i].active = false;
            gateway_on = false;
            bc_rpm_ms = 0u; rpm_last_ms = 0u; bc_speed_x100 = 0u;
            gear_led_clear(&gear_led);   /* next key-on learns the gear afresh */
            if (remote_start_starter_engaged(&rs)) {
                remote_start_cancel(&rs, now); remote_start_apply(RS_EV_ABORTED);
            }
            emit("STATUS,SYS,POWER,PARKED\r\n");
        }
    }
    if (change || (uint32_t)(now - veh_emit_ms) >= 1000u) {
        veh_emit_ms = now;
        const bool obd = rpm_last_ms != 0u && (uint32_t)(now - rpm_last_ms) < RPM_FRESH_MS;
        char s[80];
        (void)snprintf(s, sizeof s, "STATUS,SYS,VEH,%s,%u,%u,%u,%s\r\n", VEH_NAME[veh],
                       (unsigned)rpm, (unsigned)bc_speed_x100, acc ? 1u : 0u,
                       obd ? "OBD" : (rs_in.rpm_fresh ? "BCAST" : "NONE"));
        emit(s);
    }
}
static bool veh_key_on(void) { return veh >= VEH_ON; }
static bool veh_parked(void) { return veh == VEH_OFF; }

#ifdef USB_CDC_ENABLED
static bool usb_tx_cb(uint8_t bus, uint32_t id, bool ext, bool rtr,
                      const uint8_t *data, uint8_t dlc)
{
    if (bus > 1u) return false;
    can_ctrl_t *c = &s_can[bus];
    if (!can_hw_tx_window_open(c, system_millis) || c->tx_tokens == 0u) return false;
    return can_hw_load_mailbox(c, id, ext, rtr, data, dlc, true);
}
#endif

int main(void)
{
    /* Fixed wiring for the two controllers (see file header). */
    s_cfg[0] = (can_cfg_t){
        .can = CAN1, .bus_index = 0u, .gpio = GPIOD,
        .rx_pin = 0u, .tx_pin = 1u, .s_gpio = GPIOD, .s_pin = 2u, .af = 9u, .filter_bank = 0u,
        .is_can2 = false, .label = "F", .preferred_bitrate = 500000u
    };
    s_cfg[1] = (can_cfg_t){
        .can = CAN2, .bus_index = 1u, .gpio = GPIOB,
        .rx_pin = 12u, .tx_pin = 13u, .s_gpio = GPIOD, .s_pin = 3u, .af = 9u, .filter_bank = 14u,
        .is_can2 = true, .label = "B", .preferred_bitrate = 125000u
    };

    clock_ok = clock_init_168mhz();
    bus_clocks_update();
    can_hw_set_pclk1(pclk1_hz);
    uart_esp_init();              /* USART2 PA2/PA3 '-'> ESP32 GPIO16/17 */

    dev_init(dev_global());   /* seed profiles; journal replay adds the saved ones */
    mail_init(mail_global()); /* default addresses; journal replay restores settings */
    can_capture_queue_init(&capture_queue[0]);
    can_capture_queue_init(&capture_queue[1]);
    for (uint32_t i = 0; i < 2u; i++) { can_profiler_init(&profiler[i]); can_profiler_set_bus(&profiler[i], (uint8_t)i); }
    {   /* Engine start works in read-only and write mode (no ARM needed). */
        remote_start_cfg_t rcfg = { .timeout_ms = RS_TIMEOUT_DEFAULT_MS,
            .rpm_running_threshold = (uint16_t)RS_RPM_THRESHOLD_DEFAULT,
            .min_crank_ms = RS_MIN_CRANK_DEFAULT_MS, .require_armed = false,
            .require_neutral_or_park = true, .require_brake = true,
            .require_immobilizer_ok = true };
        remote_start_init(&rs, &rcfg);
    }
    gear_led_init(&gear_led);
    starter_gpio_init();
    ignition_inputs_init();

    if (SysTick_Config(SystemCoreClock / 1000u) != 0u) for (;;) { }
    tim2_us_init();

    { char s[48]; (void)snprintf(s, sizeof s, "STATUS,SYS,RESET,%s\r\n", reset_cause_take()); emit(s); }
    if (!clock_ok) emit("STATUS,SYS,CLOCK,HSI_FALLBACK\r\n");

    audio_codec_hold_reset_init(); /* PD4 low: CS43L22 idle */
    can_hw_gpio_init(&s_can[0], &s_cfg[0]);
    can_hw_gpio_init(&s_can[1], &s_cfg[1]);
    leds_init();
    vdd_monitor_init();
    /* Configure filters; passive (listen-only) scanning starts right below.
     * PE5/PE6 only gate TX-capable features (ARM, UDS, start). */
    can_hw_filters_init(&s_can[0], &s_can[1], true);
    /* Listen-only capture from power-up: the first frame the car sends is
     * caught whatever the key-input wiring says. SILM = no ACK, no TX. */
    if (clock_ok) {
        for (uint32_t i = 0; i < 2u; i++) can_hw_start_passive(&s_can[i], system_millis);
        emit("STATUS,SYS,POWER,LISTENING\r\n");
    }
#ifdef USB_CDC_ENABLED
    if (!clock_ok) {
        emit("STATUS,SYS,USB,CLOCK_UNAVAILABLE\r\n");
    } else if (!usb_gvret_init(usb_tx_cb, &s_can[0].current_bitrate,
                               &s_can[1].current_bitrate)) {
        emit("STATUS,SYS,USB,INIT_FAILED\r\n");
    } else {
        emit("STATUS,SYS,USB,STARTED\r\n");
    }
#endif
#ifdef NET_DASHBOARD
    if (!clock_ok) emit("STATUS,SYS,NET,CLOCK_UNAVAILABLE\r\n");
    else if (!net_usb_init()) emit("STATUS,SYS,NET,INIT_FAILED\r\n");
    else if (!dash_init(dash_command, &capture_queue[0], &capture_queue[1])) emit("STATUS,SYS,NET,DASH_DEGRADED\r\n");
    else emit("STATUS,SYS,NET,STARTED\r\n");
#endif
    /* Saved target: boot straight into it. Otherwise identify the network
     * on every boot until the user selects a device. */
    if (dev_global()->target == 0u) dev_observe_start(dev_global(), system_millis);
    dev_announce();
    if (!iwdg_start()) emit("STATUS,SYS,WATCHDOG,FAILED\r\n");

    uint32_t last_stats[2] = {0,0}, last_dwell[2] = {0,0};
#ifdef USB_CDC_ENABLED
    uint32_t last_usb_stats = system_millis;
    usb_gvret_stats_t reported_usb_stats = {0};
#endif

    for (;;) {
        iwdg_feed();

        for (uint32_t i = 0; i < 2u; i++) {
            bus_periodic(&s_can[i], &last_stats[i], &last_dwell[i]);
            uds_service(&s_can[i]);
            ecuscan_service(&s_can[i]);
        }

        /* remote-start state machine: RPM freshness, poll, starter output */
        {   /* OBD 01 0C wins when polled; else the F-CAN broadcast candidate */
            const bool obd = rpm_last_ms != 0u && (uint32_t)(system_millis - rpm_last_ms) < RPM_FRESH_MS;
            const bool bc  = bc_rpm_ms   != 0u && (uint32_t)(system_millis - bc_rpm_ms)   < RPM_FRESH_MS;
            if (!obd && bc) rs_in.rpm = bc_rpm;
            rs_in.rpm_fresh = obd || bc;
        }
        veh_update();
        write_service();
        dev_service();
        rs_in.armed = s_can[0].tx_armed;
        remote_start_apply(remote_start_poll(&rs, &rs_in, system_millis));
        start_crank_can();

        periodic_tx_service();
#ifdef NET_DASHBOARD
        net_usb_poll();
        dash_poll();
        /* Poll UART RX from ESP32 — feed received lines into dashboard parser */
        {   const char *esp_line;
            while ((esp_line = uart_esp_rx_line()) != NULL) {
                dash_on_line(esp_line);
            }
        }
#endif
#ifdef USB_CDC_ENABLED
        usb_gvret_poll();
        if ((uint32_t)(system_millis - last_usb_stats) >= STATS_PERIOD_MS) {
            last_usb_stats = system_millis;
            usb_gvret_stats_t stats;
            usb_gvret_get_stats(&stats);
            if (stats.rx_packet_drops != reported_usb_stats.rx_packet_drops ||
                stats.tx_queue_drops != reported_usb_stats.tx_queue_drops ||
                stats.tx_transport_errors != reported_usb_stats.tx_transport_errors ||
                stats.bridge.output_drops != reported_usb_stats.bridge.output_drops ||
                stats.bridge.rejected_frames != reported_usb_stats.bridge.rejected_frames ||
                stats.bridge.ignored_setup_commands !=
                    reported_usb_stats.bridge.ignored_setup_commands) {
                char s[128];
                (void)snprintf(s, sizeof s,
                    "STATUS,SYS,USB,STATS,%lu,%lu,%lu,%lu,%lu,%lu\r\n",
                    (unsigned long)stats.rx_packet_drops,
                    (unsigned long)stats.tx_queue_drops,
                    (unsigned long)stats.tx_transport_errors,
                    (unsigned long)stats.bridge.output_drops,
                    (unsigned long)stats.bridge.rejected_frames,
                    (unsigned long)stats.bridge.ignored_setup_commands);
                emit(s);
                reported_usb_stats = stats;
            }
        }
#endif
        leds_update();

        {
            can_capture_frame_t frame;
            bool got_frame;
            /* Drain one from each bus per pass so a busy bus cannot starve the other. */
            do {
                got_frame = false;
                for (uint8_t b = 0u; b < 2u; b++) {
                    if (!can_capture_queue_pop(&capture_queue[b], &frame)) continue;
                    got_frame = true;
                    processed_can_frames++;
                    processed_last_activity_ms = system_millis;
                    dev_observe(dev_global(), frame.bus, frame.identifier, frame.extended != 0u);
                    char csv[80];
                    if (can_capture_frame_to_csv(&frame, csv, sizeof csv)) emit(csv);
                    char pid[OBD2_LINE_CAPACITY];
                    if (obd2_decode(&frame, pid, sizeof pid)) emit(pid);
                    rpm_from_frame(&frame);
                    broadcast_decode(&frame);
                    bus_last_rx_ms[b] = system_millis ? system_millis : 1u;
                    can_profiler_update(&profiler[b], &frame);
#ifdef USB_CDC_ENABLED
                    usb_gvret_stream_frame(&frame);
#endif
                    if (s_can[b].uds_ready && uds_busy(&s_can[b].uds))
                        uds_on_frame(&s_can[b].uds, frame.identifier, frame.extended != 0u,
                                     frame.data, frame.dlc, system_millis);
                    if (ecu_scan_active(&s_can[b].ecuscan) && frame.remote == 0u)
                        ecu_scan_on_frame(&s_can[b].ecuscan, frame.identifier, frame.extended != 0u,
                                          frame.data, frame.dlc);
                    if (gateway_on) {
                        can_ctrl_t *o = &s_can[b ^ 1u];
                        if (can_hw_tx_window_open(o, system_millis) && o->tx_tokens > 0u)
                            (void)can_hw_load_mailbox(o, frame.identifier, frame.extended != 0u,
                                                      frame.remote != 0u, frame.data, frame.dlc, true);
                    }
                }
            } while (got_frame);
        }

        for (uint32_t i = 0; i < 2u; i++)
            can_profiler_emit(&profiler[i], system_millis, profiler_write);

        /* Parked: sleep until the next interrupt (SysTick 1 ms, CAN RX, host
         * UART). Keeps the watchdog fed and the ESP32 command path live. */
        if (veh_parked() && !remote_start_starter_engaged(&rs)) __WFI();
    }
}
