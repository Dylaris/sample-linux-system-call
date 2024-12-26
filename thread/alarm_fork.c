#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	int seconds;
	char line[128], message[64];
	pid_t pid;

	while (1) {
		printf("Alarm> ");
		if (fgets(line, sizeof(line), stdin) == NULL) exit(EXIT_FAILURE);
		if (strlen(line) <= 1) continue;

		/*
		 * Parse input line into seconds (%d) and a message
		 * (%64[^\n]), consisting of up to 64 characters
		 * seperated from the seconds by whitespace.
		 */
		if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2) {
			fprintf(stderr, "Bad command\n");
		} else {
			pid = fork();			
			if (pid == (pid_t) -1) {
				fprintf(stderr, "FORK\n");
			} else if (pid == (pid_t) 0) {
				/* In the child, wait and then print a message */
				sleep(seconds);
				printf("(%d) %s\n", seconds, message);
				exit(EXIT_SUCCESS);
			} else {
				/*
				 * In the parent, call waitpid() to collect children
				 * that have already terminated
				 */
				do {
					pid = waitpid((pid_t) -1, NULL, WNOHANG);
					if (pid == (pid_t) -1)
						fprintf(stderr, "Wait for child\n");
				} while (pid != (pid_t) 0);
			}
		}
	}

	exit(EXIT_SUCCESS);
}