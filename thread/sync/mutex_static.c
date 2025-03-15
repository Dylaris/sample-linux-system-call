#include <pthread.h>
#include "errors.h"

/*
 * A static initialized mutex.
 */

typedef struct my_struct_tag {
    pthread_mutex_t mutex; /* protect access to value */
    int             value; /* access protected by mutex */
} my_struct_t;

my_struct_t data = {PTHREAD_MUTEX_INITIALIZER, 0};

int main(void)
{
    return 0;
}
