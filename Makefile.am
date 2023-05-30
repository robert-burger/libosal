ACLOCAL_AMFLAGS = -I m4

#include $(top_srcdir)/aminclude.am

SUBDIRS=src

if BUILD_POSIX
if !BUILD_MINGW32
SUBDIRS += src/tools/logger 
endif
endif

pkgconfig_DATA = libosal.pc


