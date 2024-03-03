// g++ witness.cc -O -march=native third_party/cityhash/src/city.cc -I. -Ithird_party/cityhash/src -o b.out
#include <limits.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#include <city.h>
#include <citycrc.h>

#define KB 1024ul
#define MB 1024ul * KB
#define GB 1024ul * MB

#ifndef N // Buffer size
#define N (1 * GB)
#endif

void randomize(std::vector<uint8_t> &v, int ref) {
  uint8_t val[8];
  size_t qwords = v.size() / sizeof(uint64_t);
  size_t extra = v.size() - (qwords * sizeof(uint64_t));
  size_t extraoff = qwords * sizeof(uint64_t);

  for (size_t i = 0; i < qwords; i += sizeof(uint64_t)) {
    for (int j = 0; j < sizeof(uint64_t); j++) {
      val[i] = 0xAA + ref + ((i + j) & 0xff);
    }
    memcpy(v.data() + i, val, sizeof(val));
  }
  for (size_t i = 0; i < extra; i++) {
    v[extraoff + i] = 0xAA + ref + ((extraoff + i) & 0xFF);
  }
}

int main(int argc, char *argv[]) {
  std::vector<uint8_t> data_src, data_dst;
  uint128 city_hash0;
  uint32_t loops = 0;
  size_t size;
  int i;
  int misalign = 0;
  int MAX_MISALIGN = 64;
  int max_loops;

  if (argc > 1)
    size = atol(argv[1]);
  else
    size = N;

  if (argc > 2)
    max_loops = atoi(argv[2]);
  else
    max_loops = INT_MAX;

  data_src.resize(size);
  data_dst.resize(size + MAX_MISALIGN);
  randomize(data_src, loops);
  city_hash0 = CityHashCrc128((const char *)data_src.data(), data_src.size());
  for (loops = 0; loops < max_loops; loops++) {
    printf("loop %d ==>\n", loops);
    uint8_t *const dst = data_dst.data() + misalign;
    size_t dst_size = size;
    memcpy(dst, data_src.data(), dst_size);
    uint128 city_hash = CityHashCrc128((const char *)dst, dst_size);
    if (city_hash0 != city_hash) {
      printf("Expected %lx:%lx, got %lx:%lx\n", city_hash0.second, city_hash0.first,
             city_hash.second, city_hash.first);
      abort();
    }
    misalign++;
    if (misalign == MAX_MISALIGN)
      misalign = 0;

    memset(data_dst.data(), 0xff, data_dst.size()); 
    usleep(1);
  }
}
