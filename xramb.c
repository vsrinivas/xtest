#include <sys/mman.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define KB (1024)
#define MB (1024 * KB)

// lock a pattern in ram and scan for it.
// argv[1] = mb to lock in ram.;
unsigned long pattern(unsigned long l) {
	return (l) | ((~l) << 32);
}

void CHECK_EQ(unsigned long x, unsigned long y) {
	if (x != y) {
		printf("CHECK FAILED: %lx != %lx\n", x, y);
		abort();
	}
}

int main(int argc, char *argv[]) {
	unsigned long l;
	unsigned long *p;
	unsigned long size;
	unsigned long n;
	int rc;

	if (argc == 1)
		size = 1 * MB;
	else
		size = atol(argv[1]) * MB;

	n = size / sizeof(unsigned long);
	p = malloc(size);
	memset(p, 0, size);
	
	rc = mlock(p, size);
	CHECK_EQ(rc, 0);

	for (l = 0; l < n; l++) {
		p[l] = pattern(l);
	}
	for (;;) {
		for (l = 0; l < n; l++) {
			CHECK_EQ(p[l], pattern(l));
			asm volatile("pause ; lfence" ::: "memory");
		}
		sleep(16);
	}
}
