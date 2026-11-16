/* usbd_cdc_if.c - CDC VCP interface: received bytes go to the GVRET bridge,
 * CDC_Transmit_FS sends GVRET bytes back to the host (SavvyCAN). */
#include "usbd_cdc_if.h"
#include "usb_gvret.h"

#define APP_RX_DATA_SIZE 512U
#define APP_TX_DATA_SIZE 512U

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint8_t UserRxBuffer[APP_RX_DATA_SIZE];
static uint8_t UserTxBuffer[APP_TX_DATA_SIZE];

/* Minimal CDC line coding store (SavvyCAN sets it but the value is irrelevant
 * to a USB CDC endpoint). */
static uint8_t LineCoding[7] = { 0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08 };

static int8_t CDC_Init_FS(void)
{
    if (USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBuffer, 0) != USBD_OK ||
        USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBuffer) != USBD_OK)
        return (int8_t)USBD_FAIL;
    return (int8_t)USBD_OK;
}
static int8_t CDC_DeInit_FS(void)
{
    usb_gvret_on_disconnect();
    return (int8_t)USBD_OK;
}

static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
    switch (cmd) {
    case CDC_SET_LINE_CODING:
        if (length >= sizeof LineCoding) for (uint16_t i = 0; i < sizeof LineCoding; i++) LineCoding[i] = pbuf[i];
        break;
    case CDC_GET_LINE_CODING:
        if (length >= sizeof LineCoding) for (uint16_t i = 0; i < sizeof LineCoding; i++) pbuf[i] = LineCoding[i];
        break;
    default: break;
    }
    return (int8_t)USBD_OK;
}

static int8_t CDC_Receive_FS(uint8_t *Buf, uint32_t *Len)
{
    if (!Buf || !Len) return (int8_t)USBD_FAIL;
    usb_gvret_rx(Buf, *Len);
    if (USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBuffer) != USBD_OK ||
        USBD_CDC_ReceivePacket(&hUsbDeviceFS) != USBD_OK)
        return (int8_t)USBD_FAIL;
    return (int8_t)USBD_OK;
}

static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
    (void)Buf; (void)Len; (void)epnum;
    usb_gvret_tx_complete();
    return (int8_t)USBD_OK;
}

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = {
    CDC_Init_FS, CDC_DeInit_FS, CDC_Control_FS, CDC_Receive_FS, CDC_TransmitCplt_FS
};

uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len)
{
    USBD_CDC_HandleTypeDef *h = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
    if (!Buf || Len == 0u) return USBD_FAIL;
    if (h == NULL || h->TxState != 0U) return USBD_BUSY;
    if (USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len) != USBD_OK)
        return USBD_FAIL;
    return USBD_CDC_TransmitPacket(&hUsbDeviceFS);
}
