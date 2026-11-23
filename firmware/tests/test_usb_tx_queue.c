#include <stdio.h>
#include <string.h>

#include "usb_tx_queue.h"

static int fail(const char *message)
{
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(void)
{
    usb_tx_queue_t queue;
    usb_tx_queue_init(&queue);
    if (usb_tx_queue_size(&queue) != 0u) return fail("empty queue after init");

    const uint8_t first[] = { 0xF1, 0x09, 0xDE, 0xAD };
    if (!usb_tx_queue_write(&queue, first, sizeof first))
        return fail("write first packet");

    uint16_t length = 0u;
    uint8_t *pending = usb_tx_queue_peek(&queue, &length);
    if (!pending || length != sizeof first ||
        memcmp(pending, first, sizeof first) != 0)
        return fail("peek pending packet");

    const uint8_t second[] = { 0x01, 0x02, 0x03 };
    if (!usb_tx_queue_write(&queue, second, sizeof second) ||
        memcmp(pending, first, sizeof first) != 0)
        return fail("queued data remains stable while transfer is pending");

    if (!usb_tx_queue_consume(&queue, sizeof first))
        return fail("consume completed packet");
    pending = usb_tx_queue_peek(&queue, &length);
    if (!pending || length != sizeof second ||
        memcmp(pending, second, sizeof second) != 0)
        return fail("peek following packet");

    uint8_t fill[USB_TX_QUEUE_CAPACITY - 1u];
    memset(fill, 0x5A, sizeof fill);
    if (!usb_tx_queue_consume(&queue, sizeof second) ||
        !usb_tx_queue_write(&queue, fill, sizeof fill) ||
        usb_tx_queue_size(&queue) != sizeof fill)
        return fail("fill queue to its usable capacity");
    if (usb_tx_queue_write(&queue, first, sizeof first))
        return fail("full queue rejects overflow");

    if (!usb_tx_queue_consume(&queue, 900u)) return fail("consume across first segment");
    uint8_t wrapped[900];
    memset(wrapped, 0xA5, sizeof wrapped);
    if (!usb_tx_queue_write(&queue, wrapped, sizeof wrapped) ||
        usb_tx_queue_size(&queue) != USB_TX_QUEUE_CAPACITY - 1u)
        return fail("write across ring boundary");

    pending = usb_tx_queue_peek(&queue, &length);
    if (!pending || length != 117u) return fail("first contiguous ring segment");
    if (!usb_tx_queue_consume(&queue, length)) return fail("consume first ring segment");
    pending = usb_tx_queue_peek(&queue, &length);
    if (!pending || length != 906u ||
        memcmp(pending, fill + 1017u, 6u) != 0 ||
        memcmp(pending + 6u, wrapped, sizeof wrapped) != 0)
        return fail("second contiguous ring segment");

    if (usb_tx_queue_write(NULL, first, sizeof first) ||
        usb_tx_queue_write(&queue, NULL, sizeof first) ||
        usb_tx_queue_consume(&queue, 0u))
        return fail("invalid queue arguments rejected");

    {
        usb_tx_queue_t empty;
        uint16_t n = 99u;
        usb_tx_queue_init(&empty);
        if (usb_tx_queue_peek(&empty, &n) != NULL || n != 0u)
            return fail("peek on empty queue returns NULL and length 0");
    }
    puts("ALL USB TX QUEUE TESTS PASS");
    return 0;
}
