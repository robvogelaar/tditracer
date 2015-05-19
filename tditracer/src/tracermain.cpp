
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <syscall.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#include <errno.h>

#include "gldefs.h"

#include <dlfcn.h>

extern "C" {
#include "texturecapture.h"
}

#if 0
#include <execinfo.h>
#include <cxxabi.h>
#endif

/*
 * TDITRACE
 */
extern "C" int  tditrace_init(void);
extern "C" void tditrace(const char* format, ...);
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


#include <stdarg.h>

void tdiprintf( const char *format, ...)
{
    va_list args;
    fprintf(stdout, "TDI:");
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "\n");
}


#define TDIPRINTF(...)              \
    do                              \
    {                               \
        if (0) {                    \
            tdiprintf(__VA_ARGS__); \
        }                           \
    } while (0)                     \


static void init(void);

static void __attribute__ ((constructor)) tditracer_constructor();
static void __attribute__ ((destructor)) tditracer_destructor();

static void tditracer_constructor()
{
    init();
}

static void tditracer_destructor()
{
}


/*
 * FRAMECAPTURE
 */
extern "C" void framecapture_capframe(void);
extern "C" void framecapture_writebmpframes(int frames);
extern "C" void framecapture_writepngframes(void);
int framestorecord = 0;

/*
 * TEXTURECAPTURE
 */
bool texturerecording = false;
bool renderbufferrecording = false;

/*
 * SHADERCAPTURE
 */
extern "C" void shadercapture_capshader(unsigned int name, int count, const char** string, const int* length);
extern "C" void shadercapture_writeshaders(void);
extern "C" void shadercapture_referenceprogram(unsigned int shader, unsigned int program);

bool shaderrecording = false;

bool pthread = false;


int  dumpafter = 0;
bool diddump = false;

static GLuint boundtexture;
static GLuint currentprogram;
static GLuint boundframebuffer = 0;
static GLuint boundrenderbuffer = 0;
static GLuint framebuffertexture[1024] = {};
static GLuint framebufferrenderbuffer[1024] = {};
static GLuint renderbufferwidth[1024] = {};
static GLuint renderbufferheight[1024] = {};


static void sighandler(int signum);


static int shaders_captured = 0;
static int textures_captured = 0;
static int texturebytes_captured = 0;
static int frames_captured = 0;


extern int trace_counter;

static void dump(void)
{
    if (texturerecording || (framestorecord > 0) || shaderrecording) {
        printf("dumping, #shaders captured = %d, #textures captured = %d, #frames captured = %d\n", shaders_captured, textures_captured, frames_captured);

        if (shaderrecording) {
            shadercapture_writeshaders();
        }

        if (texturerecording) {
            texturecapture_writepngtextures();
            texturecapture_deletetextures();
        }

        if (framestorecord > 0) {
            framecapture_writepngframes();
        }
    }

    printf("tditracer:#traces = %d\n", trace_counter);
}


static void init(void)
{
    printf("%s\n", __func__);

    static bool inited = false;
    if (!inited) {

        signal(SIGINT, sighandler);

        if (getenv("PT")) {
            pthread = (atoi(getenv("PT")) >= 1);
        }

        if (getenv("TR")) {
            texturerecording = (atoi(getenv("TR")) >= 1);
        }
        if (getenv("RR")) {
            renderbufferrecording = (atoi(getenv("RR")) >= 1);
        }
        if (getenv("FR")) {
            framestorecord = atoi(getenv("FR"));
        }
        if (getenv("SR")) {
            shaderrecording = (atoi(getenv("SR")) >= 1);
        }
        if (getenv("DA")) {
            dumpafter = atoi(getenv("DA"));
        }

        if (renderbufferrecording) {
            texturerecording = true;
        }

        printf("tditrace:init, pthread:%s, shaders:%s, textures:%s, renderbuffers:%s, frames:%d, dumpafter:%d\n",
            pthread? "yes":"no",
            shaderrecording? "yes":"no",
            texturerecording? "yes":"no",
            renderbufferrecording? "yes":"no",
            framestorecord,
            dumpafter);

        inited = true;
    }
}


typedef unsigned int mz_uint;
typedef int mz_bool;


extern "C" void *tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out);
extern "C" void *tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, mz_uint level, mz_bool flip);

extern "C" void glReadPixels(GLint x,  GLint y,  GLsizei width,  GLsizei height,  GLenum format,  GLenum type,  GLvoid * data);

static void sighandler(int signum)
{
    printf("tditracer received SIGNAL : %d\n\n\n", signum);

    /*
     * do not dump again if another ctrl-c is received, instead
     * go direct to abort, allowing a current dumping to be aborted
     */
    if (!diddump) {
        diddump = true;

        if (framestorecord > 0) {

            framecapture_capframe();
            frames_captured++;

            framecapture_capframe();
            frames_captured++;

            framecapture_capframe();
            frames_captured++;
        }

        dump();
    }


    if (texturerecording) {

        unsigned char* p = (unsigned char*)malloc(1280 * 720 * 4);
        glReadPixels(0, 0, 1280, 720, GL_RGBA, GL_UNSIGNED_BYTE, p);
        void *pPNG_data;
        size_t png_data_size = 0;
        pPNG_data = tdefl_write_image_to_png_file_in_memory_ex(p, 1280, 720, 4, &png_data_size, 6, 1);
        FILE *pFile = fopen("frame.png", "wb");
        fwrite(pPNG_data, 1, png_data_size, pFile);
        chmod("frame.png", 0666);
        fclose(pFile);
    }

    abort();
}


#if 0
const char* demangle(const char* s)
{
    if (s) {
        if (strstr(s, "_ZN")) {
            if (dlsym(RTLD_NEXT, "__cxa_demangle")) {
                size_t length = 0;
                int status = 0;
                return abi::__cxa_demangle(s, NULL, &length, &status);
            }
        }
    }

    return s;
}

char* addrinfo(void* addr)
{
    static char text[256];
    Dl_info dli;

    dladdr(addr, &dli);
    sprintf(text, "[0x%08x] %s (%s)", (int)addr, demangle(dli.dli_sname), dli.dli_fname);
    return text;
}


//extern "C" pthread_t pthread_self(void);

int pthreadid(pthread_t ptid)
{
    int threadId = 0;
    memcpy(&threadId, &ptid, 4);
    return threadId;
}

#define MAXFRAMES 32

static inline void print_stacktrace(int max_frames = MAXFRAMES)
{
    // storage array for stack trace address data
    void* addrlist[max_frames + 1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    if (addrlen == 0) {
        printf("    <empty, possibly corrupt>\n");
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.

    // also skip 1 as it is in libtditracer.so
    for (int i = 2; i < addrlen; i++)
    {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist[i]; *p; ++p)
        {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset && begin_name < begin_offset)
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status;
            char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
            if (status == 0) {
                funcname = ret; // use possibly realloc()-ed string
                printf("    [%ld][%2d][0x%08x]%s : %s+%s\n", syscall(SYS_gettid), i, (int)addrlist[i], symbollist[i], funcname, begin_offset);
            }
            else {
                // demangling failed. Output function name as a C function with no arguments.
                printf("    [%ld][%2d][0x%08x]%s : %s()+%s\n", syscall(SYS_gettid), i, (int)addrlist[i], symbollist[i], begin_name, begin_offset);
            }
        }
        else
        {
            // couldn't parse the line? print the whole line.
            printf("    [%ld][%2d][0x%08x],%s\n", syscall(SYS_gettid), i, (int)addrlist[i], symbollist[i]);
        }
    }

    free(funcname);
    free(symbollist);
}
#endif

#if 0

extern "C" int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start)(void *), void* arg) throw()
{
    static int (*__pthread_create)(pthread_t*, const pthread_attr_t*, void* (*start)(void *), void*) = NULL;

    if (!__pthread_create)
        __pthread_create = (int (*)(pthread_t*, const pthread_attr_t*, void* (*start)(void *), void*)) dlsym(RTLD_NEXT, "pthread_create");

    if (pthread) TDITRACE("pthread_create() 0x%08x %p,%s", *thread, start, addrinfo(__builtin_return_address(0)));

    TDIPRINTF("[%ld]+pthread_create(), called from: %s, thread: %s\n", syscall(SYS_gettid), addrinfo(__builtin_return_address(0)), addrinfo((void*)start));

    int r =  __pthread_create(thread, attr, start, arg);

    TDIPRINTF("[%ld]-pthread_create():0x%08x\n", syscall(SYS_gettid), pthreadid(*thread));

    return r;
}


#include <sched.h>

extern "C" int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
    static int (*__pthread_setschedparam)(pthread_t, int policy, const struct sched_param*) = NULL;

    if (!__pthread_setschedparam)
        __pthread_setschedparam = (int (*)(pthread_t, int policy, const struct sched_param*)) dlsym(RTLD_NEXT, "pthread_setschedparam");

    TDIPRINTF("[%ld]pthread_setschedparam([0x%08x, %s, %d])\n",
                    syscall(SYS_gettid),
                    pthreadid(thread),
                    (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                    (policy == SCHED_RR)    ? "SCHED_RR" :
                    (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???",
                    param->sched_priority);

    return __pthread_setschedparam(thread, policy, param);
}

#if 0
extern "C" int pthread_mutex_trylock(pthread_mutex_t* mutex)
{
    static int (*__pthread_mutex_trylock)(pthread_mutex_t*) = NULL;

    if (!__pthread_mutex_trylock)
        __pthread_mutex_trylock = (int (*)(pthread_mutex_t*)) dlsym(RTLD_NEXT, "pthread_mutex_trylock");

    TDITRACE("pthread_mutex_trylock() 0x%08x", mutex);

    return __pthread_mutex_trylock(mutex);
}
#endif

extern "C" int pthread_mutex_lock(pthread_mutex_t* mutex)
{
    static int (*__pthread_mutex_lock)(pthread_mutex_t*) = NULL;

    if (!__pthread_mutex_lock)
        __pthread_mutex_lock = (int (*)(pthread_mutex_t*)) dlsym(RTLD_NEXT, "pthread_mutex_lock");

    // TDITRACE("@T+pthread_mutex_lock() 0x%08x", mutex);

    //TDIPRINTF("[%ld]+pthread_mutex_lock(0x%08x), %s\n", syscall(SYS_gettid), (int)mutex, addrinfo(__builtin_return_address(0)));
    //print_stacktrace(MAXFRAMES);

    int r = __pthread_mutex_lock(mutex);

    //TDIPRINTF("[%ld]-pthread_mutex_lock()\n", syscall(SYS_gettid));

    // TDITRACE("@T-pthread_mutex_lock() 0x%08x", mutex);

    if (pthread) TDITRACE("@T+mutex 0x%08x,%s", mutex, addrinfo(__builtin_return_address(0)));

    return r;
}


extern "C" int pthread_mutex_unlock(pthread_mutex_t* mutex)
{
    static int (*__pthread_mutex_unlock)(pthread_mutex_t*) = NULL;

    if (!__pthread_mutex_unlock)
        __pthread_mutex_unlock = (int (*)(pthread_mutex_t*)) dlsym(RTLD_NEXT, "pthread_mutex_unlock");

    // TDITRACE("@T+pthread_mutex_unlock() 0x%08x", mutex);

    //TDIPRINTF("[%ld]+pthread_mutex_unlock(0x%08x), %s\n", syscall(SYS_gettid), (int)mutex, addrinfo(__builtin_return_address(0)));

    int r = __pthread_mutex_unlock(mutex);

    //TDIPRINTF("[%ld]-pthread_mutex_unlock()\n", syscall(SYS_gettid));

    // TDITRACE("@T-pthread_mutex_unlock() 0x%08x", mutex);

    if (pthread) TDITRACE("@T-mutex 0x%08x,%s", mutex, addrinfo(__builtin_return_address(0)));

    return r;
}


extern "C" int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex)
{
    static int (*__pthread_cond_wait)(pthread_cond_t*, pthread_mutex_t*) = NULL;

    if (!__pthread_cond_wait)
        __pthread_cond_wait = (int (*)(pthread_cond_t*, pthread_mutex_t*)) dlsym(RTLD_NEXT, "pthread_cond_wait");

    // TDITRACE("@T+pthread_cond_wait()");

    //TDIPRINTF("[%ld]+pthread_cond_wait   (0x%08x, 0x%08x), %s\n", syscall(SYS_gettid), (int)cond, (int)mutex, addrinfo(__builtin_return_address(0)));
    //print_stacktrace(MAXFRAMES);

    int r = __pthread_cond_wait(cond, mutex);

    // TDIPRINTF("[%ld]-pthread_cond_wait()\n", syscall(SYS_gettid));

    if (pthread) TDITRACE("@T+cond 0x%08x,0x%08x,%s", cond, mutex, addrinfo(__builtin_return_address(0)));

    return r;
}

#if 0
extern "C" int pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime)
{
    static int (*__pthread_cond_timedwait)(pthread_cond_t*, pthread_mutex_t*, const struct timespec*) = NULL;

    if (!__pthread_cond_timedwait)
        __pthread_cond_timedwait = (int (*)(pthread_cond_t*, pthread_mutex_t*, const struct timespec*)) dlsym(RTLD_NEXT, "pthread_cond_timedwait");

    //TDITRACE("@T+pthread_cond_timedwait()");

    //TDIPRINTF("[%ld]+pthread_cond_timedwait   (0x%08x, 0x%08x), %s\n", syscall(SYS_gettid), (int)cond, (int)mutex, addrinfo(__builtin_return_address(0)));

    int r = __pthread_cond_timedwait(cond, mutex, abstime);

    //TDIPRINTF("[%ld]-pthread_cond_timedwait()\n", syscall(SYS_gettid));

    //TDITRACE("@T-pthread_cond_timedwait()");

    return r;
}
#endif

extern "C" int pthread_cond_signal(pthread_cond_t* cond)
{
    static int (*__pthread_cond_signal)(pthread_cond_t*) = NULL;

    if (!__pthread_cond_signal)
        __pthread_cond_signal = (int (*)(pthread_cond_t*)) dlsym(RTLD_NEXT, "pthread_cond_signal");

    //TDITRACE("@T+pthread_cond_signal()");

    //TDIPRINTF("[%ld]+pthread_cond_signal (0x%08x), %s\n", syscall(SYS_gettid), (int)cond, addrinfo(__builtin_return_address(0)));

    //print_stacktrace(MAXFRAMES);

    int r = __pthread_cond_signal(cond);

    //TDIPRINTF("[%ld]-pthread_cond_signal()\n", syscall(SYS_gettid));

    //TDITRACE("@T-pthread_cond_signal()");

    if (pthread) TDITRACE("@T-cond 0x%08x,%s", cond, addrinfo(__builtin_return_address(0)));

    return r;
}

#endif


#if 0

#include <semaphore.h>

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



/*
 * #include <syslog.h>
 * syslog(1,"@T+FBIOPAN_DISPLAY");
 * syslog(0,"@T-FBIOPAN_DISPLAY");
 */

extern "C" void syslog (int f, const char *format, ...)
{
    char buf[256];

    static void (*__syslog)(int, const char *, ...) = NULL;

    if (!__syslog)
        __syslog = (void (*)(int, const char *, ...)) dlsym(RTLD_NEXT, "syslog");

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    TDITRACE("%s", buf);

    #if 0
    if (f) {
        print_stacktrace();
    }
    #endif
}


#if 0
/*
 * Several applications, such as Quake3, use dlopen("libGL.so.1"), but
 * LD_PRELOAD does not intercept symbols obtained via dlopen/dlsym, therefore
 * we need to intercept the dlopen() call here, and redirect to our wrapper
 * shared object.
 */

/*
 * Invoke the true dlopen() function.
 */
static inline void *_dlopen(const char* filename, int flag)
{
    typedef void * (*PFN_DLOPEN)(const char *, int);
    static PFN_DLOPEN dlopen_ptr = NULL;

    if (!dlopen_ptr) {
        dlopen_ptr = (PFN_DLOPEN)dlsym(RTLD_NEXT, "dlopen");
        if (!dlopen_ptr) {
            printf("tditrace: error: failed to look up real dlopen\n");
            return NULL;
        }
    }

    return dlopen_ptr(filename, flag);
}


extern "C" void* dlopen(const char* filename, int flag)
{
    /*
     * dlopen will always trigger (first)
     */
    init();

    // TDITRACE("dlopen() %s", filename);

    bool intercept = false;
    if (filename) {
        intercept = false;
        //intercept =
        //strcmp(filename, "libEGL.so") == 0 ||
        //strcmp(filename, "libGLESv2.so") == 0;

        if (intercept) {
            printf("tditrace: redirecting dlopen(\"%s\", 0x%x)\n", filename, flag);

            /* The current dispatch implementation relies on core entry-points to be globally available, so force this.
             *
             * TODO: A better approach would be note down the entry points here and
             * use them latter. Another alternative would be to reopen the library
             * with RTLD_NOLOAD | RTLD_GLOBAL.
             */
            flag &= ~RTLD_LOCAL;
            flag |= RTLD_GLOBAL;
        }
    }

    void* handle = _dlopen(filename, flag);

    if (intercept) {
        // Get the file path for our shared object, and use it instead
        static int dummy = 0xdeedbeef;
        Dl_info info;

        if (dladdr(&dummy, &info)) {
            handle = _dlopen(info.dli_fname, flag);
            //handle = _dlopen("/appfs/3ddrivers/usr/lib/libGLESv2.so", flag);
        } else {
            printf("tditrace: warning: dladdr() failed\n");
        }
    }

    return handle;
}
#endif

#if 0
extern "C" void* _dl_sym(void*, const char*, void*);
extern "C" void* dlsym(void* handle, const char* name)
{
    printf("[%s]\n",name);
    static void* (*real_dlsym)(void*, const char*)=NULL;
    if (real_dlsym == NULL)
        real_dlsym = _dl_sym(RTLD_NEXT, "dlsym", dlsym);
    return real_dlsym(handle, name);
}
#endif


int draw_counter = 0;
int texturebind_counter = 0;

int current_frame = 1;

int glDrawArrays_counter = 0;
int glDrawElements_counter = 0;
int glTexImage2D_counter = 0;
int glTexSubImage2D_counter = 0;
int glBindTexture_counter = 0;

int glShaderSource_counter = 0;


bool disableblend = false;


int RenderBlock__paint_counter = 0;


// extern "C"  void *malloc(size_t size)
// {
//     static void* (*real_malloc)(size_t)=NULL;
//     if (!real_malloc) {
//         real_malloc = (void* (*)(size_t)) dlsym(RTLD_NEXT, "malloc");
//     }
//
//     void *p = NULL;
//
//     TDITRACE("malloc");
//
//     p = real_malloc(size);
//     return p;
// }


extern "C" EGLBoolean eglSwapBuffers(EGLDisplay display, EGLSurface surface)
{
    static EGLBoolean (*__eglSwapBuffers)(EGLDisplay display, EGLSurface surface)=NULL;

    if (__eglSwapBuffers==NULL) {
        __eglSwapBuffers = (EGLBoolean (*)(EGLDisplay, EGLSurface))dlsym(RTLD_NEXT, "eglSwapBuffers");
        if (NULL == __eglSwapBuffers) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (framestorecord > 0) {

        framecapture_capframe();
        frames_captured++;
    }

    if (dumpafter !=0) {

        if ((framestorecord > 0) || texturerecording || shaderrecording) {

            if ((current_frame) >= dumpafter) {

                /*
                 * let eglSwapBuffers finish
                 */

                __eglSwapBuffers(display, surface);

                usleep(500 * 1000);

                if (framestorecord > 0) {

                    framecapture_capframe();
                    frames_captured++;

                    framecapture_capframe();
                    frames_captured++;

                    framecapture_capframe();
                    frames_captured++;
                }

                dump();

                abort();

            }

        }
    }

    TDITRACE("@T+eglSwapBuffers()");

    EGLBoolean ret = __eglSwapBuffers(display, surface);

    TDITRACE("@T-eglSwapBuffers()");

    TDITRACE("@E+eglSwapBuffers() #gldraws=%d #gltexturebinds=%d", draw_counter, texturebind_counter);

    TDITRACE("#gldraws~%d", 0);
    TDITRACE("#gltexturebinds~%d", 0);

    glDrawElements_counter = 0;
    glDrawArrays_counter = 0;
    glTexImage2D_counter = 0;
    glTexSubImage2D_counter = 0;
    glBindTexture_counter = 0;
    RenderBlock__paint_counter = 0;

    draw_counter = 0;
    texturebind_counter = 0;

    current_frame++;

    return ret;
}



extern "C" EGLBoolean eglMakeCurrent(EGLDisplay display, EGLSurface draw, EGLSurface read, EGLContext context)
{
    static EGLBoolean (*__eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext)=NULL;

    if (__eglMakeCurrent==NULL) {
        __eglMakeCurrent = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLSurface, EGLContext))dlsym(RTLD_NEXT, "eglMakeCurrent");
        if (NULL == __eglMakeCurrent) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("@T+eglMakeCurrent() %d,%d,%d,%d", display, draw, read, context);
    EGLBoolean b = __eglMakeCurrent(display, draw, read, context);
    TDITRACE("@T-eglMakeCurrent()");

    boundframebuffer = 0;

    return b;
}

extern "C" GLvoid glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    static void (*__glDrawArrays)(GLenum, GLint, GLsizei)=NULL;

    if (__glDrawArrays==NULL) {
        __glDrawArrays = (void(*)(GLenum, GLint, GLsizei))dlsym(RTLD_NEXT, "glDrawArrays");
        if (NULL == __glDrawArrays) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    draw_counter++;
    glDrawArrays_counter++;

    TDITRACE("#gldraws~%d", draw_counter);
    if (boundframebuffer != 0) {
        TDITRACE("@T+glDrawArrays() #%d,%d,%s,#i=%d,t=%u,p=%u,f=%u,ft=%u,r=%u,%ux%u", current_frame, glDrawArrays_counter, MODESTRING(mode), count, boundtexture, currentprogram,
             boundframebuffer, framebuffertexture[boundframebuffer], framebufferrenderbuffer[boundframebuffer],
             renderbufferwidth[framebufferrenderbuffer[boundframebuffer]],
             renderbufferheight[framebufferrenderbuffer[boundframebuffer]]);

    } else {
        TDITRACE("@T+glDrawArrays() #%d,%d,%s,#i=%d,t=%u,p=%u", current_frame, glDrawArrays_counter, MODESTRING(mode), count, boundtexture, currentprogram);
    }
    __glDrawArrays(mode, first, count);

    if ((boundframebuffer != 0) && renderbufferrecording) {

        if (framebufferrenderbuffer[boundframebuffer]) {

            if (renderbufferwidth[framebufferrenderbuffer[boundframebuffer]] &&
                renderbufferheight[framebufferrenderbuffer[boundframebuffer]]) {

                unsigned char* p = (unsigned char*)malloc(1280 * 720 * 4);
                glReadPixels(0, 0, renderbufferwidth[framebufferrenderbuffer[boundframebuffer]],
                               renderbufferheight[framebufferrenderbuffer[boundframebuffer]],
                            GL_RGBA, GL_UNSIGNED_BYTE, p);
                texturecapture_captexture(framebuffertexture[boundframebuffer], RENDER, current_frame, 0, 0,
                         renderbufferwidth[framebufferrenderbuffer[boundframebuffer]],
                         renderbufferheight[framebufferrenderbuffer[boundframebuffer]], (int)GL_RGBA, (int)GL_UNSIGNED_BYTE, p);
                free(p);
                textures_captured++;
            }
        }
    }


    TDITRACE("@T-glDrawArrays()");
}


extern "C" GLvoid glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
    static void (*__glDrawElements)(GLenum, GLsizei, GLenum, const GLvoid *)=NULL;

    if (__glDrawElements==NULL) {
        __glDrawElements = (void (*)(GLenum, GLsizei, GLenum, const GLvoid *))dlsym(RTLD_NEXT, "glDrawElements");
        if (NULL == __glDrawElements) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    draw_counter++;
    glDrawElements_counter++;

    TDITRACE("#gldraws~%d", draw_counter);
    if (boundframebuffer) {
        TDITRACE("@T+glDrawElements() #%d,%d,%s,#i=%d,t=%u,p=%u,f=%u,ft=%u,r=%u,%ux%u", current_frame, glDrawElements_counter, MODESTRING(mode), count, boundtexture, currentprogram,
             boundframebuffer, framebuffertexture[boundframebuffer], framebufferrenderbuffer[boundframebuffer],
             renderbufferwidth[framebufferrenderbuffer[boundframebuffer]],
             renderbufferheight[framebufferrenderbuffer[boundframebuffer]]);
    
    } else {
        TDITRACE("@T+glDrawElements() #%d,%d,%s,#i=%d,t=%u,p=%u", current_frame, glDrawElements_counter, MODESTRING(mode), count, boundtexture, currentprogram);
    }

    __glDrawElements(mode, count, type, indices);

    if ((boundframebuffer != 0) && renderbufferrecording) {

        if (framebufferrenderbuffer[boundframebuffer]) {

            if (renderbufferwidth[framebufferrenderbuffer[boundframebuffer]] &&
                renderbufferheight[framebufferrenderbuffer[boundframebuffer]]) {

                unsigned char* p = (unsigned char*)malloc(1280 * 720 * 4);
                glReadPixels(0, 0, renderbufferwidth[framebufferrenderbuffer[boundframebuffer]],
                               renderbufferheight[framebufferrenderbuffer[boundframebuffer]],
                            GL_RGBA, GL_UNSIGNED_BYTE, p);
                texturecapture_captexture(framebuffertexture[boundframebuffer], RENDER, current_frame, 0, 0,
                         renderbufferwidth[framebufferrenderbuffer[boundframebuffer]],
                         renderbufferheight[framebufferrenderbuffer[boundframebuffer]], (int)GL_RGBA, (int)GL_UNSIGNED_BYTE, p);
                free(p);
                textures_captured++;
            }
        }
    }

    TDITRACE("@T-glDrawElements()");
}


extern "C" void glGenTextures(GLsizei n, GLuint * textures)
{
    static void (*__glGenTextures)(GLsizei, GLuint *)=NULL;

    if (__glGenTextures==NULL) {
        __glGenTextures = (void (*)(GLsizei, GLuint *))dlsym(RTLD_NEXT, "glGenTextures");
        if (NULL == __glGenTextures) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glGenTextures() %d", n);

    __glGenTextures(n, textures);
}

extern "C" GLvoid glBindTexture(GLenum target, GLuint texture)
{
    static void (*__glBindTexture)(GLenum, GLuint)=NULL;

    if (__glBindTexture==NULL) {
        __glBindTexture = (void (*)(GLenum, GLuint))dlsym(RTLD_NEXT, "glBindTexture");
        if (NULL == __glBindTexture) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("#gltexturebinds~%d\n", ++texturebind_counter);

    TDITRACE("glBindTexture() #%d,%u", ++glBindTexture_counter, texture);

    __glBindTexture(target, texture);

    boundtexture = texture;
}


extern "C" GLvoid glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    static void (*__glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*)=NULL;

    if (__glTexImage2D==NULL) {
        __glTexImage2D = (void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*))dlsym(RTLD_NEXT, "glTexImage2D");
        if (NULL == __glTexImage2D) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("@T+glTexImage2D() #%d,%dx%d,%s,%s,%u,%p", ++glTexImage2D_counter, width, height, TYPESTRING(type), FORMATSTRING(format), boundtexture, pixels);

    if (texturerecording) {
        texturecapture_captexture(boundtexture, PARENT, current_frame, 0, 0, width, height, (int)format, (int)type, pixels);
        textures_captured++;
    }

    __glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);

    TDITRACE("@T-glTexImage2D()");
}

extern "C" GLvoid glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    static void (*__glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*)=NULL;

    if (__glTexSubImage2D==NULL) {
        __glTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid* pixels))dlsym(RTLD_NEXT, "glTexSubImage2D");
        if (NULL == __glTexSubImage2D) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("@T+glTexSubImage2D() #%d,%dx%d+%d+%d,%s,%s,%u,%p", ++glTexSubImage2D_counter, width, height, xoffset, yoffset, TYPESTRING(type), FORMATSTRING(format), boundtexture, pixels);

    if (texturerecording) {
        texturecapture_captexture(boundtexture, SUB, current_frame, xoffset, yoffset, width, height, (int)format, (int)type, pixels);
        textures_captured++;
    }

    __glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);

    TDITRACE("@T-glTexSubImage2D()");
}


extern "C" GLvoid glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    static void (*__glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint)=NULL;

    if (__glCopyTexImage2D==NULL) {
        __glCopyTexImage2D = (void (*)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint))dlsym(RTLD_NEXT, "glCopyTexImage2D");
        if (NULL == __glCopyTexImage2D) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glCopyTexImage2D() %dx%d+%d+%d,%u", width, height, x, y , boundtexture);

    __glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}


extern "C" GLvoid glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    static void (*__glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei)=NULL;

    if (__glCopyTexSubImage2D==NULL) {
        __glCopyTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei))dlsym(RTLD_NEXT, "glCopyTexSubImage2D");
        if (NULL == __glCopyTexSubImage2D) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glCopyTexSubImage2D() %dx%d+%d+%d+%d+%d,%u", width, height, x, y, xoffset, yoffset, boundtexture);

    __glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}


extern "C" GLvoid glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    static void (*__glTexParameteri)(GLenum, GLenum, GLint)=NULL;

    if (__glTexParameteri==NULL) {
        __glTexParameteri = (void (*)(GLenum, GLenum, GLint))dlsym(RTLD_NEXT, "glTexParameteri");
        if (NULL == __glTexParameteri) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    __glTexParameteri(target, pname, param);
}


extern "C" GLvoid glUseProgram(GLuint program)
{
    static void (*__glUseProgram)(GLuint)=NULL;

    if (__glUseProgram==NULL) {
        __glUseProgram = (void (*)(GLuint program))dlsym(RTLD_NEXT, "glUseProgram");
        if (NULL == __glUseProgram) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    __glUseProgram(program);

    currentprogram = program;
}


extern "C" GLvoid glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
    static void (*__glShaderSource)(GLuint, GLsizei, const GLchar**, const GLint*)=NULL;

    if (__glShaderSource==NULL) {
        __glShaderSource = (void (*)(GLuint, GLsizei, const GLchar**, const GLint*))dlsym(RTLD_NEXT, "glShaderSource");
        if (NULL == __glShaderSource) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glShaderSource() #%d %u", ++glShaderSource_counter, shader);

    if (shaderrecording) {
        shadercapture_capshader(shader, count, string, length);
        shaders_captured++;
    }

    __glShaderSource(shader, count, string, length);
}


extern "C" GLvoid glAttachShader(GLuint program, GLuint shader)
{
    static void (*__glAttachShader)(GLuint, GLuint)=NULL;

    if (__glAttachShader==NULL) {
        __glAttachShader = (void (*)(GLuint, GLuint))dlsym(RTLD_NEXT, "glAttachShader");
        if (NULL == __glAttachShader) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glAttachShader() %u %u", program, shader);

    if (shaderrecording) {
        shadercapture_referenceprogram(shader, program);
    }

    __glAttachShader(program, shader);
}

#if 0
extern "C" GLvoid glEnable(GLenum cap)
{
    static void (*__glEnable)(GLenum)=NULL;

    if (__glEnable==NULL) {
        __glEnable = (void (*)(GLenum))dlsym(RTLD_NEXT, "glEnable");
        if (NULL == __glEnable) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glEnable() %s", CAPSTRING(cap));

    __glEnable(cap);
}
#endif

#if 0
extern "C" GLvoid glDisable(GLenum cap)
{
    static void (*__glDisable)(GLenum)=NULL;

    if (__glDisable==NULL) {
        __glDisable = (void (*)(GLenum))dlsym(RTLD_NEXT, "glDisable");
        if (NULL == __glDisable) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glDisable() %s", CAPSTRING(cap));

    __glDisable(cap);
}
#endif

#if 0
extern "C" GLvoid glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    static void (*__glBlendFunc)(GLenum, GLenum)=NULL;

    if (__glBlendFunc==NULL) {
        __glBlendFunc = (void (*)(GLenum, GLenum))dlsym(RTLD_NEXT, "glBlendFunc");
        if (NULL == __glBlendFunc) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glBlendFunc() %s %s", BLENDSTRING(sfactor), BLENDSTRING(dfactor));

    __glBlendFunc(sfactor, dfactor);
}
#endif

#if 0
extern "C" GLvoid glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    static void (*__glScissor)(GLint, GLint, GLsizei, GLsizei)=NULL;

    if (__glScissor==NULL) {
        __glScissor = (void (*)(GLint, GLint, GLsizei, GLsizei))dlsym(RTLD_NEXT, "glScissor");
        if (NULL == __glScissor) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glScissor() %dx%d+%d+%d", width, height, x, y);

    __glScissor(x, y, width, height);
}
#endif

#if 0
extern "C" GLvoid glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    static void (*__glClearColor)(GLclampf, GLclampf, GLclampf, GLclampf)=NULL;

    if (__glClearColor==NULL) {
        __glClearColor = (void (*)(GLclampf, GLclampf, GLclampf, GLclampf))dlsym(RTLD_NEXT, "glClearColor");
        if (NULL == __glClearColor) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glClearColor() %g %g %g %g", red, green, blue, alpha);

    __glClearColor(red, green, blue, alpha);
}
#endif

extern "C" GLvoid glClear(GLbitfield mask)
{
    static void (*__glClear)(GLbitfield)=NULL;

    if (__glClear==NULL) {
        __glClear = (void (*)(GLbitfield))dlsym(RTLD_NEXT, "glClear");
        if (NULL == __glClear) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("@T+glClear() %s", CLEARSTRING(mask));

    __glClear(mask);

    TDITRACE("@T-glClear()");
}



extern "C" GLvoid glGenFramebuffers(GLsizei n, GLuint * framebuffers)
{
    static void (*__glGenFramebuffers)(GLsizei, GLuint*)=NULL;

    if (__glGenFramebuffers==NULL) {
        __glGenFramebuffers = (void (*)(GLsizei, GLuint*))dlsym(RTLD_NEXT, "glGenFramebuffers");
        if (NULL == __glGenFramebuffers) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glGenFramebuffers() %d", n);

    __glGenFramebuffers(n, framebuffers);
}


extern "C" GLvoid glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    static void (*__glBindFramebuffer)(GLenum, GLuint)=NULL;

    if (__glBindFramebuffer==NULL) {
        __glBindFramebuffer = (void (*)(GLenum, GLuint))dlsym(RTLD_NEXT, "glBindFramebuffer");
        if (NULL == __glBindFramebuffer) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glBindFramebuffer() %u", framebuffer);

    __glBindFramebuffer(target, framebuffer);

    boundframebuffer = framebuffer;
}


extern "C" GLvoid glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    static void (*__glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint)=NULL;

    if (__glFramebufferTexture2D==NULL) {
        __glFramebufferTexture2D = (void (*)(GLenum, GLenum, GLenum, GLuint, GLint))dlsym(RTLD_NEXT, "glFramebufferTexture2D");
        if (NULL == __glFramebufferTexture2D) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glFramebufferTexture2D() %s,%u", ATTACHMENTSTRING(attachment), texture);

    __glFramebufferTexture2D(target, attachment, textarget, texture, level);

    framebuffertexture[boundframebuffer] = texture;
}


extern "C" GLvoid glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    static void (*__glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei)=NULL;

    if (__glRenderbufferStorage==NULL) {
        __glRenderbufferStorage = (void (*)(GLenum, GLenum, GLsizei, GLsizei))dlsym(RTLD_NEXT, "glRenderbufferStorage");
        if (NULL == __glRenderbufferStorage) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glRenderbufferStorage() %s0x%08x,%dx%d", IFORMATSTRING(internalformat), internalformat, width, height);

    __glRenderbufferStorage(target, internalformat, width, height);

    renderbufferwidth[boundrenderbuffer] = width;
    renderbufferheight[boundrenderbuffer] = height;
}


extern GLvoid glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
    static void (*__glgenRenderbuffers)(GLsizei, GLuint*)=NULL;

    if (__glgenRenderbuffers==NULL) {
        __glgenRenderbuffers = (void (*)(GLsizei, GLuint*))dlsym(RTLD_NEXT, "glgenRenderbuffers");
        if (NULL == __glgenRenderbuffers) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glgenRenderbuffers() %d", n);

    __glgenRenderbuffers(n, renderbuffers);
}

extern "C" GLvoid glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    static void (*__glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint)=NULL;

    if (__glFramebufferRenderbuffer==NULL) {
        __glFramebufferRenderbuffer = (void (*)(GLenum, GLenum, GLenum, GLuint))dlsym(RTLD_NEXT, "glFramebufferRenderbuffer");
        if (NULL == __glFramebufferRenderbuffer) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glFramebufferRenderbuffer() %s,%u", ATTACHMENTSTRING(attachment), renderbuffer);

    __glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);

    framebufferrenderbuffer[boundframebuffer] = renderbuffer;
}

extern "C" GLvoid glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    static void (*__glBindRenderbuffer)(GLenum, GLuint)=NULL;

    if (__glBindRenderbuffer==NULL) {
        __glBindRenderbuffer = (void (*)(GLenum, GLuint))dlsym(RTLD_NEXT, "glBindRenderbuffer");
        if (NULL == __glBindRenderbuffer) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glBindRenderbuffer() %u", renderbuffer);

    __glBindRenderbuffer(target, renderbuffer);

    boundrenderbuffer = renderbuffer;
    framebufferrenderbuffer[boundframebuffer] = renderbuffer;
}



#if 0
extern "C" int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
    static int (*__connect)(int, const struct sockaddr*, socklen_t)=NULL;

    #if 0
    unsigned char *c;
    int port,ok=1;
    #endif

    if (__connect==NULL) {
        __connect = (int (*)(int, const struct sockaddr*, socklen_t))dlsym(RTLD_NEXT,"connect");
        if (NULL == __connect) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    #if 0
    if (serv_addr->sa_family==AF_INET6) return EACCES;
    if (serv_addr->sa_family==AF_INET){
        c=serv_addr->sa_data;
        port=256*c[0]+c[1];
        c+=2;
        ok=0;
        // Allow all contacts with localhost
        if ((*c==127)&&(*(c+1)==0)&&(*(c+2)==0)&&(*(c+3)==1)) ok=1;
        // Allow contact to any WWW cache on 8080
        if (port==8080) ok=1;
    }
    //if (ok) return connect_real(sockfd,serv_addr,addrlen);
    return EACCES;
    #endif

    TDITRACE("connect()");

    return __connect(sockfd,serv_addr,addrlen);

    #if 0
    if (getenv("WRAP_TCP_DEBUG"))
      fprintf(stderr,"connect() denied to address %d.%d.%d.%d port %d\n",
              (int)(*c),(int)(*(c+1)),(int)(*(c+2)),(int)(*(c+3)),
              port);
    #endif
}

extern "C" ssize_t read(int fd, void *buf, size_t count)
{
    static ssize_t (*__read)(int, void *, size_t)=NULL;

    if (__read==NULL) {
        __read = (ssize_t (*)(int, void*, size_t))dlsym(RTLD_NEXT,"read");
        if (NULL == __read) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }


    TDITRACE("@T+read() %d", count);

    ssize_t s = __read(fd, buf, count);

    TDITRACE("@T+read()");

    return s;
}

extern "C" ssize_t write(int fd, const void *buf, size_t count)
{
    static ssize_t (*__write)(int, const void *, size_t)=NULL;

    if (__write==NULL) {
        __write = (ssize_t (*)(int, const void*, size_t))dlsym(RTLD_NEXT,"write");
        if (NULL == __write) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }


    TDITRACE("@T+write() %d", count);

    ssize_t s = __write(fd, buf, count);

    TDITRACE("@T-write()");

    return s;
}
#endif
