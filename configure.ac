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

case $target_os in
    mingw32*)
        BUILD_POSIX=true
        AC_DEFINE([BUILD_POSIX], [true], [Use POSIX build on linux])
        ;;
    linux*)
        BUILD_POSIX=true
        AC_DEFINE([BUILD_POSIX], [true], [Use POSIX build on linux])
        ;;
    vxworks*)
        BUILD_VXWORKS=true
        AC_DEFINE([BUILD_VXWORKS], [true], [Use VxWorks build])
        ;;
    pikeos*)
        BUILD_PIKEOS=true
        AC_DEFINE([BUILD_PIKEOS], [true], [Use PikeOS build])
        ;;
esac

if test x$BUILD_POSIX == xtrue; then
    AC_DEFINE([HAVE_ENOTRECOVERABLE], [], [Check if errno ENOTRECOVERABLE is present.])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
        #include <errno.h>
    ],[
        int ret = ENOTRECOVERABLE;
    ])], [AC_DEFINE([HAVE_ENOTRECOVERABLE], [1])],
         [AC_DEFINE([HAVE_ENOTRECOVERABLE], [0])])

    AC_DEFINE([HAVE_SIGSTOP], [], [Check if signal SIGSTOP is present.])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
        #include <signal.h>
    ],[
        int ret = SIGSTOP;
    ])], [AC_DEFINE([HAVE_SIGSTOP], [1])],
         [AC_DEFINE([HAVE_SIGSTOP], [0])])

    AC_DEFINE([HAVE_SIGCONT], [], [Check if signal SIGCONT is present.])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
        #include <signal.h>
    ],[
        int ret = SIGCONT;
    ])], [AC_DEFINE([HAVE_SIGCONT], [1])],
         [AC_DEFINE([HAVE_SIGCONT], [0])])
            
    AC_DEFINE([HAVE_PTHREAD_MUTEXATTR_SETROBUST], [], [Check if posix function pthread_mutexattr_setrobust present.])
    AC_CHECK_LIB(pthread, pthread_mutexattr_setrobust,
                 [AC_DEFINE([HAVE_PTHREAD_MUTEXATTR_SETROBUST], [1])],
                 [AC_DEFINE([HAVE_PTHREAD_MUTEXATTR_SETROBUST], [0])])
    
    AC_DEFINE([HAVE_PTHREAD_SETAFFINITY_NP], [], [Check if posix function pthread_setaffinity_np present.])
    AC_CHECK_LIB(pthread, pthread_setaffinity_np,
                 [AC_DEFINE([HAVE_PTHREAD_SETAFFINITY_NP], [1])],
                 [AC_DEFINE([HAVE_PTHREAD_SETAFFINITY_NP], [0])])
fi

AM_CONDITIONAL([BUILD_POSIX], [ test x$BUILD_POSIX = xtrue]) 
AM_CONDITIONAL([BUILD_VXWORKS], [ test x$BUILD_VXWORKS = xtrue]) 
AM_CONDITIONAL([BUILD_PIKEOS], [ test x$BUILD_PIKEOS = xtrue]) 

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h])
dnl check for sys/prctl for setting thread name on Linux
AC_CHECK_HEADERS([sys/prctl.h], [], [], [AC_INCLUDES_DEFAULT])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
