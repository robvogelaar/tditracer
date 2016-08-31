
#define VERSION "0.1"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

void usage(void) {
  printf("tdistat v%s (%s %s)\n", VERSION, __DATE__, __TIME__);
  printf("\nUsage: tdistat\n\n");
}

/******************************************************************************/
int main(int argc, char *argv[]) {
  if (argc > 1 &&
      (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
    usage();
    return 0;
  }

  DIR *dp;
  struct dirent *ep;

  dp = opendir("/tmp/");
  if (dp != NULL) {
    while (ep = readdir(dp)) {
      if (strncmp(ep->d_name, "tditracebuffer@", 15) == 0) {
        FILE *file;
        char filename[128];
        char *bufmmapped;
        unsigned int *ptr;

        sprintf(filename, "/tmp/%s", ep->d_name);

        if ((file = fopen(filename, "r")) != NULL) {
          /* /tmp/.tditracebuffer-xxx-xxx */

          struct stat st;
          stat(filename, &st);

          fprintf(stderr, "\"%s\" (%lluMB)\n", filename, (unsigned long long)st.st_size / (1024 * 1024));

          bufmmapped = (char *)mmap(0, st.st_size, PROT_READ,
                                    MAP_PRIVATE, fileno(file), 0);

          // token should hold "TDITRACEBUFFER"
          if (strncmp("TDITRACE", bufmmapped, 8) !=
              0) {
            fprintf(stderr,
                    "invalid "
                    "tracebuffer, skipping\n");
            break;
          }

          if ((argc == 1) || (strcmp(argv[1], "-pct") == 0)) {

            /*
             * [TDIT]
             * [RACE]
             * [    ]timeofday_offset.tv_usec
             * [    ]timeofday_offset.tv_sec
             * [    ]clock_monotonic_offset.tv_nsec
             * [    ]clock_monotonic_offset.tv_sec
             * ------
             * [    ]marker, lower 2 bytes is length in dwords
             * [    ]clock_monotonic_timestamp.tv_nsec
             * [    ]clock_monotonic_timestamp.tv_sec
             * [    ]text, padded with 0 to multiple of 4 bytes
             * ...
             * ------
             */

            unsigned int *ptr = (unsigned int*)bufmmapped;

            int i = 0;
            ptr+= 6;
            while(*ptr) {
              ptr+= *ptr & 0xffff;
              i++;
            }
            fprintf(stdout, "%lld%% (#%d, %dB)\n", ((char*)ptr - bufmmapped) * 100LL / (st.st_size) + 1, i, (char*)ptr - bufmmapped);
          }

          munmap(bufmmapped, st.st_size);

          fclose(file);
        }
      }
    }
  }
  return 0;
}
