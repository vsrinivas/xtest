       #define _GNU_SOURCE
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include  <aio.h>

main(argc, argv)
	int argc;
	char *argv[];
{
	/* append [dest] [src] */
	int src, dst;
	struct stat xsrc, xdst;
	off_t newsz;
	off_t off_out, off_in;
	ssize_t rc, req;
	char buf[262144];

	if (argc != 3)
		exit(-1);

	dst = open(argv[1], O_RDWR);
	src = open(argv[2], O_RDONLY);
	if (dst == -1 || src == -1)
		exit(-1);
	fstat(dst, &xdst);
	fstat(src, &xsrc);

	newsz = xdst.st_size + xsrc.st_size;
	if (newsz < xdst.st_size)
		exit(-1);
	rc = ftruncate(dst, newsz);
	if (rc) {
		printf("err %d\n", rc);
		return -1;
	}
	off_in = 0;
	off_out = xdst.st_size;
	for (;;) {
		bzero(buf, sizeof(buf));
		rc = pread(src, buf, sizeof(buf), off_in);
		if (rc == 0)
			break;
		if (rc < 0) {
			printf("err %d\n", rc);
			return -1;
		}
		off_in += rc;
		req = rc;
		rc = pwrite(dst, buf, rc, off_out + off_in);
		if (rc != req) {
			printf("err_out %d\n", rc);
			return -1;
		}	
	}
	close(dst);
	close(src);
}
