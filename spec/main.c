#include <stdint.h>
#include <limits.h>
#include <x86intrin.h>

uint64_t safe;
uint64_t standoff[64];
uint64_t buffer;

uint64_t load(uint64_t *);
void touch(uint64_t *);
uint64_t now(void);
uint64_t brcond(uint64_t *, uint64_t);

uint64_t lo_cached;
uint64_t lo_uncached;
int is_buffer_cached(void) {
	uint64_t a, b, c;
	uint64_t dcached, duncached;

	a = now();
	load(&buffer);
	b = now();

	c = b - a;
	dcached = abs(c - lo_cached);
	duncached = abs(c - lo_uncached);
	if (dcached < duncached)
		return 1;
	else
		return 0;
}

int main(int argc, char *argv[]) {
	int i, j;
	int lo, hi, sum;
	uint64_t a, b, c;
	void *p;
	uint64_t ctr, ctr_max;

	// cached access time
	lo = INT_MAX, hi = 0, sum = 0;
	load(&safe);
	for (i = 0; i < 1000; i++) {
		a = now();
		load(&safe);
		b = now();
		c = b - a;
		if (lo > c) lo = c;
		if (hi < c) hi = c;
		sum += c;
	}
	lo_cached = lo;
	printf("cached access [%d, %d, %d]\n", lo, sum/i, hi);

	// uncached access time
	lo = INT_MAX, hi = 0, sum = 0;
	for (i = 0; i < 1000; i++) {
		_mm_clflush(&buffer);
		a = now();
		load(&buffer);
		b = now();
		c = b - a;
		if (lo > c) lo = c;
		if (hi < c) hi = c;
		sum += c;
	}
	lo_uncached = lo;
	printf("uncached access [%d, %d, %d]\n", lo, sum/i, hi);

	// quickcheck;
	load(&buffer);
	printf("acccess to |buffer| after load %d (exp 1)\n", is_buffer_cached());
	_mm_clflush(&buffer);
	printf("access to |buffer| after clflush %d (exp 0)\n", is_buffer_cached());

	// speculation demo
	lo = INT_MAX, hi = 0, sum = 0;
	ctr_max = 1129, ctr = ctr_max;
	for (i = 0; i < 1000; i++) {
		load(&safe);
		_mm_clflush(&buffer);
		ctr = ctr_max;
		for (j = 0; j < ctr_max; j++) {
			p = (ctr) ? &safe : &buffer;
			//p = &safe;	// masks speculation by accessing only |safe|
			ctr = brcond(p, ctr);
		}
		a = now();
		load(&buffer);
		b = now();
		c = b - a;
		if (lo > c) lo = c;
		if (hi < c) hi = c;
		sum += c;
	}
		// if lo overlaps w/ the cached access time, the branch
		// in brcond() was mispredicted and a speculative access to |buffer|
		// happened.
	printf("candidate access [%d, %d, %d]\n", lo, sum/i, hi);
}
