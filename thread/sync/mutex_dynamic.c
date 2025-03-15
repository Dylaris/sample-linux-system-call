#include <pthread.h>
#include "errors.h"

/*
 * A dyanamic initialized mutex.
 */

typedef struct my_struct_tag {
    pthread_mutex_t mutex;
    int             value;
} my_struct_t;

int main(void)
{
    my_struct_t *data = malloc(sizeof(my_struct_t));
    if (data == NULL) 
        errno_abort("malloc");

    int status = pthread_mutex_init(&data->mutex, NULL);
    if (status != 0) 
        err_abort(status, "pthread_mutex_init"); 
    status = pthread_mutex_destroy(&data->mutex);
    if (status != 0)
        err_abort(status, "pthread_mutex_destroy");

    free(data);
    return 0;
}
