#ifndef FLASH_STORE_H
#define FLASH_STORE_H
/* Internal-flash storage that replaces the ESP32 LittleFS partition.
 *
 * Layout (STM32F407VG, 1 MiB single bank):
 *   sectors 0-5   0x08000000 256 KB  application (linker-limited)
 *   sectors 6-7   0x08040000 256 KB  dashboard firmware-update staging
 *   sectors 8-9   0x08080000 2x128KB log.csv / log.1.csv (rotating)
 *   sectors 10-11 0x080C0000 2x128KB DTC event journal + snapshots
 *
 * A sector erase stalls flash for ~1-2 s on this single-bank part. The erase
 * runs from RAM with interrupts masked and keeps draining both bxCAN RX FIFOs
 * into the capture queues and feeding the watchdog, so CAN capture continues.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_frame_queue.h"

/* Memory-mapped read access to a flash address (host tests map it to RAM). */
#ifdef FS_HOST_TEST
const uint8_t *fs_ptr(uint32_t addr);
#else
static inline const uint8_t *fs_ptr(uint32_t addr) { return (const uint8_t *)(uintptr_t)addr; }
#endif

#define FS_STAGING_ADDR   0x08040000u
#define FS_STAGING_SIZE   (256u * 1024u)
#define FS_APP_MAX_SIZE   (256u * 1024u)
#define FS_LOG_A          0x08080000u
#define FS_LOG_B          0x080A0000u
#define FS_EVT_A          0x080C0000u
#define FS_EVT_B          0x080E0000u
#define FS_SECTOR_128K    (128u * 1024u)

/* Queues the RAM erase loop feeds while flash is busy (bus 0 = CAN1 F-CAN). */
void fs_init(can_capture_queue_t *q_fcan, can_capture_queue_t *q_bcan);

/* Erase the 128 KB sector that starts at addr (sectors 5..11 only). */
bool fs_erase_128k(uint32_t addr);
/* Program bytes into erased flash; never touches sectors 0-5. */
bool fs_program(uint32_t addr, const void *src, size_t len);
/* Number of sector erases performed and CAN frames captured during them. */
uint32_t fs_erase_count(void);
uint32_t fs_frames_during_erase(void);

/* Copy the staged image (sectors 6-7) over the application and reset.
 * Runs entirely from RAM; does not return. Caller validates first. */
void fs_apply_staged_image(uint32_t size) __attribute__((noreturn));

#endif
