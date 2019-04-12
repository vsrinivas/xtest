#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

/*
 *  argv[1] -> src directory
 *  argv[2] -> dst directory
 *
 * stdin: list of files (relative paths) to copy from src->dst.
 */
int main(int argc, char *argv[]) {
	char buf[1024];
	char cmdbuf[2048];
	char *p, *q;
	int n = 0;
	int rc;

	if (argc < 2)
		return -1;

	while (!feof(stdin)) {
		p = buf;
		bzero(buf, sizeof(buf));
		p += sprintf(buf, "%s/", argv[1]);
		q = fgets(p, sizeof(buf) - strlen(p) - 1, stdin);
		if (!q) {
			break;
		}
		q=rindex(p, '\n');
		if (q) {
			*q = '\0';
		}

		rc = access(buf, R_OK);
		if (rc != 0) {
			fprintf(stderr, "access: %s %d\n", buf, errno);
		}
		assert(rc == 0);

		sprintf(cmdbuf, "cp --parents \"%s/%s\" \"%s\"", argv[1], p, argv[2]);
		printf(cmdbuf);
		system(cmdbuf);
		printf("\n");
		n++;
	}
	fprintf(stderr, "nfiles: %d\n", n);
}
