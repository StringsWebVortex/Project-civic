/* USB network link: STM32F407 OTG-FS (MB997 CN5: PA11 DM, PA12 DP) enumerates
 * as an RNDIS Ethernet adapter. lwIP (NO_SYS, raw API) runs on top;
 * the host receives 192.168.7.2..5 from the built-in DHCP server and opens
 * the dashboard at http://192.168.7.1/ (also http://civic.usb/ via DNS).
 *
 * Android, Linux and Windows all have built-in RNDIS host drivers. Android
 * additionally triggers a captive-portal detection flow: the DNS server
 * resolves all names to 192.168.7.1, the web server returns a 302 redirect
 * for the connectivity-check URLs, and Android shows the sign-in notification
 * which opens the dashboard automatically. */
#include "net_usb.h"

#include <string.h>
#include "stm32f4xx.h"
#include "usbd_core.h"
#include "usbd_rndis.h"
#include "dhserver.h"
#include "dnserver.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/etharp.h"
#include "netif/ethernet.h"

#define INIT_IP4(a, b, c, d) { PP_HTONL(LWIP_MAKEU32(a, b, c, d)) }

extern volatile uint32_t system_millis;
extern USBD_DescriptorsTypeDef RNDIS_Desc;

USBD_HandleTypeDef USBD_Device;
static struct netif netif_data;
static bool net_up;

/* Host-side MAC (locally administered). Board MAC has last byte flipped. */
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

/* ── lwIP netif ─────────────────────────────────────────────────────── */
static err_t linkoutput_fn(struct netif *netif, struct pbuf *p)
{
    (void)netif;
    usbd_rndis_t *h = usbd_rndis_handle(&USBD_Device);
    if (!h || !h->link_up || USBD_Device.dev_state != USBD_STATE_CONFIGURED)
        return ERR_USE;
    if (p->tot_len > RNDIS_ETH_FRAME_MAX) return ERR_BUF;

    /* Wait for previous TX to finish (bounded) */
    const uint32_t t0 = system_millis;
    while (h->tx_busy) {
        if ((uint32_t)(system_millis - t0) > 20u) return ERR_MEM;
    }

    /* Linearize pbuf chain into a contiguous buffer */
    static uint8_t eth_frame[RNDIS_ETH_FRAME_MAX];
    (void)pbuf_copy_partial(p, eth_frame, p->tot_len, 0);
    return (usbd_rndis_transmit(&USBD_Device, eth_frame, p->tot_len) == USBD_OK)
           ? ERR_OK : ERR_MEM;
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

/* DNS: resolve ALL queries to our IP (captive portal). civic.usb is the
 * friendly name; everything else is for captive-portal detection. */
bool dns_query_proc(const char *name, ip4_addr_t *addr)
{
    (void)name;
    *addr = ipaddr;
    return true;    /* resolve every domain to 192.168.7.1 */
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
    netif->hwaddr[5] ^= 0x01;             /* must differ from the host side MAC */
    netif = netif_add(netif, &ipaddr, &netmask, &gateway, NULL, netif_init_cb, ethernet_input);
    if (!netif) return false;
    netif_set_default(netif);
    if (dhserv_init(&dhcp_config) != ERR_OK) return false;
    if (dnserv_init(IP_ADDR_ANY, 53, dns_query_proc) != ERR_OK) return false;

    /* Start USB RNDIS device */
    if (USBD_Init(&USBD_Device, &RNDIS_Desc, DEVICE_FS) != USBD_OK) return false;
    if (USBD_RegisterClass(&USBD_Device, USBD_RNDIS_CLASS) != USBD_OK) return false;
    if (USBD_Start(&USBD_Device) != USBD_OK) return false;
    net_up = true;
    return true;
}

void net_usb_poll(void)
{
    if (!net_up) return;

    /* Process received RNDIS packets */
    const uint8_t *frame;
    uint16_t flen;
    if (usbd_rndis_rx_get(&USBD_Device, &frame, &flen)) {
        if (flen >= 14u && flen <= RNDIS_ETH_FRAME_MAX) {
            struct pbuf *p = pbuf_alloc(PBUF_RAW, flen, PBUF_POOL);
            if (p) {
                (void)pbuf_take(p, frame, flen);
                if (netif_data.input(p, &netif_data) != ERR_OK) pbuf_free(p);
            }
        }
        usbd_rndis_rx_done(&USBD_Device);
    }
    sys_check_timeouts();
}

bool net_usb_host_connected(void)
{
    usbd_rndis_t *h = usbd_rndis_handle(&USBD_Device);
    return net_up && USBD_Device.dev_state == USBD_STATE_CONFIGURED && h && h->link_up;
}
