#include <assert.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <czmq.h>

#include "ancillary.h"
#include "czev.h"

char *name = "Xeventfd_socket";
uint64_t u = 4711;

int timer_fn (zloop_t *loop, int timer_id, void *arg)
{
    int efd = (int) arg;
    ssize_t s = write(efd, &u, sizeof(uint64_t));
    if (s != sizeof(uint64_t))
	perror("write");
    else
	printf("Sent %llu \n", u);
    u++;
    return 0;
}

static int
s_socket_event (zloop_t *loop, zmq_pollitem_t *poller, void *arg)
{
    struct sockaddr_un address;
    socklen_t address_length  = sizeof(address);

    printf("listen socket %d connected\n", poller->fd);

    int cfd = accept(poller->fd,
		     (struct sockaddr *) &address,
		     &address_length);
    printf("accepted socket %d\n", cfd);

    if (cfd > -1) {
	struct zanvil_request zrq;
	int n = read(cfd, &zrq, sizeof(zrq));
	if (n != sizeof(zrq)) {
	    close(cfd);
	    return 0;
	}
	printf("request.request_name='%s' request.param=%d\n",
	       zrq.request_name, zrq.param);

	int efd  = eventfd(0,0);
	assert (efd != -1);

	if (ancil_send_fd(cfd, efd)) {
	    perror("ancil_send_fd");
	    exit(1);
	} else {
	    printf("Sent evfd %d\n", efd);
	    // this leaks eventfd's - never closed
	    zloop_timer (loop, 500, zrq.param, timer_fn, (void *)efd);
	}
	close(cfd);
    }
    return 0;
}

int main(void)
{
    struct sockaddr_un address;
    int socket_fd;

    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if((socket_fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
	printf("socket() failed\n");
	exit(1);
    }

    int enable = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path,sizeof(address.sun_path), name);
    address.sun_path[0] = '\0';

    if (bind(socket_fd, (struct sockaddr *) &address,
	    sizeof(struct sockaddr_un)) != 0) {
	fprintf(stderr,"bind() failed: %s\n", strerror(errno));
	return 1;
    }

    if (listen(socket_fd, 5) != 0) {
	fprintf(stderr,"listen() failed: %s\n", strerror(errno));
	return 1;
    }

    zloop_t *self = zloop_new();

    zmq_pollitem_t poll_input = { 0, socket_fd, ZMQ_POLLIN };
    int rc = zloop_poller (self, &poll_input, s_socket_event, NULL);
    assert (rc == 0);
    zloop_start (self);

    fprintf(stderr,"accept() failed: %s\n", strerror(errno));
    close(socket_fd);
    return 0;
}
