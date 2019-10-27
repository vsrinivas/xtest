#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>


#ifndef MIN
#define MIN(x,y)	(((x)<(y))? (x):(y))
#endif
#ifndef MAX
#define MAX(x,y)	(((x)>(y))? (x):(y))
#endif

int sum(uint64_t *buf, int len) {
	int i;
	for (i = 0; i < len/8; i++) {
		if (buf[i] != 0)
			return 1;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	int fd;
	uint64_t n;
	unsigned long long blocks;
	char *chkbuf;
	int rc;
	uint64_t start, end;
	struct stat sb;

	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		return 1;

	rc = ioctl(fd, BLKGETSIZE64, &blocks);
	if (rc == -1) {
		fstat(fd, &sb);
		blocks = sb.st_size;
	}
	blocks = blocks / 512;

	start = end = -1;
	chkbuf = calloc(1, 512);
	for (n = 0; n < blocks; n++) {
		rc = pread(fd, chkbuf, 512, n * 512);
		if (rc != 512)
			return 2;

		rc = sum(chkbuf, 512);
		if (rc == 0) {
			// if we're starting a run:
			if (start == -1) {
				start = end = n;
			} else {
				end = n;
			}
		} else {
			if (start != -1) {
				printf("[%lx, %lx)\n", start, end);
				start = end = -1;
			}
		}
	//	if (rc == 0)
		//		printf("block: %lx\n", n);
	}
	free(chkbuf);
	printf("Ok.\n");
	return 0;
}
