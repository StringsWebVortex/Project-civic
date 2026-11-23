/* usbd_desc_rndis.c - device and string descriptors for the RNDIS network
 * adapter. VID 0x0483 (STMicroelectronics); RNDIS class codes in IAD. */
#include "usbd_core.h"
#include "usbd_conf.h"

#define USBD_VID      0x0483
#define USBD_PID      0x5740
#define USBD_LANGID   0x0409
#define USBD_MANUF    "Project Civic"
#define USBD_PRODUCT  "Civic CAN Monitor"
#define USBD_CFG_STR  "RNDIS"
#define USBD_IF_STR   "Civic CAN Monitor Network"

static uint8_t *dev_desc(USBD_SpeedTypeDef s, uint16_t *len);
static uint8_t *lang_desc(USBD_SpeedTypeDef s, uint16_t *len);
static uint8_t *manuf_desc(USBD_SpeedTypeDef s, uint16_t *len);
static uint8_t *prod_desc(USBD_SpeedTypeDef s, uint16_t *len);
static uint8_t *serial_desc(USBD_SpeedTypeDef s, uint16_t *len);
static uint8_t *cfg_desc(USBD_SpeedTypeDef s, uint16_t *len);
static uint8_t *if_desc(USBD_SpeedTypeDef s, uint16_t *len);

USBD_DescriptorsTypeDef RNDIS_Desc = { dev_desc, lang_desc, manuf_desc, prod_desc, serial_desc, cfg_desc, if_desc };

__ALIGN_BEGIN static uint8_t DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = {
    0x12, USB_DESC_TYPE_DEVICE, 0x00, 0x02,
    0xEF, 0x02, 0x01,                          /* IAD: misc / common / IAD */
    USB_MAX_EP0_SIZE,
    LOBYTE(USBD_VID), HIBYTE(USBD_VID), LOBYTE(USBD_PID), HIBYTE(USBD_PID),
    0x00, 0x02, USBD_IDX_MFC_STR, USBD_IDX_PRODUCT_STR, USBD_IDX_SERIAL_STR, USBD_MAX_NUM_CONFIGURATION
};
__ALIGN_BEGIN static uint8_t LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END = {
    USB_LEN_LANGID_STR_DESC, USB_DESC_TYPE_STRING, LOBYTE(USBD_LANGID), HIBYTE(USBD_LANGID)
};
__ALIGN_BEGIN static uint8_t StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;
__ALIGN_BEGIN static uint8_t SerialStr[2u + 48u] __ALIGN_END;

static uint8_t *dev_desc(USBD_SpeedTypeDef s, uint16_t *len)   { (void)s; *len = sizeof DeviceDesc; return DeviceDesc; }
static uint8_t *lang_desc(USBD_SpeedTypeDef s, uint16_t *len)  { (void)s; *len = sizeof LangIDDesc; return LangIDDesc; }
static uint8_t *manuf_desc(USBD_SpeedTypeDef s, uint16_t *len) { (void)s; USBD_GetString((uint8_t *)USBD_MANUF, StrDesc, len); return StrDesc; }
static uint8_t *prod_desc(USBD_SpeedTypeDef s, uint16_t *len)  { (void)s; USBD_GetString((uint8_t *)USBD_PRODUCT, StrDesc, len); return StrDesc; }
static uint8_t *cfg_desc(USBD_SpeedTypeDef s, uint16_t *len)   { (void)s; USBD_GetString((uint8_t *)USBD_CFG_STR, StrDesc, len); return StrDesc; }
static uint8_t *if_desc(USBD_SpeedTypeDef s, uint16_t *len)    { (void)s; USBD_GetString((uint8_t *)USBD_IF_STR, StrDesc, len); return StrDesc; }

static uint8_t *serial_desc(USBD_SpeedTypeDef s, uint16_t *len)
{
    (void)s;
    const volatile uint8_t *uid = (const volatile uint8_t *)UID_BASE;
    uint8_t n = 2u;
    for (uint8_t i = 0u; i < 12u; i++) {
        SerialStr[n++] = (uint8_t)"0123456789ABCDEF"[uid[i] >> 4]; SerialStr[n++] = 0u;
        SerialStr[n++] = (uint8_t)"0123456789ABCDEF"[uid[i] & 15u]; SerialStr[n++] = 0u;
    }
    SerialStr[0] = n; SerialStr[1] = USB_DESC_TYPE_STRING;
    *len = n;
    return SerialStr;
}
