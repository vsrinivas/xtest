#include "worker.h"

static void *worker(void *arg) {
  static_cast<Worker *>(arg)->WorkerThread();
  return NULL;
}

void Worker::Start() {
  pthread_mutex_init(&mtx_, NULL);
  pthread_cond_init(&cv_, NULL);

  pthread_create(&worker_, NULL, worker, this);

  pthread_mutex_lock(&mtx_);
  while (!started_) pthread_cond_wait(&cv_, &mtx_);
  pthread_mutex_unlock(&mtx_);
}

void Worker::Stop() {
  pthread_mutex_lock(&mtx_);
  should_exit_ = true;
  started_ = false;
  pthread_cond_signal(&cv_);
  pthread_mutex_unlock(&mtx_);

  pthread_join(worker_, NULL);
}

bool Worker::Run(std::function<void()> callback) {
  pthread_mutex_lock(&mtx_);
  if (!started_) {
    pthread_mutex_unlock(&mtx_);
    return false;
  }

  work_.push(std::move(callback));
  pthread_cond_signal(&cv_);
  pthread_mutex_unlock(&mtx_);
  return true;
}

void Worker::WorkerThread() {
  pthread_mutex_lock(&mtx_);
  started_ = true;
  pthread_cond_signal(&cv_);
  pthread_mutex_unlock(&mtx_);
  sched_yield();

  pthread_mutex_lock(&mtx_);
  bool latched_should_exit = false;
  for (;;) {
    while (!work_.empty()) {
      std::queue<std::function<void()>> work;
      work_.swap(work);
      pthread_mutex_unlock(&mtx_);

      while (!work.empty()) {
        auto w = work.front();
        work.pop();
        w();
      }

      pthread_mutex_lock(&mtx_);
    }

    if (latched_should_exit) {
      break;
    }
    latched_should_exit = should_exit_;
    if (!latched_should_exit) {
      pthread_cond_wait(&cv_, &mtx_);
    }
  }
  pthread_mutex_unlock(&mtx_);
}
