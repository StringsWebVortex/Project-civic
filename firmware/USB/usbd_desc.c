/* usbd_desc.c - USB descriptors for the Project-Civic CDC virtual COM port.
 * VID/PID 0x0483:0x5740 (ST CDC VCP) so the host mounts a standard serial port
 * for SavvyCAN (GVRET over the virtual COM). Based on the STM32CubeF4 template.
 */
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"

#define USBD_VID            0x0483
#define USBD_PID            0x5740
#define USBD_LANGID         0x0409
#define USBD_MANUF          "Project-Civic"
#define USBD_PRODUCT        "Civic CAN GVRET (CDC)"
#define USBD_CFG_STR        "CDC Config"
#define USBD_IF_STR         "CDC Interface"

static uint8_t *Get_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *len);
static uint8_t *Get_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *len);
static uint8_t *Get_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *len);
static uint8_t *Get_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *len);
static uint8_t *Get_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *len);
static uint8_t *Get_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *len);
static uint8_t *Get_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *len);

USBD_DescriptorsTypeDef CDC_Desc = {
    Get_DeviceDescriptor, Get_LangIDStrDescriptor, Get_ManufacturerStrDescriptor,
    Get_ProductStrDescriptor, Get_SerialStrDescriptor, Get_ConfigStrDescriptor,
    Get_InterfaceStrDescriptor,
};

__ALIGN_BEGIN static uint8_t DeviceDesc[18] __ALIGN_END = {
    0x12, USB_DESC_TYPE_DEVICE, 0x00, 0x02, 0x02, 0x02, 0x00, USB_MAX_EP0_SIZE,
    LOBYTE(USBD_VID), HIBYTE(USBD_VID), LOBYTE(USBD_PID), HIBYTE(USBD_PID),
    0x00, 0x02, 1, 2, 3, USBD_MAX_NUM_CONFIGURATION
};
__ALIGN_BEGIN static uint8_t LangIDDesc[4] __ALIGN_END = { 4, USB_DESC_TYPE_STRING, LOBYTE(USBD_LANGID), HIBYTE(USBD_LANGID) };
__ALIGN_BEGIN static uint8_t StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;
__ALIGN_BEGIN static uint8_t SerialStr[26] __ALIGN_END = { 26, USB_DESC_TYPE_STRING };

static void u32_to_unicode(uint32_t v, uint8_t *p, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        uint8_t n = (v >> 28) & 0xF;
        p[2*i] = (uint8_t)(n < 10 ? n + '0' : n + 'A' - 10);
        p[2*i+1] = 0;
        v <<= 4;
    }
}

static uint8_t *Get_DeviceDescriptor(USBD_SpeedTypeDef s, uint16_t *len) { (void)s; *len = sizeof DeviceDesc; return DeviceDesc; }
static uint8_t *Get_LangIDStrDescriptor(USBD_SpeedTypeDef s, uint16_t *len) { (void)s; *len = sizeof LangIDDesc; return LangIDDesc; }
static uint8_t *Get_ProductStrDescriptor(USBD_SpeedTypeDef s, uint16_t *len) { (void)s; USBD_GetString((uint8_t*)USBD_PRODUCT, StrDesc, len); return StrDesc; }
static uint8_t *Get_ManufacturerStrDescriptor(USBD_SpeedTypeDef s, uint16_t *len) { (void)s; USBD_GetString((uint8_t*)USBD_MANUF, StrDesc, len); return StrDesc; }
static uint8_t *Get_ConfigStrDescriptor(USBD_SpeedTypeDef s, uint16_t *len) { (void)s; USBD_GetString((uint8_t*)USBD_CFG_STR, StrDesc, len); return StrDesc; }
static uint8_t *Get_InterfaceStrDescriptor(USBD_SpeedTypeDef s, uint16_t *len) { (void)s; USBD_GetString((uint8_t*)USBD_IF_STR, StrDesc, len); return StrDesc; }
static uint8_t *Get_SerialStrDescriptor(USBD_SpeedTypeDef s, uint16_t *len)
{
    (void)s; *len = sizeof SerialStr;
    uint32_t a = *(uint32_t*)0x1FFF7A10, b = *(uint32_t*)0x1FFF7A14, c = *(uint32_t*)0x1FFF7A18;
    a += c;
    u32_to_unicode(a, &SerialStr[2], 8);
    u32_to_unicode(b, &SerialStr[18], 4);
    return SerialStr;
}
