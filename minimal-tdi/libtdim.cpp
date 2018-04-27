#define VERSION "0.1"

//#define DEBUG

extern "C" {

#include <ctype.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <malloc.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <syscall.h>
#include <unistd.h>

#include "tdim.h"

void tditrace(const char *format, ...);
void tditrace_ex(int mask, const char *format, ...);

#ifndef TMPFS
#define TMPFS "/tmp/"
#endif

static pid_t gpid;
static char gprocname[128];

static char pid_procname_table_procname[128][16];
static unsigned int pid_procname_table_pid[16];
static int pid_procname_table_nr = 0;

static char envs[128][16];
static int envs_nr;

static char diskslist[128];
static char netslist[128];

static pthread_mutex_t lock;

static void LOCK_init(void) {
  if (pthread_mutex_init(&lock, NULL) != 0) {
    fprintf(stderr, "\n mutex init failed\n");
  }
}

static void LOCK(void) { pthread_mutex_lock(&lock); }
static void UNLOCK(void) { pthread_mutex_unlock(&lock); }

static unsigned int gtracebuffersize = 16 * 1024 * 1024;

static char gtracebufferfilename[128];
static char gsocket_path[128];

static struct stat gtrace_buffer_st;

static char *gtrace_buffer;
static char *gtrace_buffer_byte_ptr;
static unsigned int *gtrace_buffer_dword_ptr;
static unsigned int *gtrace_buffer_rewind_ptr;

static int gtditrace_inited;
static int reported_full;
static int report_tid;

typedef unsigned long long _u64;

static char tasks_array[MAX_TASKS][MAX_TEXT];
static int nr_tasks = 0;
static int nr_task_entries = 0;

static char isrs_array[MAX_ISRS][MAX_TEXT];
static int nr_isrs = 0;
static int nr_isr_entries = 0;

static char semas_array[MAX_SEMAS][MAX_TEXT];
static int nr_semas = 0;
static int nr_sema_entries = 0;

static char queues_array[MAX_QUEUES][MAX_TEXT];
static int prev_queues[MAX_QUEUES];
static int nr_queues = 0;
static int nr_queue_entries = 0;

static char values_array[MAX_VALUES][MAX_TEXT];
static int nr_values = 0;
static int nr_value_entries = 0;

static char cycles_array[MAX_CYCLES][MAX_TEXT];
static int nr_cycles = 0;
static int nr_cycle_entries = 0;

static char events_array[MAX_EVENTS][MAX_TEXT];
static int nr_events = 0;
static int nr_event_entries = 0;

static char notes_array[MAX_NOTES][MAX_TEXT];
static int nr_notes = 0;
static int nr_note_entries = 0;

static char agents_array[MAX_AGENTS][MAX_TEXT];
static int nr_agents = 0;
static int nr_agent_entries = 0;

static void tditrace_rewind();

#define docycle(number, id, pidid, span)                                       \
  static unsigned int _##id##_seen[span];                                      \
                                                                               \
  if (_##id##_seen[pidid] == 0) {                                              \
    _##id##_seen[pidid] = 1;                                                   \
  }                                                                            \
  fprintf(stdout, "VAL 6 %d %lld\n", id + pidid * span, number * 10000000LL);

#define doqueue(number, id, pidid, span, tstamp)                               \
  static unsigned int _##id##_seen[span];                                      \
  static unsigned int _##id##_prev[span];                                      \
                                                                               \
  if (_##id##_seen[pidid] == 0) {                                              \
    if (number != 0) {                                                         \
      _##id##_seen[pidid] = 1;                                                 \
      if (number >= _##id##_prev[pidid])                                       \
        fprintf(stdout, "STA 3 %d %lld %d\n", id + pidid * span, tstamp,       \
                number - _##id##_prev[pidid]);                                 \
      else                                                                     \
        fprintf(stdout, "STO 3 %d %lld %d\n", id + pidid * span, tstamp,       \
                _##id##_prev[pidid] - number);                                 \
    }                                                                          \
  } else {                                                                     \
    if (number != _##id##_prev[pidid]) {                                       \
      if (number >= _##id##_prev[pidid])                                       \
        fprintf(stdout, "STA 3 %d %lld %d\n", id + pidid * span, tstamp,       \
                number - _##id##_prev[pidid]);                                 \
      else                                                                     \
        fprintf(stdout, "STO 3 %d %lld %d\n", id + pidid * span, tstamp,       \
                _##id##_prev[pidid] - number);                                 \
    }                                                                          \
  }                                                                            \
  _##id##_prev[pidid] = number;

#define doqueue_base(number, id, pidid, span, tstamp)                          \
  static unsigned int _##id##_seen[span];                                      \
  static unsigned int _##id##_prev[span];                                      \
  static unsigned int _##id##_base[span];                                      \
                                                                               \
  if (_##id##_seen[pidid] == 0) {                                              \
    if (number != 0) {                                                         \
      _##id##_seen[pidid] = 1;                                                 \
      _##id##_base[pidid] = number;                                            \
      if ((number - _##id##_base[pidid]) >= _##id##_prev[pidid])               \
        fprintf(stdout, "STA 3 %d %lld %d\n", id + pidid * span, tstamp,       \
                (number - _##id##_base[pidid]) - _##id##_prev[pidid]);         \
      else                                                                     \
        fprintf(stdout, "STO 3 %d %lld %d\n", id + pidid * span, tstamp,       \
                _##id##_prev[pidid] - (number - _##id##_base[pidid]));         \
    }                                                                          \
  } else {                                                                     \
    if ((number - _##id##_base[pidid]) != _##id##_prev[pidid]) {               \
      if ((number - _##id##_base[pidid]) >= _##id##_prev[pidid])               \
        fprintf(stdout, "STA 3 %d %lld %d\n", id + pidid * span, tstamp,       \
                (number - _##id##_base[pidid]) - _##id##_prev[pidid]);         \
      else                                                                     \
        fprintf(stdout, "STO 3 %d %lld %d\n", id + pidid * span, tstamp,       \
                _##id##_prev[pidid] - (number - _##id##_base[pidid]));         \
    }                                                                          \
  }                                                                            \
  _##id##_prev[pidid] = (number - _##id##_base[pidid]);

#define dovalue(number, id, pidid, span)                                       \
                                                                               \
  static unsigned int _##id##_seen[span];                                      \
  static unsigned int _##id##_prev[span];                                      \
  static unsigned int _##id##_prevprev[span];                                  \
  if (_##id##_seen[pidid] == 0) {                                              \
    if (number != 0) {                                                         \
      _##id##_seen[pidid] = 1;                                                 \
    }                                                                          \
  } else if (_##id##_seen[pidid] == 1) {                                       \
    if (_##id##_prev[pidid] != number) {                                       \
      _##id##_seen[pidid] = 2;                                                 \
      fprintf(stdout, "VAL 5 %d %d\n", id + pidid * span,                      \
              _##id##_prev[pidid]);                                            \
    }                                                                          \
  } else if ((_##id##_prev[pidid] != number) ||                                \
             (_##id##_prevprev[pidid] != number)) {                            \
    fprintf(stdout, "VAL 5 %d %d\n", id + pidid * span, _##id##_prev[pidid]);  \
  }                                                                            \
  _##id##_prevprev[pidid] = _##id##_prev[pidid];                               \
  _##id##_prev[pidid] = number;

static void addentry_cpuinfo(unsigned int *numbers, _u64 timestamp) {
  static int nams_added = 0;

  if (!nams_added) {
    fprintf(stdout, "NAM 6 6500 CPU0_USER\n");
    fprintf(stdout, "NAM 6 6501 CPU0_SYSTEM\n");
    fprintf(stdout, "NAM 6 6502 CPU0_IO\n");
    fprintf(stdout, "NAM 6 6503 CPU0_IRQ\n");
    fprintf(stdout, "NAM 6 6504 CPU1_USER\n");
    fprintf(stdout, "NAM 6 6505 CPU1_SYSTEM\n");
    fprintf(stdout, "NAM 6 6506 CPU1_IO\n");
    fprintf(stdout, "NAM 6 6507 CPU1_IRQ\n");
    nams_added = 1;
  }

  fprintf(stdout, "TIM %lld\n", timestamp);
  docycle(numbers[0], 6500, 0, 1);
  docycle(numbers[1], 6501, 0, 1);
  docycle(numbers[2], 6502, 0, 1);
  docycle(numbers[3], 6503, 0, 1);
  docycle(numbers[4], 6504, 0, 1);
  docycle(numbers[5], 6505, 0, 1);
  docycle(numbers[6], 6506, 0, 1);
  docycle(numbers[7], 6507, 0, 1);
}

static void addentry_meminfo(unsigned int *numbers, _u64 timestamp) {
  static int nams_added = 0;

  if (!nams_added) {
    fprintf(stdout, "NAM 3 3500 FREE\n");
    fprintf(stdout, "NAM 3 3501 BUFFERS\n");
    fprintf(stdout, "NAM 3 3502 CACHED\n");
    fprintf(stdout, "NAM 3 3503 SWAP\n");
    fprintf(stdout, "NAM 3 3504 SWOUT\n");
    fprintf(stdout, "NAM 3 3505 SWIN\n");

    fprintf(stdout, "NAM 5 5500 PGIN\n");
    fprintf(stdout, "NAM 5 5501 PGOUT\n");
    fprintf(stdout, "NAM 5 5502 MINFLT\n");
    fprintf(stdout, "NAM 5 5503 MAJFLT\n");
    fprintf(stdout, "NAM 5 5504 SWOUT\n");
    fprintf(stdout, "NAM 5 5505 SWIN\n");
    nams_added = 1;
  }

  doqueue(numbers[0], 3500, 0, 1, timestamp);
  doqueue(numbers[1], 3501, 0, 1, timestamp);
  doqueue(numbers[2], 3502, 0, 1, timestamp);
  doqueue(numbers[3], 3503, 0, 1, timestamp);
  doqueue_base(numbers[4], 3504, 0, 1, timestamp);
  doqueue_base(numbers[5], 3505, 0, 1, timestamp);

  fprintf(stdout, "TIM %lld\n", timestamp);
  dovalue(numbers[6], 5500, 0, 1);
  dovalue(numbers[7], 5501, 0, 1);
  dovalue(numbers[8], 5502, 0, 1);
  dovalue(numbers[9], 5503, 0, 1);
  dovalue(numbers[4], 5504, 0, 1);
  dovalue(numbers[5], 5505, 0, 1);
}

static void addentry_dskinfo(unsigned int *numbers, _u64 timestamp) {
#define MAXDISKS 4
  static int entries_added = 0;
  static int nrdisks;
  static char disks[64][MAXDISKS];

  if (!entries_added) {
    char *saveptr;
    char *pt = strtok_r(diskslist, ",", &saveptr);
    while (pt != NULL) {
      strcpy(disks[nrdisks], pt);
      pt = strtok_r(NULL, ",", &saveptr);

      fprintf(stdout, "NAM 5 %d %s:read\n", 5700 + nrdisks * MAXDISKS,
              disks[nrdisks]);
      fprintf(stdout, "NAM 5 %d %s:write\n", 5701 + nrdisks * MAXDISKS,
              disks[nrdisks]);
      nrdisks++;
      if (nrdisks == MAXDISKS)
        break;
    }
    entries_added = 1;
  }

  fprintf(stdout, "TIM %lld\n", timestamp);
  int i;
  for (i = 0; i < nrdisks; i++) {
    dovalue(numbers[i * 2 + 0], 5700, i, MAXDISKS);
    dovalue(numbers[i * 2 + 1], 5701, i, MAXDISKS);
  }
}

static void addentry_netinfo(unsigned int *numbers, _u64 timestamp) {
#define MAXNETS 2
  static int entries_added = 0;
  static int nrnets;
  static char nets[64][MAXNETS];

  if (!entries_added) {
    char *saveptr;
    char *pt = strtok_r(netslist, ",", &saveptr);
    while (pt != NULL) {
      strcpy(nets[nrnets], pt);
      pt = strtok_r(NULL, ",", &saveptr);

      fprintf(stdout, "NAM 5 %d %s:receive\n", 5800 + nrnets * MAXNETS,
              nets[nrnets]);
      fprintf(stdout, "NAM 5 %d %s:transmit\n", 5801 + nrnets * MAXNETS,
              nets[nrnets]);
      nrnets++;
      if (nrnets == MAXNETS)
        break;
    }
    entries_added = 1;
  }

  fprintf(stdout, "TIM %lld\n", timestamp);
  int i;
  for (i = 0; i < nrnets; i++) {
    dovalue(numbers[i * 2 + 0], 5800, i, MAXNETS);
    dovalue(numbers[i * 2 + 1], 5801, i, MAXNETS);
  }
}

static void addentry_slfinfo(unsigned int *numbers, _u64 timestamp) {
  static unsigned int pidlist[16];
  static int nrpids_seen = 0;

  unsigned int pid = numbers[0];
  char procname[128] = "?";
  int i;
  for (i = 0; i < pid_procname_table_nr; i++) {
    if (pid == pid_procname_table_pid[i])
      strcpy(procname, pid_procname_table_procname[i]);
  }

  for (i = 0; i < nrpids_seen; i++) {
    if (pid == pidlist[i])
      break;
  }
  if (pid != pidlist[i]) {
    pidlist[i] = pid;
    nrpids_seen++;

    fprintf(stdout, "NAM 3 %d [%d:%s]rss\n", 3600 + i * 10, pid, procname);
    fprintf(stdout, "NAM 3 %d [%d:%s]code:rss\n", 3601 + i * 10, pid, procname);
    fprintf(stdout, "NAM 3 %d [%d:%s]code:ref\n", 3602 + i * 10, pid, procname);
    fprintf(stdout, "NAM 3 %d [%d:%s]data\n", 3603 + i * 10, pid, procname);
    fprintf(stdout, "NAM 3 %d [%d:%s]data:ref\n", 3604 + i * 10, pid, procname);
    fprintf(stdout, "NAM 3 %d [%d:%s]data:swap\n", 3605 + i * 10, pid,
            procname);
    fprintf(stdout, "NAM 3 %d [%d:%s]malloc:sbrk\n", 3606 + i * 10, pid,
            procname);
    fprintf(stdout, "NAM 3 %d [%d:%s]malloc:mmap\n", 3607 + i * 10, pid,
            procname);

    fprintf(stdout, "NAM 5 %d [%d:%s]minflt\n", 5600 + i * 10, pid, procname);
    fprintf(stdout, "NAM 5 %d [%d:%s]majflt\n", 5601 + i * 10, pid, procname);
  }

  doqueue(numbers[1], 3600, i, 10, timestamp);
  doqueue(numbers[2], 3601, i, 10, timestamp);
  doqueue(numbers[3], 3602, i, 10, timestamp);
  doqueue(numbers[4], 3603, i, 10, timestamp);
  doqueue(numbers[5], 3604, i, 10, timestamp);
  doqueue(numbers[6], 3605, i, 10, timestamp);
  doqueue(numbers[7], 3606, i, 10, timestamp);
  doqueue(numbers[8], 3607, i, 10, timestamp);

  fprintf(stdout, "TIM %lld\n", timestamp);
  dovalue(numbers[9], 5600, i, 10);
  dovalue(numbers[10], 5601, i, 10);
}

static void addentry_marker(const char *text, int text_len, _u64 timestamp) {
  static char markers[128][16];
  static char marker_color[16];
  static int markers_nr;
  int i;

  for (i = 0; i < markers_nr; i++) {
    if (strncmp(text, markers[i], text_len) == 0)
      break;
  }

  if (strncmp(text, markers[i], text_len) != 0) {
    strncpy(markers[i], text, text_len);
    markers[i][text_len] = 0;
    markers_nr++;
    fprintf(stdout, "NAM %d %d %s\n", AGENTS, 8900 + i, markers[i]);
    fprintf(stdout, "STA %d %d %lld\n", AGENTS, 8900 + i, timestamp);
    fprintf(stdout, "DSC 3 0 %d\n", marker_color[i]);
  } else {
    fprintf(stdout, "STO %d %d %lld\n", AGENTS, 8900 + i, timestamp);
    fprintf(stdout, "STA %d %d %lld\n", AGENTS, 8900 + i, timestamp);
    marker_color[i] += 1;
    marker_color[i] &= 7;
    fprintf(stdout, "DSC 3 0 %d\n", marker_color[i]);
  }
}

static void addentry_envinfo(const char *text, int text_len) {
  int i;
  for (i = 0; i < envs_nr; i++) {
    if (strncmp(text, envs[i], text_len) == 0)
      break;
  }

  if (strncmp(text, envs[i], text_len) != 0) {
    strncpy(envs[i], text, text_len);
    envs[i][text_len] = 0;
    envs_nr++;
  }
}

static void addentry(FILE *stdout, const char *text_in, int text_len,
                     _u64 timestamp, const char *procname, int pid, int tid,
                     int nr_numbers, unsigned int *numbers,
                     unsigned short identifier) {
  int i;
  int entry;
  char fullentry[1024];

  char name[1024];
  int value = 0;
  int isampersand;

  static int maxtasks = 0;

  if (maxtasks == 0) {
    char *env;
    if ((env = getenv("MAXTASKS")))
      maxtasks = atoi(env);
    else
      maxtasks = MAX_TASKS;
  }

  // if (nr_numbers) fprintf(stderr, "nr_numbers=%d(%x)\n", nr_numbers,
  // numbers[0]);

  // fprintf(stderr, "identifier=%x(%d)(%d)\n", identifier, nr_numbers,
  // text_len);

  // fprintf(stderr, "text_in(%d)=\"", text_len);
  // for (i = 0; i < text_len; i++)
  //   fprintf(stderr, "%c", text_in[i]);
  // fprintf(stderr, "\"\n");

  char text_in1[1024];
  char *text = text_in1;

  if (identifier == CPUINFO) {
    addentry_cpuinfo(numbers, timestamp);
    return;
  } else if (identifier == MEMINFO) {
    addentry_meminfo(numbers, timestamp);
    return;
  } else if (identifier == SLFINFO) {
    addentry_slfinfo(numbers, timestamp);
    return;
  } else if (identifier == DSKINFO) {
    addentry_dskinfo(numbers, timestamp);
    return;
  } else if (identifier == NETINFO) {
    addentry_netinfo(numbers, timestamp);
    return;
  } else if (identifier == PIDINFO) {
    int i;
    unsigned int pid = numbers[0];
    for (i = 0; i < pid_procname_table_nr; i++) {
      if (pid == pid_procname_table_pid[i])
        break;
    }
    if (pid != pid_procname_table_pid[i]) {
      pid_procname_table_pid[i] = pid;
      strncpy(pid_procname_table_procname[i], text_in, text_len);
      pid_procname_table_procname[i][text_len] = 0;
      pid_procname_table_nr++;
    }
    return;
  } else if (identifier == MARKER) {
    addentry_marker(text_in, text_len, timestamp);
    return;
  } else if (identifier == ENVINFO) {
    addentry_envinfo(text_in, text_len);
    return;
  } else if (identifier == DISKSLIST) {
    strncpy(diskslist, text_in, text_len);
    diskslist[text_len] = 0;
    return;
  } else if (identifier == NETSLIST) {
    strncpy(netslist, text_in, text_len);
    netslist[text_len] = 0;
    return;
  }

  sprintf(text_in1, "[%d:%s]", pid, procname);
  int procpidtidlen = strlen(text_in1);

  isampersand =
      ((text_in[0] == '@') && ((text_in[2] == '+') || (text_in[2] == '-')));

  if (isampersand) {
    strncpy(text_in1, text_in, 3);

    snprintf(&text_in1[3], procpidtidlen + text_len + 1 - 3, "[%d:%s]%s", pid,
             procname, &text_in[3]);

  } else {
    if (text_len)

      snprintf(text_in1, procpidtidlen + text_len + 1, "[%d:%s]%s", pid,
               procname, text_in);
    else
      snprintf(text_in1, procpidtidlen + 2 + 1, "[%d:%s]%02X", pid, procname,
               identifier);
  }

  // fprintf(stderr, "text=\"%s\"\n", text);
  // for (i=0;i<30;i++)
  //   fprintf(stderr, "%02x ", text[i]);
  // fprintf(stderr,"\n");

  // get rid of any '\n', replace with '_'
  for (i = 0; i < (int)strlen(text); i++) {
    if ((text[i] == 13) || (text[i] == 10)) {
      text[i] = 0x5f;
    }
  }

  for (i = 0; i < nr_numbers; i++) {
    char number[16];
    sprintf(number, "%s@%d=%x", i == 0 ? " " : "", i, numbers[i]);
    strcat(text, number);
  }

  if (isampersand) {
    /*
     * TASKS entry
     *
     */
    if ((strncmp(text, "@T+", 3) == 0) || (strncmp(text, "@T-", 3) == 0)) {
      int enter_not_exit = (strncmp(text + 2, "+", 1) == 0);

      nr_task_entries++;

      text += 3;

      // if the entry contains a '|' then optionally replace with a space
      for (i = 0; i < (int)strlen(text); i++) {
        if (text[i] == '|')
          text[i] = ' ';
      }

      // if the entry has not been seen before, add a new entry for it and
      // issue a NAM
      entry = -1;
      for (i = 0; i < nr_tasks; i++) {
        char *pos;
        char comparestr[1024];

        strcpy(comparestr, text);
        /*
         * the portion of the text before the first space in the text
         * is considered the unique part of the text
         */
        pos = strchr(comparestr, ' ');
        if (pos) {
          *pos = 0;
        }

        if (strcmp(tasks_array[i], comparestr) == 0) {
          // found the entry
          entry = i;
          break;
        }
      }

      // Do we need to add the entry?
      if (entry == -1) {
        int len;
        char *pos;

        // ignore tasks if too many tasks
        if (nr_tasks >= maxtasks)
          return;

        pos = strchr(text, ' ');
        if (pos)
          len = pos - text;
        else
          len = strlen(text);

        strncpy(tasks_array[nr_tasks], text, len);
        tasks_array[nr_tasks][len] = 0;

        entry = nr_tasks;
        nr_tasks++;

        // Since we added a new entry we need to create a NAM, with only the
        // first part of the text
        fprintf(stdout, "NAM %d %d %s\n", TASKS, TASKS_BASE + entry,
                tasks_array[entry]);
      }

      // Add the STA or STO entry
      fprintf(stdout, "%s %d %d %lld\n", enter_not_exit ? "STA" : "STO", TASKS,
              TASKS_BASE + entry, timestamp);

      // Add the DSC entry, replace any spaces with commas
      sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
      for (i = 8; i < (int)strlen(fullentry); i++) {
        if (fullentry[i] == 32)
          fullentry[i] = 0x2c;
      }
      fwrite(fullentry, strlen(fullentry), 1, stdout);

      return;
    }

    /*
     * SEMAS entry
     *
     */
    if (strncmp(text, "@S+", 3) == 0) {

      nr_sema_entries++;

      text += 3;
      // if the entry has not been seen before, add a new entry for it and
      // issue a NAM
      entry = -1;
      for (i = 0; i < nr_semas; i++) {
        char *pos;
        char comparestr[1024];

        strcpy(comparestr, text);
        /*
         * the portion of the text before the first space in the text
         * is considered the unique part of the text
         */
        pos = strchr(comparestr, ' ');
        if (pos) {
          *pos = 0;
        }

        if (strcmp(semas_array[i], comparestr) == 0) {
          // found the entry
          entry = i;
          break;
        }
      }

      // Do we need to add the entry?
      if (entry == -1) {
        int len;
        char *pos;

        pos = strchr(text, ' ');
        if (pos)
          len = pos - text;
        else
          len = strlen(text);

        strncpy(semas_array[nr_semas], text, len);
        semas_array[nr_semas][len] = 0;

        entry = nr_semas;
        nr_semas++;

        // Since we added a new entry we need to create a NAM, with only the
        // first part of the text
        fprintf(stdout, "NAM %d %d %s\n", SEMAS, SEMAS_BASE + entry,
                semas_array[entry]);
        nr_sema_entries++;
      }

      // Add the OCC entry
      fprintf(stdout, "OCC %d %d %lld\n", SEMAS, SEMAS_BASE + entry, timestamp);
      nr_sema_entries++;

      // Add the DSC entry, replace any spaces with commas
      sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
      for (i = 8; i < (int)strlen(fullentry); i++) {
        if (fullentry[i] == 32)
          fullentry[i] = 0x2c;
      }
      fwrite(fullentry, strlen(fullentry), 1, stdout);
      nr_sema_entries++;

      return;
    }

    /*
     * ISRS entry
     *
     */
    if ((strncmp(text, "@I+", 3) == 0) || (strncmp(text, "@I-", 3) == 0)) {
      int enter_not_exit = (strncmp(text + 2, "+", 1) == 0);

      nr_isr_entries++;

      text += 3;
      // if the entry has not been seen before, add a new entry for it and
      // issue a NAM
      entry = -1;
      for (i = 0; i < nr_isrs; i++) {
        char *pos;
        char comparestr[1024];

        strcpy(comparestr, text);
        /*
         * the portion of the text before the first space in the text
         * is considered the unique part of the text
         */
        pos = strchr(comparestr, ' ');
        if (pos) {
          *pos = 0;
        }

        if (strcmp(isrs_array[i], comparestr) == 0) {
          // found the entry
          entry = i;
          break;
        }
      }

      // Do we need to add the entry?
      if (entry == -1) {
        int len;
        char *pos;

        pos = strchr(text, ' ');
        if (pos)
          len = pos - text;
        else
          len = strlen(text);

        strncpy(isrs_array[nr_isrs], text, len);
        isrs_array[nr_isrs][len] = 0;

        entry = nr_isrs;
        nr_isrs++;

        // Since we added a new entry we need to create a NAM, with only the
        // first part of the text
        fprintf(stdout, "NAM %d %d %s\n", ISRS, ISRS_BASE + entry,
                isrs_array[entry]);
        nr_isr_entries++;
      }

      // Add the STA or STO entry
      fprintf(stdout, "%s %d %d %lld\n", enter_not_exit ? "STA" : "STO", ISRS,
              ISRS_BASE + entry, timestamp);
      nr_isr_entries++;

      // Add the DSC entry, replace any spaces with commas
      sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
      for (i = 8; i < (int)strlen(fullentry); i++) {
        if (fullentry[i] == 32)
          fullentry[i] = 0x2c;
      }
      fwrite(fullentry, strlen(fullentry), 1, stdout);
      nr_isr_entries++;

      return;
    }

    /*
     * EVENTS entry
     *
     */
    if (strncmp(text, "@E+", 3) == 0) {

      nr_event_entries++;

      text += 3;

      // if the entry has not been seen before, add a new entry for it and
      // issue a NAM
      entry = -1;
      for (i = 0; i < nr_events; i++) {
        char *pos;
        char comparestr[1024];

        strcpy(comparestr, text);
        /*
         * the portion of the text before the first space in the text
         * is considered the unique part of the text
         */
        pos = strchr(comparestr, ' ');
        if (pos) {
          *pos = 0;
        }

        if (strcmp(events_array[i], comparestr) == 0) {
          // found the entry
          entry = i;
          break;
        }
      }

      // Do we need to add the entry?
      if (entry == -1) {
        int len;
        char *pos;

        pos = strchr(text, ' ');
        if (pos)
          len = pos - text;
        else
          len = strlen(text);

        strncpy(events_array[nr_events], text, len);
        events_array[nr_events][len] = 0;

        entry = nr_events;
        nr_events++;

        // Since we added a new entry we need to create a NAM, with only the
        // first part of the text
        fprintf(stdout, "NAM %d %d %s\n", EVENTS, EVENTS_BASE + entry,
                events_array[entry]);
        nr_event_entries++;
      }

      // Add the OCC entry
      fprintf(stdout, "OCC %d %d %lld\n", EVENTS, EVENTS_BASE + entry,
              timestamp);
      nr_event_entries++;

      // Add the DSC entry, replace any spaces with commas
      sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
      for (i = 8; i < (int)strlen(fullentry); i++) {
        if (fullentry[i] == 32)
          fullentry[i] = 0x2c;
      }
      nr_event_entries++;
      fwrite(fullentry, strlen(fullentry), 1, stdout);

      return;
    }

/*
 * AGENTS entry
 *
 */
#if 1
    if (1) {
#endif
      // fprintf(stderr, "text=\"%s\"\n", text);

      int enter_not_exit = (strncmp(text + 2, "+", 1) == 0);
      char agentchar = text[1];

      text += 3;
      // if the entry has not been seen before, add a new entry for it and
      // issue a NAM
      entry = -1;
      for (i = 0; i < nr_agents; i++) {
        char *pos;
        char comparestr[1024];

        strcpy(comparestr, text);
        /*
         * the portion of the text before the first space in the text
         * is considered the unique part of the text
         */
        pos = strchr(comparestr, ' ');
        if (pos) {
          *pos = 0;
        }

        if (strcmp(agents_array[i], comparestr) == 0) {
          // found the entry
          entry = i;
          break;
        }
      }

      // Do we need to add the entry?
      if (entry == -1) {
        int len;
        char *pos;

        pos = strchr(text, ' ');
        if (pos)
          len = pos - text;
        else
          len = strlen(text);

        strncpy(agents_array[nr_agents], text, len);
        agents_array[nr_agents][len] = 0;

        entry = nr_agents;
        nr_agents++;

        // Since we added a new entry we need to create a NAM, with only the
        // first part of the text
        fprintf(stdout, "NAM %d %d %s\n", AGENTS, AGENTS_BASE + entry,
                agents_array[entry]);
      }

      // Add the STA or STO entry
      fprintf(stdout, "%s %d %d %lld\n", enter_not_exit ? "STA" : "STO", AGENTS,
              AGENTS_BASE + entry, timestamp);

      // Add the DSC entry, replace any spaces with commas
      sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
      for (i = 8; i < (int)strlen(fullentry); i++) {
        if (fullentry[i] == 32)
          fullentry[i] = 0x2c;
      }
      fwrite(fullentry, strlen(fullentry), 1, stdout);

      // Add a DSC color entry
      if (enter_not_exit && ((agentchar >= '0') && (agentchar <= '7'))) {
        fprintf(stdout, "DSC 3 0 %d\n", agentchar - '0');
      }

      return;
    }
  }

  /*
   * QUEUES entry
   *
   * if text is of the form name~value then interpret this as a queue and add
   * a queue entry for it.
   * check whether '~' exists and pull out the name and value
   */
  name[0] = 0;
  char *q;
  q = strchr(text, '~');
  if (q && isdigit(q[1])) {
    for (i = 0; i < (int)strlen(text); i++) {
      if (text[i] == 32)
        break;
      if (text[i] == 126) {
        // fprintf(stderr, "text1=\"%s\"\n", text);

        strncpy(name, text, i);
        name[i] = 0;
        value = atoi(text + i + 1);
        text[i] = 32; // create split marker
      }
    }

    if (strlen(name)) {
      // check to see if we need to add this value entry (first occurrence)
      entry = -1;

      for (i = 0; i < nr_queues; i++) {
        if (strcmp(queues_array[i], name) == 0) {
          // found the entry
          entry = i;
          break;
        }
      }

      // Do we need to add the entry?
      if (entry == -1) {
        strcpy(queues_array[nr_queues], name);
        entry = nr_queues;
        nr_queues++;

        // Since we added a new entry we need to create a NAM
        fprintf(stdout, "NAM %d %d %s\n", QUEUES, QUEUES_BASE + entry,
                queues_array[entry]);

        // reset the prev_value;
        prev_queues[entry] = 0;
      }

      // fill in the value
      // add a STA (with the delta) or a STO (with the delta) depending on
      // whether the value went up or went down
      if (value >= prev_queues[entry])
        fprintf(stdout, "STA %d %d %lld %d\n", QUEUES, QUEUES_BASE + entry,
                timestamp, value - prev_queues[entry]);
      else
        fprintf(stdout, "STO %d %d %lld %d\n", QUEUES, QUEUES_BASE + entry,
                timestamp, prev_queues[entry] - value);
      prev_queues[entry] = value;

      return;
    }
  }

  /*
     * VALUES entry
     *
     * if text is of the form name#value then interpret this as a value and
     * add a value entry for it.
     * check whether '#' exists and pull out the name and value
     */

  name[0] = 0;
  char *v;
  v = strchr(text, '#');
  if (v && isdigit(v[1])) {
    for (i = 0; i < (int)strlen(text); i++) {
      if (text[i] == 32)
        break;
      if (text[i] == 35) {
        strncpy(name, text, i);
        name[i] = 0;
        value = atoi(text + i + 1);
        text[i] = 32; // create split marker
      }
    }

    if (strlen(name)) {
      // check to see if we need to add this value entry (first occurrence)
      entry = -1;

      for (i = 0; i < nr_values; i++) {
        if (strcmp(values_array[i], name) == 0) {
          // found the entry
          entry = i;
          break;
        }
      }

      // Do we need to add the entry?
      if (entry == -1) {
        strcpy(values_array[nr_values], name);
        entry = nr_values;
        nr_values++;

        // Since we added a new entry we need to create a NAM
        fprintf(stdout, "NAM %d %d %s\n", VALUES, VALUES_BASE + entry,
                values_array[entry]);
      }

      // fill in the value
      // add a TIM and a VAL
      fprintf(stdout, "TIM %lld\n", timestamp);
      fprintf(stdout, "VAL %d %d %d\n", VALUES, VALUES_BASE + entry, value);

      return;
    }
  }

  /*
     * CYCLES entry
     *
     * if text is of the form name^cycle then interpret this as a cycle and
     * add a cycle entry for it.
     * check whether '^' exists and pull out the name and cycle
     */

  name[0] = 0;
  char *c;
  c = strchr(text, '^');
  if (c && isdigit(c[1])) {
    for (i = 0; i < (int)strlen(text); i++) {
      if (text[i] == 32)
        break;
      if (text[i] == 94) {
        strncpy(name, text, i);
        name[i] = 0;
        value = atoi(text + i + 1);
        text[i] = 32; // create split marker
      }
    }

    if (strlen(name)) {
      // check to see if we need to add this cycle entry (first occurrence)
      entry = -1;

      for (i = 0; i < nr_cycles; i++) {
        if (strcmp(cycles_array[i], name) == 0) {
          // found the entry
          entry = i;
          break;
        }
      }

      // Do we need to add the entry?
      if (entry == -1) {
        strcpy(cycles_array[nr_cycles], name);
        entry = nr_cycles;
        nr_cycles++;

        // Since we added a new entry we need to create a NAM
        fprintf(stdout, "NAM %d %d %s\n", CYCLES, CYCLES_BASE + entry,
                cycles_array[entry]);
      }

      // fill in the value
      // add a TIM and a CYCLE
      fprintf(stdout, "TIM %lld\n", timestamp);
      fprintf(stdout, "VAL %d %d %lld\n", CYCLES, CYCLES_BASE + entry,
              value * 10000000LL);

      return;
    }
  }

  /*
   * Treat everything else as a NOTES entry
   *
   */
  nr_note_entries++;

  // if the entry has not been seen before, add a new entry for it and issue a
  // NAM
  entry = -1;
  for (i = 0; i < nr_notes; i++) {
    char *pos;
    char comparestr[1024];

    strcpy(comparestr, text);
    /*
     * the portion of the text before the first space in the text
     * is considered the unique part of the text
     */
    pos = strchr(comparestr, ' ');
    if (pos) {
      *pos = 0;
    }

    if (strcmp(notes_array[i], comparestr) == 0) {
      // found the entry
      entry = i;
      break;
    }
  }

  // Do we need to add the entry?
  if (entry == -1) {
    int len;
    char *pos;

    pos = strchr(text, ' ');
    if (pos)
      len = pos - text;
    else
      len = strlen(text);

    strncpy(notes_array[nr_notes], text, len);
    notes_array[nr_notes][len] = 0;

    entry = nr_notes;
    nr_notes++;

    // Since we added a new entry we need to create a NAM, with only the
    // first part of the text
    fprintf(stdout, "NAM %d %d %s\n", NOTES, NOTES_BASE + entry,
            notes_array[entry]);
  }

  // Add the OCC entry
  fprintf(stdout, "OCC %d %d %lld\n", NOTES, NOTES_BASE + entry, timestamp);

  // Add the DSC entry, replace any spaces with commas
  sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
  for (i = 8; i < (int)strlen(fullentry); i++) {
    if (fullentry[i] == 32)
      fullentry[i] = 0x2c;
  }
  fwrite(fullentry, strlen(fullentry), 1, stdout);
}

typedef struct {
  char filename[128];
  char procname[64];
  int pid;
  char *bufmmapped;
  int bufsize;
  char *ptr;
  unsigned int *dword_ptr;
  _u64 timeofday_start;
  _u64 monotonic_start;
  _u64 monotonic_first;
  _u64 monotonic_timestamp;
  unsigned short identifier;
  int nr_numbers;
  unsigned int *numbers;
  char *text;
  int text_len;
  int tid;
  int valid;
} tracebuffer_t;

static tracebuffer_t tracebuffers[10];

static void parse(int bid) {
  /*
   * [    ]marker, lower 2 bytes is total length in dwords, upper byte is
   * identifier,
   *       middle byte is nr numbers
   * [    ]clock_monotonic_timestamp.tv_nsec
   * [    ]clock_monotonic_timestamp.tv_sec
   * [    ]<optional> numbers
   * [    ]<optional> text, padded with 0's to multiple of 4 bytes
   */

  // fprintf(stderr, "parse %d, tracebuffers[bid].dword_ptr = 0x%08x\n", bid,
  //       (int)tracebuffers[bid].dword_ptr);

  tracebuffers[bid].valid = 0;

  unsigned int *p = tracebuffers[bid].dword_ptr;
  unsigned int marker = *p++;

  tracebuffers[bid].identifier = marker >> 24;
  tracebuffers[bid].dword_ptr += marker & 0xffff;

  int tvsec = *p++;
  int tvnsec = *p++;
  tracebuffers[bid].monotonic_timestamp =
      (_u64)tvsec * (_u64)1000000000 + (_u64)tvnsec;

  tracebuffers[bid].nr_numbers = (marker >> 16) & 0xff;
  tracebuffers[bid].numbers = p;
  int i = (marker >> 16) & 0xff;
  while (i--)
    p++;

  tracebuffers[bid].tid = 0;
  tracebuffers[bid].text = (char *)p;
  tracebuffers[bid].text_len =
      (((marker & 0xffff) - 3) - ((marker >> 16) & 0xff)) << 2;
  tracebuffers[bid].valid = (marker != 0);

  // fprintf(stderr,"marker[0x%08x](%d)(%d)\n", marker,
  // tracebuffers[bid].nr_numbers, tracebuffers[bid].text_len);
  // fprintf(stderr, "monotonic_timestamp:%lld, [%s]\n",
  //       tracebuffers[bid].monotonic_timestamp, tracebuffers[bid].text);
}

#if 0
static unsigned int find_process_name(char *p_processname) {
  DIR *dir_p;
  struct dirent *dir_entry_p;
  char dir_name[64];
  char target_name[128];
  int target_result;
  char exe_link[128];
  int result;

  result = 0;
  dir_p = opendir("/proc/");
  while (NULL != (dir_entry_p = readdir(dir_p))) {
    if (strspn(dir_entry_p->d_name, "0123456789") ==
        strlen(dir_entry_p->d_name)) {
      strcpy(dir_name, "/proc/");
      strcat(dir_name, dir_entry_p->d_name);
      strcat(dir_name, "/");
      exe_link[0] = 0;
      strcat(exe_link, dir_name);
      strcat(exe_link, "exe");
      target_result = readlink(exe_link, target_name, sizeof(target_name) - 1);
      if (target_result > 0) {
        target_name[target_result] = 0;
        if (strstr(target_name, p_processname) != NULL) {
          result = atoi(dir_entry_p->d_name);
          closedir(dir_p);
          return result;
        }
      }
    }
  }
  closedir(dir_p);
  return result;
}
#endif

static void get_process_name_by_pid(const int pid, char *name) {
  char fullname[1024];
  if (name) {
    sprintf(name, "/proc/%d/cmdline", pid);

    FILE *f = fopen(name, "r");
    if (f) {
      size_t size;
      size = fread(fullname, sizeof(char), 1024, f);

      if (size > 0) {
        if ('\n' == fullname[size - 1])
          fullname[size - 1] = '\0';

        if (strrchr(fullname, '/')) {
          strcpy(name, strrchr(fullname, '/') + 1);
        } else {
          strcpy(name, fullname);
        }
      }
      fclose(f);
    }
  }
}

static void dump_proc_self_maps(void) {
  int fd;
  int bytes;
  char proc_self_maps[16 * 1024 + 1];

  fprintf(stderr, "tdi: maps...[%s][%d]\n", gprocname, gpid);

  tditrace("MAPS [%s][%d] begin", gprocname, gpid);

  fd = open("/proc/self/maps", O_RDONLY);
  if (fd < 0) {
    tditrace("MAPS [%s][%d] end", gprocname, gpid);
    return;
  }

  while (1) {
    bytes = read(fd, proc_self_maps, sizeof(proc_self_maps) - 1);
    if ((bytes == -1) && (errno == EINTR))
      /* keep trying */;
    else if (bytes > 0) {
      proc_self_maps[bytes] = '\0';
      char *saveptr;
      char *line = strtok_r(proc_self_maps, "\n", &saveptr);
      while (line) {
        if (strlen(line) > 50) {
          tditrace("MAPS [%s][%d] %s", gprocname, gpid, line);
        }
        line = strtok_r(NULL, "\n", &saveptr);
      }

    } else
      break;
  }

  tditrace("MAPS [%s][%d] end", gprocname, gpid);
  close(fd);
}

static void procversion(int *version, int *major, int *minor) {
  char line[256];
  FILE *f = NULL;

  if ((f = fopen("/proc/version", "r"))) {
    if (fgets(line, 255, f)) {
      sscanf(line, "Linux version: %d.%d.%d", version, major, minor);
    }
    fclose(f);
  }
}

static int gmask = 0x0;

static int do_sysinfo = 0;
static int do_selfinfo = 0;
static int do_persecond = 0;

static int monitor;

static int do_offload = 0;
static char offload_location[256] = {0};
static int offload_counter = 0;
static int offload_over50 = 0;

static int do_wrap = 0;
static int do_maps = 0;
static int do_dump_proc_self_maps = 0;

static int sample_info_init = 0;

extern void shadercapture_writeshaders(void) __attribute__((weak));
extern void texturecapture_writepngtextures(void) __attribute__((weak));
extern void texturecapture_deletetextures(void) __attribute__((weak));
extern void framecapture_writepngframes(void) __attribute__((weak));
extern void framecapture_deleteframes(void) __attribute__((weak));

static void tmpfs_message(void) {
  fprintf(stderr, "\n");
  fprintf(stderr,
          "tdi: init[%s][%d], "
          "----------------------------------------------------------------"
          "\n",
          gprocname, gpid);
  fprintf(stderr, "tdi: init[%s][%d], adjust the trace buffer size:\n",
          gprocname, gpid);
  fprintf(stderr, "tdi: init[%s][%d],     \"TRACEBUFFERSIZE=<MB>\"\n",
          gprocname, gpid);
  fprintf(stderr, "tdi: init[%s][%d], adjust the %s size:\n", gprocname, gpid,
          getenv("TMPFS") ? getenv("TMPFS") : TMPFS);
  fprintf(stderr, "tdi: init[%s][%d],     \"mount -o "
                  "remount,noexec,nosuid,nr_blocks=15000 %s\"\n",
          gprocname, gpid, getenv("TMPFS") ? getenv("TMPFS") : TMPFS);
  fprintf(stderr,
          "tdi: init[%s][%d], "
          "----------------------------------------------------------------"
          "\n",
          gprocname, gpid);
  fprintf(stderr, "\n");
}

static char procbuf[384 * 1024] __attribute__((aligned(4)));

static int readprocbuf(const char *path) {
  char *p = procbuf;
  int fd, bytes = 0;

#define BLOCKSIZE 4096

#ifdef DEBUG
  tditrace("@A+readprocbuf");
#endif

  if ((fd = open(path, O_RDONLY))) {
    while (1) {
      bytes = read(fd, p, BLOCKSIZE);
      if ((bytes == -1) && (errno == EINTR))
        /* keep trying */;
      else if (bytes > 0) {
        p[bytes] = '\0';
        p += bytes;
      } else
        break;
    }
    close(fd);
#ifdef DEBUG
    tditrace("@A-readprocbuf");
#endif
    return (int)(p - procbuf);
  }
#ifdef DEBUG
  tditrace("@A-readprocbuf");
#endif

  return 0;
}

/*
 * tdiprocvmstat
 */
int tdiprocvmstat(struct tdistructprocvmstat *s) {
#ifdef DEBUG
  tditrace("@A+do_procvmstat");
#endif

  char *buf = procbuf;
  if (readprocbuf("/proc/vmstat")) {
    char *saveptr;
    char *line = strtok_r(buf, "\n", &saveptr);

    while (line) {
      if (sscanf(line, "pswpin %d", &s->pswpin))
        ;
      else if (sscanf(line, "pswpout %d", &s->pswpout))
        ;
      else if (sscanf(line, "pgpgin %d", &s->pgpgin))
        ;
      else if (sscanf(line, "pgpgout %d", &s->pgpgout))
        ;
      else if (sscanf(line, "pgfault %d", &s->pgfault))
        ;
      else if (sscanf(line, "pgmajfault %d", &s->pgmajfault))
        break;
      line = strtok_r(NULL, "\n", &saveptr);
    }
  }

#ifdef DEBUG
  tditrace("@A-do_procvmstat");
#endif

  return 0;
}

/*
 * tdiprocmeminfo
 */
int tdiprocmeminfo(struct tdistructprocmeminfo *s) {
  char line[256];
  FILE *f = NULL;

#ifdef DEBUG
  tditrace("@A+tdiprocmeminfo");
#endif

  f = fopen("/proc/meminfo", "r");
  if (f) {
    while (fgets(line, 255, f)) {
      if (sscanf(line, "Cached: %d", &s->cached))
        break;
    }
    fclose(f);
  }
#ifdef DEBUG
  tditrace("@A-tdiprocmeminfo");
#endif

  return 0;
}

int tdiproctvbcmmeminfo(struct tdistructproctvbcmmeminfo *s) {
  char line[1024];
  FILE *f = NULL;

#ifdef DEBUG
  tditrace("@A+tdiproctvbcmmeminfo");
#endif

  s->heap0free = 0;
  s->heap1free = 0;
  f = fopen("/proc/tvbcm/meminfo", "r");
  if (f) {
    while (fgets(line, 256, f)) {
      if (&s->heap0free == 0)
        sscanf(line, "free %d", &s->heap0free);
      else if (&s->heap1free == 0)
        sscanf(line, "free %d", &s->heap1free);
    }
    fclose(f);
  }

#ifdef DEBUG
  tditrace("@A-tdiproctvbcmmeminfo");
#endif

  return 0;
}

int tdiprocstat(struct tdistructprocstat *s) {
  char line[1024];
  FILE *f = NULL;

#ifdef DEBUG
  tditrace("@A+tdiprocstat");
#endif

  if ((f = fopen("/proc/stat", "r"))) {
    if (fgets(line, 256, f)) {
#if 0
      sscanf(line, "cpu %u %u %u %u %u %u %u", &s->cpu_user, &s->cpu_nice,
             &s->cpu_system, &s->cpu_idle, &s->cpu_iowait, &s->cpu_irq,
             &s->cpu_softirq);
#endif
    }
    if (fgets(line, 256, f))
      sscanf(line, "cpu0 %u %u %u %u %u %u %u", &s->cpu0_user, &s->cpu0_nice,
             &s->cpu0_system, &s->cpu0_idle, &s->cpu0_iowait, &s->cpu0_irq,
             &s->cpu0_softirq);
    if (fgets(line, 256, f))
      sscanf(line, "cpu1 %u %u %u %u %u %u %u", &s->cpu1_user, &s->cpu1_nice,
             &s->cpu1_system, &s->cpu1_idle, &s->cpu1_iowait, &s->cpu1_irq,
             &s->cpu1_softirq);
    fclose(f);
  }

#ifdef DEBUG
  tditrace("@A-tdiprocstat");
#endif

  return 0;
}

int tdiprocselfstatm(struct tdistructprocselfstatm *s) {
  int fd;
  char buffer[65];
  int bytes;

#ifdef DEBUG
  tditrace("@A+tdiprocselfstatm");
#endif

  fd = open("/proc/self/statm", O_RDONLY);
  if (fd >= 0) {
    bytes = read(fd, buffer, 64);
    buffer[bytes] = '\0';
    sscanf(buffer, "%lu %lu", &s->vmsize, &s->rss);
  }
  close(fd);

#ifdef DEBUG
  tditrace("@A-tdiprocselfstatm");
#endif

  return 0;
}

int tdiprocselfstatus(struct tdistructprocselfstatus *s) {
  int fd;
  int bytes;
  static char proc_self_status[16 * 1024 + 1];

#ifdef DEBUG
  tditrace("@A+tdiprocselfstatus");
#endif

  fd = open("/proc/self/status", O_RDONLY);
  if (fd >= 0) {
    while (1) {
      bytes = read(fd, proc_self_status, sizeof(proc_self_status) - 1);
      if ((bytes == -1) && (errno == EINTR))
        /* keep trying */;
      else if (bytes > 0) {
        proc_self_status[bytes] = '\0';
        char *saveptr;
        char *line = strtok_r(proc_self_status, "\n", &saveptr);
        while (line) {
          if (sscanf(line, "VmSwap: %d", &s->vmswap))
            break;
          line = strtok_r(NULL, "\n", &saveptr);
        }
      } else
        break;
    }
    close(fd);
  }

#ifdef DEBUG
  tditrace("@A+tdiprocselfstatus");
#endif

  return 0;
}

/*
          1111111111222222222233333333
01234567890123456789012345678901234567

8007b000-80196000 r-xp 00000000 08:02 1418       /lib/systemd/systemd
Size:               1132 kB
Rss:                1044 kB
Pss:                1044 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:      1044 kB
Private_Dirty:         0 kB
Referenced:         1044 kB
Anonymous:             0 kB
AnonHugePages:         0 kB
Swap:                  0 kB
KernelPageSize:        4 kB
MMUPageSize:           4 kB
Locked:                0 kB
VmFlags: rd ex mr mw me dw

2b21c000-2b21d000 rw-p 00000000 00:00 0
Size:                  4 kB
Rss:                   0 kB
Pss:                   0 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         0 kB
Referenced:            0 kB
Anonymous:             0 kB
Swap:                  0 kB
KernelPageSize:        4 kB
MMUPageSize:           4 kB

*/

#define _Rss (1 * 28 + 19 + 4)
#define _Ref (7 * 28 + 19 + 4)

#if defined(__i386__)
#define _Swap (10 * 28 + 19 + 4)
#elif defined(__mips__)
#define _Swap (9 * 28 + 19 + 4)
#else
#define _Swap (10 * 28 + 19 + 4)
#endif

#if defined(__i386__)
#define _Block (15 * 28)
#elif defined(__mips__)
#define _Block (12 * 28)
#else
#define _Block (15 * 28)
#endif

#define I(n) (p[n] == ' ' ? 0 : p[n] - '0')

#define I5(n)                                                                  \
  I(n) * 10000 + I(n + 1) * 1000 + I(n + 2) * 100 + I(n + 3) * 10 + I(n + 4);

#define I5r(n)                                                                 \
  I(n) + I(n - 1) * 10 + I(n - 2) * 100 + I(n - 3) * 1000 + I(n - 4) * 10000;

#define II(val, idx)                                                           \
  if (p[idx] != ' ') {                                                         \
    val += (p[idx] - '0');                                                     \
    if (p[idx - 1] != ' ') {                                                   \
      val += ((p[idx - 1] - '0') * 10);                                        \
      if (p[idx - 2] != ' ') {                                                 \
        val += ((p[idx - 2] - '0') * 100);                                     \
        if (p[idx - 3] != ' ') {                                               \
          val += ((p[idx - 3] - '0') * 1000);                                  \
          if (p[idx - 4] != ' ') {                                             \
            val += ((p[idx - 4] - '0') * 10000);                               \
          }                                                                    \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

int tdiprocsmaps(const char *pathname, tdistructprocsmaps *s) {
  int bufbytes;

#ifdef DEBUG
  tditrace("@A+tdiprocsmaps");
#endif

  char *p = procbuf;
  if ((bufbytes = readprocbuf(pathname))) {
    s->code_rss = 0;
    s->code_pss = 0;
    s->code_ref = 0;
    s->data_rss = 0;
    s->data_pss = 0;
    s->data_ref = 0;
    s->data_swap = 0;

    while (((unsigned int)(p - procbuf) != (unsigned int)bufbytes)) {
#ifdef DEBUG
      tditrace("@A+scan");
#endif

      // fprintf(stderr, "1[%c%c%c%c]\n", p[18], p[19], p[20], p[21]);

      // r-xp
      if ((p[18] == 'r') && (p[19] == '-') && (p[20] == 'x') &&
          (p[21] == 'p')) {
        p += 36;
        while (*p++ != 0x0a)
          ;

        II(s->code_rss, _Rss);
        II(s->code_ref, _Ref);

        // r--p rw-p rwxp
      } else if ((p[18] == 'r') && (p[21] == 'p')) {
        p += 36;
        while (*p++ != 0x0a)
          ;

        II(s->data_rss, _Rss);
        II(s->data_ref, _Ref);
        II(s->data_swap, _Swap);

      } else {
        p += 36;
        while (*p++ != 0x0a)
          ;
      }

#if defined(__i386__)
      p += _Block - 28;
      p += 10;
      while (*p++ != 0x0a)
        ;
#elif defined(__mips__)
      p += _Block;
#else
      p += _Block - 28;
      p += 10;
      while (*p++ != 0x0a)
        ;
#endif

#ifdef DEBUG
      tditrace("@A-scan");
#endif
    }
  }

#ifdef DEBUG
  tditrace("@A-tdiprocsmaps");
#endif

  return 0;
}

int tdiprocdiskstats(struct tdistructprocdiskstats s[], const char *disks,
                     int *nrdisks) {
  char dsks[256];
  char *pt;
  char *saveptr;

  *nrdisks = 0;
  if (!disks) {
    return 0;
  }

#ifdef DEBUG
  tditrace("@A+tdiprocdiskstats");
#endif

  strcpy(dsks, disks);

  pt = strtok_r(dsks, ",", &saveptr);

  while (pt != NULL) {
    strcpy(s[*nrdisks].name, pt);
    sprintf(s[*nrdisks].match, "%s %%u %%u %%u %%u %%u %%u %%u %%u", pt);
    pt = strtok_r(NULL, ",", &saveptr);
    (*nrdisks)++;
  }

  if (*nrdisks) {
    int d;
    char *buf = procbuf;
    if (readprocbuf("/proc/diskstats")) {
      char *saveptr;
      char *line = strtok_r(buf, "\n", &saveptr);
      while (line) {
        for (d = 0; d < *nrdisks; d++) {
          char *pos;
          struct tdistructprocdiskstats *ps = &s[d];
          if ((pos = strstr(line, ps->name))) {
            sscanf(pos, ps->match, &ps->reads, &ps->reads_merged,
                   &ps->reads_sectors, &ps->reads_time, &ps->writes,
                   &ps->writes_merged, &ps->writes_sectors, &ps->writes_time);
            break;
          }
        }

        line = strtok_r(NULL, "\n", &saveptr);
      }
    }
  }
#ifdef DEBUG
  tditrace("@A-tdiprocdiskstats");
#endif

  return 0;
}

int tdiprocnetdev(struct tdistructprocnetdev s[], const char *nets,
                  int *nrnets) {
  char nts[256];
  char *pt;
  char *saveptr;

  *nrnets = 0;
  if (!nets) {
    return 0;
  }

#ifdef DEBUG
  tditrace("@A+tdiprocnetdev");
#endif

  strcpy(nts, nets);
  pt = strtok_r(nts, ",", &saveptr);
  while (pt != NULL) {
    strcpy(s[*nrnets].name, pt);
    sprintf(s[*nrnets].match,
            "%s: %%lu %%u %%u %%u %%u %%u %%u %%u %%lu %%u %%u %%u %%u %%u %%u "
            "%%u",
            pt);
    pt = strtok_r(NULL, ",", &saveptr);
    (*nrnets)++;
  }

  if (*nrnets) {
    char line[1024];
    FILE *f = NULL;
    int n;
    if ((f = fopen("/proc/net/dev", "r"))) {
      while (fgets(line, 256, f)) {
        for (n = 0; n < *nrnets; n++) {
          char *pos;
          struct tdistructprocnetdev *pn = &s[n];
          if ((pos = strstr(line, pn->name))) {
            sscanf(pos, pn->match, &pn->r_bytes, &pn->r_packets, &pn->r_errs,
                   &pn->r_drop, &pn->r_fifo, &pn->r_frame, &pn->r_compressed,
                   &pn->r_multicast, &pn->t_bytes, &pn->t_packets, &pn->t_errs,
                   &pn->t_drop, &pn->t_fifo, &pn->t_frame, &pn->t_compressed,
                   &pn->t_multicast);
            break;
          }
        }
      }
      fclose(f);
    }
  }

#ifdef DEBUG
  tditrace("@A-tdiprocnetdev");
#endif

  return 0;
}

void tdiproctest1(void) {
#ifdef DEBUG
  tditrace("@A+tdiproctest1");
#endif

  char *buf = procbuf;
  if (readprocbuf("/proc/self/smaps")) {
    char *saveptr;
    char *line = strtok_r(buf, "\n", &saveptr);
    while (line) {
      unsigned long start, end;
      char flag_r, flag_w, flag_x, flag_s;
      unsigned long long pgoff;
      unsigned int maj, min;
      unsigned long ino, dum;
      char name[512];
      int n;

      if (sscanf(line, "%08lx-%08lx", &dum, &dum) == 2) {
        strcpy(name, "ANONYMOUS");
        if ((n = sscanf(line, "%08lx-%08lx %c%c%c%c %08llx %02x:%02x %lu %s",
                        &start, &end, &flag_r, &flag_w, &flag_x, &flag_s,
                        &pgoff, &maj, &min, &ino, name)) >= 10) {
        }
      }
      int rss, pss, sc, sd, pc, pd, ref, anon, swap;
      sscanf(line, "Rss:%d", &rss);
      sscanf(line, "Pss:%d", &pss);
      sscanf(line, "Shared_Clean:%d", &sc);
      sscanf(line, "Shared_Dirty:%d", &sd);
      sscanf(line, "Private_Clean:%d", &pc);
      sscanf(line, "Private_Dirty:%d", &pd);
      sscanf(line, "Referenced:%d", &ref);
      sscanf(line, "Anonymous:%d", &anon);
      sscanf(line, "Swap:%d", &swap);
      if (strstr(line, "Swap")) {
        if (swap) {
          char *nm = strrchr(name, '/');
          tditrace("_swap_ %s %d", nm ? nm + 1 : name, swap);
        }
      }
      line = strtok_r(NULL, "\n", &saveptr);
    }
  }

#ifdef DEBUG
  tditrace("@A-tdiproctest1");
#endif
}

static void sample_info(void) {
  static int do_structsysinfo;
  static int do_structmallinfo;
  static int do_structgetrusage;

  static int do_procvmstat;
  static int do_procmeminfo;
  static int do_proctvbcmmeminfo;
  static int do_procstat;
  static int do_procdiskstats;
  static int do_procnetdev;

  static int do_procselfstatm;
  static int do_procselfstatus;
  static int do_procselfsmaps;

  if (!sample_info_init) {
    sample_info_init = 1;

    tditrace("%P", gpid, gprocname);

    char *e;
    if ((e = getenv("DISKS"))) {
      tditrace("%m%s", DISKSLIST, e);
    }

    if ((e = getenv("NETS"))) {
      tditrace("%m%s", NETSLIST, e);
    }

    do_structsysinfo = do_sysinfo;
    do_structmallinfo = do_selfinfo;
    do_structgetrusage = 1; // do_selfinfo;

    do_procvmstat = do_sysinfo;
    do_procmeminfo = do_sysinfo;
    do_proctvbcmmeminfo = do_sysinfo;
    do_procstat = do_sysinfo;
    do_procdiskstats = do_sysinfo;
    do_procnetdev = do_sysinfo;

    do_procselfstatm = do_selfinfo;
    do_procselfstatus = 0; // do_selfinfo;
    do_procselfsmaps = do_selfinfo;
  }

  struct sysinfo si;
  if (do_structsysinfo) {
#ifdef DEBUG
    tditrace("@A+tdistructsysinfo");
#endif

    sysinfo(&si);

// struct sysinfo {
//         long uptime;             /* Seconds since boot */
//         unsigned long loads[3];  /* 1, 5, and 15 minute load averages
//         */
//         unsigned long totalram;  /* Total usable main memory size */
//         unsigned long freeram;   /* Available memory size */
//         unsigned long sharedram; /* Amount of shared memory */
//         unsigned long bufferram; /* Memory used by buffers */
//         unsigned long totalswap; /* Total swap space size */
//         unsigned long freeswap;  /* Swap space still available */
//         unsigned short procs;    /* Number of current processes */
//         unsigned long totalhigh; /* Total high memory size */
//         unsigned long freehigh;  /* Available high memory size */
//         unsigned int mem_unit;   /* Memory unit size in bytes */
//         char _f[20-2*sizeof(long)-sizeof(int)];
//                                  /* Padding to 64 bytes */
//     };

#ifdef DEBUG
    tditrace("@A-tdistructsysinfo");
#endif
  }

  struct mallinfo mi;
  if (do_structmallinfo) {
#ifdef DEBUG
    tditrace("@A+tdistructmallinfo");
#endif

    mi = mallinfo();

// struct mallinfo {
//       int arena;     /* Non-mmapped space allocated (bytes) */
//       int ordblks;   /* Number of free chunks */
//       int smblks;    /* Number of free fastbin blocks */
//       int hblks;     /* Number of mmapped regions */
//       int hblkhd;    /* Space allocated in mmapped regions
//       (bytes) */
//       int usmblks;   /* Maximum total allocated space (bytes) */
//       int fsmblks;   /* Space in freed fastbin blocks (bytes) */
//       int uordblks;  /* Total allocated space (bytes) */
//       int fordblks;  /* Total free space (bytes) */
//       int keepcost;  /* Top-most, releasable space (bytes) */
//    };

#ifdef DEBUG
    tditrace("@A-tdistructmallinfo");
#endif
  }

  struct rusage ru;
  if (do_structgetrusage) {
#ifdef DEBUG
    tditrace("@A+tdistructgetrusage");
#endif

    getrusage(RUSAGE_SELF, &ru);

// struct rusage {
//       struct timeval ru_utime; /* user CPU time used */
//       struct timeval ru_stime; /* system CPU time used */
//       long   ru_maxrss;        /* maximum resident set size */
//       long   ru_ixrss;         /* integral shared memory size */
//       long   ru_idrss;         /* integral unshared data size */
//       long   ru_isrss;         /* integral unshared stack size */
//       long   ru_minflt;        /* page reclaims (soft page faults) */
//       long   ru_majflt;        /* page faults (hard page faults) */
//       long   ru_nswap;         /* swaps */
//       long   ru_inblock;       /* block input operations */
//       long   ru_oublock;       /* block output operations */
//       long   ru_msgsnd;        /* IPC messages sent */
//       long   ru_msgrcv;        /* IPC messages received */
//       long   ru_nsignals;      /* signals received */
//       long   ru_nvcsw;         /* voluntary context switches */
//       long   ru_nivcsw;        /* involuntary context switches */
//   };

#ifdef DEBUG
    tditrace("@A-tdistructgetrusage");
#endif
  }

  /*
   * procvmstat
   */
  struct tdistructprocvmstat vmstat;
  if (do_procvmstat) {
    tdiprocvmstat(&vmstat);
  }

  /*
   * procmeminfo
   */
  struct tdistructprocmeminfo meminfo;
  if (do_procmeminfo) {
    tdiprocmeminfo(&meminfo);
  }

  /*
   * proctvbcmmeminfo
   */
  struct tdistructproctvbcmmeminfo tvbcmmeminfo;
  if (do_proctvbcmmeminfo) {
    tdiproctvbcmmeminfo(&tvbcmmeminfo);
  }

  /*
   * procstat
   */
  struct tdistructprocstat procstat;
  if (do_procstat) {
    tdiprocstat(&procstat);

    int c_numbers[8];
    c_numbers[0] = procstat.cpu0_user + procstat.cpu0_nice;
    c_numbers[1] = procstat.cpu0_system;
    c_numbers[2] = procstat.cpu0_iowait;
    c_numbers[3] = procstat.cpu0_irq + procstat.cpu0_softirq;
    c_numbers[4] = procstat.cpu1_user + procstat.cpu1_nice;
    c_numbers[5] = procstat.cpu1_system;
    c_numbers[6] = procstat.cpu1_iowait;
    c_numbers[7] = procstat.cpu1_irq + procstat.cpu1_softirq;
    tditrace("%C", c_numbers);
  }

  /*
   * procselfstatm
   */
  struct tdistructprocselfstatm procselfstatm;
  if (do_procselfstatm) {
    tdiprocselfstatm(&procselfstatm);
  }

  /*
   * procselfsmaps
   */
  struct tdistructprocsmaps procselfsmaps;
  if (do_procselfsmaps) {
    tdiprocsmaps("/proc/self/smaps", &procselfsmaps);
  }

  /*
   * procselfstatus
   */
  struct tdistructprocselfstatus procselfstatus;
  if (do_procselfstatus) {
    tdiprocselfstatus(&procselfstatus);
  }

  /*
   * procdiskstats
   */
  struct tdistructprocdiskstats procdiskstats[6];
  int nrdisks;
  if (do_procdiskstats) {
    tdiprocdiskstats(procdiskstats, getenv("DISKS"), &nrdisks);
  }

  /*
   * procnetdev
   */
  struct tdistructprocnetdev procnetdev[4];
  int nrnets;
  if (do_procnetdev) {
    tdiprocnetdev(procnetdev, getenv("NETS"), &nrnets);
  }

#if 0
  tdiproctest1();
#endif

  if (do_sysinfo) {
    int m_numbers[10];
    m_numbers[0] = (unsigned int)((si.freeram / 1024) * si.mem_unit);
    m_numbers[1] = (unsigned int)((si.bufferram / 1024) * si.mem_unit);
    m_numbers[2] = (unsigned int)meminfo.cached;
    m_numbers[3] =
        (unsigned int)(((si.totalswap - si.freeswap) / 1024) * si.mem_unit);
    m_numbers[4] = (unsigned int)vmstat.pswpout * 4;
    m_numbers[5] = (unsigned int)vmstat.pswpin * 4;
    m_numbers[6] = (unsigned int)vmstat.pgpgin;
    m_numbers[7] = (unsigned int)vmstat.pgpgout;
    m_numbers[8] = (unsigned int)vmstat.pgfault;
    m_numbers[9] = (unsigned int)vmstat.pgmajfault;
    tditrace("%M", m_numbers);

    // tditrace("HEAP0FREE~%u", (unsigned int)(heap0free / 1024));
    // tditrace("HEAP1FREE~%u", (unsigned int)(heap0free / 1024));

    unsigned int d_numbers[8];
    if (nrdisks >= 1) {
      d_numbers[0] = procdiskstats[0].reads_sectors;
      d_numbers[1] = procdiskstats[0].writes_sectors;
    }
    if (nrdisks >= 2) {
      d_numbers[2] = procdiskstats[1].reads_sectors;
      d_numbers[3] = procdiskstats[1].writes_sectors;
    }
    if (nrdisks >= 3) {
      d_numbers[4] = procdiskstats[2].reads_sectors;
      d_numbers[5] = procdiskstats[2].writes_sectors;
    }
    if (nrdisks >= 4) {
      d_numbers[6] = procdiskstats[3].reads_sectors;
      d_numbers[7] = procdiskstats[3].writes_sectors;
    }
    if (nrdisks)
      tditrace("%D", d_numbers);

    unsigned int n_numbers[4];
    if (nrnets >= 1) {
      n_numbers[0] = procnetdev[0].r_packets;
      n_numbers[1] = procnetdev[0].t_packets;
    }
    if (nrnets >= 2) {
      n_numbers[2] = procnetdev[1].r_packets;
      n_numbers[3] = procnetdev[1].t_packets;
    }
    if (nrnets)
      tditrace("%N", n_numbers);
  }

  if (do_selfinfo) {
    unsigned int s_numbers[11];
    s_numbers[0] = gpid;
    s_numbers[1] = procselfstatm.rss * 4;
    s_numbers[2] = procselfsmaps.code_rss;
    s_numbers[3] = procselfsmaps.code_ref;
    s_numbers[4] = procselfsmaps.data_rss;
    s_numbers[5] = procselfsmaps.data_ref;
    s_numbers[6] = procselfsmaps.data_swap;
    s_numbers[7] = mi.arena / 1024;
    s_numbers[8] = mi.hblkhd / 1024;
    s_numbers[9] = (unsigned int)ru.ru_minflt;
    s_numbers[10] = (unsigned int)ru.ru_majflt;
    tditrace("%S", s_numbers);
  }
}

static void *socket_thread(void *param) {
  sprintf(gsocket_path, (char *)"%stditracesocket@%s@%d",
          getenv("TMPFS") ? getenv("TMPFS") : TMPFS, gprocname, gpid);

  struct sockaddr_un addr;
  char buf[100];
  int fd, cl, rc;

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "socket error\n");
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  if (*gsocket_path == '\0') {
    *addr.sun_path = '\0';
    strncpy(addr.sun_path + 1, gsocket_path + 1, sizeof(addr.sun_path) - 2);
  } else {
    strncpy(addr.sun_path, gsocket_path, sizeof(addr.sun_path) - 1);
    unlink(gsocket_path);
  }

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    fprintf(stderr, "bind error\n");
  }

  if (listen(fd, 5) == -1) {
    fprintf(stderr, "listen error\n");
  }

  while (1) {
    // fprintf(stdout,"+accept...\n");
    if ((cl = accept(fd, NULL, NULL)) == -1) {
      fprintf(stderr, "accept error\n");
      continue;
    }

    rc = read(cl, buf, sizeof(buf));
    fprintf(stdout, "tdi: read[%s][%d], \"%s\"\n", gprocname, gpid, buf);

    if (rc == -1) {
      fprintf(stdout, "read error\n");
    } else if (rc == 0) {
      // fprintf(stdout, "EOF\n");
      close(cl);
    } else {
      buf[rc] = 0;

      if (strstr(buf, "rewind")) {
        rc = sprintf(buf, "%s", "rewinding");
        write(cl, buf, rc);

        fprintf(stderr, "tdi: rewinding...[%s][%d]\n", gprocname, gpid);
        tditrace_rewind();
        do_dump_proc_self_maps = 1;
        sample_info_init = 0;

      } else if (strstr(buf, "version")) {
        rc = sprintf(buf, "v%s (%s %s)", VERSION, __DATE__, __TIME__);
        write(cl, buf, rc);

      } else if (strstr(buf, "shaders")) {
        rc = sprintf(buf, "dumping shaders");
        write(cl, buf, rc);

        fprintf(stderr, "tdi: dump shaders...[%s][%d]\n", gprocname, gpid);
        if (shadercapture_writeshaders != NULL) {
          shadercapture_writeshaders();
        }
      } else if (strstr(buf, "textures")) {
        rc = sprintf(buf, "dumping ");
        write(cl, buf, rc);

        fprintf(stderr, "tdi: dump textures...[%s][%d]\n", gprocname, gpid);

        if (texturecapture_writepngtextures != NULL) {
          texturecapture_writepngtextures();
        }
        if (texturecapture_deletetextures != NULL) {
          texturecapture_deletetextures();
        }
      } else if (strstr(buf, "frames")) {
        rc = sprintf(buf, "dumping frames");
        write(cl, buf, rc);

        if (framecapture_writepngframes != NULL) {
          framecapture_writepngframes();
        }
        if (framecapture_deleteframes != NULL) {
          framecapture_deleteframes();
        }
      } else {
        tditrace("%s", buf);

        rc = sprintf(buf,
                     "unrecognized message, created a tracepoint for it, not "
                     "taking other action");
        write(cl, buf, rc);
      }
      close(cl);
    }
  }

  return 0;
}

static void offload() {
  static char offloadfilename[256];
  static char *offload_buffer;
  static FILE *offload_file;

  if (!offload_over50) {
    LOCK();
    int check = (((unsigned int)gtrace_buffer_byte_ptr -
                  (unsigned int)gtrace_buffer) > (gtracebuffersize / 2));
    UNLOCK();

    if (check) {
      offload_over50 = 1;
      fprintf(stderr, "tdi: at 50%%...[%d,%s]\n", gpid, gprocname);
      // create a new file and fill with 0..50% data

      offload_counter++;

      sprintf(offloadfilename, (char *)"%s/tditracebuffer@%s@%d@%04d",
              offload_location, gprocname, gpid, offload_counter);

      if ((offload_file = fopen(offloadfilename, "w+")) == 0) {
        int errsv = errno;
        fprintf(stderr, "Error creating file \"%s\" [%d]\n", offloadfilename,
                errsv);
      }

#ifndef __UCLIBC__
      if (posix_fallocate(fileno(offload_file), 0, gtracebuffersize) != 0) {
        fprintf(stderr,
                "tdi: [%d][%s], !!! failed to resize offloadfile: \"%s\" \n",
                gpid, gprocname, offloadfilename);
        tmpfs_message();

        fclose(offload_file);
        unlink(offloadfilename);

        pthread_exit(NULL);
      }
#else
      if (ftruncate(fileno(offload_file), gtracebuffersize) == -1) {
        fprintf(stderr,
                "tdi: [%d][%s], !!! failed to resize offloadfile: \"%s\" \n",
                gpid, gprocname, offloadfilename);
        pthread_exit(NULL);
        tmpfs_message();
      }
#endif

      offload_buffer = (char *)mmap(0, gtracebuffersize, PROT_WRITE, MAP_SHARED,
                                    fileno(offload_file), 0);

      if (offload_buffer == MAP_FAILED) {
        fprintf(stderr,
                "tdi: [%d][%s], !!! failed to mmap offloadfile: \"%s\" \n",
                gpid, gprocname, offloadfilename);
        pthread_exit(NULL);
      }

      fprintf(stderr, "tdi: [%d][%s], created offloadfile: \"%s\" \n", gpid,
              gprocname, offloadfilename);

      memcpy(offload_buffer, gtrace_buffer, gtracebuffersize / 2);

      fprintf(stderr, "tdi: [%d,%s], copied 0..50%% to "
                      "offloadfile: \"%s\"\n",
              gpid, gprocname, offloadfilename);
    }

  } else {
    LOCK();
    int check = (((unsigned int)gtrace_buffer_byte_ptr -
                  (unsigned int)gtrace_buffer) < (gtracebuffersize / 2));
    UNLOCK();

    if (check) {
      offload_over50 = 0;
      fprintf(stderr, "tdi: [%d,%s], at 100%%...\n", gpid, gprocname);
      // fill remaining 50..100% data to existing file and close
      // file

      memcpy(offload_buffer + gtracebuffersize / 2,
             gtrace_buffer + gtracebuffersize / 2, gtracebuffersize / 2);
      munmap(offload_buffer, gtracebuffersize);
      fclose(offload_file);

      fprintf(stderr, "tdi: [%d,%s], copied 50..100%% to "
                      "offloadfile: \"%s\"\n",
              gpid, gprocname, offloadfilename);
    }
  }
}

static void *monitor_thread(void *param) {
  if (do_maps) {
    sample_info();
    usleep(1 * 1000 * 1000);
    dump_proc_self_maps();
  }

  fprintf(stderr, "tdi: monitoring[%s][%d]\n", gprocname, gpid);
  while (1) {
    if (do_maps) {
      if (do_dump_proc_self_maps) {
        dump_proc_self_maps();
        do_dump_proc_self_maps = 0;
      }
    }

    // tditrace("@A+sample");
    sample_info();
    // tditrace("@A-sample");

    if (do_offload) {
      offload();
    }

    usleep(1000 * 1000 / do_persecond);
  }

  pthread_exit(NULL);
}

static int create_trace_buffer(void) {
  /*
   * [TDIT]
   * [RACE]
   * [    ]timeofday_start.tv_usec
   * [    ]timeofday_start.tv_sec
   * [    ]clock_monotonic_start.tv_nsec
   * [    ]clock_monotonic_start.tv_sec
   * ------
   * [    ]marker, lower 2 bytes is total length in dwords, upper 2bytes is nr
   * numbers
   * [    ]clock_monotonic_timestamp.tv_nsec
   * [    ]clock_monotonic_timestamp.tv_sec
   * [    ]text, padded with 0 to multiple of 4 bytes
   * ...
   * ------
   */
  sprintf(gtracebufferfilename, (char *)"%s/tditracebuffer@%s@%d",
          getenv("TMPFS") ? getenv("TMPFS") : TMPFS, gprocname, gpid);
  FILE *file;
  if ((file = fopen(gtracebufferfilename, "w+")) == 0) {
    fprintf(stderr, "tdi: [%s][%d], !!! failed to create \"%s\"\n", gprocname,
            gpid, gtracebufferfilename);
    return -1;
  }

#ifndef __UCLIBC__
  if (posix_fallocate(fileno(file), 0, gtracebuffersize) != 0) {
    fprintf(stderr, "tdi: [%s][%d], !!! failed to resize \"%s\" (%dMB)\n",
            gprocname, gpid, gtracebufferfilename,
            gtracebuffersize / (1024 * 1024));
    tmpfs_message();

    fclose(file);
    unlink(gtracebufferfilename);

    return -1;
  }
#else
  if (ftruncate(fileno(file), gtracebuffersize) == -1) {
    fprintf(stderr, "tdi: [%s][%d], !!! failed to resize \"%s\" (%dMB)\n",
            gprocname, gpid, gtracebufferfilename,
            gtracebuffersize / (1024 * 1024));

    tmpfs_message();
    return -1;
  }
#endif

  gtrace_buffer = (char *)mmap(0, gtracebuffersize, PROT_READ | PROT_WRITE,
                               MAP_SHARED, fileno(file), 0);

  if (gtrace_buffer == MAP_FAILED) {
    fprintf(stderr, "tdi: [%s][%d], !!! failed to mmap \"%s\" (%dMB)\n",
            gprocname, gpid, gtracebufferfilename,
            gtracebuffersize / (1024 * 1024));
    return -1;
  }

  stat(gtracebufferfilename, &gtrace_buffer_st);

  unsigned int i;
  for (i = 0; i < gtracebuffersize; i++) {
    gtrace_buffer[i] = 0;
  }

  fprintf(stderr, "tdi: init[%s][%d], allocated \"%s\" (%dMB)\n", gprocname,
          gpid, gtracebufferfilename, gtracebuffersize / (1024 * 1024));

  gtrace_buffer_byte_ptr = gtrace_buffer;
  gtrace_buffer_dword_ptr = (unsigned int *)gtrace_buffer;

  /*
   * write one time start text
   */
  sprintf((char *)gtrace_buffer_dword_ptr, (char *)"TDITRACE");
  gtrace_buffer_dword_ptr += 2;

  unsigned int *p = gtrace_buffer_dword_ptr;

  gettimeofday((struct timeval *)gtrace_buffer_dword_ptr, 0);
  gtrace_buffer_dword_ptr += 2;

  clock_gettime(CLOCK_MONOTONIC, (struct timespec *)gtrace_buffer_dword_ptr);
  gtrace_buffer_dword_ptr += 2;

  _u64 atimeofday_start = (_u64)*p++ * 1000000000;
  atimeofday_start += (_u64)*p++ * 1000;

  _u64 amonotonic_start = (_u64)*p++ * 1000000000;
  amonotonic_start += (_u64)*p++;

  /*
  fprintf(stderr,
          "tdi: [%s][%d], timeofday_start:%lld, "
          "monotonic_start:%lld\n",
          gprocname, gpid, atimeofday_start, amonotonic_start);
  */

  /*
   * rewind ptr is used for offloading set to after start timestamps
   */
  gtrace_buffer_rewind_ptr = gtrace_buffer_dword_ptr;
  *gtrace_buffer_dword_ptr = 0;

  reported_full = 0;

  LOCK();
  gtditrace_inited = 1;
  UNLOCK();

  return 0;
}

static int thedelay;

static void start_monitor_thread(void);

static void *delayed_init_thread(void *param) {
  int *pdelay = (int *)param;
  int delay = *pdelay;

  fprintf(stderr, "tdi: init[%s][%d], delay is %d\n", gprocname, gpid, delay);

  if (delay == -1) {
    /*
     * wait for timeofday is 'today'
     */
    while (1) {
      struct timeval tv;
      struct tm *ptm;
      char time_string[40];
      gettimeofday(&tv, NULL);
      ptm = localtime(&tv.tv_sec);
      strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm);

      if (tv.tv_sec > (45 * 365 * 24 * 3600)) {
        fprintf(stderr,
                "tdi: init[%s][%d], delay until timeofday is set, \"%s\", "
                "timeofday is set\n",
                gprocname, gpid, time_string);
        break;
      }

      ptm = localtime(&tv.tv_sec);
      strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm);
      fprintf(stderr,
              "tdi: init[%s][%d], delay until timeofday is set, \"%s\", "
              "timeofday "
              "is not set\n",
              gprocname, gpid, time_string);

      usleep(1 * 1000000);
    }
  }

  else if (delay == -2) {
    /*
     * wait until tracebuffer modification time is changed.
     */

    while (1) {
      fprintf(stderr, "tdi: init[%s][%d], paused...\n", gprocname, gpid);
      usleep(1 * 1000000);

      struct stat st;
      stat(gtracebufferfilename, &st);

      /*
      printf("%s, %d %d\n", asctime(gmtime((const
      time_t*)&st.st_atim)), st.st_atim);
      printf("%s, %d %d\n", asctime(gmtime((const
      time_t*)&st.st_mtim)), st.st_mtim);
      printf("%s, %d %d\n", asctime(gmtime((const
      time_t*)&st.st_ctim)), st.st_ctim);
      */

      if (st.st_mtim.tv_sec != gtrace_buffer_st.st_mtim.tv_sec) {
        fprintf(stderr, "tdi: init[%s][%d], started...\n", gprocname, gpid);

        stat(gtracebufferfilename, &gtrace_buffer_st);
        break;
      }
    }

  } else {
    while (delay > 0) {
      fprintf(stderr, "tdi: init[%s][%d], delay %d second(s)...\n", gprocname,
              gpid, delay);
      usleep(1 * 1000 * 000);

      delay--;
    }
  }

  fprintf(stderr, "tdi: init[%s][%d], delay finished...\n", gprocname, gpid);

  if (create_trace_buffer() == -1) {
    pthread_exit(NULL);
  }

  start_monitor_thread();
  pthread_exit(NULL);
}

static const char *instruments[] = {"console",    // 0x00000001
                                    "render",     // 0x00000002
                                    "css",        // 0x00000004
                                    "dom",        // 0x00000008
                                    "canvas",     // 0x00000010
                                    "webgl",      // 0x00000020
                                    "image",      // 0x00000040
                                    "graphics",   // 0x00000080
                                    "graphicsqt", // 0x00000100
                                    "texmap",     // 0x00000200
                                    "opengl",     // 0x00000400
                                    "qweb",       // 0x00000800
                                    "resource",   // 0x00001000
                                    "javascript", // 0x00002000
                                    "allocator"}; // 0x00004000

void start_monitor_thread(void) {
  static pthread_t monitor_thread_id;
  static pthread_t socket_thread_id;

  if (do_sysinfo | do_selfinfo | do_maps) {
    pthread_create(&monitor_thread_id, NULL, monitor_thread, &monitor);
  }

  pthread_create(&socket_thread_id, NULL, socket_thread, &monitor);
}

int tditrace_init(void) {
  unsigned int i;

  if (gtditrace_inited) {
    return 0;
  }

  gpid = getpid();
  get_process_name_by_pid(gpid, gprocname);

  if (strcmp(gprocname, "tdim") == 0) {
    if (!getenv("NOSKIPINIT")) {
#if 0
      fprintf(
          stderr,
          "tdi: init[%s][%d], procname=tdi, and no NOSKIPINIT, exiting init\n",
          gprocname, gpid);
#endif
      return -1;
    } else {
#if 0
      fprintf(stderr,
              "tdi: init[%s][%d], procname=tdi, and NOSKIPINIT, continuing "
              "init\n",
              gprocname, gpid);
#endif
    }
  }

  if (getenv("PROCONLY")) {
    if (strcmp(gprocname, getenv("PROCONLY")) != 0) {
      return -1;
    }
  }

  if (strcmp(gprocname, "mkdir") == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"mkdir\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strncmp(gprocname, "crypto", 2) == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"crypto\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strncmp(gprocname, "sh", 2) == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"sh*\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strncmp(gprocname, "tivosh", 2) == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"tivosh\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strncmp(gprocname, "bash", 2) == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"bash\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strncmp(gprocname, "ls", 2) == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"ls\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strncmp(gprocname, "genkey", 2) == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"genkey\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strncmp(gprocname, "rm", 2) == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"rm\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strcmp(gprocname, "iptables") == 0) {
    fprintf(stderr,
            "tdi: init[%s][%d], procname is \"iptables\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strcmp(gprocname, "route") == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"route\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strcmp(gprocname, "strace") == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"strace\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else if (strcmp(gprocname, "gdbserver") == 0) {
    fprintf(stderr,
            "tdi: init[%s][%d], procname is \"gdbserver\" ; not tracing\n",
            gprocname, gpid);
    return -1;
  } else {
    // fprintf(stderr, "tdi: init[%s][%d]\n", gprocname, gpid);
  }

  static int gversion, gmajor, gminor;
  procversion(&gversion, &gmajor, &gminor);

  char *env;

  if ((env = getenv("TRACEBUFFERSIZE"))) {
    gtracebuffersize = atoi(env) * 1024 * 1024;
  }

  if ((env = getenv(gprocname))) {
    gtracebuffersize = atoi(env) * 1024 * 1024;
  }

  gmask = 0x0;
  if ((env = getenv("MASK"))) {
    for (i = 0; i < sizeof(instruments) / sizeof(char *); i++) {
      if (strstr(env, instruments[i]))
        gmask |= (1 << i);
    }
    if (gmask == 0x0)
      gmask = strtoul(env, 0, 16);
  }

  if (gmask) {
    fprintf(stderr, "tdi: init[%s][%d], MASK = 0x%08x (", gprocname, gpid,
            gmask);
    int d = 0;
    for (i = 0; i < sizeof(instruments) / sizeof(char *); i++) {
      if (gmask & (1 << i)) {
        fprintf(stderr, "%s%s", d ? "+" : "", instruments[i]);
        d = 1;
      }
    }
    fprintf(stderr, ")\n");
  }

  do_persecond = 1;

  do_sysinfo = 0;
  if ((env = getenv("SYSINFO"))) {
    do_sysinfo = atoi(env);
    if (do_sysinfo > do_persecond)
      do_persecond = do_sysinfo;
  }

  do_selfinfo = 0;
  if ((env = getenv("SELFINFO"))) {
    do_selfinfo = atoi(env);
    if (do_selfinfo > do_persecond)
      do_persecond = do_selfinfo;
  }

  do_offload = 0;
  if ((env = getenv("OFFLOAD"))) {
    do_offload = 1;
    strcpy(offload_location, env);
  }

  do_wrap = 0;
  if ((env = getenv("WRAP"))) {
    do_wrap = (atoi(env) >= 1);
  }

  do_maps = 0;
  if ((env = getenv("MAPS"))) {
    do_maps = (atoi(env) >= 1);
  }

  report_tid = 0;
  if ((env = getenv("TID"))) {
    report_tid = (atoi(env) >= 1);
  }

  thedelay = 0;
  if ((env = getenv("DELAY"))) {
    thedelay = atoi(env);
  }

  /*
   * remove inactive tracefiles
   */

  int remove = 1;
  if ((env = getenv("REMOVE"))) {
    remove = (atoi(env) >= 1);
  }

  if (remove) {
    DIR *dp;
    struct dirent *ep;

    dp = opendir(getenv("TMPFS") ? getenv("TMPFS") : TMPFS);
    if (dp != NULL) {
      while ((ep = readdir(dp))) {
        if (strncmp(ep->d_name, "tditracebuffer@", 15) == 0) {
          char procpid[128];
          sprintf(procpid, (char *)"/proc/%d",
                  atoi(strrchr(ep->d_name, '@') + 1));

          char fullname[128];
          sprintf(fullname, "%s%s", getenv("TMPFS") ? getenv("TMPFS") : TMPFS,
                  ep->d_name);

          struct stat sts;

          if (stat(procpid, &sts) == -1) {
            unlink(fullname);
            fprintf(stderr, "tdi: init[%s][%d], removed \"%s\"\n", gprocname,
                    gpid, fullname);
          } else {
            fprintf(stderr, "tdi: init[%s][%d], not removed \"%s\"\n",
                    gprocname, gpid, fullname);
          }
        }
      }

      closedir(dp);
    }
  }

  /*
   * remove inactive sockets
   */

  if (1) {
    DIR *dp;
    struct dirent *ep;

    dp = opendir(getenv("TMPFS") ? getenv("TMPFS") : TMPFS);
    if (dp != NULL) {
      while ((ep = readdir(dp))) {
        if (strncmp(ep->d_name, "tditracesocket@", 15) == 0) {
          char procpid[128];
          sprintf(procpid, (char *)"/proc/%d",
                  atoi(strrchr(ep->d_name, '@') + 1));

          char fullname[128];
          sprintf(fullname, "%s%s", getenv("TMPFS") ? getenv("TMPFS") : TMPFS,
                  ep->d_name);

          struct stat sts;

          if (stat(procpid, &sts) == -1) {
            unlink(fullname);
            fprintf(stderr, "tdi: init[%s][%d], removed \"%s\"\n", gprocname,
                    gpid, fullname);
          } else {
            fprintf(stderr, "tdi: init[%s][%d], not removed \"%s\"\n",
                    gprocname, gpid, fullname);
          }
        }
      }

      closedir(dp);
    }
  }

  LOCK_init();

  if (thedelay == 0) {
    if (create_trace_buffer() == -1) {
      return -1;
    }

    start_monitor_thread();
    return 0;
  }

  pthread_t delayed_init_thread_id;
  pthread_create(&delayed_init_thread_id, NULL, delayed_init_thread, &thedelay);

  if (thedelay == -1) {
    pthread_join(delayed_init_thread_id, NULL);
  }

  return 0;
}

static void tditrace_rewind() {
  LOCK();
  gtditrace_inited = 0;
  UNLOCK();

  gtrace_buffer_byte_ptr = gtrace_buffer;
  gtrace_buffer_dword_ptr = (unsigned int *)gtrace_buffer;

  /*
   * write one time start text
   */
  sprintf((char *)gtrace_buffer_dword_ptr, (char *)"TDITRACE");
  gtrace_buffer_dword_ptr += 2;

  gettimeofday((struct timeval *)gtrace_buffer_dword_ptr, 0);
  gtrace_buffer_dword_ptr += 2;

  clock_gettime(CLOCK_MONOTONIC, (struct timespec *)gtrace_buffer_dword_ptr);
  gtrace_buffer_dword_ptr += 2;

  gtrace_buffer_rewind_ptr = gtrace_buffer_dword_ptr;
  *gtrace_buffer_dword_ptr = 0;

  reported_full = 0;

  LOCK();
  gtditrace_inited = 1;
  UNLOCK();
}

#define KTDIM_IOCTL_TYPE 99

static void check_trace_buffer(int b) {
  FILE *file;

  if ((file = fopen(tracebuffers[b].filename, "r")) != NULL) {
    /* dev/ktdim */
    if (strncmp("/dev/", tracebuffers[b].filename, 5) == 0) {
      long size = ioctl(fileno(file), _IO(KTDIM_IOCTL_TYPE, 0), NULL);
      sprintf(tracebuffers[b].procname, "K");
      tracebuffers[b].pid = 0;
      tracebuffers[b].bufmmapped =
          (char *)mmap(0, size, PROT_READ, MAP_PRIVATE, fileno(file), 0);
      fprintf(stderr, "\"%s\" (%lluMB) ...\n", tracebuffers[b].filename,
              (unsigned long long)size / (1024 * 1024));
      tracebuffers[b].bufsize = size;
      tracebuffers[b].ptr = tracebuffers[b].bufmmapped;
      tracebuffers[b].dword_ptr = (unsigned int *)tracebuffers[b].bufmmapped;
    } else {
      /* /tmp/tditracebuffer@xxx@xxx */

      struct stat st;
      stat(tracebuffers[b].filename, &st);

      fprintf(stderr, "\"%s\" (%lluMB) ...\n", tracebuffers[b].filename,
              (unsigned long long)st.st_size / (1024 * 1024));

      char *s1 = strchr(tracebuffers[b].filename, '@');
      char *s2 = strchr(s1 + 1, '@');

      strncpy(tracebuffers[b].procname, s1 + 1, s2 - s1);
      tracebuffers[b].procname[(s2 - s1) - 1] = 0;
      tracebuffers[b].pid = atoi(s2 + 1);

      if (st.st_size == 0) {
        fprintf(stderr, "empty tracebuffer, skipping\n");
        return;
      }

      tracebuffers[b].bufmmapped = (char *)mmap(
          0, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(file), 0);
      tracebuffers[b].bufsize = st.st_size;
      tracebuffers[b].ptr = tracebuffers[b].bufmmapped;
      tracebuffers[b].dword_ptr = (unsigned int *)tracebuffers[b].bufmmapped;
    }

    // should hold "TDITRACE"
    if (strncmp("TDITRACE", tracebuffers[b].ptr, 8) != 0) {
      fprintf(stderr, "invalid tracebuffer, skipping\n");
      return;
    }

    tracebuffers[b].dword_ptr += 2;
    unsigned int *p = tracebuffers[b].dword_ptr;

    tracebuffers[b].timeofday_start = (_u64)*p++ * (_u64)1000000000;
    tracebuffers[b].timeofday_start += (_u64)*p++ * (_u64)1000;

    tracebuffers[b].monotonic_start = (_u64)*p++ * (_u64)1000000000;
    tracebuffers[b].monotonic_start += (_u64)*p++;

    /*
    fprintf(stderr,
            "\"%s\" "
            "timeofday_start:%lld, "
            "monotonic_start:%lld\n",
            tracebuffers[b].filename, tracebuffers[b].timeofday_start,
            tracebuffers[b].monotonic_start);
    */
    tracebuffers[b].dword_ptr += 4;

    fclose(file);
  }
}

void tditrace_exit(int argc, char *argv[]) {
  int i, nr_entries;
  int buffers = 0;
  FILE *file;

  if (argc > 1) {
    int tracebufferid = argc;

    while (--tracebufferid) {
      if (strstr(argv[tracebufferid], "tditracebuffer@")) {
        sprintf(tracebuffers[buffers].filename, "%s", argv[tracebufferid]);
        check_trace_buffer(buffers);
        buffers++;
      }
    }
  }

  else {
    DIR *dp;
    struct dirent *ep;

    dp = opendir(getenv("TMPFS") ? getenv("TMPFS") : TMPFS);
    if (dp != NULL) {
      while ((ep = readdir(dp))) {
        if (strncmp(ep->d_name, "tditracebuffer@", 15) == 0) {
          sprintf(tracebuffers[buffers].filename, "%s%s",
                  getenv("TMPFS") ? getenv("TMPFS") : TMPFS, ep->d_name);
          check_trace_buffer(buffers);
          buffers++;
        }
      }
    }
    closedir(dp);
  }

  sprintf(tracebuffers[buffers].filename, "/dev/ktdim");
  if ((file = fopen(tracebuffers[buffers].filename, "r")) != NULL) {
    fclose(file);
    check_trace_buffer(buffers);
    buffers++;
  }

  if (buffers == 0) {
    fprintf(stderr, "Not found: \""
                    "TMPFS/"
                    "tditracebuffer@*@*, /dev/ktdim\"\n");
    return;
  }

  _u64 last_timestamp = 0;

  for (i = 0; i < buffers; i++) {
    parse(i);
    if (tracebuffers[i].valid) {
      tracebuffers[i].monotonic_first = tracebuffers[i].monotonic_timestamp;
    }
  }

  fprintf(stdout, "TIME %d\n", 1000000000);
  fprintf(stdout, "SPEED %d\n", 1000000000);
  fprintf(stdout, "MEMSPEED %d\n", 1000000000);

  /*
   * play out all entries from all buffers in order of monotonic timestamp
   *
   */
  nr_entries = 0;
  while (1) {
    int pctused;
    int bytesused;
    int d = -1;

    for (i = 0; i < buffers; i++) {
      if (tracebuffers[i].valid) {
        d = i;
      }
    }

    /*
     * if no more valid entries in all buffers
     * then be done
     */
    if (d == -1) {
      break;
    }

    /*
     * select the entry with the lowest monotonic timestamp
     */
    for (i = 0; i < buffers; i++) {
      if (tracebuffers[i].valid) {
        if (tracebuffers[i].monotonic_timestamp <
            tracebuffers[d].monotonic_timestamp) {
          d = i;
        }
      }
    }

    addentry(stdout, tracebuffers[d].text, tracebuffers[d].text_len,
             tracebuffers[d].monotonic_timestamp, tracebuffers[d].procname,
             tracebuffers[d].pid, tracebuffers[d].tid,
             tracebuffers[d].nr_numbers, tracebuffers[d].numbers,
             tracebuffers[d].identifier);

    pctused = (((tracebuffers[d].text - tracebuffers[d].bufmmapped) * 100.0) /
               tracebuffers[d].bufsize) +
              1;
    bytesused = tracebuffers[d].text - tracebuffers[d].bufmmapped;

    nr_entries++;

    last_timestamp = tracebuffers[d].monotonic_timestamp;
    parse(d);

    if (!tracebuffers[d].valid) {
      fprintf(stderr, "\"%s\" %d%% (#%d,%dB)\n", tracebuffers[d].filename,
              pctused, nr_entries, bytesused);

      fprintf(stderr, "\"%s\" [t:%d,%d][i:%d,%d][s:%d,%d][e:%d,%d][n:%d,%d]\n",
              tracebuffers[d].filename, nr_tasks, nr_task_entries, nr_isrs,
              nr_isr_entries, nr_semas, nr_sema_entries, nr_events,
              nr_event_entries, nr_notes, nr_note_entries);
    }
  }

  // Add one more entry 0.1 sec behind all the previous ones
  addentry(stdout, "TDITRACE_EXIT", strlen("TDITRACE_EXIT"),
           last_timestamp + 100 * 1000000, "", 0, 0, 0, 0, 0);

  struct timespec atime;

  for (i = 0; i < buffers; i++) {
    atime.tv_sec = tracebuffers[i].timeofday_start / 1000000000;
    atime.tv_nsec = tracebuffers[i].timeofday_start - atime.tv_sec * 1000000000;
    fprintf(stdout, "END %d %lld %lld %lld %s", i,
            tracebuffers[i].timeofday_start, tracebuffers[i].monotonic_start,
            tracebuffers[i].monotonic_first,
            asctime(gmtime((const time_t *)&atime)));
  }
}

static void tditrace_internal(va_list args, const char *format);

void tditrace(const char *format, ...) {
  va_list args;

  va_start(args, format);

  tditrace_internal(args, format);

  va_end(args);
}

void tditrace_ex(int mask, const char *format, ...) {
  va_list args;

  if (mask & gmask) {
    va_start(args, format);

    tditrace_internal(args, format);

    va_end(args);
  }
}

/*
 * [TDIT]
 * [RACE]
 * [    ]timeofday_start.tv_usec
 * [    ]timeofday_start.tv_sec
 * [    ]clock_monotonic_start.tv_nsec
 * [    ]clock_monotonic_start.tv_sec
 * ------
 * [    ]marker, lower 2 bytes is total length in dwords, upper byte is
 * identifier,
 *       middle byte is nr numbers
 * [    ]clock_monotonic_timestamp.tv_nsec
 * [    ]clock_monotonic_timestamp.tv_sec
 * [    ]<optional> numbers
 * [    ]<optional> text, padded with 0's to multiple of 4 bytes
 * ...
 * ------
 */

void tditrace_internal(va_list args, const char *format) {
  if (!gtditrace_inited) {
    return;
  }

  unsigned int trace_text[1024 / 4];
  unsigned int numbers[16];
  unsigned int *pnumbers = numbers;
  unsigned char nr_numbers = 0;
  unsigned char identifier = 0;
  unsigned int i;

  /*
   * take and store timestamp
   */
  struct timespec mytime;
  clock_gettime(CLOCK_MONOTONIC, &mytime);

  /*
   * parse the format string
   * %0 %1 %2 pull in integers
   */
  char *trace_text_ptr = (char *)trace_text;
  unsigned int *trace_text_dword_ptr = (unsigned int *)trace_text;
  char ch;

  while ((ch = *(format++))) {
    if (ch == '%') {
      switch (ch = (*format++)) {
      case 's': {
        char *s;
        s = va_arg(args, char *);
        if (s) {
          int i = 0;
          while (*s) {
            *trace_text_ptr++ = *s++;
            i++;
            if (i > 256)
              break;
          }
        } else {
          *trace_text_ptr++ = 'n';
          *trace_text_ptr++ = 'i';
          *trace_text_ptr++ = 'l';
          *trace_text_ptr++ = 'l';
        }
        break;
      }
      case 'd': {
        int n = 0;
        unsigned int d = 1;
        int num = va_arg(args, int);
        if (num < 0) {
          num = -num;
          *trace_text_ptr++ = '-';
        }

        while (num / d >= 10)
          d *= 10;

        while (d != 0) {
          int digit = num / d;
          num %= d;
          d /= 10;
          if (n || digit > 0 || d == 0) {
            *trace_text_ptr++ = digit + '0';
            n++;
          }
        }
        break;
      }
      case 'u': {
        int n = 0;
        unsigned int d = 1;
        unsigned int num = va_arg(args, int);

        while (num / d >= 10)
          d *= 10;

        while (d != 0) {
          int digit = num / d;
          num %= d;
          d /= 10;
          if (n || digit > 0 || d == 0) {
            *trace_text_ptr++ = digit + '0';
            n++;
          }
        }
        break;
      }

      case 'x':
      case 'p': {
        int n = 0;
        unsigned int d = 1;
        unsigned int num = va_arg(args, int);

        while (num / d >= 16)
          d *= 16;

        while (d != 0) {
          int dgt = num / d;
          num %= d;
          d /= 16;
          if (n || dgt > 0 || d == 0) {
            *trace_text_ptr++ = dgt + (dgt < 10 ? '0' : 'a' - 10);
            ++n;
          }
        }
        break;
      }

      case 'n': {
        pnumbers[nr_numbers] = va_arg(args, int);
        nr_numbers++;
        break;
      }

      case 'm': {
        identifier = va_arg(args, int) & 0xff;
        break;
      }

      case 'C': {
        identifier = CPUINFO;
        nr_numbers = CPUINFO_MAXNUMBER + 1;
        pnumbers = (unsigned int *)va_arg(args, int);
        break;
      }

      case 'M': {
        identifier = MEMINFO;
        nr_numbers = MEMINFO_MAXNUMBER + 1;
        pnumbers = (unsigned int *)va_arg(args, int);
        break;
      }

      case 'S': {
        identifier = SLFINFO;
        nr_numbers = SLFINFO_MAXNUMBER + 1;
        pnumbers = (unsigned int *)va_arg(args, int);
        break;
      }

      case 'D': {
        identifier = DSKINFO;
        nr_numbers = DSKINFO_MAXNUMBER + 1;
        pnumbers = (unsigned int *)va_arg(args, int);
        break;
      }

      case 'N': {
        identifier = NETINFO;
        nr_numbers = NETINFO_MAXNUMBER + 1;
        pnumbers = (unsigned int *)va_arg(args, int);
        break;
      }

      case 'P': {
        identifier = PIDINFO;
        nr_numbers = 1;
        numbers[0] = va_arg(args, int);

        char *s;
        s = va_arg(args, char *);
        if (s) {
          int i = 0;
          while (*s) {
            *trace_text_ptr++ = *s++;
            i++;
            if (i > 256)
              break;
          }
        } else {
          *trace_text_ptr++ = 'n';
          *trace_text_ptr++ = 'i';
          *trace_text_ptr++ = 'l';
          *trace_text_ptr++ = 'l';
        }
        break;
      }

      case 'K': {
        identifier = MARKER;

        char *s;
        s = va_arg(args, char *);
        if (s) {
          int i = 0;
          while (*s) {
            *trace_text_ptr++ = *s++;
            i++;
            if (i > 256)
              break;
          }
        } else {
          *trace_text_ptr++ = 'n';
          *trace_text_ptr++ = 'i';
          *trace_text_ptr++ = 'l';
          *trace_text_ptr++ = 'l';
        }
        break;
      }

      case 'E': {
        identifier = ENVINFO;

        char *s;
        s = va_arg(args, char *);
        if (s) {
          int i = 0;
          while (*s) {
            *trace_text_ptr++ = *s++;
            i++;
            if (i > 256)
              break;
          }
        } else {
          *trace_text_ptr++ = 'n';
          *trace_text_ptr++ = 'i';
          *trace_text_ptr++ = 'l';
          *trace_text_ptr++ = 'l';
        }
        break;
      }

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        identifier = 100 + ch - '0';
        nr_numbers = ch - '0' + 1;
        pnumbers = (unsigned int *)va_arg(args, int);
        break;
      }

      default:
        break;
      }

    } else {
      *trace_text_ptr++ = ch;
    }
  }

  while ((unsigned int)trace_text_ptr & 0x3)
    *trace_text_ptr++ = 0;

  int nr_textdwords = (trace_text_ptr - (char *)trace_text) >> 2;

  /*
   * store into tracebuffer
   */
  LOCK();

  /*
   * marker, 4 bytes
   *       bytes 1+0 hold total length in dwords : 3 (marker,sec,nsec) +
   *                                           nr_numbers + nr_dwordtext
   *       byte    2 hold nr_numbers
   *       byte    3 hold 00..ff
   */

  *gtrace_buffer_dword_ptr++ = (0x0003 + nr_numbers + nr_textdwords) |
                               ((nr_numbers & 0xff) << 16) |
                               ((identifier & 0xff) << 24);
  *gtrace_buffer_dword_ptr++ = mytime.tv_sec;
  *gtrace_buffer_dword_ptr++ = mytime.tv_nsec;

  i = 0;
  while (i != nr_numbers) {
    *gtrace_buffer_dword_ptr++ = pnumbers[i];
    i++;
  }

  i = nr_textdwords;
  while (i--) {
    *gtrace_buffer_dword_ptr++ = *trace_text_dword_ptr++;
  }

  /*
   * mark the next marker as invalid
   */
  *gtrace_buffer_dword_ptr = 0;

  if (((unsigned int)gtrace_buffer_dword_ptr - (unsigned int)gtrace_buffer) >
      (gtracebuffersize - 1024)) {
    if (do_offload || do_wrap) {
      // clear unused and rewind to rewind ptr
      fprintf(stderr, "tdi: rewind[%d,%s]\n", gpid, gprocname);
      unsigned int i;
      for (i = (unsigned int)gtrace_buffer_dword_ptr -
               (unsigned int)gtrace_buffer;
           i < gtracebuffersize; i++) {
        gtrace_buffer[i] = 0;
      }
      gtrace_buffer_dword_ptr = gtrace_buffer_rewind_ptr;
      if (!do_offload)
        do_dump_proc_self_maps = 1;
    } else {
      fprintf(stderr, "tdi: full[%s][%d]\n", gprocname, gpid);
      gtditrace_inited = 0;
    }
  }

  UNLOCK();
}

} // extern "C"

static void __attribute__((constructor)) tditracer_constructor();
static void __attribute__((destructor)) tditracer_destructor();

static void tditracer_constructor() {
  if (tditrace_init() == -1) {
    return;
  }
}

static void tditracer_destructor() {
#if 0
  fprintf(stderr, "tdi: exit[%d]\n", getpid());
#endif
}

/*
 ******************************************************************************************
 */

#if 0
extern "C" int extra(int in, int *out) {

    static int (*__extra)(int, int *) = NULL;

    if (NULL == __extra) {
        __extra = (int (*)(int, int *))dlsym(RTLD_NEXT, __func__);
        if (NULL == __extra) {
            fprintf(stderr, "Error in `dlsym`: %s %s\n", __func__, dlerror());
        }
    }

    tditrace("@T+extra()");
    int ret = __extra(in, out);
    tditrace("@T-extra()");

    return ret;
}
#endif

#if 0
#if 1
extern "C" void *mmap(void *__addr, size_t __len, int __prot, int __flags,
                      int __fd, __off_t __offset) {
  static void *(*__mmap)(void *, size_t, int, int, int, __off_t) = NULL;
  if (__mmap == NULL) {
    __mmap = (void *(*)(void *, size_t, int, int, int, __off_t))dlsym(RTLD_NEXT,
                                                                      "mmap");
    if (__mmap == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  tditrace("@T+mmap%n", (int)__len);
  void *ret = __mmap(__addr, __len, __prot, __flags, __fd, __offset);
  tditrace("@T-mmap%n", (int)ret);

  return ret;
}
#endif

#if 1
extern "C" int munmap(void *__addr, size_t __len) {
  static int (*__munmap)(void *, size_t) = NULL;
  if (__munmap == NULL) {
    __munmap = (int (*)(void *, size_t))dlsym(RTLD_NEXT, "munmap");
    if (__munmap == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  tditrace("@T+munmap%n%n", __addr, __len);
  int ret = __munmap(__addr, __len);
  tditrace("@T-munmap");

  return ret;
}
#endif

extern "C" void syslog(int pri, const char *fmt, ...) {
  if (pri == 187) {
    va_list args;
    va_start(args, fmt);
    int a1 = va_arg(args, int);
    tditrace("syslog() \"%s\"", a1);
  }
}

#if 1
extern "C" void *memset(void *dest, int c, size_t n) {
  static void *(*__memset)(void *, int, size_t) = NULL;

  if (__memset == NULL) {
    __memset = (void *(*)(void *, int, size_t))dlsym(RTLD_NEXT, "memset");
    if (NULL == __memset) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  tditrace("@I+memset%n%n%n", dest, c, n);
  void *ret = __memset(dest, c, n);
  tditrace("@I-memset");

  return ret;
}
#endif
#endif

/*
 ******************************************************************************************
 ******************************************************************************************
 ******************************************************************************************
 */
