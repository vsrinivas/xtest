#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t FNV1A_32(const char *s, size_t len);
uint64_t FNV1A_64(const char *s, size_t len);
uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length);
uint64_t fasthash64(const void *buf, size_t len, uint64_t seed);
uint32_t fasthash32(const void *buf, size_t len, uint32_t seed);

#ifdef __cplusplus
}
#endif
