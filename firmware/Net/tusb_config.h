/* TinyUSB configuration: STM32F407 OTG-FS (MB997 CN5, PA11/PA12) as a
 * USB CDC-NCM network adapter. The STM32 then serves the dashboard (HTTP 80,
 * WebSocket 81), GVRET TCP 23 and the observer TCP 2323 itself. */
#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

#include "lwipopts.h"

#define CFG_TUSB_MCU               OPT_MCU_STM32F4
#define CFG_TUSB_OS                OPT_OS_NONE
#define CFG_TUSB_DEBUG             0
#define BOARD_TUD_RHPORT           0
#define CFG_TUSB_RHPORT0_MODE      (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)
#define CFG_TUD_ENABLED            1
#define CFG_TUD_MAX_SPEED          OPT_MODE_FULL_SPEED
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN         __attribute__((aligned(4)))
#define CFG_TUD_ENDPOINT0_SIZE     64

#define CFG_TUD_ECM_RNDIS          0
#define CFG_TUD_NCM                1
#define CFG_TUD_NCM_IN_NTB_MAX_SIZE  (2 * TCP_MSS + 100)
#define CFG_TUD_NCM_OUT_NTB_MAX_SIZE (2 * TCP_MSS + 100)
#define CFG_TUD_NCM_OUT_NTB_N      1
#define CFG_TUD_NCM_IN_NTB_N       1

#endif
