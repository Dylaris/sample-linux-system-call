#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* 
 * It is a client program.
 * Client has its own fifo to receive response
 * from server and send request to server throught
 * server fifo 
 */

#define SERVER_NAME "server-fifo"
#define BUF_SIZE 128

int main()
{
    mode_t cl_perm;         /* client fifo's permission */
    int cl_r_fd, cl_w_fd;   /* client read/write fd */
    char buf[BUF_SIZE + 1]; /* store the request/response */
    char clname[64];        /* store the client fifo name */
    pid_t cl_pid;           /* client process's pid */
    ssize_t rbytes, wbytes; /* read/write bytes */
    int msglen;             /* message length */

    /* check if the server is exist */
    while (access(SERVER_NAME, F_OK) != 0);

    umask(0);
    cl_perm = 0666;         /* rw-rw-rw- */
    cl_pid = getpid();
    snprintf(clname, sizeof(clname), "client.%d-fifo", cl_pid);
    if (mkfifo(clname, cl_perm) < 0) {
        perror("client mkfifo");
        return -1;
    }

    /* open the write end of server fifo */
    cl_w_fd = open(SERVER_NAME, O_WRONLY);
    if (cl_w_fd < 0) {
        perror("client open for writing");
        return -1;
    }

    /* request to server */
    snprintf(buf, sizeof(buf), "request from client process %d", cl_pid);
    msglen = strlen(buf);

    wbytes = write(cl_w_fd, &cl_pid, sizeof(pid_t));
    if (wbytes < 0 || (size_t) wbytes != sizeof(pid_t)) {
        perror("client write for pid");
        return -1;
    }
    wbytes = write(cl_w_fd, &msglen, sizeof(int));
    if (wbytes < 0 || (size_t) wbytes != sizeof(int)) {
        perror("client write for msglen");
        return -1;
    }
    wbytes = write(cl_w_fd, buf, msglen);
    if (wbytes < 0 || (int) wbytes != msglen) {
        perror("client write for buf");
        return -1;
    }

    /* end the request */
    close(cl_w_fd);

    /* open the read end of server fifo */
    cl_r_fd = open(clname, O_RDONLY);
    if (cl_r_fd < 0) {
        perror("client open for reading");
        return -1;
    }

    /* read the response */
    rbytes = read(cl_r_fd, buf, BUF_SIZE);
    if (rbytes < 0) {
        perror("client read");
        return -1;
    }
    buf[rbytes] = '\0';

    /* acknowledge message */
    printf("Thank you! I [pid=%d] received your response: %s\n", cl_pid, buf);

    /* end the close */
    close(cl_r_fd);

    /* delete the client fifo file */
    unlink(clname);

    return 0;
}
