ACLOCAL_AMFLAGS = -I m4
pkgconfigdir=$(libdir)/pkgconfig

#include $(top_srcdir)/aminclude.am

SUBDIRS=src

if BUILD_POSIX
if !BUILD_MINGW32
SUBDIRS += src/tools/logger 
SUBDIRS += src/tools/shmtest
endif
endif

pkgconfig_DATA = libosal.pc


