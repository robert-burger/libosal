ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = logger
logger_SOURCES = main.c 
logger_CFLAGS = -I$(top_srcdir)/include
logger_LDADD = $(top_builddir)/src/.libs/libosal.la 
logger_LDFLAGS =

if BUILD_PIKEOS
logger_LDADD += $(PIKEOS_LIBS)
logger_LDFLAGS += $(PIKEOS_LDFLAGS)
endif

