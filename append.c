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
	off_t off_out;
	ssize_t rc;

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
	ftruncate(dst, newsz);
	off_out = xdst.st_size;
	rc = copy_file_range(src, NULL, dst, &off_out, xsrc.st_size, 0);
	if (rc < xsrc.st_size) {
		printf("error %ld\n", rc);
	}
	close(dst);
	close(src);
}
