#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
	int seconds;
	char line[128], message[64];

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
			sleep(seconds);
			printf("(%d) %s\n", seconds, message);
		}
	}

	exit(EXIT_SUCCESS);
}