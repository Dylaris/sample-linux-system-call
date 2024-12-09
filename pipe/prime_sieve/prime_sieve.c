/*
 * Sieve of Eratosthenes
 * 
 * Fork a new process when encountering a prime.
 * The new process read data from its left process (parent)
 * and write data to its right process (child) after sieving.
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NUMBER  50
#define READ_END    0
#define WRITE_END   1

int sieve(int read_end);

int main()
{
	int pfd[2];     /* pipe file description */
	pid_t pid;

	/* create the pipe */
	if (pipe(pfd) < 0) {
			perror("pipe");
			return -1;
	}

	pid = fork();
	if (pid < 0) {  /* fork failed */
		perror("fork");
		return -1;
	} else if (pid == 0) {  /* child process */
		close(pfd[WRITE_END]);
		if (sieve(pfd[READ_END]) < 0)
			exit(EXIT_FAILURE);
		close(pfd[READ_END]);
		exit(EXIT_SUCCESS);
	} else {    /* parent process */
		close(pfd[READ_END]);
		for (int i = 2; i <= MAX_NUMBER; i++) {
			if (write(pfd[WRITE_END], &i, sizeof(i)) != sizeof(i)) {
				perror("write");
				return -1;
			}
		}
		close(pfd[WRITE_END]);

		wait(NULL);
	}

	return 0;
}

int sieve(int read_end)
{
	int pfd[2];     
	pid_t pid;
	int prime, number;

	if (pipe(pfd) < 0) {
		perror("pipe");
		close(read_end);
		return -1;
	}

	if (read(read_end, &prime, sizeof(prime)) > 0) {
		printf("Prime: %d\n", prime);

		pid = fork();
		if (pid < 0) {
			perror("fork");
			close(read_end);
			close(pfd[READ_END]);
			close(pfd[WRITE_END]);
			return -1;
		} else if (pid == 0) {
			close(read_end);
			close(pfd[WRITE_END]);
			if (sieve(pfd[READ_END]) < 0)
				exit(EXIT_FAILURE);
			close(pfd[READ_END]);
			exit(EXIT_SUCCESS);
		} else {
			close(pfd[READ_END]);
			while (read(read_end, &number, sizeof(number)) > 0) {
				if (number % prime == 0) continue;

				if (write(pfd[WRITE_END], &number, sizeof(number)) != sizeof(number)) {
					perror("write");
					return -1;
				}
			}
			close(read_end);
			close(pfd[WRITE_END]);
			wait(NULL);
		}

		return 0;
	}

	return -1;
}
