#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>

static char buf_a[1048576];
static char buf_b[1048576];

int main(int argc, char *argv[]) {
	int a, b;
	off_t i;
	int rc_a, rc_b;
	struct stat s_a, s_b;
	int j;

	a = open(argv[1], O_RDONLY);
	b = open(argv[2], O_RDONLY);
	if (a == -1 || b == -1)
		return -1;

	fstat(a, &s_a);
	fstat(b, &s_b);
//	if (s_a.st_size != s_b.st_size)
//		return -2;

	for (i = 0; i < s_a.st_size;) {
		bzero(buf_a, sizeof(buf_a));
		bzero(buf_b, sizeof(buf_b));
		rc_a = pread(a, buf_a, sizeof(buf_a), i);
		rc_b = pread(b, buf_b, sizeof(buf_b), i);
		if (rc_a != rc_b)
			return -3;
		if (rc_a < 0 || rc_b < 0)
			return -4;

		if (rc_a == 0)
			break;
		for (j = 0; j < rc_a; j++) {
			if (buf_a[j] != buf_b[j])
				printf("%lx %x %x\n", i+j, buf_a[j], buf_b[j]);	
		}
		i += rc_a;
	}

	return 0;
}
