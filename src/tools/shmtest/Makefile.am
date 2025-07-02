ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = shmtest
shmtest_SOURCES = main.c 
shmtest_CFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include
shmtest_LDADD = $(top_builddir)/src/.libs/libosal.la 
shmtest_LDFLAGS =

if BUILD_PIKEOS
shmtest_LDADD += $(PIKEOS_LIBS)
shmtest_LDFLAGS += $(PIKEOS_LDFLAGS)
endif

