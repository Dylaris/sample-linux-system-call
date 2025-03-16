#include <pthread.h>
#include "errors.h"

/*
 * Dynamic initialized mutex and condtion variable.
 */

typedef struct my_struct_tag {
    pthread_mutex_t mutex; /* protect access to value */
    pthread_cond_t  cond;  /* signal change to value */
    int             value; /* access protected by mutex */
} my_struct_t;

int main(void)
{
    int status;

    my_struct_t *data = malloc(sizeof(my_struct_t));
    if (data == NULL) errno_abort("mallco");

    status = pthread_mutex_init(&data->mutex, NULL);
    if (status != 0) err_abort(status, "init mutex");
    status = pthread_cond_init(&data->cond, NULL);
    if (status != 0) err_abort(status, "init condition");

    status = pthread_cond_destroy(&data->cond);
    if (status != 0) err_abort(status, "destroy condition");
    status = pthread_mutex_destroy(&data->mutex);
    if (status != 0) err_abort(status, "destroy mutex");

    return 0;
}
