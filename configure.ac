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
        BUILD_MINGW32=true
        AC_DEFINE([BUILD_POSIX], [true], [Use POSIX build on windows mingw32])
        AC_DEFINE([BUILD_MINGW32], [true], [Use MINGW32 build on windows mingw32])
        AX_APPEND_FLAG([-DPTW32_STATIC_LIB], [CFLAGS])
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
        CFLAGS="$CFLAGS -Wno-suggest-attribute=format -Wno-error=suggest-attribute=format"
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
            
    PTHREAD_LIBS=""
    RT_LIBS=""

    AC_DEFINE([HAVE_PTHREAD_MUTEXATTR_SETROBUST], [], [Check if posix function pthread_mutexattr_setrobust present.])
    AC_CHECK_LIB(pthread, pthread_mutexattr_setrobust,
                 [AC_DEFINE([HAVE_PTHREAD_MUTEXATTR_SETROBUST], [1])
                 PTHREAD_LIBS="-lpthread"],
                 [AC_DEFINE([HAVE_PTHREAD_MUTEXATTR_SETROBUST], [0])])
    
    AC_DEFINE([HAVE_PTHREAD_SETAFFINITY_NP], [], [Check if posix function pthread_setaffinity_np present.])
    AC_CHECK_LIB(pthread, pthread_setaffinity_np,
                 [AC_DEFINE([HAVE_PTHREAD_SETAFFINITY_NP], [1])
                 PTHREAD_LIBS="-lpthread"],
                 [AC_DEFINE([HAVE_PTHREAD_SETAFFINITY_NP], [0])])

    AC_CHECK_LIB(pthread, pthread_create, PTHREAD_LIBS="-lpthread")
    AC_CHECK_LIB(rt, clock_gettime, RT_LIBS="-lrt")
    AC_SUBST(PTHREAD_LIBS)
    AC_SUBST(RT_LIBS)
fi
    
AC_CHECK_LIB(m, sqrt, MATH_LIBS="-lm")
AC_SUBST(MATH_LIBS)

AM_CONDITIONAL([BUILD_POSIX], [ test x$BUILD_POSIX = xtrue]) 
AM_CONDITIONAL([BUILD_MINGW32], [ test x$BUILD_MINGW32 = xtrue]) 
AM_CONDITIONAL([BUILD_VXWORKS], [ test x$BUILD_VXWORKS = xtrue]) 
AM_CONDITIONAL([BUILD_PIKEOS], [ test x$BUILD_PIKEOS = xtrue]) 
AM_CONDITIONAL([BUILD_WIN32], [ text x$BUILD_WIN32 = xtrue])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([math.h])
AC_CHECK_HEADERS([sys/mman.h], HAVE_SYS_MMAN_H=true, HAVE_SYS_MMAN_H=false)
AC_CHECK_HEADERS([mqueue.h], HAVE_MQUEUE_H=true, HAVE_MQUEUE_H=false)
dnl check for sys/prctl for setting thread name on Linux
AC_CHECK_HEADERS([sys/prctl.h], [], [], [AC_INCLUDES_DEFAULT])

# Checks for header files.
AC_CHECK_HEADERS([p4ext_threads.h])

# Check support for -mrdseed
AS_IF([test "$ac_cv_header_p4ext_threads_h" = "yes"], [
  AC_DEFINE([HAVE_P4_MUTEX_INIT_EXT], [], [Check if pikeos support p4_mutext_init_ext.])

  CFLAGS_save=$CFLAGS
  CFLAGS="$CFLAGS -Werror"
  XXX_PROGRAM="
#include <p4ext_threads.h>
int main(void) {
    P4_mutex_t pikeos_mtx;
    P4_uint32_t flags = 0u;
    P4_prio_t ceil_prio = 0u;
    p4_mutex_init_ext(&pikeos_mtx, flags, ceil_prio);
    return 0;
}
"
  AC_MSG_CHECKING([whether pikeos support for p4_mutex_init_ext])
  AC_COMPILE_IFELSE(
    [AC_LANG_SOURCE([$XXX_PROGRAM])],
    [
     AC_MSG_RESULT([yes])
     AC_DEFINE([HAVE_P4_MUTEX_INIT_EXT], [1])
    ],
    [
     AC_MSG_RESULT([no])
     AC_DEFINE([HAVE_P4_MUTEX_INIT_EXT], [0])
    ]
  )

  CFLAGS=$CFLAGS_save
])

AM_CONDITIONAL([HAVE_SYS_MMAN_H], [ test x$HAVE_SYS_MMAN_H = xtrue])
AM_CONDITIONAL([HAVE_MQUEUE_H], [ test x$HAVE_MQUEUE_H = xtrue])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile src/tools/logger/Makefile libosal.pc])
AC_OUTPUT
