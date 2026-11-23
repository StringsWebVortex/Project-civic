#ifndef ISOTP_H
#define ISOTP_H
/*
 * isotp.h - ISO 15765-2 (ISO-TP) transport over classic CAN, normal addressing.
 *
 * Own implementation (no third-party library). Frame layout and behaviour
 * follow the Linux kernel ISO-TP implementation (net/can/isotp.c, mainline):
 *   PCI high nibble: 0 = single frame (SF), 1 = first frame (FF),
 *                    2 = consecutive frame (CF), 3 = flow control (FC).
 *   FC flow status: 0 = CTS, 1 = WAIT, 2 = OVERFLOW.
 *   STmin: 0x00..0x7F = milliseconds, 0xF1..0xF9 = 100..900 us, any other
 *          value is treated as 0x7F (127 ms).
 *   SN starts at 1 for the first CF and wraps modulo 16.
 *   BS = 0: no further flow control frames during the transfer.
 *   FC / CF wait timeout: 1 s.
 *   12-bit FF length (max 4095 bytes); longer messages (FF_DL = 0 escape)
 *   are answered with FC OVERFLOW.
 * The ISO 15765-2 text itself is not public and was not read.
 *
 * Time base is milliseconds. 100..900 us STmin values are rounded up to 1 ms,
 * and one extra ms tick is waited between consecutive frames whenever STmin
 * is not 0 (a 1 ms counter can tick just after a send), so the real gap is
 * never shorter than the STmin the receiver asked for.
 */
#include <stdbool.h>
#include <stdint.h>

#define ISOTP_MAX_PAYLOAD     4095u
#define ISOTP_TIMEOUT_MS      1000u
#define ISOTP_MAX_WAIT_FRAMES   10u

typedef enum {
    ISOTP_OK = 0,
    ISOTP_ERR_BUSY,          /* a transmission is already running        */
    ISOTP_ERR_LENGTH,        /* 0 or > ISOTP_MAX_PAYLOAD                  */
    ISOTP_ERR_TIMEOUT_FC,    /* no flow control within ISOTP_TIMEOUT_MS   */
    ISOTP_ERR_TIMEOUT_CF,    /* receiver: next CF did not arrive in time  */
    ISOTP_ERR_WRONG_SN,      /* receiver: CF sequence number mismatch     */
    ISOTP_ERR_OVERFLOW,      /* peer answered FC OVERFLOW / we can't hold */
    ISOTP_ERR_WAIT_LIMIT,    /* more than ISOTP_MAX_WAIT_FRAMES FC WAIT   */
    ISOTP_ERR_BAD_FC,        /* invalid flow status                       */
    ISOTP_ERR_TIMEOUT_TX     /* a frame could not be handed to the CAN
                                controller within ISOTP_TIMEOUT_MS: send
                                timeout, like the Linux isotp.c tx timer
                                ("no flow control or echo frame in time") */
} isotp_status_t;

/* Returns true if the frame was handed to the CAN controller. false means
 * "not now" (no mailbox / rate limit); the link retries on the next poll. */
typedef bool (*isotp_send_fn)(void *ctx, uint32_t id, bool ext,
                              const uint8_t *data, uint8_t len);

typedef enum { ISOTP_TX_IDLE, ISOTP_TX_SF, ISOTP_TX_FF, ISOTP_TX_WAIT_FC, ISOTP_TX_CF } isotp_tx_state_t;
typedef enum { ISOTP_RX_IDLE, ISOTP_RX_SEND_FC, ISOTP_RX_CF, ISOTP_RX_SEND_OVFLW } isotp_rx_state_t;

typedef struct {
    /* configuration */
    uint32_t      tx_id, rx_id;
    bool          ext;
    bool          pad;            /* pad every frame to 8 bytes            */
    uint8_t       pad_byte;
    uint8_t       rx_bs;          /* block size we announce (0 = no limit) */
    uint8_t       rx_stmin;       /* STmin we announce                     */
    isotp_send_fn send;
    void         *ctx;

    /* transmit side */
    isotp_tx_state_t tx_state;
    uint8_t  tx_buf[ISOTP_MAX_PAYLOAD];
    uint16_t tx_len, tx_pos;
    uint8_t  tx_sn, tx_bs, tx_bs_count, tx_wait_count;
    uint32_t tx_gap_ms, tx_next_ms, tx_deadline_ms;
    bool     tx_done;             /* last message fully handed over        */

    /* receive side */
    isotp_rx_state_t rx_state;
    uint8_t  rx_buf[ISOTP_MAX_PAYLOAD];
    uint16_t rx_len, rx_pos;
    uint8_t  rx_sn, rx_bs_count;
    uint32_t rx_deadline_ms;
    bool     rx_ready;            /* complete message waiting in rx_buf    */

    isotp_status_t last_error;    /* set when a transfer is aborted        */
    bool           error_pending;
} isotp_link_t;

void isotp_init(isotp_link_t *l, uint32_t tx_id, uint32_t rx_id, bool ext,
                isotp_send_fn send, void *ctx);
void isotp_set_padding(isotp_link_t *l, bool pad, uint8_t pad_byte);
void isotp_reset(isotp_link_t *l);

/* Start sending one message. */
isotp_status_t isotp_send(isotp_link_t *l, const uint8_t *data, uint16_t len, uint32_t now_ms);

/* Feed every received CAN frame; frames with another ID are ignored. */
void isotp_on_frame(isotp_link_t *l, uint32_t id, bool ext,
                    const uint8_t *data, uint8_t dlc, uint32_t now_ms);

/* Call often: sends pending frames, enforces timeouts. */
void isotp_poll(isotp_link_t *l, uint32_t now_ms);

bool isotp_tx_busy(const isotp_link_t *l);

/* Takes a complete received message (copies up to cap bytes). */
bool isotp_take(isotp_link_t *l, uint8_t *out, uint16_t cap, uint16_t *len);

/* Takes the last abort reason, if any. */
bool isotp_take_error(isotp_link_t *l, isotp_status_t *err);

const char *isotp_status_name(isotp_status_t s);
#endif
