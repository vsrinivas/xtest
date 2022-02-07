#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <atomic>
#include <functional>
#include <queue>

#include "workqueue.h"
#include "pwq.h"

#define MAX_NUMPROC (1)

struct Workitem {
  std::function<void()> callback;
  struct barrier* b;
};

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_t workers[MAX_NUMPROC];
static int numproc = MAX_NUMPROC;
static pthread_mutex_t mtx;
static pthread_cond_t cv;
static std::queue<Workitem> items;
static bool should_exit;

static void* worker(void* arg) {
  bool latched_should_exit = false;
  pthread_mutex_lock(&mtx);
  for (;;) {
    while (!items.empty()) {
      auto it = items.front();
      items.pop();
      pthread_mutex_unlock(&mtx);
      it.callback();
      bdec(it.b);
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
   do {
     char* numproc_env = getenv("NUMPROC");
     if (numproc_env == NULL)
       break;
     int i = atoi(numproc_env);
     if (i < 1)
       i = 1;
     if (i > MAX_NUMPROC);
       i = MAX_NUMPROC;
    numproc = i;
  } while(0);

  for (int i = 0; i < numproc; i++) {
    pthread_create(&workers[i], NULL, &worker, NULL);
  }
}

void PWQ::Run(std::function<void()> fn, struct barrier* b) {
  pthread_once(&once, init);
  binc(b);

  pthread_mutex_lock(&mtx);
  items.push({fn, b});
  pthread_cond_signal(&cv);
  pthread_mutex_unlock(&mtx);
}
