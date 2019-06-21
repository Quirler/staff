#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#define RING_BUFFER_SIZE 15

struct ring_buffer {
    uint32_t *ring;
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t occupied;
};

/* init pointers and size of ring_buffer */
int ring_buffer_init(struct ring_buffer *circle) {
    circle->size = RING_BUFFER_SIZE;    /* set size for buffer */
    circle->ring = (uint32_t *)malloc(sizeof(uint32_t) * circle->size);    /* try malloc */
    if(!circle->ring) {   /* malloc failed, return */
        printf("ring buffer malloc failed\n");
        return 0;
    }
    circle->head = 0u;      /* set head index */
    circle->tail = 0u;      /* set tail index */
    circle->occupied = 0u;  /* number of items; used for full/empty flag too */
    return 1;
}

/* insert item into ring_buffer */
uint8_t ring_buffer_push(struct ring_buffer *circle, uint32_t item) {

    /* check if ring buffer is full */
    if(circle->occupied == circle->size) {
        printf("ring buffer is full.\n");
        return 0;   /* write failed; nothing left to do */
    }

    /* set new item and move head to next empty slot*/
    circle->ring[circle->head++] = item;

    /* recalculate head index if bigger than size */
    circle->head %= circle->size;

    /* print newly set element */
    printf("val %u was added to the head\n", item);

    /* if push is called and not full occupied increments */
    ++circle->occupied;

    return 1;   /* successfully written */
}

/* pop item from ring_buffer */
uint32_t ring_buffer_pop(struct ring_buffer *circle) {

    /* check if ring buffer is empty */
    if(circle->occupied == 0) {
        printf("ring buffer is empty.\n");
        return 0;   /* pop failed; nothing left to do */
    }

    /* var for popped item */
    uint32_t item;

    /* pop the last item and move tail to last used slot */
    item = circle->ring[circle->tail++];

    /* print the popped item */
    printf("val %u was popped from the tail.\n", item);

    /* recalculate tail index if bigger than size */
    circle->tail %= circle->size;

    /* if pop is called and not empty occupied decrements */
    --circle->occupied;

    /* return popped item */
    return item;
}

#ifdef DEBUG

void main(int argc, char **argv) {
    struct ring_buffer circle;
    uint32_t item;
    uint32_t j;

    /* init pointers and size of ring_buffer */
    /* @EXIT 1 */
    if(!ring_buffer_init(&circle)) {
        exit(EXIT_FAILURE);
    };
    
    for(j = 0; j < RING_BUFFER_SIZE + 5; ++j) {
        ring_buffer_push(&circle, 10 * j);
    }

    for(j = 0; j < RING_BUFFER_SIZE - 10; ++j) {
        ring_buffer_pop(&circle);
    }

    for(j = 0; j < RING_BUFFER_SIZE - 13; ++j) {
        ring_buffer_push(&circle, 10 * j);
    }

    for(j = 0; j < RING_BUFFER_SIZE + 10; ++j) {
        ring_buffer_pop(&circle);
    }

    for(j = 0; j < RING_BUFFER_SIZE - 5; ++j) {
        ring_buffer_push(&circle, 10 * j);
    }

    for(j = 0; j < RING_BUFFER_SIZE + 5; ++j) {
        ring_buffer_pop(&circle);
    }

    for(j = 0; j < RING_BUFFER_SIZE - 10; ++j) {
        ring_buffer_push(&circle, 10 * j);
    }

    for(j = 0; j < RING_BUFFER_SIZE - 1; ++j) {
        ring_buffer_push(&circle, 10 * j);
    }

    for(j = 0; j < RING_BUFFER_SIZE + 25; ++j) {
        ring_buffer_pop(&circle);
    }

    // @EXIT 2
    exit(EXIT_SUCCESS);
}

#endif