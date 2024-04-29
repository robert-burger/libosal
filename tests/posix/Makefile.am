# This setup uses the configuration explained in
# detail in
# https://stackoverflow.com/questions/35998856/how-can-i-use-google-test-with-my-project-that-builds-via-autotools

ACLOCAL_AMFLAGS = -I m4
AM_LDFLAGS = -no-undefined

#############################################################################
##
## This section below partially replicates the make script for the
## library and builds a copy of libosal.la with extra flags for
## testing and coverage analysis.
##
## This duplication is not yet satisfying, but seems for a start
## necessary for setting the right flags for coverage analysis, and
## could also help with debugging.
##
#############################################################################

includedir = $(prefix)/include/libosal
includeposixdir = $(prefix)/include/libosal/posix

lib_LTLIBRARIES = libosal.la

include_HEADERS = $(top_srcdir)/include/libosal/osal.h \
				  $(top_srcdir)/include/libosal/config.h \
				  $(top_srcdir)/include/libosal/types.h \
				  $(top_srcdir)/include/libosal/mutex.h \
				  $(top_srcdir)/include/libosal/task.h \
				  $(top_srcdir)/include/libosal/timer.h \
				  $(top_srcdir)/include/libosal/semaphore.h \
				  $(top_srcdir)/include/libosal/spinlock.h \
				  $(top_srcdir)/include/libosal/binary_semaphore.h \
				  $(top_srcdir)/include/libosal/condvar.h \
				  $(top_srcdir)/include/libosal/queue.h \
				  $(top_srcdir)/include/libosal/trace.h \
				  $(top_srcdir)/include/libosal/shm.h \
				  $(top_srcdir)/include/libosal/io.h

if HAVE_MQUEUE_H
include_HEADERS += $(top_srcdir)/include/libosal/mq.h
endif

includeposix_HEADERS = 

libosal_la_SOURCES	= $(top_srcdir)/src/io.c  \
			$(top_srcdir)/src/osal.c  \
			$(top_srcdir)/src/trace.c \
			$(top_srcdir)/src/timer.c 

ADD_LIBS = @MATH_LIBS@
ADD_CFLAGS = 

if BUILD_POSIX
includeposix_HEADERS    += $(top_srcdir)/include/libosal/posix/binary_semaphore.h \
						   $(top_srcdir)/include/libosal/posix/condvar.h \
						   $(top_srcdir)/include/libosal/posix/mutex.h \
						   $(top_srcdir)/include/libosal/posix/semaphore.h \
						   $(top_srcdir)/include/libosal/posix/task.h \
						   $(top_srcdir)/include/libosal/posix/timer.h \
						   $(top_srcdir)/include/libosal/posix/shm.h \
						   $(top_srcdir)/include/libosal/posix/spinlock.h 

libosal_la_SOURCES += $(top_srcdir)/src/posix/binary_semaphore.c
libosal_la_SOURCES += $(top_srcdir)/src/posix/mutex.c
libosal_la_SOURCES += $(top_srcdir)/src/posix/condvar.c
libosal_la_SOURCES += $(top_srcdir)/src/posix/task.c
libosal_la_SOURCES += $(top_srcdir)/src/posix/timer.c
libosal_la_SOURCES += $(top_srcdir)/src/posix/semaphore.c
libosal_la_SOURCES += $(top_srcdir)/src/posix/spinlock.c
libosal_la_SOURCES += $(top_srcdir)/src/posix/io.c

if HAVE_MQUEUE_H
includeposix_HEADERS    += $(top_srcdir)/include/libosal/posix/mq.h
libosal_la_SOURCES += $(top_srcdir)/src/posix/mq.c
endif

if HAVE_SYS_MMAN_H
libosal_la_SOURCES += $(top_srcdir)/src/posix/shm.c
endif

ADD_LIBS += @PTHREAD_LIBS@ @RT_LIBS@
ADD_CFLAGS += -Wno-unused-const-variable --coverage -O0
endif


libosal_la_CFLAGS	= -std=gnu99 -Wall -Werror $(ADD_CFLAGS) -I$(top_srcdir)/include
libosal_la_LDFLAGS	= -Wall -Werror $(AM_LDFLAGS) $(ADD_LIBS)
libosal_la_LIBS     = -lm $(ADD_LIBS)



##################################################################################
## end of build script for libosal.la
##################################################################################


if BUILD_POSIX

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
		 check_mutex check_spinlock check_tasks \
		 check_messagequeue check_sharedmemory

check_timer_SOURCES = test_timer.cc

check_timer_LDADD = libgtest.la libosal.la

check_timer_LDFLAGS = -pthread -Wall -Werror

check_timer_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of Mutex

check_mutex_SOURCES = test_mutex.cc

check_mutex_LDADD = libgtest.la libosal.la

check_mutex_LDFLAGS = -pthread -Wall -Werror

check_mutex_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of tasks

check_tasks_SOURCES = test_tasks.cc

check_tasks_LDADD = libgtest.la libosal.la

check_tasks_LDFLAGS = -pthread -Wall -Werror

check_tasks_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of counting semaphore

check_sema_SOURCES = test_semaphore.cc

check_sema_LDADD = libgtest.la libosal.la

check_sema_LDFLAGS = -pthread -Wall -Werror

check_sema_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

# check of binary semaphore

check_binarysema_SOURCES = test_binary_semaphore.cc

check_binarysema_LDADD = libgtest.la libosal.la

check_binarysema_LDFLAGS = -pthread -Wall -Werror

check_binarysema_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of condition variables

check_condvar_SOURCES = test_condvar.cc

check_condvar_LDADD = libgtest.la libosal.la

check_condvar_LDFLAGS = -pthread -Wall -Werror

check_condvar_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

# check of spinlocks

check_spinlock_SOURCES = test_spinlock.cc

check_spinlock_LDADD = libgtest.la libosal.la

check_spinlock_LDFLAGS = -pthread -Wall -Werror

check_spinlock_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread


# check of inter-process message queues

check_messagequeue_SOURCES = test_messagequeue.cc test_messagequeue_timed.cc

check_messagequeue_LDADD = libgtest.la libosal.la

check_messagequeue_LDFLAGS = -pthread -Wall -Werror

check_messagequeue_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread



# check of shared memory

check_sharedmemory_SOURCES = test_sharedmemory.cc
check_sharedmemory_LDADD = libgtest.la libosal.la

check_sharedmemory_LDFLAGS = -pthread -Wall -Werror

check_sharedmemory_CPPFLAGS = -Wall -Werror -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -I$(top_srcdir)/include -pthread

# you can quickly run individual tests, for example using
# "make check TESTS=check_mutex"

TESTS = check_spinlock check_condvar check_binarysema \
	check_sema check_timer check_mutex check_tasks \
	check_messagequeue check_sharedmemory



endif
