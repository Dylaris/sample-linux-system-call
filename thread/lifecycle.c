#include <pthread.h>
#include "errors.h"

/*
 * thread start routine
 */
void *thread_routine(void *arg)     // running
{
    return arg;     // terminated
}

int main()
{
    int status;
    pthread_t thread_id;
    void *thread_result;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);    // ready
    if (status != 0) err_abort(status, "Create thread");

    status = pthread_join(thread_id, &thread_result);
    if (status != 0) err_abort(status, "Join thread");

    if (thread_result == NULL)
        return 0;
    else
        return 1;
}