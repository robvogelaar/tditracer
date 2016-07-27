#include <dlfcn.h>
#include <fcntl.h>
#include <malloc.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tdi.h"
#include "tracermain.h"

#define MAXSTRLEN 128
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#ifdef __mips__
#define save_ra() \
  int ra;         \
  asm volatile("move %0, $ra" : "=r"(ra));
#else
#define save_ra() int ra = 0;
#endif

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
extern "C" int open(const char* pathname, int flags, ...) {
  save_ra();
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

  if (libcopenrecording) {
    // tditrace("@I+open() %s", pathname);
    tditrace("@E+open() %s%n", pathname, ra);
  }

  int ret = __open(pathname, flags, a1);

  if (libcopenrecording) {
    // tditrace("@I-open() =%d", pathname, ret);
  }

  return ret;
}
#endif

extern "C" FILE* fopen(const char* path, const char* mode) {
  save_ra();
  static FILE* (*__fopen)(const char*, const char*) = NULL;

  if (__fopen == NULL) {
    __fopen = (FILE * (*)(const char*, const char*))dlsym(RTLD_NEXT, "fopen");
    if (NULL == __fopen) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcfopenrecording) {
    // tditrace("@I+fopen() %s", path);
    tditrace("@E+fopen() %s%n", path, ra);
  }

  FILE* ret = __fopen(path, mode);

  if (libcfopenrecording) {
    // tditrace("@I-fopen() =%d", ret);
  }

  return ret;
}

extern "C" FILE* fdopen(int fd, const char* mode) {
  save_ra();
  static FILE* (*__fdopen)(int, const char*) = NULL;

  if (__fdopen == NULL) {
    __fdopen = (FILE * (*)(int, const char*))dlsym(RTLD_NEXT, "fdopen");
    if (NULL == __fdopen) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcfdopenrecording) {
    // tditrace("@I+fdopen() %d", fd);
    tditrace("@E+fdopen() %d%n", fd, ra);
  }

  FILE* ret = __fdopen(fd, mode);

  if (libcfdopenrecording) {
    // tditrace("@I-fdopen() =%d", ret);
  }

  return ret;
}

extern "C" FILE* freopen(const char* path, const char* mode, FILE* stream) {
  save_ra();
  static FILE* (*__freopen)(const char*, const char*, FILE*) = NULL;

  if (__freopen == NULL) {
    __freopen = (FILE * (*)(const char*, const char*, FILE*))dlsym(RTLD_NEXT,
                                                                   "freopen");
    if (NULL == __freopen) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcfreopenrecording) {
    // tditrace("@I+freopen() %s", path);
    tditrace("@E+freopen() %s%n", path, ra);
  }

  FILE* ret = __freopen(path, mode, stream);

  if (libcfreopenrecording) {
    // tditrace("@I-freopen() =%d", ret);
  }

  return ret;
}

#if 1
extern "C" int socket(int domain, int type, int protocol) {
  static int (*__socket)(int, int, int) = NULL;

  if (__socket == NULL) {
    __socket = (int (*)(int, int, int))dlsym(RTLD_NEXT, "socket");
    if (NULL == __socket) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcsocketrecording) {
    tditrace("@I+socket() %d %d %d", domain, type, protocol);
  }

  int ret = __socket(domain, type, protocol);

  if (libcsocketrecording) {
    tditrace("@I-socket() =%d", ret);
  }

  return ret;
}
#endif

char* StrStr(const char* str, const char* target) {
  if (!*target) return NULL;
  char* p1 = (char*)str;
  while (*p1) {
    char *p1Begin = p1, *p2 = (char *)target;
    while (*p1 && *p2 && (*p1 == *p2 || *p2 == '!')) {
      p1++;
      p2++;
    }
    if (!*p2) return p1Begin;
    p1 = p1Begin + 1;
  }
  return NULL;
}

#if 1
extern "C" ssize_t write(int fd, const void* buf, size_t count) {
  save_ra();
  static ssize_t (*__write)(int, const void*, size_t) = NULL;

  if (__write == NULL) {
    __write = (ssize_t(*)(int, const void*, size_t))dlsym(RTLD_NEXT, "write");
    if (NULL == __write) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcwritematch) {
    if (StrStr((const char*)buf, libcwritematch)) {
      char s[MAXSTRLEN + 1];
      strncpy(s, (const char*)buf, MIN(MAXSTRLEN, count));
      s[MIN(MAXSTRLEN, count)] = '\0';
      int i;
      for (i = 0; i < MIN(MAXSTRLEN, count); i++)
        if (s[i] < 0x20 || s[i] >= 0x7f) s[i] = '.';
      tditrace("@E+write():\"%s\" \"%s\"", libcwritematch,
               (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
    }
  }

  if (libcwriterecording) {
    // tditrace("@I+write() %d %d", fd, count);

    if (MAXSTRLEN) {
      if (buf) {
        char s[MAXSTRLEN + 1];
        strncpy(s, (const char*)buf, MIN(MAXSTRLEN, count));
        s[MIN(MAXSTRLEN, count)] = '\0';

        if (strncmp((const char*)buf, "GET", 3) == 0) {
          if (libcfd)
            tditrace("@E+write()_%d_GET %d \"%s\"%n", fd, count, s, ra);
          else
            tditrace("@E+write()_GET %d %d \"%s\"%n", fd, count, s, ra);

        } else if (strncmp((const char*)buf, "PUT", 3) == 0) {
          if (libcfd)
            tditrace("@E+write()_%d_PUT %d \"%s\"%n", fd, count, s, ra);
          else
            tditrace("@E+write()_PUT %d %d \"%s\"%n", fd, count, s, ra);
        } else if (strncmp((const char*)buf, "POST", 4) == 0) {
          if (libcfd)
            tditrace("@E+write()_%d_POST %d \"%s\"%n", fd, count, s, ra);
          else
            tditrace("@E+write()_POST %d %d \"%s\"%n", fd, count, s, ra);
        } else if (strncmp((const char*)buf, "{\"", 2) == 0) {
          if (libcfd)
            tditrace("@E+write()_%d_{ %d \"%s\"%n", fd, count, s, ra);
          else
            tditrace("@E+write()_{ %d %d \"%s\"%n", fd, count, s, ra);
        } else if (strncmp((const char*)buf + 2, "xml", 3) == 0) {
          if (libcfd)
            tditrace("@E+write()_%d_xml %d \"%s\"%n", fd, count, s, ra);
          else
            tditrace("@E+write()_xml %d %d \"%s\"%n", fd, count, s, ra);
        } else {
          s[MIN(1, count)] = '\0';
          if (libcfd)
            tditrace("@E+write()_%d %d \"%s...\"%n", fd, count,
                     (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
          else
            tditrace("@E+write() %d %d \"%s...\"%n", fd, count,
                     (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
          // tditrace("@E+write()_%d_? %d \"%s...\"", fd, count, s);
        }

      } else {
        if (libcfd)
          tditrace("@E+write()_%d %d%n", fd, count, ra);
        else
          tditrace("@E+write() %d %d%n", fd, count, ra);
      }

    } else {
      tditrace("@E+write()_%d %d%n", fd, count, ra);
    }
  }

  ssize_t ret = __write(fd, buf, count);

  if (libcwriterecording) {
    // tditrace("@I-write() =%d", ret);
  }

  return ret;
}
#endif

#if 1
extern "C" ssize_t read(int fd, void* buf, size_t count) {
  save_ra();
  static ssize_t (*__read)(int, void*, size_t) = NULL;

  if (__read == NULL) {
    __read = (ssize_t(*)(int, void*, size_t))dlsym(RTLD_NEXT, "read");
    if (NULL == __read) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcreadrecording) {
    // tditrace("@I+read() %d %d", fd, count);
  }

  ssize_t ret = __read(fd, buf, count);

  if (libcreadmatch) {
    if (ret != -1) {
      if (StrStr((const char*)buf, libcreadmatch)) {
        char s[MAXSTRLEN + 1];
        strncpy(s, (const char*)buf, MIN(MAXSTRLEN, ret));
        s[MIN(MAXSTRLEN, ret)] = '\0';
        int i;
        for (i = 0; i < MIN(MAXSTRLEN, ret); i++)
          if (s[i] < 0x20 || s[i] >= 0x7f) s[i] = '.';
        tditrace("@E+read():\"%s\" \"%s\"", libcreadmatch,
                 (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
      }
    }
  }

  if (libcreadrecording) {
    if (ret == -1) {
      // tditrace("@I-read()) =-1");
      if (libcfd)
        tditrace("@E+read()_%d %d =-1%n", fd, count, ra);
      else
        tditrace("@E+read() %d %d =-1%n", fd, count, ra);
    } else if (ret == 0) {
      // tditrace("@I-read() =0");
      if (libcfd)
        tditrace("@E+read()_%d %d =0%n", fd, count, ra);
      else
        tditrace("@E+read() %d %d =0%n", fd, count, ra);
    } else if (count == 64) {
      // ignore the read from /proc/pid/statm
    } else {
      if (MAXSTRLEN) {
        if (buf) {
          char s[MAXSTRLEN + 1];
          strncpy(s, (const char*)buf, MIN(MAXSTRLEN, ret));
          s[MIN(MAXSTRLEN, ret)] = '\0';

          if (strncmp((const char*)buf, "HTTP", 4) == 0) {
            if (libcfd)
              tditrace("@E+read()_%d_HTTP %d =%d \"%s\"%n", fd, count, ret, s,
                       ra);
            else
              tditrace("@E+read()_HTTP %d %d =%d \"%s\"%n", fd, count, ret, s,
                       ra);
          } else {
            int i;
            for (i = 0; i < MIN(MAXSTRLEN, ret); i++)
              if (s[i] < 0x20 || s[i] >= 0x7f) s[i] = '.';

            if (libcfd)
              tditrace("@E+read()_%d %d =%d \"%s\"%n", fd, count, ret,
                       (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
            else
              tditrace("@E+read() %d %d =%d \"%s\"", fd, count, ret,
                       (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
          }
        }
      }
    }
    // tditrace("@I-read() =%d", ret);
  }

  return ret;
}
#endif

#if 1
extern "C" ssize_t send(int sockfd, const void* buf, size_t len, int flags) {
  save_ra();
  static ssize_t (*__send)(int, const void*, size_t, int) = NULL;

  if (__send == NULL) {
    __send =
        (ssize_t(*)(int, const void*, size_t, int))dlsym(RTLD_NEXT, "send");
    if (NULL == __send) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcsendmatch) {
    if (StrStr((const char*)buf, libcsendmatch)) {
      char s[MAXSTRLEN + 1];
      strncpy(s, (const char*)buf, MIN(MAXSTRLEN, len));
      s[MIN(MAXSTRLEN, len)] = '\0';
      int i;
      for (i = 0; i < MIN(MAXSTRLEN, len); i++)
        if (s[i] < 0x20 || s[i] >= 0x7f) s[i] = '.';
      tditrace("@E+send():\"%s\" \"%s\"", libcsendmatch,
               (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
    }
  }

  if (libcrecording || libcsendrecording) {
    // tditrace("@I+send() %d %d", sockfd, len);

    if (MAXSTRLEN) {
      if (buf) {
        char s[MAXSTRLEN + 1];
        strncpy(s, (const char*)buf, MIN(MAXSTRLEN, len));
        s[MIN(MAXSTRLEN, len)] = '\0';

        if (strncmp((const char*)buf, "GET", 3) == 0) {
          if (libcfd)
            tditrace("@E+send()_%d_GET %d \"%s\"%n", sockfd, len, s, ra);
          else
            tditrace("@E+send()_GET %d %d \"%s\"%n", sockfd, len, s, ra);
        } else if (strncmp((const char*)buf, "PUT", 3) == 0) {
          if (libcfd)
            tditrace("@E+send()_%d_PUT %d \"%s\"%n", sockfd, len, s, ra);
          else
            tditrace("@E+send()_PUT %d %d \"%s\"%n", sockfd, len, s, ra);
        } else if (strncmp((const char*)buf, "POST", 4) == 0) {
          if (libcfd)
            tditrace("@E+send()_%d_POST %d \"%s\"%n", sockfd, len, s, ra);
          else
            tditrace("@E+send()_POST %d %d \"%s\"%n", sockfd, len, s, ra);
        } else if (strncmp((const char*)buf, "{\"", 2) == 0) {
          if (libcfd)
            tditrace("@E+send()_%d_{ %d \"%s\"%n", sockfd, len, s, ra);
          else
            tditrace("@E+send()_{ %d %d \"%s\"%n", sockfd, len, s, ra);
        } else {
          // s[MIN(4, len)] = '\0';
          // tditrace("@E+send()_%d_? %d \"%s...\"", sockfd, len, s);
          if (libcfd)
            tditrace("@E+send()_%d %d \"...\"%n", sockfd, len, ra);
          else
            tditrace("@E+send() %d %d \"...\"%n", sockfd, len, ra);
        }

      } else {
        if (libcfd)
          tditrace("@E+send()_%d %d%n", sockfd, len, ra);
        else
          tditrace("@E+send() %d %d%n", sockfd, len, ra);
      }

    } else {
      if (libcfd)
        tditrace("@E+send()_%d %d", sockfd, len, ra);
      else
        tditrace("@E+send() %d %d", sockfd, len, ra);
    }
  }

  ssize_t ret = __send(sockfd, buf, len, flags);

  if (libcrecording || libcsendrecording) {
    // tditrace("@I-send() =%d", ret);
  }

  return ret;
}

extern "C" ssize_t recv(int sockfd, void* buf, size_t len, int flags) {
  save_ra();
  static ssize_t (*__recv)(int, void*, size_t, int) = NULL;

  if (__recv == NULL) {
    __recv = (ssize_t(*)(int, void*, size_t, int))dlsym(RTLD_NEXT, "recv");
    if (NULL == __recv) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcrecording || libcrecvrecording) {
    // tditrace("@I+recv() %d %d", sockfd, len);
  }

  ssize_t ret = __recv(sockfd, buf, len, flags);

  if (libcrecvmatch) {
    if (ret != -1) {
      if (StrStr((const char*)buf, libcrecvmatch)) {
        char s[MAXSTRLEN + 1];
        strncpy(s, (const char*)buf, MIN(MAXSTRLEN, ret));
        s[MIN(MAXSTRLEN, ret)] = '\0';
        int i;
        for (i = 0; i < MIN(MAXSTRLEN, ret); i++)
          if (s[i] < 0x20 || s[i] >= 0x7f) s[i] = '.';
        tditrace("@E+recv():\"%s\" \"%s\"", libcrecvmatch,
                 (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
      }
    }
  }

  if (libcrecording || libcrecvrecording) {
    if (ret == -1) {
      // tditrace("@I-recv() =-1");
      if (libcfd)
        tditrace("@E+recv()_%d %d =-1%n", sockfd, len, ra);
      else
        tditrace("@E+recv() %d %d =-1%n", sockfd, len, ra);
    } else if (ret == 0) {
      // tditrace("@I-recv() =0");
      if (libcfd)
        tditrace("@E+recv()_%d %d =0%n", sockfd, len, ra);
      else
        tditrace("@E+recv() %d %d =0%n", sockfd, len, ra);
    } else {
      if (MAXSTRLEN) {
        if (buf) {
          char s[MAXSTRLEN + 1];
          strncpy(s, (const char*)buf, MIN(MAXSTRLEN, ret));
          s[MIN(MAXSTRLEN, ret)] = '\0';

          if (strncmp((const char*)buf, "HTTP", 4) == 0) {
            if (libcfd)
              tditrace("@E+recv()_%d_HTTP %d =%d \"%s\"%n", sockfd, len, ret, s,
                       ra);
            else
              tditrace("@E+recv()_HTTP %d %d =%d \"%s\"%n", sockfd, len, ret, s,
                       ra);
          } else {
            int i;
            for (i = 0; i < MIN(MAXSTRLEN, ret); i++)
              if (s[i] < 0x20 || s[i] >= 0x7f) s[i] = '.';

            if (libcfd)
              tditrace("@E+recv()_%d %d =%d \"%s\"%n", sockfd, len, ret,
                       (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
            else
              tditrace("@E+recv() %d %d =%d \"%s\"", sockfd, len, ret,
                       (s[0] >= 0x20 && s[0] < 0x7f) ? s : "?", ra);
          }
        }
      }
    }
    // tditrace("@I-recv() =%d", ret);
  }

  return ret;
}
#endif

#if 1
extern "C" ssize_t sendto(int sockfd, const void* buf, size_t len, int flags,
                          const struct sockaddr* dest_addr, socklen_t addrlen) {
  static ssize_t (*__sendto)(int, const void*, size_t, int,
                             const struct sockaddr*, socklen_t) = NULL;

  if (__sendto == NULL) {
    __sendto =
        (ssize_t(*)(int, const void*, size_t, int, const struct sockaddr*,
                    socklen_t))dlsym(RTLD_NEXT, "sendto");
    if (NULL == __sendto) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcrecording || libcsendtorecording) {
    if (0 /*MAXSTRLEN*/) {
      char s[MAXSTRLEN + 1];
      strncpy(s, (const char*)buf, MIN(MAXSTRLEN, len));
      s[MIN(MAXSTRLEN, len)] = '\0';
      // tditrace("@I+sendto() %d %d", sockfd, len);
      tditrace("@E+sendto()_%d =%d \"%s\"", sockfd, len, s);
    } else {
      tditrace("@E+sendto() %d %d ", sockfd, len);
      // tditrace("@I+sendto() %d %d ", sockfd, len);
    }
  }

  ssize_t ret = __sendto(sockfd, buf, len, flags, dest_addr, addrlen);

  if (libcrecording || libcsendtorecording) {
    // tditrace("@I-sendto() =%d", ret);
  }

  return ret;
}

extern "C" ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags,
                            struct sockaddr* src_addr, socklen_t* addrlen) {
  static ssize_t (*__recvfrom)(int, void*, size_t, int, struct sockaddr*,
                               socklen_t*) = NULL;

  if (__recvfrom == NULL) {
    __recvfrom = (ssize_t(*)(int, void*, size_t, int, struct sockaddr*,
                             socklen_t*))dlsym(RTLD_NEXT, "recvfrom");
    if (NULL == __recvfrom) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcrecording || libcrecvfromrecording) {
    // tditrace("@I+recvfrom() %d %d 0x%x", sockfd, len, flags);
  }

  ssize_t ret = __recvfrom(sockfd, buf, len, flags, src_addr, addrlen);

  if (libcrecording || libcrecvfromrecording) {
    if (ret == -1) {
      tditrace("@E+recvfrom() =-1");
      // tditrace("@I-recvfrom() =-1");
    } else if (ret == 0) {
      tditrace("@E+recvfrom() =0");
      // tditrace("@I-recvfrom() =0");
    } else {
      if (0 /*MAXSTRLEN*/) {
        char s[MAXSTRLEN + 1];
        strncpy(s, (const char*)buf, MIN(MAXSTRLEN, ret));
        s[MIN(MAXSTRLEN, ret)] = '\0';
        // tditrace("@I-recvfrom() =%d", ret);
        tditrace("@E+recvfrom()_%d =%d \"%s\"", sockfd, ret, s);
      } else {
        tditrace("@E+recvfrom() =%d", ret);
        // tditrace("@I-recvfrom() =%d", ret);
      }
    }
  }

  return ret;
}
#endif

#if 1
extern "C" ssize_t sendmsg(int sockfd, const struct msghdr* msg, int flags) {
  static ssize_t (*__sendmsg)(int, const struct msghdr*, int) = NULL;

  if (__sendmsg == NULL) {
    __sendmsg =
        (ssize_t(*)(int, const struct msghdr*, int))dlsym(RTLD_NEXT, "sendmsg");
    if (NULL == __sendmsg) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcrecording || libcsendmsgrecording) {
    // tditrace("@I+sendmsg() %d", sockfd);
    if (libcfd)
      tditrace("@E+sendmsg()_%d [%d]-%d-%d", sockfd, msg->msg_iovlen,
               msg->msg_iov[0].iov_len,
               msg->msg_iovlen > 1 ? msg->msg_iov[1].iov_len : 0);
    else
      tditrace("@E+sendmsg() %d [%d]-%d-%d", sockfd, msg->msg_iovlen,
               msg->msg_iov[0].iov_len,
               msg->msg_iovlen > 1 ? msg->msg_iov[1].iov_len : 0);
  }

  ssize_t ret = __sendmsg(sockfd, msg, flags);

  if (libcrecording || libcsendmsgrecording) {
    // tditrace("@I-sendmsg() =%d", ret);
  }

  return ret;
}

extern "C" ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags) {
  static ssize_t (*__recvmsg)(int, struct msghdr*, int) = NULL;

  if (__recvmsg == NULL) {
    __recvmsg =
        (ssize_t(*)(int, struct msghdr*, int))dlsym(RTLD_NEXT, "recvmsg");
    if (NULL == __recvmsg) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcrecording || libcrecvmsgrecording) {
    // tditrace("@I+recvmsg() %d", sockfd);
  }

  ssize_t ret = __recvmsg(sockfd, msg, flags);

  if (libcrecording || libcrecvmsgrecording) {
    // tditrace("@I-recvmsg() =%d", ret);
    if (libcfd)
      tditrace("@E+recvmsg()_%d [%d]-%d=%d", sockfd, msg->msg_iovlen,
               msg->msg_iov[0].iov_len, ret);
    else
      tditrace("@E+recvmsg() %d [%d]-%d=%d", sockfd, msg->msg_iovlen,
               msg->msg_iov[0].iov_len, ret);
  }

  return ret;
}
#endif

#if 1
extern "C" int sendmmsg(int sockfd, struct mmsghdr* msgvec, unsigned int vlen,
                        int flags) {
  static int (*__sendmmsg)(int, struct mmsghdr*, unsigned int, int) = NULL;

  if (__sendmmsg == NULL) {
    __sendmmsg = (int (*)(int, struct mmsghdr*, unsigned int, int))dlsym(
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

#if defined(HAVE_REFSW_NEXUS_CONFIG_H) || defined(__i386)
#define USE_CONST
#else
#define USE_CONST const
#endif

extern "C" int recvmmsg(int sockfd, struct mmsghdr* msgvec, unsigned int vlen,
                        int flags, USE_CONST struct timespec* timeout) {
  static int (*__recvmmsg)(int, struct mmsghdr*, unsigned int, int,
                           USE_CONST struct timespec*) = NULL;

  if (__recvmmsg == NULL) {
    __recvmmsg =
        (int (*)(int, struct mmsghdr*, unsigned int, int,
                 USE_CONST struct timespec*))dlsym(RTLD_NEXT, "recvmmsg");
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
#endif

#if 0
extern "C" int select(int nfds, fd_set* readfds, fd_set* writefds,
                      fd_set* exceptfds, struct timeval* timeout) {
  static int (*__select)(int, fd_set*, fd_set*, fd_set*, struct timeval*) =
      NULL;

  if (__select == NULL) {
    __select = (int (*)(int, fd_set*, fd_set*, fd_set*, struct timeval*))dlsym(
        RTLD_NEXT, "select");
    if (NULL == __select) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcselectrecording) {
    tditrace("@I+select() %d %x %x %x", nfds, readfds, writefds, exceptfds);
  }

  int ret = __select(nfds, readfds, writefds, exceptfds, timeout);

  if (libcselectrecording) {
    tditrace("@I-select() =%d", ret);
  }

  return ret;
}
#endif

#if 0
extern "C" int poll(struct pollfd* fds, nfds_t nfds, int timeout) {
  static int (*__poll)(struct pollfd*, nfds_t, int) = NULL;

  if (__poll == NULL) {
    __poll = (int (*)(struct pollfd*, nfds_t, int))dlsym(RTLD_NEXT, "poll");
    if (NULL == __poll) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcpollrecording) {
    tditrace("@I+poll() %x %d %d", fds, nfds, timeout);
  }

  int ret = __poll(fds, nfds, timeout);

  if (libcpollrecording) {
    tditrace("@I-poll() =%d", ret);
  }

  return ret;
}
#endif

#if 0
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

  if (libcioctlrecording) {
    tditrace("@I+ioctl() %d 0x%x", d, request);
  }

  int ret = __ioctl(d, request, a1);

  if (libcioctlrecording) {
    tditrace("@I-ioctl() =%d", ret);
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

static void heaprss(void) {
  struct mallinfo mi;
  mi = mallinfo();
  static int prev_heap = 0;
  if ((mi.arena + mi.hblkhd) != prev_heap) {
    tditrace("HEAP~%d", mi.arena + mi.hblkhd);
    prev_heap = mi.arena + mi.hblkhd;
  }

  unsigned long vmsize = 0L;
  static unsigned long prev_vmsize = 0L;
  unsigned long rss = 0L;
  static unsigned long prev_rss = 0L;

  int fh = 0;
  char buffer[65];
  int gotten;
  fh = open("/proc/self/statm", O_RDONLY);
  gotten = read(fh, buffer, 64);
  buffer[gotten] = '\0';
  if (sscanf(buffer, "%lu %lu", &vmsize, &rss) != 1) {
    // if (vmsize != prev_vmsize) {
    //  tditrace("VMSIZE~%d", (int)(vmsize * 4096));
    //  prev_vmsize = vmsize;
    //}
    if (rss != prev_rss) {
      tditrace("RSS~%d", (int)(rss * 4096));
      prev_rss = rss;
    }
  }
  close(fh);
}

#if 1
extern "C" void* malloc(size_t size) {
  save_ra();
  static void* (*__malloc)(size_t) = NULL;

  if (unlikely(__malloc == NULL)) {
    __malloc = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
    if (NULL == __malloc) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  void* ret = __malloc(size);

  if (libcmalloc && (size >= libcmalloc)) {
    // tditrace("m%n%n", ra, size);
    tditrace("m%n%n%n", ra, size, ret);
    // tditrace("%m%n%n%n", 0x01, ra, size, ret);
    heaprss();
  }

  return ret;
}
#endif

// our temporary calloc used until we get the address of libc provided
// one in our interposed calloc
static void* temporary_calloc(size_t x, size_t y) {
  // printf("empty calloc called\n");
  return NULL;
}

#if 1
extern "C" void* calloc(size_t nmemb, size_t size) {
  save_ra();
  static void* (*__calloc)(size_t, size_t) = NULL;

  if (unlikely(__calloc == NULL)) {
    __calloc = temporary_calloc;
    __calloc = (void* (*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
    if (NULL == __calloc) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  void* ret = __calloc(nmemb, size);

  if (libccalloc && (nmemb * size) >= libccalloc) {
    // tditrace("c%n%n", ra, nmemb * size);
    tditrace("c%n%n%n", ra, nmemb * size, ret);
    heaprss();
  }

  return ret;
}
#endif

#if 1
extern "C" void* realloc(void* ptr, size_t size) {
  save_ra();
  static void* (*__realloc)(void*, size_t) = NULL;

  if (unlikely(__realloc == NULL)) {
    __realloc = (void* (*)(void*, size_t))dlsym(RTLD_NEXT, "realloc");
    if (NULL == __realloc) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  void* ret = __realloc(ptr, size);

  if (libcrealloc && (size >= libcrealloc)) {
    // tditrace("r%n%n", ra, size);
    tditrace("r%n%n%n", ra, size, ret);
    heaprss();
  }

  return ret;
}
#endif

#if 1
extern "C" void free(void* ptr) {
  save_ra();
  static void (*__free)(void*) = NULL;

  if (__free == NULL) {
    __free = (void (*)(void*))dlsym(RTLD_NEXT, "free");
    if (NULL == __free) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  __free(ptr);

  if (libcfree) {
    tditrace("f%n%x", ra, ptr);
    heaprss();
  }
}
#endif

#if 1
extern "C" int brk(void* __addr) {
  static int (*__brk)(void*) = NULL;
  if (__brk == NULL) {
    __brk = (int (*)(void*))dlsym(RTLD_NEXT, "brk");
    if (__brk == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  tditrace("brk");
  int ret = __brk(__addr);
  return ret;
}
#endif

#if 1
extern "C" void* sbrk(intptr_t __delta) {
  static void* (*__sbrk)(intptr_t) = NULL;
  if (__sbrk == NULL) {
    __sbrk = (void* (*)(intptr_t))dlsym(RTLD_NEXT, "sbrk");
    if (__sbrk == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  tditrace("sbrk");
  void* ret = __sbrk(__delta);
  return ret;
}
#endif

#if 1
extern "C" void* mmap(void* __addr, size_t __len, int __prot, int __flags,
                      int __fd, __off_t __offset) {
  save_ra();
  static void* (*__mmap)(void*, size_t, int, int, int, __off_t) = NULL;
  if (unlikely(__mmap == NULL)) {
    __mmap = (void* (*)(void*, size_t, int, int, int, __off_t))dlsym(RTLD_NEXT,
                                                                     "mmap");
    if (__mmap == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  void* ret = __mmap(__addr, __len, __prot, __flags, __fd, __offset);

  if (libcmmap) {
    // tditrace("mm%n%n", ra, (int)__len);
    tditrace("mm%n%n%n", ra, (int)__len, ret);
    heaprss();
  }

  return ret;
}
#endif

#if 1
extern "C" int munmap(void* __addr, size_t __len) {
  save_ra();

  static int (*__munmap)(void*, size_t) = NULL;
  if (unlikely(__munmap == NULL)) {
    __munmap = (int (*)(void*, size_t))dlsym(RTLD_NEXT, "munmap");
    if (__munmap == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  int ret = __munmap(__addr, __len);

  if (libcmunmap) {
    tditrace("mu%n%n", ra, __len);
    heaprss();
  }

  return ret;
}
#endif

#if 1
extern "C" void* memalign(size_t __alignment, size_t __size) {
  save_ra() static void* (*__memalign)(size_t, size_t) = NULL;
  if (unlikely(__memalign == NULL)) {
    __memalign = (void* (*)(size_t, size_t))dlsym(RTLD_NEXT, "memalign");
    if (__memalign == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  void* ret = __memalign(__alignment, __size);

  if (libcmemalign && (__size >= libcmemalign)) {
    // tditrace("ma%n%n", ra, __size);
    tditrace("ma%n%n%n", ra, __size, ret);
    heaprss();
  }

  return ret;
}
#endif

#if 0
extern "C" int posix_memalign(void** __memptr, size_t __alignment,
                              size_t __size) {
  save_ra();
  static int (*__posix_memalign)(void**, size_t, size_t) = NULL;
  if (__posix_memalign == NULL) {
    __posix_memalign =
        (int (*)(void**, size_t, size_t))dlsym(RTLD_NEXT, "posix_memalign");
    if (__posix_memalign == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }


  int ret = __posix_memalign(__memptr, __alignment, __size);

  if (libcmemalign && (__size >= libcmemalign)) {
    tditrace("posix_memalign %d,ra=%x", __size, ra);

    mi();
    ru();
  }

  return ret;
}
#endif

#if 0
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
#endif

#if 0
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

#if 1
void* operator new[](unsigned int i) {
  save_ra();

  // use a local copy so as to not incurr the tditrace from malloc
  static void* (*___malloc)(size_t) = NULL;
  if (unlikely(___malloc == NULL)) {
    ___malloc = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
    if (NULL == ___malloc) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  void* ret = ___malloc(i);

  if (libcoperatornew && (i >= libcoperatornew)) {
    // tditrace("n%n%n", ra, i);
    tditrace("n[]%n%n%n", ra, i, ret);
    heaprss();
  }

  return ret;
}
#endif

//_Znwj
#if 1
void* operator new(unsigned int i) {
  save_ra();

  static void* (*___malloc)(size_t) = NULL;
  if (unlikely(___malloc == NULL)) {
    ___malloc = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
    if (NULL == ___malloc) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  void* ret = ___malloc(i);

  if (libcoperatornew && (i >= libcoperatornew)) {
    // tditrace("n%n%n", ra, i);
    tditrace("n%n%n%n", ra, i, ret);
    heaprss();
  }

  return ret;
}
#endif

#if 1
extern "C" void syslog(int pri, const char* fmt, ...) {
  save_ra();
  static void (*__syslog)(int, const char*, ...) = NULL;

  if (__syslog == NULL) {
    __syslog = (void (*)(int, const char*, ...))dlsym(RTLD_NEXT, "syslog");
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

#define MAXSYSLOGSTRLEN 256
  if (libcsyslog) {
    char s[MAXSYSLOGSTRLEN + 1];
    char* m = s;
    strncpy(s, (const char*)a1, MIN(MAXSYSLOGSTRLEN, libcsyslog));
    s[MIN(MAXSYSLOGSTRLEN, libcsyslog)] = '\0';

#if 0
    if (strstr(s, "p=0x61af")) {
      tditrace("@S+SEGFAULT");
      char *p = 0;
      *p = 0;
    }
#endif

    if (m = strstr(s, "[mod=")) {
      tditrace("@S+syslog():%s%n", m + 5, ra);
    } else if (m = strstr(s, "[console] ")) {
      tditrace("@S+syslog():%s%n", m + 10, ra);
    } else {
      tditrace("@S+syslog() %s%n", s, ra);
    }
  }

  __syslog(pri, (const char*)a1, a2, a3);
}
#endif

#if 1

extern "C" int sigaction(int signum, const struct sigaction* act,
                         struct sigaction* oldact) {
  save_ra();
  static int (*__sigaction)(int, const struct sigaction*, struct sigaction*) =
      NULL;
  if (__sigaction == NULL) {
    __sigaction = (int (*)(int, const struct sigaction*,
                           struct sigaction*))dlsym(RTLD_NEXT, "sigaction");
    if (NULL == __sigaction) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcsigactionrecording) {
#if 0
    // block the redirecting of sigsegv and sigtrap
    if ((signum == 11) || (signum == 5)) {
      tditrace("@S+sigaction()_%d_SKIP%n", signum, ra);
      return 0;
    } else {
      tditrace("@S+sigaction()_%d%n", signum, ra);
    }
#else
    tditrace("@S+sigaction() %d%n", signum, ra);
#endif
  }

  return __sigaction(signum, act, oldact);
}
#endif

#if 1
int sigqueue(pid_t pid, int sig, const union sigval value) {
  save_ra();
  static int (*__sigqueue)(pid_t, int, const union sigval) = NULL;
  if (__sigqueue == NULL) {
    __sigqueue =
        (int (*)(pid_t, int, const union sigval))dlsym(RTLD_NEXT, "sigqueue");
    if (NULL == __sigqueue) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (libcsigqueuerecording) {
    tditrace("@S+sigqueue() %d%d%n", pid, sig, ra);
  }

  return __sigqueue(pid, sig, value);
}
#endif

#if 1
int raise(int sig) {
  save_ra();
  static int (*__raise)(int) = NULL;
  if (__raise == NULL) {
    __raise = (int (*)(int))dlsym(RTLD_NEXT, "raise");
    if (NULL == __raise) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  tditrace("@S+raise()_%d%n", sig, ra);

  return __raise(sig);
}
#endif
