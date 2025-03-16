#include <pthread.h>
#include "errors.h"

#define ITERATIONS 10

static int backoff = 1;     /* whether to backoff */
static int yield_flag = 0;  /* 0, no yield; > 0, yield; < 0, sleep */

static pthread_mutex_t mutexes[3] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER
};

/*
 * Lock all mutexes in order.
 */
static void *lock_forward(void *arg)
{
    (void) arg;

    int status, backoff_count = 0;

    for (int i = 0; i < ITERATIONS; i++) {
        for (int j = 0; j < 3; j++) {
            if (j == 0) {
                /* normal lock */
                status = pthread_mutex_lock(&mutexes[j]);
                if (status != 0) err_abort(status, "lock mutex in forward");
            } else {
                if (backoff) /* try lock */
                    status = pthread_mutex_trylock(&mutexes[j]);
                else
                    status = pthread_mutex_lock(&mutexes[j]);

                if (status == EBUSY) {
                    backoff_count++;
                    /* backoff */
                    printf("forward locker backing off at %d\n", j);
                    for (; j >= 0; j--) {
                        status = pthread_mutex_unlock(&mutexes[j]);
                        if (status != 0) err_abort(status, "backoff");
                    }
                } else {
                    if (status != 0) err_abort(status, "lock mutex in forward");
                    printf("forward locker got at %d\n", j);
                }
            }
            
            if (yield_flag) {
                if (yield_flag > 0)
                    sched_yield();
                else
                    sleep(1);
            }
        }

        printf("lock forward got all locks, %d backoffs\n", backoff_count);
        pthread_mutex_unlock(&mutexes[0]);
        pthread_mutex_unlock(&mutexes[1]);
        pthread_mutex_unlock(&mutexes[2]);
        sched_yield();
    }

    return NULL;
}

/*
 * Lock all mutexes in reverse order.
 */
static void *lock_backward(void *arg)
{
    (void) arg;

    int status, backoff_count = 0;

    for (int i = 0; i < ITERATIONS; i++) {
        for (int j = 2; j >= 0; j--) {
            if (j == 2) {
                /* normal lock */
                status = pthread_mutex_lock(&mutexes[j]);
                if (status != 0) err_abort(status, "lock mutex in backward");
            } else {
                if (backoff) /* try lock */
                    status = pthread_mutex_trylock(&mutexes[j]);
                else
                    status = pthread_mutex_lock(&mutexes[j]);

                if (status == EBUSY) {
                    backoff_count++;
                    /* backoff */
                    printf("backward locker backing off at %d\n", j);
                    for (; j < 3; j++) {
                        status = pthread_mutex_unlock(&mutexes[j]);
                        if (status != 0) err_abort(status, "backoff");
                    }
                } else {
                    if (status != 0) err_abort(status, "lock mutex in backward");
                    printf("backward locker got at %d\n", j);
                }
            }

            if (yield_flag) {
                if (yield_flag > 0)
                    sched_yield();
                else
                    sleep(1);
            }
        }

        printf("lock backward got all locks, %d backoffs\n", backoff_count);
        pthread_mutex_unlock(&mutexes[0]);
        pthread_mutex_unlock(&mutexes[1]);
        pthread_mutex_unlock(&mutexes[2]);
        sched_yield();
    }

    return NULL;
}


int main(int argc, char **argv)
{
    if (argc > 1) backoff = atoi(argv[1]);
    if (argc > 2) yield_flag = atoi(argv[2]);
    
    pthread_t forward, backward;
    int status;

    status = pthread_create(&forward, NULL, lock_forward, NULL);
    if (status != 0) err_abort(status, "create thread forward");
    status = pthread_create(&backward, NULL, lock_backward, NULL);
    if (status != 0) err_abort(status, "create thread backward");
    
    pthread_exit(NULL);
}
