ACLOCAL_AMFLAGS = -I m4
pkgconfigdir=$(libdir)/pkgconfig

#include $(top_srcdir)/aminclude.am

SUBDIRS=src

if LIBOSAL_BUILD_POSIX
if !LIBOSAL_BUILD_RTEMS
SUBDIRS += tests
if !LIBOSAL_BUILD_MINGW32
SUBDIRS += src/tools/logger 
SUBDIRS += src/tools/shmtest
endif
endif
endif

pkgconfig_DATA = libosal.pc

pkgcmakedir = $(libdir)/cmake/libosal
dist_pkgcmake_DATA = cmake/libosal/libosalConfig.cmake

