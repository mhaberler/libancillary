#include <sys/types.h>
#include <stdlib.h>
#include <sys/uio.h>
#include "ancillary.h"
#include <sys/socket.h>

static struct cmsghdr *ancil_init_msghdr(struct msghdr *msghdr,
    struct iovec *nothing_ptr, char *nothing, void *buffer, int n)
{
    struct cmsghdr *cmsg;

    nothing_ptr->iov_base = &nothing;
    nothing_ptr->iov_len = 1;
    msghdr->msg_name = NULL;
    msghdr->msg_namelen = 0;
    msghdr->msg_iov = nothing_ptr;
    msghdr->msg_iovlen = 1;
    msghdr->msg_flags = 0;
    msghdr->msg_control = buffer;
    msghdr->msg_controllen = sizeof(struct cmsghdr) + sizeof(int) * n;
    cmsg = CMSG_FIRSTHDR(msghdr);
    cmsg->cmsg_len = msghdr->msg_controllen;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    return(cmsg);
}

int ancil_send_fds_with_buffer(int sock, int n_fds, const int *fds, void *buffer)
{
    struct msghdr msghdr;
    char nothing = '!';
    struct iovec nothing_ptr;
    struct cmsghdr *cmsg;
    int i;

    cmsg = ancil_init_msghdr(&msghdr, &nothing_ptr, &nothing, buffer, n_fds);
    for(i = 0; i < n_fds; i++)
	((int *)CMSG_DATA(cmsg))[i] = fds[i];
    return(sendmsg(sock, &msghdr, 0) >= 0 ? 0 : -1);
}

int ancil_recv_fds_with_buffer(int sock, int n_fds, int *fds, void *buffer)
{
    struct msghdr msghdr;
    char nothing;
    struct iovec nothing_ptr;
    struct cmsghdr *cmsg;
    int i;

    cmsg = ancil_init_msghdr(&msghdr, &nothing_ptr, &nothing, buffer, n_fds);
    for(i = 0; i < n_fds; i++)
	((int *)CMSG_DATA(cmsg))[i] = -1;
    
    if(recvmsg(sock, &msghdr, 0) < 0)
	return(-1);
    for(i = 0; i < n_fds; i++)
	fds[i] = ((int *)CMSG_DATA(cmsg))[i];
    return(0);
}

int ancil_send_fd(int sock, int fd)
{
    ANCIL_FD_BUFFER(1) buffer;

    return(ancil_send_fds_with_buffer(sock, 1, &fd, &buffer));
}

int ancil_recv_fd(int sock, int *fd)
{
    ANCIL_FD_BUFFER(1) buffer;

    return(ancil_recv_fds_with_buffer(sock, 1, fd, &buffer));
}

