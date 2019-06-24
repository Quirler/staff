#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

#ifndef _RING_BUF_
#define _RING_BUF_

typedef struct {
    uint32_t *ring;
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t occupied;
    pthread_mutex_t mtx;
    pthread_cond_t prod;
    pthread_cond_t cons;
}ring_buffer;

/* init pointers and size of ring_buffer */
ring_buffer new_ring_buffer(uint32_t size);

/* insert item into ring_buffer: 0->fail, 1->success*/
uint8_t ring_buffer_push(ring_buffer *circle, uint32_t item);

/* pop item from ring_buffer: 0->fail, item->success */
uint32_t ring_buffer_pop(ring_buffer *circle);

#endif