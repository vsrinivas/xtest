#include <sys/mman.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define KPAGE_SIZE ((4096 * 4096))
#define KPAGE_MASK ((KPAGE_SIZE) - 1)
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
	}

int main(int argc, char *argv[]) {
	unsigned long l, m;
	unsigned long *p;
	unsigned long size;
	unsigned long n;
	int rc;
	int match;

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
	}
	asm volatile("pause ; lfence" ::: "memory");
	unsigned long step = KPAGE_SIZE / sizeof(unsigned long);
	for (l = 0; l < n; l += step) {
		void *pp = &p[l];
		match = 0;
		for (m = 0; m < step; m++) {
			asm volatile("pause ; lfence" ::: "memory");
			if (p[m] != pattern(m)) {
				match = 1;
				break;
			} else {
				p[m] = 0;
			}
		}
		if (!match) {
			munlock(pp, KPAGE_SIZE);
			madvise(pp, KPAGE_SIZE, MADV_FREE);
		}

	}
}
