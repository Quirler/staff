#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#define RING_BUFFER_SIZE 15

struct ring_buffer {
    uint16_t *start;
    uint16_t *head;
    uint16_t *tail;
    uint16_t size;
};

/* init pointers and size of ring_buffer */
void ring_buffer_init(struct ring_buffer *circle) {
    circle->size = RING_BUFFER_SIZE;    /* get size for buffer */
    circle->start = (uint16_t *)malloc(sizeof(uint16_t) * circle->size);    /* try malloc */
    if(!circle->start) {    /* malloc failed, EXIT */
        printf("malloc failed\n");
        exit(EXIT_FAILURE);
    }
    circle->head = circle->start;   /* set head pointer */
    circle->tail = circle->start;   /* set tail pointer */
}

/* insert item into ring_buffer */
int ring_buffer_push(struct ring_buffer *circle, uint16_t item) {
    
    uint16_t *overflow = circle->start + circle->size - 1;  /* temp pointer for last possible item */
    uint16_t *next_head;    /* where we intent to go */

    if(circle->head == overflow) {    /* here last available position */
        next_head = circle->start;   /* next position outofrange -> start */
    }
    else {
        next_head = circle->head + 1;   /* just incremented head */
    }

    if(next_head != circle->tail) { /* actual check for free space */
        circle->head = next_head; /* we just calculated that its next correct position */
        *circle->head = item;   /* include new item */
        printf("item %u was added to the head\n", item);
    }
    else {
        printf("Ring buffer is full!\n");    /* failed to write the new item */
        return -1;  /* failed to write the new item */
    }
    return 1;   /* successfully written */
}

/* pop item from ring_buffer */
void ring_buffer_pop(struct ring_buffer *circle, uint16_t *item) {
    if(circle->tail == circle->head) {  /* nothing to pop */
        printf("Ring buffer is empty!\n");
    }
    else {
        uint16_t *overflow = circle->start + circle->size - 1;
        if(circle->tail == overflow) {
            circle->tail = circle->start;
        }
        else
            circle->tail++;
        
        *item = *circle->tail;
        printf("item %u was deleted from the tail\n", *item);
    }
}

void main(int argc, char **argv) {
    struct ring_buffer circle;
    uint16_t item;
    uint32_t j;
    /* init pointers and size of ring_buffer */
    /* @EXIT 1 */
    ring_buffer_init(&circle);
    
    for(j = 0; j < RING_BUFFER_SIZE + 5; ++j) {
        ring_buffer_push(&circle, 10 * j);
    }

    for(j = 0; j < RING_BUFFER_SIZE - 10; ++j) {
        ring_buffer_pop(&circle, &item);
    }

    for(j = 0; j < RING_BUFFER_SIZE - 13; ++j) {
        ring_buffer_push(&circle, 10 * j);
    }

    for(j = 0; j < RING_BUFFER_SIZE + 10; ++j) {
        ring_buffer_pop(&circle, &item);
    }

    // @EXIT 2
    exit(EXIT_SUCCESS);
}