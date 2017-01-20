#ifndef NOGLES2

#include <math.h>
#include <stdio.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <gles2util.h>

static const char *const vertShader =
    "\n\
uniform mat4 mvp;                           \n\
attribute vec4 position;                    \n\
attribute vec2 texcoords;                   \n\
varying vec2 texcoord;                      \n\
void main(void)                             \n\
{                                           \n\
  texcoord = texcoords;                     \n\
  gl_Position = mvp * position;             \n\
}                                           \n";

static const char *const fragShader =
    "\n\
uniform sampler2D tex;                      \n\
varying highp vec2 texcoord;                \n\
void main(void)                             \n\
{                                           \n\
  gl_FragColor = texture2D(tex, texcoord);  \n\
}                                           \n";

#define FLOAT_TO_FIXED(x) (long)((x)*65536.0f)

static GLfixed vertices[] = {
    FLOAT_TO_FIXED(0.5),  FLOAT_TO_FIXED(-0.5), FLOAT_TO_FIXED(-0.5),
    FLOAT_TO_FIXED(-0.5), FLOAT_TO_FIXED(-0.5), FLOAT_TO_FIXED(0.5),
    FLOAT_TO_FIXED(0.5),  FLOAT_TO_FIXED(0.5),
};

static GLfloat texcoords[] = {1.0, 1.0, 0.0, 1.0, 0.0,
                              0.0, 1.0, 0.0

};

#define WIDTH 1280
#define HEIGHT 720
#define BPP 4

static GLubyte texdata[WIDTH * HEIGHT * BPP];
static GLubyte *ptexdata = texdata;
static GLuint progid;
static GLuint texid;

void extra_init() {
  int i, j;
  for (j = 0; j < HEIGHT; j++) {
    for (i = 0; i < WIDTH; i++) {
      if ((i ^ j) & 0x80) {
        ptexdata[0] = ptexdata[1] = ptexdata[2] = 0x00;
        ptexdata[3] = 0x00;
      } else if ((i ^ j) & 0x40) {
        ptexdata[0] = ptexdata[1] = ptexdata[2] = 0xff;
        ptexdata[3] = 0xdf;
      } else {
        ptexdata[0] = ptexdata[1] = 0x00;
        ptexdata[2] = 0xff;
        ptexdata[3] = 0xdf;
      }
      ptexdata += 4;
    }
  }

  glGenTextures(1, &texid);
  glBindTexture(GL_TEXTURE_2D, texid);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, WIDTH, HEIGHT, 0, GL_BGRA_EXT,
               GL_UNSIGNED_BYTE, texdata);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  progid = create_program(vertShader, fragShader);
}

void extra_render(void) {
  static GLfloat projection[4][4];
  static GLfloat modelview[4][4];
  static GLfloat mvp[4][4];

  Identity(projection);
  Perspective(projection, 90, 1, 0, 1);

  Identity(modelview);
  Translate(modelview, 0, 0, -0.5);

  static int fr = 0;
  // Rotate(modelview, 1, 0, 0, 0 / 1.0);
  // Rotate(modelview, 0, 1, 0, 0 / 1.0);
  Rotate(modelview, 0, 0, 1, fr++ / 1.0);

  glUseProgram(progid);

  glBindTexture(GL_TEXTURE_2D, texid);

  MultiplyMatrix(mvp, modelview, projection);
  glUniformMatrix4fv(glGetUniformLocation(progid, "mvp"), 1, GL_FALSE,
                     (const GLfloat *)mvp);

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

#endif
