# Automake configuration file

lib_LTLIBRARIES = libtditracer.la

libtditracer_la_CPPFLAGS = -fpermissive -O2 -g \
    $(libpng_CFLAGS)

libtditracer_la_LIBADD = \
   ../../tdi/src/libtdi.la -ldl

libtditracer_la_SOURCES = \
   tracermain.cpp \
   tracerextra.cpp

#   tracerwayland.c

######################################################

if NOEGL
   libtditracer_la_CPPFLAGS += -DNOEGL
endif

if !NOEGL
   libtditracer_la_SOURCES += 
   traceregl.cpp
endif

######################################################

if NOGLES2
libtditracer_la_CPPFLAGS += -DNOGLES2
endif

if !NOGLES2
   libtditracer_la_SOURCES += 
   tracergles2.cpp \
   gles2spinner.cpp \
   gles2text.cpp \
   gles2bars.cpp \
   gles2capture.cpp \
   gles2extra.cpp \
   gles2util.cpp
   framecapture.c \
   framelinkedlist.c \
   shadercapture.c \
   shaderlinkedlist.c \
   texturecapture.c \
   texturelinkedlist.c
endif

######################################################

if NOLIBC
libtditracer_la_CPPFLAGS += -DNOLIBC
endif

if !NOLIBC
   libtditracer_la_SOURCES +=
   tracerlibc.cpp
endif

######################################################

if NOLIBPTHREAD
   libtditracer_la_CPPFLAGS += -DNOLIBPTHREAD
endif

if !NOLIBPTHREAD
   libtditracer_la_SOURCES +=
   tracerpthread.cpp \
   tracersem.cpp \
   tracermq.cpp
endif

######################################################

if NOSGX
   libtditracer_la_CPPFLAGS += -DNOSGX
endif

if !NOSGX
   libtditracer_la_SOURCES +=
      tracersgx.cpp
endif
