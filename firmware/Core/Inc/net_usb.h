#ifndef NET_USB_H
#define NET_USB_H
#include <stdbool.h>

/* USB CDC-ECM network adapter (ST USB Device Library) + lwIP; host gets 192.168.7.x by DHCP and the
 * STM32 answers at 192.168.7.1. */
bool net_usb_init(void);
void net_usb_poll(void);
bool net_usb_host_connected(void);

#endif
