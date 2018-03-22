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
  char line[1024];
  FILE *f = NULL;

  typedef struct statstruct_proc {
    int pid;                       /** The process id. **/
    char exName[_POSIX_PATH_MAX];  /** The filename of the executable **/
    /*  3 */ char state; /** 1 **/ /** R is running, S is sleeping,
           D is sleeping in an uninterruptible wait,
           Z is zombie, T is traced or stopped **/
    /*  4 */ int ppid;             /** The pid of the parent. **/
    /*  5 */ int pgrp;             /** The pgrp of the process. **/
    /*  6 */ int session;          /** The session id of the process. **/
    /*  7 */ int tty;              /** The tty the process uses **/
    /*  8 */ int tpgid;            /** (too long) **/
    /*  9 */ unsigned int flags;   /** The flags of the process. **/
    /* 10 */ unsigned long int minflt;  /** The number of minor faults **/
    /* 11 */ unsigned long int
        cminflt;                  /** The number of minor faults with childs **/
    /* 12 */ unsigned long int majflt; /** The number of major faults **/
    /* 13 */ unsigned long int
        cmajflt;         /** The number of major faults with childs **/
    /* 14 */ unsigned long int utime;  /** user mode jiffies **/
    /* 15 */ unsigned long int stime;  /** kernel mode jiffies **/
    /* 16 */ long int cutime; /** user mode jiffies with childs **/
    /* 17 */ long int cstime; /** kernel mode jiffies with childs **/

  } procinfo;

  procinfo pinfo;
  char *s, *t;

  while (1) {
    if ((f = fopen("/proc/self/stat", "r"))) {
      if ((fgets(line, 1024, f))) {
        /** pid **/
        sscanf(line, "%u", &(pinfo.pid));
        s = strchr(line, '(') + 1;
        t = strchr(line, ')');
        strncpy(pinfo.exName, s, t - s);
        pinfo.exName[t - s] = '\0';

        sscanf(t + 2,
               /*
                 3  4  5  6  7  8  9  10  11  12  13  14  15  16  17
               */
               "%c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld",
               &(pinfo.state), &(pinfo.ppid), &(pinfo.pgrp), &(pinfo.session),
               &(pinfo.tty), &(pinfo.tpgid), &(pinfo.flags), &(pinfo.minflt),
               &(pinfo.cminflt), &(pinfo.majflt), &(pinfo.cmajflt),
               &(pinfo.utime), &(pinfo.stime), &(pinfo.cutime),
               &(pinfo.cstime));

        fprintf(stdout,
                /*
                  3  4  5  6  7  8  9  10  11  12  13  14  15  16  17
                */
                "%c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld\n",
                pinfo.state, pinfo.ppid, pinfo.pgrp, pinfo.session, pinfo.tty,
                pinfo.tpgid, pinfo.flags, pinfo.minflt, pinfo.cminflt,
                pinfo.majflt, pinfo.cmajflt, pinfo.utime, pinfo.stime,
                pinfo.cutime, pinfo.cstime);
        fclose(f);
        usleep(1 * 1000 * 1000);
      }
    }
  }
}
