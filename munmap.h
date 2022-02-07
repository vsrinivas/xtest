#include <utility>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <queue>
#include <sys/mman.h>

#include "workqueue.h"

class MUnmap {
 public:
  void Start() { worker_.Start(); worker_.Run([this] { Pull(); }); }

  void Push(char* base, size_t size) {
    pthread_mutex_lock(&mtx_);
    ++nPush_;
    if (!last_base_) {
      last_base_ = base;
      last_size_ = size;
    } else {
      if (last_base_ + last_size_ == base) {
        last_size_ += size;
        ++nCoalesced_;
      } else {
        to_munmap_.push({last_base_, last_size_});
        last_base_ = base;
        last_size_ = size;
      }
    }
    pthread_mutex_unlock(&mtx_);
  }

  void Stop() {
    pthread_mutex_lock(&mtx_);
    should_exit_ = true;
    pthread_mutex_unlock(&mtx_);
    worker_.Stop();

    printf("Pushes %d, Merged %d\n", nPush_, nCoalesced_);
  }

  void Pull() {
    sleep(1);
    pthread_mutex_lock(&mtx_);
    bool should_exit = false; 
    for (;;) {
      if (last_base_) {
        to_munmap_.push({last_base_, last_size_});
        last_base_ = nullptr;
        last_size_ = 0;
      }

      std::queue<std::pair<char*, size_t>> to_munmap;
      to_munmap_.swap(to_munmap);

      pthread_mutex_unlock(&mtx_);

      while (!to_munmap.empty()) {
        auto it = to_munmap.front();
        to_munmap.pop();
        ::munmap(it.first, it.second);
      }
      if (should_exit) {
        pthread_mutex_lock(&mtx_);
        assert(to_munmap_.empty());
        break;
      }
      sleep(3);
      pthread_mutex_lock(&mtx_);
      should_exit = should_exit_;
    }
    pthread_mutex_unlock(&mtx_);
  }

 private:
  pthread_mutex_t mtx_;
  char* last_base_ = nullptr;
  size_t last_size_ = 0;
  bool should_exit_ = false;
  int nPush_ = 0;
  int nCoalesced_ = 0;

  std::queue<std::pair<char*, size_t>> to_munmap_;
};
