#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <thread>
#include <sched.h>
#include <string.h>
#include <limits.h>
#include "hashes.h"

#include "checkbuf.inc"

static int do_checkbuf = 0;

#ifdef __x86_64__
static void
rep_movsb(unsigned char* dst, unsigned char const* src, size_t n) {
  __asm__ __volatile__("rep movsb" : "+D"(dst), "+S"(src), "+c"(n)
                       : : "memory");
}
#endif

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
// 	hash it ( both fnv1a, jenkins, murmur one-at-a-time hashes )
// 	compare the jenkins hash against a precomputed hash (if available)
//	make a copy of it into the destination buffer (misaligned if possible)
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
// c++ -o cpu_check cpu_check.cc fnv1a.cc -DN=<size>
// ./cpu_check <N> <max_loops>
int main(int argc, char *argv[]) {
  std::vector<uint8_t> data_src, data_dst;
  volatile uint32_t jhash0;
  volatile uint64_t hash0;
  volatile uint32_t mhash0;
  int loops = 0;
  int cpus;
  size_t size;
  int i;
  int rotor = 0;
  int misalign = 0;
  int MAX_MISALIGN = 64;
  int max_loops;

  cpus = std::thread::hardware_concurrency();

  if (argc > 1)
    size = atol(argv[1]);
  else
    size = N;

  if (argc > 2)
    max_loops = atoi(argv[2]);
  else
    max_loops = INT_MAX;

  if (size == N) {
    printf("do_checkbuf=1, Jenkins hashes will be checked.\n");
    do_checkbuf = 1;
  }

  data_src.resize(N);
  data_dst.resize(N + MAX_MISALIGN);
  for (loops = 0; loops < max_loops; loops++) {
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
    if (do_checkbuf && jcheckbuf.count(loops)) {
      if (jcheckbuf[loops] != jhash0) {
	      printf("WARNING: jenkins hash at loop %d didn't match precomputed table, %x exp %x\n", loops, jhash0, jcheckbuf[loops]);
	      abort();
      }
    }
#ifdef DEBUG
    printf("Source (cpu %d) jhash %x hash %lx mhash %x...\n", rotor, jhash0, hash0, mhash0);
#endif
#ifdef __x86_64__
    if (loops & 1) {
      rep_movsb(dst, data_src.data(), data_src.size());
    } else {
      memcpy(dst, data_src.data(), data_src.size());
    }
#else
    memcpy(dst, data_src.data(), data_src.size());
#endif

    for (int i = 0; i < cpus; i++) {
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
    if (rotor == cpus)
      rotor = 0;
  }
  misalign++;
  if (misalign == MAX_MISALIGN)
    misalign = 0;
}
