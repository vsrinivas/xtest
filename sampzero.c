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
	int N;
	int M = 1048576;
	int fd;
	int n;
	unsigned long long blocks;
	char *chkbuf;
	unsigned long long tblk;
	unsigned long long ntblk;
	int rc;

	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		return 1;

	ioctl(fd, BLKGETSIZE64, &blocks);
	blocks = blocks / 512;
	N = 5000;

	srand(11);
	for (n = 0; n < MIN(N, 128); n++) {
		ntblk = random() % blocks;
		readahead(fd, ntblk * 512, M);
	}

	chkbuf = calloc(1, M);
	srand(11);
	tblk = random() % blocks;
	for (n = 0; n < N; n++) {
		rc = pread(fd, chkbuf, M, tblk * 512);
		if (rc != M)
			return 2;

		ntblk = random() % blocks;
		readahead(fd, ntblk * 512, M);
		if (sum((uint64_t*)chkbuf, M) != 0) {
			printf("%llu\n", tblk);
			return 3;
		}
		tblk = ntblk;
	}
	free(chkbuf);
	printf("Ok.\n");
	return 0;
}
