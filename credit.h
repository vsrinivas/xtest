#include <pthread.h>
#include <unistd.h>

class Credit {
 public:
  explicit Credit(size_t max): max_(max), current_(max) { }

  void Get(size_t budget) {
    for (;;) {
      pthread_mutex_lock(&mtx_);
      if (current_ >= budget) {
        current_ -= budget;
        pthread_mutex_unlock(&mtx_);
        return;
      }
      pthread_mutex_unlock(&mtx_);
      usleep(7000);  // 7 msec.
    }
  }

  void Put(size_t budget) {
     pthread_mutex_lock(&mtx_);
     current_ += budget;
     pthread_mutex_unlock(&mtx_);
  }

 private:
  pthread_mutex_t mtx_;
  size_t max_;
  size_t current_;
};
