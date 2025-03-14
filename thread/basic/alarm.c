#include "errors.h"

/*
 * Process one alarm request each time.
 */

int main(void)
{
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

        /* process request */
        if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2) {
            fprintf(stderr, "Bad command!\n");
        } else {
            sleep(seconds);
            printf("sleep %d -> %s\n", seconds, message);
        }
    }

    exit(0);
}
