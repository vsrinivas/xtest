#include "hashes.h"
#include <atomic>
#include <limits.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <vector>

#ifdef __x86_64__
#include <emmintrin.h>
#endif

#define KB 1024ul
#define MB 1024ul * KB
#define GB 1024ul * MB

#ifndef N // Default buffer size
#define N (1 * GB)
#endif

#ifdef __x86_64__
extern "C" void _zencpy(void *dst, void *src, size_t len);
extern "C" void _zencpy2(void *dst, void *src, size_t len);
extern "C" void vcopy(void *dst, void *src, size_t len);
extern "C" void vcopy2(void *dst, void *src, size_t len);

static void rep_movsb(void *dst, void *src, size_t n) {
  __asm__ __volatile__("rep movsb"
                       : "+D"(dst), "+S"(src), "+c"(n)
                       :
                       : "memory");
}
#endif

void xpause() {
#ifdef __x86_64__
  __builtin_ia32_pause();
#endif
}

void flush(void *p, size_t size) {
#ifdef __x86_64__
  for (size_t i = 0; i < size; i += 64) {
    _mm_clflush((char *)p + i);
  }
  asm volatile("sfence" ::: "memory");
#endif
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

void randomize(std::vector<uint8_t> &v, int ref) {
  uint8_t val[8];
  size_t qwords = v.size() / sizeof(uint64_t);
  size_t extra = v.size() - (qwords * sizeof(uint64_t));
  size_t extraoff = qwords * sizeof(uint64_t);

  for (size_t i = 0; i < qwords; i += sizeof(uint64_t)) {
    for (int j = 0; j < sizeof(uint64_t); j++) {
      val[j] = 0xAA + ref + ((i + j) & 0xff);
    }
    memcpy(v.data() + i, val, sizeof(val));
  }
  for (size_t i = 0; i < extra; i++) {
    v[extraoff + i] = 0xAA + ref + ((extraoff + i) & 0xFF);
  }
}

extern "C" uint32_t murmur3_32(const uint8_t *key, size_t len, uint32_t seed);

struct args {
  uint64_t hash;
  uint32_t mhash;
  void *dst;
  size_t dst_size;
};
static std::atomic<uint32_t> g_go;
static std::atomic<uint32_t> g_ack;
static std::atomic<bool> g_should_exit;
static std::atomic<int> rotor;
struct args g_args;

static void pickmemcpy(void *dst, void *src, size_t size) {
#ifdef __x86_64__
    static __thread int loops;
    int t = loops++ % 6;
    if (t == 0) {
      rep_movsb(dst, src, size);
    } else if (t == 1) {
      _zencpy(dst, src, size);
    } else if (t == 2) {
      vcopy(dst, src, size);
    } else if (t == 3) {
      vcopy2(dst, src, size);
    } else if (t == 4) {
      _zencpy2(dst, src, size);
    } else {
      memcpy(dst, src, size);
    }
#else
    memcpy(dst, data_src.data(), data_src.size());
#endif
}

static void check(int cpu) {
  uint32_t last = 0;
  move(cpu);

  for (;;) {
    if (g_should_exit) {
      g_ack--;
      break;
    }
    if (last == g_go) {
      xpause();
      usleep(1);
      continue;
    }

    uint64_t hash = FNV1A_64((const char *)g_args.dst, g_args.dst_size);
    uint32_t mhash =
        murmur3_32((const uint8_t *)g_args.dst, g_args.dst_size, 0x1);
#ifdef DEBUG
    printf("Validate buffer (source cpu %d target cpu %d align %lu ("
           "hash %lx mhash %x)...\n",
           rotor.load(), cpu, ((uintptr_t)g_args.dst) & (64 - 1), hash, mhash);
#endif
    if (g_args.hash != hash) {
      abort();
    }
    if (g_args.mhash != mhash) {
      abort();
    }

    last = g_go;
    g_ack--;
  }
}

// A simple tester for CPU and memory subsystems.
//
// For each cpu
// 	fill a random buffer;
// 	hash it ( fnv1a, murmur one-at-a-time hashes )
//	make a copy of it into the destination buffer (misaligned if possible)
//	   uses one of six copy routines; rep movsb, movsq+movsb, movnti, memcpy, sse, avx(256-bit)
//	for each cpu (in parallel optionally):
//		hash the copy (fnv1a, murmur)
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
// c++ -o pcpucheck pcpucheck.cc hashes.c murmur3.c  zencpy.S zencpy2.S -pthread -DPARALLEL
// ./pcpucheck <N> <max_loops>
int main(int argc, char *argv[]) {
  std::vector<uint8_t> data_src, data_dst, witness;
  volatile uint64_t hash0;
  volatile uint32_t mhash0;
  uint64_t witness_hash_0, witness_hash_1;
  uint32_t loops = 0;
  int cpus;
  size_t size;
  int i;
  int misalign = 0;
  int MAX_MISALIGN = 64;
  int max_loops;

  cpus = std::thread::hardware_concurrency();
  std::vector<std::thread> threads;
  for (int i = 0; i < cpus; i++) {
    std::thread th(check, i);
    threads.push_back(std::move(th));
  }

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
  witness.resize(8 * MB);
  randomize(witness, 1);
  flush(witness.data(), witness.size());
  witness_hash_0 = FNV1A_64((const char *) witness.data(), witness.size());
  for (loops = 0; loops < max_loops; loops++) {
    uint8_t *const dst = data_dst.data() + misalign;
    size_t dst_size = size;
    printf("loop %d ==>\n", loops);
    ::move(rotor);
#ifdef DEBUG
    printf("Randomize buffer... (source cpu %d)\n", rotor.load());
#endif
    randomize(data_src, loops);
    hash0 = FNV1A_64((const char *)data_src.data(), data_src.size());
    mhash0 = murmur3_32((const uint8_t *)data_src.data(), data_src.size(), 0x1);
#ifdef DEBUG
    printf("Source (cpu %d) hash %lx mhash %x...\n", rotor.load(),
           hash0, mhash0);
#endif
    pickmemcpy(dst, data_src.data(), data_src.size());

    g_ack = cpus;
    g_args.hash = hash0;
    g_args.mhash = mhash0;
    g_args.dst = dst;
    g_args.dst_size = dst_size;
    g_go = loops + 1; // Release all other CPUs;
    // Wait for every CPU to checksum buffers;
    while (g_ack.load() != 0) {
      xpause();
      usleep(5);
    }

#ifdef DEBUG
    printf("Check buffers (source cpu %d)...\n", rotor.load());
#endif
    // Back on the source CPU; quick check the src/dst buffers once again
    uint64_t hash = FNV1A_64((const char *)data_src.data(), data_src.size());
    if (hash0 != hash) {
      abort();
    }
    rotor++;
    if (rotor == cpus)
      rotor = 0;
    misalign++;
    if (misalign == MAX_MISALIGN)
      misalign = 0;

    if (misalign == 0) {  // Once in a while, check the witness buffer.
      witness_hash_1 = FNV1A_64((const char *) witness.data(), witness.size());
      if (witness_hash_0 != witness_hash_1)
        abort();
    }
  }

  g_should_exit = true;
  for (int i = 0; i < cpus; i++) {
    threads[i].join();
  }
}
