# Automake configuration file

lib_LTLIBRARIES = libtditracer.la

libtditracer_la_CPPFLAGS = -fpermissive -O2 -g \
    $(libpng_CFLAGS)

libtditracer_la_LIBADD = \
   ../../tdi/src/libtdi.la -ldl

libtditracer_la_SOURCES = \
   framecapture.c \
   framelinkedlist.c \
   framelinkedlist.h \
   shadercapture.c \
   shaderlinkedlist.c \
   shaderlinkedlist.h \
   texturecapture.c \
   texturelinkedlist.c \
   texturelinkedlist.h \
   gldefs.h \
   tracermain.cpp \
   tracerutils.cpp \
   tracergles2.cpp \
   tracerlibc.cpp \
   tracersgx.cpp \
   tracerpthread.cpp \
   tracersem.cpp \
   tracermq.cpp \
   tracerextra.cpp \
   tracerwayland.c \
   gles2spinner.cpp \
   gles2text.cpp \
   gles2bars.cpp \
   gles2capture.cpp \
   gles2extra.cpp \
   gles2util.cpp

if NOEGL
libtditracer_la_CPPFLAGS += -DNOEGL
endif

if NOGLES2
libtditracer_la_CPPFLAGS += -DNOGLES2
endif

if NOLIBC
libtditracer_la_CPPFLAGS += -DNOLIBC
endif

if NOLIBPTHREAD
libtditracer_la_CPPFLAGS += -DNOLIBPTHREAD
endif

if NOSGX
libtditracer_la_CPPFLAGS += -DNOSGX
endif
