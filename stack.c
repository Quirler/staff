#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#define STACK_SIZE 20
#define N_THR_PUSH 2
#define N_THR_PULL 2

struct stack_context {
    uint8_t *stack;
    uint8_t *head;
    pthread_mutex_t mtx;
};

/* print the stack */
void print_stack(uint8_t *stack, uint8_t *head) {
    uint8_t *j;
    for(j = stack; j <= head; ++j) {
        printf("%02x\t", *j);
    }
    printf("\n");
}

/* thread push element into stack */
void *push(void *ctx_) {
    struct stack_context *ctx = (struct stack_context *)ctx_;
    uint8_t val;
    while(1) {
        val = 1 + rand() % 255; /* for new record and sleep */

        /* start of crit section */
        pthread_mutex_lock(&ctx->mtx);

        /* secure range of stack */
        if(ctx->head >= ctx->stack && ctx->head < ctx->stack + STACK_SIZE) {

            /* print the new val */
            printf("%lu push: %02x  ->\t", pthread_self(), val);

            if(ctx->head == ctx->stack + STACK_SIZE - 1) {  /* stack is full */
                printf("stack is full!\t");
            }
            else if(*ctx->head == 0x00) {   /* stack is empty */
                *ctx->head = val; /* write new val, pos 0 */
            }
            else {  /* stack is not full nor empty*/
                ++ctx->head; /* increment current head of the stack */
                *ctx->head = val; /* write new val */
            }
            /* print whole stack */
            print_stack(ctx->stack, ctx->head);       
        }

        /* end of crit section */
        fflush(stdout);
        pthread_mutex_unlock(&ctx->mtx);
        usleep(val*100);
    }
}

/* thread pull element into stack */
void *pull(void *ctx_) {
    struct stack_context *ctx = (struct stack_context *)ctx_;
    uint8_t val;
    while(1) {
        val = 1 + (rand() % 255);   /* just for the sleep */

        /* start crit section */
        pthread_mutex_lock(&ctx->mtx);

        /* secure range of stack */
        if(ctx->head >= ctx->stack && ctx->head < ctx->stack + STACK_SIZE) {
            printf("%lu pull: %02x  ->\t", pthread_self(), *ctx->head);

            *ctx->head = 0; /* delete current head value */

            if(ctx->head == ctx->stack) {   /* stack is empty */
                printf("stack is empty!\n");
                //sem_post(&ctx->sem_push);   /* let someone push */
            }
            else {  /* stack is not empty */
                --ctx->head; /* decrement current head of the stack */
                
                /* print whole stack only when not empty */
                print_stack(ctx->stack, ctx->head);
            }
        }

        /* end crit section */
        fflush(stdout);
        pthread_mutex_unlock(&ctx->mtx);
        usleep(val*100);
    }
}

void main(int argc, char **argv) {
    struct stack_context ctx = {
        .mtx = PTHREAD_MUTEX_INITIALIZER
    };

    // init stack
    // @EXIT 1
    ctx.stack = (uint8_t *)malloc(STACK_SIZE);
    if(!ctx.stack) {
        fprintf(stderr, "malloc failed\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
    ctx.head = ctx.stack;

    // init threads 
    pthread_t thread_push[N_THR_PUSH];
    pthread_t thread_pull[N_THR_PULL];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    void **ret = NULL;
    srand(time(NULL));

    uint16_t j;
    // create threads
    for(j = 0; j < N_THR_PUSH; ++j) {
        pthread_create(&thread_push[j], &attr, push, &ctx);
    }
    for(j = 0; j < N_THR_PULL; ++j) {
        pthread_create(&thread_pull[j], &attr, pull, &ctx);
    }

    // join threads
    for(j = 0; j < N_THR_PUSH; ++j) {
        pthread_join(thread_push[j], ret);
    }
    for(j = 0; j < N_THR_PULL; ++j) {
        pthread_join(thread_pull[j], ret);
    }

    free(ctx.stack);
    ctx.stack = NULL;
    // @EXIT 2
    exit(EXIT_SUCCESS);
}