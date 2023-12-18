#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

/* acquirex <file> */
int main(int argc, char *argv[]) {
	int fd;
	int i;
	int len;
	char *cmd;
	char *p;

	if (argc < 2)
		exit(-1);

	/* lock-acquire */
	for (i = 0;; i++) {
		fd = open(argv[1], O_CREAT|O_EXCL, 0700);
		if (fd != -1) {
			close(fd);
			break;
		}
		if (fd == -1 && errno == EEXIST) {
			sleep(i % 7);
			continue;
		} else if (fd == -1) {
			exit(-1);
		}
	}
	return 0;
}
