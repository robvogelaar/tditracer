#include <dirent.h>
#include <limits.h>
#include <linux/futex.h>
#include <malloc.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syscall.h>
#include <time.h>
#include <unistd.h>

extern "C" void tditrace(const char *format, ...) __attribute__((weak));

struct simplefu_semaphore {
  int avail;
  int waiters;
};

typedef struct simplefu_semaphore *simplefu;

struct simplefu_mutex {
  struct simplefu_semaphore sema;
};

typedef struct simplefu_mutex *simplemu;

void simplefu_down(simplefu who);
void simplefu_up(simplefu who);

void simplefu_mutex_init(simplemu mx);
void simplefu_mutex_lock(simplemu mx);
void simplefu_mutex_unlock(simplemu mx);

#define SIMPLEFU_MUTEX_INITIALIZER \
  {                                \
    { 1, 0 }                       \
  }

void simplefu_mutex_init(simplemu mx) {
  mx->sema.avail = 1;
  mx->sema.waiters = 0;
}

void simplefu_mutex_lock(simplemu mx) { simplefu_down(&mx->sema); }

void simplefu_mutex_unlock(simplemu mx) { simplefu_up(&mx->sema); }

void simplefu_down(simplefu who) {
  int val;
  do {
    val = who->avail;
    if (val > 0 && __sync_bool_compare_and_swap(&who->avail, val, val - 1))
      return;
    __sync_fetch_and_add(&who->waiters, 1);
    syscall(__NR_futex, &who->avail, FUTEX_WAIT, val, NULL, 0, 0);
    __sync_fetch_and_sub(&who->waiters, 1);
  } while (1);
}

void simplefu_up(simplefu who) {
  int nval = __sync_add_and_fetch(&who->avail, 1);
  if (who->waiters > 0) {
    syscall(__NR_futex, &who->avail, FUTEX_WAKE, nval, NULL, 0, 0);
  }
}

void run_1(void) {
  struct timeval mytimeval;
  struct timespec mytimespec;
  int i;

  if (tditrace != NULL) tditrace("@T+CLOCK_REALTIMEx1000000");
  for (i = 0; i < 1000000; i++) {
    clock_gettime(CLOCK_REALTIME, &mytimespec);
  }
  if (tditrace != NULL) tditrace("@T-CLOCK_REALTIMEx1000000");

  if (tditrace != NULL) tditrace("@T+CLOCK_MONOTONICx1000000");
  for (i = 0; i < 1000000; i++) {
    clock_gettime(CLOCK_MONOTONIC, &mytimespec);
  }
  if (tditrace != NULL) tditrace("@T-CLOCK_MONOTONICx1000000");

  if (tditrace != NULL) tditrace("@T+CLOCK_PROCESS_CPUTIME_IDx1000000");
  for (i = 0; i < 1000000; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mytimespec);
  }
  if (tditrace != NULL) tditrace("@T-CLOCK_PROCESS_CPUTIME_IDx1000000");

  if (tditrace != NULL) tditrace("@T+CLOCK_THREAD_CPUTIME_IDx1000000");
  for (i = 0; i < 1000000; i++) {
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &mytimespec);
  }
  if (tditrace != NULL) tditrace("@T-CLOCK_THREAD_CPUTIME_IDx1000000");

  if (tditrace != NULL) tditrace("@T+GETTIMEOFDAYx1000000");
  for (i = 0; i < 1000000; i++) {
    gettimeofday(&mytimeval, 0);
  }
  if (tditrace != NULL) tditrace("@T-GETTIMEOFDAYx1000000");
}

void run_2(void) {
  struct simplefu_mutex myMutex;
  int i;

  if (tditrace != NULL) tditrace("@T+mutexx1000000");
  for (i = 0; i < 1000000; i++) {
    simplefu_mutex_lock(&myMutex);
    simplefu_mutex_unlock(&myMutex);
  }
  if (tditrace != NULL) tditrace("@T-mutexx1000000");
}

void run_3(void) {
  int i;
  if (tditrace != NULL) tditrace("@T+tditrace_10x1000");
  for (i = 0; i < 1000; i++) {
    if (tditrace != NULL) tditrace("1234567890");
  }
  if (tditrace != NULL) tditrace("@T-tditrace_10x1000");

  if (tditrace != NULL) tditrace("@T+tditrace_50x1000");
  for (i = 0; i < 1000; i++) {
    if (tditrace != NULL)
      tditrace("12345678901234567890123456789012345678901234567890");
  }
  if (tditrace != NULL) tditrace("@T-tditrace_50x1000");

  if (tditrace != NULL) tditrace("@T+tditrace_200x1000");
  for (i = 0; i < 1000; i++) {
    if (tditrace != NULL)
      tditrace(
          "12345678901234567890123456789012345678901234567890123456789012345678"
          "90123456789012345678901234567890123456789012345678901234567890123456"
          "7890123456789012345678901234567890123456789012345678901234567890");
  }
  if (tditrace != NULL) tditrace("@T-tditrace_200x1000");
}

void run_4(void) {
  int i;
  if (tditrace != NULL) tditrace("@T+syscall(SYS_gettid)x1000000");
  for (i = 0; i < 1000000; i++) {
    unsigned int num = (int)syscall(SYS_gettid);
  }
  if (tditrace != NULL) tditrace("@T-syscall(SYS_gettid)x1000000");
}

void run_5(void) {
  int i;
  struct timeval mytimeval;
  struct timespec mytimespec;

  printf("start\n");

#if 1

  time_t t;
  t = time(NULL);
  printf("Local time and date: %s\n", asctime(localtime(&t)));
  printf("UTC time and date: %s\n", asctime(gmtime(&t)));

  struct timespec mytime;
  mytime.tv_nsec = 0;
  mytime.tv_sec = 0;
  printf("0 -> time and date: %s\n", ctime((const time_t *)&mytime));
  printf("0 -> UTC time and date: %s\n",
         asctime(gmtime((const time_t *)&mytime)));

#endif

  if (tditrace)
    printf("tditrace\n");
  else
    printf("no tditrace\n");
  if (tditrace)
    printf("tditrace\n");
  else
    printf("no tditrace\n");

  for (i = 0; i < 25; i++) {
    printf("%d\n", i);

    // will all appear in the same "HELLO" , "NOTES"-timeline
    if (tditrace) tditrace("HELLO");
    printf("HELLO\n");

    if (tditrace) tditrace("HELLO %d", i);
    printf("HELLO %d\n", i);

    if (tditrace) tditrace("HELLO %d %s %s", i, "yes", "no");
    printf("HELLO %d %s %s\n", i, "yes", "no");

    printf("malloc(1 * 1024)\n");

    char *d = (char *)malloc(1 * 1024);
    d[0] = 0;

    // create separate "HELLO1", "HELLO2" ,..   "NOTES"-timelines
    if (tditrace) tditrace("HELLO%d", i);

    // variable~value creates a QUEUES-timeline
    if (tditrace) tditrace("i~%d", 12);
    usleep(10000);

    if (tditrace) tditrace("i~%d", 16);
    usleep(10000);

    if (tditrace) tditrace("i~%d", 8);
    usleep(100000);

    // create "TASKS"-timeline, using the @T+ and #T- identifier
    if (tditrace) tditrace("@T+HELLO");

    usleep(20000);

    if (tditrace) tditrace("@I+interrupt begin");

    usleep(10000);

    if (tditrace) tditrace("@I-interrupt end");

    usleep(20000);

    if (tditrace) tditrace("@T-HELLO");

    if (tditrace) tditrace("@T+TEST");

    if (tditrace) tditrace("@T+T0");

    gettimeofday(&mytimeval, 0);
    gettimeofday(&mytimeval, 0);
    gettimeofday(&mytimeval, 0);
    gettimeofday(&mytimeval, 0);
    gettimeofday(&mytimeval, 0);
    gettimeofday(&mytimeval, 0);
    gettimeofday(&mytimeval, 0);
    gettimeofday(&mytimeval, 0);
    gettimeofday(&mytimeval, 0);
    gettimeofday(&mytimeval, 0);

    if (tditrace) tditrace("@T-T0");

    if (tditrace) tditrace("@T+T1");

    clock_gettime(CLOCK_MONOTONIC, &mytimespec);
    clock_gettime(CLOCK_MONOTONIC, &mytimespec);
    clock_gettime(CLOCK_MONOTONIC, &mytimespec);
    clock_gettime(CLOCK_MONOTONIC, &mytimespec);
    clock_gettime(CLOCK_MONOTONIC, &mytimespec);
    clock_gettime(CLOCK_MONOTONIC, &mytimespec);
    clock_gettime(CLOCK_MONOTONIC, &mytimespec);
    clock_gettime(CLOCK_MONOTONIC, &mytimespec);
    clock_gettime(CLOCK_MONOTONIC, &mytimespec);

    if (tditrace) tditrace("@T-T1");

    if (tditrace)
      tditrace(
          "@T+T2 "
          "012345678901234567890123456789012345678901234567890123456"
          "78901234567890123456789");
    if (tditrace) tditrace("@T-T2");

    if (tditrace)
      tditrace("@T+T3 %s",
               "012345678901234567890123456789012345678901234"
               "567890123456789012345678901234567890123456789"
               "012345678901234567890123456789012345678901234"
               "5678901234567890123456789");
    if (tditrace) tditrace("@T-T3");

    if (tditrace) tditrace("@T+T5 %d %u %x %p", 0, 0, 0, 0);
    if (tditrace) tditrace("@T-T5");

    if (tditrace)
      tditrace("@T+T5 %d %u %x %p", 0xffffffff, 0xffffffff, 0xffffffff,
               0xffffffff);
    if (tditrace) tditrace("@T-T5");

    if (tditrace) tditrace("@T-TEST");

    usleep(25000);

    // create "EVENTS"-timeline, using the @E+ identifier
    if (tditrace) tditrace("@E+HELLO");

    usleep(25000);

    // create "SEMAPHORES"-timeline, using the @S+ identifier
    if (tditrace) tditrace("@S+SEMA semaphore test");

    usleep(10000);

    if (tditrace) tditrace("@A+Agent agent test");

    usleep(40000);

    if (tditrace) tditrace("@A-Agent");

    usleep(50000);
  }

  usleep(100000);
  if (tditrace) tditrace("END");

  printf("stop\n");
}

#if 0
void foo(char* p, int i) {

    int b;

    unsigned int ra = 0;
    unsigned int sp = 0;

#ifdef __mips__
    asm volatile("move %0, $ra" : "=r"(ra));
    asm volatile("move %0, $sp" : "=r"(sp));
#endif

    printf("foo, return address : %p, ra : %p, sp : %p\n", __builtin_return_address(0), ra, sp);

    printf("%d\n", i);


    if (i < 3) {
        foo(p, ++i);
    } else {
        b = (int)realloc(p, i * 5 * 1024);
        printf("b=0x%08x (0x%08x)\n", b, &b);
    }

}
#endif

#if 0
int main(int argc, char **argv) {

    sleep(3);

    char *p = malloc(1 * 1024);

    foo(p, 0);

    sleep(3);

    printf("stop\n");
    return 0;
}
#endif

#if 0
int main(int argc, char **argv) {
  if (tditrace) tditrace("START");

  usleep(2 * 1000 * 1000);

  if (tditrace) tditrace("CHECK");

  int i;
  for (i = 0; i < 10; i++) {
    // int *p1 = (int*)malloc(5 * sizeof(int));
    // int *p1 = new int(5);
    int *p1 = new int[420];

    printf("p1=%d(%x)\n", *p1, p1);
  }

  if (tditrace) tditrace("STOP");

  return 0;
}
#endif

#if 1
int main(int argc, char **argv) {
 run_1();
 run_2();
 run_3();
 run_4();
}

#endif