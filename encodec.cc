#include <unordered_set>
#include <stdint.h>
#include <stdio.h>

#define N	(65536)

#define FNV_PRIME_64	(0x100000001b3ull)
#define FNV_OFFSET_64	(0xcbf29ce484222325ull)
uint64_t FNV1A(const char *s, size_t len) {             // FNV1a
        uint64_t hash = FNV_OFFSET_64;
        size_t i;
        for(i = 0; i < len; i++) {
                hash = hash ^ (s[i]);
                hash = hash * FNV_PRIME_64;
        }
        return hash;
} 

typedef uint64_t hash_t;

int main(int argc, char *argv[]) {
	std::unordered_set<hash_t> blocks;
	char* buf;
	int i;
	uint64_t nBlocks = 0;

	buf = (char*) calloc(1, N);
	for (;;) {
		if (feof(stdin))
			break;

		i = fread(buf, 1, N, stdin);
		if (i == 0)
			break;
		if (i != N) {
			printf("Short Block\n");	
			continue;
		}

		hash_t h = FNV1A(buf, i);
		blocks.insert(h);
		nBlocks++; 
	}
	free(buf);

	printf("%ld unique %d byte blocks\n", blocks.size(), N);
	printf("%ld total blocks\n", nBlocks);
}
