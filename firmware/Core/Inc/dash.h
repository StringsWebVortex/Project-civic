#ifndef DASH_H
#define DASH_H
/* Single-board dashboard: the STM32 performs every duty the ESP32 dashboard
 * firmware performed (esp32-dashboard/src/main.cpp), over USB networking
 * instead of Wi-Fi. Every STM32 output line feeds the same parser the ESP32
 * ran on its UART; every dashboard command becomes the same STM32 command
 * line the ESP32 used to send. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_frame_queue.h"

typedef void (*dash_cmd_fn)(const char *line);   /* line without newline */

bool dash_init(dash_cmd_fn cmd, can_capture_queue_t *q_fcan, can_capture_queue_t *q_bcan);
void dash_on_line(const char *line);             /* every line the STM32 emits */
void dash_poll(void);
/* Virtual WebSocket peer used by the ESP32 Wi-Fi bridge over USART2. */
void dash_bridge_connected(void);
void dash_bridge_disconnected(void);
void dash_bridge_text(char *msg, size_t len);
#ifdef FS_HOST_TEST
void dash_test_fail_save_events(bool fail);
void dash_test_fail_event_rewrite(bool fail);
void dash_test_run_backup_service(void);
void dash_test_capture_post_event_sample(void);
bool dash_test_snapshot_ready(uint32_t id);
uint32_t dash_test_latest_event_id(void);
void dash_test_reset_frame_table(void);
#endif
/* Device profiles / target / learned grades changed: mirror to the EEPROM. */
void dash_identity_changed(void);
/* Live bus state (bitrate, listen-only); false if the bus is not running. */
bool project_bus_state(uint8_t bus, uint32_t *bit_rate, bool *listen_only);

#endif
