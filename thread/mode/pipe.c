#include <pthread.h>
#include "errors.h"

/*
 * Internal structure describing a "stage" in the
 * pipeline. One for each thread, plus a "result 
 * stage" where the final thread can stash the value.
 */
typedef struct stage_tag {
    pthread_mutex_t  mutex;      /* protect data */
    pthread_cond_t   avail;      /* data available */
    pthread_cond_t   ready;      /* ready for receiving data */
    int              data_ready; /* data is ready to be processed */
    long             data;       /* data to process */
    pthread_t        thread;     /* thread for stage */
    struct stage_tag *next;      /* next stage */
} stage_t;

/*
 * External structure representing the entire pipeline.
 */
typedef struct pipe_tag {
    pthread_mutex_t mutex;  /* mutex to protect pipe */
    stage_t         *head;  /* first staget */
    stage_t         *tail;  /* final staget */
    int             stages; /* number of stages */
    int             active; /* active data elements */
} pipe_t;

/*
 * Internal function to send a "message" to the
 * specified pipe stage. Threads use this to pass
 * along the modified data item.
 */
static int pipe_send(stage_t *stage, long data)
{
    int status;

    status = pthread_mutex_lock(&stage->mutex);
    if (status != 0)
        return status;

    /* If there's a data in the pipe stage, wait for it
       to be coinsumed. */
    while (stage->data_ready) {
        status = pthread_cond_wait(&stage->ready, &stage->mutex);
        if (status != 0) {
            pthread_mutex_unlock(&stage->mutex);
            return status;
        }
    }

    /* send the new data */
    stage->data = data;
    stage->data_ready = 1;
    status = pthread_cond_wait(&stage->avail, &stage->mutex);
    if (status != 0) {
        pthread_mutex_unlock(&stage->mutex);
        return status;
    }
    pthread_mutex_unlock(&stage->mutex);
    return status;
}

/*
 * The thread start routine for pipe stage threads.
 * Each will wait for a data item passed from the 
 * caller or the previous stage, modify the data 
 * and pass it along to the next (or final) stage.
 */
static void *pipe_stage(void *arg)
{
    stage_t *stage = (stage_t *) arg;
    stage_t *next_stage = stage->next;
    int status;

    status = pthread_mutex_lock(&stage->mutex);
    if (status != 0)
        err_abort(status, "Lock pipe stage");

    while (1) {
        while (stage->data_ready != 1) {
            status = pthread_cond_wait(&stage->avail, &stage->mutex);
            if (status != 0)
                err_abort(status, "Wait for previous stage");
        }
        pipe_send(next_stage, stage->data + 1);
        stage->data_ready = 0;
        status = pthread_cond_wait(&stage->ready, &stage->mutex);
        if (status != 0)
            err_abort(status, "Wake previous stage");
    }

    /* Notice that the routine never unlocks the stage->mutex.
       The call to pthread_cond_wait implicitly unlocks the
       mutex while the thread is waiting, allowing other threads
       to make progress. Because the loop never terminates, this 
       function has no need to unlock the mutex emplicityly. */
}

/*
 * External interface to create a pipeline. All the
 * data is initialilzed and the threads created. They'll
 * wait for data.
 */
static int pipe_create(pipe_t *pipe, int stages)
{
    int status;
    stage_t *stage, **link = &pipe->head;

    /* init pipeline */
    status = pthread_mutex_init(&pipe->mutex, NULL);
    if (status != 0)
        err_abort(status, "init pipeline mutex");
    pipe->stages = stages;
    pipe->active = 0;

    /* init stages */
    for (int i = 0; i <= stages; i++) {
        stage = malloc(sizeof(stage_t));
        if (stage == NULL)
            errno_abort("new stage");
        status = pthread_mutex_init(&stage->mutex, NULL);
        if (status != 0)
            err_abort(status, "init stage mutex");
        status = pthread_cond_init(&stage->avail, NULL);
        if (status != 0)
            err_abort(status, "init stage avail");
        status = pthread_cond_init(&stage->ready, NULL);
        if (status != 0)
            err_abort(status, "init stage ready");
        stage->data_ready = 0;

        *link = stage;
        link = &stage->next;
    }
    *link = NULL;        /* terminate list */
    pipe->tail = stage; /* record the tail */

    /* create thread for each stage */
    stage = pipe->head;
    while (stage->next != NULL) {
        status = pthread_create(&stage->thread, NULL, pipe_stage, stage);
        if (status != 0)
            err_abort(status, "create pipe stage thread");
        stage = stage->next;
    }

    return 0;
}

/*
 * External interface to start a pipeline by passing
 * data to the first stage. The routine returns while 
 * the pipelines processes in parallel. Call the 
 * pipe_result return to collect the final stage values
 * (note that the pipe will stall when each stage fills
 * until the result is collected).
 */
static int pipe_start(pipe_t *pipe, long value)
{
    int status;

    status = pthread_mutex_lock(&pipe->mutex);
    if (status != 0)
        err_abort(status, "lock pipe mutex");
    pipe->active++;
    status = pthread_mutex_unlock(&pipe->mutex);
    if (status != 0)
        err_abort(status, "unlock pipe mutex");
    pipe_send(pipe->head, value);
    return 0;
}

/*
 * Collect the result of the pipeline. Wait for a 
 * result if the pipeline hasn't produced one.
 */
static int pipe_result(pipe_t *pipe, long *result)
{
    stage_t *tail = pipe->tail;
    int empty = 0;
    int status;

    status = pthread_mutex_lock(&pipe->mutex);
    if (status != 0)
        err_abort(status, "lock pipe mutex");

    if (pipe->active <= 0)
        empty = 1;
    else 
        pipe->active--;

    status = pthread_mutex_unlock(&pipe->mutex);
    if (status != 0)
        err_abort(status, "unlock pipe mutex");

    if (empty)
        return 0;

    pthread_mutex_lock(&tail->mutex);
    while (tail->data_ready == 0)
        pthread_cond_wait(&tail->avail, &tail->mutex);
    *result = tail->data;  
    tail->data_ready = 0;
    pthread_cond_signal(&tail->ready);
    pthread_mutex_lock(&tail->mutex);

    return 1;
}

int main(void)
{
    pipe_t my_pipe;
    long value, result;
    char line[128];

    pipe_create(&my_pipe, 10);
    puts("Enter integer values, or \"=\" for next result\n");

    while (1) {
        fputs("Data> ", stdout);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL)
            exit(0);
        if (strlen(line) <= 1)
            continue;
        if (strlen(line) <= 2 && line[0] == '=') {
            if (pipe_result(&my_pipe, &result))
                printf("result is %ld\n", result);
            else
                puts("Pipe is empty\n");
        } else {
            if (sscanf(line, "%ld", &value) < 1)
                fputs("Enter an integer value\n", stderr);
            else
                pipe_start(&my_pipe, value);
        }
    }

    return 0;
}
