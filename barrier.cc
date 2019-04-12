#include "barrier.h"

#include <pthread.h>

void Barrier::Inc() {
  pthread_mutex_lock(&mtx_);
  ++count_;
  pthread_mutex_unlock(&mtx_);
}

void Barrier::Dec() {
  pthread_mutex_lock(&mtx_);
  if (--count_ == 0) {
    pthread_cond_broadcast(&cv_);
  }
  pthread_mutex_unlock(&mtx_);
}

void Barrier::Wait() {
  pthread_mutex_lock(&mtx_);
  while (count_ != 0) {
    pthread_cond_wait(&cv_, &mtx_);
  }
  pthread_mutex_unlock(&mtx_);
}
