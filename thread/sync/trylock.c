#include <pthread.h>
#include "errors.h"

#define SPIN 10000000
long counter;
time_t end_time;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Until end_time, increment the counter by SPIN each second.
 */
void *counter_thread(void *arg)
{
    (void) arg;

    int status;

    while (time(NULL) < end_time) {
        status = pthread_mutex_lock(&mutex);
        if (status != 0) err_abort(status, "lock mutex in counter");

        for (int i = 0; i < SPIN; i++)
            counter++;

        status = pthread_mutex_unlock(&mutex);
        if (status != 0) err_abort(status, "unlock mutex in counter");

        sleep(1);
    }

    printf("Counter is %#lx\n", counter);
    return NULL;
}

/*
 * Until end_time, check the counter every 3 seconds.
 */
void *monitor_thread(void *arg)
{
    (void) arg;

    int status;
    int miss = 0;

    while (time(NULL) < end_time) {
        sleep(3);

        status = pthread_mutex_trylock(&mutex);
        if (status != EBUSY) {
            if (status != 0) err_abort(status, "trylock mutex in monitor");
            printf("Counter is %ld\n", counter / SPIN);
            status = pthread_mutex_unlock(&mutex);
            if (status != 0) err_abort(status, "unlock mutex in monitor");
        } else {
            miss++;
        }
    }

    printf("Monitor misses undate %d times\n", miss);
    return NULL;
}

int main(void)
{
    pthread_t monitor_thread_id;
    pthread_t counter_thread_id;
    int status;

    counter = 0;
    end_time = time(NULL) + 60; /* run for 1 minute */

    status = pthread_create(&counter_thread_id, NULL, counter_thread, NULL);
    if (status != 0) err_abort(status, "create counter thread");
    status = pthread_create(&monitor_thread_id, NULL, monitor_thread, NULL);
    if (status != 0) err_abort(status, "create monitor thread");

    status = pthread_join(counter_thread_id, NULL);
    if (status != 0) err_abort(status, "join counter thread");
    status = pthread_join(monitor_thread_id, NULL);
    if (status != 0) err_abort(status, "join monitor thread");

    return 0;
}
