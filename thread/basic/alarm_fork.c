#include "errors.h"
#include <sys/types.h>
#include <wait.h>

/*
 * Process alarm requests through multiple processes.
 */

int main(void)
{
    pid_t pid;
    int seconds;
    char line[128];
    char message[64];

    while (1) {
        /* prompt */
        fputs("Alarm> ", stdout);
        fflush(stdout);

        /* read input to line buffer */
        if (fgets(line, 128, stdin) == NULL) exit(0);
        if (strlen(line) <= 1) continue; /* only \n */

        if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2) {
            fprintf(stderr, "Bad command!\n");
            continue;
        }

        pid = fork();
        if (pid == (pid_t) -1) {
            /* failed */
            errno_abort("fork");
        } else if (pid == (pid_t) 0) {
            /* in the child process, process request */
            sleep(seconds);
            printf("sleep %d -> %s\n", seconds, message);
            exit(0);
        } else {
            /* in the parent, collect the terminated child process */
            do {
                pid = waitpid((pid_t) -1, NULL, WNOHANG); /* non-block */
                if (pid == (pid_t) -1) errno_abort("waitpid");
            } while (pid != (pid_t) 0);
        }
    }

    exit(0);
}
