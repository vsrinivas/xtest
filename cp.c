#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <string.h>
#include "md5.h"

int
copy_file(char *src, char *dst)
{
	int from_fd;
	int to_fd;
	char *p;
	struct stat src_sb, dst_sb;
	int rc;
	size_t it;
	size_t bytes;
	char *src_md5;
	char *dst_md5;

	from_fd = open(src, O_RDONLY);
	if (from_fd == -1) {
		printf("Err open: %s\n", src);
		return (-1);
	}

	to_fd = open(dst, O_RDWR | O_CREAT | O_EXCL, 0600);
	if (to_fd == -1) {
		printf("Err creat %s for %s\n", dst, src);
		close (from_fd);
		return (-1);
	}

	rc = fstat(from_fd, &src_sb);
	if (rc == -1) {
		printf("Err stat %s\n", src);
		return (-1);
	}
	rc = fstat(to_fd, &dst_sb);
	if (rc == -1) {
		printf("Err stat %s\n", dst);
		return (-1);
	}
	if (!(src_sb.st_mode & S_IFREG)) {
		printf("Err stat S_IFREG %s\n", src);
		return (-1);
	}

	p = mmap(0, src_sb.st_size, PROT_READ, MAP_FILE | MAP_SHARED, from_fd, 0);
	if (p == MAP_FAILED) {
		printf("mmap failed %s\n", src);
		return (-1);
	}

	src_md5 = md5sumbuf(p, src_sb.st_size);

	#define MAXBUFSZ (64 * 1024)
	bytes = src_sb.st_size;
	for (it = 0; it < src_sb.st_size;) {
		rc = pwrite(to_fd, &p[it], MIN(bytes, MAXBUFSZ), it);
		if (rc == -1 || rc == 0)
			break;

		bytes -= rc;
		it += rc;
	}
	if (bytes != 0) {
		printf("copy failed %s\n", src);
		return (-1);
	}
	munmap(p, src_sb.st_size);
	close(from_fd);
	fsync(to_fd);
	close(to_fd);

	dst_md5 = md5sum(dst);
	if (strcmp(src_md5, dst_md5) != 0) {
		printf("Copy hash err: got %s expected %s\n", dst_md5, src_md5);
		free(src_md5);
		free(dst_md5);
		return (-1);
	}

	free(src_md5);
	free(dst_md5);
	return (0);

}

#ifdef TEST
main(argc, argv)
	char *argv[];
{
	copy_file(argv[1], argv[2]);
}
#endif
