#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <atomic>
#include <functional>
#include <queue>
#include "pwq.h"

#define NUMPROC (4)
static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_t workers[NUMPROC];
static pthread_mutex_t mtx;
static pthread_cond_t cv;
static std::queue<std::function<void()>> items;
static std::atomic<int> nItems;
static bool should_exit;

static void* worker(void* arg) {
  bool latched_should_exit = false;
  pthread_mutex_lock(&mtx);
  for (;;) {
    while (!items.empty()) {
      auto it = items.front();
      items.pop();
      nItems--;
      pthread_mutex_unlock(&mtx);
      it();
      pthread_mutex_lock(&mtx);
    }

    if (latched_should_exit)
      break;
    if (!should_exit)
      pthread_cond_wait(&cv, &mtx);
    latched_should_exit = should_exit;
  }
  pthread_mutex_unlock(&mtx);
  return NULL;
}

static void init() {
  pthread_mutex_lock(&mtx);
  should_exit = false;
  pthread_mutex_unlock(&mtx);

  for (int i = 0; i < NUMPROC; i++) {
    pthread_create(&workers[i], NULL, &worker, NULL);
  }
}

void PWQ::Run(std::function<void()> fn) {
  pthread_once(&once, init);

#if 0
  if (nItems.load() > 8) {
    for (int i = 0; i < nItems.load(); i++) {
      sched_yield();
      sync();
    }
  }
  if (nItems.load() > 32) {
    usleep(100);
    sync();
  }
#endif
#if 0
  if (nItems.load() > 32) {
    printf("> XOFF %d\n", nItems.load());
    sync();
    usleep(10 * std::min(4, nItems.load()));
    printf("> XON %d\n", nItems.load());
  }
#endif

  pthread_mutex_lock(&mtx);
  items.push(fn);
  nItems++;
  pthread_cond_signal(&cv);
  pthread_mutex_unlock(&mtx);

  if (nItems.load() > 32) {
    usleep(100);
  }
}

void PWQ::Flush() {
  pthread_mutex_lock(&mtx);
  should_exit = true;
  pthread_cond_broadcast(&cv);
  pthread_mutex_unlock(&mtx);

  for (int i = 0; i < NUMPROC; i++) {
    pthread_join(workers[i], NULL);
  }
  assert(nItems == 0);
}
