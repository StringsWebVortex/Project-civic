/*
 * uart_esp.c — USART2 DMA TX + ISR RX to/from ESP32 Wi-Fi bridge
 *
 * Wiring
 * ------
 *   PA2  USART2_TX  AF7  push-pull  → ESP32 GPIO16 (RX2)
 *   PA3  USART2_RX  AF7  input      ← ESP32 GPIO17 (TX2)
 *   GND  GND                        — common
 *
 * Both sides are 3.3 V; no level shifter needed.
 *
 * Clock math (APB1 = 42 MHz, baud = 2 000 000)
 *   USARTDIV = 42 000 000 / 2 000 000 = 21.000  → exact, no fractional error
 *
 * TX: DMA1 Stream6 Channel4 — non-blocking.  Falls back to polled if DMA busy.
 *     This avoids blocking the main loop during flash erase (scope #13).
 *
 * RX: USART2 RXNE interrupt → ring buffer.  Main loop polls uart_esp_rx_line()
 *     which assembles complete lines from the ring buffer.
 */

#include "uart_esp.h"
#include "stm32f4xx.h"
#include <string.h>

/* ── config ──────────────────────────────────────────────────────────────── */
#ifndef PROJECT_HOST_UART_BAUD
#define PROJECT_HOST_UART_BAUD  2000000u
#endif

#define UART_APB1_HZ    42000000u
#define UART_BAUD       PROJECT_HOST_UART_BAUD

/* ── DMA TX buffer ───────────────────────────────────────────────────────── */
#define TX_BUF_SIZE  2048u
static char         tx_buf[TX_BUF_SIZE];
static volatile bool tx_busy = false;

/* ── RX ring buffer (ISR → main loop) ────────────────────────────────────── */
#define RX_RING_SIZE  1024u
static volatile uint8_t rx_ring[RX_RING_SIZE];
static volatile uint16_t rx_head;   /* written by ISR */
static volatile uint16_t rx_tail;   /* read by main loop */

/* Line assembly buffer (main loop only) */
#define RX_LINE_SIZE  256u
static char rx_line[RX_LINE_SIZE];
static uint16_t rx_line_pos;

/* ── init ────────────────────────────────────────────────────────────────── */
bool uart_esp_init(void)
{
    /* 1. Clock gates */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_DMA1EN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;

    /* 2. PA2 → USART2_TX, AF7, push-pull, high speed, no pull */
    /*    PA3 → USART2_RX, AF7, input,      no pull              */
    GPIOA->MODER &= ~((3u << (2u*2u)) | (3u << (2u*3u)));
    GPIOA->MODER |=   (2u << (2u*2u)) | (2u << (2u*3u));
    GPIOA->OTYPER &= ~((1u << 2u) | (1u << 3u));
    GPIOA->OSPEEDR |= (3u << (2u*2u)) | (3u << (2u*3u));
    GPIOA->PUPDR &= ~((3u << (2u*2u)) | (3u << (2u*3u)));
    GPIOA->AFR[0] &= ~((0xFu << (4u*2u)) | (0xFu << (4u*3u)));
    GPIOA->AFR[0] |=   (7u   << (4u*2u)) | (7u   << (4u*3u));

    /* 3. USART2: 8N1, baud = APB1 / UART_BAUD */
    USART2->CR1 = 0;
    USART2->CR2 = 0;
    USART2->CR3 = 0;
    uint32_t brr = UART_APB1_HZ / UART_BAUD;
    if (brr == 0u || brr > 0xFFFFu) return false;
    USART2->BRR = brr;

    /* 4. DMA1 Stream6 Channel4 for USART2_TX */
    DMA1_Stream6->CR = 0;
    while (DMA1_Stream6->CR & DMA_SxCR_EN) {}   /* wait disable */
    DMA1->HIFCR = 0x3Fu << 16u;                 /* clear Stream6 flags */
    DMA1_Stream6->PAR  = (uint32_t)&USART2->DR;
    DMA1_Stream6->CR   = (4u << DMA_SxCR_CHSEL_Pos)  /* channel 4 */
                       | DMA_SxCR_MINC               /* memory increment */
                       | (1u << DMA_SxCR_DIR_Pos)     /* mem→periph */
                       | DMA_SxCR_TCIE;               /* transfer complete IRQ */
    USART2->CR3 |= USART_CR3_DMAT;              /* USART2 DMA TX enable */

    /* 5. Enable USART2 RXNE interrupt for RX */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;

    /* 6. NVIC: USART2 (IRQ 38) for RX, DMA1_Stream6 (IRQ 17) for TX complete */
    NVIC_SetPriority(USART2_IRQn, 5);
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_SetPriority(DMA1_Stream6_IRQn, 6);
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);

    return true;
}

/* ── DMA TX complete ISR ─────────────────────────────────────────────────── */
void DMA1_Stream6_IRQHandler(void)
{
    DMA1->HIFCR = 0x3Fu << 16u;   /* clear all Stream6 flags */
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    tx_busy = false;
}

/* ── USART2 RX ISR ──────────────────────────────────────────────────────── */
void USART2_IRQHandler(void)
{
    uint32_t sr = USART2->SR;
    if (sr & USART_SR_RXNE) {
        uint8_t byte = (uint8_t)(USART2->DR & 0xFFu);
        uint16_t next = (rx_head + 1u) % RX_RING_SIZE;
        if (next != rx_tail) {   /* drop byte on overflow */
            rx_ring[rx_head] = byte;
            rx_head = next;
        }
    }
    /* Clear overrun without losing data */
    if (sr & USART_SR_ORE) {
        (void)USART2->DR;
    }
}

/* ── emit (DMA TX, non-blocking) ─────────────────────────────────────────── */
void uart_esp_emit(const char *s)
{
    if (!s || !*s) return;
    size_t len = strlen(s);
    if (len > TX_BUF_SIZE) len = TX_BUF_SIZE;

    if (tx_busy) {
        /* Fallback: polled TX if DMA still running (should be rare) */
        const char *p = s;
        while (*p) {
            while (!(USART2->SR & USART_SR_TXE)) {}
            USART2->DR = (uint8_t)*p++;
        }
        while (!(USART2->SR & USART_SR_TC)) {}
        return;
    }

    memcpy(tx_buf, s, len);
    tx_busy = true;

    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream6->CR & DMA_SxCR_EN) {}
    DMA1->HIFCR = 0x3Fu << 16u;
    DMA1_Stream6->M0AR = (uint32_t)tx_buf;
    DMA1_Stream6->NDTR = (uint16_t)len;
    DMA1_Stream6->CR |= DMA_SxCR_EN;
}

/* ── emit line (data + '\n' in one DMA shot) ────────────────────────────── */
void uart_esp_emit_line(const char *s, size_t n)
{
    if (!s || n == 0) return;
    if (n > TX_BUF_SIZE - 1u) n = TX_BUF_SIZE - 1u;   /* leave room for '\n' */

    if (tx_busy) {
        /* Fallback: polled TX */
        for (size_t i = 0; i < n; i++) {
            while (!(USART2->SR & USART_SR_TXE)) {}
            USART2->DR = (uint8_t)s[i];
        }
        while (!(USART2->SR & USART_SR_TXE)) {}
        USART2->DR = (uint8_t)'\n';
        while (!(USART2->SR & USART_SR_TC)) {}
        return;
    }

    memcpy(tx_buf, s, n);
    tx_buf[n] = '\n';
    tx_busy = true;

    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream6->CR & DMA_SxCR_EN) {}
    DMA1->HIFCR = 0x3Fu << 16u;
    DMA1_Stream6->M0AR = (uint32_t)tx_buf;
    DMA1_Stream6->NDTR = (uint16_t)(n + 1u);
    DMA1_Stream6->CR |= DMA_SxCR_EN;
}

/* ── RX line assembly (main loop) ────────────────────────────────────────── */
const char *uart_esp_rx_line(void)
{
    while (rx_tail != rx_head) {
        uint8_t c = rx_ring[rx_tail];
        rx_tail = (rx_tail + 1u) % RX_RING_SIZE;

        if (c == '\n') {
            rx_line[rx_line_pos] = '\0';
            /* trim trailing CR */
            if (rx_line_pos > 0 && rx_line[rx_line_pos - 1] == '\r')
                rx_line[--rx_line_pos] = '\0';
            rx_line_pos = 0;
            if (rx_line[0] != '\0') return rx_line;
            continue;  /* skip empty lines */
        }
        if (c != '\r' && rx_line_pos < RX_LINE_SIZE - 1u) {
            rx_line[rx_line_pos++] = (char)c;
        }
    }
    return NULL;  /* no complete line yet */
}
