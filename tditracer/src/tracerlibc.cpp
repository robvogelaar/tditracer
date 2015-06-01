#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdarg.h>

#include "tracermain.h"
#include "tdi.h"


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
    static void* (*real_dlsym)(void*, const char*) = NULL;
    if (real_dlsym == NULL)
        real_dlsym = _dl_sym(RTLD_NEXT, "dlsym", dlsym);
    return real_dlsym(handle, name);
}
#endif


#if 0
extern "C" int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
    static int (*__connect)(int, const struct sockaddr*, socklen_t) = NULL;

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
#endif


#if 1
extern "C" int open(const char *pathname, int flags, ...)
{
    static int (*__open)(const char*, int, ...) = NULL;

    if (__open == NULL) {
        __open = (int (*)(const char*, int, ...))dlsym(RTLD_NEXT, "open");
        if (NULL == __open) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    va_list args;
    va_start(args, flags);
    int a1 = va_arg(args, int);
    va_end(args);

    if (libcrecording) {
        TDITRACE("@T+open()_%s", pathname);
    }

    int ret = __open(pathname, flags, a1);

    if (libcrecording) {
        TDITRACE("@T-open()_%s %d", pathname, ret);
    }

    return ret;
}
#endif


#if 1
extern "C" ssize_t read(int fd, void *buf, size_t count)
{
    static ssize_t (*__read)(int, void *, size_t) = NULL;

    if (__read==NULL) {
        __read = (ssize_t (*)(int, void*, size_t))dlsym(RTLD_NEXT,"read");
        if (NULL == __read) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }


    if (libcrecording) {
        TDITRACE("@T+read() %d %d", fd, count);
    }

    ssize_t s = __read(fd, buf, count);

    if (libcrecording) {
        TDITRACE("@T-read()");
    }

    return s;
}
#endif


#if 1
extern "C" ssize_t write(int fd, const void *buf, size_t count)
{
    static ssize_t (*__write)(int, const void *, size_t) = NULL;

    if (__write==NULL) {
        __write = (ssize_t (*)(int, const void*, size_t))dlsym(RTLD_NEXT,"write");
        if (NULL == __write) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }


    if (libcrecording) {
        TDITRACE("@T+write() %d %d", fd, count);
    }

    ssize_t s = __write(fd, buf, count);

    if (libcrecording) {
        TDITRACE("@T-write()");
    }

    return s;
}
#endif


#if 1
extern "C" int ioctl(int d, int request, ...)
{
    static int (*__ioctl)(int d, int request, ...) = NULL;

    if (__ioctl == NULL) {
        __ioctl = (int (*)(int d, int request, ...))dlsym(RTLD_NEXT, "ioctl");
        if (NULL == __ioctl) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    va_list args;
    va_start(args, request);
    int a1 = va_arg(args, int);
    va_end(args);

    if (libcrecording) {
        TDITRACE("@T+ioctl() %d 0x%x", d, request);
    }

    int ret = __ioctl(d, request, a1);
    
    if (libcrecording) {
        TDITRACE("@T-ioctl()");
    }

    return ret;
}
#endif


#if 1
extern "C" void *memcpy(void *dest, const void *src, size_t n)
{
    static void* (*__memcpy)(void *, const void *, size_t) = NULL;

    if (__memcpy == NULL) {
        __memcpy = (void* (*)(void *, const void *, size_t))dlsym(RTLD_NEXT,"memcpy");
        if (NULL == __memcpy) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        TDITRACE("@T+memcpy() 0x%x 0x%x %d", dest, src, n);
    }

    void* ret = __memcpy(dest, src, n);
    
    if (libcrecording) {
        TDITRACE("@T-memcpy()");
    }

    return ret;
}
#endif


#if 1
extern "C" void *memset(void *dest, int c, size_t n)
{
    static void* (*__memset)(void *, int, size_t) = NULL;

    if (__memset == NULL) {
        __memset = (void* (*)(void *, int, size_t))dlsym(RTLD_NEXT,"memset");
        if (NULL == __memset) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        TDITRACE("@T+memset() 0x%x,0x%x,%d", dest, c, n);
    }

    void* ret = __memset(dest, c, n);

    if (libcrecording) {
        TDITRACE("@T-memset()");
    }

    return ret;
}
#endif


#if 1
extern "C" char *strcpy(char *dest, const char *src)
{
    static char* (*__strcpy)(char *, const char *) = NULL;

    if (__strcpy == NULL) {
        __strcpy = (char* (*)(char *, const char *))dlsym(RTLD_NEXT,"strcpy");
        if (NULL == __strcpy) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        TDITRACE("@T+strcpy() 0x%x,0x%x", dest, src);
    }

    char* ret = __strcpy(dest, src);
    
    if (libcrecording) {
        TDITRACE("@T-strcpy()");
    }

    return ret;
}
#endif


#if 1
extern "C" char *strncpy(char *dest, const char *src, size_t n)
{
    static char* (*__strncpy)(char *, const char *, size_t) = NULL;

    if (__strncpy == NULL) {
        __strncpy = (char* (*)(char *, const char *, size_t))dlsym(RTLD_NEXT,"strncpy");
        if (NULL == __strncpy) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        TDITRACE("@T+strncpy() 0x%x 0x%x %d", dest, src, n);
    }

    char* ret = __strncpy(dest, src, n);

    if (libcrecording) {
        TDITRACE("@T-strncpy()");
    }

    return ret;
}
#endif


#if 1
extern "C" void *malloc(size_t size)
{
    static void* (*__malloc)(size_t) = NULL;

    if (__malloc == NULL) {
        __malloc = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
        if (NULL == __malloc) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        TDITRACE("@T+malloc() %d", size);
    }

    void* ret = __malloc(size);

    if (libcrecording) {
        TDITRACE("@T-malloc()");
    }

    return ret;
}
#endif
