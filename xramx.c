#include <sys/mman.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <emmintrin.h>
#include <stdio.h>

#define KB (1024)
#define MB (1024 * KB)

// lock a pattern in ram and scan for it.
// argv[1] = mb to lock in ram.;
unsigned long pattern(unsigned long l) {
	return (l) | ((~l) << 32);
}

#define CHECK_EQ(_x, _y)	\
	if ((_x) != (_y)) {	\
		printf("%s:%d Check failed: %lx != %lx\n", __FILE__, __LINE__, _x, _y);	\
		abort();	\
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
		CHECK_EQ(p[l], pattern(l));
		_mm_clflush(&p[l]);
		asm volatile("pause ; lfence ; sfence" ::: "memory");
		CHECK_EQ(p[l], pattern(l));
		asm volatile("pause ; lfence ; sfence" ::: "memory");
	}
	for (;;) {
		for (l = 0; l < n; l++) {
			CHECK_EQ(p[l], pattern(l));
			asm volatile("pause ; lfence" ::: "memory");
		}
		sleep(16);
	}
}
