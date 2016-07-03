#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
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
#include <unistd.h>

#include <dirent.h>

void tditrace(const char *format, ...);
void tditrace_ex(int mask, const char *format, ...);

pid_t gpid;
char gprocname[128];

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

#define TASKS 0
#define ISRS 1
#define SEMAS 2
#define QUEUES 3
#define EVENTS 4
#define NOTES 7
#define AGENTS 8

static unsigned int gtracebuffersize = 16 * 1024 * 1024;

char gtracebufferfilename[128];
struct stat gtrace_buffer_st;

static char *gtrace_buffer;
static char *trace_buffer_ptr;
static char *gtrace_buffer_rewind_ptr;

static int tditrace_inited;
static int reported_full;
static int report_tid;

typedef unsigned long long _u64;

struct simplefu_mutex myMutex;

// 100 tasks of 1000 chars each
static char tasks_array[1024][128];
static int nr_tasks = 0;

// 100 isrs of 1000 chars each
static char isrs_array[1024][128];
static int nr_isrs = 0;

// 100 semas of 1000 chars each
static char semas_array[1024][128];
static int nr_semas = 0;

// 100 queues of 1000 chars each
static char queues_array[1024][128];
static int nr_queues = 0;
static int prev_queues[128];

// 100 events of 1000 chars each
static char events_array[1024][128];
static int nr_events = 0;

// 100 notes of 1000 chars each
static char notes_array[1024][128];
static int nr_notes = 0;

// 100 notes of 1000 agents each
static char agents_array[1024][128];
static int nr_agents = 0;

static _u64 timestamp_timeofday_nsec(void) {
  struct timeval mytime;

  gettimeofday(&mytime, 0);
  return ((_u64)((_u64)mytime.tv_usec * (_u64)1000 +
                 (_u64)mytime.tv_sec * (_u64)1000000000));
}

static _u64 timestamp_monotonic_nsec(void) {
  struct timespec mytime;

  clock_gettime(CLOCK_MONOTONIC, &mytime);
  return (
      (_u64)((_u64)mytime.tv_nsec + (_u64)mytime.tv_sec * (_u64)1000000000));
}

void tditrace_rewind();

static void addentry(FILE *stdout, char *text_in, _u64 timestamp,
                     char *procname, int pid, int tid) {
  int i;
  int entry;
  char fullentry[1024];

  char name[1024];
  int value = 0;

  char text_in1[1024];
  char *text = text_in1;

  sprintf(text_in1, "[%s][%d][%d]", procname, pid, tid);
  int procpidtidlen = strlen(text_in1);

  // fprintf(stderr, "addentry:[%s]\n", text_in);

  if ((strncmp(text_in, "@T+", 3) == 0) || (strncmp(text_in, "@T-", 3) == 0) ||
      (strncmp(text_in, "@I+", 3) == 0) || (strncmp(text_in, "@I-", 3) == 0) ||
      (strncmp(text_in, "@A+", 3) == 0) || (strncmp(text_in, "@A-", 3) == 0) ||
      (strncmp(text_in, "@S+", 3) == 0) || (strncmp(text_in, "@E+", 3) == 0)) {
    strncpy(text_in1, text_in, 3);
    sprintf(&text_in1[3], "[%s][%d][%d]%s", procname, pid, tid, &text_in[3]);

  } else {
    sprintf(text_in1, "[%s][%d][%d]%s", procname, pid, tid, text_in);
  }

  // get rid of any '\n', replace with '_'
  for (i = 0; i < (int)strlen(text); i++) {
    if ((text[i] == 13) || (text[i] == 10)) {
      text[i] = 0x5f;
    }
  }

  /*
   * TASKS entry
   *
   */
  if ((strncmp(text, "@T+", 3) == 0) || (strncmp(text, "@T-", 3) == 0)) {
    int enter_not_exit = (strncmp(text + 2, "+", 1) == 0);

    text += 3;
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

      pos = strchr(text, ' ');
      if (pos)
        len = pos - text;
      else
        len = strlen(text);

      strncpy(tasks_array[nr_tasks], text, len);
      entry = nr_tasks;
      nr_tasks++;

      // Since we added a new entry we need to create a NAM, with only the
      // first part of the text
      sprintf(fullentry, "NAM %d %d %s\n", TASKS, TASKS * 1000 + entry,
              tasks_array[entry]);
      fwrite(fullentry, strlen(fullentry), 1, stdout);
    }

    // Add the STA or STO entry
    sprintf(fullentry, "%s %d %d %lld\n", enter_not_exit ? "STA" : "STO", TASKS,
            TASKS * 1000 + entry, timestamp);
    fwrite(fullentry, strlen(fullentry), 1, stdout);

    // Add the DSC entry, replace any spaces with commas
    sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);

    for (i = 8; i < (int)strlen(fullentry); i++) {
      if (fullentry[i] == 32) fullentry[i] = 0x2c;
    }
    fwrite(fullentry, strlen(fullentry), 1, stdout);

    return;
  }

  /*
   * SEMAS entry
   *
   */
  if (strncmp(text, "@S+", 3) == 0) {
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
      entry = nr_semas;
      nr_semas++;

      // Since we added a new entry we need to create a NAM, with only the
      // first part of the text
      sprintf(fullentry, "NAM %d %d %s\n", SEMAS, SEMAS * 1000 + entry,
              semas_array[entry]);
      fwrite(fullentry, strlen(fullentry), 1, stdout);
    }

    // Add the OCC entry
    sprintf(fullentry, "OCC %d %d %lld\n", SEMAS, SEMAS * 1000 + entry,
            timestamp);
    fwrite(fullentry, strlen(fullentry), 1, stdout);

    // Add the DSC entry, replace any spaces with commas
    sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
    for (i = 8; i < (int)strlen(fullentry); i++) {
      if (fullentry[i] == 32) fullentry[i] = 0x2c;
    }
    fwrite(fullentry, strlen(fullentry), 1, stdout);

    return;
  }

  /*
   * ISRS entry
   *
   */
  if ((strncmp(text, "@I+", 3) == 0) || (strncmp(text, "@I-", 3) == 0)) {
    int enter_not_exit = (strncmp(text + 2, "+", 1) == 0);

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
      entry = nr_isrs;
      nr_isrs++;

      // Since we added a new entry we need to create a NAM, with only the
      // first part of the text
      sprintf(fullentry, "NAM %d %d %s\n", ISRS, ISRS * 1000 + entry,
              isrs_array[entry]);
      fwrite(fullentry, strlen(fullentry), 1, stdout);
    }

    // Add the STA or STO entry
    sprintf(fullentry, "%s %d %d %lld\n", enter_not_exit ? "STA" : "STO", ISRS,
            ISRS * 1000 + entry, timestamp);
    fwrite(fullentry, strlen(fullentry), 1, stdout);

    // Add the DSC entry, replace any spaces with commas
    sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
    for (i = 8; i < (int)strlen(fullentry); i++) {
      if (fullentry[i] == 32) fullentry[i] = 0x2c;
    }
    fwrite(fullentry, strlen(fullentry), 1, stdout);

    return;
  }

  /*
   * EVENTS entry
   *
   */
  if (strncmp(text, "@E+", 3) == 0) {
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
      entry = nr_events;
      nr_events++;

      // Since we added a new entry we need to create a NAM, with only the
      // first part of the text
      sprintf(fullentry, "NAM %d %d %s\n", EVENTS, EVENTS * 1000 + entry,
              events_array[entry]);
      fwrite(fullentry, strlen(fullentry), 1, stdout);
    }

    // Add the OCC entry
    sprintf(fullentry, "OCC %d %d %lld\n", EVENTS, EVENTS * 1000 + entry,
            timestamp);
    fwrite(fullentry, strlen(fullentry), 1, stdout);

    // Add the DSC entry, replace any spaces with commas
    sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
    for (i = 8; i < (int)strlen(fullentry); i++) {
      if (fullentry[i] == 32) fullentry[i] = 0x2c;
    }
    fwrite(fullentry, strlen(fullentry), 1, stdout);

    return;
  }

  /*
   * AGENTS entry
   *
   */
  if ((strncmp(text, "@A+", 3) == 0) || (strncmp(text, "@A-", 3) == 0)) {
    // fprintf(stderr, "text=\"%s\"\n", text);

    int enter_not_exit = (strncmp(text + 2, "+", 1) == 0);

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
      entry = nr_agents;
      nr_agents++;

      // Since we added a new entry we need to create a NAM, with only the
      // first part of the text
      sprintf(fullentry, "NAM %d %d %s\n", AGENTS, AGENTS * 1000 + entry,
              agents_array[entry]);
      fwrite(fullentry, strlen(fullentry), 1, stdout);
    }

    // Add the STA or STO entry
    sprintf(fullentry, "%s %d %d %lld\n", enter_not_exit ? "STA" : "STO",
            AGENTS, AGENTS * 1000 + entry, timestamp);
    fwrite(fullentry, strlen(fullentry), 1, stdout);

    // Add the DSC entry, replace any spaces with commas
    sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
    for (i = 8; i < (int)strlen(fullentry); i++) {
      if (fullentry[i] == 32) fullentry[i] = 0x2c;
    }
    fwrite(fullentry, strlen(fullentry), 1, stdout);

    return;
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
      if (text[i] == 32) break;
      if (text[i] == 126) {
        // fprintf(stderr, "text1=\"%s\"\n", text);

        strncpy(name, text, i);
        name[i] = 0;
        value = atoi(text + i + 1);
        text[i] = 32;  // create split marker

        // fprintf(stderr, "name=\"%s\", text=\"%s\"\n", name, text);
      }
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
      sprintf(fullentry, "NAM %d %d %s\n", QUEUES, QUEUES * 1000 + entry,
              queues_array[entry]);
      fwrite(fullentry, strlen(fullentry), 1, stdout);

      // reset the prev_value;
      prev_queues[entry] = 0;
    }

    // fill in the value
    // add a STA (with the delta) or a STO (with the delta) depending on
    // whether the value went up or went down
    if (value >= prev_queues[entry])
      sprintf(fullentry, "STA %d %d %lld %d\n", QUEUES, QUEUES * 1000 + entry,
              timestamp, value - prev_queues[entry]);
    else
      sprintf(fullentry, "STO %d %d %lld %d\n", QUEUES, QUEUES * 1000 + entry,
              timestamp, prev_queues[entry] - value);
    fwrite(fullentry, strlen(fullentry), 1, stdout);
    prev_queues[entry] = value;

    return;
  }

  /*
   * Treat everything else as a NOTES entry
   *
   */

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
    entry = nr_notes;
    nr_notes++;

    // Since we added a new entry we need to create a NAM, with only the
    // first part of the text
    sprintf(fullentry, "NAM %d %d %s\n", NOTES, NOTES * 1000 + entry,
            notes_array[entry]);
    fwrite(fullentry, strlen(fullentry), 1, stdout);
  }

  // Add the OCC entry
  sprintf(fullentry, "OCC %d %d %lld\n", NOTES, NOTES * 1000 + entry,
          timestamp);
  fwrite(fullentry, strlen(fullentry), 1, stdout);

  // Add the DSC entry, replace any spaces with commas
  sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text + procpidtidlen);
  for (i = 8; i < (int)strlen(fullentry); i++) {
    if (fullentry[i] == 32) fullentry[i] = 0x2c;
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
  char *saveptr;
  _u64 timeofday_offset;
  _u64 monotonic_offset;
  _u64 monotonic_timestamp;
  char *text;
  int tid;
  int valid;
} tracebuffer_t;

tracebuffer_t tracebuffers[10];

static void parse(int bid) {
  const char *search = "\f";
  char *token;

  int timestamp_sec;
  int timestamp_nsec;

  // layout is "\ftimestamp_sec\ftimestamp_nsec\ftid\ftracestring"

  tracebuffers[bid].valid = 0;

#if 0
    char buf[100];
    snprintf(buf, 79, tracebuffers[bid].saveptr);
    int i;
    for (i = 0 ; i < 79 ; i++)
    {
        fprintf(stderr, "%c", buf[i] == 0x0c ? '|' : buf[i]); 
    }
    fprintf(stderr, "\n");
#endif

  char *endptr;
  long int val;

  token = strtok_r(NULL, search, &tracebuffers[bid].saveptr);
  if (token) {
    // token is timestamp_sec
    val = strtol(token, &endptr, 10);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
        (errno != 0 && val == 0)) {
      fprintf(stderr, "strtol error\n");
      return;
    }
    if (endptr == token) {
      fprintf(stderr, "No digits were found[%s]\n", token);
#if 1
      char buf[100];
      snprintf(buf, 79, tracebuffers[bid].saveptr);
      int i;
      for (i = 0; i < 79; i++) {
        fprintf(stderr, "%c", buf[i] == 0x0c ? '|' : buf[i]);
      }
      fprintf(stderr, "\n");
#endif
      return;
    }
    timestamp_sec = val;

    token = strtok_r(NULL, search, &tracebuffers[bid].saveptr);
    if (token) {
      // token is timestamp_nsec
      long int val = strtol(token, &endptr, 10);
      if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
          (errno != 0 && val == 0)) {
        fprintf(stderr, "strtol error\n");
        return;
      }
      if (endptr == token) {
        fprintf(stderr, "No digits were found[%s]\n", token);
#if 1
        char buf[100];
        snprintf(buf, 79, tracebuffers[bid].saveptr);
        int i;
        for (i = 0; i < 79; i++) {
          fprintf(stderr, "%c", buf[i] == 0x0c ? '|' : buf[i]);
        }
        fprintf(stderr, "\n");
#endif
        return;
      }
      timestamp_nsec = val;

      tracebuffers[bid].monotonic_timestamp =
          (_u64)timestamp_nsec + (_u64)timestamp_sec * (_u64)1000000000;

      token = strtok_r(NULL, search, &tracebuffers[bid].saveptr);
      if (token) {
        // token is tid

        long int val = strtol(token, &endptr, 10);
        if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
            (errno != 0 && val == 0)) {
          fprintf(stderr, "strtol error\n");
          return;
        }
        if (endptr == token) {
          fprintf(stderr, "No digits were found[%s]\n", token);
#if 1
          char buf[100];
          snprintf(buf, 79, tracebuffers[bid].saveptr);
          int i;
          for (i = 0; i < 79; i++) {
            fprintf(stderr, "%c", buf[i] == 0x0c ? '|' : buf[i]);
          }
          fprintf(stderr, "\n");
#endif
          return;
        }

        tracebuffers[bid].tid = val;

        token = strtok_r(NULL, search, &tracebuffers[bid].saveptr);
        if (token) {
          // token is tid
          tracebuffers[bid].text = token;
          tracebuffers[bid].valid = 1;
        }
      }
    }
  }
}

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

static void get_process_name_by_pid(const int pid, char *name) {
  char fullname[1024];
  if (name) {
    sprintf(name, "/proc/%d/cmdline", pid);

    FILE *f = fopen(name, "r");
    if (f) {
      size_t size;
      size = fread(fullname, sizeof(char), 1024, f);

      if (size > 0) {
        if ('\n' == fullname[size - 1]) fullname[size - 1] = '\0';

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

char proc_self_maps[16 * 1024 + 1];

static void dump_proc_self_maps(void) {
  int fd;
  int bytes;

  fprintf(stderr, "tdi: [%s][%d], maps...\n", gprocname, gpid);

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
      // fprintf(stderr, "bytes=%d[%s]\n", bytes, proc_self_maps);

      char *saveptr;
      char *line = strtok_r(proc_self_maps, "\n", &saveptr);

      while (line) {
        if (strlen(line) > 50) {
          // fprintf(stderr, "+%d[%s]\n", strlen(line), line);
          tditrace("MAPS [%s][%d] %s", gprocname, gpid, line);
          // fprintf(stderr, "=%d\n", trace_buffer_ptr -
          // gtrace_buffer);
        }
        line = strtok_r(NULL, "\n", &saveptr);
      }

    } else
      break;
  }

  close(fd);

  tditrace("MAPS [%s][%d] end", gprocname, gpid);
}

static int gmask = 0x0;

static int do_vmsize = 0;
static int do_rss = 0;
static int do_heap = 0;
static int do_maxrss = 0;
static int do_majflt = 0;
static int do_minflt = 0;
static int do_persecond = 0;

static int allow_rewind = 0;

static int monitor;

static int do_offload = 0;
static char offload_location[256] = {0};
static int offload_counter = 0;
static int offload_over50 = 0;

void *monitor_thread(void *param) {
  static int seconds_counter = 0;
  static int do_dump_proc_self_maps = 0;

  stat(gtracebufferfilename, &gtrace_buffer_st);
  usleep(1 * 1000 * 1000);
  dump_proc_self_maps();

  fprintf(stderr, "tdi: [%s][%d], monitoring...\n", gprocname, gpid);
  while (1) {
    seconds_counter++;

    if (do_dump_proc_self_maps) {
      dump_proc_self_maps();
      do_dump_proc_self_maps = 0;
    }

    if (do_heap) {
      struct mallinfo mi;

      mi = mallinfo();

      // printf("Total non-mmapped bytes (arena):       %d\n", mi.arena);
      // printf("# of free chunks (ordblks):            %d\n",
      // mi.ordblks);
      // printf("# of free fastbin blocks (smblks):     %d\n", mi.smblks);
      // printf("# of mapped regions (hblks):           %d\n", mi.hblks);
      // printf("Bytes in mapped regions (hblkhd):      %d\n", mi.hblkhd);
      // printf("Max. total allocated space (usmblks):  %d\n",
      // mi.usmblks);
      // printf("Free bytes held in fastbins (fsmblks): %d\n",
      // mi.fsmblks);
      // printf("Total allocated space (uordblks):      %d\n",
      // mi.uordblks);
      // printf("Total free space (fordblks):           %d\n",
      // mi.fordblks);
      // printf("Topmost releasable block (keepcost):   %d\n",
      // mi.keepcost);

      tditrace("HEAP~%d", mi.arena + mi.hblkhd);
      // tditrace("mi_ordblks~%d", mi.ordblks);
      // tditrace("mi_smblks~%d", mi.smblks);

      // tditrace("hblks~%d", mi.hblks);
      // tditrace("hblkhd~%d", mi.hblkhd);

      // tditrace("mi_usmblks~%d", mi.usmblks);
      // tditrace("mi_fsmblks~%d", mi.fsmblks);
      // tditrace("mi_uordblks~%d", mi.uordblks);
      // tditrace("mi_fordblks~%d", mi.fordblks);
      // tditrace("mi_keepcost~%d", mi.keepcost);
    }

    if (do_vmsize || do_rss) {
      unsigned long vmsize = 0L;
      unsigned long rss = 0L;

      int fh = 0;
      char buffer[65];
      int gotten;
      fh = open("/proc/self/statm", O_RDONLY);
      gotten = read(fh, buffer, 64);
      buffer[gotten] = '\0';
      if (sscanf(buffer, "%lu %lu", &vmsize, &rss) != 1) {
        if (do_vmsize) tditrace("VMSIZE~%d", (int)(vmsize * 4096));
        if (do_rss) tditrace("RSS~%d", (int)(rss * 4096));
      }
      close(fh);
    }

    if (do_maxrss || do_minflt || do_majflt) {
      struct rusage resourceUsage;
      getrusage(RUSAGE_SELF, &resourceUsage);

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

      if (do_maxrss) tditrace("MAXRSS~%d", resourceUsage.ru_maxrss);
      if (do_minflt) tditrace("MINFLT~%d", resourceUsage.ru_minflt);
      if (do_majflt) tditrace("MAJFLT~%d", resourceUsage.ru_majflt);
    }

    struct stat st;
    stat(gtracebufferfilename, &st);

    if (allow_rewind) {
      // fprintf(stderr, "tdi-check: %s mtim=%d gmtim=%d\n",
      // gtracebufferfilename, st.st_mtim.tv_sec,
      // gtrace_buffer_st.st_mtim.tv_sec);

      if (st.st_mtim.tv_sec != gtrace_buffer_st.st_mtim.tv_sec) {
        stat(gtracebufferfilename, &gtrace_buffer_st);

        fprintf(stderr, "tdi: [%s][%d], rewinding...\n", gprocname, gpid);
        tditrace_rewind();
        do_dump_proc_self_maps = 1;
      }
    }

    if (do_offload) {
      static char offloadfilename[256];
      static char *offload_buffer;
      static FILE *offload_file;

      if (!offload_over50) {
        simplefu_mutex_lock(&myMutex);
        int check =
            ((trace_buffer_ptr - gtrace_buffer) > (gtracebuffersize / 2));
        simplefu_mutex_unlock(&myMutex);

        if (check) {
          offload_over50 = 1;
          fprintf(stderr, "tdi: [%d][%s], at 50%%...\n", gpid, gprocname);
          // create a new file and fill with 0..50% data

          offload_counter++;

          sprintf(offloadfilename, (char *)"%s/tditracebuffer@%s@%d@%04d",
                  offload_location, gprocname, gpid, offload_counter);

          if ((offload_file = fopen(offloadfilename, "w+")) == 0) {
            int errsv = errno;
            fprintf(stderr, "Error creating file \"%s\" [%d]\n",
                    offloadfilename, errsv);
          }

          (void)ftruncate(fileno(offload_file), gtracebuffersize);

          offload_buffer = (char *)mmap(0, gtracebuffersize, PROT_WRITE,
                                        MAP_SHARED, fileno(offload_file), 0);

          fprintf(stderr, "tdi: [%d][%s], created offloadfile: \"%s\" \n", gpid,
                  gprocname, offloadfilename);

          memcpy(offload_buffer, gtrace_buffer, gtracebuffersize / 2);

          fprintf(stderr,
                  "tdi: [%d][%s], copied 0..50%% to "
                  "offloadfile: \"%s\" \n",
                  gpid, gprocname, offloadfilename);
        }

      } else {
        simplefu_mutex_lock(&myMutex);
        int check =
            ((trace_buffer_ptr - gtrace_buffer) < (gtracebuffersize / 2));
        simplefu_mutex_unlock(&myMutex);

        if (check) {
          offload_over50 = 0;
          fprintf(stderr, "tdi: [%d][%s], at 100%%...\n", gpid, gprocname);
          // fill remaining 50..100% data to existing file and close
          // file

          memcpy(offload_buffer + gtracebuffersize / 2,
                 gtrace_buffer + gtracebuffersize / 2, gtracebuffersize / 2);
          munmap(offload_buffer, gtracebuffersize);
          fclose(offload_file);

          fprintf(stderr,
                  "tdi: [%d][%s], copied 50..100%% to "
                  "offloadfile: \"%s\" \n",
                  gpid, gprocname, offloadfilename);
        }
      }
    }

    usleep(1000 * 1000 / do_persecond);
  }

  pthread_exit(NULL);
}

static int thedelay;

void *delayed_init_thread(void *param) {
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
      usleep(1 * 1000000);

#if 0
            struct stat st;
            stat(gtracebufferfilename, &st);
            if (st.st_atim.tv_sec != gtrace_buffer_st.st_atim.tv_sec) {
            }
#endif
      delay--;
    }
  }

  fprintf(stderr, "tdi: init[%s][%d], delay finished...\n", gprocname, gpid,
          delay);

  /*
   * Create tracebuffer
   */
  sprintf(gtracebufferfilename, (char *)"/tmp/tditracebuffer@%s@%d", gprocname,
          gpid);
  FILE *file;
  if ((file = fopen(gtracebufferfilename, "w+")) == 0) {
    fprintf(stderr, "Error creating file \"%s\"", gtracebufferfilename);
  }
  int i = ftruncate(fileno(file), gtracebuffersize);
  gtrace_buffer = (char *)mmap(0, gtracebuffersize, PROT_READ | PROT_WRITE,
                               MAP_SHARED, fileno(file), 0);
  stat(gtracebufferfilename, &gtrace_buffer_st);

  // for (i = 0; i < gtracebuffersize; i++) {
  //   gtrace_buffer[i] = 0;
  //}

  fprintf(stderr, "tdi: init[%s][%d], allocated \"%s\" (%dMB)\n", gprocname,
          gpid, gtracebufferfilename, gtracebuffersize / (1024 * 1024));

  trace_buffer_ptr = gtrace_buffer;
  // write one time start text
  trace_buffer_ptr += sprintf(trace_buffer_ptr, (char *)"TDITRACEBUFFER\f");
  // obtain timeofday timestamp and write to buffer
  _u64 timeofday_timestamp = timestamp_timeofday_nsec();
  // obtain monotonic timestamp and write to buffer
  _u64 monotonic_timestamp = timestamp_monotonic_nsec();
  trace_buffer_ptr +=
      sprintf(trace_buffer_ptr, (char *)"%lld\f", timeofday_timestamp);
  // obtain monotonic timestamp and write to buffer
  trace_buffer_ptr +=
      sprintf(trace_buffer_ptr, (char *)"%lld\f", monotonic_timestamp);
  gtrace_buffer_rewind_ptr = trace_buffer_ptr;
  fprintf(stderr,
          "tdi: init[%s][%d], timeofday_timestamp:%lld, "
          "monotonic_timestamp:%lld\n",
          gprocname, gpid, timeofday_timestamp, monotonic_timestamp);
  reported_full = 0;

  simplefu_mutex_lock(&myMutex);
  tditrace_inited = 1;
  simplefu_mutex_unlock(&myMutex);

  pthread_t monitor_thread_id;
  pthread_create(&monitor_thread_id, NULL, monitor_thread, &monitor);

  pthread_exit(NULL);
}

const char *instruments[] = {"console",     // 0x00000001
                             "render",      // 0x00000002
                             "css",         // 0x00000004
                             "dom",         // 0x00000008
                             "canvas",      // 0x00000010
                             "webgl",       // 0x00000020
                             "image",       // 0x00000040
                             "graphics",    // 0x00000080
                             "graphicsqt",  // 0x00000100
                             "texmap",      // 0x00000200
                             "opengl",      // 0x00000400
                             "qweb",        // 0x00000800
                             "resource",    // 0x00001000
                             "javascript",  // 0x00002000
                             "allocator"};  // 0x00004000

int tditrace_init(void) {
  struct timeval mytime;
  int i;

  if (tditrace_inited) {
    return 0;
  }

  gpid = getpid();
  get_process_name_by_pid(gpid, gprocname);

  if (strcmp(gprocname, "mkdir") == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"mkdir\" ; not tracing\n",
            gprocname, gpid);
    return 0;
  } else if (strncmp(gprocname, "sh", 2) == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"sh*\" ; not tracing\n",
            gprocname, gpid);
    return 0;
  } else if (strcmp(gprocname, "strace") == 0) {
    fprintf(stderr, "tdi: init[%s][%d], procname is \"strace\" ; not tracing\n",
            gprocname, gpid);
    return 0;
  } else if (strcmp(gprocname, "gdbserver") == 0) {
    fprintf(stderr,
            "tdi: init[%s][%d], procname is \"gdbserver\" ; not tracing\n",
            gprocname, gpid);
    return 0;
  } else {
    // fprintf(stderr, "tdi: init[%s][%d]\n", gprocname, gpid);
  }

  char *env;

  if (env = getenv("TRACEBUFFERSIZE")) {
    gtracebuffersize = atoi(env) * 1024 * 1024;
  }

  gmask = 0x0;
  if (env = getenv("MASK")) {
    for (i = 0; i < sizeof(instruments) / sizeof(char *); i++) {
      if (strstr(env, instruments[i])) gmask |= (1 << i);
    }
    if (gmask == 0x0) gmask = strtoul(env, 0, 16);
  }
  fprintf(stderr, "tdi: init[%s][%d], mask = 0x%08x (", gprocname, gpid, gmask);
  for (i = 0; i < sizeof(instruments) / sizeof(char *); i++) {
    if (gmask & (1 << i)) fprintf(stderr, "%s%s", i==0? "":",", instruments[i]);
  }
  fprintf(stderr, ")\n");

  allow_rewind = 0;
  if (env = getenv("REWIND")) {
    allow_rewind = (atoi(env) >= 1);
  }

  do_persecond = 1;

  do_vmsize = 0;
  if (env = getenv("VMSIZE")) {
    do_vmsize = atoi(env);
    if (do_vmsize > do_persecond) do_persecond = do_vmsize;
  }
  do_rss = 0;
  if (env = getenv("RSS")) {
    do_rss = atoi(env);
    if (do_rss > do_persecond) do_persecond = do_rss;
  }
  do_heap = 0;
  if (env = getenv("HEAP")) {
    do_heap = atoi(env);
    if (do_heap > do_persecond) do_persecond = do_heap;
  }
  do_maxrss = 0;
  if (env = getenv("MAXRSS")) {
    do_maxrss = atoi(env);
    if (do_maxrss > do_persecond) do_persecond = do_maxrss;
  }
  do_minflt = 0;
  if (env = getenv("MINFLT")) {
    do_minflt = atoi(env);
    if (do_minflt > do_persecond) do_persecond = do_minflt;
  }
  do_majflt = 0;
  if (env = getenv("MAJFLT")) {
    do_majflt = atoi(env);
    if (do_majflt > do_persecond) do_persecond = do_majflt;
  }

  do_offload = 0;
  if (env = getenv("OFFLOAD")) {
    do_offload = 1;
    strcpy(offload_location, env);
  }

  report_tid = 0;
  if (env = getenv("TID")) {
    report_tid = (atoi(env) >= 1);
  }

  thedelay = 0;
  if (env = getenv("DELAY")) {
    thedelay = atoi(env);
  }

  /*
   * remove inactive tracefiles
   */

  int remove = 1;
  if (env = getenv("REMOVE")) {
    remove = (atoi(env) >= 1);
  }

  if (remove) {
    DIR *dp;
    struct dirent *ep;

    dp = opendir("/tmp/");
    if (dp != NULL) {
      while (ep = readdir(dp)) {
        if (strncmp(ep->d_name, "tditracebuffer@", 15) == 0) {
          char procpid[128];
          sprintf(procpid, (char *)"/proc/%d",
                  atoi(strrchr(ep->d_name, '@') + 1));

          char fullname[128];
          sprintf(fullname, "/tmp/%s", ep->d_name);

          struct stat sts;

          // if (stat(procpid, &sts) != -1) {
          //    printf("found \"%s\"\n", procpid);
          //}

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

  simplefu_mutex_init(&myMutex);

  if (thedelay == 0) {
    /*
     * Create tracebuffer
     */
    sprintf(gtracebufferfilename, (char *)"/tmp/tditracebuffer@%s@%d",
            gprocname, gpid);
    FILE *file;
    if ((file = fopen(gtracebufferfilename, "w+")) == 0) {
      fprintf(stderr, "Error creating file \"%s\"", gtracebufferfilename);
      return -1;
    }
    i = ftruncate(fileno(file), gtracebuffersize);
    gtrace_buffer = (char *)mmap(0, gtracebuffersize, PROT_READ | PROT_WRITE,
                                 MAP_SHARED, fileno(file), 0);
    stat(gtracebufferfilename, &gtrace_buffer_st);
    for (i = 0; i < gtracebuffersize; i++) {
      gtrace_buffer[i] = 0;
    }
    fprintf(stderr, "tdi: init[%s][%d], allocated \"%s\" (%dMB)\n", gprocname,
            gpid, gtracebufferfilename, gtracebuffersize / (1024 * 1024));

    trace_buffer_ptr = gtrace_buffer;
    // write one time start text
    trace_buffer_ptr += sprintf(trace_buffer_ptr, (char *)"TDITRACEBUFFER\f");
    // obtain timeofday timestamp and write to buffer
    _u64 timeofday_timestamp = timestamp_timeofday_nsec();
    // obtain monotonic timestamp and write to buffer
    _u64 monotonic_timestamp = timestamp_monotonic_nsec();
    trace_buffer_ptr +=
        sprintf(trace_buffer_ptr, (char *)"%lld\f", timeofday_timestamp);
    // obtain monotonic timestamp and write to buffer
    trace_buffer_ptr +=
        sprintf(trace_buffer_ptr, (char *)"%lld\f", monotonic_timestamp);
    gtrace_buffer_rewind_ptr = trace_buffer_ptr;
    fprintf(stderr,
            "tdi: init[%s][%d], timeofday_timestamp = %lld, "
            "monotonic_timestamp = %lld\n",
            gprocname, gpid, timeofday_timestamp, monotonic_timestamp);
    reported_full = 0;

    simplefu_mutex_lock(&myMutex);
    tditrace_inited = 1;
    simplefu_mutex_unlock(&myMutex);

    pthread_t monitor_thread_id;
    pthread_create(&monitor_thread_id, NULL, monitor_thread, &monitor);

    return 0;
  }

  pthread_t delayed_init_thread_id;
  pthread_create(&delayed_init_thread_id, NULL, delayed_init_thread, &thedelay);

  if (thedelay == -1) {
    pthread_join(delayed_init_thread_id, NULL);
  }

  return 0;
}

void tditrace_rewind() {
  struct timeval mytime;
  int i;

  simplefu_mutex_lock(&myMutex);
  tditrace_inited = 0;
  simplefu_mutex_unlock(&myMutex);

  trace_buffer_ptr = gtrace_buffer;
  // write one time start text
  trace_buffer_ptr += sprintf(trace_buffer_ptr, (char *)"TDITRACEBUFFER\f");
  // obtain timeofday timestamp and write to buffer
  _u64 timeofday_timestamp = timestamp_timeofday_nsec();
  // obtain monotonic timestamp and write to buffer
  _u64 monotonic_timestamp = timestamp_monotonic_nsec();
  trace_buffer_ptr +=
      sprintf(trace_buffer_ptr, (char *)"%lld\f", timeofday_timestamp);
  // obtain monotonic timestamp and write to buffer
  trace_buffer_ptr +=
      sprintf(trace_buffer_ptr, (char *)"%lld\f", monotonic_timestamp);
  gtrace_buffer_rewind_ptr = trace_buffer_ptr;

  for (i = gtrace_buffer_rewind_ptr - gtrace_buffer; i < gtracebuffersize;
       i++) {
    gtrace_buffer[i] = 0;
  }

  trace_buffer_ptr = gtrace_buffer_rewind_ptr;

  reported_full = 0;

  simplefu_mutex_lock(&myMutex);
  tditrace_inited = 1;
  simplefu_mutex_unlock(&myMutex);
}

void tditrace_exit(int argc, char *argv[]) {
  // Create the TDI file from the rough traces

  int i;
  const char *search = "\f";

  char *ptr;
  char *ptr1;

  int buffers = 0;

  if (argc > 1) {
    int tracebufferid = argc;

    while (--tracebufferid) {
      if (strstr(argv[tracebufferid], "tditracebuffer@") != 0) {
        FILE *file;

        sprintf(tracebuffers[buffers].filename, "%s", argv[tracebufferid]);

        if ((file = fopen(tracebuffers[buffers].filename, "r")) != NULL) {
          /* tditracebuffer@xxx@xxx */

          fprintf(stderr, "Found \"%s\"\n", tracebuffers[buffers].filename);

          char *s1 = strchr(tracebuffers[buffers].filename, '@');
          char *s2 = strchr(s1 + 1, '@');

          strncpy(tracebuffers[buffers].procname, s1 + 1, s2 - s1);
          tracebuffers[buffers].procname[(s2 - s1) - 1] = 0;
          tracebuffers[buffers].pid = atoi(s2 + 1);

          // fprintf(stderr, "[%s] [%d]\n",
          // tracebuffers[buffers].procname,
          // tracebuffers[buffers].pid);

          struct stat st;
          stat(tracebuffers[buffers].filename, &st);

          if (st.st_size == 0) {
            fprintf(stderr, "empty tracebuffer, skipping\n");
            break;
          }

          tracebuffers[buffers].bufmmapped =
              (char *)mmap(0, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
                           fileno(file), 0);
          tracebuffers[buffers].bufsize = st.st_size;

          tracebuffers[buffers].ptr = tracebuffers[buffers].bufmmapped;

          // token should hold "TDITRACEBUFFER"
          if (strncmp("TDITRACEBUFFER",
                      strtok_r(tracebuffers[buffers].ptr, search,
                               &tracebuffers[buffers].saveptr),
                      14) != 0) {
            fprintf(stderr, "invalid tracebuffer, skipping\n");
            break;
          }

          fprintf(stderr, "bufpct=%d\n",
                  (int)(strlen(tracebuffers[buffers].saveptr) * 100.0 /
                        st.st_size));

          // token should hold "timeofday timestamp offset in nsecs"
          tracebuffers[buffers].timeofday_offset = (_u64)atoll(
              strtok_r(NULL, search, &tracebuffers[buffers].saveptr));

          tracebuffers[buffers].monotonic_offset = (_u64)atoll(
              strtok_r(NULL, search, &tracebuffers[buffers].saveptr));

          fclose(file);

          buffers++;
        }
      }
    }

  } else {
    DIR *dp;
    struct dirent *ep;

    dp = opendir("/tmp/");
    if (dp != NULL) {
      while (ep = readdir(dp)) {
        if (strncmp(ep->d_name, "tditracebuffer@", 15) == 0) {
          FILE *file;

          sprintf(tracebuffers[buffers].filename, "/tmp/%s", ep->d_name);

          if ((file = fopen(tracebuffers[buffers].filename, "r")) != NULL) {
            /* /tmp/tditracebuffer@xxx@xxx */

            fprintf(stderr, "Found \"%s\"\n", tracebuffers[buffers].filename);

            char *s1 = strchr(tracebuffers[buffers].filename, '@');
            char *s2 = strchr(s1 + 1, '@');

            strncpy(tracebuffers[buffers].procname, s1 + 1, s2 - s1);
            tracebuffers[buffers].procname[(s2 - s1) - 1] = 0;
            tracebuffers[buffers].pid = atoi(s2 + 1);

            struct stat st;
            stat(tracebuffers[buffers].filename, &st);

            if (st.st_size == 0) {
              fprintf(stderr, "empty tracebuffer, skipping\n");
              break;
            }

            tracebuffers[buffers].bufmmapped =
                (char *)mmap(0, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
                             fileno(file), 0);
            tracebuffers[buffers].bufsize = st.st_size;
            tracebuffers[buffers].ptr = tracebuffers[buffers].bufmmapped;

            // token should hold "TDITRACEBUFFER"
            if (strncmp("TDITRACEBUFFER",
                        strtok_r(tracebuffers[buffers].ptr, search,
                                 &tracebuffers[buffers].saveptr),
                        14) != 0) {
              fprintf(stderr,
                      "tdi: init[%s][%d], invalid "
                      "tracebuffer, skipping\n",
                      gprocname, gpid);
              break;
            }

            fprintf(stderr, "bufpct=%d\n",
                    (int)(strlen(tracebuffers[buffers].saveptr) * 100.0 /
                          (st.st_size)));

            // token should hold "timeofday timestamp offset in
            // nsecs"
            tracebuffers[buffers].timeofday_offset = (_u64)atoll(
                strtok_r(NULL, search, &tracebuffers[buffers].saveptr));

            tracebuffers[buffers].monotonic_offset = (_u64)atoll(
                strtok_r(NULL, search, &tracebuffers[buffers].saveptr));

            fclose(file);

            buffers++;
          }
        }
      }

      closedir(dp);
    }
  }

  if (buffers == 0) {
    fprintf(stderr, "Not found: \"/tmp/tditracebuffer@*@*\"\n");
    return;
  }

  _u64 abs_timeofday = 0;
  _u64 last_timestamp = 0;

  for (i = 0; i < buffers; i++) {
    parse(i);

    if (abs_timeofday == 0) {
      abs_timeofday = tracebuffers[i].timeofday_offset;
    } else if (tracebuffers[i].timeofday_offset < abs_timeofday) {
      abs_timeofday = tracebuffers[i].timeofday_offset;
    }
  }

  fprintf(stdout, "TIME %d\n", 1000000000);
  fprintf(stdout, "SPEED %d\n", 1000000000);

  while (1) {
    int pctused;
    int d = -1;

    for (i = 0; i < buffers; i++) {
      if (tracebuffers[i].valid) {
        d = i;
      }
    }

    if (d == -1) {
      break;
    }

    for (i = 0; i < buffers; i++) {
      if (tracebuffers[i].valid) {
        if ((tracebuffers[i].timeofday_offset - abs_timeofday +
             tracebuffers[i].monotonic_timestamp -
             tracebuffers[i].monotonic_offset) <
            (tracebuffers[d].timeofday_offset - abs_timeofday +
             tracebuffers[d].monotonic_timestamp -
             tracebuffers[d].monotonic_offset)) {
          d = i;
        }
      }
    }

    addentry(stdout, tracebuffers[d].text,

             tracebuffers[d].timeofday_offset - abs_timeofday +
                 tracebuffers[d].monotonic_timestamp -
                 tracebuffers[d].monotonic_offset,

             tracebuffers[d].procname, tracebuffers[d].pid,
             tracebuffers[d].tid);

    pctused = (((tracebuffers[d].text - tracebuffers[d].bufmmapped) * 100.0) /
               tracebuffers[d].bufsize) +
              1;

    last_timestamp = tracebuffers[d].timeofday_offset - abs_timeofday +
                     tracebuffers[d].monotonic_timestamp -
                     tracebuffers[d].monotonic_offset;

    parse(d);

    if (!tracebuffers[d].valid) {
      fprintf(stderr, "\"%s\" final entry (%d%% used)\n",
              tracebuffers[d].filename, pctused);
    }
  }

  // Add one more entry 0.1 sec behind all the previous ones
  addentry(stdout, "TDITRACE_EXIT\0", last_timestamp + 100 * 1000000, "", 0, 0);

  struct timespec atime;

  atime.tv_sec = abs_timeofday / 1000000000;
  atime.tv_nsec = abs_timeofday - atime.tv_sec * 1000000000;

  fprintf(stdout, "END %lld UTC %s", abs_timeofday,
          asctime(gmtime((const time_t *)&atime)));
}

void tditrace_internal(va_list args, const char *format) {
  if (!tditrace_inited) {
    return;
  }

  struct timespec mytime;
  clock_gettime(CLOCK_MONOTONIC, &mytime);

  // Add the string to the tracebuffer, and add timestamp
  // layout is "\ftimestamp_sec\ftimestamp_nsec\ftid\ftracestring"

  char trace_text[1024];
  char *trace_text_ptr = trace_text;

  *trace_text_ptr++ = 0x0c;

  int n1 = 0;
  unsigned int d1 = 1;
  unsigned int num1 = (int)mytime.tv_sec;

  while (num1 / d1 >= 10) d1 *= 10;

  while (d1 != 0) {
    int digit1 = num1 / d1;
    num1 %= d1;
    d1 /= 10;
    if (n1 || digit1 > 0 || d1 == 0) {
      *trace_text_ptr++ = digit1 + '0';
      n1++;
    }
  }
  *trace_text_ptr++ = 0x0c;

  int n2 = 0;
  unsigned int d2 = 1;
  unsigned int num2 = (int)mytime.tv_nsec;

  while (num2 / d2 >= 10) d2 *= 10;

  while (d2 != 0) {
    int digit2 = num2 / d2;
    num2 %= d2;
    d2 /= 10;
    if (n2 || digit2 > 0 || d2 == 0) {
      *trace_text_ptr++ = digit2 + '0';
      n2++;
    }
  }
  *trace_text_ptr++ = 0x0c;

  if (!report_tid) {
    *trace_text_ptr++ = 0x30;
  } else {
    int n = 0;
    unsigned int d = 1;
    unsigned int num = (int)syscall(SYS_gettid);

    while (num / d >= 10) d *= 10;

    while (d != 0) {
      int digit = num / d;
      num %= d;
      d /= 10;
      if (n || digit > 0 || d == 0) {
        *trace_text_ptr++ = digit + '0';
        n++;
      }
    }
  }

  *trace_text_ptr++ = 0x0c;

  // obtain the trace string

  char ch;

  while (ch = *(format++)) {
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
              if (i > 256) break;
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

          while (num / d >= 10) d *= 10;

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

          while (num / d >= 10) d *= 10;

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

          while (num / d >= 16) d *= 16;

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

        default:
          break;
      }

    } else {
      *trace_text_ptr++ = ch;
    }
  }

  simplefu_mutex_lock(&myMutex);

  memcpy(trace_buffer_ptr, trace_text, trace_text_ptr - trace_text);
  trace_buffer_ptr += trace_text_ptr - trace_text;

  if ((trace_buffer_ptr - gtrace_buffer) > (gtracebuffersize - 1024)) {
    if (do_offload) {
      // clear unused and rewind to rewind ptr
      fprintf(stderr, "tdi: [%d][%s], rewind for offload\n", gpid, gprocname);
      int i;
      for (i = trace_buffer_ptr - gtrace_buffer; i < gtracebuffersize; i++) {
        gtrace_buffer[i] = 0;
      }
      trace_buffer_ptr = gtrace_buffer_rewind_ptr;
    } else {
      fprintf(stderr, "tdi: [%s][%d], full\n", gprocname, gpid);
      tditrace_inited = 0;
    }
  }

  simplefu_mutex_unlock(&myMutex);
}

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
