#define _GNU_SOURCE
#include <fcntl.h>
#include <stddef.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <assert.h>

// Take anonymous memory and back it with a file (a memfd).
// Somehow, this works.
int mem_back(void *p, size_t size) {
	int mfd;
	int pipes[2];
	struct iovec v;

	mfd = syscall(SYS_memfd_create, "test", 0);
	pipe(pipes);

	// Gift the source pages to the pipe. Splice the pipe into the dest memfd.
	v.iov_base = p;
	v.iov_len = size;
	vmsplice(pipes[1], &v, 1, SPLICE_F_GIFT);
	splice(pipes[0], NULL, mfd, NULL, size, 0 /* SPLICE_F_MOVE? */);
	mmap(p, 0, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_FILE | MAP_SHARED, mfd, 0);
	return mfd;
}

int main(int argc, char *argv[]) {
	unsigned char *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	int i;
	int fd;

	for (i = 0; i < 4096; i++)
		p[i] = i;
	for (i = 0; i < 4096; i++)
		assert(p[i] == (i & 0xff));

	fd = mem_back(p, 4096);

	for (i = 0; i < 4096; i++)
		assert(p[i] == (i & 0xff));

	unsigned char buf[4];
	pread(fd, buf, 4, 1);
	assert(buf[0] == 1);
	assert(buf[1] == 2);
	assert(buf[2] == 3);
	assert(buf[3] == 4);
}
