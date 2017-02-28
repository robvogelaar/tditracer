#define VERSION "0.1"

#include <dirent.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

extern "C" {
#include "tdim.h"
}

void usage(void) {
  printf("tdim v%s (%s %s)\n", VERSION, __DATE__, __TIME__);
  printf("\nUsage: tdim -s\n");
  printf("         tdistat : report tditracebuffer(s) status.\n");
  printf("\n       tdim -d [tracebuffer]\n");
  printf("         tdidump : convert tditracebuffer(s) to tdi format.\n");
  printf("\n       tdim -t\n");
  printf(
      "         tditest : run an internal test that creates a tracebuffer with "
      "small set of tracepoints.\n");
  printf("\n       tdim -m <message>\n");
  printf("         tdimessage: send a message to the tditracer(s)\n");
  printf("         'rewind' = rewind the tditracebuffer(s)\n");
  printf("\n       tdim -p [pid]\n");
  printf("         tdiproc : report procfs data.\n");
  printf("\n       tdim -h\n");
  printf("         display this help message.\n\n");
}

static int tdidump(int argc, char *argv[]);
static int tdistat(int argc, char *argv[]);
static int tditest(int argc, char *argv[]);
static int tdimessage(int argc, char *argv[]);
static int tdiproc(int argc, char *argv[]);
static int tdipipe(int argc, char *argv[]);

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

  else if (argc > 1 && (strcmp(argv[1], "-m") == 0)) {
    tdimessage(argc - 1, &argv[1]);
    return 0;
  }

  else if (argc > 1 && (strcmp(argv[1], "-p") == 0)) {
    tdiproc(argc - 1, &argv[1]);
    return 0;
  }

  else if (argc > 1 && (strcmp(argv[1], "-h") == 0)) {
    usage();
    return -1;
  }

  else {
    tdipipe(argc, argv);
    return 0;
  }

  return -1;
}

/******************************************************************************/
static int tdidump(int argc, char *argv[]) {
  void *handle;
  void (*tditrace_exit)(int argc, char *argv[]);

  handle = dlopen("libtdim.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    exit(EXIT_FAILURE);
  }

  dlerror(); /* Clear any existing error */

  tditrace_exit =
      (void (*)(int argc, char *argv[]))dlsym(handle, "tditrace_exit");

  tditrace_exit(argc, argv);

  dlclose(handle);

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
          // /tmp/tditracebuffer@proc@pid

          struct stat st;
          stat(filename, &st);

          fprintf(stderr, "\"%s\", (%lluMB), ", filename,
                  (unsigned long long)st.st_size / (1024 * 1024));

          fflush(stderr);

          bufmmapped = (char *)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE,
                                    fileno(file), 0);

          // token should hold "TDITRACE"
          if (strncmp("TDITRACE", bufmmapped, 8) != 0) {
            fprintf(stderr,
                    "invalid "
                    "tracebuffer, skipping\n");
            break;
          }

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

          munmap(bufmmapped, st.st_size);
          fclose(file);
        }
      }
    }
  }
  return 0;
}

void (*tditrace)(const char *format, ...);

//#pragma GCC push_options
//#pragma GCC optimize("O0")

static void *thread_task(void *param) {
  time_t t;
  int *p = (int *)param;
  srand((unsigned)time(&t));

  int loops = 100;
  while (loops--) {
    tditrace("%m%n", *p, 10000);

    tditrace("@%d+task%d", *p & 7, *p);

    usleep((rand() % 100) * 1000);

#if 0
    long long int j = (long long int)((rand() % 100) * 10000LL);
    while (j) {
      j--;
    };
#endif

    tditrace("@%d-task%d", *p & 7, *p);

    usleep((rand() % 100) * 1000);
  }
  return NULL;
}
//#pragma GCC pop_options

#define I(n) (p[n] == ' ' ? 0 : p[n] - '0')

#define I5(n) \
  I(n) * 10000 + I(n + 1) * 1000 + I(n + 2) * 100 + I(n + 3) * 10 + I(n + 4);

#if 0
    int val = 0;
    while( *str ) {
        val = val*10 + (*str++ - '0');
    }
#endif

static int tditest(int argc, char *argv[]) {
  int i;
  void *handle;

#if 0
  extern char **environ;
  char **p = environ;
  while (*p) {
    fprintf(stderr, "[%s]\n", *p);
    p++;
  }
#endif

  setenv("NOSKIPINIT", "1", -1);

  handle = dlopen("libtdim.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    exit(EXIT_FAILURE);
  }

  dlerror(); /* Clear any existing error */

  tditrace = (void (*)(const char *format, ...))dlsym(handle, "tditrace");

#define I(n) (p[n] == ' ' ? 0 : p[n] - '0')

#define I2(n) (p[n] == ' ' ? 0 : p[n] - '0')

#define I5(n) \
  I(n) * 10000 + I(n + 1) * 1000 + I(n + 2) * 100 + I(n + 3) * 10 + I(n + 4);

#define I5r(n) \
  I(n) + I(n - 1) * 10 + I(n - 2) * 100 + I(n - 3) * 1000 + I(n - 4) * 10000;

#define II(val, idx)                             \
  if (p[idx] != ' ') {                           \
    val += (p[idx] - '0');                       \
    if (p[idx - 1] != ' ') {                     \
      val += ((p[idx - 1] - '0') * 10);          \
      if (p[idx - 2] != ' ') {                   \
        val += ((p[idx - 2] - '0') * 100);       \
        if (p[idx - 3] != ' ') {                 \
          val += ((p[idx - 3] - '0') * 1000);    \
          if (p[idx - 4] != ' ') {               \
            val += ((p[idx - 4] - '0') * 10000); \
          }                                      \
        }                                        \
      }                                          \
    }                                            \
  }

#if 0

  for (int j = 0; j < 10; j++) {
#if 0
    char p[255] =
        "Rss:                2140 kB\r"
        "Referenced:         1076 kB\r"
        "Swap:                  0 kB";
#else
    char p[255] =
        "Rss:                   4 kB\r"
        "Referenced:            8 kB\r"
        "Swap:                  0 kB";
#endif

    int rss1 = 0;
    int ref1 = 0;
    int swap1 = 0;

    int rss2 = 0;
    int ref2 = 0;
    int swap2 = 0;

    tditrace("@T+action1");
    for (i = 0; (unsigned int)i < 100000; i++) {
      rss1 += I5(19);
      ref1 += I5(1 * 28 + 19);
      swap1 += I5(2 * 28 + 19);
    }
    tditrace("@T-action1");

    tditrace("@T+action2");
    for (i = 0; (unsigned int)i < 100000; i++) {
      II(rss2, 19 + 4);
      II(ref2, 1 * 28 + 19 + 4);
      II(swap2, 2 * 28 + 19 + 4);
    }
    tditrace("@T-action2");

    fprintf(stderr, "rss1=%d,ref1=%d,swap1=%d\n", rss1, ref1, swap1);
    fprintf(stderr, "rss2=%d,ref2=%d,swap2=%d\n", rss2, ref2, swap2);
  }

#endif

#if 0
  for (i = 0; (unsigned int)i < 10; i++) {
    tditrace("%K", "general");
    tditrace("%K", "general2");
    tditrace("%K", "general20");
    tditrace("%K", "general200");
    tditrace("%K", "general2000");
    tditrace("%K", "general20000");

    usleep(50 * 1000);

  }
#endif

#if 0
  for (i = 0; (unsigned int)i < 10; i++) {
    tditrace("%P", 1039+i, "pid1039");

    usleep(100 * 1000);
  }
#endif

#if 0
  for (i = 0; (unsigned int)i < 10; i++) {

    tditrace("%m%n%n", i,i,10+i);

    usleep(100 * 1000);
  }
#endif

#if 0
  for (i = 0; i < 10; i++) {

    unsigned int ints[] = {1,2,3,4,5,6,7,8,9,10};

    tditrace("%9", ints);
    usleep(10 * 1000);

  }
#endif

#if 0
  for (i = 0; i < 10; i++) {

    unsigned int ints[] = {1,2,3,4,5,6,7,8,9,10};

    tditrace("%C", ints);
    tditrace("%M", ints);
    //tditrace("%D", ints);
    //tditrace("%N", ints);
    usleep(10 * 1000);

  }
#endif

#if 0
  int value = 0;
  int values[] = {0,  0, 0, 0, 10, 0, 0,  0,  0,  0,  0, 0, 0,
                  30, 0, 0, 0, 0,  0, 40, 0,  0,  0,  0, 0, 0,
                  0,  0, 8, 0, 0,  0, 9,  10, 11, 12, 0, 0, 0};

  for (i = 0; (unsigned int)i < (sizeof(values) / sizeof(int)); i++) {
    value += values[i];

    OUT(value, "VALUE#%u", (unsigned int)value)
    OUT1(value1, "%s#%u", "VALUE1", (unsigned int)value)
    OUT2(value2, "%s#%u", "VALUE2", (unsigned int)value)
    OUT3(value3, "%s#%u", "VALUE3", (unsigned int)value)

    usleep(100 * 1000);
  }
#endif
#if 0
  for (i = 0; i < 10; i++) {
    tditrace("@T+task1 %d", i);
    usleep(10 * 1000);
    tditrace("@T-task1");

    tditrace("@I+isr1 %x", i);
    usleep(20 * 1000);
    tditrace("@I-isr1");

    tditrace("note %d");
    usleep(1 * 1000);

    tditrace("@S+semaphore1 %d");
    usleep(2 * 1000);

    tditrace("@E+event1 %d,%d", i, i);
    usleep(3 * 1000);

    tditrace("queue1~%d", i);

    tditrace("@A+agent1 %s,%d", "number", i);
    usleep(30 * 1000);
    tditrace("@A-agent1");

    tditrace("@A+agent2 %s,%d", "number", i);
    usleep(30 * 1000);
    tditrace("@A-agent2");
  }
#endif

#if 1
#define NR_THREADS_TASK 10
  static pthread_t thread_id_task[NR_THREADS_TASK];
  static int param_task[NR_THREADS_TASK];

  for (i = 0; i < NR_THREADS_TASK; i++) {
    param_task[i] = i;
    pthread_create(&thread_id_task[i], NULL, thread_task, &param_task[i]);
  }

  for (i = 0; i < NR_THREADS_TASK; i++) {
    pthread_join(thread_id_task[i], NULL);
  }

#endif

  dlclose(handle);
  return 0;
}

int send(const char *msg) {
  DIR *dp;
  struct dirent *ep;
  char socket_path[128];

  dp = opendir("/tmp/");
  if (dp != NULL) {
    while ((ep = readdir(dp))) {
      if (strncmp(ep->d_name, "tditracesocket@", 15) == 0) {
        fprintf(stdout, "\"%s\"\n", ep->d_name);

        sprintf(socket_path, "/tmp/%s", ep->d_name);

        struct sockaddr_un addr;
        int fd;

        if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
          fprintf(stderr, "socket error\n");
          continue;
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
          fprintf(stderr, "connect error\n");
          continue;
        }

        int rc = strlen(msg);
        if (write(fd, msg, rc) != rc) {
          if (rc > 0)
            fprintf(stderr, "partial write");
          else {
            perror("write error");
          }
        }

        char buf[100];
        rc = read(fd, buf, sizeof(buf));
        if (rc == -1) {
          fprintf(stdout, "[%d]\n", rc);
          perror("read");
        } else if (rc == 0) {
          fprintf(stdout, "[%d]\n", rc);
        } else {
          buf[rc] = 0;
          fprintf(stdout, "received:\"%s\"\n", buf);
        }

        close(fd);
      }
    }
    closedir(dp);
  }

  return 0;
}

static int tdimessage(int argc, char *argv[]) {
  if (argc > 1) {
    fprintf(stdout, "sending:\"%s\"\n", argv[1]);
    send(argv[1]);
  } else {
    fprintf(stderr, "no message provided\n");
  }
  return 0;
}

static int tdiproc(int argc, char *argv[]) {
  void *handle;

  setenv("NOSKIPINIT", "1", -1);

  handle = dlopen("libtdim.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    exit(EXIT_FAILURE);
  }

  dlerror(); /* Clear any existing error */

  tditrace = (void (*)(const char *format, ...))dlsym(handle, "tditrace");

  pfntdiprocvmstat tdiprocvmstat =
      (pfntdiprocvmstat)dlsym(handle, "tdiprocvmstat");
  struct tdistructprocvmstat vmstat;
  if (tdiprocvmstat) {
    tditrace("@A+tdiprocvmstat");
    tdiprocvmstat(&vmstat);
    tditrace("@A-tdiprocvmstat");

    fprintf(stdout,
            "vmstat          pswpin:%d, pswpout:%d, pgpgin:%d, pgpgout:%d, "
            "pgfault:%d, "
            "pgmajfault:%d\n",
            vmstat.pswpin, vmstat.pswpout, vmstat.pgpgin, vmstat.pgpgout,
            vmstat.pgfault, vmstat.pgmajfault);
  }

  pfntdiprocmeminfo tdiprocmeminfo =
      (pfntdiprocmeminfo)dlsym(handle, "tdiprocmeminfo");
  struct tdistructprocmeminfo meminfo;
  if (tdiprocmeminfo) {
    tditrace("@A+tdiprocmeminfo");
    tdiprocmeminfo(&meminfo);
    tditrace("@A-tdiprocmeminfo");
    fprintf(stdout, "meminfo         cached:%d\n", meminfo.cached);
  }

  pfntdiproctvbcmmeminfo tdiproctvbcmmeminfo =
      (pfntdiproctvbcmmeminfo)dlsym(handle, "tdiproctvbcmmeminfo");
  struct tdistructproctvbcmmeminfo tvbcmmeminfo;
  if (tdiproctvbcmmeminfo) {
    tditrace("@A+tdiproctvbcmmeminfo");
    tdiproctvbcmmeminfo(&tvbcmmeminfo);
    tditrace("@A-tdiproctvbcmmeminfo");
    fprintf(stdout, "tvbcmmeminfo    heap0free:%d, heap1free:%d\n",
            tvbcmmeminfo.heap0free / 1024, tvbcmmeminfo.heap1free / 1024);
  }

  pfntdiprocstat tdiprocstat = (pfntdiprocstat)dlsym(handle, "tdiprocstat");
  struct tdistructprocstat stat;
  if (tdiprocstat) {
    tditrace("@A+tdiprocstat");
    tdiprocstat(&stat);
    tditrace("@A-tdiprocstat");
#if 0
    fprintf(stdout,
            "stat= cpu_user:%u, cpu_nice:%u, cpu_sys:%u, cpu_idle:%u, "
            "cpu_iowait:%u, cpu_irq:%u, cpu_softirq:%u\n",
            stat.cpu_user, stat.cpu_nice, stat.cpu_system, stat.cpu_idle,
            stat.cpu_iowait, stat.cpu_irq, stat.cpu_softirq);
#endif
    fprintf(stdout,
            "stat            cpu0_user:%u, cpu0_nice:%u, cpu0_sys:%u, "
            "cpu0_idle:%u, "
            "cpu0_iowait:%u, cpu0_irq:%u, cpu0_softirq:%u\n",
            stat.cpu0_user, stat.cpu0_nice, stat.cpu0_system, stat.cpu0_idle,
            stat.cpu0_iowait, stat.cpu0_irq, stat.cpu0_softirq);
    fprintf(stdout,
            "stat            cpu1_user:%u, cpu1_nice:%u, cpu1_sys:%u, "
            "cpu1_idle:%u, "
            "cpu1_iowait:%u, cpu1_irq:%u, cpu1_softirq:%u\n",
            stat.cpu1_user, stat.cpu1_nice, stat.cpu1_system, stat.cpu1_idle,
            stat.cpu1_iowait, stat.cpu1_irq, stat.cpu1_softirq);
  }

  pfntdiprocselfstatm tdiprocselfstatm =
      (pfntdiprocselfstatm)dlsym(handle, "tdiprocselfstatm");
  struct tdistructprocselfstatm selfstatm;
  if (tdiprocselfstatm) {
    tditrace("@A+tdiselfstatm");
    tdiprocselfstatm(&selfstatm);
    tditrace("@A-tdiselfstatm");
    fprintf(stdout, "selfstatm       vmsize:%lu, rss:%lu\n", selfstatm.vmsize,
            selfstatm.rss);
  }

  pfntdiprocselfstatus tdiprocselfstatus =
      (pfntdiprocselfstatus)dlsym(handle, "tdiprocselfstatus");
  struct tdistructprocselfstatus selfstatus;
  if (tdiprocselfstatus) {
    tditrace("@A-tdiprocvmstat");
    tdiprocselfstatus(&selfstatus);
    tditrace("@A-tdiprocvmstat");
    fprintf(stdout, "selfstatus      vmswap:%d\n", selfstatus.vmswap);
  }

  pfntdiprocdiskstats tdiprocdiskstats =
      (pfntdiprocdiskstats)dlsym(handle, "tdiprocdiskstats");
  static struct tdistructprocdiskstats diskstats[10];
  if (tdiprocdiskstats) {
    int m, n;
    tditrace("@A+tdiprocdiskstats");
    tdiprocdiskstats(diskstats, getenv("DISKS"), &n);
    tditrace("@A-tdiprocdiskstats");

    for (m = 0; m < n; m++) {
      fprintf(stdout,
              "diskstats       name:%s, reads:%u, reads_merged:%u, "
              "reads_sectors:%u, "
              "reads_time:%u, writes:%u, writes_merged:%u, writes_sectors:%u, "
              "writes_time:%u,\n",
              diskstats[m].name, diskstats[m].reads, diskstats[m].reads_merged,
              diskstats[m].reads_sectors, diskstats[m].reads_time,
              diskstats[m].writes, diskstats[m].writes_merged,
              diskstats[m].writes_sectors, diskstats[m].writes_time);
    }
  }

  pfntdiprocnetdev tdiprocnetdev =
      (pfntdiprocnetdev)dlsym(handle, "tdiprocnetdev");
  static struct tdistructprocnetdev netdev[10];
  if (tdiprocnetdev) {
    int m, n;
    tditrace("@A+tdiprocnetdev");
    tdiprocnetdev(netdev, getenv("NETS"), &n);
    tditrace("@A-tdiprocnetdev");

    for (m = 0; m < n; m++) {
      fprintf(stdout,
              "netdev          name:%s, r_bytes: %lu, r_packets: %u, "
              "r_errs: %u, "
              "r_drop: %u, r_fifo: %u, r_frame: %u, r_compressed: %u, "
              "r_multicast: %u, t_bytes: %lu, t_packets: %u, "
              "t_errs: %u, t_drop: %u, t_fifo: %u, t_frame: %u, "
              "t_compressed: %u, t_multicast:%u\n",
              netdev[m].name, netdev[m].r_bytes, netdev[m].r_packets,
              netdev[m].r_errs, netdev[m].r_drop, netdev[m].r_fifo,
              netdev[m].r_frame, netdev[m].r_compressed, netdev[m].r_multicast,
              netdev[m].t_bytes, netdev[m].t_packets, netdev[m].t_errs,
              netdev[m].t_drop, netdev[m].t_fifo, netdev[m].t_frame,
              netdev[m].t_compressed, netdev[m].t_multicast);
    }
  }

  pfntdiprocsmaps tdiprocsmaps = (pfntdiprocsmaps)dlsym(handle, "tdiprocsmaps");
  struct tdistructprocsmaps smaps;
  char pathname[256];
  sprintf(pathname, "/proc/%s/smaps", (argc > 1) ? argv[1] : "self");
  if (tdiprocsmaps) {
    tditrace("@A+tdiprocsmaps");
    tdiprocsmaps(pathname, &smaps);
    tditrace("@A-tdiprocsmaps");
    fprintf(stdout,
            "smaps           code_rss:%d, code_ref:%d, data:%d, data_ref:%d, "
            "data_swap:%d\n",
            smaps.code_rss, smaps.code_ref, smaps.data_rss, smaps.data_ref,
            smaps.data_swap);
  }

  dlclose(handle);
  return 0;
}

const char *const eventnames[] = {"TASKS",  "ISRS",        "SEMAS",  "QUEUES",
                                  "EVENTS", "VALUES",      "CYCLES", "NOTES",
                                  "AGENTS", "MEMORYCYCLES"};

#include <getopt.h>
#include <regex.h>
#include <algorithm>
#include <string>
#include <vector>

std::vector<std::string> SemaphoreNams;
std::vector<std::string> EventNams;
std::vector<std::string> NoteNams;

static char *ptimestamp_regex;
static char *pnote_regex;
static char *psemaphore_regex;
static char *pevent_regex;

static int do_tstamp(char *buffer, char *p, long long int *tstamp) {
  static long long int itstamp = 0;
  if (p) {
    size_t nmatch = 1;
    regmatch_t pmatch[1];
    regex_t regex_ts;
    int reti;

    reti = regcomp(&regex_ts, p, 0);
    if (reti) {
      fprintf(stderr, "Could not compile timestamp regex\n");
      exit(1);
    }

    reti = regexec(&regex_ts, buffer, nmatch, pmatch, REG_EXTENDED);
    if (!reti) {
      char buf[1024];
      int j = 0;
      for (int i = pmatch[0].rm_so; i < pmatch[0].rm_eo; i++) {
        buf[j] = buffer[i];
        j++;
      }
      buf[j] = 0;

      //  Feb 24 01:04:41
      //  [   18.947554]
      if (strchr(buffer, ':')) {
        *tstamp = atoi(&buf[7]) * 3600 + atoi(&buf[10]) * 60 + atoi(&buf[13]);
      } else {
        *tstamp = (long long int)(atof(buf) * 1000000000LL);
      }

      static int ts_once = 0;
      if (!ts_once) {
        if (strchr(buf, ':')) {
          fprintf(stderr, "timestamps (UTC) = \"%s\" = %lld\n", buf, *tstamp);
        } else {
          *tstamp = (long long int)(atof(buf) * 1000000000LL);
          fprintf(stderr, "timestamps (short-monotonic) = \"%s\" = %lld\n", buf, *tstamp);
        }
        ts_once = 1;
      }

    } else if (reti == REG_NOMATCH) {
      // fprintf(stderr, "Regex tstamp no match\n");
      return 0;
    } else {
      char msgbuf[100];
      regerror(reti, &regex_ts, msgbuf, sizeof(msgbuf));
      fprintf(stderr, "Regex tstamp match failed: %s\n", msgbuf);
      exit(1);
    }

    regfree(&regex_ts);
  } else {
    itstamp += 100000000LL;
    *tstamp = itstamp;
  }
  return 1;
}

static void do_line(char *buffer, char *p, int id,
                    std::vector<std::string> &Nams, long long int tstamp) {
  regex_t regex;
  int reti;
  size_t nmatch = 2;
  regmatch_t pmatch[2];
  char buf[1024];

  if (!p) return;

  reti = regcomp(&regex, p, 0);
  if (reti) {
    fprintf(stderr, "Could not compile regex\n");
    exit(1);
  }

  reti = regexec(&regex, buffer, nmatch, pmatch, REG_EXTENDED);
  if (!reti) {
    unsigned int i, index;
    strncpy(buf, &buffer[pmatch[1].rm_so], pmatch[1].rm_eo - pmatch[1].rm_so);
    buf[pmatch[1].rm_eo - pmatch[1].rm_so] = 0;

    for (i = 0; i < strlen(buffer); i++) {
      buf[i] = buf[i];
      if ((buf[i] == ' ') || (buf[i] == '\n')) buf[i] = '_';
    }

    if (std::find(Nams.begin(), Nams.end(), buf) == Nams.end()) {
      Nams.push_back(buf);
      index =
          std::distance(Nams.begin(), std::find(Nams.begin(), Nams.end(), buf));

      fprintf(stdout, "NAM %d %d %s\n", id, (id * 1000) + index, buf);
      fprintf(stderr, "[%s]\"%s\"\n", eventnames[id], buf);
    }

    index =
        std::distance(Nams.begin(), std::find(Nams.begin(), Nams.end(), buf));

    for (i = 0; i < strlen(buffer); i++) {
      buf[i] = buffer[i];
      if (buf[i] == ' ') buf[i] = '_';
      if (buf[i] == '\n') {
        buf[i] = 0;
        break;
      }
    }

    fprintf(stdout, "OCC %d %d %lld\n", id, (id * 1000) + index, tstamp);
    fprintf(stdout, "DSC 0 0 %s\n", buf);
  }
  regfree(&regex);
}

static int tdipipe(int argc, char *argv[]) {
  int c;
  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
        {"v", no_argument, 0, 0},       {"t", required_argument, 0, 0},
        {"s", required_argument, 0, 0}, {"e", required_argument, 0, 0},
        {"n", required_argument, 0, 0}, {0, 0, 0, 0}};

    c = getopt_long(argc, argv, "", long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 0:
        fprintf(stderr, "option %s", long_options[option_index].name);
        if (optarg) fprintf(stderr, " with arg %s", optarg);
        fprintf(stderr, "\n");

        if (option_index == 1) {
          ptimestamp_regex = optarg;
        }
        if (option_index == 2) {
          psemaphore_regex = optarg;
        }
        if (option_index == 3) {
          pevent_regex = optarg;
        }
        if (option_index == 4) {
          pnote_regex = optarg;
        }
        break;

      default:
        printf("?? getopt returned character code 0%o ??\n", c);
    }
  }

#define BLOCK_SIZE 4096
  char buffer[BLOCK_SIZE];
  static int do_once = 0;
  long long int tstamp;

  while (fgets(buffer, 255, stdin)) {
    if (!do_once) {
      do_once = 1;
      fprintf(stdout, "TIME 1000000000\n");
      fprintf(stdout, "SPEED 1000000000\n");
      fprintf(stdout, "MEMSPEED 1000000000\n");
    }

    if (do_tstamp(buffer, ptimestamp_regex, &tstamp)) {
      do_line(buffer, psemaphore_regex, SEMAS, SemaphoreNams, tstamp);
      do_line(buffer, pevent_regex, EVENTS, EventNams, tstamp);
      do_line(buffer, pnote_regex, NOTES, NoteNams, tstamp);
    }
  }

  return 0;
}
