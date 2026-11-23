/* Internal-flash driver for the single-board dashboard (see flash_store.h). */
#include "flash_store.h"
#include "stm32f4xx.h"

#define RAMFUNC __attribute__((section(".RamFunc"), noinline, long_call, used))

static can_capture_queue_t *s_q[2];
static uint32_t s_erases, s_erase_frames;

void fs_init(can_capture_queue_t *q_fcan, can_capture_queue_t *q_bcan)
{
    s_q[0] = q_fcan; s_q[1] = q_bcan;
}

uint32_t fs_erase_count(void) { return s_erases; }
uint32_t fs_frames_during_erase(void) { return s_erase_frames; }

static void flash_unlock(void)
{
    if (FLASH->CR & FLASH_CR_LOCK) { FLASH->KEYR = 0x45670123u; FLASH->KEYR = 0xCDEF89ABu; }
}
static void flash_lock(void) { FLASH->CR |= FLASH_CR_LOCK; }

/* Flush ART instruction/data caches so readers see the new flash content. */
static void flash_cache_reset(void)
{
    const uint32_t acr = FLASH->ACR;
    FLASH->ACR = acr & ~(FLASH_ACR_ICEN | FLASH_ACR_DCEN);
    FLASH->ACR |= FLASH_ACR_ICRST | FLASH_ACR_DCRST;
    FLASH->ACR &= ~(FLASH_ACR_ICRST | FLASH_ACR_DCRST);
    FLASH->ACR = acr;
}

/* ── RAM-resident pieces: no calls, no constant tables, no memcpy ─────── */
static RAMFUNC uint32_t ram_poll_fifo(CAN_TypeDef *can, can_capture_queue_t *q, uint32_t fifo,
                                      uint8_t bus, uint32_t now_ms)
{
    volatile uint32_t *rfr = fifo ? &can->RF1R : &can->RF0R;
    uint32_t n = 0;
    while (*rfr & 3u) {                       /* FMPx */
        const uint32_t rir  = can->sFIFOMailBox[fifo].RIR;
        const uint32_t rdtr = can->sFIFOMailBox[fifo].RDTR;
        const uint32_t low  = can->sFIFOMailBox[fifo].RDLR;
        const uint32_t high = can->sFIFOMailBox[fifo].RDHR;
        *rfr = (1u << 5);                     /* RFOMx: release mailbox */
        if (!q) continue;
        const uint16_t h = q->head;
        const uint16_t next = (uint16_t)((h + 1u) % CAN_CAPTURE_QUEUE_CAPACITY);
        if (next == q->tail) { q->dropped++; continue; }
        can_capture_frame_t *f = &q->frames[h];
        const uint32_t ext = (rir >> 2) & 1u, rtr = (rir >> 1) & 1u;
        uint32_t dlc = rdtr & 0x0Fu; if (dlc > 8u) dlc = 8u;
        f->timestamp_ms = now_ms;
        f->timestamp_us = TIM2->CNT;
        f->identifier = ext ? ((rir >> 3) & 0x1FFFFFFFu) : ((rir >> 21) & 0x7FFu);
        f->extended = (uint8_t)ext; f->remote = (uint8_t)rtr; f->dlc = (uint8_t)dlc; f->bus = bus;
        for (uint32_t i = 0; i < 8u; i++)
            f->data[i] = (uint8_t)(rtr || i >= dlc ? 0u : (i < 4u ? (low >> (i * 8u)) : (high >> ((i - 4u) * 8u))));
        q->head = next;
        n++;
    }
    return n;
}

static RAMFUNC uint32_t ram_erase_sector(uint32_t snb, uint32_t now_ms,
                                         can_capture_queue_t *q0, can_capture_queue_t *q1)
{
    uint32_t frames = 0;
    while (FLASH->SR & FLASH_SR_BSY) { }
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_OPERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR |
                FLASH_SR_PGPERR | FLASH_SR_PGSERR;
    FLASH->CR = (FLASH->CR & ~(FLASH_CR_PSIZE | FLASH_CR_SNB)) | FLASH_CR_PSIZE_1 |
                FLASH_CR_SER | (snb << FLASH_CR_SNB_Pos);
    FLASH->CR |= FLASH_CR_STRT;
    while (FLASH->SR & FLASH_SR_BSY) {
        IWDG->KR = 0xAAAAu;
        frames += ram_poll_fifo(CAN1, q0, 0u, 0u, now_ms);
        frames += ram_poll_fifo(CAN1, q0, 1u, 0u, now_ms);
        frames += ram_poll_fifo(CAN2, q1, 0u, 1u, now_ms);
        frames += ram_poll_fifo(CAN2, q1, 1u, 1u, now_ms);
    }
    FLASH->CR &= ~FLASH_CR_SER;
    return frames;
}

extern volatile uint32_t system_millis;

static int sector_of(uint32_t addr)
{
    if (addr < 0x08010000u) return (int)((addr - 0x08000000u) / 0x4000u);       /* 0-3 */
    if (addr < 0x08020000u) return 4;
    return (int)(5u + (addr - 0x08020000u) / FS_SECTOR_128K);                   /* 5-11 */
}

bool fs_erase_128k(uint32_t addr)
{
    if (addr < FS_STAGING_ADDR || addr > FS_EVT_B || (addr - 0x08020000u) % FS_SECTOR_128K) return false;
    const uint32_t snb = (uint32_t)sector_of(addr);
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();
    flash_unlock();
    s_erase_frames += ram_erase_sector(snb, system_millis, s_q[0], s_q[1]);
    const bool ok = (FLASH->SR & (FLASH_SR_OPERR | FLASH_SR_WRPERR | FLASH_SR_PGSERR)) == 0u;
    flash_lock();
    flash_cache_reset();
    if (!primask) __enable_irq();
    s_erases++;
    return ok;
}

bool fs_program(uint32_t addr, const void *src, size_t len)
{
    if (addr < FS_STAGING_ADDR || addr + len > 0x08100000u || addr + len < addr) return false;
    const uint8_t *p = (const uint8_t *)src;
    bool ok = true;
    flash_unlock();
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_OPERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR |
                FLASH_SR_PGPERR | FLASH_SR_PGSERR;
    FLASH->CR = (FLASH->CR & ~FLASH_CR_PSIZE);             /* x8: byte program */
    for (size_t i = 0; i < len && ok; i++) {
        volatile uint8_t *dst = (volatile uint8_t *)(addr + i);
        if (*dst == p[i]) continue;
        if (*dst != 0xFFu && (*dst & p[i]) != p[i]) { ok = false; break; }
        FLASH->CR |= FLASH_CR_PG;
        *dst = p[i];
        __DSB();
        while (FLASH->SR & FLASH_SR_BSY) { }
        FLASH->CR &= ~FLASH_CR_PG;
        if (FLASH->SR & (FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR | FLASH_SR_WRPERR)) ok = false;
    }
    flash_lock();
    flash_cache_reset();
    return ok;
}

/* ── staged-image apply: erase 0-5, copy 6-7 -> 0, reset; all from RAM ── */
static RAMFUNC __attribute__((noreturn)) void ram_apply(uint32_t words)
{
    while (FLASH->SR & FLASH_SR_BSY) { }
    for (uint32_t snb = 0; snb <= 5u; snb++) {
        FLASH->CR = (FLASH->CR & ~(FLASH_CR_PSIZE | FLASH_CR_SNB)) | FLASH_CR_PSIZE_1 |
                    FLASH_CR_SER | (snb << FLASH_CR_SNB_Pos);
        FLASH->CR |= FLASH_CR_STRT;
        while (FLASH->SR & FLASH_SR_BSY) IWDG->KR = 0xAAAAu;
    }
    FLASH->CR = (FLASH->CR & ~(FLASH_CR_PSIZE | FLASH_CR_SER)) | FLASH_CR_PSIZE_1 | FLASH_CR_PG;
    volatile uint32_t *dst = (volatile uint32_t *)0x08000000u;
    const volatile uint32_t *srcw = (const volatile uint32_t *)FS_STAGING_ADDR;
    for (uint32_t i = 0; i < words; i++) {
        dst[i] = srcw[i];
        while (FLASH->SR & FLASH_SR_BSY) IWDG->KR = 0xAAAAu;
    }
    FLASH->CR &= ~FLASH_CR_PG;
    FLASH->CR |= FLASH_CR_LOCK;
    __DSB();
    SCB->AIRCR = (0x5FAu << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk;
    __DSB();
    for (;;) { }
}

void fs_apply_staged_image(uint32_t size)
{
    __disable_irq();
    flash_unlock();
    ram_apply((size + 3u) / 4u);
}
