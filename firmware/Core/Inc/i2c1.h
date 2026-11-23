#ifndef I2C1_H
#define I2C1_H
/* I2C1 master on PB6 (SCL) / PB9 (SDA), AF4 (DS8626 Table 9), 100 kHz
 * standard mode. MB997 has 4.7 k pull-ups to VDD on both lines (R29/R33,
 * MB997 sheet 7); the CS43L22 (0x4A) shares the bus and is held in reset. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool i2c1_init(void);
bool i2c1_write(uint8_t addr7, const uint8_t *w1, size_t n1, const uint8_t *w2, size_t n2);
bool i2c1_read(uint8_t addr7, const uint8_t *w, size_t nw, uint8_t *r, size_t nr);
uint32_t i2c1_now_ms(void);
#endif
