#include "md5.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	char *sum;

	sum = md5sum(argv[1]);
	printf("%s %s\n", sum, argv[1]);
	free(sum);

	int fd = open(argv[1], O_RDONLY);
	char *buf;
	struct stat sb;
	fstat(fd, &sb);
	buf = malloc(sb.st_size);
	read(fd, buf, sb.st_size);

	sum = md5sumbuf(buf, sb.st_size);
	printf("%s %s\n", sum, argv[1]);
	free(sum);
	free(buf);
	
	return 0;
}
