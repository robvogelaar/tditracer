
#include <dlfcn.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// extern "C" void tditrace(const char* format, ...)  __attribute__((weak));

void (*tditrace)(const char* format,
                 ...) = (void (*)(const char* format, ...))dlsym(RTLD_DEFAULT,
                                                                 "tditrace");

// char *socket_path = "./socket";
char* socket_path = (char*)"\0hidden";

void execute(const char* msg);

int main(int argc, char* argv[]) {
  struct sockaddr_un addr;
  char buf[100];
  int fd, cl, rc;

  tditrace("@T+action %s", argv[0]);
  tditrace("@T-action");

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
#if 1
      printf("simserver:\"%s\"\n", buf);
#endif

      if (strcmp(buf, "exit") == 0) {
        tditrace("@T+action exit");
        tditrace("@T-action exit");
        write(cl, "done", 4);
        close(cl);
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

  fprintf(stdout, "+execute[%s]\n", msg);

  static unsigned int mmap1_bytes;
  static char* mmap1;
  static unsigned int malloc1_bytes;
  static char* malloc1;

  /*
   * marker
   */
  if (strcmp(msg, "mark") == 0) {
    static int mark_seen = 0;
    static int mark_color = 0;

    if (!mark_seen) {
      mark_seen = 1;
      tditrace("@%d+mark", mark_color);
    } else {
      tditrace("@%d-mark", mark_color);
      mark_color++;
      mark_color &= 7;
      tditrace("@%d+mark", mark_color);
    }
  }

  /*
   * mmap
   */
  if ((n = sscanf(msg, "mmap %s", s)) >= 1) {
    mmap1_bytes =
        atoi(s) * (strchr(s, 'M') ? 1024 * 1024 : strchr(s, 'K') ? 1024 : 1);

    fprintf(stdout, "mmap %d bytes\n", mmap1_bytes);

    tditrace("@T+action mmap %s", s);
    mmap1 = (char*)mmap(NULL, mmap1_bytes, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS
                        /* | MAP_POPULATE */
                        ,
                        -1, 0);
    tditrace("@T-action");

    if ((int)mmap1 != -1) {
      tditrace("@T+action memset %s", s);
      memset(mmap1, 0, mmap1_bytes);
      tditrace("@T-action");
    }
  }

  /*
   * munmap
   */
  if ((strcmp(msg, "munmap")) == 0) {
    fprintf(stdout, "munmap %d bytes\n", mmap1_bytes);
    tditrace("@T+action munmap %s", s);
    munmap(mmap1, mmap1_bytes);
    tditrace("@T-action");
  }

  if ((n = sscanf(msg, "malloc %s", s)) >= 1) {
    malloc1_bytes =
        atoi(s) * (strchr(s, 'M') ? 1024 * 1024 : strchr(s, 'K') ? 1024 : 1);

    fprintf(stdout, "malloc %d bytes\n", malloc1_bytes);
    malloc1 = (char*)malloc(malloc1_bytes);
    memset(malloc1, 0, malloc1_bytes);
  }

  if ((strcmp(msg, "free")) == 0) {
    fprintf(stdout, "free %d bytes\n", malloc1_bytes);
    free(malloc1);
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
    printf("%d prime numbers under %d\n", primes, primesunder);
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

    tditrace("@T+action dlopen(%s)", lname);
    handle = dlopen(lname, RTLD_LAZY);
    tditrace("@T-action");

    if (!handle) {
      fprintf(stderr, "%s\n", dlerror());
    }

    f = (void (*)(int, int))dlsym(handle, fname);

    if (f) {
      fprintf(stdout, "%s %s %d %d\n", lname, fname, fparam1, fparam2);
      tditrace("@T+action %s %d %d", fname, fparam1, fparam2);
      f(fparam1, fparam2);
      tditrace("@T-action");
    }
  }
  fprintf(stdout, "-execute[%s]\n", msg);
}
