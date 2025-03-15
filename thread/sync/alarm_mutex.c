#include <pthread.h>
#include "errors.h"

/*
 * Process alarm requests using multiple threads with a mutex.
 * 
 * This ensures that the thread is blocked while processing an alarm request.
 * Once a thread receives an alarm, it will sleep until the processing is complete,
 * allowing it to handle new alarm requests afterward.
 */

typedef struct alarm_tag {
    struct alarm_tag *next;
    time_t           time; /* seconds from epoch */
    int              seconds;
    char             message[64];
} alarm_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /* mutex for alarm list, not alarm node */
alarm_t *alarm_list = NULL;

static void *alarm_thread(void *arg)
{
    (void) arg;

    int status;
    int sleep_time;
    time_t now;
    alarm_t *alarm;

    while (1) {
        status = pthread_mutex_lock(&mutex);
        if (status != 0) err_abort(status, "pthread_mutex_lock");

        /* determine the sleep time */
        alarm = alarm_list;
        if (alarm == NULL) {
            /* give the main thread a chance to run 
               (add a new request) */
            sleep_time = 1;
        } else {
            alarm_list = alarm->next; /* delete processed alarm */
            now = time(NULL);
            if (alarm->time > now)
                sleep_time = alarm->time - now;
            else 
                sleep_time = 0;
        }

        /* sleep after unlock 
           (allow the main thread to insert new request) */
        status = pthread_mutex_unlock(&mutex);
        if (status != 0) err_abort(status, "pthread_mutex_unlock");

        if (sleep_time > 0) 
            sleep(sleep_time);
        else 
            sched_yield(); /* switch to main thread if there has input waiting */

        /* response to alarm request */
        if (alarm != NULL) {
            printf("sleep %d -> %s\n", alarm->seconds, alarm->message);
            free(alarm);
        }
    }

    return NULL;
}

static void insert_alarm(alarm_t *alarm)
{
    alarm_t *prev = NULL, *cur = alarm_list;

    if (cur == NULL || alarm->time < cur->time) {
        /* empty */
        alarm->next = alarm_list;
        alarm_list = alarm;
        return;
    }

    while (cur != NULL) {
        if (alarm->time < cur->time)
            break;
        prev = cur;
        cur = cur->next;
    }
    alarm->next = cur->next;
    prev->next = alarm;
}

int main(void)
{
    char line[128];
    alarm_t *alarm;
    pthread_t thread;

    /* create a thread */
    int status = pthread_create(&thread, NULL, alarm_thread, NULL);
    if (status != 0) err_abort(status, "pthread_create");

    while (1) {
        /* prompt */
        fputs("Alarm> ", stdout);
        fflush(stdout);

        /* read input to line buffer */
        if (fgets(line, 128, stdin) == NULL) exit(0);
        if (strlen(line) <= 1) continue; /* only \n */

        /* init alarm */
        alarm = malloc(sizeof(alarm_t));
        if (alarm == NULL) errno_abort("malloc");
        if (sscanf(line, "%d %64[^\n]", 
            &alarm->seconds, alarm->message) < 2) {
            fprintf(stderr, "Bad command!\n");
            free(alarm);
            continue;
        }
        alarm->time = alarm->seconds + time(NULL);
        alarm->next = NULL;

        /* add this alarm to alarm list */
        status = pthread_mutex_lock(&mutex);
        if (status != 0) err_abort(status, "pthread_mutex_lock");
        insert_alarm(alarm);
        status = pthread_mutex_unlock(&mutex);
        if (status != 0) err_abort(status, "pthread_mutex_unlock");
    }

    exit(0);
}
