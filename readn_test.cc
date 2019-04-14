#include "md5.h"
#include "readn.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
	char *sum;

	sum = md5sum(argv[1]);
	printf("%s %s\n", sum, argv[1]);
	free(sum);

	struct stat sb;
	stat(argv[1], &sb);
	ScopedFile s(strdup(argv[1]), sb);
	s.Read();
	sum = md5sumbuf(s.data(), s.size());
	printf("%s %s\n", sum, argv[1]);
	free(sum);
	
	return 0;
}
