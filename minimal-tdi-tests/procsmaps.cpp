#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if 0
unsigned long hash(unsigned char *str) {
  unsigned long hash = 5381;
  int c;

  while (c = *str++) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}
#endif

#if 0
int hash(char const *input) {
  int result = 0x55555555;

  while (*input) {
    result ^= *input++;
    result = rol(result, 5);
  }
}
#endif

#if 0
#define A 54059   /* a prime */
#define B 76963   /* another prime */
#define C 86969   /* yet another prime */
#define FIRSTH 37 /* also prime */
unsigned hash(const char *s) {
  unsigned h = FIRSTH;
  while (*s) {
    h = (h * A) ^ (s[0] * B);
    s++;
  }
  return h % 101;  // or return h % C;
}
#endif

unsigned int hash(unsigned int x) {
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = (x >> 16) ^ x;
  return x;
}

int main(int argc, char **argv) {
  char fname[1024];
  static char proc_self_smaps[16 * 1024 + 1];
  int fd;
  int bytes;

  sprintf(fname, "/proc/%s/smaps", argc > 1 ? argv[1] : "self");
  fd = open(fname, O_RDONLY);
  if (fd >= 0) {
    unsigned int code_rss = 0;
    unsigned int code_pss = 0;
    unsigned int code_ref = 0;
    unsigned int data_rss = 0;
    unsigned int data_pss = 0;
    unsigned int data_ref = 0;
    unsigned int data_swap = 0;

    while (1) {
      bytes = read(fd, proc_self_smaps, sizeof(proc_self_smaps) - 1);
      // fprintf(stdout, "read:%d\n", bytes);
      if ((bytes == -1) && (errno == EINTR))
        /* keep trying */;
      else if (bytes > 0) {
        proc_self_smaps[bytes] = '\0';
        char *saveptr;
        char *line = strtok_r(proc_self_smaps, "\n", &saveptr);
        while (line) {
          unsigned long start, end;
          char flags[5];
          unsigned long long pgoff;
          unsigned int maj, min;
          unsigned long ino, dum;
          char name[1024];
          int n;
          if (sscanf(line, "%08lx-%08lx", &dum, &dum) == 2) {
            strcpy(name, "ANONYMOUS");
            if ((n = sscanf(line, "%08lx-%08lx %s %08llx %02x:%02x %lu %s",
                            &start, &end, flags, &pgoff, &maj, &min, &ino,
                            name)) >= 7) {
            } else {
              printf("SCAN ERROR\n");
              exit(EXIT_FAILURE);
            }
          }
          int rss, pss, sc, sd, pc, pd, ref, anon, swap;
          sscanf(line, "Rss:%d", &rss);
          sscanf(line, "Pss:%d", &pss);
          sscanf(line, "Shared_Clean:%d", &sc);
          sscanf(line, "Shared_Dirty:%d", &sd);
          sscanf(line, "Private_Clean:%d", &pc);
          sscanf(line, "Private_Dirty:%d", &pd);
          sscanf(line, "Referenced:%d", &ref);
          sscanf(line, "Anonymous:%d", &anon);
          sscanf(line, "Swap:%d", &swap);
          if (strstr(line, "Swap")) {
            char *nm = strrchr(name, '/');

            if (strcmp(flags, "---p") != 0) {
              printf("%40s %08lx-%08lx %6ld %s %5d %5d %5d %5d %5d\n",
                     nm ? nm + 1 : name, start, end, (end - start) / 1024,
                     flags, rss, pss, ref, anon, swap);
            }

            if (strcmp(flags, "r-xp") == 0) {
              code_rss += rss;
              code_pss += pss;
              code_ref += ref;
            } else if ((strcmp(flags, "r--p") == 0) ||
                       (strcmp(flags, "rw-p") == 0) ||
                       (strcmp(flags, "rwxp") == 0)) {
              data_rss += rss;
              data_pss += pss;
              data_ref += ref;
              data_swap += swap;
            }
          }
          line = strtok_r(NULL, "\n", &saveptr);
        }

      } else
        break;
    }
    close(fd);
  } else {
    fprintf(stderr, "invalid pid\n");
    exit(EXIT_FAILURE);
  }
}
