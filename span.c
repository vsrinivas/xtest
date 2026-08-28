#include <stdbool.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

#define N (384)
static atomic_int m_ack;
static atomic_int m_done;

static atomic_ulong go;
static atomic_ulong s;
struct __attribute__((aligned(64))) td {
	pthread_t thr;
	unsigned long i;
};
static struct td tds[N];

static unsigned long rdtsc(void) { return __builtin_ia32_rdtsc(); }
static unsigned long rdtscp(void) { unsigned int aux; return __builtin_ia32_rdtscp(&aux); }
static void xpause(void) { __builtin_ia32_pause(); }

void *worker(void *p) {
        unsigned long i;
	unsigned long upto;
	unsigned long me = (uintptr_t) p;

	atomic_fetch_add(&m_ack, 1);
        for (;;) {
		upto = atomic_load_explicit(&go, memory_order_relaxed);
		if (upto)
			break;
        }

        for (i = 0;; i++) {
                atomic_fetch_add(&s, 1);

		if (((i & 0xF) == 0) && (rdtsc() > upto)) {
			tds[me].i = i;
			break;
		}
        }
	atomic_fetch_add(&m_done, 1);
        return NULL;
}

/* span CPUs seconds (opt) */
/* Increment contended atomic, see how many increments each core gets in */
int main(int argc, char *argv[]) {
        int m;
        int i;
        unsigned long l;
	int st = 1;
	unsigned long t0, t1, seconds;
	double s1 = 0, s2 = 0;

        m = atoi(argv[1]);
	if (argc > 2)
		st = atoi(argv[2]);

	t0 = rdtscp();
	sleep(st);
	t1 = rdtscp();
	seconds = t1 - t0;

        for (i = 0; i < m; i++)
                pthread_create(&tds[i].thr, NULL, worker, (uintptr_t) i);

	while (atomic_load(&m_ack) != m)
		xpause();
        atomic_store_explicit(&go, rdtscp() + seconds, memory_order_relaxed);
		/* accumulate */;
	while (atomic_load(&m_done) != m)
		xpause();

	for (i = 0; i < m; i++)
		pthread_join(tds[i].thr, NULL);

        l = atomic_load_explicit(&s, memory_order_relaxed);
        printf("%d, %lu, %lu\n", m, l, l / m);
	for (i = 0; i < m; i++)
		printf("%d, %lu\n", i, tds[i].i);
	/* Jain's fairness index */
	for (i = 0; i < m; i++) {
		s1 += tds[i].i;
		s2 += (tds[i].i * tds[i].i);
	}
	s2 *= m;
	printf("%f\n", (s1 * s1) / s2);
}
