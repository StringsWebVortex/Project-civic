/* usbd_conf.h - USB Device library configuration for Project-Civic (OTG_FS, CDC).
 * Static allocation (no heap). Based on the STM32CubeF4 CDC VCP template. */
#ifndef USBD_CONF_H
#define USBD_CONF_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx_hal.h"

#define USBD_MAX_NUM_INTERFACES       1U
#define USBD_MAX_NUM_CONFIGURATION    1U
#define USBD_MAX_STR_DESC_SIZ         512U
#define USBD_DEBUG_LEVEL              0U
#define USBD_SELF_POWERED             1U
#define USBD_LPM_ENABLED              0U
#define DEVICE_FS                     0

/* Static memory management (single CDC handle pool) */
void *USBD_static_malloc(uint32_t size);
void  USBD_static_free(void *p);
#define USBD_malloc   USBD_static_malloc
#define USBD_free     USBD_static_free
#define USBD_memset   memset
#define USBD_memcpy   memcpy
#define USBD_Delay    HAL_Delay

#if (USBD_DEBUG_LEVEL > 0U)
#define USBD_UsrLog(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#else
#define USBD_UsrLog(...)
#endif
#define USBD_ErrLog(...)
#define USBD_DbgLog(...)

#endif /* USBD_CONF_H */
