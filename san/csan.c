#define NULL	((void *) 0)
typedef char __tsan_atomic8;

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

static void spin_fast(void) { for (int i = 0; i < 100; i++) asm volatile("pause" ::: "memory"); }
static __thread int mycpu;

#define N	(4)

static void *g_in_flight_writes_old3[N];
static void *g_in_flight_writes_old2[N];
static void *g_in_flight_writes_old1[N];
static void *g_in_flight_writes_cur[N];

__attribute__((no_sanitize_thread))
static int match(void *addr) {
	for (int i = 0; i < N; i++) {
		if (addr == g_in_flight_writes_cur[i]) return 1;
		if (addr == g_in_flight_writes_old1[i]) return 1;
		if (addr == g_in_flight_writes_old2[i]) return 1;
		if (addr == g_in_flight_writes_old3[i]) return 1;
	}
	return 0;
}

__attribute__((no_sanitize_thread))
static void arm(void *addr) {
	g_in_flight_writes_old3[mycpu] = g_in_flight_writes_old2[mycpu];
	g_in_flight_writes_old2[mycpu] = g_in_flight_writes_old1[mycpu];
	g_in_flight_writes_old1[mycpu] = g_in_flight_writes_cur[mycpu];
	g_in_flight_writes_cur[mycpu] = addr;
}
__attribute__((no_sanitize_thread))
static void disarm() {
	arm(NULL);
}
__attribute__((no_sanitize_thread))
static void clear() {  g_in_flight_writes_cur[mycpu] = g_in_flight_writes_old1[mycpu] = g_in_flight_writes_old2[mycpu] = g_in_flight_writes_old3[mycpu] = NULL; }

__attribute__((no_sanitize_thread))
void __tsan_read1(void *addr) {
	unsigned char t0 = *(unsigned char *) addr;
	if (match(addr))
		asm volatile("int3");
	spin_fast();
	unsigned char t1 = *(unsigned char *) addr;
	if (t0 != t1)
		asm volatile("int3");
}

__attribute__((no_sanitize_thread))
void __tsan_write1(void *addr) {
	arm(addr);
	spin_fast();
	disarm();
}

__attribute__((no_sanitize_thread))
__tsan_atomic8 __tsan_atomic8_load(const volatile __tsan_atomic8 *addr, __tsan_memory_order order) {
	return __atomic_load_n(addr, order);
}
__attribute__((no_sanitize_thread))
void __tsan_atomic8_store(volatile __tsan_atomic8 *addr, __tsan_atomic8 v, __tsan_memory_order order) {
	__atomic_store_n(addr, v, order);
	if (order == __tsan_memory_order_seq_cst ||
	    order == __tsan_memory_order_release ||
	    order == __tsan_memory_order_acq_rel)
		clear();
}

__attribute__((no_sanitize_thread))
__tsan_atomic8 __tsan_atomic8_fetch_add(volatile __tsan_atomic8 *a,
                                        __tsan_atomic8 v, __tsan_memory_order order) {
	__atomic_fetch_add(a, v, order);
	if (order == __tsan_memory_order_seq_cst ||
	    order == __tsan_memory_order_release ||
	    order == __tsan_memory_order_acq_rel)
		clear();
}

__attribute__((no_sanitize_thread))
void __tsan_atomic_thread_fence(__tsan_memory_order order) {
	if (order == __tsan_memory_order_seq_cst ||
	    order == __tsan_memory_order_release ||
	    order == __tsan_memory_order_acq_rel)
		clear();
}

void __tsan_init(void) {}
void __tsan_func_entry(void *call_pc) {}
void __tsan_func_exit(void *call_pc) {}

static unsigned char checkpt = 0;
static unsigned char guarded = 0;

__attribute__((no_sanitize_thread)) void cpu_boot(int cpu_num) { mycpu = cpu_num; }
void *cpu(void *ip) {
	int i = (int) ip;
	cpu_boot(i);

	for (;;) {
#ifdef TEST0
		checkpt++;
#endif
#ifdef TEST1
		__atomic_fetch_add(&checkpt, 1, __ATOMIC_SEQ_CST);
#endif
#ifdef TEST2
		if (i == 0) {
			guarded = 1;
			__atomic_store_n(&checkpt, 1, __ATOMIC_RELEASE);
			while (__atomic_load_n(&checkpt, __ATOMIC_ACQUIRE) == 1)
				;
			for (;;);	// success
		} else {
			while (__atomic_load_n(&checkpt, __ATOMIC_ACQUIRE) != 1);
			if (guarded != 1) asm volatile("int3");
			__atomic_store_n(&checkpt, 2, __ATOMIC_RELEASE); 
		}
#endif
	}
}
