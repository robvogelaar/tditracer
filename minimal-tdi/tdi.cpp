#define VERSION "0.1"

#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

void usage(void) {
  printf("tdi v%s (%s %s)\n", VERSION, __DATE__, __TIME__);
  printf("\nUsage: tdi -d [tracebuffer]\n");
  printf("         tdidump : convert tditracebuffer(s) to tdi.\n");
  printf("\nUsage: tdi -s [tracebuffer]\n");
  printf("         tdistat : tditracebuffer(s) status.\n");
  printf("\nUsage: tdi -t\n");
  printf("         tditest : create small set of tracepoints.\n");
}

static int tdidump(int argc, char *argv[]);
static int tdistat(int argc, char *argv[]);
static int tditest(int argc, char *argv[]);

/******************************************************************************/
int main(int argc, char *argv[]) {
  if (argc > 1 && (strcmp(argv[1], "-d") == 0)) {
    tdidump(argc - 1, &argv[1]);
    return 0;
  }

  else if (argc > 1 && (strcmp(argv[1], "-s") == 0)) {
    tdistat(argc - 1, &argv[1]);
    return 0;
  }

  else if (argc > 1 && (strcmp(argv[1], "-t") == 0)) {
    tditest(argc - 1, &argv[1]);
    return 0;
  }

  else if (argc == 1) {
    tdistat(argc, argv);
    return 0;
  }

  usage();
  return -1;
}

/******************************************************************************/
static int tdidump(int argc, char *argv[]) {
  void *handle;
  void (*tditrace_exit)(int argc, char *argv[]);

  handle = dlopen("libtdi.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    exit(EXIT_FAILURE);
  }

  dlerror(); /* Clear any existing error */

  tditrace_exit =
      (void (*)(int argc, char *argv[]))dlsym(handle, "tditrace_exit");

  tditrace_exit(argc, argv);

  return 0;
}

/******************************************************************************/
static int tdistat(int argc, char *argv[]) {
  DIR *dp;
  struct dirent *ep;

  dp = opendir("/tmp/");
  if (dp != NULL) {
    while ((ep = readdir(dp))) {
      if (strncmp(ep->d_name, "tditracebuffer@", 15) == 0) {
        FILE *file;
        char filename[128];
        char *bufmmapped;

        sprintf(filename, "/tmp/%s", ep->d_name);

        if ((file = fopen(filename, "r")) != NULL) {
          /* /tmp/.tditracebuffer-xxx-xxx */

          struct stat st;
          stat(filename, &st);

          fprintf(stderr, "\"%s\" (%lluMB)\n", filename,
                  (unsigned long long)st.st_size / (1024 * 1024));

          bufmmapped = (char *)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE,
                                    fileno(file), 0);

          // token should hold "TDITRACEBUFFER"
          if (strncmp("TDITRACE", bufmmapped, 8) != 0) {
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

            unsigned int *ptr = (unsigned int *)bufmmapped;

            int i = 0;
            ptr += 6;
            while (*ptr) {
              ptr += *ptr & 0xffff;
              i++;
            }
            fprintf(stdout, "%lld%% (#%d, %dB)\n",
                    ((char *)ptr - bufmmapped) * 100LL / (st.st_size) + 1, i,
                    (char *)ptr - bufmmapped);
          }

          munmap(bufmmapped, st.st_size);

          fclose(file);
        }
      }
    }
  }
  return 0;
}

static int tditest(int argc, char *argv[]) {
  int i;
  void *handle;
  void (*tditrace)(const char *format, ...);

  setenv("NOSKIPINIT", "1", -1);

  handle = dlopen("libtdi.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    exit(EXIT_FAILURE);
  }

  dlerror(); /* Clear any existing error */

  tditrace =
      (void (*)(const char *format, ...))dlsym(handle, "tditrace");

  for (i = 0; i < 10; i++) {

    tditrace("@T+task1 %d", i);
    usleep(10 * 1024);
    tditrace("@T-task1");

    tditrace("@I+isr1 %x", i);
    usleep(20 * 1024);
    tditrace("@I-isr1");

    tditrace("note %d");
    usleep(1 * 1024);

    tditrace("@S+semaphore1 %d");
    usleep(2 * 1024);

    tditrace("@E+event1 %d,%d", i, i);
    usleep(3 * 1024);

    tditrace("queue1~%d", i);

    tditrace("@A+agent1 %s,%d", "number", i);
    usleep(30 * 1024);
    tditrace("@A-agent1");

    tditrace("@A+agent2 %s,%d", "number", i);
    usleep(30 * 1024);
    tditrace("@A-agent2");
  }
  return 0;
}
