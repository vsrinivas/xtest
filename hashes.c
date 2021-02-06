#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>
#include <nmmintrin.h>


uint32_t crc32c(unsigned char *buf, size_t len) {
	uint32_t l = 0;
	for (int i = 0; i < len; i++) {
		l = _mm_crc32_u8(l, *buf++);
	}
			asm volatile("" : : : "memory");
	return l;
}



// Compression function for Merkle-Damgard construction.
// This function is generated using the framework provided.
#define mix(h) ({					\
			(h) ^= (h) >> 23;		\
			(h) *= 0x2127599bf4325c37ULL;	\
			(h) ^= (h) >> 47; })

uint64_t fasthash64(const void *buf, size_t len, uint64_t seed)
{
	const uint64_t    m = 0x880355f21e6d1965ULL;
	const uint64_t *pos = (const uint64_t *)buf;
	const uint64_t *end = pos + (len / 8);
	const unsigned char *pos2;
	uint64_t h = seed ^ (len * m);
	uint64_t v;

	while (pos != end) {
		v  = *pos++;
		h ^= mix(v);
		h *= m;
	}

	pos2 = (const unsigned char*)pos;
	v = 0;

	switch (len & 7) {
	case 7: v ^= (uint64_t)pos2[6] << 48;
	case 6: v ^= (uint64_t)pos2[5] << 40;
	case 5: v ^= (uint64_t)pos2[4] << 32;
	case 4: v ^= (uint64_t)pos2[3] << 24;
	case 3: v ^= (uint64_t)pos2[2] << 16;
	case 2: v ^= (uint64_t)pos2[1] << 8;
	case 1: v ^= (uint64_t)pos2[0];
		h ^= mix(v);
		h *= m;
	}

			asm volatile("" : : : "memory");
	return mix(h);
} 

uint32_t fasthash32(const void *buf, size_t len, uint32_t seed)
{
	// the following trick converts the 64-bit hashcode to Fermat
	// residue, which shall retain information from both the higher
	// and lower parts of hashcode.
        uint64_t h = fasthash64(buf, len, seed);
	return h - (h >> 32);
}





// FNV1a (32-bit)
#define FNV_PRIME_32 16777619
#define FNV_OFFSET_32 2166136261U
uint32_t FNV32(const char *s, size_t len) {             // FNV1a
        uint32_t hash = FNV_OFFSET_32;
        size_t i; 
        for(i = 0; i < len; i++) {
                hash = hash ^ (s[i]);
                hash = hash * FNV_PRIME_32;
        }
			asm volatile("" : : : "memory");
        return hash;
}

uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length) {
  size_t i = 0;
  uint32_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
			asm volatile("" : : : "memory");
  return hash;
}

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

	int sizes[] = { 1024, 4096, 16384, 65536, 262144, 1048576 };
	unsigned long iters = 1000000ul;

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
			FNV32(buf, sizes[i]);
			asm volatile("" : : : "memory");
		}
		after = xtime();
		printf("FNV1a: %lu iters over %lu bytes in %lu usec: ", iters, sizes[i], after-before);
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


		// fasthash32
		                // jenkins
                before = xtime();
                for (j = 0; j < iters; j++) {
                        fasthash32(buf, sizes[i], 0);
                        asm volatile("" : : : "memory");
                }
                after = xtime();
                printf("fasthash32: %lu iters over %lu bytes in %lu usec: ", iters, sizes[i], after-before);
                printf("%f GB/s \n", rate(iters * sizes[i], after-before));


                // fasthash32
                                // jenkins
                before = xtime();
                for (j = 0; j < iters; j++) {
                        crc32c(buf, sizes[i]);
                        asm volatile("" : : : "memory");
                }
                after = xtime();
                printf("crc32: %lu iters over %lu bytes in %lu usec: ", iters, sizes[i], after-before);
                printf("%f GB/s \n", rate(iters * sizes[i], after-before));


		free(buf);
	}

}
