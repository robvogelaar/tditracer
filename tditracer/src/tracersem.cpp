#include <semaphore.h>

#if 0
extern "C" int sem_init(sem_t* sem, int pshared, unsigned int value)
{
    static int (*__sem_init)(sem_t*, int, unsigned int) = NULL;

    if (!__sem_init)
        __sem_init = (int (*)(sem_t*, int, unsigned int)) dlsym(RTLD_NEXT, "sem_init");

    printf("[%ld]sem_init()   (0x%08x), %s\n", syscall(SYS_gettid), (int)sem, addrinfo(__builtin_return_address(0)));
    print_stacktrace(MAXFRAMES);

    int r = __sem_init(sem, pshared, value);

    return r;
}
#endif

#if 0
extern "C" int sem_wait(sem_t* sem)
{
    static int (*__sem_wait)(sem_t*) = NULL;

    if (!__sem_wait)
        __sem_wait = (int (*)(sem_t*)) dlsym(RTLD_NEXT, "sem_wait");

    printf("[%ld]sem_wait()   (0x%08x), %s\n", syscall(SYS_gettid), (int)sem, addrinfo(__builtin_return_address(0)));
    print_stacktrace(MAXFRAMES);

    int r = __sem_wait(sem);

    return r;
}
#endif

#if 0
extern "C" int sem_post(sem_t* sem)
{
    static int (*__sem_post)(sem_t*) = NULL;

    if (!__sem_post)
        __sem_post = (int (*)(sem_t*)) dlsym(RTLD_NEXT, "sem_post");

    printf("[%ld]sem_post()   (0x%08x), %s\n", syscall(SYS_gettid), (int)sem, addrinfo(__builtin_return_address(0)));
    print_stacktrace(MAXFRAMES);

    int r = __sem_post(sem);

    return r;
}
#endif
