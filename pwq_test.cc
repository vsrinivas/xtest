#include "pwq.h"

int main(int argc, char* argv[]) {
  struct barrier b = {};
  for (int i = 0; i < 4; i++) {
    PWQ::Run([](struct barrier*, uint64_t) { printf("hello, world\n"); }, &b, 0);
  }
  bwait(&b);
}
