#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include "ancillary.h"

void child_process(int sock)
{
    int fd;
    int fds[3];
    char b[] = "This is on the received fd!\n";

    if(ancil_recv_fd(sock, &fd)) {
	perror("ancil_recv_fd");
	exit(1);
    } else {
	printf("Received fd: %d\n", fd);
    }
    write(fd, b, sizeof(b));
    close(fd);
    sleep(2);

    if(ancil_recv_fds(sock, fds, 3)) {
	perror("ancil_recv_fds");
	exit(1);
    } else {
	printf("Received one fds: %d %d %d.\n", fds[0], fds[1], fds[2]);
    }
}

void parent_process(int sock)
{
    int fds[2] = { 1, 2 };

    if(ancil_send_fd(sock, 1)) {
	perror("ancil_send_fd");
	exit(1);
    } else {
	printf("Sent fd.\n");
    }
    sleep(1);

    if(ancil_send_fds(sock, fds, 2)) {
	perror("ancil_send_fds");
	exit(1);
    } else {
	printf("Sent two fds.\n");
    }
}

int main(void)
{
    int sock[2];

    if(socketpair(PF_UNIX, SOCK_STREAM, 0, sock)) {
	perror("socketpair");
	exit(1);
    } else {
	printf("Established socket pair: (%d, %d)\n", sock[0], sock[1]);
    }

    switch(fork()) {
	case 0:
	    close(sock[0]);
	    child_process(sock[1]);
	    break;
	case -1:
	    perror("fork");
	    exit(1);
	default:
	    close(sock[1]);
	    parent_process(sock[0]);
	    wait(NULL);
	    break;
    }
    return(0);
}
