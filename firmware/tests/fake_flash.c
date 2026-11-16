/* Host model of the STM32F407 upper flash (sectors 6-11) with NOR rules:
 * programming can only clear bits, erase sets a whole 128 KB sector to 0xFF. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flash_store.h"

#define FAKE_BASE 0x08040000u
#define FAKE_SIZE (768u * 1024u)
uint8_t  fake_flash[FAKE_SIZE];
uint32_t fake_erases;
int      fake_fail_after = -1;      /* power-cut model: fail programs after N bytes */

const uint8_t *fs_ptr(uint32_t addr)
{
    if (addr < FAKE_BASE || addr >= FAKE_BASE + FAKE_SIZE) { fprintf(stderr, "fs_ptr out of range %08x\n", addr); abort(); }
    return &fake_flash[addr - FAKE_BASE];
}

void fs_init(can_capture_queue_t *a, can_capture_queue_t *b) { (void)a; (void)b; }
uint32_t fs_erase_count(void) { return fake_erases; }
uint32_t fs_frames_during_erase(void) { return 0; }

bool fs_erase_128k(uint32_t addr)
{
    if (addr < FAKE_BASE || (addr - FAKE_BASE) % FS_SECTOR_128K || addr + FS_SECTOR_128K > FAKE_BASE + FAKE_SIZE) return false;
    memset(&fake_flash[addr - FAKE_BASE], 0xFF, FS_SECTOR_128K);
    fake_erases++;
    return true;
}

bool fs_program(uint32_t addr, const void *src, size_t len)
{
    if (addr < FAKE_BASE || addr + len > FAKE_BASE + FAKE_SIZE) return false;
    const uint8_t *p = (const uint8_t *)src;
    for (size_t i = 0; i < len; i++) {
        if (fake_fail_after == 0) return false;
        if (fake_fail_after > 0) fake_fail_after--;
        uint8_t *d = &fake_flash[addr - FAKE_BASE + i];
        if ((*d & p[i]) != p[i]) { fprintf(stderr, "NOR violation at %08zx\n", (size_t)(addr + i)); abort(); }
        *d = p[i];
    }
    return true;
}

void fs_apply_staged_image(uint32_t size) { (void)size; fprintf(stderr, "apply called\n"); exit(3); }

void fake_flash_reset(void) { memset(fake_flash, 0xFF, sizeof fake_flash); fake_erases = 0; fake_fail_after = -1; }
