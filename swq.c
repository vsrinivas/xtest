#include "swq.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>

struct workqueue_item {
	void (*fn)(void*);
	void *priv;
};

#define N (32)
static workqueue_item swq_fifo[N];
static atomic_uint_least32_t swq_head, swq_tail;
static atomic_
static pthread_once_t swq_once = PTHREAD_ONCE_INIT;

static void *swq(void *arg) {
	for (;;) {
		sleep(5);
		if (atomic_load(
	}
}

static void swq_init(void) {

}

void swq(void (*fn)(void *), void *priv) {
	pthread_once(&swq_once, swq_init);

	
}
