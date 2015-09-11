
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
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
#define ISRS 1
#define SEMAS 2
#define QUEUES 3
#define EVENTS 4
#define NOTES 7
#define AGENTS 8

#define TRACEBUFFERSIZE (16 * 1024 * 1024)

char gtracebufferfilename[128];
struct stat gtrace_buffer_st;

static char *gtrace_buffer;
static char *trace_buffer_ptr;
static char *gtrace_buffer_rewind_ptr;

int trace_counter;
static int tditrace_inited;
static int reported_full;
static int report_tid;
static int report_tditrace;

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

    if ((strncmp(text_in, "@T+", 3) == 0) ||
        (strncmp(text_in, "@T-", 3) == 0) ||
        (strncmp(text_in, "@I+", 3) == 0) ||
        (strncmp(text_in, "@I-", 3) == 0) ||
        (strncmp(text_in, "@A+", 3) == 0) ||
        (strncmp(text_in, "@A-", 3) == 0) ||
        (strncmp(text_in, "@S+", 3) == 0) ||
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
            fwrite(fullentry, strlen(fullentry), 1, stdout);
        }

        // Add the STA or STO entry
        sprintf(fullentry, "%s %d %d %lld\n", enter_not_exit ? "STA" : "STO",
                TASKS, TASKS * 1000 + entry, timestamp);
        fwrite(fullentry, strlen(fullentry), 1, stdout);

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
            if (fullentry[i] == 32)
                fullentry[i] = 0x2c;
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
        sprintf(fullentry, "%s %d %d %lld\n", enter_not_exit ? "STA" : "STO",
                ISRS, ISRS * 1000 + entry, timestamp);
        fwrite(fullentry, strlen(fullentry), 1, stdout);

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
            if (fullentry[i] == 32)
                fullentry[i] = 0x2c;
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
            if (fullentry[i] == 32)
                fullentry[i] = 0x2c;
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
            if (text[i] == 32)
                break;
            if (text[i] == 126) {

                // fprintf(stderr, "text1=\"%s\"\n", text);

                strncpy(name, text, i);
                name[i] = 0;
                value = atoi(text + i + 1);
                text[i] = 32; // create split marker

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
            sprintf(fullentry, "STA %d %d %lld %d\n", QUEUES,
                    QUEUES * 1000 + entry, timestamp,
                    value - prev_queues[entry]);
        else
            sprintf(fullentry, "STO %d %d %lld %d\n", QUEUES,
                    QUEUES * 1000 + entry, timestamp,
                    prev_queues[entry] - value);
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

char proc_self_maps[32 * 1024 + 1];

static void dump_proc_self_maps(void) {
    int fd;
    int bytes;

    fd = open("/proc/self/maps", O_RDONLY);
    if (fd < 0)
        return;

    while (1) {
        bytes = read(fd, proc_self_maps, sizeof(proc_self_maps) - 1);
        if ((bytes == -1) && (errno == EINTR))
            /* keep trying */;
        else if (bytes > 0) {
            proc_self_maps[bytes] = '\0';
            // printf("bytes=%d[%s]\n", bytes, proc_self_maps);
        } else
            break;
    }

    close(fd);

    char *line = strtok(proc_self_maps, "\n");
    while (line) {
        if (strlen(line) > 50) {
            // printf("%d[%s]\n", strlen(line), line);
            tditrace("MAPS %s", line);
        }
        line = strtok(NULL, "\n");
    }
}


static int monitor;

void *monitor_thread(void *param) {

    static int seconds_counter = 0;
    static int do_dump_proc_self_maps = 1;
    while (1) {

        usleep(1 * 1000000);

        seconds_counter++;

        if (do_dump_proc_self_maps) {
            dump_proc_self_maps();
            do_dump_proc_self_maps = 0;
        }

        struct mallinfo mi;

        mi = mallinfo();

        // printf("Total non-mmapped bytes (arena):       %d\n", mi.arena);
        // printf("# of free chunks (ordblks):            %d\n", mi.ordblks);
        // printf("# of free fastbin blocks (smblks):     %d\n", mi.smblks);
        // printf("# of mapped regions (hblks):           %d\n", mi.hblks);
        // printf("Bytes in mapped regions (hblkhd):      %d\n", mi.hblkhd);
        // printf("Max. total allocated space (usmblks):  %d\n", mi.usmblks);
        // printf("Free bytes held in fastbins (fsmblks): %d\n", mi.fsmblks);
        // printf("Total allocated space (uordblks):      %d\n", mi.uordblks);
        // printf("Total free space (fordblks):           %d\n", mi.fordblks);
        // printf("Topmost releasable block (keepcost):   %d\n", mi.keepcost);

        tditrace("mi.arena~%d", mi.arena);
        tditrace("mi.ordblks~%d", mi.ordblks);
        tditrace("mi.smblks~%d", mi.smblks);
        tditrace("mi.hblks~%d", mi.hblks);
        tditrace("mi.hblkhd~%d", mi.hblkhd);
        tditrace("mi.usmblks~%d", mi.usmblks);
        tditrace("mi.fsmblks~%d", mi.fsmblks);
        tditrace("mi.uordblks~%d", mi.uordblks);
        tditrace("mi.fordblks~%d", mi.fordblks);
        tditrace("mi.keepcost~%d", mi.keepcost);

        struct stat st;
        stat(gtracebufferfilename, &st);

        if (st.st_mtim.tv_sec != gtrace_buffer_st.st_mtim.tv_sec) {
            stat(gtracebufferfilename, &gtrace_buffer_st);

            printf("tdi: init[%d][%s], rewinding...\n", gpid, gprocname);
            tditrace_rewind();
            do_dump_proc_self_maps = 1;
        }
    }

    pthread_exit(NULL);
}


static int thedelay;

void *delayed_init_thread(void *param) {

    int *pdelay = (int *)param;
    int delay = *pdelay;

    printf("tdi: init[%d][%s], delay is %d\n", gpid, gprocname, delay);

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
            strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S",
                     ptm);

            if (tv.tv_sec > (45 * 365 * 24 * 3600)) {
                printf(
                    "tdi: init[%d][%s], delay until timeofday is set, \"%s\", "
                    "timeofday is set\n",
                    gpid, gprocname, time_string);
                break;
            }

            ptm = localtime(&tv.tv_sec);
            strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S",
                     ptm);
            printf("tdi: init[%d][%s], delay until timeofday is set, \"%s\", "
                   "timeofday "
                   "is not set\n",
                   gpid, gprocname, time_string);

            usleep(1 * 1000000);
        }
    }

    else if (delay < -1) {
        /*
         * wait until tracebuffer modification time is chaned.
         */

        while (1) {
            printf("tdi: init[%d][%s], paused...\n", gpid, gprocname);
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
                printf("tdi: init[%d][%s], started...\n", gpid, gprocname);

                stat(gtracebufferfilename, &gtrace_buffer_st);
                break;
            }
        }

    } else {

        while (delay > 0) {
            printf("tdi: init[%d][%s], delay %d second(s)...\n", gpid,
                   gprocname, delay);
            usleep(1 * 1000000);

            struct stat st;
            stat(gtracebufferfilename, &st);

            if (st.st_atim.tv_sec != gtrace_buffer_st.st_atim.tv_sec) {
            }

            delay--;
        }
    }

    simplefu_mutex_lock(&myMutex);
    tditrace_inited = 1;
    simplefu_mutex_unlock(&myMutex);

    pthread_t monitor_thread_id;
    pthread_create(&monitor_thread_id, NULL, monitor_thread, &monitor);

    pthread_exit(NULL);
}


int tditrace_init(void) {
    struct timeval mytime;
    int i;

    if (tditrace_inited) {
        return 0;
    }

    gpid = getpid();
    get_process_name_by_pid(gpid, gprocname);

    if (strcmp(gprocname, "mkdir") == 0) {
        printf("tdi: init[%d][%s], procname is \"mkdir\" ; not tracing\n", gpid,
               gprocname);
        return;
    } else if (strcmp(gprocname, "sh") == 0) {
        printf("tdi: init[%d][%s], procname is \"sh\" ; not tracing\n", gpid,
               gprocname);
        return;
    } else if (strcmp(gprocname, "strace") == 0) {
        printf("tdi: init[%d][%s], procname is \"strace\" ; not tracing\n",
               gpid, gprocname);
        return;
    } else {
        printf("tdi: init[%d][%s]\n", gpid, gprocname);
    }

    static int do_mallinfo = 0;
    if (getenv("MALLINFO")) {
        do_mallinfo = atoi(getenv("REMOVE"));
    }

    /*
     * remove inactive tracefiles
     */

    int remove = 1;
    if (getenv("REMOVE")) {
        remove = (atoi(getenv("REMOVE")) >= 1);
    }

    if (remove) {
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

                    if (stat(procpid, &sts) != -1) {
                        printf("found \"%s\"\n", procpid);
                    }

                    if (stat(procpid, &sts) == -1) {

                        unlink(fullname);
                        printf("tdi: init[%d][%s], removed: \"%s\"\n", gpid,
                               gprocname, fullname);
                    } else {

                        printf("tdi: init[%d][%s], not removed: \"%s\"\n", gpid,
                               gprocname, fullname);
                    }
                }
            }

            closedir(dp);
        }
    }

    sprintf(gtracebufferfilename, (char *)"/tmp/.tditracebuffer:%s:%d",
            gprocname, gpid);

    FILE *file;
    if ((file = fopen(gtracebufferfilename, "w+")) == 0) {
        fprintf(stderr, "Error creating file \"%s\"", gtracebufferfilename);
        return -1;
    }

    i = ftruncate(fileno(file), TRACEBUFFERSIZE);

    gtrace_buffer = (char *)mmap(0, TRACEBUFFERSIZE, PROT_READ | PROT_WRITE,
                                 MAP_SHARED, fileno(file), 0);

    stat(gtracebufferfilename, &gtrace_buffer_st);

    for (i = 0; i < TRACEBUFFERSIZE; i++) {
        gtrace_buffer[i] = 0;
    }

    printf("tdi: init[%d][%s], allocated: \"%s\" (16MB)\n", gpid, gprocname,
           gtracebufferfilename);

    trace_buffer_ptr = gtrace_buffer;
    trace_counter = 0;

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

    printf("tdi: init[%d][%s], timeofday_timestamp:%lld, "
           "monotonic_timestamp:%lld\n",
           gpid, gprocname, timeofday_timestamp, monotonic_timestamp);

    reported_full = 0;

    if (getenv("TID")) {
        report_tid = (atoi(getenv("TID")) >= 1);
    } else {
        report_tid = 0;
    }

    if (getenv("TDITRACE")) {
        report_tditrace = (atoi(getenv("TDITRACE")) >= 1);
    } else {
        report_tditrace = 1;
    }

    simplefu_mutex_init(&myMutex);

    if (getenv("DELAY")) {
        thedelay = atoi(getenv("DELAY"));
        pthread_t delayed_init_thread_id;
        pthread_create(&delayed_init_thread_id, NULL, delayed_init_thread,
                       &thedelay);

        // pthread_join(delayed_init_thread_id, NULL);
    } else {

        simplefu_mutex_lock(&myMutex);
        tditrace_inited = 1;
        simplefu_mutex_unlock(&myMutex);

        pthread_t monitor_thread_id;
        pthread_create(&monitor_thread_id, NULL, monitor_thread, &monitor);
    }
}

void tditrace_rewind() {
    struct timeval mytime;
    int i;

    simplefu_mutex_lock(&myMutex);
    tditrace_inited = 0;
    simplefu_mutex_unlock(&myMutex);

    for (i = gtrace_buffer_rewind_ptr - gtrace_buffer; i < TRACEBUFFERSIZE;
         i++) {
        gtrace_buffer[i] = 0;
    }

    trace_buffer_ptr = gtrace_buffer_rewind_ptr;
    trace_counter = 0;

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

            if (strstr(argv[tracebufferid], ".tditracebuffer:") != 0) {

                FILE *file;

                sprintf(tracebuffers[buffers].filename, "%s",
                        argv[tracebufferid]);

                if ((file = fopen(tracebuffers[buffers].filename, "r")) !=
                    NULL) {

                    /* /tmp/.tditracebuffer-xxx-xxx */

                    fprintf(stderr, "Found \"%s\"\n",
                            tracebuffers[buffers].filename);

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

                    if (st.st_size == 0) {
                        printf("empty tracebuffer, skipping\n");
                        break;
                    }

                    tracebuffers[buffers].bufmmapped =
                        (char *)mmap(0, st.st_size, PROT_READ | PROT_WRITE,
                                     MAP_PRIVATE, fileno(file), 0);
                    tracebuffers[buffers].bufsize = st.st_size;

                    tracebuffers[buffers].ptr =
                        tracebuffers[buffers].bufmmapped;

                    // token should hold "TDITRACEBUFFER"
                    if (strncmp("TDITRACEBUFFER",
                                strtok_r(tracebuffers[buffers].ptr, search,
                                         &tracebuffers[buffers].saveptr),
                                14) != 0) {
                        fprintf(stderr, "invalid tracebuffer, skipping\n");
                        break;
                    }

                    fprintf(stderr, "bufpct=%d\n",
                            (int)(strlen(tracebuffers[buffers].saveptr) *
                                  100.0 / (TRACEBUFFERSIZE)));

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

                if (strncmp(ep->d_name, ".tditracebuffer:", 16) == 0) {

                    FILE *file;

                    sprintf(tracebuffers[buffers].filename, "/tmp/%s",
                            ep->d_name);

                    if ((file = fopen(tracebuffers[buffers].filename, "r")) !=
                        NULL) {

                        /* /tmp/.tditracebuffer-xxx-xxx */

                        fprintf(stderr, "Found \"%s\"\n",
                                tracebuffers[buffers].filename);

                        strncpy(
                            tracebuffers[buffers].procname,
                            (const char *)&tracebuffers[buffers].filename[21],
                            strchr(&tracebuffers[buffers].filename[21], ':') -
                                &tracebuffers[buffers].filename[21]);

                        tracebuffers[buffers].pid = atoi(
                            &tracebuffers[buffers].filename
                                 [22 + strlen(tracebuffers[buffers].procname)]);

                        struct stat st;
                        stat(tracebuffers[buffers].filename, &st);

                        if (st.st_size == 0) {
                            printf("empty tracebuffer, skipping\n");
                            break;
                        }

                        tracebuffers[buffers].bufmmapped =
                            (char *)mmap(0, st.st_size, PROT_READ | PROT_WRITE,
                                         MAP_PRIVATE, fileno(file), 0);
                        tracebuffers[buffers].bufsize = st.st_size;
                        tracebuffers[buffers].ptr =
                            tracebuffers[buffers].bufmmapped;

                        // token should hold "TDITRACEBUFFER"
                        if (strncmp("TDITRACEBUFFER",
                                    strtok_r(tracebuffers[buffers].ptr, search,
                                             &tracebuffers[buffers].saveptr),
                                    14) != 0) {
                            fprintf(stderr, "tdi: init[%d][%s], invalid "
                                            "tracebuffer, skipping\n",
                                    gpid, gprocname);
                            break;
                        }

                        fprintf(stderr, "bufpct=%d\n",
                                (int)(strlen(tracebuffers[buffers].saveptr) *
                                      100.0 / (TRACEBUFFERSIZE)));

                        // token should hold "timeofday timestamp offset in
                        // nsecs"
                        tracebuffers[buffers].timeofday_offset =
                            (_u64)atoll(strtok_r(
                                NULL, search, &tracebuffers[buffers].saveptr));

                        tracebuffers[buffers].monotonic_offset =
                            (_u64)atoll(strtok_r(
                                NULL, search, &tracebuffers[buffers].saveptr));

                        fclose(file);

                        buffers++;
                    }
                }
            }

            closedir(dp);
        }
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

        pctused =
            (((tracebuffers[d].text - tracebuffers[d].bufmmapped) * 100.0) /
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
    addentry(stdout, "TDITRACE_EXIT\0", last_timestamp + 100 * 1000000, "", 0,
             0);

    struct timespec atime;

    atime.tv_sec = abs_timeofday / 1000000000;
    atime.tv_nsec = abs_timeofday - atime.tv_sec * 1000000000;

    fprintf(stdout, "END %lld UTC %s", abs_timeofday,
            asctime(gmtime((const time_t *)&atime)));
}

void tditrace(const char *format, ...) {
    va_list args;

    if (!tditrace_inited) {
        return;
    }

    if (!report_tditrace) {
        return;
    }

    struct timespec mytime;
    clock_gettime(CLOCK_MONOTONIC, &mytime);

    simplefu_mutex_lock(&myMutex);

    if ((trace_buffer_ptr - gtrace_buffer) > (TRACEBUFFERSIZE - 500)) {
        if (!reported_full) {
            printf("tdi: full[%d][%s]\n", gpid, gprocname);
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

void tditrace_ex(const char *format, ...) {
    va_list args;

    if (!tditrace_inited) {
        return;
    }

    struct timespec mytime;
    clock_gettime(CLOCK_MONOTONIC, &mytime);

    simplefu_mutex_lock(&myMutex);

    if ((trace_buffer_ptr - gtrace_buffer) > (TRACEBUFFERSIZE - 500)) {
        if (!reported_full) {
            printf("tdi: full[%d][%s]\n", gpid, gprocname);
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
