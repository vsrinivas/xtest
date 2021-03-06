#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t FNV1A_32(const char *s, size_t len);
uint64_t FNV1A_64(const char *s, size_t len);

#ifdef __cplusplus
}
#endif
