/* USB network link: STM32F407 OTG-FS (MB997 CN5: PA11 DM, PA12 DP) enumerates
 * as a CDC-NCM Ethernet adapter. lwIP (NO_SYS, raw API) runs on top; the host
 * receives 192.168.7.2..4 from the built-in DHCP server and opens the
 * dashboard at http://192.168.7.1/ (also http://civic.usb/ via the DNS stub).
 *
 * Replaces the ESP32 Wi-Fi access point: same services, cable instead of RF.
 * Glue derived from TinyUSB examples/device/net_lwip_webserver (MIT). */
#include "net_usb.h"

#include <string.h>
#include "stm32f4xx.h"
#include "tusb.h"
#include "dhserver.h"
#include "dnserver.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/etharp.h"
#include "netif/ethernet.h"

#define INIT_IP4(a, b, c, d) { PP_HTONL(LWIP_MAKEU32(a, b, c, d)) }

extern volatile uint32_t system_millis;

static struct netif netif_data;
static struct pbuf *received_frame;
static bool net_up;

/* Locally administered MAC (bit 1 of the first byte set). */
uint8_t tud_network_mac_address[6] = {0x02, 0x43, 0x49, 0x56, 0x49, 0x43};

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
    TU_ARRAY_SIZE(entries),
    entries
};

static err_t linkoutput_fn(struct netif *netif, struct pbuf *p)
{
    (void)netif;
    /* Bounded: never spin forever if the host stops reading. */
    for (uint32_t tries = 0; tries < 2000u; tries++) {
        if (!tud_ready()) return ERR_USE;
        if (tud_network_can_xmit(p->tot_len)) { tud_network_xmit(p, 0); return ERR_OK; }
        tud_task();
    }
    return ERR_MEM;
}

static err_t ip4_output_fn(struct netif *netif, struct pbuf *p, const ip4_addr_t *addr)
{
    return etharp_output(netif, p, addr);
}

static err_t netif_init_cb(struct netif *netif)
{
    netif->mtu = CFG_TUD_NET_MTU;
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

bool tud_network_recv_cb(const uint8_t *src, uint16_t size)
{
    if (received_frame) return false;
    if (size) {
        struct pbuf *p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
        if (p) { pbuf_take(p, src, size); received_frame = p; }
    }
    return true;
}

uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg)
{
    struct pbuf *p = (struct pbuf *)ref;
    (void)arg;
    return pbuf_copy_partial(p, dst, p->tot_len, 0);
}

void tud_network_init_cb(void)
{
    if (received_frame) { pbuf_free(received_frame); received_frame = NULL; }
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
uint32_t tusb_time_millis_api(void) { return system_millis; }

void OTG_FS_IRQHandler(void) { tud_int_handler(0); }

/* USB serial-number string from the 96-bit STM32 unique ID. */
size_t board_usb_get_serial(uint16_t desc_str1[], size_t max_chars);
size_t board_usb_get_serial(uint16_t desc_str1[], size_t max_chars)
{
    const volatile uint8_t *uid = (const volatile uint8_t *)UID_BASE;
    size_t n = 0;
    for (size_t i = 0; i < 12u && n + 2u <= max_chars; i++) {
        desc_str1[n++] = (uint16_t)"0123456789ABCDEF"[uid[i] >> 4];
        desc_str1[n++] = (uint16_t)"0123456789ABCDEF"[uid[i] & 15u];
    }
    return n;
}

static void usb_hw_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    (void)RCC->AHB1ENR;
    /* PA11 = OTG_FS_DM, PA12 = OTG_FS_DP: AF10, very high speed, no pull. */
    GPIOA->MODER   = (GPIOA->MODER & ~((3u << 22) | (3u << 24))) | (2u << 22) | (2u << 24);
    GPIOA->OTYPER &= ~((1u << 11) | (1u << 12));
    GPIOA->OSPEEDR |= (3u << 22) | (3u << 24);
    GPIOA->PUPDR  &= ~((3u << 22) | (3u << 24));
    GPIOA->AFR[1]  = (GPIOA->AFR[1] & ~((15u << 12) | (15u << 16))) | (10u << 12) | (10u << 16);
    RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
    (void)RCC->AHB2ENR;
    /* MB997: PA9 VBUS is not used for sensing here; treat VBUS as always valid. */
    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
    USB_OTG_FS->GCCFG &= ~(USB_OTG_GCCFG_VBUSBSEN | USB_OTG_GCCFG_VBUSASEN);
    NVIC_SetPriority(OTG_FS_IRQn, 6);
}

bool net_usb_init(void)
{
    usb_hw_init();
    if (!tud_init(BOARD_TUD_RHPORT)) return false;
    lwip_init();
    struct netif *netif = &netif_data;
    netif->hwaddr_len = sizeof tud_network_mac_address;
    memcpy(netif->hwaddr, tud_network_mac_address, sizeof tud_network_mac_address);
    netif->hwaddr[5] ^= 0x01;   /* must differ from the host side MAC */
    netif = netif_add(netif, &ipaddr, &netmask, &gateway, NULL, netif_init_cb, ethernet_input);
    if (!netif) return false;
    netif_set_default(netif);
    if (dhserv_init(&dhcp_config) != ERR_OK) return false;
    if (dnserv_init(IP_ADDR_ANY, 53, dns_query_proc) != ERR_OK) return false;
    net_up = true;
    return true;
}

void net_usb_poll(void)
{
    if (!net_up) return;
    tud_task();
    if (received_frame) {
        struct pbuf *p = received_frame;
        received_frame = NULL;
        if (netif_data.input(p, &netif_data) != ERR_OK) pbuf_free(p);
        tud_network_recv_renew();
    }
    sys_check_timeouts();
}

bool net_usb_host_connected(void) { return net_up && tud_ready(); }
