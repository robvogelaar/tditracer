#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

extern "C" {
#include "shadercapture.h"
#include "texturecapture.h"
#include "framecapture.h"
}

#include "tracermain.h"
#include "tracerutils.h"
#include "gldefs.h"
#include "tdi.h"

extern "C" void glReadPixels(GLint x,  GLint y,  GLsizei width,  GLsizei height,  GLenum format,  GLenum type,  GLvoid * data);

static GLuint boundtexture;
static GLuint currentprogram;
static GLuint boundframebuffer = 0;
static GLuint boundrenderbuffer = 0;
static GLuint framebuffertexture[1024] = {};
static GLuint framebufferrenderbuffer[1024] = {};
static GLuint renderbufferwidth[1024] = {};
static GLuint renderbufferheight[1024] = {};

static int draw_counter = 0;
static int texturebind_counter = 0;

static int current_frame = 1;

static int glDrawArrays_counter = 0;
static int glDrawElements_counter = 0;
static int glTexImage2D_counter = 0;
static int glTexSubImage2D_counter = 0;
static int glBindTexture_counter = 0;

static int glShaderSource_counter = 0;


extern "C" EGLBoolean eglSwapBuffers(EGLDisplay display, EGLSurface surface)
{
    static EGLBoolean (*__eglSwapBuffers)(EGLDisplay display, EGLSurface surface) = NULL;

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

    draw_counter = 0;
    texturebind_counter = 0;

    current_frame++;

    return ret;
}


extern "C" EGLBoolean eglMakeCurrent(EGLDisplay display, EGLSurface draw, EGLSurface read, EGLContext context)
{
    static EGLBoolean (*__eglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext) = NULL;

    if (__eglMakeCurrent==NULL) {
        __eglMakeCurrent = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLSurface, EGLContext))dlsym(RTLD_NEXT, "eglMakeCurrent");
        if (NULL == __eglMakeCurrent) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }


    //printf("eglMakeCurrent display=%d draw=0x%x read=0x%x context=0x%x%s,[%s]\n", display, draw, read, context, addrinfo(__builtin_return_address(0)));

    TDITRACE("@T+eglMakeCurrent() display=%d draw=0x%x read=0x%x context=0x%x", display, draw, read, context);
    EGLBoolean b = __eglMakeCurrent(display, draw, read, context);
    TDITRACE("@T-eglMakeCurrent()");

    boundframebuffer = 0;

    return b;
}


extern "C" void glFinish(void)
{
    static void (*__glFinish)(void) = NULL;

    if (__glFinish==NULL) {
        __glFinish = (void(*)(void))dlsym(RTLD_NEXT, "glFinish");
        if (NULL == __glFinish) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("@T+glFinish()");
    __glFinish();
    TDITRACE("@T-glFinish()");
}


extern "C" void glFlush(void)
{
    static void (*__glFlush)(void) = NULL;

    if (__glFlush==NULL) {
        __glFlush = (void(*)(void))dlsym(RTLD_NEXT, "glFlush");
        if (NULL == __glFlush) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("@T+glFlush()");
    __glFlush();
    TDITRACE("@T-glFlush()");
}


extern "C" GLvoid glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    static void (*__glDrawArrays)(GLenum, GLint, GLsizei) = NULL;

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
             boundframebuffer, framebuffertexture[boundframebuffer & 0x3ff], framebufferrenderbuffer[boundframebuffer & 0x3ff],
             renderbufferwidth[framebufferrenderbuffer[boundframebuffer & 0x3ff] & 0x3ff],
             renderbufferheight[framebufferrenderbuffer[boundframebuffer & 0x3ff] & 0x3ff]);

    } else {
        TDITRACE("@T+glDrawArrays() #%d,%d,%s,#i=%d,t=%u,p=%u", current_frame, glDrawArrays_counter, MODESTRING(mode), count, boundtexture, currentprogram);
    }
    __glDrawArrays(mode, first, count);

    if ((boundframebuffer != 0) && renderbufferrecording) {

        if (framebufferrenderbuffer[boundframebuffer & 0x3ff]) {

            int w = renderbufferwidth[framebufferrenderbuffer[boundframebuffer & 0x3ff] & 0x3ff];
            int h = renderbufferheight[framebufferrenderbuffer[boundframebuffer & 0x3ff] & 0x3ff];
            if (w && h) {
                unsigned char* p = (unsigned char*)malloc(w * h * 4);
                glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, p);
                texturecapture_captexture(framebuffertexture[boundframebuffer & 0x3ff], RENDER, current_frame,
                     0, 0, w, h, (int)GL_RGBA, (int)GL_UNSIGNED_BYTE, p);
                free(p);
                textures_captured++;
            }

        }
    }

    TDITRACE("@T-glDrawArrays()");
}


extern "C" GLvoid glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
    static void (*__glDrawElements)(GLenum, GLsizei, GLenum, const GLvoid *) = NULL;

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
             boundframebuffer, framebuffertexture[boundframebuffer & 0x3ff], framebufferrenderbuffer[boundframebuffer & 0x3ff],
             renderbufferwidth[framebufferrenderbuffer[boundframebuffer & 0x3ff] & 0x3ff],
             renderbufferheight[framebufferrenderbuffer[boundframebuffer & 0x3ff] & 0x3ff]);
    
    } else {
        TDITRACE("@T+glDrawElements() #%d,%d,%s,#i=%d,t=%u,p=%u", current_frame, glDrawElements_counter, MODESTRING(mode), count, boundtexture, currentprogram);
    }

    __glDrawElements(mode, count, type, indices);

    if ((boundframebuffer != 0) && renderbufferrecording) {

        if (framebufferrenderbuffer[boundframebuffer & 0x3ff]) {

            int w = renderbufferwidth[framebufferrenderbuffer[boundframebuffer & 0x3ff] & 0x3ff];
            int h = renderbufferheight[framebufferrenderbuffer[boundframebuffer & 0x3ff] & 0x3ff];
            if (w && h) {
                unsigned char* p = (unsigned char*)malloc(w * h * 4);
                glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, p);
                texturecapture_captexture(framebuffertexture[boundframebuffer & 0x3ff], RENDER, current_frame,
                     0, 0, w, h, (int)GL_RGBA, (int)GL_UNSIGNED_BYTE, p);
                free(p);
                textures_captured++;
            }
        }
    }

    TDITRACE("@T-glDrawElements()");
}


extern "C" void glGenTextures(GLsizei n, GLuint * textures)
{
    static void (*__glGenTextures)(GLsizei, GLuint *) = NULL;

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
    static void (*__glBindTexture)(GLenum, GLuint) = NULL;

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
    static void (*__glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*) = NULL;

    if (__glTexImage2D==NULL) {
        __glTexImage2D = (void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*))dlsym(RTLD_NEXT, "glTexImage2D");
        if (NULL == __glTexImage2D) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("@T+glTexImage2D() #%d,%dx%d,%s,%s,%u,0x%x", ++glTexImage2D_counter, width, height, TYPESTRING(type), FORMATSTRING(format), boundtexture, pixels);

    if (texturerecording) {
        texturecapture_captexture(boundtexture, PARENT, current_frame, 0, 0, width, height, (int)format, (int)type, pixels);
        textures_captured++;
    }

    __glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);

    TDITRACE("@T-glTexImage2D()");
}


extern "C" GLvoid glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    static void (*__glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*) = NULL;

    if (__glTexSubImage2D==NULL) {
        __glTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid* pixels))dlsym(RTLD_NEXT, "glTexSubImage2D");
        if (NULL == __glTexSubImage2D) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("@T+glTexSubImage2D() #%d,%dx%d+%d+%d,%s,%s,%u,0x%x", ++glTexSubImage2D_counter, width, height, xoffset, yoffset, TYPESTRING(type), FORMATSTRING(format), boundtexture, pixels);

    if (texturerecording) {
        texturecapture_captexture(boundtexture, SUB, current_frame, xoffset, yoffset, width, height, (int)format, (int)type, pixels);
        textures_captured++;
    }

    __glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);

    TDITRACE("@T-glTexSubImage2D()");
}


extern "C" GLvoid glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    static void (*__glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) = NULL;

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
    static void (*__glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = NULL;

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
    static void (*__glTexParameteri)(GLenum, GLenum, GLint) = NULL;

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
    static void (*__glUseProgram)(GLuint) = NULL;

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
    static void (*__glShaderSource)(GLuint, GLsizei, const GLchar**, const GLint*) = NULL;

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
    static void (*__glAttachShader)(GLuint, GLuint) = NULL;

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
    static void (*__glEnable)(GLenum) = NULL;

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
    static void (*__glDisable)(GLenum) = NULL;

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
    static void (*__glBlendFunc)(GLenum, GLenum) = NULL;

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
    static void (*__glScissor)(GLint, GLint, GLsizei, GLsizei) = NULL;

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
    static void (*__glClearColor)(GLclampf, GLclampf, GLclampf, GLclampf) = NULL;

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
    static void (*__glClear)(GLbitfield) = NULL;

    if (__glClear==NULL) {
        __glClear = (void (*)(GLbitfield))dlsym(RTLD_NEXT, "glClear");
        if (NULL == __glClear) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    //printf("%s,[%s]\n", CLEARSTRING(mask), addrinfo(__builtin_return_address(0)));

    TDITRACE("@T+glClear() %s", CLEARSTRING(mask));

    __glClear(mask);

    TDITRACE("@T-glClear()");
}


extern "C" GLvoid glGenFramebuffers(GLsizei n, GLuint * framebuffers)
{
    static void (*__glGenFramebuffers)(GLsizei, GLuint*) = NULL;

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
    static void (*__glBindFramebuffer)(GLenum, GLuint) = NULL;

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
    static void (*__glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint) = NULL;

    if (__glFramebufferTexture2D==NULL) {
        __glFramebufferTexture2D = (void (*)(GLenum, GLenum, GLenum, GLuint, GLint))dlsym(RTLD_NEXT, "glFramebufferTexture2D");
        if (NULL == __glFramebufferTexture2D) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glFramebufferTexture2D() %s,%u", ATTACHMENTSTRING(attachment), texture);

    __glFramebufferTexture2D(target, attachment, textarget, texture, level);

    framebuffertexture[boundframebuffer & 0x3ff] = texture;
}


extern "C" GLvoid glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    static void (*__glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei) = NULL;

    if (__glRenderbufferStorage==NULL) {
        __glRenderbufferStorage = (void (*)(GLenum, GLenum, GLsizei, GLsizei))dlsym(RTLD_NEXT, "glRenderbufferStorage");
        if (NULL == __glRenderbufferStorage) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glRenderbufferStorage() %s0x%x,%dx%d", IFORMATSTRING(internalformat), internalformat, width, height);

    __glRenderbufferStorage(target, internalformat, width, height);

    renderbufferwidth[boundrenderbuffer & 0x3ff] = width;
    renderbufferheight[boundrenderbuffer & 0x3ff] = height;
}


extern GLvoid glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
    static void (*__glgenRenderbuffers)(GLsizei, GLuint*) = NULL;

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
    static void (*__glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint) = NULL;

    if (__glFramebufferRenderbuffer==NULL) {
        __glFramebufferRenderbuffer = (void (*)(GLenum, GLenum, GLenum, GLuint))dlsym(RTLD_NEXT, "glFramebufferRenderbuffer");
        if (NULL == __glFramebufferRenderbuffer) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glFramebufferRenderbuffer() %s,%u", ATTACHMENTSTRING(attachment), renderbuffer);

    __glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);

    framebufferrenderbuffer[boundframebuffer & 0x3ff] = renderbuffer;
}


extern "C" GLvoid glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    static void (*__glBindRenderbuffer)(GLenum, GLuint) = NULL;

    if (__glBindRenderbuffer==NULL) {
        __glBindRenderbuffer = (void (*)(GLenum, GLuint))dlsym(RTLD_NEXT, "glBindRenderbuffer");
        if (NULL == __glBindRenderbuffer) {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
        }
    }

    TDITRACE("glBindRenderbuffer() %u", renderbuffer);

    __glBindRenderbuffer(target, renderbuffer);

    boundrenderbuffer = renderbuffer;
    framebufferrenderbuffer[boundframebuffer & 0x3ff] = renderbuffer;
}
