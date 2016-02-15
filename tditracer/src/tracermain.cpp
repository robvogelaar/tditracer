
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <stdarg.h>

#include "tdi.h"

#include "tracermain.h"
#include "tracerutils.h"

extern "C" {
#include "shadercapture.h"
#include "texturecapture.h"
#include "framecapture.h"
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

bool pthreadrecording;
bool eglrecording;
bool gles2recording;
bool gldrawrecording;

int shaders_captured = 0;
int textures_captured = 0;
int texturebytes_captured = 0;
int frames_captured = 0;

typedef unsigned int mz_uint;
typedef int mz_bool;
extern "C" void *tdefl_write_image_to_png_file_in_memory(const void *pImage,
                                                         int w, int h,
                                                         int num_chans,
                                                         size_t *pLen_out);
extern "C" void *
tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h,
                                           int num_chans, size_t *pLen_out,
                                           mz_uint level, mz_bool flip);

static void signalhandler(int sig, siginfo_t *si, void *context) {

    switch (sig) {
    case SIGINT:
        printf("tditracer: received SIGINT\n");
        break;

    case SIGQUIT:
        printf("tditracer: received SIGQUIT, rewinding tracebuffer\n");
        tditrace_rewind();
        break;
    }
}

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
            libcopenrecording = true;
            libcfopenrecording = true;
            libcfdopenrecording = true;
            libcfreopenrecording = true;
            libcreadrecording = true;
            libcwriterecording = true;
            libcsocketrecording = true;
            libcsendrecording = true;
            libcsendtorecording = true;
            libcsendmsgrecording = true;
            libcsendmmsgrecording = true;
            libcrecvrecording = true;
            libcrecvfromrecording = true;
            libcrecvmsgrecording = true;
            libcrecvmmsgrecording = true;
            libcselectrecording = true;
            libcpollrecording = true;
            libcioctlrecording = false;
            libcsyslogrecording = true;
            libcmalloc = 0;
            libccalloc = 0;
            libcrealloc = 0;

        } else {
            libcrecording = false;
            libcioctlrecording = false;
        }

        if (env = getenv("LIBCOPEN")) {
            libcopenrecording = libcfopenrecording = libcfdopenrecording =
                libcfreopenrecording = (atoi(env) >= 1);
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

        printf("tditracer: init[%d], libc:%s, pthread:%s, shaders:%s, "
               "textures:%s, "
               "renderbuffers:%s, frames:%d\n",
               getpid(), libcrecording ? "yes" : "no",
               pthreadrecording ? "yes" : "no", shaderrecording ? "yes" : "no",
               texturerecording ? "yes" : "no",
               renderbufferrecording ? "yes" : "no", framerecording);

        inited = true;
    }
}
