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
attribute vec4 position;              \n\
void main(void)                       \n\
{                                     \n\
  gl_Position = position;             \n\
}                                     \n";

static const char *const fragShader =
    "\n\
void main(void)                       \n\
{                                     \n\
  gl_FragColor = vec4(1,0,0,1);       \n\
}                                     \n";

static GLuint vbo;
static int progid;
static int attriblocation;

static _u64 prev_timestamp = 0;

#define FLOAT_TO_FIXED(x) (long)((x) * (65536.0f / 1))

static GLfixed vertices[360 * 2 * 2] = {};

static void bars_init(void) {
  progid = create_program(vertShader, fragShader);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  GLfixed *v = vertices;

  int i;
  for (i = 0; i < 360; i++) {
    *v++ = FLOAT_TO_FIXED(-1.0 + ((i * 3) / 540.0));
    *v++ = FLOAT_TO_FIXED(-1.0 + 0.0);

    *v++ = FLOAT_TO_FIXED(-1.0 + (((i * 3) + 3) / 540.0));
    *v++ = FLOAT_TO_FIXED(-1.0 + 0.0);
  }

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  attriblocation = glGetAttribLocation(progid, "position");
}

void bars_render(void) {
  static bool inited = false;
  if (!inited) {
    bars_init();
    inited = true;
  }

  _u64 timestamp = timestamp_usec();
  _u64 current;

  if (!prev_timestamp) {
    prev_timestamp = timestamp;
    return;
  }
  current = timestamp - prev_timestamp;
  prev_timestamp = timestamp;

  glUseProgram(progid);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  GLfixed *v = vertices;
  int i;
  for (i = 0; i < 359; i++) {
    v[1] = v[5];
    v[3] = v[7];
    v += 4;
  }

  v[1] = v[3] = FLOAT_TO_FIXED(-1.0 + 1000000.0f / (current * 60.0));

  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glEnableVertexAttribArray(attriblocation);
  glVertexAttribPointer(attriblocation, 2, GL_FIXED, 0, 0, 0);

  glDrawArrays(GL_LINE_STRIP, 0, 360 * 2);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
