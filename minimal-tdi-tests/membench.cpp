#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>

extern void tditrace(const char* format, ...) __attribute__((weak));

static char uninitialized1[8*4096];

static char initialized2[8*4096] = {0,1};

int main(int argc, char* argv[]) {
  void* allocations_mmap1[1024];
  void* allocations_malloc1[1024];
  void* allocations_malloc2[1024];

  if (argc != 10) {
    fprintf(stdout,
            "usage: %s [loops] [secondsperloop] [timespersecond] mmap1[KB] "
            "malloc1[KB] "
            "malloc2[KB] mmap1_inc[KB] malloc1_inc[KB] malloc2_inc[KB]\n",
            argv[0]);
    exit(-1);
  }

  int loops = atoi(argv[1]);
  int secondsperloop = atoi(argv[2]);
  int timespersecond = atoi(argv[3]);
  int mmap1 = atoi(argv[4]) * 1024;
  int malloc1 = atoi(argv[5]) * 1024;
  int malloc2 = atoi(argv[6]) * 1024;
  int mmap1_inc = atoi(argv[7]) * 1024;
  int malloc1_inc = atoi(argv[8]) * 1024;
  int malloc2_inc = atoi(argv[9]) * 1024;

  fprintf(
      stdout,
      "[loops=%d] [secondsperloop=%d] [timespersecond=%d] mmap1[KB=%d] "
      "malloc1[KB=%d] "
      "malloc2[KB=%d] mmap1_inc[KB=%d] malloc1_inc[KB=%d] malloc2_inc[KB=%d]\n",
      loops, secondsperloop, timespersecond, mmap1 / 1024, malloc1 / 1024,
      malloc2 / 1024, mmap1_inc / 1024, malloc1_inc / 1024, malloc2_inc / 1024);

  memset(allocations_mmap1, 0, sizeof allocations_mmap1);
  memset(allocations_malloc1, 0, sizeof allocations_malloc1);
  memset(allocations_malloc2, 0, sizeof allocations_malloc2);

  int i = 0;

  while (loops--) {
    fprintf(stdout, "%d:", loops);
    fflush(stdout);
    for (i = 0; i < timespersecond; i++) {
      if (mmap1) {
        allocations_mmap1[i] = mmap(NULL, mmap1, PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE | MAP_ANONYMOUS
                                    /* | MAP_POPULATE */
                                    ,
                                    -1, 0);
        if (allocations_mmap1[i] != (char*)-1) {
          memset(allocations_mmap1[i], 0, mmap1);
          fprintf(stdout, ".");
          fflush(stdout);
        }
      }

      if (malloc1) {
        allocations_malloc1[i] = malloc(malloc1);
        memset(allocations_malloc1[i], 0, malloc1);
      }
      if (malloc2) {
        allocations_malloc2[i] = malloc(malloc2);
        if (allocations_malloc2[i]) memset(allocations_malloc2[i], 0, malloc2);
      }
      usleep(secondsperloop * 500 * 1000 / timespersecond);
    }

    for (i = 0; i < timespersecond; i++) {
      if (allocations_mmap1[i]) munmap(allocations_mmap1[i], mmap1);
      if (allocations_malloc1[i]) free(allocations_malloc1[i]);
      if (allocations_malloc2[i]) free(allocations_malloc2[i]);
      usleep(secondsperloop * 500 * 1000 / timespersecond);
    }

    fprintf(stdout, "\n");

    mmap1 += mmap1_inc;
    malloc1 += malloc1_inc;
    malloc2 += malloc2_inc;
  }
  printf("done\n");
  while (1) usleep(1000 * 1000);
}
