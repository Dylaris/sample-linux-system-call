#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* 
 * It is a server program.
 * Server has one fifo receiving the request
 * from all clients and there is one fifo for
 * each client to response.
 */

#define SERVER_NAME "server-fifo"
#define BUF_SIZE 128

int main()
{
    mode_t sv_perm;         /* server fifo's permission */
    int sv_r_fd, sv_w_fd;   /* server read/write fd */
    char buf[BUF_SIZE + 1]; /* store the request/response */
    char clname[64];        /* store the client fifo name */
    pid_t cl_pid;           /* client process's pid */
    ssize_t rbytes, wbytes; /* read/write bytes */
    int msglen;             /* message length */

    umask(0);
    sv_perm = 0666;         /* rw-rw-rw- */
    if (access(SERVER_NAME, F_OK) != 0 && mkfifo(SERVER_NAME, sv_perm) < 0) {
        perror("server mkfifo");
        return -1;
    }

    /* open the read end of server fifo */
    sv_r_fd = open(SERVER_NAME, O_RDONLY);
    if (sv_r_fd < 0) {
        perror("server open for reading");
        return -1;
    }

    while (1) {
        /* get the pid of client process */
        while ((rbytes = read(sv_r_fd, &cl_pid, sizeof(pid_t))) == 0);  /* wait for request */
        if (rbytes < 0) {
            perror("server read for cl_pid");
            return -1;
        }

        /* get the length of message length */
        rbytes = read(sv_r_fd, &msglen, sizeof(int));
        if (rbytes < 0) {
            perror("server read for msglen");
            return -1;
        }

        /* check if the msglen is valid */
        if (msglen < 0 || msglen > BUF_SIZE) {
            fprintf(stderr, "message length: %d is invalid\n", msglen);
            return -1;
        }

        /* read the message content */
        rbytes = read(sv_r_fd, buf, msglen);
        if (rbytes != msglen) {
            perror("server read for buf");
            return -1;
        }
        buf[msglen] = '\0';

        /* acknowledge message */
        printf("Hi, process %d. I received your request: %s\n", cl_pid, buf);

        /* open the write end of client fifo */
        snprintf(clname, sizeof(clname), "client.%d-fifo", cl_pid);
        sv_w_fd = open(clname, O_WRONLY);
        if (sv_w_fd < 0) {
            perror("server open for writing");
            return -1;
        }

        /* response to client */
        snprintf(buf, sizeof(buf), "Nice to meet you: %s", clname);
        wbytes = write(sv_w_fd, buf, strlen(buf));
        if (wbytes < 0 || (size_t) wbytes != strlen(buf)) {
            perror("server write for buf");
            return -1;
        }

        /* end the response */
        close(sv_w_fd);
    }

    /* end the service */
    close(sv_r_fd);

    return 0;
}
