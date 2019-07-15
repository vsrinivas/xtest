#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/nbd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int fd;

	fd = open(argv[1], O_RDWR);
	int sv[2];
	sv[0] = socket(AF_UNIX, SOCK_STREAM, 0);
	sv[1] = socket(AF_UNIX, SOCK_STREAM, 0);

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	addr.sun_path[0] = '\0';
	addr.sun_path[1] = '9';
	bind(sv[1], (struct sockaddr*)&addr, sizeof(addr));
	listen(sv[1], 5);

	int rc = fork();
	switch (rc) {
	case 0: {
		char *b = calloc(1, 1048576);
		int u = accept(sv[1], NULL, NULL);
		for (;;) {
		struct nbd_request r = {};
		struct nbd_reply rp = {};
		uint32_t l;
		uint32_t x ;
		x = read(u, &r, sizeof(r));
		if (x != sizeof(r)) continue;

		l = ntohl(r.len);
		//printf("I/O magic %lx type %x offset %lx size %x\n", r.magic, r.type, r.from, l);
		if (l == 0) continue;

		memcpy(rp.handle, r.handle, 8);
		rp.magic = htonl(0x67446698);
		rp.error = htonl(0);
		struct iovec iv[2];
		iv[0].iov_base = &rp;
		iv[0].iov_len = sizeof(rp);
		iv[1].iov_base = b;
		iv[1].iov_len = l;
		//write(u, &rp, sizeof(rp));
		//write(u, b, l);
		writev(u, iv, 2);
		}
		free(b);
		return 0;
	}
	}

	connect(sv[0], (struct sockaddr*)&addr, sizeof(addr));
	sleep(5);
	ioctl(fd, NBD_SET_SOCK, sv[0]);
	ioctl(fd, NBD_SET_BLKSIZE, (unsigned long) 512);
	ioctl(fd, NBD_SET_SIZE_BLOCKS, (unsigned long) 10000200400ul);
	ioctl(fd, NBD_SET_TIMEOUT, (unsigned long) 60);
	ioctl(fd, NBD_SET_FLAGS, NBD_FLAG_READ_ONLY);
	ioctl(fd, NBD_DO_IT);
}
