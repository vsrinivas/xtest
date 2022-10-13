#define NULL	((void *) 0)
typedef char __tsan_atomic8;
typedef int __tsan_atomic32;

__attribute__((no_sanitize_thread)) void spin_lock(unsigned long *l) {  while (__atomic_exchange_n(l, 1, __ATOMIC_ACQ_REL)); }
__attribute__((no_sanitize_thread)) void spin_unlock(unsigned long *l) { __atomic_store_n(l, 0, __ATOMIC_RELEASE); }

// Part of ABI, do not change.
// http://llvm.org/viewvc/llvm-project/libcxx/trunk/include/atomic?view=markup
typedef enum {
  __tsan_memory_order_relaxed,
  __tsan_memory_order_consume,
  __tsan_memory_order_acquire,
  __tsan_memory_order_release,
  __tsan_memory_order_acq_rel,
  __tsan_memory_order_seq_cst
} __tsan_memory_order;

static void clear(void) {}

#define N	(4)
static __thread int g_mycpu;


struct vectortab {
	int seqno[N];
};
static struct vectortab g_vectortab[N];

struct locationtab {
	unsigned long addr;
	int cpu;
	int seq;
};

#define M	(10000)
static unsigned long g_location_lock;
static struct locationtab g_locations[M];

__attribute__((no_sanitize_thread))
struct locationtab *find(unsigned long addr) {
	static int rotor = 0;
	struct locationtab *l = NULL;

	for (int i = 0; i < M; i++) {
		if (g_locations[i].addr == addr) {
			l = &g_locations[i];
			break;
		}
	}
	// Reclaim an entry.
	if (l == NULL) {
		l = &g_locations[rotor++ % M];
		l->cpu = l->seq = 0;
		l->addr = addr;
	}
	return l;
}


__attribute__((no_sanitize_thread))
void __tsan_read1(void *addr) {
	struct vectortab *vec;
	struct locationtab *loc;

	vec = &g_vectortab[g_mycpu];
	spin_lock(&g_location_lock);
	loc = find((unsigned long) addr);
	// We last synchronized with the writer before it did this write, data race.
	if (vec->seqno[loc->cpu] < loc->seq) {
		asm volatile("int3");
	}
	spin_unlock(&g_location_lock);
}

__attribute__((no_sanitize_thread))
void __tsan_write1(void *addr) {
	struct vectortab *vec;
	struct locationtab *loc;
	int nseq;

	vec = &g_vectortab[g_mycpu];
	nseq = vec->seqno[g_mycpu]++;
	spin_lock(&g_location_lock);
	loc = find((unsigned long) addr);
	loc->cpu = g_mycpu;
	loc->seq = nseq;
	spin_unlock(&g_location_lock);
}

__attribute__((no_sanitize_thread))
__tsan_atomic8 __tsan_atomic8_load(const volatile __tsan_atomic8 *addr, __tsan_memory_order order) {
	struct vectortab *vec;
	struct locationtab *loc;

	spin_lock(&g_location_lock);
	__tsan_atomic8 val = __atomic_load_n(addr, order);
	if (order == __tsan_memory_order_seq_cst ||
	    order == __tsan_memory_order_acquire ||
	    order == __tsan_memory_order_acq_rel) {
		vec = &g_vectortab[g_mycpu];
		loc = find((unsigned long) addr);
		if (loc->seq) {
			// TODO: We might need to update our entire vector
			// with the max of our own value for each remote cpu
			// and the remote cpu's? otherwise we can't track
			// three-cpu sequences?;
			vec->seqno[loc->cpu] = loc->seq;
		}
	} else { asm volatile("int3"); }
	spin_unlock(&g_location_lock);
	return val;
}
__attribute__((no_sanitize_thread))
void __tsan_atomic8_store(volatile __tsan_atomic8 *addr, __tsan_atomic8 v, __tsan_memory_order order) {
	struct vectortab *vec;
	struct locationtab *loc;
	int nseq;

	spin_lock(&g_location_lock);
	__atomic_store_n(addr, v, order);
	if (order == __tsan_memory_order_seq_cst ||
	    order == __tsan_memory_order_release ||
	    order == __tsan_memory_order_acq_rel) {
		vec = &g_vectortab[g_mycpu];
		nseq = vec->seqno[g_mycpu]++;
		loc = find((unsigned long) addr);
		loc->cpu = g_mycpu;
		loc->seq = nseq;
	} else { asm volatile("int3"); }
	spin_unlock(&g_location_lock);
}

// For g_max_ticks, incomplete.
__attribute__((no_sanitize_thread))
__tsan_atomic32 __tsan_atomic32_load(const volatile __tsan_atomic32 *addr, __tsan_memory_order order) {
        return __atomic_load_n(addr, order);
}

void __tsan_init(void) {}
void __tsan_func_entry(void *call_pc) {}
void __tsan_func_exit(void *call_pc) {}

static unsigned char g_check;
static unsigned char g_flag;

__attribute__((no_sanitize_thread)) void cpu_boot(int cpu_num) { g_mycpu = cpu_num; g_vectortab[g_mycpu].seqno[g_mycpu]++; }
void *cpu(void *ip) {
	int i = (int) ip;
	cpu_boot(i);
	extern int g_max_ticks;
	int ticks = 0;

	for (;;) {
#ifdef TEST0
		g_check++;
#endif
#ifdef TEST1
		if (ticks == 0) {
		if (i == 0) {
			g_check++;
			__atomic_store_n(&g_flag, 1, __ATOMIC_RELEASE);
		} else {
			while (__atomic_load_n(&g_flag, __ATOMIC_ACQUIRE) == 0);
			unsigned char xp = g_check;
			if (xp != 1) asm volatile("int3");
		}
		} // ticks == 0;
#endif
		ticks++;
		if (ticks == __atomic_load_n(&g_max_ticks, __ATOMIC_ACQUIRE))
			break;
	}
}
