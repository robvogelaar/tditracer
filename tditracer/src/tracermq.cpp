#include <mqueue.h>


#if 0
extern "C" mqd_t mq_open(const char *name, int oflag, ...)
{
    static int (*__mq_open)(const char *, int, ...) = NULL;

    if (!__mq_open)
        __mq_open = (mqd_t (*)(const char *, int, ...)) dlsym(RTLD_NEXT, "mq_open");

    printf("mq_open\n");
    print_stacktrace(MAXFRAMES);

    mqd_t r = __mq_open(name, oflag);

    return r;
}
#endif


#if 0
extern "C" int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio, const struct timespec *abs_timeout)
{
    static int (*__mq_timedsend)(mqd_t, const char*, size_t, unsigned, const struct timespec*) = NULL;

    if (!__mq_timedsend)
        __mq_timedsend = (int (*)(mqd_t, const char*, size_t, unsigned, const struct timespec*)) dlsym(RTLD_NEXT, "mq_timedsend");

    printf("[%ld]mq_timedsend()   (%s), %s\n", syscall(SYS_gettid), msg_ptr, addrinfo(__builtin_return_address(0)));
    print_stacktrace(MAXFRAMES);

    int r = __mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);

    return r;
}
#endif


#if 0
extern "C" ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int* msg_prio, const struct timespec *abs_timeout)
{
    static ssize_t (*__mq_timedreceive)(mqd_t, char*, size_t, unsigned int*, const struct timespec*) = NULL;

    if (!__mq_timedreceive)
        __mq_timedreceive = (ssize_t (*)(mqd_t, char*, size_t, unsigned int*, const struct timespec*)) dlsym(RTLD_NEXT, "mq_timedreceive");

    printf("[%ld]mq_timedreceive()   (%s), %s\n", syscall(SYS_gettid), msg_ptr, addrinfo(__builtin_return_address(0)));
    print_stacktrace(MAXFRAMES);

    ssize_t r = __mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);

    return r;
}
#endif


#if 0
extern "C" extern "C" int mq_notify(mqd_t mqdes, const struct sigevent* sevp)
{
    static int (*__mq_notify)(mqd_t mqdes, const struct sigevent*) = NULL;

    if (!__mq_notify)
        __mq_notify = (int (*)(mqd_t mqdes, const struct sigevent*)) dlsym(RTLD_NEXT, "mq_notify");

    printf("[%ld]mq_notify()   (), %s\n", syscall(SYS_gettid), addrinfo(__builtin_return_address(0)));
    print_stacktrace(MAXFRAMES);

    int r = __mq_notify(mqdes, sevp);

    return r;
}
#endif


#if 0
extern "C" int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid)
{
    static int (*__timer_create)(clockid_t, struct sigevent*, timer_t*) = NULL;

    if (!__timer_create)
        __timer_create = (int (*)(clockid_t, struct sigevent*, timer_t*)) dlsym(RTLD_NEXT, "timer_create");

    printf("[%ld]timer_create()   (), %s\n", syscall(SYS_gettid), addrinfo(__builtin_return_address(0)));
    print_stacktrace(MAXFRAMES);

    int r = __timer_create(clockid, sevp, timerid);

    return r;
}
#endif
