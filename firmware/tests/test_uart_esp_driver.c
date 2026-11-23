#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "stm32f4xx.h"
#include "uart_esp.h"

GPIO_TypeDef mock_gpioa;
RCC_TypeDef mock_rcc;
USART_TypeDef mock_usart2;
DMA_Stream_TypeDef mock_dma1_stream6;
DMA_TypeDef mock_dma1;
uint32_t mock_primask;

#define DMA_TCIF (1u << 21u)
#define DMA_TEIF (1u << 19u)
#define DMA_DMEIF (1u << 18u)
#define DMA_FEIF (1u << 16u)

static unsigned checks;
static unsigned failures;
#define CHECK(expr, message) do { \
    checks++; \
    if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\\n", __FILE__, __LINE__, message); failures++; } \
} while (0)

void DMA1_Stream6_IRQHandler(void);
void USART2_IRQHandler(void);

static void reset_driver(void)
{
    memset(&mock_gpioa, 0, sizeof mock_gpioa);
    memset(&mock_rcc, 0, sizeof mock_rcc);
    memset(&mock_usart2, 0, sizeof mock_usart2);
    memset(&mock_dma1_stream6, 0, sizeof mock_dma1_stream6);
    memset(&mock_dma1, 0, sizeof mock_dma1);
    mock_primask = 0u;
    CHECK(uart_esp_init(), "USART2 DMA driver initializes");
}

static void rx_byte(uint8_t byte)
{
    mock_usart2.SR = USART_SR_RXNE;
    mock_usart2.DR = byte;
    USART2_IRQHandler();
}

static void rx_text(const char *text)
{
    while (*text) rx_byte((uint8_t)*text++);
}

static void dma_complete(void)
{
    mock_dma1.HISR = DMA_TCIF;
    DMA1_Stream6_IRQHandler();
}

static void test_init_and_idle_paths(void)
{
    reset_driver();
    CHECK(mock_rcc.AHB1ENR & RCC_AHB1ENR_GPIOAEN, "GPIOA clock enabled");
    CHECK(mock_rcc.AHB1ENR & RCC_AHB1ENR_DMA1EN, "DMA1 clock enabled");
    CHECK(mock_rcc.APB1ENR & RCC_APB1ENR_USART2EN, "USART2 clock enabled");
    CHECK(mock_usart2.BRR == 21u, "2 Mbaud BRR is exact for 42 MHz APB1");
    CHECK((mock_usart2.CR1 & (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE)) ==
          (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE), "USART2 TX/RX/RXNE enabled");
    CHECK((mock_gpioa.AFR[0] >> 8u & 0xFu) == 7u, "PA2 configured for USART2_TX AF7");
    CHECK((mock_gpioa.AFR[0] >> 12u & 0xFu) == 7u, "PA3 configured for USART2_RX AF7");
    CHECK((mock_dma1_stream6.CR & (DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE)) != 0u,
          "DMA completion and error interrupts enabled");
    CHECK(uart_esp_tx_dropped() == 0u && uart_esp_tx_dma_errors() == 0u &&
          uart_esp_rx_dropped() == 0u, "diagnostic counters reset at initialization");
    CHECK(uart_esp_rx_line() == NULL, "empty RX ring returns no line");
    DMA1_Stream6_IRQHandler(); /* no flags: early-return branch */
    mock_usart2.SR = 0u;
    USART2_IRQHandler();       /* no RXNE/ORE branch */
}

static void test_tx_queue_and_dma_paths(void)
{
    reset_driver();
    uart_esp_emit(NULL);
    uart_esp_emit("");
    CHECK(uart_esp_tx_dropped() == 0u, "NULL and empty TX strings are ignored");

    uart_esp_emit("ABC");
    CHECK((mock_dma1_stream6.CR & DMA_SxCR_EN) != 0u && mock_dma1_stream6.NDTR == 3u,
          "queued TX starts DMA with exact byte count");
    uart_esp_emit("D"); /* active DMA leaves the new message queued */
    CHECK(mock_dma1_stream6.NDTR == 3u, "active DMA is not restarted by a producer");
    dma_complete();
    CHECK((mock_dma1_stream6.CR & DMA_SxCR_EN) != 0u && mock_dma1_stream6.NDTR == 1u,
          "DMA completion advances queue and starts the next segment");
    dma_complete();
    CHECK((mock_dma1_stream6.CR & DMA_SxCR_EN) == 0u, "empty queue leaves DMA idle");

    CHECK(uart_esp_emit_line(NULL, 0u), "empty line request is a no-op success");
    CHECK(uart_esp_emit_line("OK", 2u), "line message is queued");
    CHECK(mock_dma1_stream6.NDTR == 3u, "line queue includes its newline delimiter");
    mock_dma1.HISR = 0u;
    DMA1_Stream6_IRQHandler();
    CHECK((mock_dma1_stream6.CR & DMA_SxCR_EN) != 0u, "unrelated DMA interrupt flags do not consume data");
    mock_dma1.HISR = DMA_TEIF;
    DMA1_Stream6_IRQHandler();
    CHECK(uart_esp_tx_dma_errors() == 1u, "DMA transfer error increments its diagnostic counter");
    CHECK(uart_esp_tx_dma_transfer_errors() == 1u, "transfer error subtype increments");
    CHECK(uart_esp_tx_dma_direct_errors() == 0u && uart_esp_tx_dma_fifo_errors() == 0u,
          "unrelated DMA error subtypes remain zero");
    CHECK(uart_esp_tx_dma_last_flags() == DMA_TEIF, "last DMA error flags are retained");
    CHECK((mock_dma1_stream6.CR & DMA_SxCR_EN) == 0u, "failed DMA segment is discarded cleanly");

    uart_esp_emit("D");
    mock_dma1.HISR = DMA_DMEIF;
    DMA1_Stream6_IRQHandler();
    CHECK(uart_esp_tx_dma_direct_errors() == 1u, "direct-mode error subtype increments");
    uart_esp_emit("E");
    mock_dma1.HISR = DMA_FEIF;
    DMA1_Stream6_IRQHandler();
    CHECK(uart_esp_tx_dma_fifo_errors() == 1u, "FIFO error subtype increments");
    CHECK(uart_esp_tx_dma_errors() == 3u, "all DMA error classes contribute to total count");

    reset_driver();
    char fill[2047];
    memset(fill, 'Q', sizeof fill);
    fill[sizeof fill - 1u] = '\0';
    uart_esp_emit("A");
    uart_esp_emit(fill); /* fills remaining ring while DMA owns the first byte */
    uart_esp_emit("B");
    CHECK(uart_esp_tx_dropped() == 1u, "full TX queue increments drop counter");
    CHECK(!uart_esp_emit_line("C", 1u), "line write reports queue-full failure");
    CHECK(uart_esp_tx_dropped() == 2u, "failed line write increments drop counter");
    dma_complete();
    CHECK(mock_dma1_stream6.NDTR == 2046u, "DMA restarts on the queued contiguous payload");
    mock_dma1.HISR = DMA_TEIF;
    DMA1_Stream6_IRQHandler();
    CHECK(uart_esp_tx_dma_errors() == 1u, "second DMA failure is counted");
}

static void test_rx_line_and_error_paths(void)
{
    reset_driver();
    rx_text("HELLO\r\n");
    const char *line = uart_esp_rx_line();
    CHECK(line && strcmp(line, "HELLO") == 0, "RX assembles CRLF-terminated line");
    CHECK(uart_esp_rx_line() == NULL, "RX line is consumed exactly once");
    rx_text("\n");
    CHECK(uart_esp_rx_line() == NULL, "empty RX lines are ignored");

    mock_usart2.SR = USART_SR_ORE;
    mock_usart2.DR = 0u;
    USART2_IRQHandler();
    CHECK(uart_esp_rx_dropped() == 0u, "USART overrun status is cleared without corrupting ring accounting");

    reset_driver();
    char long_part[1001];
    memset(long_part, 'X', 1000u);
    long_part[1000] = '\0';
    rx_text(long_part);
    CHECK(uart_esp_rx_line() == NULL, "first part of long line remains buffered without newline");
    char second_part[402];
    memset(second_part, 'X', 401u);
    second_part[401] = '\0';
    rx_text(second_part);
    CHECK(uart_esp_rx_line() == NULL, "line buffer overflow is detected while draining RX ring");
    rx_text("\nRECOVERED\n");
    line = uart_esp_rx_line();
    CHECK(line && strcmp(line, "RECOVERED") == 0, "overlong RX line is discarded and parser recovers at newline");
    CHECK(uart_esp_rx_dropped() == 1u, "overlong RX line increments drop counter");

    reset_driver();
    for (unsigned i = 0; i < 1024u; i++) rx_byte((uint8_t)'R');
    CHECK(uart_esp_rx_dropped() == 1u, "full RX ring increments drop counter");
    CHECK(uart_esp_rx_line() == NULL, "unterminated buffered RX bytes do not produce a line");
}

int main(void)
{
    test_init_and_idle_paths();
    test_tx_queue_and_dma_paths();
    test_rx_line_and_error_paths();
    if (failures) {
        fprintf(stderr, "%u/%u checks failed\\n", failures, checks);
        return 1;
    }
    printf("test_uart_esp_driver: %u checks passed\\n", checks);
    return 0;
}
