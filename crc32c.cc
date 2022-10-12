// https://github.com/google/cpu-check/blob/master/crc32c.c

#include <sys/types.h>
#include <stdint.h>

#if __SSE4_2__ || __SSE4A__
#include <x86intrin.h>

uint32_t crc32c_hw(const char *src, size_t len) {
  const unsigned char *s = (unsigned char *)src;
  uint64_t hh = ~0;
#ifdef __x86_64__
  while (len > 7) {
    uint64_t v = *(uint64_t *)s;
    hh = _mm_crc32_u64(hh, v);
    s += 8;
    len -= 8;
  }
#endif /* __x86_64__ */
  uint32_t h = (uint32_t)hh;
  if (len > 3) {
    uint32_t v = *(uint32_t *)s;
    h = _mm_crc32_u32(h, v);
    s += 4;
    len -= 4;
  }
  if (len > 1) {
    uint16_t v = *(uint16_t *)s;
    h = _mm_crc32_u16(h, v);
    s += 2;
    len -= 2;
  }
  if (len > 0) {
    uint8_t v = *(uint8_t *)s;
    h = _mm_crc32_u8(h, v);
    s += 1;
    len -= 1;
  }
  return ~h;
}
#endif /* __SSE4_2__ || __SSE4A__ */

/* CRC-32C (iSCSI) polynomial in reversed bit order. */
#define POLY 0x82f63b78

uint32_t crc32c_sw(const char *src, size_t len) {
  const unsigned char *s = (unsigned char *)src;
  uint32_t h = ~0;
  while (len--) {
    h ^= *s++;
    for (int k = 0; k < 8; k++) h = h & 1 ? (h >> 1) ^ POLY : h >> 1;
  }
  return ~h;
}
