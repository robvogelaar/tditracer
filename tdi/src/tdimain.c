
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
#include <sys/mman.h>
#include <linux/futex.h>


struct simplefu_semaphore
{
    int avail;
    int waiters;
};

typedef struct simplefu_semaphore *simplefu;

struct simplefu_mutex
{
    struct simplefu_semaphore sema;
};

typedef struct simplefu_mutex *simplemu;

void simplefu_down(simplefu who);
void simplefu_up(simplefu who);

void simplefu_mutex_init(simplemu mx);
void simplefu_mutex_lock(simplemu mx);
void simplefu_mutex_unlock(simplemu mx);

#define SIMPLEFU_MUTEX_INITIALIZER { {1, 0} }

void simplefu_mutex_init(simplemu mx)
{
    mx->sema.avail = 1;
    mx->sema.waiters = 0;
}

void simplefu_mutex_lock(simplemu mx)
{
    simplefu_down(&mx->sema);
}

void simplefu_mutex_unlock(simplemu mx)
{
    simplefu_up(&mx->sema);
}

void simplefu_down(simplefu who)
{
    int val;
    do {
        val = who->avail;
        if (val > 0 && __sync_bool_compare_and_swap(&who->avail, val, val - 1))
            return;
        __sync_fetch_and_add(&who->waiters, 1);
        syscall(__NR_futex, &who->avail, FUTEX_WAIT, val, NULL, 0, 0);
        __sync_fetch_and_sub(&who->waiters, 1);
    } while(1);
}

void simplefu_up(simplefu who)
{
    int nval = __sync_add_and_fetch(&who->avail, 1);
    if (who->waiters > 0) {
        syscall(__NR_futex, &who->avail, FUTEX_WAKE, nval, NULL, 0, 0);
    }
}


#define TASKS   0
#define QUEUES  3
#define EVENTS  4
#define NOTES   7

#define TRACEBUFFERSIZE  (16*1024*1024)

static  char    *gtrace_buffer;
static  char    *trace_buffer_ptr;
        int     trace_counter;
static  int     tditrace_inited;
static  int     reported_full;
static  int     report_tid;

typedef unsigned long long  _u64;

struct simplefu_mutex myMutex;


// 100 queues of 1000 chars each
static char tasks_array[1000][100];
static int  nr_tasks = 0;

// 100 queues of 1000 chars each
static char queues_array[1000][100];
static int  nr_queues = 0;
static int  prev_queues[100];

// 100 events of 1000 chars each
static char events_array[1000][100];
static int  nr_events = 0;

// 100 notes of 1000 chars each
static char notes_array[1000][100];
static int  nr_notes = 0;

static _u64 timestamp_nsec(void)
{
    struct timespec mytime;

    clock_gettime(CLOCK_MONOTONIC, &mytime);
    return ((_u64)((_u64)mytime.tv_nsec + (_u64)mytime.tv_sec * (_u64)1000000000));
}

static void addentry(FILE *tdifile, char *text_in, _u64 timestamp, int tid)
{
    int     i;
    int     entry;
    char    fullentry[1024];

    char    name[256];
    int     value = 0;

    char    text_in1[256];
    char    *text = text_in1;

    if ((strncmp(text_in,"@T+",3)==0) || (strncmp(text_in,"@T-",3)==0) || (strncmp(text_in,"@E+",3)==0)) {

        strncpy(text_in1, text_in, 3);
        sprintf(&text_in1[3], "[%d]%s", tid, &text_in[3]);

    } else {
        sprintf(text_in1, "[%d]%s", tid, text_in);
    }

    // get rid of any '\n', replace with '_'
    for (i = 0 ; i < (int)strlen(text) ; i++) {
        if ((text[i] == 13) || (text[i] == 10)) {
            text[i] = 0x5f;
        }
    }


    /*
     * TASKS entry
     *
     */
    if ((strncmp(text,"@T+",3)==0) || (strncmp(text,"@T-",3)==0)) {

        int enter_not_exit = (strncmp(text+2,"+",1)==0);

        text+= 3;
        // if the entry has not been seen before, add a new entry for it and issue a NAM
        entry = -1;
        for (i = 0 ; i < nr_tasks ; i++) {
            char    *pos;
            char    comparestr[1024];

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
            int     len;
            char    *pos;

            pos = strchr(text, ' ');
            if (pos) len = pos - text;
            else len = strlen(text);

            strncpy(tasks_array[nr_tasks], text, len);
            entry = nr_tasks;
            nr_tasks++;

            // Since we added a new entry we need to create a NAM, with only the first part of the text
            sprintf(fullentry, "NAM %d %d %s\n", TASKS, TASKS * 1000 + entry, tasks_array[entry]);
            fwrite(fullentry, strlen(fullentry), 1, tdifile);
        }

        // Add the STA STO entry
        if (enter_not_exit) {
            sprintf(fullentry, "STA %d %d %lld\n", TASKS, TASKS * 1000 + entry, timestamp);
            fwrite(fullentry, strlen(fullentry), 1, tdifile);
        } else {
            sprintf(fullentry, "STO %d %d %lld\n", TASKS, TASKS * 1000 + entry, timestamp);
            fwrite(fullentry, strlen(fullentry), 1, tdifile);
        }

        // Add the DSC entry, replace any spaces with commas
        sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text);
        for (i = 8 ; i < (int)strlen(fullentry) ; i++) {
            if (fullentry[i] == 32) fullentry[i] = 0x2c;
        }
        fwrite(fullentry, strlen(fullentry), 1, tdifile);

        return;
    }


    /*
     * EVENTS entry
     *
     */
    if (strncmp(text,"@E+",3)==0) {

        text+= 3;
        // if the entry has not been seen before, add a new entry for it and issue a NAM
        entry = -1;
        for (i = 0 ; i < nr_events ; i++) {
            char    *pos;
            char    comparestr[1024];

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
            int     len;
            char    *pos;

            pos = strchr(text, ' ');
            if (pos) len = pos - text;
            else len = strlen(text);

            strncpy(events_array[nr_events], text, len);
            entry = nr_events;
            nr_events++;

            // Since we added a new entry we need to create a NAM, with only the first part of the text
            sprintf(fullentry, "NAM %d %d %s\n", EVENTS, EVENTS * 1000 + entry, events_array[entry]);
            fwrite(fullentry, strlen(fullentry), 1, tdifile);
        }


        // Add the STA STO entry
        sprintf(fullentry, "STA %d %d %lld\n", EVENTS, EVENTS * 1000 + entry, timestamp);
        fwrite(fullentry, strlen(fullentry), 1, tdifile);
        sprintf(fullentry, "STO %d %d %lld\n", EVENTS, EVENTS * 1000 + entry, timestamp);
        fwrite(fullentry, strlen(fullentry), 1, tdifile);

        // Add the DSC entry, replace any spaces with commas
        sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text);
        for (i = 8 ; i < (int)strlen(fullentry) ; i++) {
            if (fullentry[i] == 32) fullentry[i] = 0x2c;
        }
        fwrite(fullentry, strlen(fullentry), 1, tdifile);

        return;
    }



    /*
     * Is it a QUEUES entry
     *
     * if text is of the form name~value then interpret this as a queue and add a queue entry for it.
     * check whether '~' exists and pull out the name and value
     */
    name[0] = 0;
    for (i = 0 ; i < (int)strlen(text) ; i++) {
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

        for (i = 0 ; i < nr_queues ; i++) {
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
            sprintf(fullentry, "NAM %d %d %s\n", QUEUES, QUEUES * 1000 + entry, queues_array[entry]);
            fwrite(fullentry, strlen(fullentry), 1, tdifile);

            // reset the prev_value;
            prev_queues[entry] = 0;
        }

        // fill in the value
        // add a STA (with the delta) or a STO (with the delta) depending on whether the value went up or went down
        if (value >= prev_queues[entry])
            sprintf(fullentry, "STA %d %d %lld %d\n", QUEUES, QUEUES * 1000 + entry, timestamp, value-prev_queues[entry]);
        else
            sprintf(fullentry, "STO %d %d %lld %d\n", QUEUES, QUEUES * 1000 + entry, timestamp, prev_queues[entry]-value);
        fwrite(fullentry, strlen(fullentry), 1, tdifile);
        prev_queues[entry] = value;

        return;
    }



    /*
     * Treat everything else as a NOTES entry
     *
     */

    // if the entry has not been seen before, add a new entry for it and issue a NAM
    entry = -1;
    for (i = 0 ; i < nr_notes ; i++) {
        char    *pos;
        char    comparestr[1024];

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
        int     len;
        char    *pos;

        pos = strchr(text, ' ');
        if (pos) len = pos - text;
        else len = strlen(text);

        strncpy(notes_array[nr_notes], text, len);
        entry = nr_notes;
        nr_notes++;

        // Since we added a new entry we need to create a NAM, with only the first part of the text
        sprintf(fullentry, "NAM %d %d %s\n", NOTES, NOTES * 1000 + entry, notes_array[entry]);
        fwrite(fullentry, strlen(fullentry), 1, tdifile);
    }

    // Add the OCC entry
    sprintf(fullentry, "OCC %d %d %lld\n", NOTES, NOTES * 1000 + entry, timestamp);
    fwrite(fullentry, strlen(fullentry), 1, tdifile);

    // Add the DSC entry, replace any spaces with commas
    sprintf(fullentry, "DSC %d %d %s\n", 0, 0, text);
    for (i = 8 ; i < (int)strlen(fullentry) ; i++) {
        if (fullentry[i] == 32) fullentry[i] = 0x2c;
    }
    fwrite(fullentry, strlen(fullentry), 1, tdifile);
}


static void converttracetotdi(FILE* tdifile, char* trace_buffer, unsigned long long specific_timestamp_offset)
{
    char        *token;
    const char  *search = "\f";
    char        *ptr;
    char        *ptr1;

    if (!trace_buffer) {
        trace_buffer = gtrace_buffer;
    }

    // strtok modifies the buffer, make a copy and work with the copy
    // such that we can call tdidump more than once
    ptr = (char*)malloc(TRACEBUFFERSIZE);
    memcpy(ptr, trace_buffer, TRACEBUFFERSIZE);

    ptr1 = ptr;

    token = strtok(ptr, search);
    // token should hold "TRACEBUFFER"

    token = strtok(NULL, search);
    // token should hold "timestamp offset in usecs"

    _u64  timestamp = 0;
    _u64  timestamp_offset;

    if (specific_timestamp_offset == 0) {
        timestamp_offset = (_u64)atoll(token);
    }
    else {
        timestamp_offset = specific_timestamp_offset;
    }

    int     tid = 0;
    char    *text;
    int     timestamp_sec;
    int     timestamp_nsec;

    while (1) {

        // layout is "\ftimestamp_sec\ftimestamp_nsec\ftid\ftracestring"

        token = strtok(NULL, search);
        if (!token) break;
        // token is timestamp_sec
        timestamp_sec = atoi(token);
        
        token = strtok(NULL, search);
        if (!token) break;
        // token is timestamp_usec
        timestamp_nsec = atoi(token);

        timestamp = (_u64)timestamp_nsec + (_u64)timestamp_sec * (_u64)1000000000;

        token = strtok(NULL, search);
        if (!token) break;
        // token is tid
        tid = atoi(token);

        token = strtok(NULL, search);
        if (!token) break;
        // token is trace text
        text = token;

        addentry(tdifile, text, timestamp - timestamp_offset, tid);
    }

    printf("Adding the final entry (%d%% used)\n", (int)((((text - ptr1) * 100.0) / TRACEBUFFERSIZE) + 1));

    // Add one more entry 0.1 sec behind all the previous ones
    addentry(tdifile, "TDITRACE_EXIT\0", timestamp - timestamp_offset + 100 * 1000000, tid);

    free(ptr);
}


int tditrace_init(void)
{
    struct timeval mytime;
    int i;

    if (tditrace_inited) {
        return 0;
    }

    simplefu_mutex_init(&myMutex);

    simplefu_mutex_lock(&myMutex);

    // TDI_LOG("(*TDI*) TDITRACE_INIT buffer size is %d\n", TRACEBUFFERSIZE);

    // Allocate memory to hold the rough traces

    FILE    *file;
    if ((file = fopen("/tmp/.tditracebuffer", "w+")) == 0) {
        printf("Error opening file /tmp/.tditracebuffer");

        simplefu_mutex_unlock(&myMutex);
        return -1;
    }

    ftruncate(fileno(file), TRACEBUFFERSIZE);

    gtrace_buffer = (char *)mmap(0, TRACEBUFFERSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(file), 0);

    for (i = 0; i < TRACEBUFFERSIZE ; i++) {
        gtrace_buffer[i] = 0;
    }

    printf("tdi: init, tracebuffer (16MB) (/tmp/.tditracebuffer) allocated\n");

    trace_buffer_ptr = gtrace_buffer;
    trace_counter = 0;

    // write one time start text
    i = sprintf(trace_buffer_ptr, (char*)"TRACEBUFFER\f");
    trace_buffer_ptr+= i;

    // obtain absolute timestamp and write to buffer
    i = sprintf(trace_buffer_ptr, (char*)"%lld\f", timestamp_nsec());
    trace_buffer_ptr+= i;

    reported_full = 0;

    report_tid = (getenv("TID") != NULL);

    tditrace_inited = 1;

    simplefu_mutex_unlock(&myMutex);

    return 0;
}


void tditrace_rewind()
{
    struct timeval mytime;
    int i;

    simplefu_mutex_lock(&myMutex);

    for (i = 0; i < TRACEBUFFERSIZE ; i++) {
        gtrace_buffer[i] = 0;
    }

    trace_buffer_ptr = gtrace_buffer;
    trace_counter = 0;

    // write one time start text
    i = sprintf(trace_buffer_ptr, (char*)"TRACEBUFFER\f");
    trace_buffer_ptr+= i;

    // obtain absolute timestamp and write to buffer
    gettimeofday(&mytime, 0);

    _u64 timestamp_offset;
    timestamp_offset = (_u64)((_u64)mytime.tv_usec + (_u64)mytime.tv_sec * (_u64)1000000);

    i = sprintf(trace_buffer_ptr, (char*)"%lld\f", timestamp_offset);
    trace_buffer_ptr+= i;

    reported_full = 0;

    simplefu_mutex_unlock(&myMutex);
}


void tditrace_exit(char* filename, char* trace_buffer)
{
    // Create the TDI file from the rough traces
    FILE    *tdifile;
    char    text[100];

    if ((tdifile = fopen(filename, "w+")) == NULL) {
        printf("Could not create trace file: [%s]\n", filename);
        return;
    }

    printf("Writing trace file \"%s\" .....\n", filename);

    sprintf(text, "TIME %d\n", 1000000000);
    fwrite(text, strlen(text), 1, tdifile);

    sprintf(text, "SPEED %d\n", 1000000000);
    fwrite(text, strlen(text), 1, tdifile);

    sprintf(text, "DNM 0 0 >\n");
    fwrite(text, strlen(text), 1, tdifile);

    converttracetotdi(tdifile, trace_buffer, 0);

    sprintf(text, "END\n");
    fwrite(text, strlen(text), 1, tdifile);

    fclose(tdifile);

    chmod(filename, 0666);

    printf("Done\n");
}


void tditrace(const char* format, ...)
{
    va_list args;

    if (!tditrace_inited) {
        return;
    }

    struct timespec  mytime;
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
                        while (*s) *trace_buffer_ptr++ = *s++;
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

                default :
                    break;
            }
            
        } else {
            *trace_buffer_ptr++ = ch;
        }
    }

    va_end(args);

    simplefu_mutex_unlock(&myMutex);
}
