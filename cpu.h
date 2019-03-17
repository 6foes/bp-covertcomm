#define __USE_GNU
#define _GNU_SOURCE

#include <sched.h>
#include <pthread.h>

void setProcessor(int proc) {
  cpu_set_t mask;
  int status;

  CPU_ZERO(&mask);
  CPU_SET(proc, &mask);
  status = sched_setaffinity(0, sizeof(mask), &mask);
  if (status != 0) {
    fprintf(stderr, "proc error\n");
    return;
  }
}
