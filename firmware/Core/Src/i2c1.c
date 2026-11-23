/* I2C1 polled master (RM0090 27.3.3 master mode), see i2c1.h. */
#include "i2c1.h"
#include "stm32f4xx.h"

extern volatile uint32_t system_millis;
uint32_t i2c1_now_ms(void) { return system_millis; }

#define I2C_TIMEOUT_MS 5u

/* Bounded by time AND by iterations: the tick does not advance while
 * interrupts are disabled (last-byte window), the iteration cap still does
 * (~1.2 M loops is well over 5 ms at 168 MHz). */
static bool wait_sr1(uint32_t mask)
{
    const uint32_t t0 = system_millis;
    for (uint32_t spins = 0u; (I2C1->SR1 & mask) == 0u; spins++) {
        if (I2C1->SR1 & I2C_SR1_AF) return false;            /* NACK */
        if ((uint32_t)(system_millis - t0) > I2C_TIMEOUT_MS || spins > 1200000u) return false;
    }
    return true;
}

static void stop_and_clear(void)
{
    I2C1->CR1 |= I2C_CR1_STOP;
    I2C1->SR1 &= ~(I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR);
    const uint32_t t0 = system_millis;
    while ((I2C1->SR2 & I2C_SR2_MSL) && (uint32_t)(system_millis - t0) <= I2C_TIMEOUT_MS) { }
}

static void pin_od(uint8_t pin, uint32_t mode)
{
    GPIOB->OTYPER |= (1u << pin);                              /* open-drain */
    GPIOB->PUPDR  &= ~(3u << (2u * pin));                      /* board has 4.7 k pull-ups */
    GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~(3u << (2u * pin))) | (1u << (2u * pin));
    const uint8_t r = pin >> 3, sh = (uint8_t)((pin & 7u) * 4u);
    GPIOB->AFR[r] = (GPIOB->AFR[r] & ~(15u << sh)) | (4u << sh);   /* AF4 = I2C1 */
    GPIOB->MODER  = (GPIOB->MODER & ~(3u << (2u * pin))) | (mode << (2u * pin));
}

/* A slave left holding SDA low (reset mid-byte): clock SCL up to 9 times. */
static void bus_recover(void)
{
    pin_od(6u, 1u); pin_od(9u, 0u);                            /* SCL output, SDA input */
    for (int i = 0; i < 9 && !(GPIOB->IDR & (1u << 9)); i++) {
        GPIOB->BSRR = (1u << (6u + 16u)); for (volatile int d = 0; d < 400; d++) { }
        GPIOB->BSRR = (1u << 6u);         for (volatile int d = 0; d < 400; d++) { }
    }
}

bool i2c1_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; (void)RCC->AHB1ENR;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;  (void)RCC->APB1ENR;
    GPIOB->BSRR = (1u << 6u) | (1u << 9u);
    bus_recover();
    pin_od(6u, 2u); pin_od(9u, 2u);                            /* AF mode */
    I2C1->CR1 = I2C_CR1_SWRST; I2C1->CR1 = 0u;
    I2C1->CR2 = 42u;                                           /* PCLK1 = 42 MHz */
    I2C1->CCR = 210u;                                          /* Sm: 42 MHz / (2 * 100 kHz) */
    I2C1->TRISE = 43u;                                         /* 1000 ns * 42 MHz + 1 */
    I2C1->CR1 = I2C_CR1_PE;
    return (GPIOB->IDR & ((1u << 6u) | (1u << 9u))) == ((1u << 6u) | (1u << 9u));
}

static bool start(uint8_t addr7, bool rd)
{
    I2C1->CR1 |= I2C_CR1_START;
    if (!wait_sr1(I2C_SR1_SB)) return false;
    I2C1->DR = (uint32_t)((addr7 << 1) | (rd ? 1u : 0u));
    if (!wait_sr1(I2C_SR1_ADDR)) return false;
    return true;
}

static bool send(const uint8_t *d, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (!wait_sr1(I2C_SR1_TXE)) return false;
        I2C1->DR = d[i];
    }
    return true;
}

bool i2c1_write(uint8_t addr7, const uint8_t *w1, size_t n1, const uint8_t *w2, size_t n2)
{
    bool ok = start(addr7, false);
    if (ok) { (void)I2C1->SR2; ok = send(w1, n1) && send(w2, n2) && ((n1 + n2) == 0u || wait_sr1(I2C_SR1_BTF)); }
    stop_and_clear();
    return ok;
}

bool i2c1_read(uint8_t addr7, const uint8_t *w, size_t nw, uint8_t *r, size_t nr)
{
    bool ok = true;
    if (nw) {
        ok = start(addr7, false);
        if (ok) { (void)I2C1->SR2; ok = send(w, nw) && wait_sr1(I2C_SR1_BTF); }
    }
    if (ok && nr) {
        I2C1->CR1 |= I2C_CR1_ACK;
        ok = start(addr7, true);                               /* repeated start */
        if (ok) {
            if (nr == 1u) {                                    /* RM0090 27.3.3: NACK before ADDR clear */
                I2C1->CR1 &= ~I2C_CR1_ACK;
                __disable_irq(); (void)I2C1->SR2; I2C1->CR1 |= I2C_CR1_STOP; __enable_irq();
                ok = wait_sr1(I2C_SR1_RXNE);
                if (ok) r[0] = (uint8_t)I2C1->DR;
            } else {
                (void)I2C1->SR2;
                for (size_t i = 0; ok && i < nr; i++) {
                    /* The NACK for the last byte must be armed while it is still
                     * being shifted in (~90 us at 100 kHz): no interrupt may
                     * delay the window between byte nr-2 and clearing ACK. */
                    const bool window = (i == nr - 2u);
                    if (window) __disable_irq();
                    ok = wait_sr1(I2C_SR1_RXNE);
                    if (ok) r[i] = (uint8_t)I2C1->DR;
                    if (window) { I2C1->CR1 &= ~I2C_CR1_ACK; I2C1->CR1 |= I2C_CR1_STOP; __enable_irq(); }
                }
            }
        }
    }
    if (!ok || nr == 0u) stop_and_clear();
    else { const uint32_t t0 = system_millis; while ((I2C1->SR2 & I2C_SR2_MSL) && (uint32_t)(system_millis - t0) <= I2C_TIMEOUT_MS) { } }
    I2C1->CR1 &= ~I2C_CR1_ACK;
    return ok;
}
