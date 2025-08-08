# This setup uses the configuration explained in
# detail in
# https://stackoverflow.com/questions/35998856/how-can-i-use-google-test-with-my-project-that-builds-via-autotools

ACLOCAL_AMFLAGS = -I m4
AM_LDFLAGS = -no-undefined


if LIBOSAL_BUILD_POSIX

cov-reset:
	rm -fr coverage
	@find ../.. -name "*.gcda" -o -name "*.gcno" -exec rm {} \;

cov-report:
	@mkdir -p coverage
	@gcov ../../src/*.c ../../src/posix/*.c *.cc \
		../../src/*.gcno ../../src/posix/*.gcno  *.gcno
	@echo result coverage reports are in *.cc.gcov
	@echo run run-gcovr.sh for additional graphical display of coverage in HTML

cov:
	@make cov-report

clean-local:
	@make cov-reset 

check:
	@make cov


AM_CXXFLAGS = --coverage -O0 -I$(top_srcdir)/src

# Build the gtest library with the same flags as we are using.
# Building gtest is necessary to avoid violations of
# the C++ one definition rule, which could cause
# undefined behavior. (This is due to the fact that
# gtest is a C++ library which does not use the
# C ABI).

check_LTLIBRARIES = libgtest.la
libgtest_la_SOURCES = ../../googletest/googletest/src/gtest-all.cc
libgtest_la_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include \
			-I$(top_srcdir)/googletest/googletest
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


# check of condition variables

check_PROGRAMS = check_condvar check_binarysema check_sema check_timer \
		 check_mutex check_spinlock check_tasks                \
		 check_messagequeue check_sharedmemory check_io        \
		 check_shmio check_trace check_mqsignals               \
		 check_messagequeue

check_timer_SOURCES = test_timer.cc

check_timer_LDADD = libgtest.la ../../src/libosal.la

check_timer_LDFLAGS = -pthread -Wall -Werror

check_timer_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of Mutex

check_mutex_SOURCES = test_mutex.cc test_mutex_priority_inversion.cc

check_mutex_LDADD = libgtest.la ../../src/libosal.la

check_mutex_LDFLAGS = -pthread -Wall -Werror

check_mutex_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of tasks

check_tasks_SOURCES = test_tasks.cc

check_tasks_LDADD = libgtest.la ../../src/libosal.la

check_tasks_LDFLAGS = -pthread -Wall -Werror

check_tasks_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of counting semaphore

check_sema_SOURCES = test_semaphore.cc

check_sema_LDADD = libgtest.la ../../src/libosal.la

check_sema_LDFLAGS = -pthread -Wall -Werror

check_sema_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

# check of binary semaphore

check_binarysema_SOURCES = test_binary_semaphore.cc

check_binarysema_LDADD = libgtest.la ../../src/libosal.la

check_binarysema_LDFLAGS = -pthread -Wall -Werror

check_binarysema_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of condition variables

check_condvar_SOURCES = test_condvar.cc

check_condvar_LDADD = libgtest.la ../../src/libosal.la

check_condvar_LDFLAGS = -pthread -Wall -Werror

check_condvar_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

# check of spinlocks

check_spinlock_SOURCES = test_spinlock.cc

check_spinlock_LDADD = libgtest.la ../../src/libosal.la

check_spinlock_LDFLAGS = -pthread -Wall -Werror

check_spinlock_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of inter-process message queues

check_messagequeue_SOURCES = test_messagequeue.cc test_messagequeue_timed.cc

check_messagequeue_LDADD = libgtest.la ../../src/libosal.la

check_messagequeue_LDFLAGS = -pthread -Wall -Werror

check_messagequeue_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

# check of shared memory

check_sharedmemory_SOURCES = test_sharedmemory.cc
check_sharedmemory_LDADD = libgtest.la ../../src/libosal.la

check_sharedmemory_LDFLAGS = -pthread -Wall -Werror

check_sharedmemory_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of IO routines

check_io_SOURCES = test_io.cc
check_io_LDADD = libgtest.la ../../src/libosal.la

check_io_LDFLAGS = -pthread -Wall -Werror

check_io_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

# check of shm io routines

check_shmio_SOURCES = test_shmio.cc
check_shmio_LDADD = libgtest.la ../../src/libosal.la

check_shmio_LDFLAGS = -pthread -Wall -Werror

check_shmio_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of tracing routines

check_trace_SOURCES = test_trace.cc
check_trace_LDADD = libgtest.la ../../src/libosal.la

check_trace_LDFLAGS = -pthread -Wall -Werror

check_trace_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

# check of inter-process message queues

check_mqsignals_SOURCES = test_mqsignals.cc

check_mqsignals_LDADD = libgtest.la ../../src/libosal.la

check_mqsignals_LDFLAGS = -pthread -Wall -Werror

check_mqsignals_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

# you can quickly run individual tests, for example using
# "make check TESTS=check_mutex"

TESTS = check_spinlock check_condvar check_binarysema  \
	check_sema check_timer check_mutex check_tasks \
	check_messagequeue check_sharedmemory check_io \
	check_shmio check_trace  check_mqsignals




endif
