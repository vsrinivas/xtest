#include "worker.h"
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  Worker w;
  int i;

  w.Start();
  for (i = 0; i < 10; i++)
    w.Run([i]() { printf("%d\n", i); sleep(1); });

  printf("all pushed.\n");
  w.Stop();
}
