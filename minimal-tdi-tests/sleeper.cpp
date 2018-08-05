#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <unistd.h>

#include <byteswap.h>

#if 0
extern "C" void tditrace(const char* format, ...) __attribute__((weak));
#endif

static void *thread_task(void *param) {
  // time_t t;
  // int *p = (int *)param;
  //  srand((unsigned)time(&t));
  //  usleep((rand() % 100) * 1000);

  while (1) {

#if 0
    long long int j = (long long int)((rand() % 100) * 10000LL);
    while (j) {
      j--;
    };
#endif

#if 0
    long long int j = (long long int)(400 * 10000LL);
    while (j) {
      j--;
    };
    usleep(50 * 1000);
#endif

    #if 0
    if (tditrace) tditrace("TICK");
    #endif

    usleep(1 * 1000 * 1000);
  }

  return NULL;
}

#define NR_THREADS_TASK 1

static pthread_t thread_id_task[NR_THREADS_TASK];
static int param_task[NR_THREADS_TASK];

int main(int argc, char *argv[]) {
  int i;

  for (i = 0; i < NR_THREADS_TASK; i++) {
    param_task[i] = i;
    pthread_create(&thread_id_task[i], NULL, thread_task, &param_task[i]);
  }

  for (i = 0; i < NR_THREADS_TASK; i++) {
    pthread_join(thread_id_task[i], NULL);
  }
}
