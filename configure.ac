#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tditracer], [0.1], [revogelaar@gmail.com])
AM_INIT_AUTOMAKE([foreign subdir-objects])
LT_INIT
AC_CONFIG_SRCDIR([tdidump/src/tdidump.c])
# AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB


AC_ARG_ENABLE([egl],
[  --disable-egl disable egl interposing],
[egl=${enableval}], [egl=yes])
AM_CONDITIONAL(NOEGL, test x$egl = xno)

AC_ARG_ENABLE([gles2],
[  --disable-gles2 disable gles2 interposing],
[gles2=${enableval}], [gles2=yes])
AM_CONDITIONAL(NOGLES2, test x$gles2 = xno)

AC_ARG_ENABLE([libc],
[  --disable-libc disable libc interposing],
[libc=${enableval}], [libc=yes])
AM_CONDITIONAL(NOLIBC, test x$libc = xno)

AC_ARG_ENABLE([libpthread],
[  --disable-libpthread disable libpthread interposing],
[libpthread=${enableval}], [libpthread=yes])
AM_CONDITIONAL(NOLIBPTHREAD, test x$libpthread = xno)

AC_ARG_ENABLE([sgx],
[  --disable-sgx disable sgx interposing],
[sgx=${enableval}], [sgx=no])
AM_CONDITIONAL(NOSGX, test x$sgx = xno)

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h refsw/nexus_config.h])

# Checks for pkg-config
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([libpng], [libpng >= 1.2.0], [], [AC_MSG_ERROR([Unable to find libpng])])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday memset strcspn strdup strerror strstr strtol strtoul])

AC_CONFIG_FILES([
    Makefile
    tdi/src/Makefile
    tditest/src/Makefile
    tdidump/src/Makefile
    tdistat/src/Makefile
    tditracer/src/Makefile
])

AC_OUTPUT
