/*
 * USART2 bridge to the ESP32.
 * PA2 USART2_TX -> ESP32 GPIO16 RX; PA3 USART2_RX <- ESP32 GPIO17 TX.
 * 8N1 at 2,000,000 baud (APB1 = 42 MHz, BRR = 21).
 * TX uses a bounded SRAM ring and DMA1 Stream6/Channel4.  The DMA ISR is the
 * only consumer; main-loop producers commit complete messages atomically.
 * RX uses USART2 RXNE interrupt and a bounded ring for line-oriented commands.
 */
#include "uart_esp.h"
#include "uart_tx_queue.h"
#include "stm32f4xx.h"
#include <string.h>

#ifndef PROJECT_HOST_UART_BAUD
#define PROJECT_HOST_UART_BAUD 2000000u
#endif

#define UART_APB1_HZ 42000000u
#define UART_BAUD PROJECT_HOST_UART_BAUD
#define RX_RING_SIZE 1024u
#define RX_LINE_SIZE 1401u

#define DMA1_S6_TCIF   (1u << 21u)
#define DMA1_S6_TEIF   (1u << 19u)
#define DMA1_S6_DMEIF  (1u << 18u)
#define DMA1_S6_FEIF   (1u << 16u)
#define DMA1_S6_CLEAR  (0x3Fu << 16u)
#define DMA1_S6_ERRORS (DMA1_S6_TEIF | DMA1_S6_DMEIF | DMA1_S6_FEIF)

static uart_tx_queue_t tx_queue; /* DMA-accessible SRAM, never CCMRAM */
static volatile bool tx_dma_active;
static volatile uint16_t tx_dma_length;
static volatile uint32_t tx_dropped;
static volatile uint32_t tx_dma_errors;
static volatile uint32_t tx_dma_transfer_errors;
static volatile uint32_t tx_dma_direct_errors;
static volatile uint32_t tx_dma_fifo_errors;
static volatile uint32_t tx_dma_last_flags;

static volatile uint8_t rx_ring[RX_RING_SIZE];
static volatile uint16_t rx_head;
static volatile uint16_t rx_tail;
static volatile uint32_t rx_dropped;
static char rx_line[RX_LINE_SIZE];
static uint16_t rx_line_pos;
static bool rx_line_overflow;

static void dma_kick(void)
{
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();
    if (tx_dma_active) {
        __set_PRIMASK(primask);
        return;
    }

    uint16_t length = 0u;
    uint8_t *data = uart_tx_queue_peek(&tx_queue, &length);
    if (!data || length == 0u) {
        __set_PRIMASK(primask);
        return;
    }

    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream6->CR & DMA_SxCR_EN) { }
    DMA1->HIFCR = DMA1_S6_CLEAR;
    DMA1_Stream6->M0AR = (uint32_t)(uintptr_t)data;
    DMA1_Stream6->NDTR = length;
    tx_dma_length = length;
    tx_dma_active = true;
    DMA1_Stream6->CR |= DMA_SxCR_EN;
    __set_PRIMASK(primask);
}

bool uart_esp_init(void)
{
    uart_tx_queue_init(&tx_queue);
    tx_dma_active = false;
    tx_dma_length = 0u;
    tx_dropped = 0u;
    tx_dma_errors = 0u;
    tx_dma_transfer_errors = 0u;
    tx_dma_direct_errors = 0u;
    tx_dma_fifo_errors = 0u;
    tx_dma_last_flags = 0u;
    rx_head = rx_tail = 0u;
    rx_dropped = 0u;
    rx_line_pos = 0u;
    rx_line_overflow = false;

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_DMA1EN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;

    GPIOA->MODER &= ~((3u << (2u * 2u)) | (3u << (2u * 3u)));
    GPIOA->MODER |=   (2u << (2u * 2u)) | (2u << (2u * 3u));
    GPIOA->OTYPER &= ~((1u << 2u) | (1u << 3u));
    GPIOA->OSPEEDR |= (3u << (2u * 2u)) | (3u << (2u * 3u));
    GPIOA->PUPDR &= ~((3u << (2u * 2u)) | (3u << (2u * 3u)));
    GPIOA->AFR[0] &= ~((0xFu << (4u * 2u)) | (0xFu << (4u * 3u)));
    GPIOA->AFR[0] |=   (7u << (4u * 2u)) | (7u << (4u * 3u));

    USART2->CR1 = 0u;
    USART2->CR2 = 0u;
    USART2->CR3 = 0u;
    const uint32_t brr = UART_APB1_HZ / UART_BAUD;
    if (brr == 0u || brr > 0xFFFFu) return false;
    USART2->BRR = brr;

    DMA1_Stream6->CR = 0u;
    while (DMA1_Stream6->CR & DMA_SxCR_EN) { }
    DMA1->HIFCR = DMA1_S6_CLEAR;
    DMA1_Stream6->PAR = (uint32_t)(uintptr_t)&USART2->DR;
    DMA1_Stream6->M0AR = 0u;
    DMA1_Stream6->NDTR = 0u;
    DMA1_Stream6->FCR = 0u; /* direct mode */
    DMA1_Stream6->CR = (4u << DMA_SxCR_CHSEL_Pos) |
                       DMA_SxCR_MINC |
                       (1u << DMA_SxCR_DIR_Pos) |
                       DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE;
    USART2->CR3 |= USART_CR3_DMAT;
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;

    NVIC_SetPriority(USART2_IRQn, 5u);
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_SetPriority(DMA1_Stream6_IRQn, 6u);
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);
    return true;
}

void DMA1_Stream6_IRQHandler(void)
{
    const uint32_t flags = DMA1->HISR;
    if ((flags & (DMA1_S6_TCIF | DMA1_S6_ERRORS)) == 0u) return;

    DMA1->HIFCR = DMA1_S6_CLEAR;
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream6->CR & DMA_SxCR_EN) { }

    if (tx_dma_active && tx_dma_length != 0u) {
        /* On an error, discard the failed segment; never splice a retry into it. */
        const uint32_t errors = flags & DMA1_S6_ERRORS;
        if (errors != 0u) {
            tx_dma_errors++;
            tx_dma_last_flags = errors;
            if ((errors & DMA1_S6_TEIF) != 0u) tx_dma_transfer_errors++;
            if ((errors & DMA1_S6_DMEIF) != 0u) tx_dma_direct_errors++;
            if ((errors & DMA1_S6_FEIF) != 0u) tx_dma_fifo_errors++;
        }
        (void)uart_tx_queue_consume(&tx_queue, tx_dma_length);
    }
    tx_dma_length = 0u;
    tx_dma_active = false;
    dma_kick();
}

void USART2_IRQHandler(void)
{
    const uint32_t sr = USART2->SR;
    if ((sr & USART_SR_RXNE) != 0u) {
        const uint8_t byte = (uint8_t)(USART2->DR & 0xFFu);
        const uint16_t next = (uint16_t)((rx_head + 1u) % RX_RING_SIZE);
        if (next != rx_tail) {
            rx_ring[rx_head] = byte;
            rx_head = next;
        } else {
            rx_dropped++;
        }
    }
    if ((sr & USART_SR_ORE) != 0u) (void)USART2->DR;
}

void uart_esp_emit(const char *s)
{
    if (!s || !*s) return;
    const size_t length = strlen(s);
    if (!uart_tx_queue_write(&tx_queue, (const uint8_t *)s, length)) {
        tx_dropped++;
        return;
    }
    dma_kick();
}

bool uart_esp_emit_line(const char *s, size_t n)
{
    if (!s || n == 0u) return true;
    if (!uart_tx_queue_write_line(&tx_queue, (const uint8_t *)s, n)) {
        tx_dropped++;
        return false;
    }
    dma_kick();
    return true;
}

uint32_t uart_esp_tx_dropped(void) { return tx_dropped; }
uint32_t uart_esp_tx_dma_errors(void) { return tx_dma_errors; }
uint32_t uart_esp_tx_dma_transfer_errors(void) { return tx_dma_transfer_errors; }
uint32_t uart_esp_tx_dma_direct_errors(void) { return tx_dma_direct_errors; }
uint32_t uart_esp_tx_dma_fifo_errors(void) { return tx_dma_fifo_errors; }
uint32_t uart_esp_tx_dma_last_flags(void) { return tx_dma_last_flags; }
uint32_t uart_esp_rx_dropped(void) { return rx_dropped; }

const char *uart_esp_rx_line(void)
{
    while (rx_tail != rx_head) {
        const uint8_t c = rx_ring[rx_tail];
        rx_tail = (uint16_t)((rx_tail + 1u) % RX_RING_SIZE);

        if (c == '\n') {
            if (rx_line_overflow) {
                rx_line_overflow = false;
                rx_line_pos = 0u;
                continue;
            }
            rx_line[rx_line_pos] = '\0';
            rx_line_pos = 0u;
            if (rx_line[0] != '\0') return rx_line;
            continue;
        }
        if (c == '\r' || rx_line_overflow) continue;
        if (rx_line_pos < RX_LINE_SIZE - 1u) {
            rx_line[rx_line_pos++] = (char)c;
        } else {
            rx_line_pos = 0u;
            rx_line_overflow = true;
            rx_dropped++;
        }
    }
    return NULL;
}
