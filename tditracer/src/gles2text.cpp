#ifndef NOGLES2

#include <algorithm>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <gles2util.h>

#include "FreeSans.ttf-36-509x134.c"

static const char *const TextvertShader =
    "\n\
attribute vec4 coord;                                               \n\
varying vec2 texpos;                                                \n\
void main(void)                                                     \n\
{                                                                   \n\
  gl_Position = vec4(coord.xy, 0, 1);                               \n\
  texpos = coord.zw;                                                \n\
}                                                                   \n";

static const char *const TextfragShader =
    "\n\
uniform sampler2D tex;                                              \n\
varying highp vec2 texpos;                                          \n\
uniform highp vec4 color;                                           \n\
void main(void)                                                     \n\
{                                                                   \n\
  gl_FragColor = vec4(1, 1, 1, texture2D(tex, texpos).a) * color;   \n\
}                                                                   \n";

static GLuint texture;
static GLuint program;
static GLint attribute_coord;
static GLint uniform_color;
static GLuint vbo;

struct point {
  GLfloat x;
  GLfloat y;
  GLfloat s;
  GLfloat t;
};

void glTexImage2D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLsizei height, GLint border, GLenum format,
                  GLenum type, const GLvoid *data);

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                     GLsizei width, GLsizei height, GLenum format, GLenum type,
                     const GLvoid *data);

static int text_init() {
  /* Create a texture that will be used to hold all ASCII glyphs */
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  /* We require 1 byte alignment when uploading texture data */
  GLint unpack_alignment;
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack_alignment);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, font_atlas_width, font_atlas_height,
               0, GL_ALPHA, GL_UNSIGNED_BYTE, font_atlas_alpha_pixels);
  glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_alignment);

  /* Clamping to edges is important to prevent artifacts when scaling */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  /* Linear filtering usually looks best for text */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  program = create_program(TextvertShader, TextfragShader);
  if (program == 0) return 0;

  attribute_coord = glGetAttribLocation(program, "coord");
  uniform_color = glGetUniformLocation(program, "color");
  if (attribute_coord == -1 || uniform_color == -1) return 0;

  glGenBuffers(1, &vbo);
  return 1;
}

/**
 * Render text using the currently loaded font and currently set font size.
 * Rendering starts at coordinates (x, y), z is always 0.
 * The pixel coordinates that the FreeType2 library uses are scaled by (sx, sy).
 */
void text_render(float x, float y, const char *s, ...) {
  char text[256];
  const uint8_t *p;

  static bool inited = false;
  if (!inited) {
    text_init();
    inited = true;
  }

  va_list args;
  va_start(args, s);
  vsprintf(text, s, args);
  va_end(args);

  glUseProgram(program);

  /* Enable blending, necessary for our alpha texture */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLfloat white[4] = {1, 1, 1, 1};
  GLfloat yellow[4] = {1, 1, 0, 1};
  GLfloat red[4] = {1, 0, 0, 1};
  glUniform4fv(uniform_color, 1, red);

  float sx = 2.0 / 1280;
  float sy = 2.0 / 720;

  /* Use the texture containing the atlas */
  glBindTexture(GL_TEXTURE_2D, texture);

  /* Set up the VBO for our vertex data */
  glEnableVertexAttribArray(attribute_coord);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

  point coords[6 * strlen(text)];
  int c = 0;

  /* Loop through all characters */
  for (p = (const uint8_t *)text; *p; p++) {
    /* Calculate the vertex and texture coordinates */
    float x2 = x + font_atlas_meta[*p].bl * sx;
    float y2 = -y - font_atlas_meta[*p].bt * sy;
    float w = font_atlas_meta[*p].bw * sx;
    float h = font_atlas_meta[*p].bh * sy;

    /* Advance the cursor to the start of the next character */
    x += font_atlas_meta[*p].ax * sx;
    y += font_atlas_meta[*p].ay * sy;

    /* Skip glyphs that have no pixels */
    if (!w || !h) continue;

    coords[c++] =
        (point){x2, -y2, font_atlas_meta[*p].tx, font_atlas_meta[*p].ty};
    coords[c++] = (point){
        x2 + w, -y2,
        font_atlas_meta[*p].tx + font_atlas_meta[*p].bw / font_atlas_width,
        font_atlas_meta[*p].ty};
    coords[c++] = (point){
        x2, -y2 - h, font_atlas_meta[*p].tx,
        font_atlas_meta[*p].ty + font_atlas_meta[*p].bh / font_atlas_height};
    coords[c++] = (point){
        x2 + w, -y2,
        font_atlas_meta[*p].tx + font_atlas_meta[*p].bw / font_atlas_width,
        font_atlas_meta[*p].ty};
    coords[c++] = (point){
        x2, -y2 - h, font_atlas_meta[*p].tx,
        font_atlas_meta[*p].ty + font_atlas_meta[*p].bh / font_atlas_height};
    coords[c++] = (point){
        x2 + w, -y2 - h,
        font_atlas_meta[*p].tx + font_atlas_meta[*p].bw / font_atlas_width,
        font_atlas_meta[*p].ty + font_atlas_meta[*p].bh / font_atlas_height};
  }

  /* Draw all the character on the screen in one go */
  glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, c);

  glDisableVertexAttribArray(attribute_coord);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#endif