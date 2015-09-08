
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

extern void tditrace(const char *format, ...) __attribute__((weak));
extern void tditrace_ex(const char *format, ...) __attribute__((weak));

void bar(void) {
    int a;
    unsigned int x;
    asm volatile ("move %0, $ra" : "=r" (x));
    printf("bar, %p, %p, %p\n", __builtin_return_address(0), &a, x);
 }

void foo(void) {
    int a;
    printf("foo, %p, %p\n", __builtin_return_address(0), &a);
    bar();
    bar();

}

void bar2(void) {
    int a;
    printf("bar2, %p, %p\n", __builtin_return_address(0), &a); }

void foo2(void) {
    int a;
    printf("foo2, %p, %p\n", __builtin_return_address(0), &a);
    bar2();
    bar2();
}


int main(int argc, char **argv) {
    int i;

#if 1

    time_t t;
    t = time(NULL);
    printf("Local time and date: %s\n", asctime(localtime(&t)));
    printf("UTC time and date: %s\n", asctime(gmtime(&t)));

    struct timespec mytime;
    mytime.tv_nsec = 0;
    mytime.tv_sec = 0;
    printf("0 -> time and date: %s\n", ctime((const time_t *)&mytime));
    printf("0 -> UTC time and date: %s\n",
           asctime(gmtime((const time_t *)&mytime)));

#endif

    struct timeval mytimeval;
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

    foo();
    foo2();

    if (tditrace)
        printf("tditrace\n");
    else
        printf("no tditrace\n");
    if (tditrace_ex)
        printf("tditrace_ex\n");
    else
        printf("no tditrace_ex\n");

    for (i = 0; i < 5; i++) {

        printf("%d\n", i);

        // will all appear in the same "HELLO" , "NOTES"-timeline
        if (tditrace)
            tditrace("HELLO");
        printf("HELLO\n");

        if (tditrace)
            tditrace("HELLO %d", i);
        printf("HELLO %d\n", i);

        if (tditrace)
            tditrace("HELLO %d %s %s", i, "yes", "no");
        printf("HELLO %d %s %s\n", i, "yes", "no");

        printf("malloc(1 * 1024)\n");

        char *d = malloc(1 * 1024);
        d[0] = 0;

        // create separate "HELLO1", "HELLO2" ,..   "NOTES"-timelines
        if (tditrace)
            tditrace("HELLO%d", i);

        // variable~value creates a QUEUES-timeline
        if (tditrace)
            tditrace("i~%d", 12);
        usleep(10000);

        if (tditrace)
            tditrace("i~%d", 16);
        usleep(10000);

        if (tditrace)
            tditrace("i~%d", 8);
        usleep(100000);

        // create "TASKS"-timeline, using the @T+ and #T- identifier
        if (tditrace)
            tditrace("@T+HELLO");

        usleep(20000);

        if (tditrace)
            tditrace("@I+interrupt begin");

        usleep(10000);

        if (tditrace)
            tditrace("@I-interrupt end");

        usleep(20000);

        if (tditrace)
            tditrace("@T-HELLO");

        if (tditrace)
            tditrace("@T+TEST");

        if (tditrace)
            tditrace("@T+T0");

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

        if (tditrace)
            tditrace("@T-T0");

        if (tditrace)
            tditrace("@T+T1");

        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);

        if (tditrace)
            tditrace("@T-T1");

        if (tditrace)
            tditrace("@T+T2 "
                     "012345678901234567890123456789012345678901234567890123456"
                     "78901234567890123456789");
        if (tditrace)
            tditrace("@T-T2");

        if (tditrace)
            tditrace("@T+T3 %s", "012345678901234567890123456789012345678901234"
                                 "567890123456789012345678901234567890123456789"
                                 "012345678901234567890123456789012345678901234"
                                 "5678901234567890123456789");
        if (tditrace)
            tditrace("@T-T3");

        if (tditrace)
            tditrace("@T+T5 %d %u %x %p", 0, 0, 0, 0);
        if (tditrace)
            tditrace("@T-T5");

        if (tditrace)
            tditrace("@T+T5 %d %u %x %p", 0xffffffff, 0xffffffff, 0xffffffff,
                     0xffffffff);
        if (tditrace)
            tditrace("@T-T5");

        if (tditrace)
            tditrace("@T-TEST");

        usleep(25000);

        // create "EVENTS"-timeline, using the @E+ identifier
        if (tditrace)
            tditrace("@E+HELLO");

        usleep(25000);

        // create "SEMAPHORES"-timeline, using the @S+ identifier
        if (tditrace)
            tditrace("@S+SEMA semaphore test");

        usleep(10000);

        if (tditrace)
            tditrace("@A+Agent agent test");

        usleep(40000);

        if (tditrace)
            tditrace("@A-Agent");

        usleep(50000);
    }

    usleep(100000);
    if (tditrace)
        tditrace("END");

    printf("stop\n");

    return 0;
}
