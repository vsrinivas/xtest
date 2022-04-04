#include "defer.h"

#include <unistd.h>

void* c_worker(void* arg) {
  Defer* d = (Defer*) arg;
  d->worker();
  return NULL;
}

Defer::Defer() {
  should_exit_ = false;
  pthread_create(&worker_, NULL, c_worker, this);
}

void Defer::stop() {
  pthread_mutex_lock(&mu_);
  should_exit_ = true;
  pthread_mutex_unlock(&mu_);
  pthread_join(worker_, NULL);
}

Defer* Defer::Default() {
  static Defer d;
  return &d;
}

void Defer::defer(std::function<void()> f, struct barrier* b) {
  struct work w = { f, b };
  binc(b);
  pthread_mutex_lock(&mu_);
  work_.push_back(w);
  pthread_mutex_unlock(&mu_);
}

void Defer::worker() {
  bool latched_should_exit = false;
  for (;;) {
    if (latched_should_exit)
      break;
    
    std::deque<struct work> work;
    pthread_mutex_lock(&mu_);
    latched_should_exit = should_exit_;
    work.swap(work_);
    pthread_mutex_unlock(&mu_);

    while (!work.empty()) {
      auto t = work.front();
      t.f();
      bdec(t.barrier);
      work.pop_front();
    }
    sleep(1);  // TODO: pthread_cond_timedwait(), so stop() is handled fast
  }
}
