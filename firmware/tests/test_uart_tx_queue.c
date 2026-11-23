#include <stdio.h>
#include <string.h>
#include "uart_tx_queue.h"

#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); return 1; } } while (0)

int main(void)
{
    uart_tx_queue_t q;
    uart_tx_queue_init(NULL);
    CHECK(uart_tx_queue_size(NULL) == 0u, "null size");
    uart_tx_queue_init(&q);
    CHECK(uart_tx_queue_size(&q) == 0u, "empty after init");

    const uint8_t a[] = { 'A', 'B', 'C' };
    const uint8_t b[] = { 'D', 'E' };
    CHECK(!uart_tx_queue_write(NULL, a, sizeof a), "null queue rejected");
    CHECK(!uart_tx_queue_write(&q, NULL, sizeof a), "null data rejected");
    CHECK(!uart_tx_queue_write(&q, a, 0u), "empty write rejected");
    CHECK(!uart_tx_queue_write(&q, a, UART_TX_QUEUE_CAPACITY), "oversize write rejected");
    CHECK(!uart_tx_queue_write_line(&q, a, 0u), "empty line rejected");
    CHECK(uart_tx_queue_write(&q, a, sizeof a), "first write");

    uint16_t n = 0u;
    uint8_t *p = uart_tx_queue_peek(&q, &n);
    CHECK(p != NULL && n == sizeof a && memcmp(p, a, sizeof a) == 0, "peek first packet");
    CHECK(uart_tx_queue_write_line(&q, b, sizeof b), "atomic line write");
    CHECK(memcmp(p, a, sizeof a) == 0, "pending DMA bytes remain stable");
    CHECK(uart_tx_queue_size(&q) == sizeof a + sizeof b + 1u, "line includes delimiter");
    CHECK(uart_tx_queue_consume(&q, sizeof a), "consume first packet");
    p = uart_tx_queue_peek(&q, &n);
    CHECK(p != NULL && n == sizeof b + 1u && memcmp(p, "DE\n", 3u) == 0, "line bytes and delimiter");
    CHECK(!uart_tx_queue_consume(&q, 0u), "zero consume rejected");
    CHECK(!uart_tx_queue_consume(&q, (uint16_t)(n + 1u)), "over-consume rejected");
    CHECK(uart_tx_queue_consume(&q, n), "consume line");
    CHECK(uart_tx_queue_peek(&q, &n) == NULL && n == 0u, "empty peek");
    CHECK(uart_tx_queue_peek(NULL, &n) == NULL, "null queue peek");
    CHECK(uart_tx_queue_peek(&q, NULL) == NULL, "null length peek");
    CHECK(!uart_tx_queue_consume(NULL, 1u), "null consume rejected");

    uart_tx_queue_init(&q);
    static uint8_t fill[UART_TX_QUEUE_CAPACITY - 1u];
    static uint8_t wrap[900];
    memset(fill, 0x5Au, sizeof fill);
    memset(wrap, 0xA5u, sizeof wrap);
    CHECK(uart_tx_queue_write(&q, fill, sizeof fill), "fill usable capacity");
    CHECK(uart_tx_queue_size(&q) == sizeof fill, "full usable size");
    CHECK(!uart_tx_queue_write(&q, a, sizeof a), "full queue rejects overflow");
    CHECK(!uart_tx_queue_write_line(&q, b, sizeof b), "full queue rejects whole line");
    CHECK(uart_tx_queue_consume(&q, 900u), "advance tail before wrap");
    CHECK(uart_tx_queue_write(&q, wrap, sizeof wrap), "write through ring wrap");
    CHECK(uart_tx_queue_size(&q) == UART_TX_QUEUE_CAPACITY - 1u, "wrapped queue remains full");

    p = uart_tx_queue_peek(&q, &n);
    CHECK(p != NULL && n == UART_TX_QUEUE_CAPACITY - 900u, "first contiguous wrapped segment");
    CHECK(memcmp(p, fill + 900u, sizeof fill - 900u) == 0 &&
          p[sizeof fill - 900u] == wrap[0], "first segment preserves old and wrapped bytes");
    CHECK(uart_tx_queue_consume(&q, n), "consume first wrapped segment");
    p = uart_tx_queue_peek(&q, &n);
    CHECK(p != NULL && n == sizeof wrap - 1u &&
          memcmp(p, wrap + 1u, sizeof wrap - 1u) == 0, "second wrapped segment preserved");
    CHECK(uart_tx_queue_consume(&q, n), "consume wrapped bytes");
    CHECK(uart_tx_queue_size(&q) == 0u, "queue empty after complete drain");

    puts("ALL UART TX QUEUE TESTS PASS");
    return 0;
}
