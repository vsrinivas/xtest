#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N (65536)
#define NUM_TARGETS 1536
#define TARGET_ALIGN 128
#define ITERS 100

extern char add_start[];

typedef void (*target_fn)(unsigned long long *);

static uint32_t xrand = 0xc0ffee;
static uint32_t xorshift32(uint32_t *state) {
	uint32_t x = *state;
	x = x ^ (x << 13);
	x = x ^ (x >> 17);
	x = x ^ (x << 5);
	*state = x;
	return x;
}

void shuffle(target_fn *array, size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + xorshift32(&xrand) % (n - i);
            target_fn t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

void CHECK_EQ(unsigned long long x, unsigned long long y) {
    if (x != y) {
        fprintf(stderr, "%llx != %llx\n", x, y);
        abort();
    }
}

static unsigned long long expected(const target_fn *array, size_t n) {
    unsigned long long v = 0;

    for (size_t i = 0; i < n; i++)
        v = v * 3 + (unsigned long long)(uintptr_t)array[i];

    return v;
}

target_fn fp[N];

__attribute__((noinline))
static void xcall(unsigned long long *ctr, int i) {
	unsigned long long frame_marker = (unsigned long long) i;

	fp[i](ctr);
	i++;
	if (i < N)
		xcall(ctr, i);

	asm volatile("" :: "r"(&frame_marker) : "memory");
}

int main(int argc, char *argv[]) {
    target_fn fp2[NUM_TARGETS];
    unsigned long long ctr, want;
    int iters = ITERS;
    int i;
    int j;

    for (i = 0; i < NUM_TARGETS; i++) {
        fp2[i] = (target_fn)(add_start + i * TARGET_ALIGN);
    }

    for (i = 0; i < N; i++) {
        fp[i] = fp2[i % NUM_TARGETS];
    }

    ctr = 0;
    xcall(&ctr, 0);
    want = expected(fp, N);
    CHECK_EQ(ctr, want);

    for (j = 0; j < iters; j++) {
        shuffle(fp, N);
        want = expected(fp, N);

        ctr = 0;
        xcall(&ctr, 0);
        CHECK_EQ(ctr, want);
    }

    return 0;
}
