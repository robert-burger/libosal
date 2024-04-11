# This setup uses the configuration explained in
# detail in
# https://stackoverflow.com/questions/35998856/how-can-i-use-google-test-with-my-project-that-builds-via-autotools

ACLOCAL_AMFLAGS = -I m4
AM_LDFLAGS = -no-undefined

if BUILD_POSIX

# build the gtest library with the same flags as we are using
check_LTLIBRARIES = libgtest.la
libgtest_la_SOURCES = ../../googletest/googletest/src/gtest-all.cc
libgtest_la_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest
libgtest_la_LDFLAGS = -pthread

# build the main test program. This one does
# not test individual classes / modules, but
# is linked with the library and tests against
# its public API.

# we test the timer seperately because it can
# fail on slow or busy machines

#check_PROGRAMS = timer  mutex sema
check_PROGRAMS = sema

#timer_SOURCES = test_timer.cc
#
#timer_LDADD = libgtest.la ../../src/libosal.la
#
#timer_LDFLAGS = -pthread -Wall -Werror
#
#timer_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread
#
#
#
#mutex_SOURCES = test_mutex.cc
#
#mutex_LDADD = libgtest.la ../../src/libosal.la
#
#mutex_LDFLAGS = -pthread -Wall -Werror
#
#mutex_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread
#

sema_SOURCES = test_semaphore.cc

sema_LDADD = libgtest.la ../../src/libosal.la

sema_LDFLAGS = -pthread -Wall -Werror

sema_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


TESTS = sema

endif
