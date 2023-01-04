#include <sys/types.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "defer.h"

struct work {
	struct work *next;
	/*
	 * 0 = MUNMAP
	 * 1 = CLOSE
	 */
	int opcode;
	int fd;
	void *buf;
	size_t size;
};

static pthread_t g_worker;
static pthread_mutex_t g_mutex;
static pthread_cond_t g_cond;
static bool g_should_exit = false;
static struct work *g_work

static void *worker(void *arg) {
	struct work *w;
	pthread_mutex_lock(&g_mutex);
	for (;;) {
		pthread_cond_wait(&g_cond, &g_mutex);
	}
	pthread_mutex_unlock(&g_mutex);
	return NULL;
}

static void worker_start(void) {
	pthread_create(&g_worker, NULL, worker, NULL);
}

void defer_init(void) {
	static pthread_once_t initializer = PTHREAD_ONCE_INIT;
	pthread_once(&initializer, worker_start);
}

void defer_munmap(int fd, void *buf, size_t size) {

}

void defer_close(int fd) {

}
