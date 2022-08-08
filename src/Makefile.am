ACLOCAL_AMFLAGS = -I m4

includedir = $(prefix)/include

lib_LTLIBRARIES = libosal.la

include_HEADERS = $(top_srcdir)/include/libosal/osal.h

libosal_la_SOURCES	= test.c

if BUILD_POSIX
libosal_la_SOURCES += posix/mutex.c
endif

libosal_la_CFLAGS	= -std=gnu99 -Wall -Werror -Wno-unused-const-variable -I$(top_srcdir)/include
libosal_la_LDFLAGS	= -Wall -Werror 
libosal_la_LIBS     = -lm

