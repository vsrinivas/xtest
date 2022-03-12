#include <stdint.h>
#include <vector>
#include <random>
#include <functional>
#include <sched.h>
#include <string.h>

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
  return hash;
}

void randomize(std::vector<uint8_t>& v, int ref) {
  for (size_t i = 0; i < v.size(); ++i) {
    v[i] = 0xAA + (i & 0xFF) + ref;
  }
}

int main(int argc, char *argv[]) {
  std::vector<uint8_t> data_src(N);
  int X = 100;
  if (argc > 1)
    X = atoi(argv[1]);
  // print
  printf("std::unordered_map<int, uint32_t> jcheckbuf = {\n");
  for (int i = 0; i < X; i++) {
    fprintf(stderr, "checkbuf [%d/%d]\n", i, X); 
    randomize(data_src, i);
    uint32_t jhash = jenkins_one_at_a_time_hash(data_src.data(), data_src.size());
    printf("{%d, 0x%x}\n", i, jhash);
  }
  printf("};\n");
}
