#include <stdatomic.h>
#include <threads.h>
#include <stdbool.h>

struct notification {
  mtx_t mutex;
  cnd_t cond;
  bool notified;
};

bool wait_for_notification(struct notification *n) {
  mtx_lock(&n->mutex);
  while (n->notified == false) {
    cnd_wait(&n->cond, &n->mutex);
  }
  mtx_unlock(&n->mutex);
  return true;
}

void signalx(struct notification *n) {
  mtx_lock(&n->mutex);
  n->notified = true;
  cnd_signal(&n->cond);
  mtx_unlock(&n->mutex);
}
