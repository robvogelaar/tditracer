#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  char fname[1024];
  static char proc_self_smaps[16 * 1024 + 1];
  int fd;
  int bytes;

  sprintf(fname, "/proc/%s/smaps", argc > 1 ? argv[1] : "self");
  fd = open(fname, O_RDONLY);
  if (fd >= 0) {
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
          char flag_r, flag_w, flag_x, flag_s;
          unsigned long long pgoff;
          unsigned int maj, min;
          unsigned long ino, dum;
          char name[1024];
          int n;
          if (sscanf(line, "%08lx-%08lx", &dum, &dum) == 2) {
            strcpy(name, "ANONYMOUS");
            if ((n = sscanf(line,
                            "%08lx-%08lx %c%c%c%c %08llx %02x:%02x %lu %s",
                            &start, &end, &flag_r, &flag_w, &flag_x, &flag_s,
                            &pgoff, &maj, &min, &ino, name)) >= 10) {
            } else {
              printf("SCAN ERROR");
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
            printf(
                "%40s %08lx-%08lx %6ld %c%c%c%c %5d %5d %5d %5d %5d %5d %5d "
                "%5d %5d\n",
                nm ? nm + 1 : name, start, end, (end - start) / 1024, flag_r,
                flag_w, flag_x, flag_s, rss, pss, sc, sd, pc, pd, ref, anon,
                swap);
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
