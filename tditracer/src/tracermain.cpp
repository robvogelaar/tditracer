
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>

#include "tdi.h"

#include "tracermain.h"
#include "tracerutils.h"

extern "C" {
#include "framecapture.h"
#include "shadercapture.h"
#include "texturecapture.h"
}

static void init(void);
static void dump(void);

static void __attribute__((constructor)) tditracer_constructor();
static void __attribute__((destructor)) tditracer_destructor();

static void tditracer_constructor() {
  tditrace_init();

  init();
}

static void tditracer_destructor() {
  fprintf(stderr, "tditracer: exit[%d]\n", getpid());
}

int framerecording;
int texturerecording;
int renderbufferrecording;
int shaderrecording;

bool libcrecording;
bool libcopenrecording;
bool libcfopenrecording;
bool libcfdopenrecording;
bool libcfreopenrecording;
bool libcfd;
bool libcreadrecording;
bool libcwriterecording;
bool libcsocketrecording;
bool libcsendrecording;
bool libcsendtorecording;
bool libcsendmsgrecording;
bool libcsendmmsgrecording;
bool libcrecvrecording;
bool libcrecvfromrecording;
bool libcrecvmsgrecording;
bool libcrecvmmsgrecording;
bool libcselectrecording;
bool libcpollrecording;
bool libcioctlrecording;
bool libcsyslogrecording;

unsigned int libcmalloc;
unsigned int libccalloc;
unsigned int libcrealloc;
unsigned int libcmemalign;
unsigned int libcfree;
unsigned int libcoperatornew;
unsigned int libcmmap;
unsigned int libcmunmap;

bool pthreadrecording;
bool eglrecording;
bool gles2recording;
bool gldrawrecording;
bool gltexturerecording;

int shaders_captured;
int textures_captured;
int texturebytes_captured;
int frames_captured;

typedef unsigned int mz_uint;
typedef int mz_bool;
extern "C" void *tdefl_write_image_to_png_file_in_memory(const void *pImage,
                                                         int w, int h,
                                                         int num_chans,
                                                         size_t *pLen_out);
extern "C" void *tdefl_write_image_to_png_file_in_memory_ex(
    const void *pImage, int w, int h, int num_chans, size_t *pLen_out,
    mz_uint level, mz_bool flip);

#if 0
static void signalhandler(int sig, siginfo_t *si, void *context) {
  switch (sig) {
    case SIGINT:
      fprintf(stderr, "tditracer: received SIGINT\n");
      break;

    case SIGQUIT:
      fprintf(stderr, "tditracer: received SIGQUIT, rewinding tracebuffer\n");
      tditrace_rewind();
      break;
  }
}
#endif

static void init(void) {
  static bool inited = false;
  if (!inited) {
#if 0
        static struct sigaction sVal;

        sVal.sa_flags = SA_SIGINFO;
        sVal.sa_sigaction = signalhandler;
        // Register for SIGINT
        sigaction(SIGINT, &sVal, NULL);
        // Register for SIGQUIT
        sigaction(SIGQUIT, &sVal, NULL);

#endif
    char *env;
    if (env = getenv("LIBC")) {
      libcrecording = (atoi(env) >= 1);

      libcreadrecording = true;
      libcwriterecording = true;

      libcsendrecording = true;
      libcrecvrecording = true;

      libcsendtorecording = true;
      libcrecvfromrecording = true;

      libcsendmsgrecording = true;
      libcrecvmsgrecording = true;

      libcsendmmsgrecording = true;
      libcrecvmmsgrecording = true;
    }

    if (env = getenv("LIBCOPEN")) {
      libcopenrecording = libcfopenrecording = libcfdopenrecording =
          libcfreopenrecording = (atoi(env) >= 1);
    }
    if (env = getenv("LIBCFD")) {
      libcfd = (atoi(env) >= 1);
    }
    if (env = getenv("LIBCREAD")) {
      libcreadrecording = (atoi(env) >= 1);
    }
    if (env = getenv("LIBCWRITE")) {
      libcwriterecording = (atoi(env) >= 1);
    }
    if (env = getenv("LIBCSOCKET")) {
      libcsocketrecording = (atoi(env) >= 1);
      libcsendrecording = libcsendtorecording = libcsendmsgrecording =
          libcsendmmsgrecording = (atoi(env) >= 1);
      libcrecvrecording = libcrecvfromrecording = libcrecvmsgrecording =
          libcrecvmmsgrecording = (atoi(env) >= 1);
      libcselectrecording = (atoi(env) >= 1);
      libcpollrecording = (atoi(env) >= 1);
    }
    if (env = getenv("LIBCIOCTL")) {
      libcioctlrecording = (atoi(env) >= 1);
    }

    if (env = getenv("LIBCMALLOC")) {
      libcmalloc = atoi(env);
    }
    if (env = getenv("LIBCCALLOC")) {
      libccalloc = atoi(env);
    }
    if (env = getenv("LIBCREALLOC")) {
      libcrealloc = atoi(env);
    }
    if (env = getenv("LIBCMEMALIGN")) {
      libcmemalign = atoi(env);
    }
    if (env = getenv("LIBCFREE")) {
      libcfree = atoi(env);
    }
    if (env = getenv("LIBCOPERATORNEW")) {
      libcoperatornew = atoi(env);
    }

    if (env = getenv("LIBCMMAP")) {
      libcmmap = atoi(env);
    }
    if (env = getenv("LIBCMUNMAP")) {
      libcmunmap = atoi(env);
    }

    if (env = getenv("LIBCSEND")) {
      libcsendrecording = (atoi(env) >= 1);
    }
    if (env = getenv("LIBCRECV")) {
      libcrecvrecording = (atoi(env) >= 1);
    }
    if (env = getenv("LIBCSYSLOG")) {
      libcsyslogrecording = (atoi(env) >= 1);
    }

    if (getenv("PTHREAD")) {
      pthreadrecording = (atoi(getenv("PTHREAD")) >= 1);
    } else {
      pthreadrecording = false;
    }

    if (getenv("EGL")) {
      eglrecording = (atoi(getenv("EGL")) >= 1);
    } else {
      eglrecording = false;
    }

    if (getenv("GLES2")) {
      gles2recording = (atoi(getenv("GLES2")) >= 1);
    } else {
      gles2recording = false;
    }

    if (getenv("GLDRAW")) {
      gldrawrecording = (atoi(getenv("GLDRAW")) >= 1);
    } else {
      gldrawrecording = false;
    }

    if (getenv("GLTEXTURE")) {
      gltexturerecording = (atoi(getenv("GLTEXTURE")) >= 1);
    } else {
      gltexturerecording = false;
    }

    if (getenv("TR")) {
      texturerecording = atoi(getenv("TR"));
    }
    if (getenv("RR")) {
      renderbufferrecording = atoi(getenv("RR"));
    }
    if (getenv("FR")) {
      framerecording = atoi(getenv("FR"));
    }
    if (getenv("SR")) {
      shaderrecording = atoi(getenv("SR"));
    }

    if (renderbufferrecording) {
      texturerecording = renderbufferrecording;
    }

    fprintf(stderr,
            "tditracer: [%d], libc:%s, pthread:%s, shaders:%s, "
            "textures:%s, "
            "renderbuffers:%s, frames:%d\n",
            getpid(), libcrecording ? "yes" : "no",
            pthreadrecording ? "yes" : "no", shaderrecording ? "yes" : "no",
            texturerecording ? "yes" : "no",
            renderbufferrecording ? "yes" : "no", framerecording);

    inited = true;
  }
}
