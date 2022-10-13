#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>
#include <nmmintrin.h>

#include "hashes.h"

uint64_t xtime(void) {
	struct timeval t0;
	gettimeofday(&t0, NULL);
	return t0.tv_sec * 1000ull * 1000ull + t0.tv_usec;
}


float rate(uint64_t bytes, uint64_t usec) {
	return (((((bytes * 1.0) / (usec * 1.0)) / 1024.0) / 1024.0) / 1024.0) * 1000.0 * 1000.0;
}


int main(int argc, char *argv[]) {
	char *buf;

	int sizes[] = { 1024, 4096, 16384, 65536 };
	unsigned long iters = 10000ul;

	int i;
	unsigned long j;
	float s;
	uint64_t before, after;

	for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
		buf = malloc(sizes[i]);
		memset(buf, 0, sizes[i]);

		// fnv1a
		before = xtime();
		for (j = 0; j < iters; j++) {
			FNV1A_32(buf, sizes[i]);
			asm volatile("" : : : "memory");
		}
		after = xtime();
		printf("FNV1a: %lu iters over %lu bytes in %lu usec: ", iters, sizes[i], after-before);
		printf("%f GB/s \n", rate(iters * sizes[i], after-before));

		// fnv1a 64-bit
		before = xtime();
                for (j = 0; j < iters; j++) {
                        FNV1A_64(buf, sizes[i]);
                        asm volatile("" : : : "memory");
                }
                after = xtime();
                printf("FNV1a (64-bit): %lu iters over %lu bytes in %lu usec: ", iters, sizes[i], after-before);
                printf("%f GB/s \n", rate(iters * sizes[i], after-before));

		// jenkins
                before = xtime();
                for (j = 0; j < iters; j++) {
                        jenkins_one_at_a_time_hash(buf, sizes[i]);
                        asm volatile("" : : : "memory");
                }
                after = xtime();
                printf("jenkins_one_at_a_time_hash: %lu iters over %lu bytes in %lu usec: ", iters, sizes[i], after-before);
                printf("%f GB/s \n", rate(iters * sizes[i], after-before));


                before = xtime();
                for (j = 0; j < iters; j++) {
                        fasthash32(buf, sizes[i], 0);
                        asm volatile("" : : : "memory");
                }
                after = xtime();
                printf("fasthash32: %lu iters over %lu bytes in %lu usec: ", iters, sizes[i], after-before);
                printf("%f GB/s \n", rate(iters * sizes[i], after-before));

                before = xtime();
                for (j = 0; j < iters; j++) {
                        fasthash64(buf, sizes[i], 0);
                        asm volatile("" : : : "memory");
                }
                after = xtime();
                printf("fasthash64: %lu iters over %lu bytes in %lu usec: ", iters, sizes[i], after-before);
                printf("%f GB/s \n", rate(iters * sizes[i], after-before));

		free(buf);
	}

}
