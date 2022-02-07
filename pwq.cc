#include <functional>

#include "pwq.h"
#include "workqueue.h"

struct Workitem {
  std::function<void(struct barrier*, uint64_t)> callback;
};

static void worker(void* arg, struct barrier* b, uint64_t key) {
  Workitem* it = static_cast<Workitem*>(arg);
  it->callback(b, key);
  delete it;
}

void PWQ::Run(std::function<void(struct barrier*, uint64_t)> fn, struct barrier* b, uint64_t key) {
  Workitem* it = new Workitem({fn});
  workqueue(worker, it, b, 0);
}
