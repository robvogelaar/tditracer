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

    // tditrace_ex("dlopen() %s", filename);

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

    tditrace_ex("connect()");

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

    if (libcrecording && libcopenrecording) {
        tditrace_ex("@A+open() %s", pathname);
    }

    int ret = __open(pathname, flags, a1);

    if (libcrecording && libcopenrecording) {
        tditrace_ex("@A-open() =%d", pathname, ret);
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

    if (libcrecording && libcfopenrecording) {
        tditrace_ex("@A+fopen() %s", path);
    }

    FILE *ret = __fopen(path, mode);

    if (libcrecording && libcfopenrecording) {
        tditrace_ex("@A-fopen() =%d", ret);
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

    if (libcrecording && libcfdopenrecording) {
        tditrace_ex("@A+fdopen() %d", fd);
    }

    FILE *ret = __fdopen(fd, mode);

    if (libcrecording && libcfdopenrecording) {
        tditrace_ex("@A-fdopen() =%d", ret);
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

    if (libcrecording && libcfreopenrecording) {
        tditrace_ex("@A+freopen() %s", path);
    }

    FILE *ret = __freopen(path, mode, stream);

    if (libcrecording && libcfreopenrecording) {
        tditrace_ex("@A-freopen() =%d", ret);
    }

    return ret;
}

#if 1
extern "C" ssize_t read(int fd, void *buf, size_t count) {
    static ssize_t (*__read)(int, void *, size_t) = NULL;

    if (__read == NULL) {
        __read = (ssize_t (*)(int, void *, size_t))dlsym(RTLD_NEXT, "read");
        if (NULL == __read) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording && libcreadrecording) {
        tditrace_ex("@A+read() %d %d", fd, count);
    }

    ssize_t ret = __read(fd, buf, count);

    if (libcrecording && libcreadrecording) {
        if (ret == -1) {
            tditrace_ex("@A-read() =-1");
        } else if (ret == 0) {
            tditrace_ex("@A-read() =0");
        } else {
            if (0) {
                char s[MAXSTRLEN + 1];
                strncpy(s, (const char *)buf, MIN(MAXSTRLEN, ret));
                s[MIN(MAXSTRLEN, ret)] = '\0';
                tditrace_ex("@A-read() =%d \"%s\"", ret, s);
            } else {
                tditrace_ex("@A-read() =%d", ret);
            }
        }
    }

    return ret;
}
#endif

#if 1
extern "C" ssize_t write(int fd, const void *buf, size_t count) {
    static ssize_t (*__write)(int, const void *, size_t) = NULL;

    if (__write == NULL) {
        __write =
            (ssize_t (*)(int, const void *, size_t))dlsym(RTLD_NEXT, "write");
        if (NULL == __write) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

#if 1
    if (libcrecording && libcwriterecording) {
        if (0) {
            char s[MAXSTRLEN + 1];
            strncpy(s, (const char *)buf, MIN(MAXSTRLEN, count));
            s[MIN(MAXSTRLEN, count)] = '\0';
            tditrace_ex("@A+write() %d %d \"%s\"", fd, count, s);
        } else {
            tditrace_ex("@A+write() %d %d", fd, count);
        }
    }
#endif

    ssize_t ret = __write(fd, buf, count);

#if 1
    if (libcrecording && libcwriterecording) {
        tditrace_ex("@A-write() =%d", ret);
    }
#endif

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

    if (libcrecording && libcsocketrecording) {
        tditrace_ex("@A+socket() %d %d %d", domain, type, protocol);
    }

    int ret = __socket(domain, type, protocol);

    if (libcrecording && libcsocketrecording) {
        tditrace_ex("@A-socket() =%d", ret);
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

    if (libcrecording && libcsendrecording) {
        if (MAXSTRLEN) {

            if (buf && !strchr((char *)buf, '\f')) {
                char s[MAXSTRLEN + 1];
                strncpy(s, (const char *)buf, MIN(MAXSTRLEN, len));
                s[MIN(MAXSTRLEN, len)] = '\0';

                if (strncmp((const char *)buf, "HTTP", 4) == 0 ||
                    strncmp((const char *)buf, "{", 1) == 0 ||
                    strncmp((const char *)buf, "data:", 5) == 0) {
                    tditrace_ex("@E+send()_%d %d \"%s\"", sockfd, len, s);
                    tditrace_ex("@A+send() %d %d \"%s\"", sockfd, len, s);
                } else {
                    s[MIN(16, len)] = '\0';
                    tditrace_ex("@A+send() %d %d \"%s\"...", sockfd, len, s);
                }

            } else {
                tditrace_ex("@A+send() %d %d \"???\"", sockfd, len);
            }

        } else {
            tditrace_ex("@A+send() %d %d ", sockfd, len);
        }
    }

    ssize_t ret = __send(sockfd, buf, len, flags);

    if (libcrecording && libcsendrecording) {
        tditrace_ex("@A-send() =%d", ret);
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

    if (libcrecording && libcsendtorecording) {
        if (MAXSTRLEN) {
            char s[MAXSTRLEN + 1];
            strncpy(s, (const char *)buf, MIN(MAXSTRLEN, len));
            s[MIN(MAXSTRLEN, len)] = '\0';
            tditrace_ex("@A+sendto() %d %d \"%s\"", sockfd, len, s);
        } else {
            tditrace_ex("@A+sendto() %d %d ", sockfd, len);
        }
    }

    ssize_t ret = __sendto(sockfd, buf, len, flags, dest_addr, addrlen);

    if (libcrecording && libcsendtorecording) {
        tditrace_ex("@A-sendto() =%d", ret);
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

    if (libcrecording && libcsendmsgrecording) {
        // tditrace_ex("@A+sendmsg() %d %d \"%s\"", sockfd, msg->msg_iovlen,
        // msg->msg_iov[0].iov_base);
        tditrace_ex("@A+sendmsg() %d %d \"\"", sockfd, msg->msg_iovlen);
    }

    ssize_t ret = __sendmsg(sockfd, msg, flags);

    if (libcrecording && libcsendmsgrecording) {
        tditrace_ex("@A-sendmsg() =%d", ret);
        tditrace_ex("@E+sendmsg()_%d =%d %d \"\"", sockfd, ret,
                    msg->msg_iovlen);
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

    if (libcrecording && libcsendmmsgrecording) {
        tditrace_ex("@A+sendmmsg() %d 0x%x", sockfd, msgvec);
    }

    int ret = __sendmmsg(sockfd, msgvec, vlen, flags);

    if (libcrecording && libcsendmmsgrecording) {
        tditrace_ex("@A-sendmmsg() =%d", ret);
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

    if (libcrecording && libcrecvrecording) {
        tditrace_ex("@A+recv() %d %d", sockfd, len);
    }

    ssize_t ret = __recv(sockfd, buf, len, flags);

    if (libcrecording && libcrecvrecording) {
        if (ret == -1) {
            tditrace_ex("@A-recv() =-1");
        } else if (ret == 0) {
            tditrace_ex("@A-recv() =0");
        } else {
            if (MAXSTRLEN) {
                if (buf && !strchr((char *)buf, '\f')) {
                    char s[MAXSTRLEN + 1];
                    strncpy(s, (const char *)buf, MIN(MAXSTRLEN, ret));
                    s[MIN(MAXSTRLEN, ret)] = '\0';

                    if (strncmp((const char *)buf, "GET", 3) == 0 ||
                        strncmp((const char *)buf, "POST", 4) == 0 ||
                        strncmp((const char *)buf, "{", 1) == 0) {
                        tditrace_ex("@E+recv()_%d =%d \"%s\"", sockfd, ret, s);
                        tditrace_ex("@A-recv() =%d \"%s\"", ret, s);
                    } else {
                        s[MIN(16, len)] = '\0';
                        tditrace_ex("@A-recv() =%d \"%s\"...", ret, s);
                    }

                } else {
                    tditrace_ex("@A-recv() =%d \"???\"", ret);
                }
            } else {
                tditrace_ex("@A-recv() =%d", ret);
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

    if (libcrecording && libcrecvfromrecording) {
        tditrace_ex("@A+recvfrom() %d %d 0x%x", sockfd, len, flags);
    }

    ssize_t ret = __recvfrom(sockfd, buf, len, flags, src_addr, addrlen);

    if (libcrecording && libcrecvfromrecording) {
        if (ret == -1) {
            tditrace_ex("@A-recvfrom() =-1");
        } else if (ret == 0) {
            tditrace_ex("@A-recvfrom() =0");
        } else {
            if (MAXSTRLEN) {
                char s[MAXSTRLEN + 1];
                strncpy(s, (const char *)buf, MIN(MAXSTRLEN, ret));
                s[MIN(MAXSTRLEN, ret)] = '\0';
                tditrace_ex("@A-recvfrom() =%d \"%s\"", ret, s);
            } else {
                tditrace_ex("@A-recvfrom() =%d", ret);
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

    if (libcrecording && libcrecvmsgrecording) {
        tditrace_ex("@A+recvmsg() %d %d", sockfd, msg->msg_iovlen);
    }

    ssize_t ret = __recvmsg(sockfd, msg, flags);

    if (libcrecording && libcrecvmsgrecording) {
        // tditrace_ex("@A+recvmsg() %d %d \"%s\"", sockfd, msg->msg_iovlen,
        // msg->msg_iov[0].iov_base);
        tditrace_ex("@A-recvmsg() =%d \"\"", ret);
        tditrace_ex("@E+recvmsg()_%d =%d \"\"", sockfd, ret);
    }

    return ret;
}

extern "C" int recvmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
                        int flags, const timespec *timeout) {
    static int (*__recvmmsg)(int, struct mmsghdr *, unsigned int, int,
                             const timespec *) = NULL;

    if (__recvmmsg == NULL) {
        __recvmmsg = (int (*)(int, struct mmsghdr *, unsigned int, int,
                              const timespec *))dlsym(RTLD_NEXT, "recvmmsg");
        if (NULL == __recvmmsg) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording && libcrecvmmsgrecording) {
        tditrace_ex("@A+recvmmsg() %d 0x%x %d", sockfd, msgvec, vlen);
    }

    int ret = __recvmmsg(sockfd, msgvec, vlen, flags, timeout);

    if (libcrecording && libcrecvmmsgrecording) {
        tditrace_ex("@A-recvmmsg() =%d", ret);
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

    if (libcrecording && libcselectrecording) {
        tditrace_ex("@A+select() %d %x %x %x", nfds, readfds, writefds,
                    exceptfds);
    }

    int ret = __select(nfds, readfds, writefds, exceptfds, timeout);

    if (libcrecording && libcselectrecording) {
        tditrace_ex("@A-select() =%d", ret);
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

    if (libcrecording && libcpollrecording) {
        tditrace_ex("@A+poll() %x %d %d", fds, nfds, timeout);
    }

    int ret = __poll(fds, nfds, timeout);

    if (libcrecording && libcpollrecording) {
        tditrace_ex("@A-poll() =%d", ret);
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

    if (libcrecording && libcioctlrecording) {
        tditrace_ex("@A+ioctl() %d 0x%x", d, request);
    }

    int ret = __ioctl(d, request, a1);

    if (libcrecording && libcioctlrecording) {
        tditrace_ex("@A-ioctl() =%d", ret);
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
        tditrace_ex("@A+memcpy() 0x%x 0x%x %d", dest, src, n);
    }

    void *ret = __memcpy(dest, src, n);

    if (libcrecording) {
        tditrace_ex("@A-memcpy()");
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
        tditrace_ex("@A+memset() 0x%x,0x%x,%d", dest, c, n);
    }

    void *ret = __memset(dest, c, n);

    if (libcrecording) {
        tditrace_ex("@A-memset()");
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
        tditrace_ex("@A+strcpy() 0x%x,0x%x", dest, src);
    }

    char *ret = __strcpy(dest, src);

    if (libcrecording) {
        tditrace_ex("@A-strcpy()");
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
        tditrace_ex("@A+strncpy() 0x%x 0x%x %d", dest, src, n);
    }

    char *ret = __strncpy(dest, src, n);

    if (libcrecording) {
        tditrace_ex("@A-strncpy()");
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

    //# if 0
    //if (libcrecording) {
    //    if (size >= 1024) {
    //        unsigned int ra = 0;
    //        #ifdef __mips__
    //        asm volatile("move %0, $ra" : "=r"(ra));
    //        #endif
    //        // tditrace_ex("@A+malloc() %d %p", size, ra);
    //        tditrace_ex("m ra=%p,sz=%d", ra, size);
    //    }
    //}
    //#endif


    void *ret = __malloc(size);

    if (libcrecording) {
        if (size >= 128) {
            unsigned int ra = 0;
            #ifdef __mips__
            asm volatile("move %0, $ra" : "=r"(ra));
            #endif
            // tditrace_ex("@A+malloc() %d %p", size, ra);

            tditrace_ex("m =%x,ra=%p,sz=%d", ret, ra, size);
        }
    }


    #if 0
    if (libcrecording) {
        // tditrace_ex("@A-malloc() =0x%x", ret);
        if (0) {//size >= 1024) {
            unsigned int ra = 0;
            #ifdef __mips__
            asm volatile("move %0, $ra" : "=r"(ra));
            #endif
            tditrace_ex("m =%x,ra=%p,sz=%d", ret, ra, size);
        }
    }
    #endif

    return ret;
}
#endif

#if 0
extern "C" void *calloc(size_t nmemb, size_t size) {
    static void *(*__calloc)(size_t, size_t) = NULL;

    if (__calloc == NULL) {
        __calloc = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
        if (NULL == __calloc) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {

        if (size >= 1024) {
            unsigned int ra = 0;
            #ifdef __mips__
            asm volatile("move %0, $ra" : "=r"(ra));
            #endif
            tditrace_ex("c ra=%p,sz=%d", ra, size);
        }
        //tditrace_ex("@A+calloc() %d %d", nmemb, size);
    }

    void *ret = __calloc(nmemb, size);

    if (libcrecording) {
        // tditrace_ex("@A-calloc() =%x", ret);
    }

    return ret;
}
#endif

#if 1
void *realloc(void *ptr, size_t size)
{
    static void *(*__realloc)(void *, size_t) = NULL;

    if (__realloc == NULL) {
        __realloc = (void *(*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
        if (NULL == __realloc) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        if (0) { //size >= 1024) {
            unsigned int ra = 0;
            #ifdef __mips__
            asm volatile("move %0, $ra" : "=r"(ra));
            #endif
            tditrace_ex("r ra=%p,sz=%d", ra, size);
        }
        // tditrace_ex("@A+realloc() %d %p", size, ra);
    }

    void *ret = __realloc(ptr, size);

    if (libcrecording) {
        // tditrace_ex("@A-realloc() =0x%x", ret);

        if (1) { //size >= 1024) {
            unsigned int ra = 0;
            #ifdef __mips__
            asm volatile("move %0, $ra" : "=r"(ra));
            #endif
            tditrace_ex("r =%x,ra=%p,sz=%d", ret, ra, size);
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
        tditrace_ex("f ra=%p", ra);

        //tditrace_ex("@A+free() 0x%x", ptr);
    }

    __free(ptr);

    if (libcrecording) {
        //tditrace_ex("@A-free()");
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
    tditrace_ex("@A+brk()");
    int ret = __brk(__addr);
    tditrace_ex("@A-brk()");
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
    tditrace_ex("@A+sbrk()");
    void *ret = __sbrk(__delta);
    tditrace_ex("@A-sbrk()");
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
    tditrace_ex("@A+mmap()");
    void *ret = __mmap(__addr, __len, __prot, __flags, __fd, __offset);
    tditrace_ex("@A-mmap()");
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
    tditrace_ex("@A+munmap()");
    int ret = __munmap(__addr, __len);
    tditrace_ex("@A-munmap()");
    return ret;
}
#endif

extern "C" void *pvalloc(size_t __size) {
    static void *(*__pvalloc)(size_t) = NULL;
    if (__pvalloc == NULL) {
        __pvalloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "pvalloc");
        if (__pvalloc == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace_ex("@A+pvalloc()");
    void *ret = __pvalloc(__size);
    tditrace_ex("@A-pvalloc()");
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
    tditrace_ex("@A+aligned_alloc()");
    void *ret = __aligned_alloc(__alignment, __size);
    tditrace_ex("@A-aligned_alloc()");
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
    tditrace_ex("@A+valloc()");
    void *ret = __valloc(__size);
    tditrace_ex("@A-valloc()");
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
    tditrace_ex("@A+memalign()");
    void *ret = __memalign(__alignment, __size);
    tditrace_ex("@A-memalign()");
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
    tditrace_ex("@A+posix_memalign()");
    int ret = __posix_memalign(__memptr, __alignment, __size);
    tditrace_ex("@A-posix_memalign()");
    return ret;
}

#if 0
extern "C" int mprotect(void *__addr, size_t __len, int __prot) {
    static int (*__mprotect)(void *, size_t, int) = NULL;
    if (__mprotect == NULL) {
        __mprotect = (int (*)(void *, size_t, int))dlsym(RTLD_NEXT, "mprotect");
        if (__mprotect == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace_ex("@A+mprotect()");
    int ret = __mprotect(__addr, __len, __prot);
    tditrace_ex("@A-mprotect()");
    return ret;
}
#endif

extern "C" int msync(void *__addr, size_t __len, int __flags) {
    static int (*__msync)(void *, size_t, int) = NULL;
    if (__msync == NULL) {
        __msync = (int (*)(void *, size_t, int))dlsym(RTLD_NEXT, "msync");
        if (__msync == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace_ex("@A+msync()");
    int ret = __msync(__addr, __len, __flags);
    tditrace_ex("@A-msync()");
    return ret;
}

#if 0
extern "C" int madvise(void *__addr, size_t __len, int __advice) {
    static int (*__madvise)(void *, size_t, int) = NULL;
    if (__madvise == NULL) {
        __madvise = (int (*)(void *, size_t, int))dlsym(RTLD_NEXT, "madvise");
        if (__madvise == NULL) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        }
    }
    tditrace_ex("@A+madvise()");
    int ret = __madvise(__addr, __len, __advice);
    tditrace_ex("@A-madvise()");
    return ret;
}
#endif