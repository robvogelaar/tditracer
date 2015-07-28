
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


extern void tditrace(const char* format, ...) __attribute__((weak));
extern void tditrace_ex(const char* format, ...) __attribute__((weak));



int main(int argc, char **argv)
{
    int i;

    struct timeval  mytimeval;
    struct timespec mytimespec;

    #if 0
    for (i = 0; i < 150; i++) {
        gettimeofday(&mytimeval, 0);
        printf("gettimeofday() : %d,%d\n", (int)mytimeval.tv_sec, (int)mytimeval.tv_usec);
        usleep(10000);
    }
    #endif

    #if 0
    for (i = 0; i < 150; i++) {
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        printf("clock_gettime(CLOCK_MONOTONIC) : %d,%d\n", (int)mytimespec.tv_sec, (int)mytimespec.tv_nsec);
        usleep(10000);
    }
    #endif

    printf("start\n");

    if (tditrace) printf("tditrace\n"); else printf("no tditrace\n");
    if (tditrace_ex) printf("tditrace_ex\n"); else printf("no tditrace_ex\n");

    for (i = 0; i < 25; i++) {

        printf("%d\n", i);

        // will all appear in the same "HELLO" , "NOTES"-timeline
        if (tditrace) tditrace("HELLO");
        if (tditrace) tditrace("HELLO %d", i);
        if (tditrace) tditrace("HELLO %d %s %s", i, "yes", "no");

        // create separate "HELLO1", "HELLO2" ,..   "NOTES"-timelines
        if (tditrace) tditrace("HELLO%d", i);

        // variable~value creates a QUEUES-timeline
        if (tditrace) tditrace("i~%d", 12);
        usleep(10000);

        if (tditrace) tditrace("i~%d", 16);
        usleep(10000);

        if (tditrace) tditrace("i~%d", 8);
        usleep(100000);

        // create "TASKS"-timeline, using the @T+ and #T- identifier
        if (tditrace) tditrace("@T+HELLO");
        usleep(50000);
        if (tditrace) tditrace("@T-HELLO");


        if (tditrace) tditrace("@T+TEST");

        if (tditrace) tditrace("@T+T0");

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

        if (tditrace) tditrace("@T-T0");
        
        if (tditrace) tditrace("@T+T1");

        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);

        if (tditrace) tditrace("@T-T1");
        

        if (tditrace) tditrace("@T+T2 01234567890123456789012345678901234567890123456789012345678901234567890123456789");
        if (tditrace) tditrace("@T-T2");

        if (tditrace) tditrace("@T+T3 %s", "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
        if (tditrace) tditrace("@T-T3");

        if (tditrace) tditrace("@T+T5 %d %u %x %p", 0, 0, 0, 0);
        if (tditrace) tditrace("@T-T5");
        
        if (tditrace) tditrace("@T+T5 %d %u %x %p", 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
        if (tditrace) tditrace("@T-T5");

        if (tditrace) tditrace("@T-TEST");

        usleep(50000);

        // create "EVENTS"-timeline, using the @E+ identifier
        if (tditrace) tditrace("@E+HELLO");

        usleep(50000);
    }

    usleep(100000);
    if (tditrace) tditrace("END");

    printf("stop\n");

    return 0;
}
