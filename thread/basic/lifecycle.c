#include "errors.h"
#include <pthread.h>

/*
 * Thread start routine.
 */

static void *thread_routine(void *arg)
{
    return arg;
}

int main(void)
{
    int status;
    pthread_t thread;
    void *thread_result;
    

    status = pthread_create(&thread, NULL, thread_routine, NULL);
    if (status != 0) err_abort(status, "pthread_create");

    status = pthread_join(thread, &thread_result);
    if (status != 0) err_abort(status, "pthread_join");

    if (thread_result == NULL) 
        return 0;
    else 
        return 1;
}