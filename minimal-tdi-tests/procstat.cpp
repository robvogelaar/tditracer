#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <malloc.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syscall.h>
#include <unistd.h>

int main(void) {
  char line[256];
  FILE *f = NULL;

  struct cpu_t {
    int user;
    int nice;
    int system;
    int idle;
    int iowait;
    int irq;
    int softirq;
  };

  cpu_t cpu, cpu0, cpu1;

  if (1) {
    f = fopen("/proc/stat", "r");
    if (f) {
      fgets(line, 256, f);
      sscanf(line, "cpu %u %u %u %u %u %u %u", &cpu.user, &cpu.nice,
             &cpu.system, &cpu.idle, &cpu.iowait, &cpu.irq, &cpu.softirq);
      fgets(line, 256, f);
      sscanf(line, "cpu0 %u %u %u %u %u %u %u", &cpu0.user, &cpu0.nice,
             &cpu0.system, &cpu0.idle, &cpu0.iowait, &cpu0.irq, &cpu0.softirq);
      fgets(line, 256, f);
      sscanf(line, "cpu1 %u %u %u %u %u %u %u", &cpu1.user, &cpu1.nice,
             &cpu1.system, &cpu1.idle, &cpu1.iowait, &cpu1.irq, &cpu1.softirq);
      fclose(f);
    }
  }

  fprintf(stdout, "cpu %u %u %u %u %u %u %u\n", cpu.user, cpu.nice, cpu.system,
          cpu.idle, cpu.iowait, cpu.irq, cpu.softirq);
  fprintf(stdout, "cpu0 %u %u %u %u %u %u %u\n", cpu0.user, cpu0.nice, cpu0.system,
          cpu0.idle, cpu0.iowait, cpu0.irq, cpu0.softirq);
  fprintf(stdout, "cpu1 %u %u %u %u %u %u %u\n", cpu1.user, cpu1.nice, cpu1.system,
          cpu1.idle, cpu1.iowait, cpu1.irq, cpu1.softirq);
}
