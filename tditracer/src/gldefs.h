
typedef unsigned int EGLBoolean;
typedef int32_t EGLint;
typedef void* EGLDisplay;
typedef void* EGLSurface;
typedef void* EGLContext;
typedef void* EGLConfig;

typedef void* NativeWindowType;
typedef void* NativePixmapType;


typedef unsigned char GLboolean;
typedef char GLchar;
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef int GLintptr;
typedef int GLsizeiptr;
typedef void GLvoid;
typedef unsigned int GLbitfield;
typedef float GLclampf;
typedef float GLfloat;
typedef int GLfixed;
typedef unsigned char GLubyte;

#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006

#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_FIXED 0x140C

#define GL_DEPTH_COMPONENT 0x1902
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_BGRA_EXT 0x80E1

#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803

#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_MIRRORED_REPEAT 0x8370

#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_SHORT_5_6_5 0x8363

/* BlendingFactorDest */
#define GL_ZERO 0
#define GL_ONE 1
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305

/* BlendingFactorSrc */
/*      GL_ZERO */
/*      GL_ONE */
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308
/*      GL_SRC_ALPHA */
/*      GL_ONE_MINUS_SRC_ALPHA */
/*      GL_DST_ALPHA */
/*      GL_ONE_MINUS_DST_ALPHA */

#define GL_TEXTURE_2D 0x0DE1
#define GL_CULL_FACE 0x0B44
#define GL_BLEND 0x0BE2
#define GL_DITHER 0x0BD0
#define GL_STENCIL_TEST 0x0B90
#define GL_DEPTH_TEST 0x0B71
#define GL_SCISSOR_TEST 0x0C11
#define GL_POLYGON_OFFSET_FILL 0x8037
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#define GL_SAMPLE_COVERAGE 0x80A0

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000

/* Framebuffer Object. */
#define GL_RGBA4 0x8056
#define GL_RGB5_A1 0x8057
#define GL_RGB565 0x8D62
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_STENCIL_INDEX 0x1901
#define GL_STENCIL_INDEX8 0x8D48

#define GL_DEPTH_STENCIL_OES 0x84F9
#define GL_UNSIGNED_INT_24_8_OES 0x84FA
#define GL_DEPTH24_STENCIL8_OES 0x88F0

#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20

#define GL_DELETE_STATUS 0x8B80
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_CURRENT_PROGRAM 0x8B8D

#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893

#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41

#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_RENDERBUFFER_BINDING 0x8CA7

#define CLEARSTRING(mask)                                                      \
  (mask == GL_DEPTH_BUFFER_BIT                                                 \
       ? "GL_DEPTH_BUFFER_BIT"                                                 \
       : mask == GL_STENCIL_BUFFER_BIT                                         \
             ? "GL_STENCIL_BUFFER_BIT"                                         \
             : mask == GL_COLOR_BUFFER_BIT                                     \
                   ? "GL_COLOR_BUFFER_BIT"                                     \
                   : mask == (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)       \
                         ? "GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT"           \
                         : mask == (GL_COLOR_BUFFER_BIT |                      \
                                    GL_DEPTH_BUFFER_BIT |                      \
                                    GL_STENCIL_BUFFER_BIT)                     \
                               ? "GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_" \
                                 "STENCIL_BUFFER_BIT"                          \
                               : "???")

#define CAPSTRING(factor)                                                             \
  (cap == GL_TEXTURE_2D                                                               \
       ? "GL_TEXTURE_2D"                                                              \
       : cap == GL_CULL_FACE                                                          \
             ? "GL_CULL_FACE"                                                         \
             : cap == GL_BLEND                                                        \
                   ? "GL_BLEND"                                                       \
                   : cap == GL_DITHER                                                 \
                         ? "GL_DITHER"                                                \
                         : cap == GL_STENCIL_TEST                                     \
                               ? "GL_STENCIL_TEST"                                    \
                               : cap == GL_DEPTH_TEST                                 \
                                     ? "GL_DEPTH_TEST"                                \
                                     : cap == GL_SCISSOR_TEST                         \
                                           ? "GL_SCISSOR_TEST"                        \
                                           : cap == GL_POLYGON_OFFSET_FILL            \
                                                 ? "GL_POLYGON_OFFSET_FILL"           \
                                                 : cap == GL_SAMPLE_ALPHA_TO_COVERAGE \
                                                       ? "GL_SAMPLE_ALPHA_"           \
                                                         "TO_COVERAGE"                \
                                                       : cap == GL_SAMPLE_COVERAGE    \
                                                             ? "GL_SAMPLE_"           \
                                                               "COVERAGE"             \
                                                             : "???")

#define BLENDSTRING(factor)                                                                  \
  (factor == GL_ZERO                                                                         \
       ? "GL_ZERO"                                                                           \
       : factor == GL_ONE                                                                    \
             ? "GL_ONE"                                                                      \
             : factor == GL_SRC_COLOR                                                        \
                   ? "GL_SRC_COLOR"                                                          \
                   : factor == GL_ONE_MINUS_SRC_COLOR                                        \
                         ? "GL_ONE_MINUS_SRC_COLOR"                                          \
                         : factor == GL_SRC_ALPHA                                            \
                               ? "GL_SRC_ALPHA"                                              \
                               : factor == GL_ONE_MINUS_SRC_ALPHA                            \
                                     ? "GL_ONE_MINUS_SRC_ALPHA"                              \
                                     : factor == GL_DST_ALPHA                                \
                                           ? "GL_DST_ALPHA"                                  \
                                           : factor == GL_ONE_MINUS_DST_ALPHA                \
                                                 ? "GL_ONE_MINUS_DST_ALPHA"                  \
                                                 : factor == GL_DST_COLOR                    \
                                                       ? "GL_DST_COLOR"                      \
                                                       : factor ==                           \
                                                                 GL_ONE_MINUS_DST_COLOR      \
                                                             ? "GL_ONE_"                     \
                                                               "MINUS_DST_"                  \
                                                               "COLOR"                       \
                                                             : factor ==                     \
                                                                       GL_SRC_ALPHA_SATURATE \
                                                                   ? "GL_"                   \
                                                                     "SRC_"                  \
                                                                     "ALPHA"                 \
                                                                     "_SATU"                 \
                                                                     "RATE"                  \
                                                                   : "???")

#define MODESTRING(mode)                                                    \
  (mode == GL_POINTS                                                        \
       ? "GL_POINTS"                                                        \
       : mode == GL_LINES ? "GL_LINES"                                      \
                          : mode == GL_LINE_LOOP                            \
                                ? "GL_LINE_LOOP"                            \
                                : mode == GL_LINE_STRIP                     \
                                      ? "GL_LINE_STRIP"                     \
                                      : mode == GL_TRIANGLES                \
                                            ? "GL_TRIANGLES"                \
                                            : mode == GL_TRIANGLE_STRIP     \
                                                  ? "GL_TRIANGLE_STRIP"     \
                                                  : mode == GL_TRIANGLE_FAN \
                                                        ? "GL_TRIANGLE_FAN" \
                                                        : "???")

#define TYPESTRING(type)                                                                         \
  (type == GL_BYTE                                                                               \
       ? "GL_BYTE"                                                                               \
       : type == GL_UNSIGNED_BYTE                                                                \
             ? "GL_UNSIGNED_BYTE"                                                                \
             : type == GL_INT                                                                    \
                   ? "GL_INT"                                                                    \
                   : type == GL_UNSIGNED_INT                                                     \
                         ? "GL_UNSIGNED_INT"                                                     \
                         : type == GL_FLOAT                                                      \
                               ? "GL_FLOAT"                                                      \
                               : type == GL_FIXED                                                \
                                     ? "GL_FIXED"                                                \
                                     : type == GL_SHORT                                          \
                                           ? "GL_SHORT"                                          \
                                           : type == GL_UNSIGNED_SHORT                           \
                                                 ? "GL_UNSIGNED_SHORT"                           \
                                                 : type == GL_UNSIGNED_SHORT_5_6_5               \
                                                       ? "GL_UNSIGNED_"                          \
                                                         "SHORT_5_6_5"                           \
                                                       : type == GL_UNSIGNED_SHORT_4_4_4_4       \
                                                             ? "GL_"                             \
                                                               "UNSIGNED_"                       \
                                                               "SHORT_4_4_"                      \
                                                               "4_4"                             \
                                                             : type == GL_UNSIGNED_SHORT_5_5_5_1 \
                                                                   ? "GL_"                       \
                                                                     "UNSIG"                     \
                                                                     "NED_"                      \
                                                                     "SHORT"                     \
                                                                     "_5_5_"                     \
                                                                     "5_1"                       \
                                                                   : "???")

#define FORMATSTRING(format)                                                 \
  (format == GL_DEPTH_COMPONENT                                              \
       ? "GL_DEPTH_COMPONENT"                                                \
       : format == GL_ALPHA                                                  \
             ? "GL_ALPHA"                                                    \
             : format == GL_RGB                                              \
                   ? "GL_RGB"                                                \
                   : format == GL_RGBA                                       \
                         ? "GL_RGBA"                                         \
                         : format == GL_LUMINANCE                            \
                               ? "GL_LUMINANCE"                              \
                               : format == GL_LUMINANCE_ALPHA                \
                                     ? "GL_LUMINANCE_ALPHA"                  \
                                     : format == GL_BGRA_EXT ? "GL_BGRA_EXT" \
                                                             : "???")

#define FORMATSIZE(format)                                                     \
  (format == GL_DEPTH_COMPONENT                                                \
       ? 1                                                                     \
       : format == GL_ALPHA                                                    \
             ? 1                                                               \
             : format == GL_RGB ? 3 : format == GL_RGBA                        \
                                          ? 4                                  \
                                          : format == GL_LUMINANCE             \
                                                ? 4                            \
                                                : format == GL_LUMINANCE_ALPHA \
                                                      ? 4                      \
                                                      : format == GL_BGRA_EXT  \
                                                            ? 4                \
                                                            : 1)

#define IFORMATSTRING(format)                                                      \
  (format == GL_RGBA4                                                              \
       ? "GL_RGBA4"                                                                \
       : format == GL_RGB5_A1                                                      \
             ? "GL_RGB5_A1"                                                        \
             : format == GL_RGB565                                                 \
                   ? "GL_RGB565"                                                   \
                   : format == GL_DEPTH_COMPONENT16                                \
                         ? "GL_DEPTH_COMPONENT16"                                  \
                         : format == GL_DEPTH_STENCIL_OES                          \
                               ? "GL_DEPTH_STENCIL_OES"                            \
                               : format == GL_STENCIL_INDEX                        \
                                     ? "GL_STENCIL_INDEX"                          \
                                     : format == GL_STENCIL_INDEX8                 \
                                           ? "GL_STENCIL_INDEX8"                   \
                                           : format ==                             \
                                                     GL_UNSIGNED_INT_24_8_OES      \
                                                 ? "GL_UNSIGNED_INT_24_8_"         \
                                                   "OES"                           \
                                                 : format ==                       \
                                                           GL_DEPTH24_STENCIL8_OES \
                                                       ? "GL_DEPTH24_"             \
                                                         "STENCIL8_OES"            \
                                                       : "???")

#define ATTACHMENTSTRING(attachment)                                         \
  (attachment == GL_COLOR_ATTACHMENT0                                        \
       ? "GL_COLOR_ATTACHMENT0"                                              \
       : attachment == GL_DEPTH_ATTACHMENT                                   \
             ? "GL_DEPTH_ATTACHMENT"                                         \
             : attachment == GL_STENCIL_ATTACHMENT ? "GL_STENCIL_ATTACHMENT" \
                                                   : "???")

#define TARGETSTRING(target)                                           \
  (target == GL_ARRAY_BUFFER                                           \
       ? "GL_ARRAY_BUFFER"                                             \
       : target == GL_ELEMENT_ARRAY_BUFFER ? "GL_ELEMENT_ARRAY_BUFFER" \
                                           : "???")



/*
** Config attributes
*/

#define EGL_BUFFER_SIZE          0x3020
#define EGL_ALPHA_SIZE           0x3021
#define EGL_BLUE_SIZE          0x3022
#define EGL_GREEN_SIZE           0x3023
#define EGL_RED_SIZE           0x3024
#define EGL_DEPTH_SIZE           0x3025
#define EGL_STENCIL_SIZE         0x3026
#define EGL_CONFIG_CAVEAT        0x3027
#define EGL_CONFIG_ID          0x3028
#define EGL_LEVEL          0x3029
#define EGL_MAX_PBUFFER_HEIGHT         0x302A
#define EGL_MAX_PBUFFER_PIXELS         0x302B
#define EGL_MAX_PBUFFER_WIDTH        0x302C
#define EGL_NATIVE_RENDERABLE        0x302D
#define EGL_NATIVE_VISUAL_ID         0x302E
#define EGL_NATIVE_VISUAL_TYPE         0x302F
/*#define EGL_PRESERVED_RESOURCES  0x3030*/
#define EGL_SAMPLES          0x3031
#define EGL_SAMPLE_BUFFERS         0x3032
#define EGL_SURFACE_TYPE         0x3033
#define EGL_TRANSPARENT_TYPE         0x3034
#define EGL_TRANSPARENT_BLUE_VALUE     0x3035
#define EGL_TRANSPARENT_GREEN_VALUE    0x3036
#define EGL_TRANSPARENT_RED_VALUE      0x3037
#define EGL_NONE           0x3038 /* Also a config value */
#define EGL_BIND_TO_TEXTURE_RGB        0x3039
#define EGL_BIND_TO_TEXTURE_RGBA       0x303A
#define EGL_MIN_SWAP_INTERVAL        0x303B
#define EGL_MAX_SWAP_INTERVAL        0x303C
#define EGL_RENDERABLE_TYPE            0x3040



#define CONFIGATTRIBUTESSTRING(attribute)  \
  (attribute == EGL_BUFFER_SIZE        \
       ? "EGL_BUFFER_SIZE"    \
       : attribute == EGL_ALPHA_SIZE ? "EGL_ALPHA_SIZE" \
       : attribute == EGL_BLUE_SIZE ? "EGL_BLUE_SIZE" \
       : attribute == EGL_GREEN_SIZE ? "EGL_GREEN_SIZE" \
       : attribute == EGL_RED_SIZE ? "EGL_RED_SIZE" \
       : attribute == EGL_DEPTH_SIZE ? "EGL_DEPTH_SIZE" \
       : attribute == EGL_STENCIL_SIZE ? "EGL_STENCIL_SIZE" \
       : attribute == EGL_CONFIG_CAVEAT ? "EGL_CONFIG_CAVEAT" \
       : attribute == EGL_CONFIG_ID ? "EGL_CONFIG_ID" \
       : attribute == EGL_LEVEL ? "EGL_LEVEL" \
       : attribute == EGL_MAX_PBUFFER_HEIGHT ? "EGL_MAX_PBUFFER_HEIGHT" \
       : attribute == EGL_MAX_PBUFFER_PIXELS ? "EGL_MAX_PBUFFER_PIXELS" \
       : attribute == EGL_MAX_PBUFFER_WIDTH ? "EGL_MAX_PBUFFER_WIDTH" \
       : attribute == EGL_NATIVE_RENDERABLE ? "EGL_NATIVE_RENDERABLE" \
       : attribute == EGL_NATIVE_VISUAL_ID ? "EGL_NATIVE_VISUAL_ID" \
       : attribute == EGL_NATIVE_VISUAL_TYPE ? "EGL_NATIVE_VISUAL_TYPE" \
       : attribute == EGL_SAMPLES ? "EGL_SAMPLES" \
       : attribute == EGL_SAMPLE_BUFFERS ? "EGL_SAMPLE_BUFFERS" \
       : attribute == EGL_SURFACE_TYPE ? "EGL_SURFACE_TYPE" \
       : attribute == EGL_TRANSPARENT_TYPE ? "EGL_TRANSPARENT_TYPE" \
       : attribute == EGL_TRANSPARENT_BLUE_VALUE ? "EGL_TRANSPARENT_BLUE_VALUE" \
       : attribute == EGL_TRANSPARENT_GREEN_VALUE ? "EGL_TRANSPARENT_GREEN_VALUE" \
       : attribute == EGL_TRANSPARENT_RED_VALUE ? "EGL_TRANSPARENT_RED_VALUE" \
       : attribute == EGL_NONE ? "EGL_NONE" \
       : attribute == EGL_BIND_TO_TEXTURE_RGB ? "EGL_BIND_TO_TEXTURE_RGB" \
       : attribute == EGL_BIND_TO_TEXTURE_RGBA ? "EGL_BIND_TO_TEXTURE_RGBA" \
       : attribute == EGL_MIN_SWAP_INTERVAL ? "EGL_MIN_SWAP_INTERVAL" \
       : attribute == EGL_MAX_SWAP_INTERVAL ? "EGL_MAX_SWAP_INTERVAL" \
       : attribute == EGL_RENDERABLE_TYPE ? "EGL_RENDERABLE_TYPE" \
                                           : "???")



#ifdef __cplusplus
extern "C" {
#endif

GLenum glGetError(void);
void glGetProgramiv(GLuint program, GLenum pname, GLint* params);
void glGetIntegerv(GLenum pname, GLint* data);
GLboolean glIsProgram(GLuint program);
void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat,
                      GLint x, GLint y, GLsizei width, GLsizei height,
                      GLint border);
void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                         GLint yoffset, GLint x, GLint y, GLsizei width,
                         GLsizei height);
extern GLvoid glBindTexture(GLenum target, GLuint texture);
extern GLenum glCheckFramebufferStatus(GLenum target);

#ifdef __cplusplus
}
#endif

#define GL_NO_ERROR 0
