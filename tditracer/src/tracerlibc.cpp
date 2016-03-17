#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <poll.h>

#include "tracermain.h"
#include "tdi.h"

#define MAXSTRLEN 512
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

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

    // tditrace("dlopen() %s", filename);

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

    tditrace("connect()");

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
extern "C" int open(const char *pathname, int flags, ...) {
    static int (*__open)(const char *, int, ...) = NULL;

    if (__open == NULL) {
        __open = (int (*)(const char *, int, ...))dlsym(RTLD_NEXT, "open");
        if (NULL == __open) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    va_list args;
    va_start(args, flags);
    int a1 = va_arg(args, int);
    va_end(args);

    if (libcrecording || libcopenrecording) {
        tditrace("@I+open() %s", pathname);
    }

    int ret = __open(pathname, flags, a1);

    if (libcrecording || libcopenrecording) {
        tditrace("@I-open() =%d", pathname, ret);
    }

    return ret;
}
#endif

extern "C" FILE *fopen(const char *path, const char *mode) {
    static FILE *(*__fopen)(const char *, const char *) = NULL;

    if (__fopen == NULL) {
        __fopen =
            (FILE * (*)(const char *, const char *))dlsym(RTLD_NEXT, "fopen");
        if (NULL == __fopen) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcfopenrecording) {
        tditrace("@I+fopen() %s", path);
    }

    FILE *ret = __fopen(path, mode);

    if (libcrecording || libcfopenrecording) {
        tditrace("@I-fopen() =%d", ret);
    }

    return ret;
}

extern "C" FILE *fdopen(int fd, const char *mode) {
    static FILE *(*__fdopen)(int, const char *) = NULL;

    if (__fdopen == NULL) {
        __fdopen = (FILE * (*)(int, const char *))dlsym(RTLD_NEXT, "fdopen");
        if (NULL == __fdopen) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcfdopenrecording) {
        tditrace("@I+fdopen() %d", fd);
    }

    FILE *ret = __fdopen(fd, mode);

    if (libcrecording || libcfdopenrecording) {
        tditrace("@I-fdopen() =%d", ret);
    }

    return ret;
}

extern "C" FILE *freopen(const char *path, const char *mode, FILE *stream) {
    static FILE *(*__freopen)(const char *, const char *, FILE *) = NULL;

    if (__freopen == NULL) {
        __freopen = (FILE * (*)(const char *, const char *, FILE *))dlsym(
            RTLD_NEXT, "freopen");
        if (NULL == __freopen) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcfreopenrecording) {
        tditrace("@I+freopen() %s", path);
    }

    FILE *ret = __freopen(path, mode, stream);

    if (libcrecording || libcfreopenrecording) {
        tditrace("@I-freopen() =%d", ret);
    }

    return ret;
}

#if 0
extern "C" ssize_t read(int fd, void *buf, size_t count) {
    static ssize_t (*__read)(int, void *, size_t) = NULL;

    if (__read == NULL) {
        __read = (ssize_t (*)(int, void *, size_t))dlsym(RTLD_NEXT, "read");
        if (NULL == __read) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcreadrecording) {
        tditrace("@I+read() %d %d", fd, count);
    }

    ssize_t ret = __read(fd, buf, count);

    if (libcrecording || libcreadrecording) {
        if (ret == -1) {
            tditrace("@I-read() =-1");
        } else if (ret == 0) {
            tditrace("@I-read() =0");
        } else {
            if (0) {
                char s[MAXSTRLEN + 1];
                strncpy(s, (const char *)buf, MIN(MAXSTRLEN, ret));
                s[MIN(MAXSTRLEN, ret)] = '\0';
                tditrace("@I-read() =%d \"%s\"", ret, s);
            } else {
                tditrace("@I-read() =%d", ret);
            }
        }
    }

    return ret;
}
#endif

#if 0
extern "C" ssize_t write(int fd, const void *buf, size_t count) {
    static ssize_t (*__write)(int, const void *, size_t) = NULL;

    if (__write == NULL) {
        __write =
            (ssize_t (*)(int, const void *, size_t))dlsym(RTLD_NEXT, "write");
        if (NULL == __write) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcwriterecording) {
        if (0) {
            char s[MAXSTRLEN + 1];
            strncpy(s, (const char *)buf, MIN(MAXSTRLEN, count));
            s[MIN(MAXSTRLEN, count)] = '\0';
            tditrace("@I+write() %d %d \"%s\"", fd, count, s);
        } else {
            tditrace("@I+write() %d %d", fd, count);
        }
    }

    ssize_t ret = __write(fd, buf, count);

    if (libcrecording || libcwriterecording) {
        tditrace("@I-write() =%d", ret);
    }

    return ret;
}
#endif

#if 1
extern "C" int socket(int domain, int type, int protocol) {
    static int (*__socket)(int, int, int) = NULL;

    if (__socket == NULL) {
        __socket = (int (*)(int, int, int))dlsym(RTLD_NEXT, "socket");
        if (NULL == __socket) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcsocketrecording) {
        tditrace("@I+socket() %d %d %d", domain, type, protocol);
    }

    int ret = __socket(domain, type, protocol);

    if (libcrecording || libcsocketrecording) {
        tditrace("@I-socket() =%d", ret);
    }

    return ret;
}
#endif

extern "C" ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    static ssize_t (*__send)(int, const void *, size_t, int) = NULL;

    if (__send == NULL) {
        __send = (ssize_t (*)(int, const void *, size_t, int))dlsym(RTLD_NEXT,
                                                                    "send");
        if (NULL == __send) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcsendrecording) {
        if (MAXSTRLEN) {

            if (buf && !strchr((char *)buf, '\f')) {
                char s[MAXSTRLEN + 1];
                strncpy(s, (const char *)buf, MIN(MAXSTRLEN, len));
                s[MIN(MAXSTRLEN, len)] = '\0';

                if (strncmp((const char *)buf, "HTTP", 4) == 0 ||
                    strncmp((const char *)buf, "{", 1) == 0 ||
                    strncmp((const char *)buf, "data:", 5) == 0) {
                    tditrace("@E+send()_%d %d \"%s\"", sockfd, len, s);
                    tditrace("@I+send() %d %d \"%s\"", sockfd, len, s);
                } else {
                    s[MIN(16, len)] = '\0';
                    tditrace("@I+send() %d %d \"%s\"...", sockfd, len, s);
                }

            } else {
                tditrace("@I+send() %d %d \"???\"", sockfd, len);
            }

        } else {
            tditrace("@I+send() %d %d", sockfd, len);
        }
    }

    ssize_t ret = __send(sockfd, buf, len, flags);

    if (libcrecording || libcsendrecording) {
        tditrace("@I-send() =%d", ret);
    }

    return ret;
}

extern "C" ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                          const struct sockaddr *dest_addr, socklen_t addrlen) {
    static ssize_t (*__sendto)(int, const void *, size_t, int,
                               const struct sockaddr *, socklen_t) = NULL;

    if (__sendto == NULL) {
        __sendto = (ssize_t (*)(int, const void *, size_t, int,
                                const struct sockaddr *,
                                socklen_t))dlsym(RTLD_NEXT, "sendto");
        if (NULL == __sendto) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcsendtorecording) {
        if (MAXSTRLEN) {
            char s[MAXSTRLEN + 1];
            strncpy(s, (const char *)buf, MIN(MAXSTRLEN, len));
            s[MIN(MAXSTRLEN, len)] = '\0';
            tditrace("@I+sendto() %d %d \"%s\"", sockfd, len, s);
        } else {
            tditrace("@I+sendto() %d %d ", sockfd, len);
        }
    }

    ssize_t ret = __sendto(sockfd, buf, len, flags, dest_addr, addrlen);

    if (libcrecording || libcsendtorecording) {
        tditrace("@I-sendto() =%d", ret);
    }

    return ret;
}

extern "C" ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
    static ssize_t (*__sendmsg)(int, const struct msghdr *, int) = NULL;

    if (__sendmsg == NULL) {
        __sendmsg = (ssize_t (*)(int, const struct msghdr *, int))dlsym(
            RTLD_NEXT, "sendmsg");
        if (NULL == __sendmsg) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcsendmsgrecording) {
        // tditrace("@I+sendmsg() %d %d \"%s\"", sockfd, msg->msg_iovlen,
        // msg->msg_iov[0].iov_base);
        tditrace("@I+sendmsg() %d %d \"\"", sockfd, msg->msg_iovlen);
    }

    ssize_t ret = __sendmsg(sockfd, msg, flags);

    if (libcrecording || libcsendmsgrecording) {
        tditrace("@I-sendmsg() =%d", ret);
        tditrace("@E+sendmsg()_%d =%d %d \"\"", sockfd, ret, msg->msg_iovlen);
    }

    return ret;
}

extern "C" int sendmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
                        int flags) {
    static int (*__sendmmsg)(int, struct mmsghdr *, unsigned int, int) = NULL;

    if (__sendmmsg == NULL) {
        __sendmmsg = (int (*)(int, struct mmsghdr *, unsigned int, int))dlsym(
            RTLD_NEXT, "sendmmsg");
        if (NULL == __sendmmsg) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcsendmmsgrecording) {
        tditrace("@I+sendmmsg() %d 0x%x", sockfd, msgvec);
    }

    int ret = __sendmmsg(sockfd, msgvec, vlen, flags);

    if (libcrecording || libcsendmmsgrecording) {
        tditrace("@I-sendmmsg() =%d", ret);
    }

    return ret;
}

extern "C" ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    static ssize_t (*__recv)(int, void *, size_t, int) = NULL;

    if (__recv == NULL) {
        __recv =
            (ssize_t (*)(int, void *, size_t, int))dlsym(RTLD_NEXT, "recv");
        if (NULL == __recv) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcrecvrecording) {
        tditrace("@I+recv() %d %d", sockfd, len);
    }

    ssize_t ret = __recv(sockfd, buf, len, flags);

    if (libcrecording || libcrecvrecording) {
        if (ret == -1) {
            tditrace("@I-recv() =-1");
        } else if (ret == 0) {
            tditrace("@I-recv() =0");
        } else {
            if (MAXSTRLEN) {
                if (buf && !strchr((char *)buf, '\f')) {
                    char s[MAXSTRLEN + 1];
                    strncpy(s, (const char *)buf, MIN(MAXSTRLEN, ret));
                    s[MIN(MAXSTRLEN, ret)] = '\0';

                    if (strncmp((const char *)buf, "GET", 3) == 0 ||
                        strncmp((const char *)buf, "POST", 4) == 0 ||
                        strncmp((const char *)buf, "{", 1) == 0) {
                        tditrace("@E+recv()_%d =%d \"%s\"", sockfd, ret, s);
                        tditrace("@I-recv() =%d \"%s\"", ret, s);
                    } else {
                        s[MIN(4, len)] = '\0';
                        // tditrace("@I-recv() =%d \"%s\"...", ret, s);
                        tditrace("@I-recv() =%d \"%s\"...", ret, "xxx");
                    }

                } else {
                    tditrace("@I-recv() =%d \"???\"", ret);
                }
            } else {
                tditrace("@I-recv() =%d", ret);
            }
        }
    }

    return ret;
}

extern "C" ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                            struct sockaddr *src_addr, socklen_t *addrlen) {
    static ssize_t (*__recvfrom)(int, void *, size_t, int, struct sockaddr *,
                                 socklen_t *) = NULL;

    if (__recvfrom == NULL) {
        __recvfrom = (ssize_t (*)(int, void *, size_t, int, struct sockaddr *,
                                  socklen_t *))dlsym(RTLD_NEXT, "recvfrom");
        if (NULL == __recvfrom) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcrecvfromrecording) {
        tditrace("@I+recvfrom() %d %d 0x%x", sockfd, len, flags);
    }

    ssize_t ret = __recvfrom(sockfd, buf, len, flags, src_addr, addrlen);

    if (libcrecording || libcrecvfromrecording) {
        if (ret == -1) {
            tditrace("@I-recvfrom() =-1");
        } else if (ret == 0) {
            tditrace("@I-recvfrom() =0");
        } else {
            if (MAXSTRLEN) {
                char s[MAXSTRLEN + 1];
                strncpy(s, (const char *)buf, MIN(MAXSTRLEN, ret));
                s[MIN(MAXSTRLEN, ret)] = '\0';
                tditrace("@I-recvfrom() =%d \"%s\"", ret, s);
            } else {
                tditrace("@I-recvfrom() =%d", ret);
            }
        }
    }

    return ret;
}

extern "C" ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    static ssize_t (*__recvmsg)(int, struct msghdr *, int) = NULL;

    if (__recvmsg == NULL) {
        __recvmsg =
            (ssize_t (*)(int, struct msghdr *, int))dlsym(RTLD_NEXT, "recvmsg");
        if (NULL == __recvmsg) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcrecvmsgrecording) {
        tditrace("@I+recvmsg() %d %d", sockfd, msg->msg_iovlen);
    }

    ssize_t ret = __recvmsg(sockfd, msg, flags);

    if (libcrecording || libcrecvmsgrecording) {
        // tditrace("@I+recvmsg() %d %d \"%s\"", sockfd, msg->msg_iovlen,
        // msg->msg_iov[0].iov_base);

        tditrace("@I-recvmsg() =%d \"\"", ret);
        tditrace("@E+recvmsg()_%d =%d \"\"", sockfd, ret);
    }

    return ret;
}

#if defined (HAVE_REFSW_NEXUS_CONFIG_H) || defined(__i386)
#define USE_CONST
#else
#define USE_CONST const
#endif

extern "C" int recvmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
                        int flags, USE_CONST struct timespec *timeout) {

    static int (*__recvmmsg)(int, struct mmsghdr *, unsigned int, int,
                             USE_CONST struct timespec *) = NULL;

    if (__recvmmsg == NULL) {
        __recvmmsg =
            (int (*)(int, struct mmsghdr *, unsigned int, int,
                     USE_CONST struct timespec *))dlsym(RTLD_NEXT, "recvmmsg");
        if (NULL == __recvmmsg) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcrecvmmsgrecording) {
        tditrace("@I+recvmmsg() %d 0x%x %d", sockfd, msgvec, vlen);
    }

    int ret = __recvmmsg(sockfd, msgvec, vlen, flags, timeout);

    if (libcrecording || libcrecvmmsgrecording) {
        tditrace("@I-recvmmsg() =%d", ret);
    }

    return ret;
}

extern "C" int select(int nfds, fd_set *readfds, fd_set *writefds,
                      fd_set *exceptfds, struct timeval *timeout) {
    static int (*__select)(int, fd_set *, fd_set *, fd_set *,
                           struct timeval *) = NULL;

    if (__select == NULL) {
        __select = (int (*)(int, fd_set *, fd_set *, fd_set *,
                            struct timeval *))dlsym(RTLD_NEXT, "select");
        if (NULL == __select) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcselectrecording) {
        tditrace("@I+select() %d %x %x %x", nfds, readfds, writefds, exceptfds);
    }

    int ret = __select(nfds, readfds, writefds, exceptfds, timeout);

    if (libcrecording || libcselectrecording) {
        tditrace("@I-select() =%d", ret);
    }

    return ret;
}

extern "C" int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    static int (*__poll)(struct pollfd *, nfds_t, int) = NULL;

    if (__poll == NULL) {
        __poll =
            (int (*)(struct pollfd *, nfds_t, int))dlsym(RTLD_NEXT, "poll");
        if (NULL == __poll) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording || libcpollrecording) {
        tditrace("@I+poll() %x %d %d", fds, nfds, timeout);
    }

    int ret = __poll(fds, nfds, timeout);

    if (libcrecording || libcpollrecording) {
        tditrace("@I-poll() =%d", ret);
    }

    return ret;
}

extern "C" int ioctl(int d, int request, ...) {
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

    if (libcrecording || libcioctlrecording) {
        tditrace("@I+ioctl() %d 0x%x", d, request);
    }

    int ret = __ioctl(d, request, a1);

    if (libcrecording || libcioctlrecording) {
        tditrace("@I-ioctl() =%d", ret);
    }

    return ret;
}

#if 0
extern "C" void *memcpy(void *dest, const void *src, size_t n) {
    static void *(*__memcpy)(void *, const void *, size_t) = NULL;

    if (__memcpy == NULL) {
        __memcpy =
            (void *(*)(void *, const void *, size_t))dlsym(RTLD_NEXT, "memcpy");
        if (NULL == __memcpy) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        tditrace("@I+memcpy() 0x%x 0x%x %d", dest, src, n);
    }

    void *ret = __memcpy(dest, src, n);

    if (libcrecording) {
        tditrace("@I-memcpy()");
    }

    return ret;
}
#endif

#if 0
extern "C" void *memset(void *dest, int c, size_t n) {
    static void *(*__memset)(void *, int, size_t) = NULL;

    if (__memset == NULL) {
        __memset = (void *(*)(void *, int, size_t))dlsym(RTLD_NEXT, "memset");
        if (NULL == __memset) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        tditrace("@I+memset() 0x%x,0x%x,%d", dest, c, n);
    }

    void *ret = __memset(dest, c, n);

    if (libcrecording) {
        tditrace("@I-memset()");
    }

    return ret;
}
#endif

#if 0
extern "C" char *strcpy(char *dest, const char *src) {
    static char *(*__strcpy)(char *, const char *) = NULL;

    if (__strcpy == NULL) {
        __strcpy = (char *(*)(char *, const char *))dlsym(RTLD_NEXT, "strcpy");
        if (NULL == __strcpy) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        tditrace("@I+strcpy() 0x%x,0x%x", dest, src);
    }

    char *ret = __strcpy(dest, src);

    if (libcrecording) {
        tditrace("@I-strcpy()");
    }

    return ret;
}
#endif

#if 0
extern "C" char *strncpy(char *dest, const char *src, size_t n) {
    static char *(*__strncpy)(char *, const char *, size_t) = NULL;

    if (__strncpy == NULL) {
        __strncpy = (char *(*)(char *, const char *, size_t))dlsym(RTLD_NEXT,
                                                                   "strncpy");
        if (NULL == __strncpy) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        tditrace("@I+strncpy() 0x%x 0x%x %d", dest, src, n);
    }

    char *ret = __strncpy(dest, src, n);

    if (libcrecording) {
        tditrace("@I-strncpy()");
    }

    return ret;
}
#endif

#if 1
extern "C" void *malloc(size_t size) {
    static void *(*__malloc)(size_t) = NULL;

    if (__malloc == NULL) {
        __malloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
        if (NULL == __malloc) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

#if 0
    if (NULL == glsl) {
        glsl = dlsym(RTLD_NEXT, "glsl_set_line_capture_vertex");
        if (glsl == NULL) {
            fprintf(stderr, "Error in `dlsym`: %s : %s\n", dlerror(), "glsl_set_line_capture_vertex");
            fprintf(stderr, "glsl = 0x%x [0x%x]\n", glsl, &glsl);
        } else {
            fprintf(stderr, "got glsl!!!!!!!!! = 0x%x [0x%x]\n", glsl, &glsl);
        }
    }
#endif

#if 0
    //static void *(*__malloc)(size_t) = NULL;
    //static void* (*operator new)(unsigned int i) = NULL;
    static void* __new = NULL;

    if (__new == NULL) {
        __new = (void *)dlsym(RTLD_NEXT, "_Znwj");
        if (NULL == __new) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
        else {
            fprintf(stderr, "Got operator new : %p\n", __new);
        }
    }
#endif

#if 0
    static void* v3ddriver_base = NULL;
    static int v3ddriver_size = 0x110000;
    static void* ___ftext = NULL;

    if (NULL == v3ddriver_base) {
        Dl_info dli;
        if (dladdr(dlsym(RTLD_NEXT, "glEnable"), &dli)) {
            fprintf(stderr, "dladdr, dli_fname[%s], dli_fbase[%x], dli_sname[%s], dli_saddr[%x]\n",
                dli.dli_fname, dli.dli_fbase, dli.dli_sname, dli.dli_saddr);

            //const char *dli_fname;  /* Pathname of shared object that
            //void       *dli_fbase;  /* Address at which shared object
            //const char *dli_sname;  /* Name of nearest symbol with address
            //void       *dli_saddr;  /* Exact address of symbol named
            v3ddriver_base = dli.dli_fbase;

            if (dlopen("/usr/lib/libv3ddriver.so", RTLD_NOW | RTLD_NOLOAD)) {
                fprintf(stderr, "/usr/lib/libv3ddriver.so - is resident\n");
                ___ftext = dlsym(dlopen("/usr/lib/libv3ddriver.so", RTLD_NOW | RTLD_NOLOAD), "_ftext");
                fprintf(stderr, "___ftext=%x\n", ___ftext);
            } else {
                fprintf(stderr, "/usr/lib/libv3ddriver.so - not resident\n");
                v3ddriver_base = NULL;
            }


        } else {
            fprintf(stderr, "dladdr, failed\n");
        }
    }
#endif

#if 0
    if (NULL == ___ftext) {
        ___ftext = dlsym(RTLD_NEXT, "_ftext");
        if (___ftext == NULL) {
            fprintf(stderr, "Error in `dlsym`: %s : %s\n", dlerror(), "_ftext");
        } else {
            fprintf(stderr, "got ___ftext = 0x%x\n", ___ftext);

            Dl_info dli;
            if (dladdr(___ftext, &dli)) {
                fprintf(stderr, "dladdr, dli_fname[%s], dli_fbase[%x], dli_sname[%s], dli_saddr[%x]\n",
                    dli.dli_fname, dli.dli_fbase, dli.dli_sname, dli.dli_saddr);

                //const char *dli_fname;  /* Pathname of shared object that
                //void       *dli_fbase;  /* Address at which shared object
                //const char *dli_sname;  /* Name of nearest symbol with address
                //void       *dli_saddr;  /* Exact address of symbol named

                if (strstr(dli.dli_fname, "v3ddriver") == NULL) {
                    ___ftext = NULL;
                }

            } else {
                fprintf(stderr, "dladdr, failed\n");
            }
        }
    }
#endif

    unsigned int ra = 0;
#ifdef __mips__
    asm volatile("move %0, $ra" : "=r"(ra));
#endif

    void *ret = __malloc(size);

    if (libcmalloc) {

        if (size >= libcmalloc) {

            tditrace("m =%x,ra=%x,sz=%d", ret, ra, size);

#if 0
            if (size == 420)
                tditrace("420m =%x,ra=%x,sz=%d", ret, ra, size);
#endif

#if 0
            if (v3ddriver_base) {
                if ((ra >= v3ddriver_base) && (ra <= v3ddriver_base + 0x110000)) {
                    tditrace("v3dm =%x,ra=%x,sz=%d", ret, ra, size);
                }
            }
#endif
#if 0
            if (___ftext) {
                if ((ra == ___ftext + 0x3534) && (size >= 1024)) {
                    closelog();
                    tditrace("v3dftext1024m =%x,ra=%x,sz=%d", ret, ra, size);
                    fprintf(stderr, "v3dftextm %d\n", size);
                }
            }
#endif
#if 0
            if ((ra >= __new - 256) && (ra <= __new + 256)) {
                tditrace("newm =%x,ra=%x,sz=%d", ret, ra, size);

                if (size > 1024) {
                    tditrace("new1024m =%x,ra=%x,sz=%d", ret, ra, size);
                    closelog();
                }
            }
#endif
        }
    }

    return ret;
}
#endif

// our temporary calloc used until we get the address of libc provided
// one in our interposed calloc
static void *temporary_calloc(size_t x, size_t y) {
    // printf("empty calloc called\n");
    return NULL;
}

#if 1
extern "C" void *calloc(size_t nmemb, size_t size) {

    static void *(*__calloc)(size_t, size_t) = NULL;

    if (__calloc == NULL) {

        __calloc = temporary_calloc;

        __calloc = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
        if (NULL == __calloc) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    unsigned int ra = 0;
#ifdef __mips__
    asm volatile("move %0, $ra" : "=r"(ra));
#endif

    void *ret = __calloc(nmemb, size);

    if (libccalloc) {

        if ((nmemb * size) >= libccalloc) {
            tditrace("c =%x,ra=%x,sz=%d", ret, ra, nmemb * size);
        }
    }

    return ret;
}
#endif

#if 1
extern "C" void *realloc(void *ptr, size_t size) {
    static void *(*__realloc)(void *, size_t) = NULL;

    if (__realloc == NULL) {
        __realloc = (void *(*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
        if (NULL == __realloc) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    unsigned int ra = 0;
#ifdef __mips__
    asm volatile("move %0, $ra" : "=r"(ra));
#endif

    void *ret = __realloc(ptr, size);

    if (libcrealloc) {

        if (size >= libcrealloc) {
            tditrace("r =%x,ra=%x,sz=%d,ptr=%x", ret, ra, size, ptr);
        }
    }

    return ret;
}
#endif

#if 0
extern "C" void free(void *ptr) {
    static void (*__free)(void *) = NULL;

    if (__free == NULL) {
        __free = (void (*)(void *))dlsym(RTLD_NEXT, "free");
        if (NULL == __free) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        unsigned int ra = 0;
#ifdef __mips__
        asm volatile("move %0, $ra" : "=r"(ra));
#endif            
        tditrace("f ra=%p", ra);

        //tditrace("@I+free() 0x%x", ptr);
    }

    __free(ptr);

    if (libcrecording) {
        //tditrace("@I-free()");
    }
}
#endif

extern "C" int brk(void *__addr) {
    static int (*__brk)(void *) = NULL;
    if (__brk == NULL) {
        __brk = (int (*)(void *))dlsym(RTLD_NEXT, "brk");
        if (__brk == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+brk()");
    int ret = __brk(__addr);
    tditrace("@I-brk()");
    return ret;
}

extern "C" void *sbrk(intptr_t __delta) {
    static void *(*__sbrk)(intptr_t) = NULL;
    if (__sbrk == NULL) {
        __sbrk = (void *(*)(intptr_t))dlsym(RTLD_NEXT, "sbrk");
        if (__sbrk == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+sbrk()");
    void *ret = __sbrk(__delta);
    tditrace("@I-sbrk()");
    return ret;
}

#if 0
extern "C" void *mmap(void *__addr, size_t __len, int __prot, int __flags,
                      int __fd, __off_t __offset) {
    static void *(*__mmap)(void *, size_t, int, int, int, __off_t) = NULL;
    if (__mmap == NULL) {
        __mmap = (void *(*)(void *, size_t, int, int, int, __off_t))dlsym(
            RTLD_NEXT, "mmap");
        if (__mmap == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+mmap()");
    void *ret = __mmap(__addr, __len, __prot, __flags, __fd, __offset);
    tditrace("@I-mmap()");
    return ret;
}

extern "C" int munmap(void *__addr, size_t __len) {
    static int (*__munmap)(void *, size_t) = NULL;
    if (__munmap == NULL) {
        __munmap = (int (*)(void *, size_t))dlsym(RTLD_NEXT, "munmap");
        if (__munmap == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+munmap()");
    int ret = __munmap(__addr, __len);
    tditrace("@I-munmap()");
    return ret;
}
#endif

#if 0
extern "C" void *pvalloc(size_t __size) {
    static void *(*__pvalloc)(size_t) = NULL;
    if (__pvalloc == NULL) {
        __pvalloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "pvalloc");
        if (__pvalloc == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+pvalloc()");
    void *ret = __pvalloc(__size);
    tditrace("@I-pvalloc()");
    return ret;
}

extern "C" void *aligned_alloc(size_t __alignment, size_t __size) {
    static void *(*__aligned_alloc)(size_t, size_t) = NULL;
    if (__aligned_alloc == NULL) {
        __aligned_alloc =
            (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "aligned_alloc");
        if (__aligned_alloc == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+aligned_alloc()");
    void *ret = __aligned_alloc(__alignment, __size);
    tditrace("@I-aligned_alloc()");
    return ret;
}

extern "C" void *valloc(size_t __size) {
    static void *(*__valloc)(size_t) = NULL;
    if (__valloc == NULL) {
        __valloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "valloc");
        if (__valloc == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+valloc()");
    void *ret = __valloc(__size);
    tditrace("@I-valloc()");
    return ret;
}

extern "C" void *memalign(size_t __alignment, size_t __size) {
    static void *(*__memalign)(size_t, size_t) = NULL;
    if (__memalign == NULL) {
        __memalign = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "memalign");
        if (__memalign == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+memalign()");
    void *ret = __memalign(__alignment, __size);
    tditrace("@I-memalign()");
    return ret;
}

extern "C" int posix_memalign(void **__memptr, size_t __alignment,
                              size_t __size) {
    static int (*__posix_memalign)(void **, size_t, size_t) = NULL;
    if (__posix_memalign == NULL) {
        __posix_memalign = (int (*)(void **, size_t, size_t))dlsym(
            RTLD_NEXT, "posix_memalign");
        if (__posix_memalign == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+posix_memalign()");
    int ret = __posix_memalign(__memptr, __alignment, __size);
    tditrace("@I-posix_memalign()");
    return ret;
}
#endif

#if 0
extern "C" int mprotect(void *__addr, size_t __len, int __prot) {
    static int (*__mprotect)(void *, size_t, int) = NULL;
    if (__mprotect == NULL) {
        __mprotect = (int (*)(void *, size_t, int))dlsym(RTLD_NEXT, "mprotect");
        if (__mprotect == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+mprotect()");
    int ret = __mprotect(__addr, __len, __prot);
    tditrace("@I-mprotect()");
    return ret;
}
#endif

#if 0
extern "C" int msync(void *__addr, size_t __len, int __flags) {
    static int (*__msync)(void *, size_t, int) = NULL;
    if (__msync == NULL) {
        __msync = (int (*)(void *, size_t, int))dlsym(RTLD_NEXT, "msync");
        if (__msync == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+msync()");
    int ret = __msync(__addr, __len, __flags);
    tditrace("@I-msync()");
    return ret;
}
#endif

#if 0
extern "C" int madvise(void *__addr, size_t __len, int __advice) {
    static int (*__madvise)(void *, size_t, int) = NULL;
    if (__madvise == NULL) {
        __madvise = (int (*)(void *, size_t, int))dlsym(RTLD_NEXT, "madvise");
        if (__madvise == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace("@I+madvise()");
    int ret = __madvise(__addr, __len, __advice);
    tditrace("@I-madvise()");
    return ret;
}
#endif

#if 0

#include <new>
#include <stdexcept>

// Regular scalar new
void* operator new(std::size_t n) throw(std::bad_alloc)
{
    using namespace std;

    for (;;) {
        void* allocated_memory = ::operator new(n, nothrow);
        if (allocated_memory != 0) return allocated_memory;

        // Store the global new handler
        new_handler global_handler = set_new_handler(0);
        set_new_handler(global_handler);

        if (global_handler) {
            global_handler();
        } else {
            throw bad_alloc();
        }
    }
}

#endif

//_Znwj
#if 0
void* operator new(unsigned int i){

    unsigned int ra = 0;
#ifdef __mips__
    asm volatile("move %0, $ra" : "=r"(ra));
#endif

    void* ret = malloc(i);

    tditrace("operator_new =%x,ra=%x,sz=%d", ret, ra, i);

    return ret;
}
#endif

#if 0
extern "C" void syslog(int pri, const char *fmt, ...) {
    static void (*__syslog)(int, const char *, ...) = NULL;

    if (__syslog == NULL) {
        __syslog = (void (*)(int, const char *, ...))dlsym(RTLD_NEXT, "syslog");
        if (NULL == __syslog) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    va_list args;
    va_start(args, fmt);
    int a1 = va_arg(args, int);
    int a2 = va_arg(args, int);
    int a3 = va_arg(args, int);
    va_end(args);

    if (libcrecording || libcsyslogrecording) {
        tditrace("@S+syslog() %s", a1 ? (char *)a1 : "");
    }

    __syslog(pri, a1, a2, a3);
}
#endif
