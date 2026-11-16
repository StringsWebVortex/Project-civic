/* Host shim: just the registers dash.c touches. */
#ifndef HOST_SHIM_STM32F4XX_H
#define HOST_SHIM_STM32F4XX_H
#include <stdint.h>
typedef struct { volatile uint32_t CNT; } shim_tim_t;
typedef struct { volatile uint32_t CR, SR, DR; } shim_rng_t;
typedef struct { volatile uint32_t AHB2ENR; } shim_rcc_t;
extern shim_tim_t shim_tim2; extern shim_rng_t shim_rng; extern shim_rcc_t shim_rcc;
#define TIM2 (&shim_tim2)
#define RNG  (&shim_rng)
#define RCC  (&shim_rcc)
#define RNG_CR_RNGEN 4u
#define RNG_SR_DRDY  1u
#define RCC_AHB2ENR_RNGEN (1u << 6)
static inline uint32_t __get_IPSR(void) { return 0u; }
#endif
