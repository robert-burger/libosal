ACLOCAL_AMFLAGS = -I m4
AM_LDFLAGS = -no-undefined

includedir = $(prefix)/include/libosal
includeposixdir = $(prefix)/include/libosal/posix
includepikeosdir = $(prefix)/include/libosal/pikeos
includevxworksdir = $(prefix)/include/libosal/vxworks
includewin32dir = $(prefix)/include/libosal/win32

lib_LTLIBRARIES = libosal.la

include_HEADERS = $(top_srcdir)/include/libosal/osal.h \
				  $(top_builddir)/include/libosal/config.h \
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
includepikeos_HEADERS = 
includevxworks_HEADERS =
includewin32_HEADERS =

libosal_la_SOURCES	= io.c osal.c trace.c timer.c

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

libosal_la_SOURCES += posix/binary_semaphore.c
libosal_la_SOURCES += posix/mutex.c
libosal_la_SOURCES += posix/condvar.c
libosal_la_SOURCES += posix/task.c
libosal_la_SOURCES += posix/timer.c
libosal_la_SOURCES += posix/semaphore.c
libosal_la_SOURCES += posix/spinlock.c
libosal_la_SOURCES += posix/io.c

if HAVE_MQUEUE_H
includeposix_HEADERS    += $(top_srcdir)/include/libosal/posix/mq.h
libosal_la_SOURCES += posix/mq.c
endif

if HAVE_SYS_MMAN_H
libosal_la_SOURCES += posix/shm.c
endif

ADD_LIBS += @PTHREAD_LIBS@ @RT_LIBS@
ADD_CFLAGS += -Wno-unused-const-variable
endif

if BUILD_VXWORKS

libosal_la_SOURCES += vxworks/binary_semaphore.c
libosal_la_SOURCES += vxworks/condvar.c
libosal_la_SOURCES += vxworks/mutex.c
libosal_la_SOURCES += vxworks/task.c
libosal_la_SOURCES += vxworks/semaphore.c

endif

if BUILD_PIKEOS

includepikeos_HEADERS   += $(top_srcdir)/include/libosal/pikeos/binary_semaphore.h \
						   $(top_srcdir)/include/libosal/pikeos/condvar.h \
						   $(top_srcdir)/include/libosal/pikeos/mutex.h \
				  		   $(top_srcdir)/include/libosal/pikeos/semaphore.h \
				  		   $(top_srcdir)/include/libosal/pikeos/task.h \
				  		   $(top_srcdir)/include/libosal/pikeos/spinlock.h \
				  		   $(top_srcdir)/include/libosal/pikeos/io.h \
				  		   $(top_srcdir)/include/libosal/pikeos/shm.h 

libosal_la_SOURCES += pikeos/binary_semaphore.c
libosal_la_SOURCES += pikeos/condvar.c
libosal_la_SOURCES += pikeos/mutex.c
libosal_la_SOURCES += pikeos/semaphore.c
libosal_la_SOURCES += pikeos/task.c
libosal_la_SOURCES += pikeos/timer.c
libosal_la_SOURCES += pikeos/io.c
libosal_la_SOURCES += pikeos/shm.c

endif

if BUILD_WIN32

includewin32_HEADERS   += 
						   $(top_srcdir)/include/libosal/win32/mutex.h 

libosal_la_SOURCES += win32/mutex.c
libosal_la_SOURCES += win32/semaphore.c

endif

libosal_la_CFLAGS	= -std=gnu99 -Wall -Werror $(ADD_CFLAGS) -I$(top_srcdir)/include -I$(top_builddir)/include
libosal_la_LDFLAGS	= -Wall -Werror $(AM_LDFLAGS) $(ADD_LIBS)
libosal_la_LIBS     = -lm $(ADD_LIBS)

install-exec-hook: 
	find $(DESTDIR)$(libdir) -type f -name \*.la -delete


