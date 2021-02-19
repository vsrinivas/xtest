#include "fnv1a.h"

#include <stdint.h>
#include <sys/types.h>

#define FNV_PRIME_64	(0x100000001b3ull)
#define FNV_OFFSET_64	(0xcbf29ce484222325ull)
uint64_t FNV1A_64(const char *s, size_t len) {             // FNV1a
        uint64_t hash = FNV_OFFSET_64;
        size_t i;
        for(i = 0; i < len; i++) {
                hash = hash ^ (s[i]);
                hash = hash * FNV_PRIME_64;
        }
        return hash;
} 

#define FNV_PRIME_32	(16777619u)
#define FNV_OFFSET_32 	(2166136261u)
uint32_t FNV1A_32(const char *s, size_t len) {             // FNV1a
	uint32_t hash = FNV_OFFSET_32;
	size_t i;       
	for(i = 0; i < len; i++) {
		hash = hash ^ (s[i]);
		hash = hash * FNV_PRIME_32;
	}       
	return hash;
}
