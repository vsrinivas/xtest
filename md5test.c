#include "md5.h"

int main(int argc, char *argv[]) {
	char *sum;

	sum = md5sum(argv[1]);
	printf("%s %s\n", sum, argv[1]);
	free(sum);
	return 0;
}
