#include "pwq.h"
#include <future>

int main(int argc, char* argv[]) {
  struct barrier b = {};
  for (int i = 0; i < 4; i++) {
    PWQ::Run([](struct barrier*, uint64_t) { printf("hello, world\n"); }, &b, 0);
  }
  bwait(&b);

  std::packaged_task<int(int, int)> task([](int x, int y) { return x + y; });
  std::future<int> r = task.get_future();
  PWQ::Run([&task]() { task(1, 2); });
  printf("%d\n", r.get());
}
