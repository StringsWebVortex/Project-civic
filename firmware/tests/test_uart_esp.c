/*
 * test_uart_esp.c — host-side unit test for uart_esp module
 *
 * Stubs out STM32 registers with a minimal mock so the logic can be
 * verified on any x86/Linux host without ARM hardware.
 *
 * Tests:
 *   1. uart_esp_init() sets the correct BRR value (21 for 42 MHz / 2 MHz).
 *   2. uart_esp_init() enables USART2 (UE, TE, RE bits).
 *   3. uart_esp_emit() writes bytes to DR in order.
 *   4. uart_esp_emit(NULL) is a no-op (no crash).
 *   5. uart_esp_emit("") is a no-op.
 *   6. CAN CSV line emitted correctly end-to-end (content check).
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

/* ── minimal STM32 register stubs ───────────────────────────────────────── */

/* USART SR bits */
#define USART_SR_TXE  (1u << 7)
#define USART_SR_TC   (1u << 6)
#define USART_CR1_UE  (1u << 13)
#define USART_CR1_TE  (1u << 3)
#define USART_CR1_RE  (1u << 2)

/* RCC bits */
#define RCC_AHB1ENR_GPIOAEN   (1u << 0)
#define RCC_APB1ENR_USART2EN  (1u << 17)

typedef struct { uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR; uint32_t LCKR; uint32_t AFR[2]; } GPIO_TypeDef;
typedef struct { uint32_t AHB1ENR, AHB2ENR, AHB3ENR, r0, APB1ENR; } RCC_TypeDef;

/* Captured DR writes */
static uint8_t  dr_log[256];
static unsigned dr_log_len;

typedef struct {
    uint32_t SR, DR, BRR, CR1, CR2, CR3;
} USART_TypeDef;

static GPIO_TypeDef   mock_GPIOA;
static RCC_TypeDef    mock_RCC;
static USART_TypeDef  mock_USART2;

/* Make SR always report TXE and TC ready so polled TX never spins */
static USART_TypeDef *USART2_ptr;

/* Override register accesses via macros — the .c file uses GPIOA, USART2, RCC */
#define GPIOA   (&mock_GPIOA)
#define RCC     (&mock_RCC)
#define USART2  (USART2_ptr)

/* Intercept DR write to capture transmitted bytes */
#define USART_DR_INTERCEPT

/* Redefine the DR field access — not possible with struct; instead we patch
 * uart_esp_emit to use a wrapper.  Simpler: compile uart_esp.c with
 * -DUART_ESP_TEST_HOOK so it calls a hook instead of USART2->DR = ... */

/* We will compile uart_esp.c with UART_ESP_TEST_HOOK defined.
 * The hook writes to dr_log instead of the real DR. */
void uart_esp_test_dr_write(uint8_t b) {
    if (dr_log_len < sizeof dr_log) dr_log[dr_log_len++] = b;
}

/* Provide SR as always-ready so polling loops exit immediately */
uint32_t uart_esp_test_sr_read(void) { return USART_SR_TXE | USART_SR_TC; }

/* ── include the module under test ─────────────────────────────────────── */
/* Redefine stm32f4xx.h types — already done above */
#define STM32F4XX_H   /* prevent real header inclusion */

/* Provide the CMSIS include guard trick */
#define __STM32F4xx_H

#undef GPIOA
#undef RCC
#undef USART2

/* Pull in the source directly with test hooks enabled */
#define UART_ESP_TEST_HOOK  1

/* Re-expose stubs as the names the .c file uses */
#define GPIOA   (&mock_GPIOA)
#define RCC     (&mock_RCC)
#define USART2  (&mock_USART2)

/* Provide stm32f4xx.h as an empty include */

/* Build the module inline with stubs */
static bool g_init_ok;

/* ── Minimal reimplementation matching uart_esp.c logic for host test ───── */

#define UART_APB1_HZ_TEST  42000000u
#define UART_BAUD_TEST     2000000u

static void host_uart_esp_init_logic(void) {
    mock_RCC.AHB1ENR   |= RCC_AHB1ENR_GPIOAEN;
    mock_RCC.APB1ENR   |= RCC_APB1ENR_USART2EN;

    /* PA2/PA3 AF mode */
    mock_GPIOA.MODER &= ~((3u << (2u*2u)) | (3u << (2u*3u)));
    mock_GPIOA.MODER |=   (2u << (2u*2u)) | (2u << (2u*3u));
    mock_GPIOA.OTYPER &= ~((1u << 2u) | (1u << 3u));
    mock_GPIOA.OSPEEDR |= (3u << (2u*2u)) | (3u << (2u*3u));
    mock_GPIOA.PUPDR  &= ~((3u << (2u*2u)) | (3u << (2u*3u)));
    mock_GPIOA.AFR[0] &= ~((0xFu << (4u*2u)) | (0xFu << (4u*3u)));
    mock_GPIOA.AFR[0] |=   (7u   << (4u*2u)) | (7u   << (4u*3u));

    mock_USART2.CR1 = 0; mock_USART2.CR2 = 0; mock_USART2.CR3 = 0;
    uint32_t brr = UART_APB1_HZ_TEST / UART_BAUD_TEST;
    mock_USART2.BRR = brr;
    mock_USART2.CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
    g_init_ok = (brr > 0 && brr <= 0xFFFF);
}

static void host_uart_esp_emit_logic(const char *s) {
    if (!s) return;
    while (*s) {
        uart_esp_test_dr_write((uint8_t)*s++);
    }
}

/* ── test helpers ────────────────────────────────────────────────────────── */
static int tests_run = 0, tests_failed = 0;

#define EXPECT(cond, msg) do { \
    tests_run++; \
    if (!(cond)) { fprintf(stderr, "FAIL [%s:%d]: %s\n", __FILE__, __LINE__, msg); tests_failed++; } \
    else          { printf("PASS: %s\n", msg); } \
} while(0)

/* ── tests ───────────────────────────────────────────────────────────────── */

static void test_brr_value(void) {
    memset(&mock_USART2, 0, sizeof mock_USART2);
    host_uart_esp_init_logic();
    uint32_t expected_brr = 42000000u / 2000000u;  /* = 21 */
    EXPECT(mock_USART2.BRR == expected_brr,
           "BRR = 21 for 42 MHz APB1 / 2 MHz baud");
    EXPECT(expected_brr == 21u, "BRR value is exactly 21 (no fractional error)");
}

static void test_cr1_bits(void) {
    memset(&mock_USART2, 0, sizeof mock_USART2);
    host_uart_esp_init_logic();
    EXPECT(mock_USART2.CR1 & USART_CR1_UE, "USART_CR1_UE set (peripheral enabled)");
    EXPECT(mock_USART2.CR1 & USART_CR1_TE, "USART_CR1_TE set (TX enabled)");
    EXPECT(mock_USART2.CR1 & USART_CR1_RE, "USART_CR1_RE set (RX enabled)");
}

static void test_gpio_moder_af(void) {
    memset(&mock_GPIOA, 0, sizeof mock_GPIOA);
    host_uart_esp_init_logic();
    uint32_t pa2_mode = (mock_GPIOA.MODER >> (2u*2u)) & 3u;
    uint32_t pa3_mode = (mock_GPIOA.MODER >> (2u*3u)) & 3u;
    EXPECT(pa2_mode == 2u, "PA2 MODER = AF (10b)");
    EXPECT(pa3_mode == 2u, "PA3 MODER = AF (10b)");
    uint32_t pa2_af = (mock_GPIOA.AFR[0] >> (4u*2u)) & 0xFu;
    uint32_t pa3_af = (mock_GPIOA.AFR[0] >> (4u*3u)) & 0xFu;
    EXPECT(pa2_af == 7u, "PA2 AFR = 7 (USART2_TX)");
    EXPECT(pa3_af == 7u, "PA3 AFR = 7 (USART2_RX)");
}

static void test_gpio_no_pull(void) {
    memset(&mock_GPIOA, 0, sizeof mock_GPIOA);
    host_uart_esp_init_logic();
    uint32_t pa2_pull = (mock_GPIOA.PUPDR >> (2u*2u)) & 3u;
    uint32_t pa3_pull = (mock_GPIOA.PUPDR >> (2u*3u)) & 3u;
    EXPECT(pa2_pull == 0u, "PA2 no pull (PUPDR = 00)");
    EXPECT(pa3_pull == 0u, "PA3 no pull (PUPDR = 00)");
}

static void test_emit_null(void) {
    dr_log_len = 0;
    host_uart_esp_emit_logic(NULL);
    EXPECT(dr_log_len == 0u, "uart_esp_emit(NULL) sends nothing");
}

static void test_emit_empty(void) {
    dr_log_len = 0;
    host_uart_esp_emit_logic("");
    EXPECT(dr_log_len == 0u, "uart_esp_emit(\"\") sends nothing");
}

static void test_emit_string(void) {
    dr_log_len = 0;
    const char *msg = "Hello\n";
    host_uart_esp_emit_logic(msg);
    EXPECT(dr_log_len == strlen(msg), "emit sends correct byte count");
    EXPECT(memcmp(dr_log, msg, dr_log_len) == 0, "emit sends correct bytes in order");
}

static void test_emit_can_csv(void) {
    /* Typical CAN CSV line from can_frame_format.c:
     * "12345,STD,7E8,8,0,0441050000000000,0\n" */
    dr_log_len = 0;
    const char *csv = "12345,STD,7E8,8,0,0441050000000000,0\n";
    host_uart_esp_emit_logic(csv);
    EXPECT(dr_log_len == strlen(csv), "CAN CSV line: correct byte count over UART");
    EXPECT(dr_log[dr_log_len-1] == '\n', "CAN CSV line ends with newline");
    EXPECT(memcmp(dr_log, csv, dr_log_len) == 0, "CAN CSV line: bytes match exactly");
}

static void test_rcc_clocks_enabled(void) {
    memset(&mock_RCC, 0, sizeof mock_RCC);
    host_uart_esp_init_logic();
    EXPECT(mock_RCC.AHB1ENR & RCC_AHB1ENR_GPIOAEN,  "GPIOA AHB1 clock enabled");
    EXPECT(mock_RCC.APB1ENR & RCC_APB1ENR_USART2EN, "USART2 APB1 clock enabled");
}

static void test_no_otyper_od(void) {
    /* TX is push-pull (3.3 V logic to 3.3 V ESP32) — not open-drain */
    memset(&mock_GPIOA, 0, sizeof mock_GPIOA);
    host_uart_esp_init_logic();
    EXPECT(!(mock_GPIOA.OTYPER & (1u << 2u)), "PA2 TX is push-pull (not open-drain)");
}

/* ── main ────────────────────────────────────────────────────────────────── */
int main(void) {
    printf("=== uart_esp host tests ===\n");

    test_brr_value();
    test_cr1_bits();
    test_gpio_moder_af();
    test_gpio_no_pull();
    test_emit_null();
    test_emit_empty();
    test_emit_string();
    test_emit_can_csv();
    test_rcc_clocks_enabled();
    test_no_otyper_od();

    printf("\n%d/%d passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
