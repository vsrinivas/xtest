#ifndef DEFER_H_
#define DEFER_H_

#include <pthread.h>
#include <deque>
#include <functional>
#include "barrier.h"

class Defer {
 public:
  ~Defer() { stop(); }

  static Defer* Default();
  void defer(std::function<void()> f, struct barrier* b);

 private:
  struct work {
    std::function<void()> f;
    struct barrier* barrier;
  };

  Defer();
  void worker();
  void stop();

  pthread_mutex_t mu_;
  pthread_t worker_;
  bool should_exit_;
  std::deque<struct work> work_;

  friend void* c_worker(void*);
};

#endif  // DEFER_H_
