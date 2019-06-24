#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include "condvar.h"

#ifdef _RING_BUF_

#define RING_BUF_SIZE 100
#define PRODUCERS 4
#define CONSUMERS 3

void *produce(void *ctx) {
    /* get context with a cast */
    ring_buffer *rb = (ring_buffer *)ctx;

    /* new item */
    uint32_t val = rand() % 1000000;

    /* forever produce */
    while(1) {

        /* simulate exhausting job OUTSIDE THE MUTUAL ACTS */
        long int i = 0;
        while(i < 0x7FFFFFF) {
            if(i % 10000000 == 0) {
                fprintf(stdout, "PRODUCER[%03lu] is WORKING to produce something\n", pthread_self() % 1000);
                fflush(stdout);
            }
            i++;
        };
        // usleep(rand() % val);


        /* mutex combo with condvar */
        pthread_mutex_lock(&rb->mtx);

        /* while no free space mtx bouncing and wait consumer's signal */
        while(rb->occupied >= rb->size) {
            pthread_cond_wait(&rb->cons, &rb->mtx);
            fprintf(stdout, "PRODUCER[%03lu] is WAITING for some free space\n", pthread_self() % 1000);
            fflush(stdout);
        }

        /* when there is some free space */
        /* push new item and check the result */
        if(!ring_buffer_push(rb, val)) {    /* failed */
            fprintf(stderr, "Oops!! bad push\n");
            fflush(stderr);
        }
        else {  /* succeeded */
            fprintf(stdout, "PRODUCER[%03lu] PUSHED %04x\t\t\toccupied: %u\n", pthread_self() % 1000, val, rb->occupied);
            fflush(stdout);

            /* make signal for consumers that there is some data available */
            //pthread_cond_signal(&rb->prod);
            pthread_cond_signal(&rb->prod);
        }

        /* this bitch unlocks that shit isnt it obviously?!?! */
        pthread_mutex_unlock(&rb->mtx);

        // usleep(val);
    }
}

void *consume(void *ctx) {
    /* get context with a cast */
    ring_buffer *rb = (ring_buffer *)ctx;

    /* var for the popped items */
    uint32_t item;

    /* forever consume */
    while(1) {

        /* mutex combo with condvar */
        pthread_mutex_lock(&rb->mtx);

        /* while there is no any data mtx bouncing and wait a producer's signal */
        while(rb->occupied <= 0u) {
            pthread_cond_wait(&rb->prod, &rb->mtx);
            fprintf(stdout, "CONSUMER[%03lu] is WAITING for some available data\n", pthread_self() % 1000);
            fflush(stdout);
        }

        /* when there is some data available pop and*/
        /* check if the item was successfully popped */
        if(!(item = ring_buffer_pop(rb))) {   /* failed */
            fprintf(stderr, "Oops!! bad pop\n");
            fflush(stderr);
        }
        else {  /* succeeded */
            fprintf(stdout, "CONSUMER[%03lu] POPPED %04x\t\t\toccupied: %u\n", pthread_self() % 1000, item, rb->occupied);
            fflush(stdout);

            /* make signal for producers that there is some free space */
            //pthread_cond_signal(&rb->cons);
            pthread_cond_signal(&rb->cons);

        }

        /* this bitch unlocks that shit isnt it obviously?!?! */
        pthread_mutex_unlock(&rb->mtx);


        /* simulate exhausting job OUTSIDE THE MUTUAL ACTS */
        int i = 0;
        while(i < 0x7FFFFFF) {
            i++;
            if(i % 10000000 == 0) {
                fprintf(stdout, "CONSUMER[%03lu] is WORKING with popped data\n", pthread_self() % 1000);
                fflush(stdout);
            }
        };
        // usleep(item);
    }
}

void main(void) {

    /* create new ring-buffer object with condition variable */
    ring_buffer pool = new_ring_buffer(RING_BUF_SIZE);

    /* check if successfully allocated mem */
    if(!pool.ring) {
        exit(EXIT_FAILURE);
    }

    /* create threading modules */
    pthread_t producers[PRODUCERS];
    pthread_t consumers[CONSUMERS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    void **ret;

    int j;
    /* run producers and consumers */
    for(j = 0; j < PRODUCERS; ++j) {
        pthread_create(&producers[j], &attr, &produce, &pool);
    }
    for(j = 0; j < CONSUMERS; ++j) {
        pthread_create(&consumers[j], &attr, &consume, &pool);
    }

//freeze
while(1);

    /* join producers and consumers */
    for(j = 0; j < PRODUCERS; ++j) {
        pthread_join(producers[j], ret);
    }
    for(j = 0; j < CONSUMERS; ++j) {
        pthread_join(producers[j], ret);
    }
}

/* init pointers synchronizators and size of ring_buffer */
ring_buffer new_ring_buffer(uint32_t size) {
    ring_buffer object = {
        .size = size,   /* set total size of buffer */
        .head = 0u,     /* set head pointer */
        .tail = 0u,     /* set tail pointer */
        .occupied = 0u, /* set used space marker */
        .mtx = PTHREAD_MUTEX_INITIALIZER,   /* init mutex */
        .prod = PTHREAD_COND_INITIALIZER,   /* init producer's condition variable */
        .cons = PTHREAD_COND_INITIALIZER    /* init consumer's condition variable */
    };

    /* try malloc */
    object.ring = (uint32_t *)malloc(sizeof(uint32_t) * object.size);
    if(!object.ring) {      /* malloc failed */
        fprintf(stderr, "ring buffer malloc failed\n");
        fflush(stderr);
    }

    /* return newly created ring_buffer */
    return object;
}

/* insert item into ring_buffer: 0->fail, 1->success*/
uint8_t ring_buffer_push(ring_buffer *circle, uint32_t item) {

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
    //printf("val %u was added to the head\n", item);

    /* if push is called and not full occupied increments */
    ++circle->occupied;

    return 1;   /* successfully written */
}

/* pop item from ring_buffer: 0->fail, item->success */
uint32_t ring_buffer_pop(ring_buffer *circle) {

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
    //printf("val %u was popped from the tail.\n", item);

    /* recalculate tail index if bigger than size */
    circle->tail %= circle->size;

    /* if pop is called and not empty occupied decrements */
    --circle->occupied;

    /* return popped item */
    return item;
}

#endif