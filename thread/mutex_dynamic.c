#include <pthread.h>
#include "errors.h"

/*
 * Define a structure, with a mutex.
 */
typedef struct my_struct_tag {	
	pthread_mutex_t mutex; 	// protects access to value
	int 			values; // access protected by mutex
} my_struct_t;

int main()
{
	my_struct_t *data;
	int status;

	data = (my_struct_t *) malloc(sizeof(my_struct_t));
	if (data == NULL) errno_abort("Allocate structure");
	status = pthread_mutex_init(&data->mutex, NULL);
	if (status != 0) err_abort(status, "Init mutex");

	status = pthread_mutex_destroy(&data->mutex);
	if (status != 0) err_abort(status, "Destroy mutex");
	free(data);

	return 0;
}
