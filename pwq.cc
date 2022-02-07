#include <functional>

#include "pwq.h"
#include "workqueue.h"

struct Workitem {
  std::function<void()> callback;
};

static void worker(void* arg, struct barrier* b) {
  Workitem* it = static_cast<Workitem*>(arg);
  it->callback();
  delete it;
}

void PWQ::Run(std::function<void()> fn, struct barrier* b) {
  Workitem* it = new Workitem({fn});
  workqueue(worker, it, b);
}
