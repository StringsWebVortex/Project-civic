/*
 * uart_dma.c - DMA1 Stream6 Channel4 TX ring buffer for USART2.
 *
 * Stream/channel mapping for USART2_TX: DMA1 Stream6 Channel4, as used by
 * ST's STM32CubeF4 example Projects/STM32F4-Discovery/Examples/UART/
 * UART_TwoBoards_ComDMA (main.h: USARTx_TX_DMA_STREAM DMA1_Stream6,
 * USARTx_TX_DMA_CHANNEL DMA_CHANNEL_4).
 *
 * Single producer (main loop) / single consumer (DMA + its IRQ).
 * Lines are queued all-or-nothing: if a whole line does not fit, it is
 * dropped and counted, so the receiver never sees a cut or merged record.
 */

#include "uart_dma.h"
#include "stm32f407xx.h"
#include <string.h>

static uint8_t           tx_ring[UART_TX_RING_SIZE];
static volatile uint16_t tx_head;            /* written by producer only */
static volatile uint16_t tx_tail;            /* written by DMA IRQ only  */
static volatile uint8_t  dma_active;
static volatile uint16_t dma_transfer_len;
static volatile uint32_t dropped_lines;
static volatile uint32_t dma_errors;

/* Start the next contiguous segment. Called with the stream idle. */
static void dma_kick(void)
{
    const uint16_t tail = tx_tail;
    const uint16_t head = tx_head;
    if (tail == head) { dma_active = 0u; return; }

    const uint16_t contig = (head > tail) ? (uint16_t)(head - tail)
                                          : (uint16_t)(UART_TX_RING_SIZE - tail);
    dma_transfer_len = contig;
    dma_active       = 1u;

    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream6->CR & DMA_SxCR_EN) { }
    DMA1->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 |
                  DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6;
    DMA1_Stream6->M0AR = (uint32_t)(uintptr_t)(&tx_ring[tail]);
    DMA1_Stream6->NDTR = contig;
    __DSB();                                  /* ring data visible before DMA starts */
    DMA1_Stream6->CR  |= DMA_SxCR_EN;
}

void uart_dma_init(void)
{
    tx_head = 0u; tx_tail = 0u; dma_active = 0u; dma_transfer_len = 0u;
    dropped_lines = 0u; dma_errors = 0u;

    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;

    DMA1_Stream6->CR = 0u;
    while (DMA1_Stream6->CR & DMA_SxCR_EN) { }

    DMA1_Stream6->CR = (4u << DMA_SxCR_CHSEL_Pos)   /* channel 4          */
                     | DMA_SxCR_DIR_0               /* memory->peripheral */
                     | DMA_SxCR_MINC
                     | DMA_SxCR_TCIE
                     | DMA_SxCR_TEIE                /* transfer error     */
                     | DMA_SxCR_DMEIE;              /* direct mode error  */
    DMA1_Stream6->PAR  = (uint32_t)(uintptr_t)(&USART2->DR);
    DMA1_Stream6->M0AR = 0u;
    DMA1_Stream6->NDTR = 0u;
    DMA1_Stream6->FCR  = 0u;                        /* direct mode */

    USART2->CR3 |= USART_CR3_DMAT;
    NVIC_SetPriority(DMA1_Stream6_IRQn, 1u);
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

bool uart_dma_put_line(const char *str)
{
    if (!str) return false;
    const size_t len = strlen(str);
    if (len == 0u) return true;

    const uint16_t head = tx_head;
    const uint16_t tail = tx_tail;
    const size_t used = (size_t)((head + UART_TX_RING_SIZE - tail) % UART_TX_RING_SIZE);
    const size_t free_bytes = (UART_TX_RING_SIZE - 1u) - used;
    if (len > free_bytes) { dropped_lines++; return false; }

    uint16_t h = head;
    for (size_t i = 0u; i < len; i++) {
        tx_ring[h] = (uint8_t)str[i];
        h = (uint16_t)((h + 1u) % UART_TX_RING_SIZE);
    }
    __DMB();                                  /* data before index */
    tx_head = h;

    __disable_irq();
    if (dma_active == 0u) dma_kick();
    __enable_irq();
    return true;
}

void uart_dma_puts(const char *str) { (void)uart_dma_put_line(str); }

uint32_t uart_dma_dropped_lines(void) { return dropped_lines; }

uint32_t uart_dma_free(void)
{
    const uint16_t head = tx_head, tail = tx_tail;
    const uint32_t used = (uint32_t)((head + UART_TX_RING_SIZE - tail) % UART_TX_RING_SIZE);
    return (UART_TX_RING_SIZE - 1u) - used;
}
uint32_t uart_dma_errors(void)        { return dma_errors; }

void DMA1_Stream6_IRQHandler(void)
{
    const uint32_t hisr = DMA1->HISR;
    if (hisr & (DMA_HISR_TEIF6 | DMA_HISR_DMEIF6)) {
        /* Error: the stream is disabled by hardware. Resend the same segment. */
        dma_errors++;
        DMA1->HIFCR = DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6;
        dma_active = 0u;
        dma_kick();
        return;
    }
    if (hisr & DMA_HISR_TCIF6) {
        DMA1->HIFCR = DMA_HIFCR_CTCIF6;
        tx_tail = (uint16_t)((tx_tail + dma_transfer_len) % UART_TX_RING_SIZE);
        dma_transfer_len = 0u;
        dma_active = 0u;
        dma_kick();
    }
}
