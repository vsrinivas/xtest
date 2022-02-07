#include "pwq.h"

int main(int argc, char* argv[]) {
  struct barrier b = {};
  PWQ::Run([]() { printf("hello, world\n"); }, &b);
  bwait(&b);
}
