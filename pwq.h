#ifndef _PWQ_H_
#define _PWQ_H_

#include <functional>
#include "workqueue.h"

class PWQ {
 public:
  // Execute |fn| in a process-wide threadpool.
  // |barrier| allows waiting for |fn| to be executed.
  // |key| is passed to each callback; all callbacks with the same key run on the same thread
  static void Run(std::function<void(struct barrier*, uint64_t)> fn, struct barrier *b, uint64_t key);
};

#endif  // _PWQ_H_
