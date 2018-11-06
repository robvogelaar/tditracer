#define TASKS 0
#define ISRS 1
#define SEMAS 2
#define QUEUES 3
#define EVENTS 4
#define VALUES 5
#define CYCLES 6
#define NOTES 7
#define AGENTS 8
#define MEMORYCYCLES 9

#define MAX_TASKS 2048
#define MAX_ISRS 512
#define MAX_SEMAS 512
#define MAX_QUEUES 512
#define MAX_EVENTS 512
#define MAX_VALUES 512
#define MAX_CYCLES 512
#define MAX_NOTES 512
#define MAX_AGENTS 512

#define MAX_TEXT 1024

#define TASKS_BASE 10000
#define ISRS_BASE 20000
#define SEMAS_BASE 21000
#define QUEUES_BASE 22000
#define EVENTS_BASE 23000
#define VALUES_BASE 24000
#define CYCLES_BASE 25000
#define NOTES_BASE 26000
#define AGENTS_BASE 27000
#define MEMORYCYCLES_BASE 28000

#define MULTIPLIER 100000

/*
 * send new value
 */
#define OUT(name, trace, value) \
                                \
  unsigned int _##name = value; \
  tditrace(trace, _##name);

/*
 * send new value, unless new value == previous value
 * i.e. do not send new value, if new value == prev value
 * and do not send unless a non 0 value is seen
 */

#define OUT1(name, trace, tname, tvalue)  \
                                          \
  static unsigned int _##name##_seen = 0; \
  static unsigned int _##name##_prev;     \
  unsigned int _##name = tvalue;          \
  if (_##name##_seen == 0) {              \
    if (_##name != 0) {                   \
      _##name##_seen = 1;                 \
      tditrace(trace, tname, _##name);    \
    }                                     \
  } else if (_##name##_prev != _##name) { \
    tditrace(trace, tname, _##name);      \
  }                                       \
  _##name##_prev = _##name;

#define OUT1base(name, trace, tname, tvalue)            \
                                                        \
  static unsigned int _##name##_seen = 0;               \
  static unsigned int _##name##_prev;                   \
  static unsigned int _##name##_base;                   \
  unsigned int _##name = tvalue;                        \
  if (_##name##_seen == 0) {                            \
    if (_##name != 0) {                                 \
      _##name##_seen = 1;                               \
      _##name##_base = _##name;                         \
      tditrace(trace, tname, _##name - _##name##_base); \
    }                                                   \
  } else if (_##name##_prev != _##name) {               \
    tditrace(trace, tname, _##name - _##name##_base);   \
  }                                                     \
  _##name##_prev = _##name;

/*
 * send new value, unless new value == prev value and new value == prev prev
 * value
 *  i.e. do not send new value, if new value == prev value == prev prev value
 * and do not send unless a non 0 value is seen
 */
#define OUT2(name, trace, tname, tvalue)                                       \
                                                                               \
  static unsigned int _##name##_seen = 0;                                      \
  static unsigned int _##name##_prev;                                          \
  static unsigned int _##name##_prevprev;                                      \
  unsigned int _##name = tvalue;                                               \
  if (_##name##_seen == 0) {                                                   \
    if (_##name != 0) {                                                        \
      _##name##_seen = 1;                                                      \
      tditrace(trace, tname, _##name);                                         \
    }                                                                          \
  } else if (_##name##_seen == 1) {                                            \
    _##name##_seen = 2;                                                        \
    tditrace(trace, tname, _##name);                                           \
  } else if ((_##name##_prev != _##name) || (_##name##_prevprev != _##name)) { \
    tditrace(trace, tname, _##name);                                           \
  }                                                                            \
  _##name##_prevprev = _##name##_prev;                                         \
  _##name##_prev = _##name;

/*
 * send previous value, unless new value == prev value and new value ==
 * prev prev value
 * i.e. do not send previous if new value == prev value == prev prev value
 * and do not send unless a non 0 value is seen
 */
#define OUT3(name, trace, tname, tvalue)                                       \
                                                                               \
  static unsigned int _##name##_seen = 0;                                      \
  static unsigned int _##name##_prev;                                          \
  static unsigned int _##name##_prevprev;                                      \
  unsigned int _##name = tvalue;                                               \
  if (_##name##_seen == 0) {                                                   \
    if (_##name != 0) {                                                        \
      _##name##_seen = 1;                                                      \
    }                                                                          \
  } else if (_##name##_seen == 1) {                                            \
    if (_##name##_prev != _##name) {                                           \
      _##name##_seen = 2;                                                      \
      tditrace(trace, tname, _##name##_prev);                                  \
    }                                                                          \
  } else if ((_##name##_prev != _##name) || (_##name##_prevprev != _##name)) { \
    tditrace(trace, tname, _##name##_prev);                                    \
  }                                                                            \
  _##name##_prevprev = _##name##_prev;                                         \
  _##name##_prev = _##name;


struct tdistructprocvmstat {
  int pswpin;
  int pswpout;
  int pgpgin;
  int pgpgout;
  int pgfault;
  int pgmajfault;
};
typedef int (*pfntdiprocvmstat)(struct tdistructprocvmstat *s);

struct tdistructprocmeminfo {
  int cached;
};
typedef int (*pfntdiprocmeminfo)(struct tdistructprocmeminfo *s);

struct tdistructproctvbcmmeminfo {
  int heap0free;
  int heap1free;
};
typedef int (*pfntdiproctvbcmmeminfo)(struct tdistructproctvbcmmeminfo *s);

struct tdistructprocstat {
  int cpu_user;
  int cpu_nice;
  int cpu_system;
  int cpu_idle;
  int cpu_iowait;
  int cpu_irq;
  int cpu_softirq;
  int cpu0_user;
  int cpu0_nice;
  int cpu0_system;
  int cpu0_idle;
  int cpu0_iowait;
  int cpu0_irq;
  int cpu0_softirq;
  int cpu1_user;
  int cpu1_nice;
  int cpu1_system;
  int cpu1_idle;
  int cpu1_iowait;
  int cpu1_irq;
  int cpu1_softirq;
};
typedef int (*pfntdiprocstat)(struct tdistructprocstat *s);

struct tdistructprocselfstatm {
  unsigned long vmsize;
  unsigned long rss;
};
typedef int (*pfntdiprocselfstatm)(struct tdistructprocselfstatm *s);

struct tdistructprocselfstatus {
  unsigned int vmswap;
};
typedef int (*pfntdiprocselfstatus)(struct tdistructprocselfstatus *s);

struct tdistructprocsmaps {
  unsigned int code_rss;
  unsigned int code_pss;
  unsigned int code_ref;
  unsigned int data_rss;
  unsigned int data_pss;
  unsigned int data_ref;
  unsigned int data_swap;
};
typedef int (*pfntdiprocsmaps)(const char* pathname, struct tdistructprocsmaps *s);

struct tdistructprocdiskstats {
  char name[32];
  char match[64];
  unsigned int reads;
  unsigned int reads_merged;
  unsigned int reads_sectors;
  unsigned int reads_time;
  unsigned int writes;
  unsigned int writes_merged;
  unsigned int writes_sectors;
  unsigned int writes_time;
};
typedef int (*pfntdiprocdiskstats)(struct tdistructprocdiskstats s[],
                                   const char *disks, int *nrdisks);

#define MAXNETS 40

struct tdistructprocnetdev {
  char name[64];
  char match[256];
  unsigned long r_bytes;
  unsigned int r_packets;
  unsigned int r_errs;
  unsigned int r_drop;
  unsigned int r_fifo;
  unsigned int r_frame;
  unsigned int r_compressed;
  unsigned int r_multicast;
  unsigned long t_bytes;
  unsigned int t_packets;
  unsigned int t_errs;
  unsigned int t_drop;
  unsigned int t_fifo;
  unsigned int t_frame;
  unsigned int t_compressed;
  unsigned int t_multicast;
};
typedef int (*pfntdiprocnetdev)(struct tdistructprocnetdev s[],
                                const char *nets, int *nrnets);


#define CPUINFO           128
#define MEMINFO           129
#define DSKINFO           130
#define NETINFO           131
#define SLFINFO           132
#define INTINFO           133

#define PIDINFO           134
#define MARKER            135
#define ENVINFO           136
#define DISKSLIST         137
#define NETSLIST          138
#define INTSLIST          139

#define CPUINFO_MAXNUMBER   14    // 0..13
#define MEMINFO_MAXNUMBER   9     // 0..9
#define DSKINFO_MAXNUMBER   7     // 0..7
#define NETINFO_MAXNUMBER   159   // 0..159 (40x interfaces, 4 items per)
#define INTINFO_MAXNUMBER   2

#define SLFINFO_MAXNUMBER  10     // 0..10
