#ifndef ANCILLARY_H__
#define ANCILLARY_H__

/***************************************************************************
 * Start of the readable part.
 ***************************************************************************/

#define ANCIL_MAX_N_FDS 960
/*
 * Maximum number of fds that can be sent or received using the "esay"
 * functions; this is so that all can fit in one page.
 */

extern int ancil_send_fds_with_buffer(int sock, const int *fds, unsigned n_fds, void *buffer);
/*
 * ancil_send_fds_with_buffer(sock, n_fds, fds, buffer)
 *
 * Sends the file descriptors in the array pointed by fds, of length n_fds
 * on the socket sock.
 * buffer is a writeable memory area large enough to hold the required data
 * structures.
 * Returns: -1 and errno in case of error, 0 in case of success.
 */

extern int ancil_recv_fds_with_buffer(int sock, int *fds, unsigned n_fds, void *buffer);
/*
 * ancil_recv_fds_with_buffer(sock, n_fds, fds, buffer)
 *
 * Receives n_fds file descriptors into the array pointed by fds
 * from the socket sock.
 * buffer is a writeable memory area large enough to hold the required data
 * structures.
 * Returns: -1 and errno in case of error, 0 in case of success.
 */

extern int ancil_send_fds(int sock, const int *fds, unsigned n_fds);
extern int ancil_recv_fds(int sock, int *fd, unsigned n_fds);

extern int ancil_send_fd(int sock, int fd);
/* ancil_recv_fd(sock, fd);
 *
 * Sends the file descriptor fd on the socket sock.
 * Returns : -1 and errno in case of error, 0 in case of success.
 */

extern int ancil_recv_fd(int sock, int *fd);
/* ancil_send_fd(sock, &fd);
 *
 * Receives the file descriptor fd from the socket sock.
 * Returns : -1 and errno in case of error, 0 in case of success.
 */

#define ANCIL_FD_BUFFER(n) \
    struct { \
	struct cmsghdr h; \
	int fd[n]; \
    }
/* ANCIL_FD_BUFFER(n)
 *
 * A structure type suitable to be used as buffer for n file descriptors.
 * Requires <sys/socket.h>.
 * Example:
 * ANCIL_FD_BUFFER(42) buffer;
 * ancil_recv_fds_with_buffer(sock, 42, my_fds, &buffer);
 */

#endif /* ANCILLARY_H__ */
