# Automake configuration file

lib_LTLIBRARIES = libtditracer.la

libtditracer_la_CPPFLAGS = -fpermissive

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
   tracermain.cpp

