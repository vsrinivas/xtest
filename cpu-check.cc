#include <stdint.h>
#include <vector>
#include <random>
#include <functional>
#include <sched.h>
#include <string.h>
#include "fnv1a.h"

#ifdef __x86_64__
void clflush(const uint8_t *addr) {
  asm __volatile__ (
     "clflush 0(%0)\n"
     "sfence\n"
     :
     : "r" (addr)
     : "memory"
  );
}
#endif

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

void randomize(std::vector<uint8_t>& v) {
  static std::default_random_engine             generator;
  static std::uniform_int_distribution<uint8_t> distribution(0, 0xff);
  static auto                                   dice = std::bind(distribution, generator);

  for (size_t i = 0; i < v.size(); ++i) {
    v[i] = dice();
  }
}

// A simple tester for CPU and memory subsystems. 
//
// For each cpu
// 	fill a random buffer;
// 	evict it from cache (clflush)
// 	hash it ( both fnv1a and jenkins one-at-a-time hashes)
//	make a copy of it into the destination buffer
//	for each cpu:
//		hash the copy (fnv1a, jenkins)
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
  uint32_t jhash0;
  uint64_t hash0;
  int loops = 0;
  int cpus = NCPU;
  size_t size = N;
  int i;
  int rotor = 0;

  data_src.resize(N);
  data_dst.resize(N);
  for (;; loops++) {
    printf("loop %d ==>\n", loops);
    move(rotor);
#ifdef DEBUG
    printf("Randomize buffer... (source cpu %d)\n", rotor);
#endif
    randomize(data_src);
#ifdef __x86_64__
    for (uint8_t* p = data_src.data(); p < data_src.data() + data_src.size(); p += 64) {
      clflush(p);
    }
#endif
    jhash0 = jenkins_one_at_a_time_hash((const uint8_t *) data_src.data(), data_src.size());
    hash0 = FNV1A_64((const char *) data_src.data(), data_src.size());
#ifdef DEBUG
    printf("Source (cpu %d) jhash %x hash %lx...\n", rotor, jhash0, hash0);
#endif
    memcpy(data_dst.data(), data_src.data(), data_src.size());

    for (int i = 0; i < NCPU; i++) {
      move(i);
      uint32_t jhash = jenkins_one_at_a_time_hash(data_dst.data(), data_dst.size());
      uint64_t hash = FNV1A_64((const char *) data_dst.data(), data_dst.size());
#ifdef DEBUG
      printf("Validate buffer (source cpu %d target cpu %d (jhash %x hash %lx)...\n", rotor, i, jhash, hash);
#endif
      if (jhash0 != jhash) {
	abort();
      }
      if (hash0 != hash) {
        abort();
      }
    }
    move(rotor);
#ifdef DEBUG
    printf("Clear buffers (source cpu %d)...\n", rotor);
#endif
    // Back on the source CPU; check the hashes again.
#ifdef __x86_64__
    for (uint8_t* p = data_src.data(); p < data_src.data() + data_src.size(); p += 64) {
      clflush(p);
    }
    for (uint8_t* p = data_dst.data(); p < data_dst.data() + data_dst.size(); p += 64) {
      clflush(p);
    }
#endif
    uint32_t jhash = jenkins_one_at_a_time_hash(data_src.data(), data_src.size());
    if (jhash0 != jhash) {
      abort();
    }
    jhash = jenkins_one_at_a_time_hash(data_dst.data(), data_dst.size());
    if (jhash0 != jhash) {
      abort();
    }
    uint64_t hash = FNV1A_64((const char *) data_dst.data(), data_dst.size());
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
}
