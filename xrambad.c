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
	unsigned long l;
	unsigned long *p;
	unsigned long size;
	unsigned long n;
	int rc;
	unsigned long *bad = NULL;
	unsigned long badl;

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
	for (l = 0; l < n; l++) {
		CHECK_EQ(p[l], pattern(l));
		if (p[l] != pattern(l)) {
			bad = &p[l];
			badl = l;
		}
		asm volatile("pause ; lfence" ::: "memory");
	}
	printf("Bad %p\n", bad);
	if (!bad) return 0;
	printf("Zerofill unneeded locations\n");
	for (l = 0; l < n; l++) {
		void *px = (void *) (((unsigned long) &p[l]) & ~KPAGE_MASK);
		void *bx = (void *) (((unsigned long) bad) & ~KPAGE_MASK);
		if (px != bx) {
			p[l] = 0;
		}
	}
	printf("Munlock\n");
	for (l = 0; l < n; l += (KPAGE_SIZE/sizeof(unsigned long))) {
		void *px = (void *) (((unsigned long) &p[l]) & ~KPAGE_MASK);
		void *bx = (void *) (((unsigned long) bad) & ~KPAGE_MASK);
		if ((px != bx) && ((((unsigned long) px) & KPAGE_MASK) == 0)) {
			munlock(px, KPAGE_SIZE);
			madvise(px, KPAGE_SIZE, MADV_DONTNEED);
		}
	}
	printf("Scan\n");
	for (;;) {
		CHECK_EQ(*bad, pattern(badl));
		asm volatile("pause ; lfence" ::: "memory");
		sleep(60);
	}
}
