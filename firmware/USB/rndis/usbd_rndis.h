/* usbd_rndis.h - RNDIS USB class for STM32 USB Device Library.
 * Compatible with Android, Linux (rndis_host) and Windows. */
#ifndef USBD_RNDIS_H
#define USBD_RNDIS_H

#include "usbd_ioreq.h"
#include <stdbool.h>

/* ── RNDIS message types ─────────────────────────────────────────────── */
#define RNDIS_MSG_INIT              0x00000002u
#define RNDIS_MSG_INIT_C            0x80000002u
#define RNDIS_MSG_HALT              0x00000003u
#define RNDIS_MSG_QUERY             0x00000004u
#define RNDIS_MSG_QUERY_C           0x80000004u
#define RNDIS_MSG_SET               0x00000005u
#define RNDIS_MSG_SET_C             0x80000005u
#define RNDIS_MSG_RESET             0x00000006u
#define RNDIS_MSG_RESET_C           0x80000006u
#define RNDIS_MSG_INDICATE          0x00000007u
#define RNDIS_MSG_KEEPALIVE         0x00000008u
#define RNDIS_MSG_KEEPALIVE_C       0x80000008u
#define RNDIS_MSG_PACKET            0x00000001u

/* ── RNDIS OIDs (subset needed) ──────────────────────────────────────── */
#define OID_GEN_SUPPORTED_LIST          0x00010101u
#define OID_GEN_HARDWARE_STATUS         0x00010102u
#define OID_GEN_MEDIA_SUPPORTED         0x00010103u
#define OID_GEN_MEDIA_IN_USE            0x00010104u
#define OID_GEN_MAXIMUM_FRAME_SIZE      0x00010106u
#define OID_GEN_LINK_SPEED             0x00010107u
#define OID_GEN_TRANSMIT_BLOCK_SIZE    0x0001010Au
#define OID_GEN_RECEIVE_BLOCK_SIZE     0x0001010Bu
#define OID_GEN_VENDOR_ID              0x0001010Cu
#define OID_GEN_VENDOR_DESCRIPTION     0x0001010Du
#define OID_GEN_CURRENT_PACKET_FILTER  0x0001010Eu
#define OID_GEN_MAXIMUM_TOTAL_SIZE     0x00010111u
#define OID_GEN_MEDIA_CONNECT_STATUS   0x00010114u
#define OID_GEN_PHYSICAL_MEDIUM        0x00010202u
#define OID_GEN_XMIT_OK               0x00020101u
#define OID_GEN_RCV_OK                 0x00020102u
#define OID_GEN_XMIT_ERROR            0x00020104u
#define OID_GEN_RCV_ERROR             0x00020105u
#define OID_GEN_RCV_NO_BUFFER         0x00020106u
#define OID_802_3_PERMANENT_ADDRESS    0x01010101u
#define OID_802_3_CURRENT_ADDRESS      0x01010102u
#define OID_802_3_MULTICAST_LIST       0x01010103u
#define OID_802_3_MAXIMUM_LIST_SIZE    0x01010104u
#define OID_802_3_RCV_ERROR_ALIGNMENT  0x01020101u
#define OID_802_3_XMIT_ONE_COLLISION  0x01020102u
#define OID_802_3_XMIT_MORE_COLLISIONS 0x01020103u

#define RNDIS_STATUS_SUCCESS       0x00000000u
#define RNDIS_STATUS_FAILURE       0xC0000001u
#define RNDIS_STATUS_NOT_SUPPORTED 0xC00000BBu

#define RNDIS_MEDIUM_802_3         0x00000000u
#define RNDIS_DF_CONNECTIONLESS    0x00000001u
#define RNDIS_MAJOR_VERSION        1u
#define RNDIS_MINOR_VERSION        0u

#define RNDIS_ETH_FRAME_MAX        1514u
#define RNDIS_HEADER_SIZE          44u     /* RNDIS_MSG_PACKET header */
#define RNDIS_RX_BUF_SIZE          (RNDIS_ETH_FRAME_MAX + RNDIS_HEADER_SIZE + 64u)
#define RNDIS_TX_BUF_SIZE          (RNDIS_ETH_FRAME_MAX + RNDIS_HEADER_SIZE)
#define RNDIS_RESP_BUF_SIZE        256u

/* Endpoints */
#define RNDIS_NOTIFICATION_EP      0x81u   /* IN  interrupt (CDC notification) */
#define RNDIS_DATA_IN_EP           0x82u   /* IN  bulk */
#define RNDIS_DATA_OUT_EP          0x02u   /* OUT bulk */
#define RNDIS_NOTIFICATION_SZ      8u
#define RNDIS_DATA_FS_MPS          64u     /* full-speed max packet size */

/* ── Handle ──────────────────────────────────────────────────────────── */
typedef struct {
    uint8_t  mac_host[6];
    uint8_t  mac_dev[6];
    uint32_t packet_filter;
    bool     link_up;
    bool     init_done;

    /* control channel */
    __ALIGN_BEGIN uint8_t resp_buf[RNDIS_RESP_BUF_SIZE] __ALIGN_END;
    uint32_t resp_len;
    bool     resp_avail;         /* notification pending */

    /* data channel - rx_buf stays in main SRAM, tx_buf allocated externally (CCM) */
    __ALIGN_BEGIN uint8_t rx_buf[RNDIS_RX_BUF_SIZE] __ALIGN_END;
    volatile uint32_t rx_len;
    volatile bool     rx_ready;

    uint8_t *tx_buf;             /* points to external CCM-RAM buffer */
    volatile bool tx_busy;

    /* stats */
    uint32_t tx_ok, rx_ok, tx_err, rx_err, rx_drop;
} usbd_rndis_t;

/* ── Class API ───────────────────────────────────────────────────────── */
extern USBD_ClassTypeDef USBD_RNDIS_ClassDriver;
#define USBD_RNDIS_CLASS &USBD_RNDIS_ClassDriver

/* Access the handle from the USBD core */
static inline usbd_rndis_t *usbd_rndis_handle(USBD_HandleTypeDef *pdev) {
    return (usbd_rndis_t *)pdev->pClassDataCmsit[0];
}

/* Called by net_usb after wrapping eth frame in RNDIS packet header */
USBD_StatusTypeDef usbd_rndis_transmit(USBD_HandleTypeDef *pdev, const uint8_t *eth, uint16_t len);

/* Get pointer + length of received eth frame (strips RNDIS header); call
 * usbd_rndis_rx_done() after processing to re-arm the OUT endpoint. */
bool    usbd_rndis_rx_get(USBD_HandleTypeDef *pdev, const uint8_t **frame, uint16_t *len);
void    usbd_rndis_rx_done(USBD_HandleTypeDef *pdev);

#endif /* USBD_RNDIS_H */
