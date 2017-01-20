#ifndef NOGLES2

#include <math.h>
#include <stdio.h>
#include <sys/time.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <gles2text.h>
#include <gles2util.h>

typedef unsigned long long _u64;

static _u64 timestamp_usec(void) {
  struct timeval mytime;
  gettimeofday(&mytime, 0);
  return ((_u64)((_u64)mytime.tv_usec + (_u64)mytime.tv_sec * (_u64)1000000));
}

static const char *const vertShader =
    "\n\
uniform mat4 mvp;                     \n\
attribute vec4 position;              \n\
void main(void)                       \n\
{                                     \n\
  gl_Position = mvp * position;       \n\
}                                     \n";

static const char *const fragShader =
    "\n\
void main(void)                       \n\
{                                     \n\
  gl_FragColor = vec4(1,0,0,1);       \n\
}                                     \n";

#define FLOAT_TO_FIXED(x) (long)((x) * (65536.0f))

static GLfixed vertices[] = {
    FLOAT_TO_FIXED(0.5),  FLOAT_TO_FIXED(-0.5), FLOAT_TO_FIXED(-0.5),
    FLOAT_TO_FIXED(-0.5), FLOAT_TO_FIXED(-0.5), FLOAT_TO_FIXED(0.5),
    FLOAT_TO_FIXED(0.5),  FLOAT_TO_FIXED(0.5),
};

static GLuint progid;

static void spinner_init() { progid = create_program(vertShader, fragShader); }

static _u64 first_timestamp = 0;
static _u64 prev_timestamp = 0;
static _u64 longest = 0;
static _u64 shortest = 0;

void spinner_render(int f) {
  static GLfloat projection[4][4];
  static GLfloat modelview[4][4];
  static GLfloat mvp[4][4];

  GLenum err;

  static bool inited = false;
  if (!inited) {
    spinner_init();
    inited = true;
  }

  _u64 timestamp = timestamp_usec();
  _u64 current;

  f = f % 360;

  Identity(projection);
  // Perspective(projection, 90, 1, 0, 1);
  Orthographic(projection, -0.5f, 0.5f, -9.0f / 32.0f, 9.0f / 32.0f, 0.0f,
               1.0f);

  Identity(modelview);
  Scale(modelview, 0.2, 0.2, 1.0);
  // Translate(modelview, 0.1, 0.05, 0);
  Rotate(modelview, 0, 0, 1, -f + 45);
  MultiplyMatrix(mvp, modelview, projection);

  glUseProgram(progid);
  glUniformMatrix4fv(glGetUniformLocation(progid, "mvp"), 1, GL_FALSE,
                     (const GLfloat *)mvp);
  GLint attriblocation = glGetAttribLocation(progid, "position");
  glEnableVertexAttribArray(attriblocation);
  glVertexAttribPointer(attriblocation, 2, GL_FIXED, 0, 0, vertices);

  glDrawArrays(GL_LINE_LOOP, 0, 4);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "GL ERROR = 0x%x\n", err);
  }

  if (prev_timestamp) {
    current = timestamp - prev_timestamp;

    if ((shortest == 0) || (current < shortest)) shortest = current;
    if ((longest == 0) || (current > longest)) longest = current;

    text_render(0.50 * (9 / 16.0) * cos(-(f - 90) * 3.14159265 / 180),
                0.50 * sin(-(f - 90) * 3.14159265 / 180),
                "%03d,%2.1f-%2.1f,%2.1f[%2.1f]", f, 1000000.0f / shortest,
                1000000.0f / longest, 1000000.0f / current,
                f * 1000000.0f / (timestamp - first_timestamp));

    if (f == 0) {
      longest = 0;
      shortest = 0;
      first_timestamp = timestamp;
    }
  }
  prev_timestamp = timestamp;
}
#endif
