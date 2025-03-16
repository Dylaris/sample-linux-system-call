#include <pthread.h>
#include <time.h>
#include "errors.h"

typedef struct my_struct_tag {
    pthread_mutex_t mutex; /* protect access to value */
    pthread_cond_t  cond;  /* signal change to value */
    int             value; /* access protected by mutex */
} my_struct_t;

static my_struct_t data = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond  = PTHREAD_COND_INITIALIZER,
    .value = 0
};

/*
 * The wait thread should sleep for the duration of "hibernation" to
 * allow the main thread an opportunity to wait for the condition variable.
 *
 * The main thread waits for the condition variable for up to 2 seconds.
 * 
 * hibernation > 2 ---> time out
 * hibernation = 2 ---> competive
 * hibernation < 2 ---> no time out
 */
static int hibernation = 1;

static void *wait_thread(void *arg)
{
    (void) arg;
    int status;

    sleep(hibernation);

    status = pthread_mutex_lock(&data.mutex);
    if (status != 0) err_abort(status, "lock mutex in wait thread");
    data.value = 1; /* set predicats */
    status = pthread_cond_signal(&data.cond);
    if (status != 0) err_abort(status, "signal in wait thread");
    status = pthread_mutex_unlock(&data.mutex);
    if (status != 0) err_abort(status, "unlock mutex in wait thread");

    return NULL;
}


int main(int argc, char **argv)
{
    int status;
    pthread_t wait_thread_id;

    if (argc > 1) hibernation = atoi(argv[1]);

    status = pthread_create(&wait_thread_id, NULL, wait_thread, NULL);
    if (status != 0) err_abort(status, "create wait thread");

    /* wait the condition variable */
    status = pthread_mutex_lock(&data.mutex);
    if (status != 0) err_abort(status, "lock mutex in main thread");

    struct timespec timeout = {
        .tv_sec  = time(NULL) + 2,
        .tv_nsec = 0
    };

    while (data.value == 0) {
        status = pthread_cond_timedwait(&data.cond, &data.mutex, &timeout);
        if (status == ETIMEDOUT) {
            puts("Condition wait time out!\n"); break;
        } else {
            if (status != 0) err_abort(status, "time wait");
        }
    }

    if (data.value != 0) puts("Condition was signaled!\n");
    status = pthread_mutex_unlock(&data.mutex);
    if (status != 0) err_abort(status, "unlock mutex in main thread");

    return 0;
}
