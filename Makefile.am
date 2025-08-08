ACLOCAL_AMFLAGS = -I m4
pkgconfigdir=$(libdir)/pkgconfig

#include $(top_srcdir)/aminclude.am

SUBDIRS=src

if LIBOSAL_BUILD_POSIX
SUBDIRS += tests
if !LIBOSAL_BUILD_MINGW32
SUBDIRS += src/tools/logger 
SUBDIRS += src/tools/shmtest
endif
endif

pkgconfig_DATA = libosal.pc


