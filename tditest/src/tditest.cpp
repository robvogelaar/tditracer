
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

extern "C" void tditrace(const char *format, ...) __attribute__((weak));
extern "C" void tditrace_ex(const char *format, ...) __attribute__((weak));

void run_1(void) {
#if 0
    for (i = 0; i < 150; i++) {
        gettimeofday(&mytimeval, 0);
        printf("gettimeofday() : %d,%d\n", (int)mytimeval.tv_sec, (int)mytimeval.tv_usec);
        usleep(10000);
    }
#endif
}

#if 0
    for (i = 0; i < 150; i++) {
        clock_gettime(CLOCK_MONOTONIC, &mytimespec);
        printf("clock_gettime(CLOCK_MONOTONIC) : %d,%d\n", (int)mytimespec.tv_sec, (int)mytimespec.tv_nsec);
        usleep(10000);
    }
#endif

void run_2(void) {

int i;
struct timeval mytimeval;
struct timespec mytimespec;

printf("start\n");

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

        char *d = (char*)malloc(1 * 1024);
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

}

#if 0
void foo(char* p, int i) {

    int b;

    unsigned int ra = 0;
    unsigned int sp = 0;

    #ifdef __mips__
    asm volatile("move %0, $ra" : "=r"(ra));
    asm volatile("move %0, $sp" : "=r"(sp));
    #endif

    printf("foo, return address : %p, ra : %p, sp : %p\n", __builtin_return_address(0), ra, sp);

    printf("%d\n", i);


    if (i < 3) {
        foo(p, ++i);
    } else {
        b = (int)realloc(p, i * 5 * 1024);
        printf("b=0x%08x (0x%08x)\n", b, &b);
    }

}
#endif

#if 0
int main(int argc, char **argv) {

    sleep(3);

    char *p = malloc(1 * 1024);

    foo(p, 0);

    sleep(3);

    printf("stop\n");
    return 0;
}
#endif

#if 0
int main(int argc, char **argv) {

    usleep(2*1000*1000);

    if (tditrace)
        tditrace("START");


    int i;
    for (i = 0; i < 10; i++) {

        int *p1 = new int(5);
        int *p2 = new int [1312 / 4];
        int *p3 = (int*)malloc(5 * sizeof(int));
    
        printf("p1=%d(%x)\n", *p1, p1);
        printf("p2=%d(%x)\n", *p2, p2);
        printf("p3=%d(%x)\n", *p3, p3);
    }


    if (tditrace)
        tditrace("STOP");

    return 0;
}
#endif

int main(int argc, char **argv) {

    run_2();
}
