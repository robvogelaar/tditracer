#ifndef NOEGL

#include <dlfcn.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern "C" {
#include "framecapture.h"
#include "shadercapture.h"
#include "texturecapture.h"
}

#include "gldefs.h"
#include "tdi.h"
#include "tracermain.h"
#include "tracerutils.h"

#include "gles2bars.h"
#include "gles2capture.h"
#include "gles2spinner.h"

#ifdef __mips__
#define save_ra() \
  int ra;         \
  asm volatile("move %0, $ra" : "=r"(ra));
#else
#ifdef __arm__
#define save_ra() \
  int ra;         \
  asm volatile("mov %0, r14" : "=r"(ra));
#else
#define save_ra() int ra = 0;
#endif
#endif

#define check_glerror()                           \
  {                                               \
    GLenum err = GL_NO_ERROR;                     \
    while ((err = glGetError()) != GL_NO_ERROR) { \
      tditrace("@S+GLERROR 0x%x", err);           \
    }                                             \
  }

extern "C" void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                             GLenum format, GLenum type, GLvoid *data);

static GLuint boundtexture;
static GLuint currentprogram;
static GLuint currentbuffer;
static GLuint boundframebuffer = 0;
static GLuint boundrenderbuffer = 0;
static GLuint framebuffertexture[1024] = {};
static GLuint framebufferrenderbuffer[1024] = {};
static GLuint renderbufferwidth[1024] = {};
static GLuint renderbufferheight[1024] = {};

static int draw_counter = 0;
static int texturebind_counter = 0;
static int texturegen_counter = 0;

static int current_frame = 1;

static int glDrawArrays_counter = 0;
static int glDrawElements_counter = 0;
static int glTexImage2D_counter = 0;
static int glTexSubImage2D_counter = 0;
static int glBindTexture_counter = 0;

static int glShaderSource_counter = 0;

extern "C" EGLBoolean eglInitialize(EGLDisplay display, EGLint *major,
                                    EGLint *minor) {
  static EGLBoolean (*__eglInitialize)(EGLDisplay, EGLint *, EGLint *) = NULL;

  if (__eglInitialize == NULL) {
    __eglInitialize = (EGLBoolean(*)(EGLDisplay, EGLint *, EGLint *))dlsym(
        RTLD_NEXT, "eglInitialize");
    if (NULL == __eglInitialize) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (eglrecording) tditrace("@I+eglInitialize()");

  EGLBoolean ret = __eglInitialize(display, major, minor);

  if (eglrecording) tditrace("@I-eglInitialize()");

  return ret;
}

extern "C" EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list,
                                      EGLConfig *configs, EGLint config_size,
                                      EGLint *num_config) {
  save_ra();
  static EGLBoolean (*__eglChooseConfig)(EGLDisplay, const EGLint *,
                                         EGLConfig *, EGLint, EGLint *) = NULL;
  if (__eglChooseConfig == NULL) {
    __eglChooseConfig =
        (EGLBoolean(*)(EGLDisplay, const EGLint *, EGLConfig *, EGLint,
                       EGLint *))dlsym(RTLD_NEXT, "eglChooseConfig");
    if (__eglChooseConfig == NULL) {
      fprintf(stderr, "Error in dlsym: %s(%s)\n", dlerror() ? dlerror() : "?",
              "eglChooseConfig");
    }
  }
  if (eglrecording) tditrace("eglChooseConfig()%n", ra);
  EGLBoolean ret =
      __eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
  return ret;
}

extern "C" EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config,
                                         EGLint attribute, EGLint *value) {
  save_ra();
  static EGLBoolean (*__eglGetConfigAttrib)(EGLDisplay, EGLConfig, EGLint,
                                            EGLint *) = NULL;
  if (__eglGetConfigAttrib == NULL) {
    __eglGetConfigAttrib =
        (EGLBoolean(*)(EGLDisplay, EGLConfig, EGLint, EGLint *))dlsym(
            RTLD_NEXT, "eglGetConfigAttrib");
    if (__eglGetConfigAttrib == NULL) {
      fprintf(stderr, "Error in dlsym: %s(%s)\n", dlerror() ? dlerror() : "?",
              "eglGetConfigAttrib");
    }
  }
  EGLBoolean ret = __eglGetConfigAttrib(dpy, config, attribute, value);

  if (eglrecording)
    tditrace("eglGetConfigAttrib() %s=%d%n", CONFIGATTRIBUTESSTRING(attribute),
             *value, ra);
  return ret;
}

extern "C" EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs,
                                    EGLint config_size, EGLint *num_config) {
  save_ra();
  static EGLBoolean (*__eglGetConfigs)(EGLDisplay, EGLConfig *, EGLint,
                                       EGLint *) = NULL;
  if (__eglGetConfigs == NULL) {
    __eglGetConfigs =
        (EGLBoolean(*)(EGLDisplay, EGLConfig *, EGLint, EGLint *))dlsym(
            RTLD_NEXT, "eglGetConfigs");
    if (__eglGetConfigs == NULL) {
      fprintf(stderr, "Error in dlsym: %s(%s)\n", dlerror() ? dlerror() : "?",
              "eglGetConfigs");
    }
  }
  if (eglrecording) tditrace("eglGetConfigs()%n", ra);
  EGLBoolean ret = __eglGetConfigs(dpy, configs, config_size, num_config);
  return ret;
}

extern "C" EGLContext eglCreateContext(EGLDisplay display, EGLConfig config,
                                       EGLContext share_context,
                                       const EGLint *attrib_list) {
  save_ra();
  static EGLContext (*__eglCreateContext)(EGLDisplay, EGLConfig, EGLContext,
                                          const EGLint *) = NULL;
  if (__eglCreateContext == NULL) {
    __eglCreateContext =
        (EGLContext(*)(EGLDisplay, EGLConfig, EGLContext, const EGLint *))dlsym(
            RTLD_NEXT, "eglCreateContext");
    if (__eglCreateContext == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  EGLContext ret =
      __eglCreateContext(display, config, share_context, attrib_list);
  if (gles2recording) tditrace("eglCreateContext() =0x%x%n", ret, ra);
  return ret;
}

extern "C" EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config,
                                              const EGLint *attrib_list) {
  static EGLSurface (*__eglCreatePbufferSurface)(EGLDisplay, EGLConfig,
                                                 const EGLint *) = NULL;
  if (__eglCreatePbufferSurface == NULL) {
    __eglCreatePbufferSurface =
        (EGLSurface(*)(EGLDisplay, EGLConfig, const EGLint *))dlsym(
            RTLD_NEXT, "eglCreatePbufferSurface");
    if (__eglCreatePbufferSurface == NULL) {
      fprintf(stderr, "Error in dlsym: %s(%s)\n", dlerror() ? dlerror() : "?",
              "eglCreatePbufferSurface");
    }
  }
  if (eglrecording) tditrace("eglCreatePbufferSurface()");
  EGLSurface ret = __eglCreatePbufferSurface(dpy, config, attrib_list);
  return ret;
}

extern "C" EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config,
                                             NativePixmapType pixmap,
                                             const EGLint *attrib_list) {
  static EGLSurface (*__eglCreatePixmapSurface)(
      EGLDisplay, EGLConfig, NativePixmapType, const EGLint *) = NULL;
  if (__eglCreatePixmapSurface == NULL) {
    __eglCreatePixmapSurface = (EGLSurface(*)(
        EGLDisplay, EGLConfig, NativePixmapType,
        const EGLint *))dlsym(RTLD_NEXT, "eglCreatePixmapSurface");
    if (__eglCreatePixmapSurface == NULL) {
      fprintf(stderr, "Error in dlsym: %s(%s)\n", dlerror() ? dlerror() : "?",
              "eglCreatePixmapSurface");
    }
  }
  if (eglrecording) tditrace("eglCreatePixmapSurface()");
  EGLSurface ret = __eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
  return ret;
}

extern "C" EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
                                             NativeWindowType window,
                                             const EGLint *attrib_list) {
  static EGLSurface (*__eglCreateWindowSurface)(
      EGLDisplay, EGLConfig, NativeWindowType, const EGLint *) = NULL;
  if (__eglCreateWindowSurface == NULL) {
    __eglCreateWindowSurface = (EGLSurface(*)(
        EGLDisplay, EGLConfig, NativeWindowType,
        const EGLint *))dlsym(RTLD_NEXT, "eglCreateWindowSurface");
    if (__eglCreateWindowSurface == NULL) {
      fprintf(stderr, "Error in dlsym: %s(%s)\n", dlerror() ? dlerror() : "?",
              "eglCreateWindowSurface");
    }
  }
  if (eglrecording) tditrace("eglCreateWindowSurface()");
  EGLSurface ret = __eglCreateWindowSurface(dpy, config, window, attrib_list);
  return ret;
}

extern "C" EGLBoolean eglMakeCurrent(EGLDisplay display, EGLSurface draw,
                                     EGLSurface read, EGLContext context) {
  static EGLBoolean (*__eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface,
                                        EGLContext) = NULL;

  if (__eglMakeCurrent == NULL) {
    __eglMakeCurrent =
        (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLSurface, EGLContext))dlsym(
            RTLD_NEXT, "eglMakeCurrent");
    if (NULL == __eglMakeCurrent) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  // printf("eglMakeCurrent display=%d draw=0x%x read=0x%x
  // context=0x%x%s,[%s]\n", display, draw, read, context,
  // addrinfo(__builtin_return_address(0)));

  if (eglrecording) tditrace("@I+eglMakeCurrent()");
  EGLBoolean b = __eglMakeCurrent(display, draw, read, context);
  if (eglrecording) {
    tditrace("@I-eglMakeCurrent()");
    tditrace(
        "@S+eglMakeCurrent()_%x_%x_%x_%x display=%d draw=0x%x read=0x%x "
        "context=0x%x",
        display, draw, read, context, display, draw, read, context);
  }
  boundframebuffer = 0;

  return b;
}

extern "C" EGLBoolean eglSwapBuffers(EGLDisplay display, EGLSurface surface) {
  static EGLBoolean (*__eglSwapBuffers)(EGLDisplay display,
                                        EGLSurface surface) = NULL;
  if (__eglSwapBuffers == NULL) {
    __eglSwapBuffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))dlsym(
        RTLD_NEXT, "eglSwapBuffers");
    if (NULL == __eglSwapBuffers) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  check_glerror();

  if (eglrecording) tditrace("@I+eglSwapBuffers()");

  if (gosd & 0x1) {
    spinner_render(current_frame);
    bars_render();
  }

  // capture_frame();
  if (framerecording) {
    tditrace("@A+FR %d", current_frame);
    static char *framebuffer = 0;
    if (!framebuffer) {
      framebuffer = (char *)memalign(4096, 1280 * 720 * 4);
    }

    glReadPixels(0, 0, 1280, 720, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer);
    framecapture_capframe(framebuffer);

    tditrace("@A-FR");
    frames_captured++;
  }

  EGLBoolean ret = __eglSwapBuffers(display, surface);

  if (eglrecording) tditrace("@I-eglSwapBuffers()");

  if (eglrecording)
    tditrace("@S+eglSwapBuffers()_%d_%d #%d #gldraws=%d #gltexturebinds=%d",
             display, surface, current_frame, draw_counter,
             texturebind_counter);

  if (eglrecording && gles2recording) tditrace("#gldraws~%d", 0);
  if (eglrecording && gles2recording) tditrace("#gltexturebinds~%d", 0);

  glDrawElements_counter = 0;
  glDrawArrays_counter = 0;
  glTexImage2D_counter = 0;
  glTexSubImage2D_counter = 0;
  glBindTexture_counter = 0;

  draw_counter = 0;
  texturebind_counter = 0;

/*
 */
#if 0
  if ((current_frame == shaderrecording) ||
      (current_frame == texturerecording) ||
      (current_frame == renderbufferrecording) ||
      (current_frame == framerecording)) {
    if (shaderrecording) {
      shadercapture_writeshaders();
    }
    if (texturerecording || renderbufferrecording) {
      texturecapture_writepngtextures();
      texturecapture_deletetextures();
    }
    if (framerecording) {
      framecapture_writepngframes();
      framecapture_deleteframes();
    }
  }
#endif

  current_frame++;

  return ret;
}

extern "C" EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval) {
  static EGLBoolean (*__eglSwapInterval)(EGLDisplay, EGLint) = NULL;
  if (__eglSwapInterval == NULL) {
    __eglSwapInterval =
        (EGLBoolean(*)(EGLDisplay, EGLint))dlsym(RTLD_NEXT, "eglSwapInterval");
    if (__eglSwapInterval == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (eglrecording) tditrace("eglSwapInterval() %d", interval);
  EGLBoolean ret = __eglSwapInterval(dpy, interval);
  return ret;
}

#if 0
/*
 *
 * eglCreateSyncKHR
 * eglDestroySyncKHR
 * eglClientWaitSyncKHR
 * eglGetSyncAttribKHR
 *
 * eglCreateImageKHR
 * eglDestroyImageKHR
 * glEGLImageTargetTexture2DOES
 *
 * eglCreateNativePixmapNDS
 * eglDestroyNativePixmapNDS
 * eglFlushNativePixmapNDS
 * eglQueryNativePixmapNDS
 *
 * eglGetVideoYUVImagesNDS
 * eglReleaseVideoYUVImagesNDS
 *
 */

#include <EGL/egl.h>
#include <EGL/eglext.h>

extern "C" {

/*---------------------------------------------------------------*/

static PFNEGLCREATESYNCKHRPROC _eglCreateSyncKHR;
static PFNEGLDESTROYSYNCKHRPROC _eglDestroySyncKHR;
static PFNEGLCLIENTWAITSYNCKHRPROC _eglClientWaitSyncKHR;
static PFNEGLGETSYNCATTRIBKHRPROC _eglGetSyncAttribKHR;

static EGLSyncKHR __eglCreateSyncKHR(EGLDisplay dpy, EGLenum type,
                                     const EGLint *attrib_list) {
  if (eglrecording) tditrace("@I+eglCreateSyncKHR()_%x", dpy);
  EGLSyncKHR sync = _eglCreateSyncKHR(dpy, type, attrib_list);
  if (eglrecording) tditrace("@I-eglCreateSyncKHR()_%x =%x", dpy, sync);
  return sync;
}

static EGLBoolean __eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR sync) {
  if (eglrecording) tditrace("@I+eglDestroySyncKHR()_%x %x", dpy, sync);
  EGLBoolean ret = _eglDestroySyncKHR(dpy, sync);
  if (eglrecording) tditrace("@I-eglDestroySyncKHR()_%x", dpy, sync);
  return ret;
}

static EGLint __eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync,
                                     EGLint flags, EGLTimeKHR timeout) {
  if (eglrecording) tditrace("@I+eglClientWaitSyncKHR()_%x %x", dpy, sync);
  EGLint ret = _eglClientWaitSyncKHR(dpy, sync, flags, timeout);
  if (eglrecording)
    tditrace("@I-eglClientWaitSyncKHR()_%x %x=%x", dpy, sync, ret);
  return ret;
}

static EGLBoolean __eglGetSyncAttribKHR(EGLDisplay dpy, EGLSyncKHR sync,
                                        EGLint attribute, EGLint *value) {
  if (eglrecording) tditrace("eglGetSyncAttribKHR()_%x %x", dpy, sync);
  return _eglGetSyncAttribKHR(dpy, sync, attribute, value);
}

/*---------------------------------------------------------------*/

static PFNEGLCREATEIMAGEKHRPROC _eglCreateImageKHR;
static PFNEGLDESTROYIMAGEKHRPROC _eglDestroyImageKHR;

static EGLImageKHR __eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx,
                                       EGLenum target, EGLClientBuffer buffer,
                                       const EGLint *attrib_list) {
  EGLImageKHR image = _eglCreateImageKHR(dpy, ctx, target, buffer, attrib_list);
  if (eglrecording) tditrace("eglCreateImageKHR()_%x =%x", dpy, image);
  return image;
}

static EGLBoolean __eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image) {
  if (eglrecording) tditrace("eglDestroyImageKHR()_%x %x", dpy, image);
  return _eglDestroyImageKHR(dpy, image);
}

// void __glEGLImageTargetTexture2DOES(GLenum target, EGLImageKHR image) {}

/*---------------------------------------------------------------*/

typedef EGLNativePixmapType(EGLAPIENTRYP PFNEGLCREATENATIVEPIXMAPNDSPROC)(
    EGLDisplay display, const EGLint *attrib_list);
typedef EGLBoolean(EGLAPIENTRYP PFNEGLFLUSHNATIVEPIXMAPNDSPROC)(
    EGLDisplay display, EGLNativePixmapType pixmap);
typedef EGLBoolean(EGLAPIENTRYP PFNEGLDESTROYNATIVEPIXMAPNDSPROC)(
    EGLDisplay display, EGLNativePixmapType pixmap);
typedef EGLBoolean(EGLAPIENTRYP PFNEGLQUERYNATIVEPIXMAPNDSPROC)(
    EGLDisplay display, EGLNativePixmapType pixmap, EGLint attribute,
    EGLint *value);

static PFNEGLCREATENATIVEPIXMAPNDSPROC _eglCreateNativePixmapNDS;
static PFNEGLFLUSHNATIVEPIXMAPNDSPROC _eglFlushNativePixmapNDS;
static PFNEGLDESTROYNATIVEPIXMAPNDSPROC _eglDestroyNativePixmapNDS;
static PFNEGLQUERYNATIVEPIXMAPNDSPROC _eglQueryNativePixmapNDS;

static EGLNativePixmapType __eglCreateNativePixmapNDS(
    EGLDisplay display, const EGLint *attrib_list) {
  EGLNativePixmapType pixmap = _eglCreateNativePixmapNDS(display, attrib_list);
  if (eglrecording)
    tditrace("eglCreateNativePixmapNDS()_%x =%x", display, pixmap);
  return pixmap;
}

static EGLBoolean __eglFlushNativePixmapNDS(EGLDisplay display,
                                            EGLNativePixmapType pixmap) {
  if (eglrecording)
    tditrace("eglFlushNativePixmapNDS()_%x %x", display, pixmap);
  return _eglFlushNativePixmapNDS(display, pixmap);
}

static EGLBoolean __eglDestroyNativePixmapNDS(EGLDisplay display,
                                              EGLNativePixmapType pixmap) {
  if (eglrecording)
    tditrace("eglDestroyNativePixmapNDS()_%x %x", display, pixmap);
  return _eglDestroyNativePixmapNDS(display, pixmap);
}

static EGLBoolean __eglQueryNativePixmapNDS(EGLDisplay display,
                                            EGLNativePixmapType pixmap,
                                            EGLint attribute, EGLint *value) {
  if (eglrecording)
    tditrace("eglQueryNativePixmapNDS()_%x %x", display, pixmap);
  return _eglQueryNativePixmapNDS(display, pixmap, attribute, value);
}

/*---------------------------------------------------------------*/

typedef EGLBoolean(EGLAPIENTRYP PFNEGLGETVIDEOYUVIMAGESNDSPROC)(
    EGLDisplay display, EGLImageKHR *images_array, EGLint nb_images,
    const EGLint *attrib_list);
typedef EGLBoolean(EGLAPIENTRYP PFNEGLRELEASEVIDEOYUVIMAGESNDSPROC)(
    EGLDisplay display, const EGLImageKHR *images_array, EGLint nb_images);

static PFNEGLGETVIDEOYUVIMAGESNDSPROC _eglGetVideoYUVImagesNDS;
static PFNEGLRELEASEVIDEOYUVIMAGESNDSPROC _eglReleaseVideoYUVImagesNDS;

EGLBoolean eglGetVideoYUVImagesNDS(EGLDisplay display,
                                   EGLImageKHR *images_array, EGLint nb_images,
                                   const EGLint *attrib_list);

EGLBoolean eglReleaseVideoYUVImagesNDS(EGLDisplay display,
                                       const EGLImageKHR *images_array,
                                       EGLint nb_images);

/*---------------------------------------------------------------*/
}

extern "C" void (*eglGetProcAddress(const char *procname))() {
  static void (*(*__eglGetProcAddress)(char const *))() = NULL;
  if (__eglGetProcAddress == NULL) {
    __eglGetProcAddress =
        (void (*(*)(const char *))())dlsym(RTLD_NEXT, "eglGetProcAddress");
    if (__eglGetProcAddress == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  if (eglrecording) tditrace("eglGetProcAddress() \"%s\"", procname);

  if (strcmp(procname, "eglCreateSyncKHR") == 0) {
    _eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC)__eglGetProcAddress(procname);
    return (void (*)())__eglCreateSyncKHR;
  } else if (strcmp(procname, "eglDestroySyncKHR") == 0) {
    _eglDestroySyncKHR =
        (PFNEGLDESTROYSYNCKHRPROC)__eglGetProcAddress(procname);
    return (void (*)())__eglDestroySyncKHR;
  } else if (strcmp(procname, "eglClientWaitSyncKHR") == 0) {
    _eglClientWaitSyncKHR =
        (PFNEGLCLIENTWAITSYNCKHRPROC)__eglGetProcAddress(procname);
    return (void (*)())__eglClientWaitSyncKHR;
  } else if (strcmp(procname, "eglGetSyncAttribKHR") == 0) {
    _eglGetSyncAttribKHR =
        (PFNEGLGETSYNCATTRIBKHRPROC)__eglGetProcAddress(procname);
    return (void (*)())__eglGetSyncAttribKHR;
  }

  else if (strcmp(procname, "eglCreateNativePixmapNDS") == 0) {
    _eglCreateNativePixmapNDS =
        (PFNEGLCREATENATIVEPIXMAPNDSPROC)__eglGetProcAddress(procname);
    return (void (*)())__eglCreateNativePixmapNDS;
  } else if (strcmp(procname, "eglDestroyNativePixmapNDS") == 0) {
    _eglDestroyNativePixmapNDS =
        (PFNEGLDESTROYNATIVEPIXMAPNDSPROC)__eglGetProcAddress(procname);
    return (void (*)())__eglDestroyNativePixmapNDS;
  } else if (strcmp(procname, "eglFlushNativePixmapNDS") == 0) {
    _eglFlushNativePixmapNDS =
        (PFNEGLFLUSHNATIVEPIXMAPNDSPROC)__eglGetProcAddress(procname);
    return (void (*)())__eglFlushNativePixmapNDS;
  } else if (strcmp(procname, "eglQueryNativePixmapNDS") == 0) {
    _eglQueryNativePixmapNDS =
        (PFNEGLQUERYNATIVEPIXMAPNDSPROC)__eglGetProcAddress(procname);
    return (void (*)())__eglQueryNativePixmapNDS;
  }

  void (*a)() = __eglGetProcAddress(procname);
  return a;
}

#endif

#endif
