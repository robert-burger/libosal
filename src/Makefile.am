ACLOCAL_AMFLAGS = -I m4

includedir = $(prefix)/include/libosal
includeposixdir = $(prefix)/include/libosal/posix
includepikeosdir = $(prefix)/include/libosal/pikeos
includevxworksdir = $(prefix)/include/libosal/vxworks

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
				  $(top_srcdir)/include/libosal/shm.h \
				  $(top_srcdir)/include/libosal/io.h

includeposix_HEADERS = 
includepikeos_HEADERS = 
includevxworks_HEADERS =

libosal_la_SOURCES	= 

if BUILD_POSIX
includeposix_HEADERS    += $(top_srcdir)/include/libosal/posix/binary_semaphore.h \
						   $(top_srcdir)/include/libosal/posix/condvar.h \
						   $(top_srcdir)/include/libosal/posix/mutex.h \
				  		   $(top_srcdir)/include/libosal/posix/semaphore.h \
				  		   $(top_srcdir)/include/libosal/posix/task.h \
				  		   $(top_srcdir)/include/libosal/posix/spinlock.h 

libosal_la_SOURCES += posix/binary_semaphore.c
libosal_la_SOURCES += posix/mutex.c
libosal_la_SOURCES += posix/condvar.c
libosal_la_SOURCES += posix/task.c
libosal_la_SOURCES += posix/timer.c
libosal_la_SOURCES += posix/semaphore.c
libosal_la_SOURCES += posix/spinlock.c
libosal_la_SOURCES += posix/io.c
libosal_la_SOURCES += posix/shm.c
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

libosal_la_CFLAGS	= -std=gnu99 -Wall -Werror -Wno-unused-const-variable -I$(top_srcdir)/include
libosal_la_LDFLAGS	= -Wall -Werror -no-undefined
libosal_la_LIBS     = -lm

