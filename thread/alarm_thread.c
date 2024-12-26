#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

typedef struct alarm_tag {
	int seconds;
	char message[64];
} alarm_t;

void *alarm_thread(void *arg)
{
	alarm_t *alarm;
	int status;

	alarm = (alarm_t *) arg;
	status = pthread_detach(pthread_self());
	if (status != 0) {
		fprintf(stderr, "pthread_detach error\n");
		exit(EXIT_FAILURE);
	}

	sleep(alarm->seconds);
	printf("(%d) %s\n", alarm->seconds, alarm->message);

	free(alarm);
	return NULL;
}

int main()
{
	int status;
	char line[128];
	alarm_t *alarm;
	pthread_t thread;

	while (1) {
		printf("Alarm> ");
		if (fgets(line, sizeof(line), stdin) == NULL) exit(EXIT_FAILURE);
		if (strlen(line) <= 1) continue;
		alarm = (alarm_t *) malloc(sizeof(alarm_t));
		if (alarm == NULL) {
			fprintf(stderr, "Allocate for alarm\n");
			exit(EXIT_FAILURE);
		}

		/*
		 * Parse input line into seconds (%d) and a message
		 * (%64[^\n]), consisting of up to 64 characters
		 * seperated from the seconds by whitespace.
		 */
		if (sscanf(line, "%d %64[^\n]", &alarm->seconds, alarm->message) < 2) {
			fprintf(stderr, "Bad command\n");
			free(alarm);
		} else {
			status = pthread_create(&thread, NULL, alarm_thread, alarm);
			if (status != 0) {
				fprintf(stderr, "pthread_create error\n");
				free(alarm);
				exit(EXIT_FAILURE);
			}
		}
	}

	exit(EXIT_SUCCESS);
}