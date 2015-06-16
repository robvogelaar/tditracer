#include <pthread.h>
#include <sched.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdarg.h>

#include "tracermain.h"
#include "tdi.h"

#if 1
extern "C" int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                              void *(*start)(void *), void *arg) {
    static int (*__pthread_create)(pthread_t *, const pthread_attr_t *,
                                   void *(*start)(void *), void *) = NULL;

    if (!__pthread_create)
        __pthread_create = (int (*)(pthread_t *, const pthread_attr_t *,
                                    void *(*start)(void *),
                                    void *))dlsym(RTLD_NEXT, "pthread_create");

    if (pthreadrecording) {
        tditrace_ex("pthread_create() 0x%x %p", *thread, start);
    }
    // TDIPRINTF("[%ld]+pthread_create(), called from: %s, thread: %s\n",
    // syscall(SYS_gettid), addrinfo(__builtin_return_address(0)),
    // addrinfo((void*)start));

    int r = __pthread_create(thread, attr, start, arg);

    // TDIPRINTF("[%ld]-pthread_create():0x%x\n", syscall(SYS_gettid),
    // pthreadid(*thread));

    return r;
}
#endif

#if 0
extern "C" int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
    static int (*__pthread_setschedparam)(pthread_t, int policy, const struct sched_param*) = NULL;

    if (!__pthread_setschedparam)
        __pthread_setschedparam = (int (*)(pthread_t, int policy, const struct sched_param*)) dlsym(RTLD_NEXT, "pthread_setschedparam");

    TDIPRINTF("[%ld]pthread_setschedparam([0x%x, %s, %d])\n",
                    syscall(SYS_gettid),
                    pthreadid(thread),
                    (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                    (policy == SCHED_RR)    ? "SCHED_RR" :
                    (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???",
                    param->sched_priority);

    return __pthread_setschedparam(thread, policy, param);
}
#endif

#if 1
extern "C" int pthread_mutex_trylock(pthread_mutex_t *mutex) {
    static int (*__pthread_mutex_trylock)(pthread_mutex_t *) = NULL;

    if (!__pthread_mutex_trylock)
        __pthread_mutex_trylock = (int (*)(pthread_mutex_t *))dlsym(
            RTLD_NEXT, "pthread_mutex_trylock");

    if (pthreadrecording) {
        tditrace_ex("pthread_mutex_trylock() 0x%x", mutex);
    }

    return __pthread_mutex_trylock(mutex);
}
#endif

#if 1
extern "C" int pthread_mutex_lock(pthread_mutex_t *mutex) {
    static int (*__pthread_mutex_lock)(pthread_mutex_t *) = NULL;

    if (!__pthread_mutex_lock)
        __pthread_mutex_lock =
            (int (*)(pthread_mutex_t *))dlsym(RTLD_NEXT, "pthread_mutex_lock");

    if (pthreadrecording) {
        tditrace_ex("@T+pthread_mutex_lock() 0x%x", mutex);
    }

    // TDIPRINTF("[%ld]+pthread_mutex_lock(0x%x), %s\n", syscall(SYS_gettid),
    // (int)mutex, addrinfo(__builtin_return_address(0)));
    // print_stacktrace(MAXFRAMES);

    int r = __pthread_mutex_lock(mutex);

    // TDIPRINTF("[%ld]-pthread_mutex_lock()\n", syscall(SYS_gettid));

    if (pthreadrecording) {
        tditrace_ex("@T-pthread_mutex_lock() 0x%x", mutex);
    }

    return r;
}
#endif

#if 1
extern "C" int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    static int (*__pthread_mutex_unlock)(pthread_mutex_t *) = NULL;

    if (!__pthread_mutex_unlock)
        __pthread_mutex_unlock = (int (*)(pthread_mutex_t *))dlsym(
            RTLD_NEXT, "pthread_mutex_unlock");

    if (pthreadrecording) {
        tditrace_ex("@T+pthread_mutex_unlock() 0x%x", mutex);
    }
    // TDIPRINTF("[%ld]+pthread_mutex_unlock(0x%x), %s\n", syscall(SYS_gettid),
    // (int)mutex, addrinfo(__builtin_return_address(0)));

    int r = __pthread_mutex_unlock(mutex);

    // TDIPRINTF("[%ld]-pthread_mutex_unlock()\n", syscall(SYS_gettid));

    if (pthreadrecording) {
        tditrace_ex("@T-pthread_mutex_unlock() 0x%x", mutex);
    }

    return r;
}
#endif

#if 0
extern "C" int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex)
{
    static int (*__pthread_cond_wait)(pthread_cond_t*, pthread_mutex_t*) = NULL;

    if (!__pthread_cond_wait)
        __pthread_cond_wait = (int (*)(pthread_cond_t*, pthread_mutex_t*)) dlsym(RTLD_NEXT, "pthread_cond_wait");

    // tditrace_ex("@T+pthread_cond_wait()");

    //TDIPRINTF("[%ld]+pthread_cond_wait   (0x%x, 0x%x), %s\n", syscall(SYS_gettid), (int)cond, (int)mutex, addrinfo(__builtin_return_address(0)));
    //print_stacktrace(MAXFRAMES);

    int r = __pthread_cond_wait(cond, mutex);

    // TDIPRINTF("[%ld]-pthread_cond_wait()\n", syscall(SYS_gettid));

    if (pthread) tditrace_ex("@T+cond 0x%x,0x%x,%s", cond, mutex, addrinfo(__builtin_return_address(0)));

    return r;
}
#endif

#if 0
extern "C" int pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime)
{
    static int (*__pthread_cond_timedwait)(pthread_cond_t*, pthread_mutex_t*, const struct timespec*) = NULL;

    if (!__pthread_cond_timedwait)
        __pthread_cond_timedwait = (int (*)(pthread_cond_t*, pthread_mutex_t*, const struct timespec*)) dlsym(RTLD_NEXT, "pthread_cond_timedwait");

    //tditrace_ex("@T+pthread_cond_timedwait()");

    //TDIPRINTF("[%ld]+pthread_cond_timedwait   (0x%x, 0x%x), %s\n", syscall(SYS_gettid), (int)cond, (int)mutex, addrinfo(__builtin_return_address(0)));

    int r = __pthread_cond_timedwait(cond, mutex, abstime);

    //TDIPRINTF("[%ld]-pthread_cond_timedwait()\n", syscall(SYS_gettid));

    //tditrace_ex("@T-pthread_cond_timedwait()");

    return r;
}
#endif

#if 0
extern "C" int pthread_cond_signal(pthread_cond_t* cond)
{
    static int (*__pthread_cond_signal)(pthread_cond_t*) = NULL;

    if (!__pthread_cond_signal)
        __pthread_cond_signal = (int (*)(pthread_cond_t*)) dlsym(RTLD_NEXT, "pthread_cond_signal");

    //tditrace_ex("@T+pthread_cond_signal()");

    //TDIPRINTF("[%ld]+pthread_cond_signal (0x%x), %s\n", syscall(SYS_gettid), (int)cond, addrinfo(__builtin_return_address(0)));

    //print_stacktrace(MAXFRAMES);

    int r = __pthread_cond_signal(cond);

    //TDIPRINTF("[%ld]-pthread_cond_signal()\n", syscall(SYS_gettid));

    //tditrace_ex("@T-pthread_cond_signal()");

    if (pthread) tditrace_ex("@T-cond 0x%x,%s", cond, addrinfo(__builtin_return_address(0)));

    return r;
}
#endif
