/* USB network link: STM32F407 OTG-FS (MB997 CN5: PA11 DM, PA12 DP) enumerates
 * as a CDC-ECM Ethernet adapter using ST's official USB Device Library
 * (STM32CubeF4 V1.28.3, Class/CDC_ECM). lwIP (NO_SYS, raw API) runs on top;
 * the host receives 192.168.7.2..5 from the built-in DHCP server and opens
 * the dashboard at http://192.168.7.1/ (also http://civic.usb/ via DNS).
 *
 * Frame path (ST CDC_ECM class): DataOut accumulates 64-byte packets into
 * rx_buf until a short packet ends the Ethernet frame, then calls Receive()
 * from the USB interrupt; net_usb_poll() hands the frame to lwIP, clears
 * RxLength and re-arms the OUT endpoint. lwIP output copies one frame into
 * tx_buf and starts the bulk IN transfer (the class adds the ZLP). */
#include "net_usb.h"

#include <string.h>
#include "stm32f4xx.h"
#include "usbd_core.h"
#include "usbd_cdc_ecm.h"
#include "dhserver.h"
#include "dnserver.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/etharp.h"
#include "netif/ethernet.h"

#define INIT_IP4(a, b, c, d) { PP_HTONL(LWIP_MAKEU32(a, b, c, d)) }
#define ECM_FRAME_MAX 1514u                    /* CDC_ECM_ETH_MAX_SEGSZE */

extern volatile uint32_t system_millis;
extern USBD_DescriptorsTypeDef ECM_Desc;

USBD_HandleTypeDef USBD_Device;
static struct netif netif_data;
static bool net_up;
static volatile bool rx_ready;
static volatile uint32_t rx_len;
static uint32_t rx_drops;
__ALIGN_BEGIN static uint8_t rx_buf[ECM_FRAME_MAX + 64u] __ALIGN_END;  /* + one packet of slack */
/* OTG_FS runs without DMA (usbd_conf_ecm.c), the CPU copies the FIFO: the
 * TX frame buffer can live in CCM RAM (RM0090 2.3.1: CCM is CPU-only). */
__ALIGN_BEGIN static uint8_t tx_buf[ECM_FRAME_MAX] __ALIGN_END __attribute__((section(".ccmram")));

/* Host-side MAC (ECM iMACAddress string); the board uses the same with the
 * last byte flipped. Locally administered (bit 1 of the first byte). */
static const uint8_t host_mac[6] = {0x02, 0x43, 0x49, 0x56, 0x49, 0x43};

static const ip4_addr_t ipaddr  = INIT_IP4(192, 168, 7, 1);
static const ip4_addr_t netmask = INIT_IP4(255, 255, 255, 0);
static const ip4_addr_t gateway = INIT_IP4(0, 0, 0, 0);

static dhcp_entry_t entries[] = {
    {{0}, INIT_IP4(192, 168, 7, 2), 24 * 60 * 60},
    {{0}, INIT_IP4(192, 168, 7, 3), 24 * 60 * 60},
    {{0}, INIT_IP4(192, 168, 7, 4), 24 * 60 * 60},
    {{0}, INIT_IP4(192, 168, 7, 5), 24 * 60 * 60},
};
static const dhcp_config_t dhcp_config = {
    .router = INIT_IP4(0, 0, 0, 0),
    .port = 67,
    .dns = INIT_IP4(192, 168, 7, 1),
    "usb",
    sizeof entries / sizeof entries[0],
    entries
};

static USBD_CDC_ECM_HandleTypeDef *ecm(void)
{
    return (USBD_CDC_ECM_HandleTypeDef *)USBD_Device.pClassDataCmsit[0];
}

/* ── ST CDC_ECM interface callbacks ─────────────────────────────────── */
static int8_t itf_init(void)
{
    (void)USBD_CDC_ECM_SetTxBuffer(&USBD_Device, tx_buf, 0U);
    (void)USBD_CDC_ECM_SetRxBuffer(&USBD_Device, rx_buf);
    rx_ready = false;
    return 0;
}
static int8_t itf_deinit(void)
{
    USBD_CDC_ECM_HandleTypeDef *h = ecm();
    if (h) h->LinkStatus = 0U;
    return 0;
}
static int8_t itf_control(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
    (void)pbuf; (void)length;
    USBD_CDC_ECM_HandleTypeDef *h = ecm();
    if (!h) return -1;
    /* The host enables the data path with SET_ETHERNET_PACKET_FILTER (ECM 6.2.4). */
    if (cmd == CDC_ECM_SET_ETH_PACKET_FILTER && h->LinkStatus == 0U) {
        h->LinkStatus = 1U;
        if (h->NotificationStatus == 0U) {
            (void)USBD_CDC_ECM_SendNotification(&USBD_Device, NETWORK_CONNECTION, CDC_ECM_NET_CONNECTED, NULL);
            h->NotificationStatus = 1U;          /* speed-change notification follows */
        }
    }
    return 0;
}
static int8_t itf_receive(uint8_t *buf, uint32_t *len)
{
    (void)buf;
    rx_len = *len;                               /* frame complete; OUT stays NAKed */
    rx_ready = true;
    return 0;
}
static int8_t itf_tx_done(uint8_t *buf, uint32_t *len, uint8_t ep) { (void)buf; (void)len; (void)ep; return 0; }
static int8_t itf_process(USBD_HandleTypeDef *pdev) { (void)pdev; return 0; }

static USBD_CDC_ECM_ItfTypeDef ecm_fops = {
    itf_init, itf_deinit, itf_control, itf_receive, itf_tx_done, itf_process,
    (const uint8_t *)"024349564943",             /* iMACAddress: 12 hex digits */
};

/* ── lwIP netif ─────────────────────────────────────────────────────── */
static err_t linkoutput_fn(struct netif *netif, struct pbuf *p)
{
    (void)netif;
    USBD_CDC_ECM_HandleTypeDef *h = ecm();
    if (!h || USBD_Device.dev_state != USBD_STATE_CONFIGURED || h->LinkStatus == 0U) return ERR_USE;
    if (p->tot_len > sizeof tx_buf) return ERR_BUF;
    /* Bounded wait for the previous frame (one bulk IN in flight). */
    const uint32_t t0 = system_millis;
    while (h->TxState != 0U) {
        if ((uint32_t)(system_millis - t0) > 20u) return ERR_MEM;
    }
    (void)pbuf_copy_partial(p, tx_buf, p->tot_len, 0);
    (void)USBD_CDC_ECM_SetTxBuffer(&USBD_Device, tx_buf, p->tot_len);
    return (USBD_CDC_ECM_TransmitPacket(&USBD_Device) == (uint8_t)USBD_OK) ? ERR_OK : ERR_MEM;
}

static err_t ip4_output_fn(struct netif *netif, struct pbuf *p, const ip4_addr_t *addr)
{
    return etharp_output(netif, p, addr);
}

static err_t netif_init_cb(struct netif *netif)
{
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;
    netif->state = NULL;
    netif->name[0] = 'E';
    netif->name[1] = 'X';
    netif->linkoutput = linkoutput_fn;
    netif->output = ip4_output_fn;
    return ERR_OK;
}

bool dns_query_proc(const char *name, ip4_addr_t *addr)
{
    if (strcmp(name, "civic.usb") == 0) { *addr = ipaddr; return true; }
    return false;
}

/* lwIP NO_SYS hooks */
sys_prot_t sys_arch_protect(void)
{
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return (sys_prot_t)primask;
}
void sys_arch_unprotect(sys_prot_t pval) { if (!pval) __enable_irq(); }
uint32_t sys_now(void) { return system_millis; }

bool net_usb_init(void)
{
    lwip_init();
    struct netif *netif = &netif_data;
    netif->hwaddr_len = sizeof host_mac;
    memcpy(netif->hwaddr, host_mac, sizeof host_mac);
    netif->hwaddr[5] ^= 0x01;                   /* must differ from the host side MAC */
    netif = netif_add(netif, &ipaddr, &netmask, &gateway, NULL, netif_init_cb, ethernet_input);
    if (!netif) return false;
    netif_set_default(netif);
    if (dhserv_init(&dhcp_config) != ERR_OK) return false;
    if (dnserv_init(IP_ADDR_ANY, 53, dns_query_proc) != ERR_OK) return false;
    if (USBD_Init(&USBD_Device, &ECM_Desc, DEVICE_FS) != USBD_OK) return false;
    if (USBD_RegisterClass(&USBD_Device, USBD_CDC_ECM_CLASS) != USBD_OK) return false;
    if (USBD_CDC_ECM_RegisterInterface(&USBD_Device, &ecm_fops) != USBD_OK) return false;
    if (USBD_Start(&USBD_Device) != USBD_OK) return false;
    net_up = true;
    return true;
}

void net_usb_poll(void)
{
    if (!net_up) return;
    if (rx_ready) {
        const uint32_t n = rx_len;
        if (n >= 14u && n <= ECM_FRAME_MAX) {
            struct pbuf *p = pbuf_alloc(PBUF_RAW, (u16_t)n, PBUF_POOL);
            if (p) {
                (void)pbuf_take(p, rx_buf, (u16_t)n);
                if (netif_data.input(p, &netif_data) != ERR_OK) pbuf_free(p);
            } else rx_drops++;
        }
        USBD_CDC_ECM_HandleTypeDef *h = ecm();
        rx_ready = false;
        if (h) { h->RxLength = 0U; (void)USBD_CDC_ECM_ReceivePacket(&USBD_Device); }
    }
    sys_check_timeouts();
}

bool net_usb_host_connected(void)
{
    const USBD_CDC_ECM_HandleTypeDef *h = ecm();
    return net_up && USBD_Device.dev_state == USBD_STATE_CONFIGURED && h && h->LinkStatus != 0U;
}
