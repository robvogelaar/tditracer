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
Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes
packets errs drop fifo colls carrier compressed
enp0s3: 14758314  103019    0    0    0     0          0         0 86157268
78296    0    0    0     0       0          0
  */

  struct netdev_t {
    unsigned long r_bytes;
    unsigned int r_packets;
    unsigned int r_errs;
    unsigned int r_drop;
    unsigned int r_fifo;
    unsigned int r_frame;
    unsigned int r_compressed;
    unsigned int r_multicast;
    unsigned long t_bytes;
    unsigned int t_packets;
    unsigned int t_errs;
    unsigned int t_drop;
    unsigned int t_fifo;
    unsigned int t_frame;
    unsigned int t_compressed;
    unsigned int t_multicast;
  };

  netdev_t n1, n2;

#if 1
  int i;
  for (i = 0; i < 100000; i++) {
    if ((f = fopen("/proc/net/dev", "r"))) {
      while (fgets(line, 256, f)) {
        char* pos;
        if ((pos = strstr(line, "bcm0:"))) {
          sscanf(pos, "bcm0:%lu %u %u %u %u %u %u %u %lu %u %u %u %u %u %u %u",
                 &n1.r_bytes, &n1.r_packets, &n1.r_errs, &n1.r_drop, &n1.r_fifo,
                 &n1.r_frame, &n1.r_compressed, &n1.r_multicast, &n1.t_bytes,
                 &n1.t_packets, &n1.t_errs, &n1.t_drop, &n1.t_fifo, &n1.t_frame,
                 &n1.t_compressed, &n1.t_multicast);
        } else if ((pos = strstr(line, "eth0:"))) {
          sscanf(pos, "eth0:%lu %u %u %u %u %u %u %u %lu %u %u %u %u %u %u %u",
                 &n2.r_bytes, &n2.r_packets, &n2.r_errs, &n2.r_drop, &n2.r_fifo,
                 &n2.r_frame, &n2.r_compressed, &n2.r_multicast, &n2.t_bytes,
                 &n2.t_packets, &n2.t_errs, &n2.t_drop, &n2.t_fifo, &n2.t_frame,
                 &n2.t_compressed, &n2.t_multicast);
        }
      }
    }
    fclose(f);
    printf("bcm0_r_bytes=%u\n", (n1.r_packets));
    printf("eth0_r_bytes=%u\n", (n2.r_packets));
    usleep(1 * 1000 * 1000);
  }
}
#endif
