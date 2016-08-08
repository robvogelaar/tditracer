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
#define save_ra() int ra = 0;
#endif

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

extern "C" EGLContext eglCreateContext(EGLDisplay display, EGLConfig config,
                                       EGLContext share_context,
                                       const EGLint *attrib_list) {
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
  if (gles2recording) tditrace("eglCreateContext() =0x%x", ret);
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
        "@S+eglMakeCurrent()_%d_%d_%d_%d display=%d draw=0x%x read=0x%x "
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

  if (eglrecording) tditrace("@I+eglSwapBuffers()");

  if (gosd & 0x1) {
    spinner_render(current_frame);
    bars_render();
  }

  // capture_frame();

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

  if (framerecording) {
    tditrace("@A+FR %d", current_frame);
    static char *framebuffer = 0;
    if (!framebuffer) {
      framebuffer = (char *)memalign(4096, 1280 * 720 * 4);
    }

    glReadPixels(0, 0, 1280, 720, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer);

    tditrace("@A-FR");
    frames_captured++;
  }

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

extern "C" void glFinish(void) {
  static void (*__glFinish)(void) = NULL;

  if (__glFinish == NULL) {
    __glFinish = (void (*)(void))dlsym(RTLD_NEXT, "glFinish");
    if (NULL == __glFinish) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording) tditrace("@I+glFinish()");
  __glFinish();
  if (gles2recording) tditrace("@I-glFinish()");
}

extern "C" void glFlush(void) {
  static void (*__glFlush)(void) = NULL;

  if (__glFlush == NULL) {
    __glFlush = (void (*)(void))dlsym(RTLD_NEXT, "glFlush");
    if (NULL == __glFlush) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording) tditrace("@I+glFlush()");
  __glFlush();
  if (gles2recording) tditrace("@I-glFlush()");
}

extern "C" void glVertexAttribPointer(GLuint index, GLint size, GLenum type,
                                      GLboolean normalized, GLsizei stride,
                                      const GLvoid *pointer) {
  static void (*__glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean,
                                         GLsizei, const GLvoid *) = NULL;

  if (__glVertexAttribPointer == NULL) {
    __glVertexAttribPointer =
        (void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei,
                  const GLvoid *))dlsym(RTLD_NEXT, "glVertexAttribPointer");
    if (NULL == __glVertexAttribPointer) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording)
    tditrace("glVertexAttribPointer() %d,%d,%s,%d,0x%x", index, size,
             TYPESTRING(type), stride, pointer);

  __glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

extern "C" GLvoid glDrawArrays(GLenum mode, GLint first, GLsizei count) {
  save_ra();
  static void (*__glDrawArrays)(GLenum, GLint, GLsizei) = NULL;

  if (__glDrawArrays == NULL) {
    __glDrawArrays =
        (void (*)(GLenum, GLint, GLsizei))dlsym(RTLD_NEXT, "glDrawArrays");
    if (NULL == __glDrawArrays) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  draw_counter++;
  glDrawArrays_counter++;

  if (gles2recording || gldrawrecording) tditrace("#gldraws~%d", draw_counter);
  if (boundframebuffer != 0) {
    if (gles2recording || gldrawrecording)
      tditrace(
          "@I+glDrawArrays() "
          "@%d,#%d,%s,#i=%d,t=%u,p=%u,f=%u,ft=%u,r=%u,%ux%u%n",
          current_frame, glDrawArrays_counter, MODESTRING(mode), count,
          boundtexture, currentprogram, boundframebuffer,
          framebuffertexture[boundframebuffer & 0x3ff],
          framebufferrenderbuffer[boundframebuffer & 0x3ff],
          renderbufferwidth[framebufferrenderbuffer[boundframebuffer & 0x3ff] &
                            0x3ff],
          renderbufferheight[framebufferrenderbuffer[boundframebuffer & 0x3ff] &
                             0x3ff],
          ra);

  } else {
    if (gles2recording || gldrawrecording)
      tditrace("@I+glDrawArrays() @%d,#%d,%s,#i=%d,t=%u,p=%u%n", current_frame,
               glDrawArrays_counter, MODESTRING(mode), count, boundtexture,
               currentprogram, ra);
  }
  __glDrawArrays(mode, first, count);

  if ((boundframebuffer != 0) && renderbufferrecording) {
    if (framebufferrenderbuffer[boundframebuffer & 0x3ff]) {
      int w =
          renderbufferwidth[framebufferrenderbuffer[boundframebuffer & 0x3ff] &
                            0x3ff];
      int h =
          renderbufferheight[framebufferrenderbuffer[boundframebuffer & 0x3ff] &
                             0x3ff];
      if (w && h) {
        tditrace("@A+RR %d", framebuffertexture[boundframebuffer & 0x3ff]);
        unsigned char *p = (unsigned char *)malloc(w * h * 4);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, p);
        texturecapture_captexture(framebuffertexture[boundframebuffer & 0x3ff],
                                  RENDER, current_frame, 0, 0, w, h,
                                  (int)GL_RGBA, (int)GL_UNSIGNED_BYTE, p);
        free(p);
        tditrace("@A-RR");
        textures_captured++;
      }
    }
  }

  if (gles2recording || gldrawrecording) tditrace("@I-glDrawArrays()");
}

extern "C" GLvoid glDrawElements(GLenum mode, GLsizei count, GLenum type,
                                 const GLvoid *indices) {
  save_ra();
  static void (*__glDrawElements)(GLenum, GLsizei, GLenum, const GLvoid *) =
      NULL;

  if (__glDrawElements == NULL) {
    __glDrawElements = (void (*)(GLenum, GLsizei, GLenum, const GLvoid *))dlsym(
        RTLD_NEXT, "glDrawElements");
    if (NULL == __glDrawElements) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  draw_counter++;
  glDrawElements_counter++;

  if (gles2recording || gldrawrecording) tditrace("#gldraws~%d", draw_counter);
  if (boundframebuffer) {
    if (gles2recording || gldrawrecording)
      tditrace(
          "@I+glDrawElements() "
          "@%d,#%d,%s,%s,#i=%d,t=%u,p=%u,f=%u,ft=%u,r=%u,%ux%u%n",
          current_frame, glDrawElements_counter, MODESTRING(mode),
          TYPESTRING(type), count, boundtexture, currentprogram,
          boundframebuffer, framebuffertexture[boundframebuffer & 0x3ff],
          framebufferrenderbuffer[boundframebuffer & 0x3ff],
          renderbufferwidth[framebufferrenderbuffer[boundframebuffer & 0x3ff] &
                            0x3ff],
          renderbufferheight[framebufferrenderbuffer[boundframebuffer & 0x3ff] &
                             0x3ff],
          ra);

  } else {
    if (gles2recording || gldrawrecording)
      tditrace("@I+glDrawElements() @%d,#%d,%s,%s,#i=%d,t=%u,p=%u%n",
               current_frame, glDrawElements_counter, MODESTRING(mode),
               TYPESTRING(type), count, boundtexture, currentprogram, ra);
  }

  __glDrawElements(mode, count, type, indices);

  if ((boundframebuffer != 0) && renderbufferrecording) {
    if (framebufferrenderbuffer[boundframebuffer & 0x3ff]) {
      int w =
          renderbufferwidth[framebufferrenderbuffer[boundframebuffer & 0x3ff] &
                            0x3ff];
      int h =
          renderbufferheight[framebufferrenderbuffer[boundframebuffer & 0x3ff] &
                             0x3ff];
      if (w && h) {
        tditrace("@A+RR %d", framebuffertexture[boundframebuffer & 0x3ff]);
        unsigned char *p = (unsigned char *)malloc(w * h * 4);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, p);
        texturecapture_captexture(framebuffertexture[boundframebuffer & 0x3ff],
                                  RENDER, current_frame, 0, 0, w, h,
                                  (int)GL_RGBA, (int)GL_UNSIGNED_BYTE, p);
        free(p);
        tditrace("@A-RR");
        textures_captured++;
      }
    }
  }

  if (gles2recording || gldrawrecording) tditrace("@I-glDrawElements()");
}

extern "C" void glGenTextures(GLsizei n, GLuint *textures) {
  static void (*__glGenTextures)(GLsizei, GLuint *) = NULL;

  if (__glGenTextures == NULL) {
    __glGenTextures =
        (void (*)(GLsizei, GLuint *))dlsym(RTLD_NEXT, "glGenTextures");
    if (NULL == __glGenTextures) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording) tditrace("#gltexturegens~%d", ++texturegen_counter);

  __glGenTextures(n, textures);

  if (gles2recording) tditrace("glGenTextures() %d=%d", n, textures[0]);
}

extern "C" GLvoid glBindTexture(GLenum target, GLuint texture) {
  static void (*__glBindTexture)(GLenum, GLuint) = NULL;

  if (__glBindTexture == NULL) {
    __glBindTexture =
        (void (*)(GLenum, GLuint))dlsym(RTLD_NEXT, "glBindTexture");
    if (NULL == __glBindTexture) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording) tditrace("#gltexturebinds~%d", ++texturebind_counter);

  if (gles2recording)
    tditrace("glBindTexture() #%d,%u", ++glBindTexture_counter, texture);

  __glBindTexture(target, texture);

  boundtexture = texture;
}

extern "C" GLvoid glTexImage2D(GLenum target, GLint level, GLint internalformat,
                               GLsizei width, GLsizei height, GLint border,
                               GLenum format, GLenum type,
                               const GLvoid *pixels) {
  static void (*__glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint,
                                GLenum, GLenum, const GLvoid *) = NULL;

  if (__glTexImage2D == NULL) {
    __glTexImage2D =
        (void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum,
                  const GLvoid *))dlsym(RTLD_NEXT, "glTexImage2D");
    if (NULL == __glTexImage2D) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording || gltexturerecording)
    tditrace("@I+glTexImage2D() #%d,%dx%d,%s,%s,%u,0x%x",
             ++glTexImage2D_counter, width, height, TYPESTRING(type),
             FORMATSTRING(format), boundtexture, pixels);

  if (texturerecording) {
    tditrace("@A+TR %d", boundtexture);
    texturecapture_captexture(boundtexture, PARENT, current_frame, 0, 0, width,
                              height, (int)format, (int)type, pixels);
    tditrace("@A-TR");
    textures_captured++;
  }

  __glTexImage2D(target, level, internalformat, width, height, border, format,
                 type, pixels);

  if (gles2recording || gltexturerecording) tditrace("@I-glTexImage2D()");

  GLenum err = GL_NO_ERROR;
  while ((err = glGetError()) != GL_NO_ERROR) {
    tditrace("@S+GLERROR 0x%x", err);
  }
}

extern "C" GLvoid glTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                  GLint yoffset, GLsizei width, GLsizei height,
                                  GLenum format, GLenum type,
                                  const GLvoid *pixels) {
  static void (*__glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei,
                                   GLsizei, GLenum, GLenum, const GLvoid *) =
      NULL;

  if (__glTexSubImage2D == NULL) {
    __glTexSubImage2D =
        (void (*)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum,
                  const GLvoid *pixels))dlsym(RTLD_NEXT, "glTexSubImage2D");
    if (NULL == __glTexSubImage2D) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording || gltexturerecording)
    tditrace("@I+glTexSubImage2D() #%d,%dx%d+%d+%d,%s,%s,%u,0x%x",
             ++glTexSubImage2D_counter, width, height, xoffset, yoffset,
             TYPESTRING(type), FORMATSTRING(format), boundtexture, pixels);

  if (texturerecording) {
    tditrace("@A+TR %d", boundtexture);
    texturecapture_captexture(boundtexture, SUB, current_frame, xoffset,
                              yoffset, width, height, (int)format, (int)type,
                              pixels);
    tditrace("@A-TR");
    textures_captured++;
  }

  __glTexSubImage2D(target, level, xoffset, yoffset, width, height, format,
                    type, pixels);

  if (gles2recording || gltexturerecording) tditrace("@I-glTexSubImage2D()");
}

extern "C" GLvoid glCopyTexImage2D(GLenum target, GLint level,
                                   GLenum internalformat, GLint x, GLint y,
                                   GLsizei width, GLsizei height,
                                   GLint border) {
  static void (*__glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint,
                                    GLsizei, GLsizei, GLint) = NULL;

  if (__glCopyTexImage2D == NULL) {
    __glCopyTexImage2D =
        (void (*)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei,
                  GLint))dlsym(RTLD_NEXT, "glCopyTexImage2D");
    if (NULL == __glCopyTexImage2D) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording)
    tditrace("@I+glCopyTexImage2D() %dx%d+%d+%d,%u", width, height, x, y,
             boundtexture);

  __glCopyTexImage2D(target, level, internalformat, x, y, width, height,
                     border);

  if (gles2recording) tditrace("@I-glCopyTexImage2D()");
}

extern "C" GLvoid glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                      GLint yoffset, GLint x, GLint y,
                                      GLsizei width, GLsizei height) {
  static void (*__glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint,
                                       GLint, GLsizei, GLsizei) = NULL;

  if (__glCopyTexSubImage2D == NULL) {
    __glCopyTexSubImage2D =
        (void (*)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei,
                  GLsizei))dlsym(RTLD_NEXT, "glCopyTexSubImage2D");
    if (NULL == __glCopyTexSubImage2D) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording)
    tditrace("@I+glCopyTexSubImage2D() %dx%d+%d+%d+%d+%d,%u", width, height, x,
             y, xoffset, yoffset, boundtexture);

  __glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);

  if (gles2recording) tditrace("@I-glCopyTexSubImage2D()");
}

extern "C" GLvoid glTexParameteri(GLenum target, GLenum pname, GLint param) {
  static void (*__glTexParameteri)(GLenum, GLenum, GLint) = NULL;

  if (__glTexParameteri == NULL) {
    __glTexParameteri =
        (void (*)(GLenum, GLenum, GLint))dlsym(RTLD_NEXT, "glTexParameteri");
    if (NULL == __glTexParameteri) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  __glTexParameteri(target, pname, param);
}

extern "C" GLvoid glUseProgram(GLuint program) {
  static void (*__glUseProgram)(GLuint) = NULL;

  if (__glUseProgram == NULL) {
    __glUseProgram = (void (*)(GLuint program))dlsym(RTLD_NEXT, "glUseProgram");
    if (NULL == __glUseProgram) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  __glUseProgram(program);

  currentprogram = program;
}

extern "C" GLvoid glShaderSource(GLuint shader, GLsizei count,
                                 const GLchar **string, const GLint *length) {
  static void (*__glShaderSource)(GLuint, GLsizei, const GLchar **,
                                  const GLint *) = NULL;

  if (__glShaderSource == NULL) {
    __glShaderSource =
        (void (*)(GLuint, GLsizei, const GLchar **, const GLint *))dlsym(
            RTLD_NEXT, "glShaderSource");
    if (NULL == __glShaderSource) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording)
    tditrace("glShaderSource() #%d %u", ++glShaderSource_counter, shader);

  if (shaderrecording) {
    shadercapture_capshader(shader, count, string, length);
    shaders_captured++;
  }

  __glShaderSource(shader, count, string, length);
}

extern "C" GLvoid glAttachShader(GLuint program, GLuint shader) {
  static void (*__glAttachShader)(GLuint, GLuint) = NULL;

  if (__glAttachShader == NULL) {
    __glAttachShader =
        (void (*)(GLuint, GLuint))dlsym(RTLD_NEXT, "glAttachShader");
    if (NULL == __glAttachShader) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording) tditrace("glAttachShader() %u %u", program, shader);

  if (shaderrecording) {
    shadercapture_referenceprogram(shader, program);
  }

  __glAttachShader(program, shader);
  // tditrace("@E+glAttachShader() %u %u", shader, program);
}

extern "C" GLvoid glDetachShader(GLuint program, GLuint shader) {
  static void (*__glDetachShader)(GLuint, GLuint) = NULL;

  if (__glDetachShader == NULL) {
    __glDetachShader =
        (void (*)(GLuint, GLuint))dlsym(RTLD_NEXT, "glDetachShader");
    if (NULL == __glDetachShader) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording) tditrace("glDetachShader() %u %u", program, shader);

#if 0
    if (shaderrecording) {
        shadercapture_referenceprogram(shader, program);
    }
#endif

  __glDetachShader(program, shader);
  // tditrace("@E+glDetachShader() %u %u", shader, program);
}

#if 0
extern "C" GLvoid glEnable(GLenum cap)
{
    static void (*__glEnable)(GLenum) = NULL;

    if (__glEnable==NULL) {
        __glEnable = (void (*)(GLenum))dlsym(RTLD_NEXT, "glEnable");
        if (NULL == __glEnable) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (gles2recording) tditrace("glEnable() %s", CAPSTRING(cap));

    __glEnable(cap);
}
#endif

#if 0
extern "C" GLvoid glDisable(GLenum cap)
{
    static void (*__glDisable)(GLenum) = NULL;

    if (__glDisable==NULL) {
        __glDisable = (void (*)(GLenum))dlsym(RTLD_NEXT, "glDisable");
        if (NULL == __glDisable) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (gles2recording) tditrace("glDisable() %s", CAPSTRING(cap));

    __glDisable(cap);
}
#endif

#if 0
extern "C" GLvoid glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    static void (*__glBlendFunc)(GLenum, GLenum) = NULL;

    if (__glBlendFunc==NULL) {
        __glBlendFunc = (void (*)(GLenum, GLenum))dlsym(RTLD_NEXT, "glBlendFunc");
        if (NULL == __glBlendFunc) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (gles2recording) tditrace("glBlendFunc() %s %s", BLENDSTRING(sfactor), BLENDSTRING(dfactor));

    __glBlendFunc(sfactor, dfactor);
}
#endif

#if 0
extern "C" GLvoid glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    static void (*__glScissor)(GLint, GLint, GLsizei, GLsizei) = NULL;

    if (__glScissor==NULL) {
        __glScissor = (void (*)(GLint, GLint, GLsizei, GLsizei))dlsym(RTLD_NEXT, "glScissor");
        if (NULL == __glScissor) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (gles2recording) tditrace("glScissor() %dx%d+%d+%d", width, height, x, y);

    __glScissor(x, y, width, height);
}
#endif

#if 0
extern "C" GLvoid glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    static void (*__glClearColor)(GLclampf, GLclampf, GLclampf, GLclampf) = NULL;

    if (__glClearColor==NULL) {
        __glClearColor = (void (*)(GLclampf, GLclampf, GLclampf, GLclampf))dlsym(RTLD_NEXT, "glClearColor");
        if (NULL == __glClearColor) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    if (gles2recording) tditrace("glClearColor() %g %g %g %g", red, green, blue, alpha);

    __glClearColor(red, green, blue, alpha);
}
#endif

extern "C" GLvoid glClear(GLbitfield mask) {
  static void (*__glClear)(GLbitfield) = NULL;

  if (__glClear == NULL) {
    __glClear = (void (*)(GLbitfield))dlsym(RTLD_NEXT, "glClear");
    if (NULL == __glClear) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording || gldrawrecording || gltexturerecording)
    tditrace("@I+glClear() %s", CLEARSTRING(mask));

  __glClear(mask);

  if (gles2recording || gldrawrecording || gltexturerecording)
    tditrace("@I-glClear()");
}

extern "C" void glGenBuffers(GLsizei n, GLuint *buffers) {
  static void (*__glGenBuffers)(GLsizei, GLuint *) = NULL;
  if (__glGenBuffers == NULL) {
    __glGenBuffers =
        (void (*)(GLsizei, GLuint *))dlsym(RTLD_NEXT, "glGenBuffers");
    if (__glGenBuffers == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  __glGenBuffers(n, buffers);
  if (gles2recording) tditrace("glGenBuffers() %d=%d", n, buffers[0]);
}

extern "C" void glDeleteBuffers(GLsizei n, const GLuint *buffers) {
  static void (*__glDeleteBuffers)(GLsizei, const GLuint *) = NULL;
  if (__glDeleteBuffers == NULL) {
    __glDeleteBuffers =
        (void (*)(GLsizei, const GLuint *))dlsym(RTLD_NEXT, "glDeleteBuffers");
    if (__glDeleteBuffers == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glDeleteBuffers() %d=%d", n, buffers[0]);
  __glDeleteBuffers(n, buffers);
}

extern "C" GLvoid glBindBuffer(GLenum target, GLuint buffer) {
  static void (*__glBindBuffer)(GLenum, GLuint) = NULL;

  if (__glBindBuffer == NULL) {
    __glBindBuffer = (void (*)(GLenum, GLuint))dlsym(RTLD_NEXT, "glBindBuffer");
    if (NULL == __glBindBuffer) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording)
    tditrace("glBindBuffer() %s,%u", TARGETSTRING(target), buffer);
  __glBindBuffer(target, buffer);
  currentbuffer = buffer;
}

extern "C" void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data,
                             GLenum usage) {
  static void (*__glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum) =
      NULL;
  if (__glBufferData == NULL) {
    __glBufferData = (void (*)(GLenum, GLsizeiptr, const GLvoid *,
                               GLenum))dlsym(RTLD_NEXT, "glBufferData");
    if (__glBufferData == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording)
    tditrace("glBufferData() b=%d,%s,%d", currentbuffer, TARGETSTRING(target),
             size);
  __glBufferData(target, size, data, usage);
}

extern "C" void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size,
                                const GLvoid *data) {
  static void (*__glBufferSubData)(GLenum, GLintptr, GLsizeiptr,
                                   const GLvoid *) = NULL;
  if (__glBufferSubData == NULL) {
    __glBufferSubData =
        (void (*)(GLenum, GLintptr, GLsizeiptr, const GLvoid *))dlsym(
            RTLD_NEXT, "glBufferSubData");
    if (__glBufferSubData == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  if (gles2recording)
    tditrace("glBufferSubData() b=%d,%s,%d,%d", currentbuffer,
             TARGETSTRING(target), offset, size);
  __glBufferSubData(target, offset, size, data);
}

extern "C" GLvoid glGenFramebuffers(GLsizei n, GLuint *framebuffers) {
  static void (*__glGenFramebuffers)(GLsizei, GLuint *) = NULL;

  if (__glGenFramebuffers == NULL) {
    __glGenFramebuffers =
        (void (*)(GLsizei, GLuint *))dlsym(RTLD_NEXT, "glGenFramebuffers");
    if (NULL == __glGenFramebuffers) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  __glGenFramebuffers(n, framebuffers);
  if (gles2recording) tditrace("glGenFramebuffers() %d=%d", n, framebuffers[0]);
}

extern "C" GLvoid glBindFramebuffer(GLenum target, GLuint framebuffer) {
  static void (*__glBindFramebuffer)(GLenum, GLuint) = NULL;

  if (__glBindFramebuffer == NULL) {
    __glBindFramebuffer =
        (void (*)(GLenum, GLuint))dlsym(RTLD_NEXT, "glBindFramebuffer");
    if (NULL == __glBindFramebuffer) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording) tditrace("glBindFramebuffer() %u", framebuffer);
  __glBindFramebuffer(target, framebuffer);
  boundframebuffer = framebuffer;
}

extern "C" GLvoid glFramebufferTexture2D(GLenum target, GLenum attachment,
                                         GLenum textarget, GLuint texture,
                                         GLint level) {
  static void (*__glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint,
                                          GLint) = NULL;

  if (__glFramebufferTexture2D == NULL) {
    __glFramebufferTexture2D =
        (void (*)(GLenum, GLenum, GLenum, GLuint, GLint))dlsym(
            RTLD_NEXT, "glFramebufferTexture2D");
    if (NULL == __glFramebufferTexture2D) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording)
    tditrace("glFramebufferTexture2D() %s,%u", ATTACHMENTSTRING(attachment),
             texture);
  __glFramebufferTexture2D(target, attachment, textarget, texture, level);
  framebuffertexture[boundframebuffer & 0x3ff] = texture;
}

extern "C" GLvoid glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) {
  static void (*__glGenRenderbuffers)(GLsizei, GLuint *) = NULL;

  if (__glGenRenderbuffers == NULL) {
    __glGenRenderbuffers =
        (void (*)(GLsizei, GLuint *))dlsym(RTLD_NEXT, "glGenRenderbuffers");
    if (NULL == __glGenRenderbuffers) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  __glGenRenderbuffers(n, renderbuffers);
  if (gles2recording)
    tditrace("glGenRenderbuffers() %d=%d", n, renderbuffers[0]);
}

extern "C" void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) {
  static void (*__glDeleteRenderbuffers)(GLsizei, const GLuint *) = NULL;
  if (__glDeleteRenderbuffers == NULL) {
    __glDeleteRenderbuffers = (void (*)(GLsizei, const GLuint *))dlsym(
        RTLD_NEXT, "glDeleteRenderbuffers");
    if (__glDeleteRenderbuffers == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording)
    tditrace("glDeleteRenderbuffers() %d,%d", n, renderbuffers[0]);
  __glDeleteRenderbuffers(n, renderbuffers);
}

extern "C" GLvoid glRenderbufferStorage(GLenum target, GLenum internalformat,
                                        GLsizei width, GLsizei height) {
  static void (*__glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei) =
      NULL;

  if (__glRenderbufferStorage == NULL) {
    __glRenderbufferStorage = (void (*)(GLenum, GLenum, GLsizei, GLsizei))dlsym(
        RTLD_NEXT, "glRenderbufferStorage");
    if (NULL == __glRenderbufferStorage) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording)
    tditrace("glRenderbufferStorage() %s,0x%x,%dx%d",
             IFORMATSTRING(internalformat), internalformat, width, height);

  __glRenderbufferStorage(target, internalformat, width, height);

  renderbufferwidth[boundrenderbuffer & 0x3ff] = width;
  renderbufferheight[boundrenderbuffer & 0x3ff] = height;
}

extern "C" GLvoid glFramebufferRenderbuffer(GLenum target, GLenum attachment,
                                            GLenum renderbuffertarget,
                                            GLuint renderbuffer) {
  static void (*__glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint) =
      NULL;

  if (__glFramebufferRenderbuffer == NULL) {
    __glFramebufferRenderbuffer =
        (void (*)(GLenum, GLenum, GLenum, GLuint))dlsym(
            RTLD_NEXT, "glFramebufferRenderbuffer");
    if (NULL == __glFramebufferRenderbuffer) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording)
    tditrace("glFramebufferRenderbuffer() %s,%u", ATTACHMENTSTRING(attachment),
             renderbuffer);

  __glFramebufferRenderbuffer(target, attachment, renderbuffertarget,
                              renderbuffer);

  framebufferrenderbuffer[boundframebuffer & 0x3ff] = renderbuffer;
}

extern "C" GLvoid glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
  static void (*__glBindRenderbuffer)(GLenum, GLuint) = NULL;

  if (__glBindRenderbuffer == NULL) {
    __glBindRenderbuffer =
        (void (*)(GLenum, GLuint))dlsym(RTLD_NEXT, "glBindRenderbuffer");
    if (NULL == __glBindRenderbuffer) {
      fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
  }

  if (gles2recording) tditrace("glBindRenderbuffer() %u", renderbuffer);

  __glBindRenderbuffer(target, renderbuffer);

  boundrenderbuffer = renderbuffer;
  framebufferrenderbuffer[boundframebuffer & 0x3ff] = renderbuffer;
}

#if 0
extern "C" GLuint glCreateProgram(void) {
    static GLuint (*__glCreateProgram)(void) = NULL;

    if (__glCreateProgram == NULL) {
        __glCreateProgram = (GLuint (*)(void))dlsym(RTLD_NEXT, "glCreateProgram");
        if (NULL == __glCreateProgram) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    GLuint program = __glCreateProgram();
    tditrace("@E+glCreateProgram() %u", program);

    return program;
}


extern "C" void glLinkProgram(GLuint program) {
    static void (*__glLinkProgram)(GLuint) = NULL;

    if (__glLinkProgram == NULL) {
        __glLinkProgram = (void (*)(GLuint))dlsym(RTLD_NEXT, "glLinkProgram");
        if (NULL == __glLinkProgram) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    //tditrace("@I+glLinkProgram() %d", program);
    __glLinkProgram(program);
    //tditrace("@I-glLinkProgram() %d", program);

}

extern "C" void glDeleteProgram(GLuint program) {
    static void (*__glDeleteProgram)(GLuint) = NULL;

    if (__glDeleteProgram == NULL) {
        __glDeleteProgram = (void (*)(GLuint))dlsym(RTLD_NEXT, "glDeleteProgram");
        if (NULL == __glDeleteProgram) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    __glDeleteProgram(program);
    tditrace("@E+glDeleteProgram() %d", program);



    GLint params = 0;
    GLint curprog = 0;

    glGetIntegerv(GL_CURRENT_PROGRAM, &curprog);
    tditrace("@E+glDeleteProgram() %d GL_CURRENT_PROGRAM=%d", program, curprog);
    
    if (glIsProgram(program)) {
        glGetProgramiv(program, GL_DELETE_STATUS, &params); 
        tditrace("@E+glDeleteProgram() %d GL_DELETE_STATUS=%d", program, params);

        glGetProgramiv(program, GL_ATTACHED_SHADERS, &params); 
        tditrace("@E+glDeleteProgram() %d GL_ATTACHED_SHADERS=%d", program, params);

        if (program == curprog) {

            glUseProgram(0);

            __glDeleteProgram(program);

            if (glIsProgram(program)) {
                tditrace("@E+glDeleteProgram() %d is_not_deleted", program);

                glGetProgramiv(program, GL_DELETE_STATUS, &params); 
                tditrace("@E+glDeleteProgram() %d GL_DELETE_STATUS=%d", program, params);

                glGetProgramiv(program, GL_ATTACHED_SHADERS, &params); 
                tditrace("@E+glDeleteProgram() %d GL_ATTACHED_SHADERS=%d", program, params);

            } else {
                tditrace("@E+glDeleteProgram() %d is_deleted", program);
            }
        }

    } else {
        tditrace("@E+glDeleteProgram() %d is_deleted", program);
    }

    GLenum err = GL_NO_ERROR;
    while ((err = glGetError()) != GL_NO_ERROR) {
        tditrace("@E+GLERROR 0x%x", err);
    }

}

extern "C" GLuint glCreateShader(GLenum shaderType){
    static GLuint (*__glCreateShader)(GLenum) = NULL;

    if (__glCreateShader == NULL) {
        __glCreateShader = (GLuint (*)(GLenum))dlsym(RTLD_NEXT, "glCreateShader");
        if (NULL == __glCreateShader) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    GLuint shader = __glCreateShader(shaderType);
    tditrace("@E+glCreateShader() %u", shader);

    return shader;
}

extern "C" void glDeleteShader(GLuint shader) {
    static void (*__glDeleteShader)(GLuint) = NULL;

    if (__glDeleteShader == NULL) {
        __glDeleteShader = (void (*)(GLuint))dlsym(RTLD_NEXT, "glDeleteShader");
        if (NULL == __glDeleteShader) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    __glDeleteShader(shader);
    tditrace("@E+glDeleteShader() %u", shader);
}
#endif

extern "C" void glUniform1i(GLint location, GLint v0) {
  static void (*__glUniform1i)(GLint, GLint) = NULL;
  if (__glUniform1i == NULL) {
    __glUniform1i = (void (*)(GLint, GLint))dlsym(RTLD_NEXT, "glUniform1i");
    if (__glUniform1i == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform1i() location=%x,v0=%d", location, v0);
  __glUniform1i(location, v0);
}

extern "C" void glUniform2i(GLint location, GLint v0, GLint v1) {
  static void (*__glUniform2i)(GLint, GLint, GLint) = NULL;
  if (__glUniform2i == NULL) {
    __glUniform2i =
        (void (*)(GLint, GLint, GLint))dlsym(RTLD_NEXT, "glUniform2i");
    if (__glUniform2i == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform2i()");
  __glUniform2i(location, v0, v1);
}

extern "C" void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
  static void (*__glUniform3i)(GLint, GLint, GLint, GLint) = NULL;
  if (__glUniform3i == NULL) {
    __glUniform3i =
        (void (*)(GLint, GLint, GLint, GLint))dlsym(RTLD_NEXT, "glUniform3i");
    if (__glUniform3i == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform3i()");
  __glUniform3i(location, v0, v1, v2);
}

extern "C" void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2,
                            GLint v3) {
  static void (*__glUniform4i)(GLint, GLint, GLint, GLint, GLint) = NULL;
  if (__glUniform4i == NULL) {
    __glUniform4i = (void (*)(GLint, GLint, GLint, GLint, GLint))dlsym(
        RTLD_NEXT, "glUniform4i");
    if (__glUniform4i == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform4i()");
  __glUniform4i(location, v0, v1, v2, v3);
}

extern "C" void glUniform1f(GLint location, GLfloat v0) {
  static void (*__glUniform1f)(GLint, GLfloat) = NULL;
  if (__glUniform1f == NULL) {
    __glUniform1f = (void (*)(GLint, GLfloat))dlsym(RTLD_NEXT, "glUniform1f");
    if (__glUniform1f == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  if (gles2recording) {
    char v0_str[256];
    sprintf(v0_str, "%f", v0);
    tditrace("glUniform1f() location=%x,v0=%s", location, v0_str);
  }
  __glUniform1f(location, v0);
}

extern "C" void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
  static void (*__glUniform2f)(GLint, GLfloat, GLfloat) = NULL;
  if (__glUniform2f == NULL) {
    __glUniform2f =
        (void (*)(GLint, GLfloat, GLfloat))dlsym(RTLD_NEXT, "glUniform2f");
    if (__glUniform2f == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform2f()");
  __glUniform2f(location, v0, v1);
}

extern "C" void glUniform3f(GLint location, GLfloat v0, GLfloat v1,
                            GLfloat v2) {
  static void (*__glUniform3f)(GLint, GLfloat, GLfloat, GLfloat) = NULL;
  if (__glUniform3f == NULL) {
    __glUniform3f = (void (*)(GLint, GLfloat, GLfloat, GLfloat))dlsym(
        RTLD_NEXT, "glUniform3f");
    if (__glUniform3f == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform3f()");
  __glUniform3f(location, v0, v1, v2);
}

extern "C" void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2,
                            GLfloat v3) {
  static void (*__glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) =
      NULL;
  if (__glUniform4f == NULL) {
    __glUniform4f = (void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat))dlsym(
        RTLD_NEXT, "glUniform4f");
    if (__glUniform4f == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform4f()");
  __glUniform4f(location, v0, v1, v2, v3);
}

extern "C" void glUniform1iv(GLint location, GLsizei count,
                             const GLint *value) {
  static void (*__glUniform1iv)(GLint, GLsizei, const GLint *) = NULL;
  if (__glUniform1iv == NULL) {
    __glUniform1iv = (void (*)(GLint, GLsizei, const GLint *))dlsym(
        RTLD_NEXT, "glUniform1iv");
    if (__glUniform1iv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform1iv()");
  __glUniform1iv(location, count, value);
}

extern "C" void glUniform2iv(GLint location, GLsizei count,
                             const GLint *value) {
  static void (*__glUniform2iv)(GLint, GLsizei, const GLint *) = NULL;
  if (__glUniform2iv == NULL) {
    __glUniform2iv = (void (*)(GLint, GLsizei, const GLint *))dlsym(
        RTLD_NEXT, "glUniform2iv");
    if (__glUniform2iv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform2iv()");
  __glUniform2iv(location, count, value);
}

extern "C" void glUniform3iv(GLint location, GLsizei count,
                             const GLint *value) {
  static void (*__glUniform3iv)(GLint, GLsizei, const GLint *) = NULL;
  if (__glUniform3iv == NULL) {
    __glUniform3iv = (void (*)(GLint, GLsizei, const GLint *))dlsym(
        RTLD_NEXT, "glUniform3iv");
    if (__glUniform3iv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform3iv()");
  __glUniform3iv(location, count, value);
}

extern "C" void glUniform4iv(GLint location, GLsizei count,
                             const GLint *value) {
  static void (*__glUniform4iv)(GLint, GLsizei, const GLint *) = NULL;
  if (__glUniform4iv == NULL) {
    __glUniform4iv = (void (*)(GLint, GLsizei, const GLint *))dlsym(
        RTLD_NEXT, "glUniform4iv");
    if (__glUniform4iv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform4iv()");
  __glUniform4iv(location, count, value);
}

extern "C" void glUniform1fv(GLint location, GLsizei count,
                             const GLfloat *value) {
  static void (*__glUniform1fv)(GLint, GLsizei, const GLfloat *) = NULL;
  if (__glUniform1fv == NULL) {
    __glUniform1fv = (void (*)(GLint, GLsizei, const GLfloat *))dlsym(
        RTLD_NEXT, "glUniform1fv");
    if (__glUniform1fv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform1fv()");
  __glUniform1fv(location, count, value);
}

extern "C" void glUniform2fv(GLint location, GLsizei count,
                             const GLfloat *value) {
  static void (*__glUniform2fv)(GLint, GLsizei, const GLfloat *) = NULL;
  if (__glUniform2fv == NULL) {
    __glUniform2fv = (void (*)(GLint, GLsizei, const GLfloat *))dlsym(
        RTLD_NEXT, "glUniform2fv");
    if (__glUniform2fv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniform2fv()");
  __glUniform2fv(location, count, value);
}

extern "C" void glUniform3fv(GLint location, GLsizei count,
                             const GLfloat *value) {
  static void (*__glUniform3fv)(GLint, GLsizei, const GLfloat *) = NULL;
  if (__glUniform3fv == NULL) {
    __glUniform3fv = (void (*)(GLint, GLsizei, const GLfloat *))dlsym(
        RTLD_NEXT, "glUniform3fv");
    if (__glUniform3fv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  tditrace("glUniform3fv()");
  __glUniform3fv(location, count, value);
}

extern "C" void glUniform4fv(GLint location, GLsizei count,
                             const GLfloat *value) {
  static void (*__glUniform4fv)(GLint, GLsizei, const GLfloat *) = NULL;
  if (__glUniform4fv == NULL) {
    __glUniform4fv = (void (*)(GLint, GLsizei, const GLfloat *))dlsym(
        RTLD_NEXT, "glUniform4fv");
    if (__glUniform4fv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  if (gles2recording) {
    char value_str[16];
    char value_strs[256] = "\0";
    int i;
    for (i = 0; i < 4; i++) {
      sprintf(value_str, "%s%f", i == 0 ? "" : ",", value[i]);
      strcat(value_strs, value_str);
    }
    tditrace("glUniform4fv() location=%x,count=%d,value[]=%s", location, count,
             value_strs);
  }
  __glUniform4fv(location, count, value);
}

extern "C" void glUniformMatrix2fv(GLint location, GLsizei count,
                                   GLboolean transpose, const GLfloat *value) {
  static void (*__glUniformMatrix2fv)(GLint, GLsizei, GLboolean,
                                      const GLfloat *) = NULL;
  if (__glUniformMatrix2fv == NULL) {
    __glUniformMatrix2fv =
        (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(
            RTLD_NEXT, "glUniformMatrix2fv");
    if (__glUniformMatrix2fv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }
  if (gles2recording) tditrace("glUniformMatrix2fv()");
  __glUniformMatrix2fv(location, count, transpose, value);
}

extern "C" void glUniformMatrix3fv(GLint location, GLsizei count,
                                   GLboolean transpose, const GLfloat *value) {
  static void (*__glUniformMatrix3fv)(GLint, GLsizei, GLboolean,
                                      const GLfloat *) = NULL;
  if (__glUniformMatrix3fv == NULL) {
    __glUniformMatrix3fv =
        (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(
            RTLD_NEXT, "glUniformMatrix3fv");
    if (__glUniformMatrix3fv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  if (gles2recording) {
    char value_str[16];
    char value_strs[256] = "\0";
    int i;
    for (i = 0; i < 9; i++) {
      sprintf(value_str, "%s%f", i == 0 ? "" : ",", value[i]);
      strcat(value_strs, value_str);
    }
    tditrace("glUniformMatrix3fv() location=%x,count=%d,value[]=%s", location,
             count, value_strs);
  }
  __glUniformMatrix3fv(location, count, transpose, value);
}

extern "C" void glUniformMatrix4fv(GLint location, GLsizei count,
                                   GLboolean transpose, const GLfloat *value) {
  static void (*__glUniformMatrix4fv)(GLint, GLsizei, GLboolean,
                                      const GLfloat *) = NULL;
  if (__glUniformMatrix4fv == NULL) {
    __glUniformMatrix4fv =
        (void (*)(GLint, GLsizei, GLboolean, const GLfloat *))dlsym(
            RTLD_NEXT, "glUniformMatrix4fv");
    if (__glUniformMatrix4fv == NULL) {
      fprintf(stderr, "Error in dlsym: %s\n", dlerror());
    }
  }

  if (gles2recording) {
    char value_str[16];
    char value_strs[256] = "\0";
    int i;
    for (i = 0; i < 16; i++) {
      sprintf(value_str, "%s%f", i == 0 ? "" : ",", value[i]);
      strcat(value_strs, value_str);
    }
    tditrace("glUniformMatrix4fv() location=%x,count=%d,value[]=%s", location,
             count, value_strs);
  }
  __glUniformMatrix4fv(location, count, transpose, value);
}
