/* usbd_rndis.c - RNDIS USB class implementation for STM32F4 OTG-FS.
 * Implements the Microsoft RNDIS protocol over USB CDC (Abstract Control Model
 * with vendor-specific encapsulation). Works with Android, Linux, Windows. */
#include "usbd_rndis.h"
#include <string.h>

/* ── Descriptors ─────────────────────────────────────────────────────── */
/* Configuration descriptor: IAD + CDC control interface (notification EP) +
 * CDC data interface (bulk IN + bulk OUT). Total = 75 bytes. */
#define RNDIS_CFG_DESC_SIZE  75u

__ALIGN_BEGIN static uint8_t rndis_cfg_desc[RNDIS_CFG_DESC_SIZE] __ALIGN_END = {
    /* Configuration */
    0x09, USB_DESC_TYPE_CONFIGURATION, LOBYTE(RNDIS_CFG_DESC_SIZE), HIBYTE(RNDIS_CFG_DESC_SIZE),
    0x02, 0x01, 0x00, 0xC0, 0xFA,  /* 2 interfaces, self-powered, 500 mA */

    /* IAD: associates the two interfaces as one RNDIS function */
    0x08, 0x0B, 0x00, 0x02, 0xE0, 0x01, 0x03, 0x00,
    /* bFunctionClass=0xE0 (Wireless), bFunctionSubClass=0x01, bFunctionProto=0x03 (RNDIS) */

    /* Interface 0: CDC Communication (control) */
    0x09, USB_DESC_TYPE_INTERFACE, 0x00, 0x00, 0x01,
    0xE0, 0x01, 0x03, 0x00,       /* Wireless / RNDIS */

    /* CDC Header */
    0x05, 0x24, 0x00, 0x10, 0x01,
    /* CDC Call Management */
    0x05, 0x24, 0x01, 0x00, 0x01,
    /* CDC ACM */
    0x04, 0x24, 0x02, 0x00,
    /* CDC Union: master=0, slave=1 */
    0x05, 0x24, 0x06, 0x00, 0x01,

    /* Notification EP (IN interrupt) */
    0x07, USB_DESC_TYPE_ENDPOINT, RNDIS_NOTIFICATION_EP, 0x03,
    LOBYTE(RNDIS_NOTIFICATION_SZ), HIBYTE(RNDIS_NOTIFICATION_SZ), 0x01,

    /* Interface 1: CDC Data */
    0x09, USB_DESC_TYPE_INTERFACE, 0x01, 0x00, 0x02,
    0x0A, 0x00, 0x00, 0x00,       /* Data interface class */

    /* Bulk OUT EP */
    0x07, USB_DESC_TYPE_ENDPOINT, RNDIS_DATA_OUT_EP, 0x02,
    LOBYTE(RNDIS_DATA_FS_MPS), HIBYTE(RNDIS_DATA_FS_MPS), 0x00,
    /* Bulk IN EP */
    0x07, USB_DESC_TYPE_ENDPOINT, RNDIS_DATA_IN_EP, 0x02,
    LOBYTE(RNDIS_DATA_FS_MPS), HIBYTE(RNDIS_DATA_FS_MPS), 0x00,
};

/* OIDs we support */
static const uint32_t supported_oids[] = {
    OID_GEN_SUPPORTED_LIST, OID_GEN_HARDWARE_STATUS, OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE, OID_GEN_MAXIMUM_FRAME_SIZE, OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BLOCK_SIZE, OID_GEN_RECEIVE_BLOCK_SIZE, OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION, OID_GEN_CURRENT_PACKET_FILTER, OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MEDIA_CONNECT_STATUS, OID_GEN_PHYSICAL_MEDIUM,
    OID_GEN_XMIT_OK, OID_GEN_RCV_OK, OID_GEN_XMIT_ERROR, OID_GEN_RCV_ERROR, OID_GEN_RCV_NO_BUFFER,
    OID_802_3_PERMANENT_ADDRESS, OID_802_3_CURRENT_ADDRESS, OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE, OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION, OID_802_3_XMIT_MORE_COLLISIONS,
};
#define NUM_OIDS (sizeof supported_oids / sizeof supported_oids[0])

static const char vendor_desc[] = "Project Civic";

/* ── Helpers ─────────────────────────────────────────────────────────── */
static inline void put32le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v); p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}
static inline uint32_t get32le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Send a RESPONSE_AVAILABLE notification on the interrupt EP.
 * Per CDC 1.2 §6.3 / USB PSTN subclass §6.5.2, the notification is an 8-byte
 * packet on the interrupt IN pipe with the standard notification header:
 *   bmRequestType=0xA1, bNotificationCode=0x01 (RESPONSE_AVAILABLE),
 *   wValue=0, wIndex=0 (interface), wLength=0 */
static void notify_response(USBD_HandleTypeDef *pdev)
{
    static __ALIGN_BEGIN uint8_t note[8] __ALIGN_END = {
        0xA1,       /* bmRequestType: class, interface, device-to-host */
        0x01,       /* bNotification: RESPONSE_AVAILABLE */
        0x00, 0x00, /* wValue */
        0x00, 0x00, /* wIndex (interface 0) */
        0x00, 0x00  /* wLength */
    };
    (void)USBD_LL_Transmit(pdev, RNDIS_NOTIFICATION_EP, note, 8);
}

/* ── RNDIS message handlers ──────────────────────────────────────────── */
static void rndis_init_msg(usbd_rndis_t *h, const uint8_t *msg)
{
    uint32_t rid = get32le(msg + 8);  /* RequestId at offset 8 (after Type + Length) */
    uint8_t *r = h->resp_buf;
    memset(r, 0, 52);
    put32le(r + 0, RNDIS_MSG_INIT_C);
    put32le(r + 4, 52);                   /* MessageLength */
    put32le(r + 8, rid);                  /* RequestId */
    put32le(r + 12, RNDIS_STATUS_SUCCESS);
    put32le(r + 16, RNDIS_MAJOR_VERSION);
    put32le(r + 20, RNDIS_MINOR_VERSION);
    put32le(r + 24, RNDIS_DF_CONNECTIONLESS);
    put32le(r + 28, RNDIS_MEDIUM_802_3);
    put32le(r + 32, 1);                   /* MaxPacketsPerTransfer */
    put32le(r + 36, RNDIS_ETH_FRAME_MAX + RNDIS_HEADER_SIZE);
    put32le(r + 40, 0);                   /* PacketAlignmentFactor */
    put32le(r + 44, 0);                   /* Reserved */
    put32le(r + 48, 0);                   /* Reserved */
    h->resp_len = 52;
    h->init_done = true;
}

static void rndis_query_msg(usbd_rndis_t *h, const uint8_t *msg)
{
    uint32_t rid = get32le(msg + 8);
    uint32_t oid = get32le(msg + 12);
    uint8_t *r = h->resp_buf;
    uint8_t *data = r + 24;    /* InformationBuffer starts at offset 24 */
    uint32_t dlen = 0;
    uint32_t status = RNDIS_STATUS_SUCCESS;

    switch (oid) {
    case OID_GEN_SUPPORTED_LIST:
        dlen = sizeof supported_oids;
        memcpy(data, supported_oids, dlen);
        break;
    case OID_GEN_HARDWARE_STATUS:     put32le(data, 0); dlen = 4; break; /* NdisHardwareStatusReady */
    case OID_GEN_MEDIA_SUPPORTED:
    case OID_GEN_MEDIA_IN_USE:        put32le(data, RNDIS_MEDIUM_802_3); dlen = 4; break;
    case OID_GEN_MAXIMUM_FRAME_SIZE:  put32le(data, 1500); dlen = 4; break;
    case OID_GEN_LINK_SPEED:          put32le(data, 12000000u / 100u); dlen = 4; break; /* 12 Mbps in 100bps units */
    case OID_GEN_TRANSMIT_BLOCK_SIZE:
    case OID_GEN_RECEIVE_BLOCK_SIZE:  put32le(data, RNDIS_ETH_FRAME_MAX); dlen = 4; break;
    case OID_GEN_VENDOR_ID:           put32le(data, 0x00FFFFFFu); dlen = 4; break;
    case OID_GEN_VENDOR_DESCRIPTION:
        dlen = (uint32_t)(strlen(vendor_desc) + 1);
        memcpy(data, vendor_desc, dlen);
        break;
    case OID_GEN_CURRENT_PACKET_FILTER: put32le(data, h->packet_filter); dlen = 4; break;
    case OID_GEN_MAXIMUM_TOTAL_SIZE:  put32le(data, RNDIS_ETH_FRAME_MAX + RNDIS_HEADER_SIZE); dlen = 4; break;
    case OID_GEN_MEDIA_CONNECT_STATUS: put32le(data, 0); dlen = 4; break; /* connected */
    case OID_GEN_PHYSICAL_MEDIUM:     put32le(data, 0); dlen = 4; break; /* unspecified */
    case OID_GEN_XMIT_OK:            put32le(data, h->tx_ok); dlen = 4; break;
    case OID_GEN_RCV_OK:             put32le(data, h->rx_ok); dlen = 4; break;
    case OID_GEN_XMIT_ERROR:         put32le(data, h->tx_err); dlen = 4; break;
    case OID_GEN_RCV_ERROR:          put32le(data, h->rx_err); dlen = 4; break;
    case OID_GEN_RCV_NO_BUFFER:      put32le(data, h->rx_drop); dlen = 4; break;
    case OID_802_3_PERMANENT_ADDRESS:
    case OID_802_3_CURRENT_ADDRESS:   memcpy(data, h->mac_dev, 6); dlen = 6; break;
    case OID_802_3_MULTICAST_LIST:    memset(data, 0, 6); dlen = 6; break;
    case OID_802_3_MAXIMUM_LIST_SIZE: put32le(data, 1); dlen = 4; break;
    case OID_802_3_RCV_ERROR_ALIGNMENT:
    case OID_802_3_XMIT_ONE_COLLISION:
    case OID_802_3_XMIT_MORE_COLLISIONS: put32le(data, 0); dlen = 4; break;
    default:
        status = RNDIS_STATUS_NOT_SUPPORTED;
        break;
    }

    memset(r, 0, 24);
    put32le(r + 0, RNDIS_MSG_QUERY_C);
    put32le(r + 4, 24 + dlen);                /* MessageLength */
    put32le(r + 8, rid);
    put32le(r + 12, status);
    put32le(r + 16, dlen);                     /* InformationBufferLength */
    put32le(r + 20, dlen ? 16u : 0u);         /* InformationBufferOffset (from RequestId) */
    h->resp_len = 24 + dlen;
}

static void rndis_set_msg(usbd_rndis_t *h, const uint8_t *msg, uint32_t msglen)
{
    uint32_t rid = get32le(msg + 8);
    uint32_t oid = get32le(msg + 12);
    uint32_t buf_len = get32le(msg + 16);
    uint32_t buf_off = get32le(msg + 20);
    uint32_t status = RNDIS_STATUS_SUCCESS;

    if (oid == OID_GEN_CURRENT_PACKET_FILTER && buf_len >= 4 && (buf_off + 8 + 4) <= msglen) {
        h->packet_filter = get32le(msg + 8 + buf_off);
        if (h->packet_filter != 0) h->link_up = true;
        else h->link_up = false;
    } else if (oid == OID_802_3_MULTICAST_LIST) {
        /* accept and ignore */
    } else {
        status = RNDIS_STATUS_NOT_SUPPORTED;
    }

    uint8_t *r = h->resp_buf;
    memset(r, 0, 16);
    put32le(r + 0, RNDIS_MSG_SET_C);
    put32le(r + 4, 16);
    put32le(r + 8, rid);
    put32le(r + 12, status);
    h->resp_len = 16;
}

static void rndis_reset_msg(usbd_rndis_t *h)
{
    uint8_t *r = h->resp_buf;
    memset(r, 0, 16);
    put32le(r + 0, RNDIS_MSG_RESET_C);
    put32le(r + 4, 16);
    put32le(r + 8, RNDIS_STATUS_SUCCESS);
    put32le(r + 12, 0);         /* AddressingReset */
    h->resp_len = 16;
    h->packet_filter = 0;
    h->link_up = false;
}

static void rndis_keepalive_msg(usbd_rndis_t *h, const uint8_t *msg)
{
    uint32_t rid = get32le(msg + 8);
    uint8_t *r = h->resp_buf;
    memset(r, 0, 12);
    put32le(r + 0, RNDIS_MSG_KEEPALIVE_C);
    put32le(r + 4, 12);
    put32le(r + 8, rid);
    h->resp_len = 12;
}

/* ── Control transfer buffer for SEND_ENCAPSULATED / GET_ENCAPSULATED ── */
static __ALIGN_BEGIN uint8_t ctrl_buf[RNDIS_RESP_BUF_SIZE] __ALIGN_END;
static uint32_t ctrl_len;

static void rndis_process_command(USBD_HandleTypeDef *pdev, usbd_rndis_t *h)
{
    if (ctrl_len < 8) return;
    uint32_t type = get32le(ctrl_buf);

    switch (type) {
    case RNDIS_MSG_INIT:      rndis_init_msg(h, ctrl_buf); break;
    case RNDIS_MSG_HALT:      h->link_up = false; h->init_done = false; return;
    case RNDIS_MSG_QUERY:     rndis_query_msg(h, ctrl_buf); break;
    case RNDIS_MSG_SET:       rndis_set_msg(h, ctrl_buf, ctrl_len); break;
    case RNDIS_MSG_RESET:     rndis_reset_msg(h); break;
    case RNDIS_MSG_KEEPALIVE: rndis_keepalive_msg(h, ctrl_buf); break;
    default: return;
    }
    h->resp_avail = true;
    notify_response(pdev);
}

/* ── USB class callbacks ─────────────────────────────────────────────── */
static usbd_rndis_t rndis_handle;      /* single static instance (no malloc) */
/* OTG_FS runs without DMA (CPU copies the FIFO), so TX buf can live in CCM. */
__ALIGN_BEGIN static uint8_t rndis_tx_ccm[RNDIS_TX_BUF_SIZE] __ALIGN_END __attribute__((section(".ccmram")));

static uint8_t rndis_init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    (void)cfgidx;
    usbd_rndis_t *h = &rndis_handle;
    memset(h, 0, sizeof *h);
    h->tx_buf = rndis_tx_ccm;
    pdev->pClassDataCmsit[0] = h;

    /* MAC addresses: locally administered */
    static const uint8_t host_mac[6] = {0x02, 0x43, 0x49, 0x56, 0x49, 0x43};
    memcpy(h->mac_host, host_mac, 6);
    memcpy(h->mac_dev, host_mac, 6);
    h->mac_dev[5] ^= 0x01;

    /* Open endpoints */
    (void)USBD_LL_OpenEP(pdev, RNDIS_NOTIFICATION_EP, USBD_EP_TYPE_INTR, RNDIS_NOTIFICATION_SZ);
    (void)USBD_LL_OpenEP(pdev, RNDIS_DATA_IN_EP,      USBD_EP_TYPE_BULK, RNDIS_DATA_FS_MPS);
    (void)USBD_LL_OpenEP(pdev, RNDIS_DATA_OUT_EP,     USBD_EP_TYPE_BULK, RNDIS_DATA_FS_MPS);

    pdev->ep_in[RNDIS_NOTIFICATION_EP & 0x7Fu].is_used = 1;
    pdev->ep_in[RNDIS_DATA_IN_EP & 0x7Fu].is_used = 1;
    pdev->ep_out[RNDIS_DATA_OUT_EP & 0x7Fu].is_used = 1;

    /* Prepare to receive */
    (void)USBD_LL_PrepareReceive(pdev, RNDIS_DATA_OUT_EP, h->rx_buf, RNDIS_RX_BUF_SIZE);

    return USBD_OK;
}

static uint8_t rndis_deinit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    (void)cfgidx;
    USBD_LL_CloseEP(pdev, RNDIS_NOTIFICATION_EP);
    USBD_LL_CloseEP(pdev, RNDIS_DATA_IN_EP);
    USBD_LL_CloseEP(pdev, RNDIS_DATA_OUT_EP);
    usbd_rndis_t *h = usbd_rndis_handle(pdev);
    if (h) { h->link_up = false; h->init_done = false; }
    pdev->pClassDataCmsit[0] = NULL;
    return USBD_OK;
}

static uint8_t rndis_setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    usbd_rndis_t *h = usbd_rndis_handle(pdev);
    if (!h) return USBD_FAIL;

    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    case USB_REQ_TYPE_CLASS:
        if ((req->bmRequest & 0x80u) == 0u) {
            /* Host-to-device: SEND_ENCAPSULATED_COMMAND (0x00) */
            ctrl_len = req->wLength;
            if (ctrl_len > sizeof ctrl_buf) ctrl_len = sizeof ctrl_buf;
            if (ctrl_len > 0) {
                USBD_CtlPrepareRx(pdev, ctrl_buf, ctrl_len);
            }
        } else {
            /* Device-to-host: GET_ENCAPSULATED_RESPONSE (0x01) */
            if (h->resp_len > 0) {
                uint32_t len = h->resp_len;
                if (len > req->wLength) len = req->wLength;
                USBD_CtlSendData(pdev, h->resp_buf, len);
                h->resp_len = 0;
                h->resp_avail = false;
            } else {
                /* No response ready: send a single zero byte (RNDIS spec) */
                static __ALIGN_BEGIN uint8_t z __ALIGN_END = 0;
                USBD_CtlSendData(pdev, &z, 1);
            }
        }
        break;
    default:
        USBD_CtlError(pdev, req);
        return USBD_FAIL;
    }
    return USBD_OK;
}

static uint8_t rndis_ep0_rx_ready(USBD_HandleTypeDef *pdev)
{
    usbd_rndis_t *h = usbd_rndis_handle(pdev);
    if (h && ctrl_len >= 4) rndis_process_command(pdev, h);
    return USBD_OK;
}

static uint8_t rndis_data_in(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    usbd_rndis_t *h = usbd_rndis_handle(pdev);
    if (!h) return USBD_OK;
    if (epnum == (RNDIS_DATA_IN_EP & 0x7Fu)) {
        h->tx_busy = false;
    }
    return USBD_OK;
}

static uint8_t rndis_data_out(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    usbd_rndis_t *h = usbd_rndis_handle(pdev);
    if (!h) return USBD_OK;
    if (epnum == (RNDIS_DATA_OUT_EP & 0x7Fu)) {
        h->rx_len = USBD_LL_GetRxDataSize(pdev, RNDIS_DATA_OUT_EP);
        h->rx_ready = true;
        /* Do NOT re-arm here; net_usb_poll() calls rx_done after processing */
    }
    return USBD_OK;
}

static uint8_t *rndis_get_cfg_desc(uint16_t *len)
{
    *len = sizeof rndis_cfg_desc;
    return rndis_cfg_desc;
}

USBD_ClassTypeDef USBD_RNDIS_ClassDriver = {
    .Init                          = rndis_init,
    .DeInit                        = rndis_deinit,
    .Setup                         = rndis_setup,
    .EP0_TxSent                    = NULL,
    .EP0_RxReady                   = rndis_ep0_rx_ready,
    .DataIn                        = rndis_data_in,
    .DataOut                       = rndis_data_out,
    .SOF                           = NULL,
    .IsoINIncomplete               = NULL,
    .IsoOUTIncomplete              = NULL,
    .GetHSConfigDescriptor         = rndis_get_cfg_desc,
    .GetFSConfigDescriptor         = rndis_get_cfg_desc,
    .GetOtherSpeedConfigDescriptor = rndis_get_cfg_desc,
    .GetDeviceQualifierDescriptor  = NULL,
};

/* ── Public API ──────────────────────────────────────────────────────── */
USBD_StatusTypeDef usbd_rndis_transmit(USBD_HandleTypeDef *pdev, const uint8_t *eth, uint16_t len)
{
    usbd_rndis_t *h = usbd_rndis_handle(pdev);
    if (!h || !h->link_up) return USBD_FAIL;
    if (h->tx_busy) return USBD_BUSY;
    if (len > RNDIS_ETH_FRAME_MAX) return USBD_FAIL;

    /* Wrap in RNDIS_MSG_PACKET */
    uint8_t *p = h->tx_buf;
    memset(p, 0, 44);
    put32le(p + 0, RNDIS_MSG_PACKET);
    put32le(p + 4, 44u + len);            /* MessageLength */
    put32le(p + 8, 36);                   /* DataOffset (from start of DataOffset field = byte 8) */
    put32le(p + 12, len);                 /* DataLength */
    memcpy(p + 44, eth, len);

    h->tx_busy = true;
    h->tx_ok++;
    return USBD_LL_Transmit(pdev, RNDIS_DATA_IN_EP, p, 44u + len);
}

bool usbd_rndis_rx_get(USBD_HandleTypeDef *pdev, const uint8_t **frame, uint16_t *len)
{
    usbd_rndis_t *h = usbd_rndis_handle(pdev);
    if (!h || !h->rx_ready) return false;

    uint32_t total = h->rx_len;
    if (total < 44 + 14) { h->rx_ready = false; return false; }  /* too short for RNDIS hdr + ETH */

    uint32_t msg_type = get32le(h->rx_buf);
    if (msg_type != RNDIS_MSG_PACKET) { h->rx_ready = false; return false; }

    uint32_t data_off = get32le(h->rx_buf + 8);  /* from byte 8 */
    uint32_t data_len = get32le(h->rx_buf + 12);
    uint32_t abs_off = 8u + data_off;

    if (abs_off + data_len > total || data_len < 14 || data_len > RNDIS_ETH_FRAME_MAX) {
        h->rx_ready = false;
        return false;
    }

    *frame = h->rx_buf + abs_off;
    *len = (uint16_t)data_len;
    h->rx_ok++;
    return true;
}

void usbd_rndis_rx_done(USBD_HandleTypeDef *pdev)
{
    usbd_rndis_t *h = usbd_rndis_handle(pdev);
    if (!h) return;
    h->rx_ready = false;
    h->rx_len = 0;
    (void)USBD_LL_PrepareReceive(pdev, RNDIS_DATA_OUT_EP, h->rx_buf, RNDIS_RX_BUF_SIZE);
}
