#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <sched.h>
#include <string.h>
#include "fnv1a.h"

#include "checkbuf.inc"

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

void move(int cpu) {
  cpu_set_t set;
  int rc;
  CPU_ZERO(&set);
  CPU_SET(cpu, &set);
  rc = sched_setaffinity(0, sizeof(set), &set);
  if (rc != 0)
    abort();
}

void randomize(std::vector<uint8_t>& v, int ref) {
  for (size_t i = 0; i < v.size(); ++i) {
    v[i] = 0xAA + (i & 0xFF) + ref;
  }
}

extern "C" uint32_t murmur3_32(const uint8_t *key, size_t len, uint32_t seed);

// A simple tester for CPU and memory subsystems. 
//
// For each cpu
// 	fill a random buffer;
// 	hash it ( both fnv1a and jenkins one-at-a-time hashes )
//	make a copy of it into the destination buffer
//	for each cpu:
//		hash the copy (fnv1a, jenkins, murmur)
//		compare the hash to the originally computed hash;
//	move back to the source cpu
//	re-hash the source and destination buffers
//	clear the source/destination buffers;
//	advance source buffer.
//
// See: https://github.com/google/cpu-check
// Think of this as a simplified, standalone version of that -- I couldn't
// build that code on a target environment where i had memtest86 report bad
// memory, so I wanted to test it via other mechanisms.
//
// c++ -o cpu_check cpu_check.cc fnv1a.cc -DN=<size> -DNCPU=<ncpu>
int main(int argc, char *argv[]) {
  std::vector<uint8_t> data_src, data_dst;
  volatile uint32_t jhash0;
  volatile uint64_t hash0;
  volatile uint32_t mhash0;
  int loops = 0;
  int cpus = NCPU;
  size_t size = N;
  int i;
  int rotor = 0;
  int misalign = 0;
  int MAX_MISALIGN = 64;

  data_src.resize(N);
  data_dst.resize(N + MAX_MISALIGN);
  for (;; loops++) {
    uint8_t* const dst = data_dst.data() + misalign;
    size_t dst_size = N;
    printf("loop %d ==>\n", loops);
    move(rotor);
#ifdef DEBUG
    printf("Randomize buffer... (source cpu %d)\n", rotor);
#endif
    randomize(data_src, loops);
    jhash0 = jenkins_one_at_a_time_hash((const uint8_t *) data_src.data(), data_src.size());
    hash0 = FNV1A_64((const char *) data_src.data(), data_src.size());
    mhash0 = murmur3_32((const uint8_t*) data_src.data(), data_src.size(), 0x1);
    if (jcheckbuf.count(loops)) {
      if (jcheckbuf[loops] != jhash0) {
	      printf("WARNING: jenkins hash at loop %d didn't match precomputed table, %x exp %x\n", loops, jhash0, jcheckbuf[loops]);
	      abort();
      }
    }
#ifdef DEBUG
    printf("Source (cpu %d) jhash %x hash %lx mhash %x...\n", rotor, jhash0, hash0, mhash0);
#endif
    memcpy(dst, data_src.data(), data_src.size());

    for (int i = 0; i < NCPU; i++) {
      move(i);
      uint32_t jhash = jenkins_one_at_a_time_hash(dst, dst_size);
      uint64_t hash = FNV1A_64((const char *) dst, dst_size);
      uint32_t mhash = murmur3_32((const uint8_t *) dst, dst_size, 0x1);
#ifdef DEBUG
      printf("Validate buffer (source cpu %d target cpu %d (jhash %x hash %lx mhash %x)...\n", rotor, i, jhash, hash, mhash);
#endif
      if (jhash0 != jhash) {
	abort();
      }
      if (hash0 != hash) {
        abort();
      }
      if (mhash0 != mhash) {
	abort();
      }
    }
    move(rotor);
#ifdef DEBUG
    printf("Clear buffers (source cpu %d)...\n", rotor);
#endif
    // Back on the source CPU; check the hashes again.
    uint32_t jhash = jenkins_one_at_a_time_hash(data_src.data(), data_src.size());
    if (jhash0 != jhash) {
      abort();
    }
    jhash = jenkins_one_at_a_time_hash(dst, dst_size);
    if (jhash0 != jhash) {
      abort();
    }
    uint64_t hash = FNV1A_64((const char *) dst, dst_size);
    if (hash0 != hash) {
      abort();
    }
    hash = FNV1A_64((const char *) data_src.data(), data_src.size());
    if (hash0 != hash) {
      abort();
    }
    memset(data_src.data(), 0xAA, data_src.size());
    memset(data_dst.data(), 0xAA, data_dst.size());
    rotor++;
    if (rotor == NCPU)
      rotor = 0;
  }
  misalign++;
  if (misalign == MAX_MISALIGN)
    misalign = 0;
}