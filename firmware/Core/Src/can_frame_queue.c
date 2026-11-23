#include "can_frame_queue.h"

void can_capture_queue_init(can_capture_queue_t *q)
{
    if (q == 0) return;
    q->head = 0u;
    q->tail = 0u;
    q->dropped = 0u;
    q->rejected = 0u;
}

bool can_capture_queue_push_isr(can_capture_queue_t *q, const can_capture_frame_t *f)
{
    if (q == 0 || f == 0 || f->dlc > CAN_CAPTURE_MAX_DLC ||
        f->extended > 1u || f->remote > 1u ||
        (f->extended == 0u && f->identifier > 0x7ffu) ||
        (f->extended != 0u && f->identifier > 0x1fffffffu)) {
        if (q != 0) q->rejected++;
        return false;
    }
    const uint16_t next = (uint16_t)((q->head + 1u) % CAN_CAPTURE_QUEUE_CAPACITY);
    if (next == q->tail) {
        q->dropped++;
        return false;
    }
    q->frames[q->head] = *f;
    /* Frame contents must be visible before the new head (ARM PM0214
     * Sections 2.2.2/2.2.4: software must order memory accesses with barriers). */
    __atomic_thread_fence(__ATOMIC_RELEASE);
    q->head = next;
    return true;
}

bool can_capture_queue_pop(can_capture_queue_t *q, can_capture_frame_t *out)
{
    if (q == 0 || out == 0 || q->tail == q->head) return false;
    __atomic_thread_fence(__ATOMIC_ACQUIRE);   /* read head before frame data */
    *out = q->frames[q->tail];
    __atomic_thread_fence(__ATOMIC_RELEASE);   /* finish reading before freeing slot */
    q->tail = (uint16_t)((q->tail + 1u) % CAN_CAPTURE_QUEUE_CAPACITY);
    return true;
}
