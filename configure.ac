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
