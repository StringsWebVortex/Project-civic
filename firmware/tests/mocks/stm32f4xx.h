#ifndef TEST_MOCK_STM32F4XX_H
#define TEST_MOCK_STM32F4XX_H

#include <stdint.h>

typedef struct {
    volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR;
    volatile uint32_t IDR, ODR, BSRR, LCKR, AFR[2];
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t AHB1ENR, AHB2ENR, AHB3ENR, RESERVED;
    volatile uint32_t APB1ENR;
} RCC_TypeDef;

typedef struct {
    volatile uint32_t SR, DR, BRR, CR1, CR2, CR3;
} USART_TypeDef;

typedef struct {
    volatile uint32_t CR, NDTR, PAR, M0AR, M1AR, FCR;
} DMA_Stream_TypeDef;

typedef struct {
    volatile uint32_t LISR, HISR, LIFCR, HIFCR;
} DMA_TypeDef;

extern GPIO_TypeDef mock_gpioa;
extern RCC_TypeDef mock_rcc;
extern USART_TypeDef mock_usart2;
extern DMA_Stream_TypeDef mock_dma1_stream6;
extern DMA_TypeDef mock_dma1;
extern uint32_t mock_primask;

#define GPIOA (&mock_gpioa)
#define RCC (&mock_rcc)
#define USART2 (&mock_usart2)
#define DMA1_Stream6 (&mock_dma1_stream6)
#define DMA1 (&mock_dma1)

#define RCC_AHB1ENR_GPIOAEN (1u << 0)
#define RCC_AHB1ENR_DMA1EN (1u << 21)
#define RCC_APB1ENR_USART2EN (1u << 17)

#define USART_CR1_UE (1u << 13)
#define USART_CR1_TE (1u << 3)
#define USART_CR1_RE (1u << 2)
#define USART_CR1_RXNEIE (1u << 5)
#define USART_CR3_DMAT (1u << 7)
#define USART_SR_RXNE (1u << 5)
#define USART_SR_ORE (1u << 3)

#define DMA_SxCR_EN (1u << 0)
#define DMA_SxCR_DMEIE (1u << 1)
#define DMA_SxCR_TEIE (1u << 2)
#define DMA_SxCR_DIR_Pos 6u
#define DMA_SxCR_MINC (1u << 10)
#define DMA_SxCR_TCIE (1u << 4)
#define DMA_SxCR_CHSEL_Pos 25u

#define USART2_IRQn 38
#define DMA1_Stream6_IRQn 17

static inline uint32_t __get_PRIMASK(void) { return mock_primask; }
static inline void __disable_irq(void) { mock_primask = 1u; }
static inline void __set_PRIMASK(uint32_t value) { mock_primask = value; }
static inline void NVIC_SetPriority(int irq, uint32_t priority) { (void)irq; (void)priority; }
static inline void NVIC_EnableIRQ(int irq) { (void)irq; }

#endif
