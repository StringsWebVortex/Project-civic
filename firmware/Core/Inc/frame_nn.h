#ifndef FRAME_NN_H
#define FRAME_NN_H
/* On-device online neural network for CAN frames.
 *
 * Learns, per CAN identifier, what the next payload looks like given the
 * previous payload of that identifier (a 9-16-8 MLP, SGD, online). Each frame
 * is first scored (prediction error against the frame actually seen), then
 * used for one training step. The anomaly score is therefore always computed
 * before the frame is learned from.
 *
 * Read-only by construction: this module has no transmit path. It includes no
 * CAN hardware header, and the host test verifies that the compiled object
 * references no CAN transmit symbol.
 *
 * No heap allocation. Fixed size: about 1.5 KiB of weights and state. */
#include <stdbool.h>
#include <stdint.h>

#define FNN_SLOTS    32u      /* distinct CAN identifiers tracked */
#define FNN_IN       9u       /* 8 previous payload bytes (scaled) + bias */
#define FNN_HID      16u
#define FNN_OUT      8u
#define FNN_LR       0.05f

typedef struct {
    uint32_t id;
    bool     ext;
    bool     used;
    bool     have_prev;
    uint8_t  dlc;
    uint8_t  prev[8];
} fnn_slot_t;

typedef struct {
    float      w1[FNN_HID][FNN_IN];
    float      w2[FNN_OUT][FNN_HID];
    float      b2[FNN_OUT];
    fnn_slot_t slot[FNN_SLOTS];
    uint32_t   frames;       /* frames processed (saturating) */
    float      ema_err;      /* running mean of anomaly score */
} fnn_t;

void  fnn_init(fnn_t *n);
/* Scores the frame against the network's prediction for this identifier,
 * then trains on it. Returns the anomaly score in [0, 1] (mean absolute error
 * over the payload bytes), or -1 when the identifier has no previous frame
 * yet (nothing to predict from) or the table is full. */
float fnn_on_frame(fnn_t *n, uint32_t id, bool ext, uint8_t dlc, const uint8_t d[8]);
/* Pure scoring, no training, no state change beyond slot lookup. */
float fnn_score(const fnn_t *n, uint32_t id, bool ext, uint8_t dlc, const uint8_t d[8]);
fnn_t *fnn_global(void);
#endif
