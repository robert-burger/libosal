#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libosal], [0.0.1], [Robert Burger <robert.burger@dlr.de>])

AC_CANONICAL_TARGET

# Automake initialization
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
#AM_INIT_AUTOMAKE([-Wall -Werror foreign silent-rules subdir-objects])
#m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])


AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AX_PREFIX_CONFIG_H(include/libosal/config.h) # prep mylib/_config.h from it..

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
        

case $target in
    *-*-linux-*)
        BUILD_POSIX=true
        AC_DEFINE([BUILD_POSIX], [true], [Use POSIX build on linux])
        ;;
esac

AM_CONDITIONAL([BUILD_POSIX], [ test x$BUILD_POSIX = xtrue]) 

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
