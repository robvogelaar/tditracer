
#include <dlfcn.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// extern "C" void tditrace(const char* format, ...)  __attribute__((weak));

static void (*tditrace)(const char* format, ...) = NULL;

#define TDITRACE(format, ...) \
  if (NULL != tditrace) tditrace(format, ##__VA_ARGS__)

// char *socket_path = "./socket";
char* socket_path = (char*)"\0hidden";

void execute(const char* msg);

int main(int argc, char* argv[]) {
  struct sockaddr_un addr;
  char buf[100];
  int fd, cl, rc;

  tditrace = (void (*)(const char* format, ...))dlsym(RTLD_DEFAULT, "tditrace");

  TDITRACE("@T+action %s", argv[0]);
  TDITRACE("@T-action");

  if (argc > 1) socket_path = argv[1];

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  if (*socket_path == '\0') {
    *addr.sun_path = '\0';
    strncpy(addr.sun_path + 1, socket_path + 1, sizeof(addr.sun_path) - 2);
  } else {
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    unlink(socket_path);
  }

  if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind error");
    exit(-1);
  }

  if (listen(fd, 5) == -1) {
    perror("listen error");
    exit(-1);
  }

  while (1) {
    if ((cl = accept(fd, NULL, NULL)) == -1) {
      perror("accept error");
      continue;
    }

    while ((rc = read(cl, buf, sizeof(buf))) > 0) {
      buf[rc] = 0;
#if 0
      printf("simserver:\"%s\"\n", buf);
#endif

      if (strcmp(buf, "exit") == 0) {
        TDITRACE("@T+action exit");
        TDITRACE("@T-action exit");
        write(cl, "done", 4);
        close(cl);
        fprintf(stdout, "+%-5d \"exit\"\n", getpid());
        exit(0);
      } else {
        execute(buf);
        write(cl, "done", 4);
      }
    }
    if (rc == -1) {
      perror("read");
      exit(-1);
    } else if (rc == 0) {
#if 0
      printf("EOF\n");
#endif
      close(cl);
    }
  }

  return 0;
}

void execute(const char* msg) {
  unsigned int n, i;
  char s[16];

  fprintf(stdout, "%-5d \"%s\"\n", getpid(), msg);

  static unsigned int mmaps_bytes[8];
  static char* mmaps[8];
  static unsigned int mallocs_bytes[8];
  static char* mallocs[8];

  static int mmap_counter = 0;
  static int malloc_counter = 0;

  int mmap_id;

  /*
   * marker
   */
  if (strcmp(msg, "mark") == 0) {
    TDITRACE("%K", "mark");
  }

  /*
   * mmap
   */
  if ((n = sscanf(msg, "mmap %s", s)) >= 1) {
    mmaps_bytes[mmap_counter] =
        atoi(s) * (strchr(s, 'M') ? 1024 * 1024 : strchr(s, 'K') ? 1024 : 1);
#if 0
    fprintf(stdout, "mmap %d bytes\n", mmap1_bytes);
#endif
    TDITRACE("@T+action mmap %s", s);
    mmaps[mmap_counter] =
        (char*)mmap(NULL, mmaps_bytes[mmap_counter], PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS
                    /* | MAP_POPULATE */
                    ,
                    -1, 0);
    TDITRACE("@T-action");

    if ((int)mmaps[mmap_counter] != -1) {
      TDITRACE("@T+action memset %s", s);
      memset(mmaps[mmap_counter], 0, mmaps_bytes[mmap_counter]);
      TDITRACE("@T-action");
    }

    mmap_counter++;
  }

  /*
   * memset
   */
  if ((n = sscanf(msg, "memset %u", &mmap_id)) >= 1) {
#if 0
    fprintf(stdout, "memset %u\n", mmap_id);
#endif
    TDITRACE("@T+action memset %s", s);
    memset(mmaps[mmap_id], 0, mmaps_bytes[mmap_id]);
    TDITRACE("@T-action");
  }

  /*
   * munmap
   */
  if ((n = sscanf(msg, "munmap %u", &mmap_id)) >= 1) {
#if 0
    fprintf(stdout, "munmap %u\n", mmap_id);
#endif
    TDITRACE("@T+action munmap %u", mmap_id);
    munmap(mmaps[mmap_id], mmaps_bytes[mmap_id]);
    TDITRACE("@T-action");
  }

  /*
   * malloc
   */
  if ((n = sscanf(msg, "malloc %s", s)) >= 1) {
    mallocs_bytes[malloc_counter] =
        atoi(s) * (strchr(s, 'M') ? 1024 * 1024 : strchr(s, 'K') ? 1024 : 1);

#if 0
    fprintf(stdout, "malloc %d bytes\n", malloc1_bytes);
#endif
    mallocs[malloc_counter] = (char*)malloc(mallocs_bytes[malloc_counter]);
    memset(mallocs[malloc_counter], 0, mallocs_bytes[malloc_counter]);
  }

  /*
   * free
   */
  int malloc_id;
  if ((n = sscanf(msg, "free %u", &malloc_id)) >= 1) {
#if 0
    fprintf(stdout, "free %u\n", malloc_id);
#endif
    free(mallocs[malloc_id]);
  }

  /*
   * primesunder
   */
  int primesunder;
  if ((n = sscanf(msg, "primes %u", &primesunder)) >= 1) {
    int i, num = 1, primes = 0;

    while (num <= primesunder) {
      i = 2;
      while (i <= num) {
        if (num % i == 0) break;
        i++;
      }
      if (i == num) primes++;
      num++;
    }
#if 0
    printf("%d prime numbers under %d\n", primes, primesunder);
#endif
  }

  /*
   * code
   */
  char lname[128];
  char fname[128];
  int fparam1;
  int fparam2;
  if ((n = sscanf(msg, "code %s %s %d %d", lname, fname, &fparam1, &fparam2)) >=
      1) {
    void* handle;
    void (*f)(int, int) = NULL;

    TDITRACE("@T+action dlopen(%s)", lname);
    handle = dlopen(lname, RTLD_LAZY);
    TDITRACE("@T-action");

    if (!handle) {
      fprintf(stderr, "%s\n", dlerror());
    }

    f = (void (*)(int, int))dlsym(handle, fname);

    if (f) {
#if 0
      fprintf(stdout, "%s %s %d %d\n", lname, fname, fparam1, fparam2);
#endif
      TDITRACE("@T+action %s %d %d", fname, fparam1, fparam2);
      f(fparam1, fparam2);
      TDITRACE("@T-action");
    }
  }
#if 0
  fprintf(stdout, "-execute[%s]\n", msg);
#endif
}
