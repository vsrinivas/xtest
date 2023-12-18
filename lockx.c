#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

/* lockf <file> [command...] */
int main(int argc, char *argv[]) {
	int fd;
	int i;
	int len;
	char *cmd;
	char *p;

	if (argc < 3)
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

	/* run */
	len = 0;
	for (i = 2; i < argc; i++) {
		len += strlen(argv[i]) + 1;
	}
	cmd = malloc(len);
	p = cmd;
	for (i = 2; i < argc; i++) {
		p += sprintf(p, "%s ", argv[i]);
	}
	system(cmd);
	free(cmd);

	/* lock-release */
	unlink(argv[1]);
	return 0;
}
