#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <assert.h>

int
sendfd(int s, int fd)
{
	char buf[1];
	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	int n;
	char cms[CMSG_SPACE(sizeof(int))];
	
	buf[0] = 0;
	iov.iov_base = buf;
	iov.iov_len = 1;

	memset(&msg, 0, sizeof msg);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = cms;
	msg.msg_controllen = CMSG_LEN(sizeof(int));

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	memcpy(CMSG_DATA(cmsg), &fd, sizeof(int));

	n = sendmsg(s, &msg, 0);
	if (n != iov.iov_len) {
		printf("Error : %d\n", errno);
		return -1;
	}
	return 0;
}

int
recvfd(int s)
{
	int n;
	int fd;
	char buf[1];
	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	char cms[CMSG_SPACE(sizeof(int))];

	iov.iov_base = buf;
	iov.iov_len = 1;

	memset(&msg, 0, sizeof msg);
	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	msg.msg_control = cms;
	msg.msg_controllen = sizeof(cms);

	n = recvmsg(s, &msg, 0);
	if (n <= 0) {
		return -1;
	}
	cmsg = CMSG_FIRSTHDR(&msg);
	memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
	assert (!(msg.msg_flags & MSG_TRUNC));
	assert (!(msg.msg_flags & MSG_CTRUNC));
	return fd;
}

int
xpipe(int fd[2]) {
	return socketpair(AF_UNIX, SOCK_SEQPACKET, 0, fd);
}

int main(int argc, char *argv[]) {
	int pfd[2];
	unsigned long val;

	int rc;
	int latch = eventfd(0, 0);

	xpipe(pfd);

	rc = fork();
	switch (rc) {
	case 0: {
		int rfd;
		close(pfd[0]);
		//read(latch, &val, sizeof(val));
		sleep (60);
		printf("     ===> Take\n");
		rfd = recvfd(pfd[1]);
		printf("     <=== Take\n");
		assert(rfd != -1);
		rc = write(rfd, &val, sizeof(val));
		assert(rc == sizeof(val));
		break;
	}
	default: {
		close(pfd[1]);
		for (int i = 0; i < N; i++) {
			int a = eventfd(0, 0);
			assert(a != -1);
			printf("%d ==>\n", i);
			rc = sendfd(pfd[0], a);
			assert(rc != -1);
			printf("%d <==\n", i);
			close(a);
		}
		write(latch, &val, sizeof(val));
		wait(NULL);
	}
	}
}

