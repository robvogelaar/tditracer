
typedef int          EGLBoolean;
typedef void*        EGLDisplay;
typedef void*        EGLSurface;
typedef void*        EGLContext;


typedef char         GLchar;
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int          GLint;
typedef int          GLsizei;
typedef void         GLvoid;
typedef unsigned int GLbitfield;
typedef float        GLclampf;


#define GL_POINTS                 0x0000
#define GL_LINES                  0x0001
#define GL_LINE_LOOP              0x0002
#define GL_LINE_STRIP             0x0003
#define GL_TRIANGLES              0x0004
#define GL_TRIANGLE_STRIP         0x0005
#define GL_TRIANGLE_FAN           0x0006

#define GL_BYTE                   0x1400
#define GL_UNSIGNED_BYTE          0x1401
#define GL_SHORT                  0x1402
#define GL_UNSIGNED_SHORT         0x1403
#define GL_INT                    0x1404
#define GL_UNSIGNED_INT           0x1405
#define GL_FLOAT                  0x1406
#define GL_FIXED                  0x140C

#define GL_DEPTH_COMPONENT        0x1902
#define GL_ALPHA                  0x1906
#define GL_RGB                    0x1907
#define GL_RGBA                   0x1908
#define GL_LUMINANCE              0x1909
#define GL_LUMINANCE_ALPHA        0x190A
#define GL_BGRA_EXT               0x80E1


#define GL_TEXTURE_MAG_FILTER     0x2800
#define GL_TEXTURE_MIN_FILTER     0x2801
#define GL_TEXTURE_WRAP_S         0x2802
#define GL_TEXTURE_WRAP_T         0x2803

#define GL_REPEAT                 0x2901
#define GL_CLAMP_TO_EDGE          0x812F
#define GL_MIRRORED_REPEAT			 0x8370


#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_SHORT_5_6_5   0x8363


/* BlendingFactorDest */
#define GL_ZERO                   0
#define GL_ONE                    1
#define GL_SRC_COLOR              0x0300
#define GL_ONE_MINUS_SRC_COLOR    0x0301
#define GL_SRC_ALPHA              0x0302
#define GL_ONE_MINUS_SRC_ALPHA    0x0303
#define GL_DST_ALPHA              0x0304
#define GL_ONE_MINUS_DST_ALPHA    0x0305

/* BlendingFactorSrc */
/*      GL_ZERO */
/*      GL_ONE */
#define GL_DST_COLOR              0x0306
#define GL_ONE_MINUS_DST_COLOR    0x0307
#define GL_SRC_ALPHA_SATURATE     0x0308
/*      GL_SRC_ALPHA */
/*      GL_ONE_MINUS_SRC_ALPHA */
/*      GL_DST_ALPHA */
/*      GL_ONE_MINUS_DST_ALPHA */


#define GL_TEXTURE_2D                     0x0DE1
#define GL_CULL_FACE                      0x0B44
#define GL_BLEND                          0x0BE2
#define GL_DITHER                         0x0BD0
#define GL_STENCIL_TEST                   0x0B90
#define GL_DEPTH_TEST                     0x0B71
#define GL_SCISSOR_TEST                   0x0C11
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_COVERAGE                0x80A0



#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000




#define CLEARSTRING(mask)       (mask==GL_DEPTH_BUFFER_BIT        ? "GL_DEPTH_BUFFER_BIT":        \
                                 mask==GL_STENCIL_BUFFER_BIT      ? "GL_STENCIL_BUFFER_BIT":      \
                                 mask==GL_COLOR_BUFFER_BIT        ? "GL_COLOR_BUFFER_BIT": "???")

#define CAPSTRING(factor)       (cap==GL_TEXTURE_2D               ? "GL_TEXTURE_2D":               \
                                 cap==GL_CULL_FACE                ? "GL_CULL_FACE":                \
                                 cap==GL_BLEND                    ? "GL_BLEND":                    \
                                 cap==GL_DITHER                   ? "GL_DITHER":                   \
                                 cap==GL_STENCIL_TEST             ? "GL_STENCIL_TEST":             \
                                 cap==GL_DEPTH_TEST               ? "GL_DEPTH_TEST":               \
                                 cap==GL_SCISSOR_TEST             ? "GL_SCISSOR_TEST":             \
                                 cap==GL_POLYGON_OFFSET_FILL      ? "GL_POLYGON_OFFSET_FILL":      \
                                 cap==GL_SAMPLE_ALPHA_TO_COVERAGE ? "GL_SAMPLE_ALPHA_TO_COVERAGE": \
                                 cap==GL_SAMPLE_COVERAGE          ? "GL_SAMPLE_COVERAGE":"???")


#define BLENDSTRING(factor)     (factor==GL_ZERO                ? "GL_ZERO":                    \
                                 factor==GL_ONE                 ? "GL_ONE":                     \
                                 factor==GL_SRC_COLOR           ? "GL_SRC_COLOR":               \
                                 factor==GL_ONE_MINUS_SRC_COLOR ? "GL_ONE_MINUS_SRC_COLOR":     \
                                 factor==GL_SRC_ALPHA           ? "GL_SRC_ALPHA":               \
                                 factor==GL_ONE_MINUS_SRC_ALPHA ? "GL_ONE_MINUS_SRC_ALPHA":     \
                                 factor==GL_DST_ALPHA           ? "GL_DST_ALPHA":               \
                                 factor==GL_ONE_MINUS_DST_ALPHA ? "GL_ONE_MINUS_DST_ALPHA":     \
                                 factor==GL_DST_COLOR           ? "GL_DST_COLOR":               \
                                 factor==GL_ONE_MINUS_DST_COLOR ? "GL_ONE_MINUS_DST_COLOR":     \
                                 factor==GL_SRC_ALPHA_SATURATE  ? "GL_SRC_ALPHA_SATURATE":"???")


#define MODESTRING(mode)        (mode==GL_POINTS?"GL_POINTS":                                      \
                                 mode==GL_LINES?"GL_LINES":                                        \
                                 mode==GL_LINE_LOOP?"GL_LINE_LOOP":                                \
                                 mode==GL_LINE_STRIP?"GL_LINE_STRIP":                              \
                                 mode==GL_TRIANGLES?"GL_TRIANGLES":                                \
                                 mode==GL_TRIANGLE_STRIP?"GL_TRIANGLE_STRIP":                      \
                                 mode==GL_TRIANGLE_FAN?"GL_TRIANGLE_FAN":"???")

#define TYPESTRING(type)        (type==GL_BYTE?"GL_BYTE":                                          \
                                 type==GL_UNSIGNED_BYTE?"GL_UNSIGNED_BYTE":                        \
                                 type==GL_INT?"GL_INT":                                            \
                                 type==GL_UNSIGNED_INT?"GL_UNSIGNED_INT":                          \
                                 type==GL_FLOAT?"GL_FLOAT":                                        \
                                 type==GL_FIXED?"GL_FIXED":                                        \
                                 type==GL_SHORT?"GL_SHORT":                                        \
                                 type==GL_UNSIGNED_SHORT?"GL_UNSIGNED_SHORT":                      \
                                 type==GL_UNSIGNED_SHORT_5_6_5?"GL_UNSIGNED_SHORT_5_6_5":          \
                                 type==GL_UNSIGNED_SHORT_4_4_4_4?"GL_UNSIGNED_SHORT_4_4_4_4":      \
                                 type==GL_UNSIGNED_SHORT_5_5_5_1?"GL_UNSIGNED_SHORT_5_5_5_1":"???")

#define FORMATSTRING(format)    (format==GL_DEPTH_COMPONENT?"GL_DEPTH_COMPONENT":                  \
                                 format==GL_ALPHA?"GL_ALPHA":                                      \
                                 format==GL_RGB?"GL_RGB":                                          \
                                 format==GL_RGBA?"GL_RGBA":                                        \
                                 format==GL_LUMINANCE?"GL_LUMINANCE":                              \
                                 format==GL_LUMINANCE_ALPHA?"GL_LUMINANCE_ALPHA":                  \
                                 format==GL_BGRA_EXT?"GL_BGRA_EXT":"???")




