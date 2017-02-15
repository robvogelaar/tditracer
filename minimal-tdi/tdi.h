#if 0
extern void tditrace(const char *format, ...);
extern void tditrace_ex(int mask, const char *format, ...);
#endif

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
  int active_anon;
  int inactive_anon;
  int active_file;
  int inactive_file;
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

struct tdistructprocselfsmaps {
  unsigned int swap;
};
typedef int (*pfntdiprocselfsmaps)(struct tdistructprocselfsmaps *s);

struct tdistructprocdiskstats {
  char name[16];
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

struct tdistructprocnetdev {
  char name[16];
  char match[64];
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
