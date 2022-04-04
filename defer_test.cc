#include "defer.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  struct barrier b = {};
  Defer::Default()->defer([](void) { printf("hello, world\n"); }, &b);
  bwait(&b);
}
