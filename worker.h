#ifndef _WORKER_H_
#define _WORKER_H_

#include <pthread.h>
#include <functional>
#include <queue>

class Worker {
 public:
  void Start();
  void Stop();
  bool Run(std::function<void()>);

  void WorkerThread();

 private:
  std::queue<std::function<void()>> work_;
  pthread_t worker_;
  pthread_mutex_t mtx_;
  pthread_cond_t cv_;
  bool started_ = false;
  bool should_exit_ = false;
};

#endif  // _WORKER_H_
