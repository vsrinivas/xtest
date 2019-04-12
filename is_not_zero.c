#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char *argv[]) {
	int fd;
	struct stat sb;
	char *ptr;
	unsigned long i;
	unsigned long *lp;

	fd = open(argv[1], O_RDONLY);
	fstat(fd, &sb);

	ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
	lp = ptr;
	for (i = 0; i < sb.st_size/sizeof(*lp); i++) {
		assert(lp[i] == 0);
	}
}
