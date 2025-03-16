#include <pthread.h>
#include "errors.h"

/*
 * Static initialized mutex and condtion variable.
 */

typedef struct my_struct_tag {
    pthread_mutex_t mutex; /* protect access to value */
    pthread_cond_t  cond;  /* signal change to value */
    int             value; /* access protected by mutex */
} my_struct_t;

my_struct_t data = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond  = PTHREAD_COND_INITIALIZER,
    .value = 0
};

int main(void)
{
    return 0;
}
