#include "errors.h"
#include <pthread.h>

/*
 * Process alarm requests through multiple threads.
 */

typedef struct alarm_tag {
    int seconds;
    char message[64];
} alarm_t;

void *alarm_thread(void *arg)
{
    alarm_t *alarm = (alarm_t *) arg;
    int status = pthread_detach(pthread_self());
    if (status != 0) err_abort(status, "pthread_detach");
    sleep(alarm->seconds);
    printf("sleep %d -> %s\n", alarm->seconds, alarm->message);
    free(alarm);
    return NULL;
}

int main(void)
{
    char line[128];
    alarm_t *alarm;
    int status;
    pthread_t thread;

    while (1) {
        /* prompt */
        fputs("Alarm> ", stdout);
        fflush(stdout);

        /* read input to line buffer */
        if (fgets(line, 128, stdin) == NULL) exit(0);
        if (strlen(line) <= 1) continue; /* only \n */

        alarm = malloc(sizeof(alarm_t));
        if (!alarm) errno_abort("malloc");
        if (sscanf(line, "%d %64[^\n]", 
            &alarm->seconds, alarm->message) < 2) {
            fprintf(stderr, "Bad command!\n");
            free(alarm);
            continue;
        }

        status = pthread_create(&thread, NULL, alarm_thread, alarm);
        if (status != 0) err_abort(status, "pthread_create");
    }

    exit(0);
}
