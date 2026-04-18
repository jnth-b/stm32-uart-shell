#include "ringbuf.h"

void rb_init(RingBuf_t *rb)
{
    rb->head = rb->tail = 0;
}

int rb_write(RingBuf_t *rb, char c)
{
    uint32_t next = (rb->head + 1) % RING_BUF_SIZE;
    if (next == rb->tail) return 0;   /* full, drop it */
    rb->buf[rb->head] = c;
    rb->head = next;
    return 1;
}

int rb_read(RingBuf_t *rb, char *c)
{
    if (rb->head == rb->tail) return 0;   /* empty */
    *c = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUF_SIZE;
    return 1;
}

int rb_empty(RingBuf_t *rb)
{
    return rb->head == rb->tail;
}
