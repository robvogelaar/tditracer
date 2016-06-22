
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
        char *ptr;
        char *saveptr;
        const char *search = "\f";

        sprintf(filename, "/tmp/%s", ep->d_name);

        if ((file = fopen(filename, "r")) != NULL) {
          /* /tmp/.tditracebuffer-xxx-xxx */

          fprintf(stderr, "Found \"%s\"\n", filename);

          struct stat st;
          stat(filename, &st);

          bufmmapped = (char *)mmap(0, st.st_size, PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE, fileno(file), 0);
          ptr = bufmmapped;

          // token should hold "TDITRACEBUFFER"
          if (strncmp("TDITRACEBUFFER", strtok_r(ptr, search, &saveptr), 14) !=
              0) {
            fprintf(stderr,
                    "invalid "
                    "tracebuffer, skipping\n");
            break;
          }

          if ((argc == 1) || (strcmp(argv[1], "-pct") == 0)) {
            fprintf(stdout, "%d\n",
                    (int)(strlen(saveptr) * 100.0 / (st.st_size)));
          }

          munmap(bufmmapped, st.st_size);

          fclose(file);
        }
      }
    }
  }
  return 0;
}
