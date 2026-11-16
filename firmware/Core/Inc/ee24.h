#ifndef EE24_H
#define EE24_H
/* Microchip 24LC512 (DS20001754): 64 KB I2C EEPROM, control code 1010 A2 A1 A0,
 * two address bytes, 128-byte page write buffer (a write wraps inside its
 * page), 5 ms max write cycle (acknowledge polling ends it early). The bus
 * transport is injected so the logic is host-testable. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define EE24_SIZE      65536u
#define EE24_PAGE      128u
#define EE24_TWC_MS    5u

typedef struct {
    /* write: addr7, w1[n1] then w2[n2] in one transaction; false on NACK/timeout */
    bool (*write)(uint8_t addr7, const uint8_t *w1, size_t n1, const uint8_t *w2, size_t n2);
    /* read: write w[nw] (no stop), repeated start, read r[nr] */
    bool (*read)(uint8_t addr7, const uint8_t *w, size_t nw, uint8_t *r, size_t nr);
    uint32_t (*now_ms)(void);
    uint8_t addr7;                     /* 0x50 with A2..A0 tied to VSS */
} ee24_t;

bool ee24_present(const ee24_t *e);                                  /* answers its address */
bool ee24_read(const ee24_t *e, uint16_t mem, uint8_t *buf, size_t n);
/* Splits at page boundaries and acknowledge-polls each page write. */
bool ee24_write(const ee24_t *e, uint16_t mem, const uint8_t *buf, size_t n);
#endif
