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

# we test the parts seperately because some, such as timer or
# semaphore, can fail on slow or busy machines.

# The "check_" prefix here is somewhat clumsy
# bit it is actually needed to prevent clashes with
# google_test internal symbols!

check_PROGRAMS = check_condvar check_binarysema check_sema check_timer \
		 check_mutex check_spinlock check_tasks \
		 check_messagequeue

check_timer_SOURCES = test_timer.cc

check_timer_LDADD = libgtest.la ../../src/libosal.la

check_timer_LDFLAGS = -pthread -Wall -Werror

check_timer_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread



check_mutex_SOURCES = test_mutex.cc

check_mutex_LDADD = libgtest.la ../../src/libosal.la

check_mutex_LDFLAGS = -pthread -Wall -Werror

check_mutex_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

check_tasks_SOURCES = test_tasks.cc

check_tasks_LDADD = libgtest.la ../../src/libosal.la

check_tasks_LDFLAGS = -pthread -Wall -Werror

check_tasks_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


check_sema_SOURCES = test_semaphore.cc

check_sema_LDADD = libgtest.la ../../src/libosal.la

check_sema_LDFLAGS = -pthread -Wall -Werror

check_sema_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


check_binarysema_SOURCES = test_binary_semaphore.cc

check_binarysema_LDADD = libgtest.la ../../src/libosal.la

check_binarysema_LDFLAGS = -pthread -Wall -Werror

check_binarysema_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


check_condvar_SOURCES = test_condvar.cc

check_condvar_LDADD = libgtest.la ../../src/libosal.la

check_condvar_LDFLAGS = -pthread -Wall -Werror

check_condvar_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

check_spinlock_SOURCES = test_spinlock.cc

check_spinlock_LDADD = libgtest.la ../../src/libosal.la

check_spinlock_LDFLAGS = -pthread -Wall -Werror

check_spinlock_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

check_messagequeue_SOURCES = test_messagequeue.cc test_messagequeue_timed.cc

check_messagequeue_LDADD = libgtest.la ../../src/libosal.la

check_messagequeue_LDFLAGS = -pthread -Wall -Werror

check_messagequeue_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread



TESTS = check_spinlock check_condvar check_binarysema \
	check_sema check_timer check_mutex check_tasks \
	check_messagequeue


endif
