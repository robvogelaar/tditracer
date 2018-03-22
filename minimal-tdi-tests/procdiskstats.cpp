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
#include <syscall.h>
#include <unistd.h>

#define OUT(name, trace, value)           \
                                          \
  static unsigned int _##name##_seen = 0; \
  static unsigned int _##name##_prev;     \
  unsigned int _##name = value;           \
  if (!_##name##_seen) {                  \
    _##name##_seen = 1;                   \
    printf(trace, _##name);               \
  } else if (_##name##_prev != _##name) { \
    printf(trace, _##name);               \
  }                                       \
  _##name##_prev = _##name;

int main(void) {
  char line[256];
  FILE* f = NULL;

  /*
     1 - major number
     2 - minor mumber
     3 - device name
     4 - reads completed successfully
     5 - reads merged
     6 - sectors read
     7 - time spent reading (ms)
     8 - writes completed
     9 - writes merged
    10 - sectors written
    11 - time spent writing (ms)
    12 - I/Os currently in progress
    13 - time spent doing I/Os (ms)
    14 - weighted time spent doing I/Os (ms)
  */

  struct diskstat_t {
    char name[16];
    char match[64];
    unsigned int reads;
    unsigned int reads_merged;
    unsigned int reads_sectors;
    unsigned int reads_time;
    unsigned int writes;
    unsigned int writes_merged;
    unsigned int writes_sectors;
    unsigned int writes_time;
  };

  diskstat_t ds[10];

  int nr_disks = 0;
  int d = 0;
  char* pt;
  pt = strtok(getenv("DISKS"), ",");
  while (pt != NULL) {
    strcpy(ds[d].name, pt);
    sprintf(ds[d].match, "%s %%u %%u %%u %%u %%u %%u %%u %%u", pt);
    pt = strtok(NULL, ",");
    d++;
    nr_disks++;
  }


  int i;
  for (i = 0; i < 100000; i++) {
    if ((f = fopen("/proc/diskstats", "r"))) {
      while (fgets(line, 256, f)) {
        for (d = 0; d < nr_disks; d++) {
          char* pos;
          diskstat_t* pds = &ds[d];
          if ((pos = strstr(line, pds->name))) {
            sscanf(pos, pds->match, &pds->reads, &pds->reads_merged,
                   &pds->reads_sectors, &pds->reads_time, &pds->writes,
                   &pds->writes_merged, &pds->writes_sectors,
                   &pds->writes_time);
            break;
          }
        }
      }
    }
    fclose(f);
    for (d = 0; d < nr_disks; d++) {
      printf("ds[%d] -> %s, r:%u w:%u\n", d, ds[d].name, ds[d].reads,
             ds[d].writes);
    }
    printf("---------\n");
    usleep(1 * 1000 * 1000);
  }
}
