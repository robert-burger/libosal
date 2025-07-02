ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = lec_logger
lec_logger_SOURCES = main.c 
lec_logger_CFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include
lec_logger_LDADD = $(top_builddir)/src/.libs/libosal.la 
lec_logger_LDFLAGS =

if BUILD_PIKEOS
lec_logger_LDADD += $(PIKEOS_LIBS)
lec_logger_LDFLAGS += $(PIKEOS_LDFLAGS)
endif

