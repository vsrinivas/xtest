#ifndef _BARRIER_H_
#define _BARRIER_H_

#include <pthread.h>

class Barrier {
 public:
  void Inc();
  void Dec();
  void Wait();

 private:
  pthread_mutex_t mtx_;
  pthread_cond_t cv_;
  int count_;
};

#endif  // _BARRIER_H_
