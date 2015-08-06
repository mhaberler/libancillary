#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <czmq.h>

#include "ancillary.h"
#include "czev.h"


char *name = "Xeventfd_socket";

static int
eventfd_readable (zloop_t *loop, zmq_pollitem_t *poller, void *arg)
{
    uint64_t u;
    ssize_t s;
    if (poller->revents & POLLIN) {
	s = read(poller->fd, &u, sizeof(uint64_t));
	if (s != sizeof(uint64_t))
	    perror("read");
	else
	    printf("read %llu (0x%llx) from fd %d\n",
		   (unsigned long long) u, (unsigned long long) u, poller->fd);
    }
    if (poller->revents & POLLERR) {
	printf("POLLERR \n");
    }
    return 0;
}


int main(int argc, char **argv)
{
    struct sockaddr_un address;
    int  socket_fd;
    int evfd;
    int count = 3;
    if (argc > 1) count = atoi(argv[1]);

    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0)	{
	fprintf(stderr,"socket() failed: %s\n", strerror(errno));
	return 1;
    }

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path,sizeof(address.sun_path), name);
    address.sun_path[0] = '\0';

    if (connect(socket_fd, (struct sockaddr *) &address, 
		sizeof(struct sockaddr_un)) != 0) {
	fprintf(stderr,"connect() failed: %s\n", strerror(errno));
	return 1;
    }
    struct zanvil_request zrq = {
	.request_name = "nevents",
	.param = count,
    };
    write(socket_fd, &zrq, sizeof(zrq));

    if (ancil_recv_fd(socket_fd, &evfd)) {
	perror("ancil_recv_fd");
	exit(1);
    } else {
	printf("Received eventfd %d on %d\n", evfd, socket_fd);
    }

    if (evfd < 0) {
	printf("bad event fd\n");
	exit(1);
    }

    zloop_t *self = zloop_new();

    zmq_pollitem_t poll_input = { 0, evfd, ZMQ_POLLIN };
    int rc = zloop_poller (self, &poll_input, eventfd_readable, NULL);
    assert (rc == 0);
    zloop_start (self);

    close(socket_fd);

    return 0;
}
