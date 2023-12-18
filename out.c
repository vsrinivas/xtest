#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

// stream out the contents of file argv[1] from offset=argv[2] to argv[3], noninclusive
// works in reasonable-sized chunks.
// 	out /etc/passwd 1 4 -> 'oot'
main(int argc, char *argv[]) {
	char buf[131072];
	int fd;
	int rc;
	off_t i, end;
	int x;

	if (argc != 4)
		exit(-1); 
	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		exit(-1);

	i = atol(argv[2]);
	x = 131072;
	end = atol(argv[3]);
	for (;;) {
		if (i + 131072ul > end) {
			x = end - i;
		}
		rc = pread(fd, buf, x, i);
		if (rc == 0)
			break;
		if (rc == -1)
			break;
		write(1, buf, rc);
		i += rc;
	}
	return 0;
}
