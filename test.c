#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "ancillary.h"
#include <sys/socket.h>

void child_process(int sock)
{
    int fd;
    char b[] = "This is on the received fd!\n";

    if(ancil_recv_fd(sock, &fd)) {
	perror("ancil_recv_fd");
	exit(1);
    } else {
	printf("Received fd: %d\n", fd);
    }
    write(fd, b, sizeof(b));
}

void parent_process(int sock)
{
    if(ancil_send_fd(sock, 1)) {
	perror("ancil_send_fd");
	exit(1);
    } else {
	printf("Sent fd.\n");
    }
    sleep(1);
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
	    break;
    }
    return(0);
}
