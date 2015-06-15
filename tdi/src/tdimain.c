
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/futex.h>

#include <dirent.h>

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

#define SIMPLEFU_MUTEX_INITIALIZER                                             \
    {                                                                          \
        { 1, 0 }                                                               \
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
#define QUEUES 3
#define EVENTS 4
#define NOTES 7

#define TRACEBUFFERSIZE (16 * 1024 * 1024)

static char *gtrace_buffer;
static char *trace_buffer_ptr;
int trace_counter;
static int tditrace_inited;
static int reported_full;
static int report_tid;

typedef unsigned long long _u64;

struct simplefu_mutex myMutex;

// 100 queues of 1000 chars each
static char tasks_array[1000][100];
static int nr_tasks = 0;

// 100 queues of 1000 chars each
static char queues_array[1000][100];
static int nr_queues = 0;
static int prev_queues[100];

// 100 events of 1000 chars each
static char events_array[1000][100];
static int nr_events = 0;

// 100 notes of 1000 chars each
static char notes_array[1000][100];
static int nr_notes = 0;

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

static void addentry(FILE *tdifile, char *text_in, _u64 timestamp,
                     char *procname, int pid, int tid) {
    int i;
    int entry;
    char fullentry[1024];

    char name[256];
    int value = 0;

    char text_in1[256];
    char *text = text_in1;

    if ((strncmp(text_in, "@T+", 3) == 0) ||
        (strncmp(text_in, "@T-", 3) == 0) ||
        (strncmp(text_in, "@E+", 3) == 0)) {

        strncpy(text_in1, text_in, 3);
        sprintf(&text_in1[3], "[%s][%d][%d]%s", procname, pid, tid,
                &text_in[3]);

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
            fwrite(fullentry, strlen(fullentry), 1, tdifile);
        }

        // Add the STA STO entry
        if (enter_not_exit) {
            sprintf(fullentry, "STA %d %d %lld\n", TASKS, TASKS * 1000 + entry,
                    timestamp);
            fwrite(fullentry, strlen(fullentry), 1, tdifile);
        } else {
            sprintf(fullentry, "STO %d %d %lld\n", TASKS, TASKS * 1000 + entry,
                    timestamp);
            fwrite(fullentry, strlen(fullentry), 1, tdifile);
        }

        // Add the DSC entry, replace any spaces with commas
        sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text);
        for (i = 8; i < (int)strlen(fullentry); i++) {
            if (fullentry[i] == 32)
                fullentry[i] = 0x2c;
        }
        fwrite(fullentry, strlen(fullentry), 1, tdifile);

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
            fwrite(fullentry, strlen(fullentry), 1, tdifile);
        }

        // Add the STA STO entry
        sprintf(fullentry, "STA %d %d %lld\n", EVENTS, EVENTS * 1000 + entry,
                timestamp);
        fwrite(fullentry, strlen(fullentry), 1, tdifile);
        sprintf(fullentry, "STO %d %d %lld\n", EVENTS, EVENTS * 1000 + entry,
                timestamp);
        fwrite(fullentry, strlen(fullentry), 1, tdifile);

        // Add the DSC entry, replace any spaces with commas
        sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text);
        for (i = 8; i < (int)strlen(fullentry); i++) {
            if (fullentry[i] == 32)
                fullentry[i] = 0x2c;
        }
        fwrite(fullentry, strlen(fullentry), 1, tdifile);

        return;
    }

    /*
     * QUEUES entry
     *
     * if text is of the form name~value then interpret this as a queue and add
     *a queue entry for it.
     * check whether '~' exists and pull out the name and value
     */
    name[0] = 0;
    for (i = 0; i < (int)strlen(text); i++) {
        if (text[i] == 126) {
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
            sprintf(fullentry, "NAM %d %d %s\n", QUEUES, QUEUES * 1000 + entry,
                    queues_array[entry]);
            fwrite(fullentry, strlen(fullentry), 1, tdifile);

            // reset the prev_value;
            prev_queues[entry] = 0;
        }

        // fill in the value
        // add a STA (with the delta) or a STO (with the delta) depending on
        // whether the value went up or went down
        if (value >= prev_queues[entry])
            sprintf(fullentry, "STA %d %d %lld %d\n", QUEUES,
                    QUEUES * 1000 + entry, timestamp,
                    value - prev_queues[entry]);
        else
            sprintf(fullentry, "STO %d %d %lld %d\n", QUEUES,
                    QUEUES * 1000 + entry, timestamp,
                    prev_queues[entry] - value);
        fwrite(fullentry, strlen(fullentry), 1, tdifile);
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
        fwrite(fullentry, strlen(fullentry), 1, tdifile);
    }

    // Add the OCC entry
    sprintf(fullentry, "OCC %d %d %lld\n", NOTES, NOTES * 1000 + entry,
            timestamp);
    fwrite(fullentry, strlen(fullentry), 1, tdifile);

    // Add the DSC entry, replace any spaces with commas
    sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text);
    for (i = 8; i < (int)strlen(fullentry); i++) {
        if (fullentry[i] == 32)
            fullentry[i] = 0x2c;
    }
    fwrite(fullentry, strlen(fullentry), 1, tdifile);
}

typedef struct {
    char filename[128];
    char procname[64];
    int pid;
    char *bufmmapped;
    char *buf;
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

    token = strtok_r(NULL, search, &tracebuffers[bid].saveptr);
    if (token) {

        // token is timestamp_sec
        timestamp_sec = atoi(token);

        token = strtok_r(NULL, search, &tracebuffers[bid].saveptr);
        if (token) {

            // token is timestamp_nsec
            timestamp_nsec = atoi(token);

            tracebuffers[bid].monotonic_timestamp =
                (_u64)timestamp_nsec + (_u64)timestamp_sec * (_u64)1000000000;

            token = strtok_r(NULL, search, &tracebuffers[bid].saveptr);
            if (token) {

                // token is tid
                tracebuffers[bid].tid = atoi(token);

                token = strtok_r(NULL, search, &tracebuffers[bid].saveptr);
                if (token) {

                    tracebuffers[bid].text = token;
                    tracebuffers[bid].valid = 1;
                }
            }
        }
    }
}

static void converttracetotdi(FILE *tdifile) {
    int i;
    const char *search = "\f";

    char *ptr;
    char *ptr1;

    int buffers = 0;

    DIR *dp;
    struct dirent *ep;

    dp = opendir("/tmp/");
    if (dp != NULL) {

        while (ep = readdir(dp)) {

            if (strncmp(ep->d_name, ".tditracebuffer:", 16) == 0) {

                FILE *file;

                sprintf(tracebuffers[buffers].filename, "/tmp/%s", ep->d_name);

                if ((file = fopen(tracebuffers[buffers].filename, "r")) !=
                    NULL) {

                    /* /tmp/.tditracebuffer-xxx-xxx */

                    printf("Found \"%s\"\n", tracebuffers[buffers].filename);

                    strncpy(tracebuffers[buffers].procname,
                            (const char *)&tracebuffers[buffers].filename[21],
                            strchr(&tracebuffers[buffers].filename[21], ':') -
                                &tracebuffers[buffers].filename[21]);

                    tracebuffers[buffers].pid = atoi(
                        &tracebuffers[buffers]
                             .filename[22 +
                                       strlen(tracebuffers[buffers].procname)]);

                    struct stat st;
                    stat(tracebuffers[buffers].filename, &st);

                    tracebuffers[buffers].bufmmapped = (char *)mmap(
                        0, st.st_size, PROT_READ, MAP_SHARED, fileno(file), 0);
                    tracebuffers[buffers].bufsize = st.st_size;
                    tracebuffers[buffers].buf = malloc(st.st_size);
                    memcpy(tracebuffers[buffers].buf,
                           tracebuffers[buffers].bufmmapped, st.st_size);
                    tracebuffers[buffers].ptr = tracebuffers[buffers].buf;

                    // token should hold "TDITRACEBUFFER"
                    if (strncmp("TDITRACEBUFFER",
                                strtok_r(tracebuffers[buffers].ptr, search,
                                         &tracebuffers[buffers].saveptr),
                                14) != 0) {
                        printf("Invalid tracebuffer, skipping\n");
                        break;
                    }

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

        closedir(dp);
    }

    if (buffers == 0) {

        fprintf(stderr, "Not found: \"/tmp/.tditracebuffer:*:*\"\n");
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

    fprintf(tdifile, "TIME %d\n", 1000000000);
    fprintf(tdifile, "SPEED %d\n", 1000000000);
    fprintf(tdifile, "DNM 0 0 >\n");

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

        addentry(tdifile, tracebuffers[d].text,

                 tracebuffers[d].timeofday_offset - abs_timeofday +
                     tracebuffers[d].monotonic_timestamp -
                     tracebuffers[d].monotonic_offset,

                 tracebuffers[d].procname, tracebuffers[d].pid,
                 tracebuffers[d].tid);

        pctused = (((tracebuffers[d].text - tracebuffers[d].buf) * 100.0) /
                   tracebuffers[d].bufsize) +
                  1;

        last_timestamp = tracebuffers[d].timeofday_offset - abs_timeofday +
                         tracebuffers[d].monotonic_timestamp -
                         tracebuffers[d].monotonic_offset;

        parse(d);

        if (!tracebuffers[d].valid) {
            printf("\"%s\" final entry (%d%% used)\n", tracebuffers[d].filename,
                   pctused);
        }
    }

    // Add one more entry 0.1 sec behind all the previous ones
    addentry(tdifile, "TDITRACE_EXIT\0", last_timestamp + 100 * 1000000, "", 0,
             0);

    fprintf(tdifile, "END %lld\n", abs_timeofday);
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
            target_result =
                readlink(exe_link, target_name, sizeof(target_name) - 1);
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

int tditrace_init(void) {
    struct timeval mytime;
    int i;

    if (tditrace_inited) {
        return 0;
    }

    pid_t pid = getpid();
    char procname[128];

    get_process_name_by_pid(pid, procname);

    char tracebufferfilename[128];

    sprintf(tracebufferfilename, (char *)"/tmp/.tditracebuffer:%s:%d", procname,
            pid);

    /*
     * remove inactive tracefiles
     */

    DIR *dp;
    struct dirent *ep;

    dp = opendir("/tmp/");
    if (dp != NULL) {
        while (ep = readdir(dp)) {

            if (strncmp(ep->d_name, ".tditracebuffer:", 16) == 0) {

                char procpid[128];
                sprintf(procpid, (char *)"/proc/%d",
                        atoi(strrchr(ep->d_name, ':') + 1));

                char fullname[128];
                sprintf(fullname, "/tmp/%s", ep->d_name);

                struct stat sts;
                if (stat(procpid, &sts) == -1) {

                    printf("tdi: init, removed: \"%s\"\n", fullname);
                    unlink(fullname);
                } else {

                    printf("tdi: init, not removed: \"%s\"\n", fullname);
                }
            }
        }

        closedir(dp);
    }

    // find_process_name("tditest");

    FILE *file;
    if ((file = fopen(tracebufferfilename, "w+")) == 0) {
        fprintf(stderr, "Error creating file \"%s\"", tracebufferfilename);
        return -1;
    }

    (void)ftruncate(fileno(file), TRACEBUFFERSIZE);

    gtrace_buffer = (char *)mmap(0, TRACEBUFFERSIZE, PROT_READ | PROT_WRITE,
                                 MAP_SHARED, fileno(file), 0);

    for (i = 0; i < TRACEBUFFERSIZE; i++) {
        gtrace_buffer[i] = 0;
    }

    printf("tdi: init, allocated: \"%s\" (16MB)\n", tracebufferfilename);

    trace_buffer_ptr = gtrace_buffer;
    trace_counter = 0;

    // write one time start text
    trace_buffer_ptr += sprintf(trace_buffer_ptr, (char *)"TDITRACEBUFFER\f");
    ;

    // obtain timeofday timestamp and write to buffer
    trace_buffer_ptr +=
        sprintf(trace_buffer_ptr, (char *)"%lld\f", timestamp_timeofday_nsec());

    // obtain monotonic timestamp and write to buffer
    trace_buffer_ptr +=
        sprintf(trace_buffer_ptr, (char *)"%lld\f", timestamp_monotonic_nsec());

    reported_full = 0;

    report_tid = (getenv("TID") != NULL);

    simplefu_mutex_init(&myMutex);

    simplefu_mutex_lock(&myMutex);

    tditrace_inited = 1;

    simplefu_mutex_unlock(&myMutex);

    return 0;
}

void tditrace_rewind() {
    struct timeval mytime;
    int i;

    simplefu_mutex_lock(&myMutex);

    for (i = 0; i < TRACEBUFFERSIZE; i++) {
        gtrace_buffer[i] = 0;
    }

    trace_buffer_ptr = gtrace_buffer;
    trace_counter = 0;

    // write one time start text
    trace_buffer_ptr += sprintf(trace_buffer_ptr, (char *)"TDITRACEBUFFER\f");

    // obtain timeofday timestamp and write to buffer
    trace_buffer_ptr +=
        sprintf(trace_buffer_ptr, (char *)"%lld\f", timestamp_timeofday_nsec());

    // obtain monotonic timestamp and write to buffer
    trace_buffer_ptr +=
        sprintf(trace_buffer_ptr, (char *)"%lld\f", timestamp_monotonic_nsec());

    reported_full = 0;

    simplefu_mutex_unlock(&myMutex);
}

void tditrace_exit(char *filename) {
    // Create the TDI file from the rough traces
    FILE *tdifile;

    if ((tdifile = fopen(filename, "w+")) == NULL) {
        printf("Could not create trace file: [%s]\n", filename);
        return;
    }

    printf("Writing tdi file \"%s\"...\n", filename);

    converttracetotdi(tdifile);

    fclose(tdifile);

    chmod(filename, 0666);

    printf("Done\n");
}

void tditrace(const char *format, ...) {
    va_list args;

    if (!tditrace_inited) {
        return;
    }

    struct timespec mytime;
    clock_gettime(CLOCK_MONOTONIC, &mytime);

    simplefu_mutex_lock(&myMutex);

    if ((trace_buffer_ptr - gtrace_buffer) > (TRACEBUFFERSIZE - 500)) {
        if (!reported_full) {
            printf("tdi: full\n");
            reported_full = 1;
        }
        simplefu_mutex_unlock(&myMutex);
        return;
    }

    // Add the string to the tracebuffer, and add timestamp
    // layout is "\ftimestamp_sec\ftimestamp_nsec\ftid\ftracestring"

    trace_counter++;

    *trace_buffer_ptr++ = 0x0c;

    int n1 = 0;
    unsigned int d1 = 1;
    unsigned int num1 = (int)mytime.tv_sec;

    while (num1 / d1 >= 10)
        d1 *= 10;

    while (d1 != 0) {
        int digit1 = num1 / d1;
        num1 %= d1;
        d1 /= 10;
        if (n1 || digit1 > 0 || d1 == 0) {
            *trace_buffer_ptr++ = digit1 + '0';
            n1++;
        }
    }
    *trace_buffer_ptr++ = 0x0c;

    int n2 = 0;
    unsigned int d2 = 1;
    unsigned int num2 = (int)mytime.tv_nsec;

    while (num2 / d2 >= 10)
        d2 *= 10;

    while (d2 != 0) {
        int digit2 = num2 / d2;
        num2 %= d2;
        d2 /= 10;
        if (n2 || digit2 > 0 || d2 == 0) {
            *trace_buffer_ptr++ = digit2 + '0';
            n2++;
        }
    }
    *trace_buffer_ptr++ = 0x0c;

    if (!report_tid) {
        *trace_buffer_ptr++ = 0x30;
    } else {

        int n = 0;
        unsigned int d = 1;
        unsigned int num = (int)syscall(SYS_gettid);

        while (num / d >= 10)
            d *= 10;

        while (d != 0) {
            int digit = num / d;
            num %= d;
            d /= 10;
            if (n || digit > 0 || d == 0) {
                *trace_buffer_ptr++ = digit + '0';
                n++;
            }
        }
    }

    *trace_buffer_ptr++ = 0x0c;

    // obtain the trace string
    va_start(args, format);

    char ch;

    va_start(args, format);

    while (ch = *(format++)) {

        if (ch == '%') {

            switch (ch = (*format++)) {

            case 's': {
                char *s;
                s = va_arg(args, char *);
                if (s) {
                    while (*s)
                        *trace_buffer_ptr++ = *s++;
                } else {
                    *trace_buffer_ptr++ = 'n';
                    *trace_buffer_ptr++ = 'i';
                    *trace_buffer_ptr++ = 'l';
                    *trace_buffer_ptr++ = 'l';
                }
                break;
            }
            case 'd':
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
                        *trace_buffer_ptr++ = digit + '0';
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
                        *trace_buffer_ptr++ = dgt + (dgt < 10 ? '0' : 'a' - 10);
                        ++n;
                    }
                }
                break;
            }

            default:
                break;
            }

        } else {
            *trace_buffer_ptr++ = ch;
        }
    }

    va_end(args);

    simplefu_mutex_unlock(&myMutex);
}
