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

	rc_b = 0;
	a = open(argv[1], O_RDONLY);
	fstat(a, &s_a);
	for (i = 0; i < s_a.st_size;) {
		bzero(buf_a, sizeof(buf_a));
		rc_a = pread(a, buf_a, sizeof(buf_a), i);
		if (rc_a < 0)
			return -4;

		for (j = 0; j < rc_a; j++) {
			if (buf_a[i + j] != 0) {
				printf("%lx\n", i+j);
				return 0;
			}	
		}
		i += rc_a;
		if (rc_a == 0)
			break;
	}

	return 0;
}
