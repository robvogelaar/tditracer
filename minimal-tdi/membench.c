#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <malloc.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

extern void tditrace(const char* format, ...) __attribute__((weak));

int main(int argc, char* argv[]) {
  void* allocations_mmap1[1024];
  void* allocations_malloc1[1024];
  void* allocations_malloc2[1024];

  if (argc != 7) {
    fprintf(
        stdout,
        "usage: %s [seconds] [timespersecond] mmap1[KB] malloc1[KB] malloc2[KB] "
        "percentage-increase[PCT]\n",
        argv[0]);
    exit(-1);
  }

  int seconds = atoi(argv[1]);
  int timespersecond = atoi(argv[2]);
  int mmap1 = atoi(argv[3]) * 1024;
  int malloc1 = atoi(argv[4]) * 1024;
  int malloc2 = atoi(argv[5]) * 1024;
  float pct = atof(argv[6]);

  fprintf(stdout,
          "[seconds=%d] [timespersecond=%d] mmap1[KB=%d] malloc1[KB=%d] "
          "malloc2[KB=%d] percentage-increase[PCT=%f]\n",
          seconds, timespersecond, mmap1/1024, malloc1/1024, malloc2/1024, pct);

  memset(allocations_mmap1, 0, sizeof allocations_mmap1);
  memset(allocations_malloc1, 0, sizeof allocations_malloc1);
  memset(allocations_malloc2, 0, sizeof allocations_malloc2);

  size_t i = 0;

  while (seconds--) {
    for (i = 0; i < timespersecond; i++) {
      if (mmap1) {
        allocations_mmap1[i] = mmap(NULL, mmap1, PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE | MAP_ANONYMOUS
                                    /* | MAP_POPULATE */
                                    ,
                                    -1, 0);
        if ((int)allocations_mmap1[i] != -1)
          memset(allocations_mmap1[i], 0, mmap1);
      }

      if (malloc1) {
        allocations_malloc1[i] = malloc(malloc1);
        memset(allocations_malloc1[i], 0, malloc1);
      }
      if (malloc2) {
        allocations_malloc2[i] = malloc(malloc2);
        if (allocations_malloc2[i]) memset(allocations_malloc2[i], 0, malloc2);
      }
      usleep(500 * 1000 / timespersecond);
    }

    //usleep(10 * 1000 * 1000);

    for (i = 0; i < timespersecond; i++) {
      if (allocations_mmap1[i]) munmap(allocations_mmap1[i], mmap1);
      if (allocations_malloc1[i]) free(allocations_malloc1[i]);
      if (allocations_malloc2[i]) free(allocations_malloc2[i]);
      usleep(500 * 1000 / timespersecond);
    }

    mmap1 += (mmap1 * pct) / 100;
    malloc1 += (malloc1 * pct) / 100;
    malloc2 += (malloc2 * pct) / 100;
  }
  printf("done\n");
  while (1) usleep(1000 * 1000);
}
