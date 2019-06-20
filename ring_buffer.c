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
};

/* init pointers and size of ring_buffer */
int ring_buffer_init(struct ring_buffer *circle) {
    circle->size = RING_BUFFER_SIZE + 1;    /* set size for buffer, +1 for the real size */
                                            /* cuz head == tail could mean empty or only 1 element*/
    circle->ring = (uint32_t *)malloc(sizeof(uint32_t) * circle->size);    /* try malloc */

    if(!circle->ring) {    /* malloc failed, return */
        printf("ring buffer malloc failed\n");
        return 0;
    }
    circle->head = 0u;   /* set head index */
    circle->tail = 0u;   /* set tail index */
    return 1;
}

/* insert item into ring_buffer */
uint8_t ring_buffer_push(struct ring_buffer *circle, uint32_t item) {

    /* find next head and tail indexes */
    uint32_t head_next = (circle->head + 1 == circle->size) ? 0 : circle->head + 1;
    uint32_t tail_next = (circle->tail + 1 == circle->size) ? 0 : circle->tail + 1;

    /* check if ring buffer is full */
    if(head_next == circle->tail) {
        printf("ring buffer is full.\n");
        return 0;   /* nothing left to do */
    }

    /* move head */
    circle->head = head_next;

    /* set new item */
    circle->ring[head_next] = item;

    /* print newly set element */
    printf("val %u was added to the head\n", item);

    return 1;   /* successfully written */
}

/* pop item from ring_buffer */
uint32_t ring_buffer_pop(struct ring_buffer *circle) {

    /* find next tail index and save current one */
    uint32_t tail_next = (circle->tail + 1 == circle->size) ? 0u : circle->tail + 1;

    /* check if ring buffer is empty */
    if(circle->head == circle->tail) {
        printf("ring buffer is empty.\n");
        return 0;   /* nothing left to do */
    }

    /* move tail */
    circle->tail = tail_next;

    /* print the popped item */
    printf("val %u was popped from the tail.\n", circle->ring[tail_next]);

    /* return last tail item */
    return circle->ring[circle->tail];
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

    for(j = 0; j < RING_BUFFER_SIZE; ++j) {
        ring_buffer_push(&circle, 10 * j);
    }

    for(j = 0; j < RING_BUFFER_SIZE; ++j) {
        ring_buffer_pop(&circle);
    }

    // @EXIT 2
    exit(EXIT_SUCCESS);
}

#endif