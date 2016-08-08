#include <malloc.h>
#include <stdio.h>


extern "C" {
#include "texturecapture.h"
#include "framecapture.h"
}

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <gles2util.h>

static const char *const vertShader =
    "\n\
attribute vec4 position;                    \n\
attribute vec2 texcoords;                   \n\
varying vec2 texcoord;                      \n\
void main(void)                             \n\
{                                           \n\
  texcoord = texcoords;                     \n\
  gl_Position = position;                   \n\
}                                           \n";

static const char *const fragShader =
    "\n\
uniform sampler2D tex;                      \n\
varying highp vec2 texcoord;                \n\
void main(void)                             \n\
{                                           \n\
  gl_FragColor = texture2D(tex, texcoord);  \n\
}                                           \n";

//  triangle_fan
//  2 components per vertex attribute : x,y
//  2 components per texcoord attribute : u,v
//
// vertices:
//
//  3 ___ 4
//   |\  |
//   | \ |
//  2|__\|1
//
//    x   y
//    1  -1  1 (right bottom)
//   -1  -1  2 (left  bottom)
//   -1   1  3 (left  top   )
//    1   1  4 (right top   )
//
// texcoords:
//
//  3 ___ 4
//   |\  |
//   | \ |
//  2|__\|1
//
//    u   v
//    1   0  4 (right bottom)
//    0   0  3 (left  bottom)
//    0   1  2 (left  top   )
//    1   1  1 (right top   )
//

#define FLOAT_TO_FIXED(x) (long)((x)*65536.0f)
static GLfixed vertices[] = {
    FLOAT_TO_FIXED(1.0),  FLOAT_TO_FIXED(-1.0), FLOAT_TO_FIXED(-1.0),
    FLOAT_TO_FIXED(-1.0), FLOAT_TO_FIXED(-1.0), FLOAT_TO_FIXED(1.0),
    FLOAT_TO_FIXED(1.0),  FLOAT_TO_FIXED(1.0),
};

static GLfloat texcoords[] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
static GLuint progid;

static void draw_texture(GLuint texid) {
  glUseProgram(progid);
  glBindTexture(GL_TEXTURE_2D, texid);
  GLint attriblocation = glGetAttribLocation(progid, "position");
  glEnableVertexAttribArray(attriblocation);
  glVertexAttribPointer(attriblocation, 2, GL_FIXED, 0, 0, vertices);
  attriblocation = glGetAttribLocation(progid, "texcoords");
  glEnableVertexAttribArray(attriblocation);
  glVertexAttribPointer(attriblocation, 2, GL_FLOAT, 0, 0, texcoords);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  GLenum e = glGetError();
  if (e != GL_NO_ERROR) {
    fprintf(stderr, "GL ERROR = 0x%x\n", e);
  }
}

static char *pixelbuffer = 0;
static GLuint texids[10];

static int frame;

static void capture_init() {
  int i;
  for (i = 0; i < 10; i++) {
    glGenTextures(1, &texids[i]);
    glBindTexture(GL_TEXTURE_2D, texids[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  progid = create_program(vertShader, fragShader);

  pixelbuffer = (char *)memalign(4096, 1280 * 720 * 4);

  frame = 0;
}

extern "C" void capture_dump_frames();
extern "C" void capture_dump_textures();

extern void capture_frame() {
  static bool inited = false;
  if (!inited) {
    capture_init();
    inited = true;
  }

  glBindTexture(GL_TEXTURE_2D, texids[frame]);
  glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1280, 720, 0);

  frame = (frame + 1) % 10;

  static int cnt = 0;
  cnt++;
  if (cnt == 30) capture_dump_textures();
}

extern "C" void capture_dump_frames() {
  int i;
  for (i = 0; i < 10; i++) {
    fprintf(stderr, "capture_dump() %d\n", i);
    draw_texture(texids[i]);

    glReadPixels(0, 0, 1280, 720, GL_RGBA, GL_UNSIGNED_BYTE, pixelbuffer);
    framecapture_capframe(pixelbuffer);
  }
}

extern "C" void capture_dump_textures() {
  int i;
  for (i = 0; i < 100; i++) {
    if (glIsTexture(i)) {
      fprintf(stderr, "capture_dump_textures() %d\n", i);
      draw_texture(i);

      glReadPixels(0, 0, 1280, 720, GL_RGBA, GL_UNSIGNED_BYTE, pixelbuffer);

      texturecapture_captexture(i, PARENT, 10, 0, 0, 1280, 720, GL_RGBA,
                                GL_UNSIGNED_BYTE, pixelbuffer);
    }
  }
}