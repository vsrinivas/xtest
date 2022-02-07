#ifndef _PWQ_H_
#define _PWQ_H_

#include <functional>
#include "workqueue.h"

class PWQ {
 public:
  // Execute |fn| in a process-wide threadpool.
  // |barrier| allows waiting for |fn| to be executed.
  static void Run(std::function<void()> fn, struct barrier *b);
};

#endif  // _PWQ_H_
