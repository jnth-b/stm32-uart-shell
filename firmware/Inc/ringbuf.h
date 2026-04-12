#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>

#define RING_BUF_SIZE 128

typedef struct {
    volatile char     buf[RING_BUF_SIZE];
    volatile uint32_t head;   // ISR writes here
    volatile uint32_t tail;   // main loop reads here
} RingBuf_t;

void rb_init(RingBuf_t *rb);
int  rb_write(RingBuf_t *rb, char c);
int  rb_read(RingBuf_t *rb, char *c);
int  rb_empty(RingBuf_t *rb);

#endif
