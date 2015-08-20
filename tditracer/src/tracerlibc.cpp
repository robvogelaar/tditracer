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

    if (libcrecording) {
        tditrace_ex("@A+open() %s", pathname);
    }

    int ret = __open(pathname, flags, a1);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+fopen() %s", path);
    }

    FILE *ret = __fopen(path, mode);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+fdopen() %d", fd);
    }

    FILE *ret = __fdopen(fd, mode);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+freopen() %s", path);
    }

    FILE *ret = __freopen(path, mode, stream);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+read() %d %d", fd, count);
    }

    ssize_t ret = __read(fd, buf, count);

    if (libcrecording) {
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
    if (libcrecording) {
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
    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+socket() %d %d %d", domain, type, protocol);
    }

    int ret = __socket(domain, type, protocol);

    if (libcrecording) {
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

    if (libcrecording) {
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

    if (libcrecording) {
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

    if (libcrecording) {
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

    if (libcrecording) {
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

    if (libcrecording) {
        // tditrace_ex("@A+sendmsg() %d %d \"%s\"", sockfd, msg->msg_iovlen,
        // msg->msg_iov[0].iov_base);
        tditrace_ex("@A+sendmsg() %d %d \"\"", sockfd, msg->msg_iovlen);
    }

    ssize_t ret = __sendmsg(sockfd, msg, flags);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+sendmmsg() %d 0x%x", sockfd, msgvec);
    }

    int ret = __sendmmsg(sockfd, msgvec, vlen, flags);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+recv() %d %d", sockfd, len);
    }

    ssize_t ret = __recv(sockfd, buf, len, flags);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+recvfrom() %d %d 0x%x", sockfd, len, flags);
    }

    ssize_t ret = __recvfrom(sockfd, buf, len, flags, src_addr, addrlen);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+recvmsg() %d %d", sockfd, msg->msg_iovlen);
    }

    ssize_t ret = __recvmsg(sockfd, msg, flags);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+recvmmsg() %d 0x%x %d", sockfd, msgvec, vlen);
    }

    int ret = __recvmmsg(sockfd, msgvec, vlen, flags, timeout);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+select() %d %x %x %x", nfds, readfds, writefds,
                    exceptfds);
    }

    int ret = __select(nfds, readfds, writefds, exceptfds, timeout);

    if (libcrecording) {
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

    if (libcrecording) {
        tditrace_ex("@A+poll() %x %d", fds, nfds);
    }

    int ret = __poll(fds, nfds, timeout);

    if (libcrecording) {
        tditrace_ex("@A-poll() =%d", ret);
    }

    return ret;
}

#if 1
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
#endif

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

#if 0
extern "C" void *malloc(size_t size) {
    static void *(*__malloc)(size_t) = NULL;

    if (__malloc == NULL) {
        __malloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
        if (NULL == __malloc) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (libcrecording) {
        tditrace_ex("@A+malloc() %d", size);
    }

    void *ret = __malloc(size);

    if (libcrecording) {
        tditrace_ex("@A-malloc()");
    }

    return ret;
}
#endif
