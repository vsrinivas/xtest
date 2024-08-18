#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	struct stat sb_a, sb_b;
	int fd_a, fd_b;
	unsigned long i;
	char *a, *b;

	fd_a = open(argv[1], O_RDONLY);
	fd_b = open(argv[2], O_RDONLY);
	fstat(fd_a, &sb_a);
	fstat(fd_b, &sb_b);
	assert(fd_a != -1);
	assert(fd_b != -1);
	assert(sb_a.st_size == sb_b.st_size);

	a = mmap(NULL, sb_a.st_size, PROT_READ, MAP_SHARED, fd_a, 0);
	b = mmap(NULL, sb_b.st_size, PROT_READ, MAP_SHARED, fd_b, 0);
	assert (a != MAP_FAILED);
	assert (b !=  MAP_FAILED);

	for (i = 0; i < sb_a.st_size; i++) {
		if (a[i] != b[i]) {
			printf("off=%lx a=%02x b=%02x\n", i, a[i], b[i]);
		}
	}
	assert(i == sb_a.st_size);

}
