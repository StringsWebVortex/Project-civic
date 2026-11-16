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

#endif
