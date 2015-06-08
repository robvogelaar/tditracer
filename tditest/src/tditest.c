
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/*
 **************************************
 */
#ifdef __cplusplus
extern "C" {
#endif
int  tditrace_init(void);
void tditrace(const char *format, ...);
#ifdef __cplusplus
}
#endif
int tditrace_inited = 0;
#define TDITRACE(...)               \
    do                              \
    {                               \
        if (!tditrace_inited) {     \
            tditrace_init();        \
            tditrace_inited = 1;    \
        }                           \
        tditrace(__VA_ARGS__);      \
    } while (0)                     \
/*
 **************************************
 */


int main(int argc, char **argv)
{
    int i;

    struct timeval  mytimeval;
    struct timespec mytimespec;


    printf("start...\n");

    #if 0
    for (i = 0; i < 150; i++) {
        gettimeofday(&mytimeval, 0); printf("%d,%d\n", (int)mytimeval.tv_sec, (int)mytimeval.tv_usec);
        usleep(10000);
    }
    #endif

    #if 0
    for (i = 0; i < 150; i++) {
        clock_gettime(CLOCK_MONOTONIC, &mytimespec); printf("%d,%d\n", (int)mytimespec.tv_sec, (int)mytimespec.tv_nsec);
        usleep(10000);
    }
    #endif

    for (i = 0; i < 50; i++) {
        // will all appear in the same "HELLO" , "NOTES"-timeline
        TDITRACE("HELLO");
        TDITRACE("HELLO %d", i);
        TDITRACE("HELLO %d %s %s", i, "yes", "no");

        // create separate "HELLO1", "HELLO2" ,..   "NOTES"-timelines
        TDITRACE("HELLO%d", i);

        // variable~value creates a QUEUES-timeline
        TDITRACE("i~%d", 12);
        usleep(10000);

        TDITRACE("i~%d", 16);
        usleep(10000);

        TDITRACE("i~%d", 8);
        usleep(100000);

        // create "TASKS"-timeline, using the @T+ and #T- identifier
        TDITRACE("@T+HELLO");
        usleep(50000);
        TDITRACE("@T-HELLO");


        TDITRACE("@T+TEST");

        TDITRACE("@T+T0");

        gettimeofday(&mytimeval, 0);
        gettimeofday(&mytimeval, 0);
        gettimeofday(&mytimeval, 0);
        gettimeofday(&mytimeval, 0);
        gettimeofday(&mytimeval, 0);
        gettimeofday(&mytimeval, 0);
        gettimeofday(&mytimeval, 0);
        gettimeofday(&mytimeval, 0);
        gettimeofday(&mytimeval, 0);
        gettimeofday(&mytimeval, 0);

        TDITRACE("@T-T0");
        
        TDITRACE("@T+T1");

        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);

        TDITRACE("@T-T1");
        

        TDITRACE("@T+T2 01234567890123456789012345678901234567890123456789012345678901234567890123456789");
        TDITRACE("@T-T2");

        TDITRACE("@T+T3 %s", "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
        TDITRACE("@T-T3");

        TDITRACE("@T+T5 %d %u %x %p", 0, 0, 0, 0);
        TDITRACE("@T-T5");
        
        TDITRACE("@T+T5 %d %u %x %p", 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
        TDITRACE("@T-T5");

        TDITRACE("@T-TEST");

        usleep(50000);

        // create "EVENTS"-timeline, using the @E+ identifier
        TDITRACE("@E+HELLO");

        usleep(50000);
    }

    usleep(100000);
    TDITRACE("END");

    printf("...end\n");

    return 0;
}
