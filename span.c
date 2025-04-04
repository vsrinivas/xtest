#include <stdbool.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#define N       (100000000ul)
static atomic_int m_ack;
static atomic_ulong go;
static atomic_ulong s;
static pthread_t thrs[16];

static unsigned long rdtsc(void) { return __builtin_ia32_rdtsc(); }
static unsigned long rdtscp(void) { unsigned int aux; return __builtin_ia32_rdtscp(&aux); }

void *worker(void *) {
        unsigned long i;
	unsigned long upto;

	atomic_fetch_add(&m_ack, 1);
        for (;;) {
		upto = atomic_load_explicit(&go, memory_order_relaxed);
		if (upto)
			break;
        }

        for (i = 0;; i++) {
                atomic_fetch_add(&s, 1);

		if (((i & 0xF) == 0) && (rdtsc() > upto))
			break;
        }
        return NULL;
}

int main(int argc, char *argv[]) {
        int m;
        int i;
        unsigned long l;
	unsigned long t0, t1, seconds;

        m = atoi(argv[1]);

	t0 = rdtscp();
	sleep(1);
	t1 = rdtscp();
	seconds = t1 - t0;

        for (i = 0; i < m; i++)
                pthread_create(&thrs[i], NULL, worker, NULL);

	while (atomic_load(&m_ack) != m)
		;
        atomic_store_explicit(&go, rdtscp() + seconds, memory_order_relaxed);

	for (i = 0; i < m; i++)
		pthread_join(thrs[i], NULL);

        l = atomic_load_explicit(&s, memory_order_relaxed);
        printf("%d, %lu, %lu\n", m, l, l / m);
}
