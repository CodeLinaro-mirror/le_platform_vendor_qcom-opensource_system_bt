# sources and intermediate files are separated

AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -std=c99 \
        -DHAS_NO_BDROID_BUILDCFG

AM_CPPFLAGS = \
        -I$(srcdir)/.. \
        -I$(srcdir)/include \
        -I$(srcdir)/../types \
        -I$(srcdir)/../include

cpp_sources = \
        src/device_class.cc \
        src/module.cc \
        src/osi_module.cc \
        src/property.cc

common_LDFLAGS = -lpthread  -lrt
common_CFLAGS =  $(AM_CFLAGS)

library_includedir = $(pkgincludedir)\
library_include_HEADERS = $(h_sources)
lib_LTLIBRARIES = libbtcore.la
libbtcore_la_CC = @CC@
libbtcore_la_SOURCES = $(cpp_sources)
libbtcore_la_CPPFLAGS = $(AM_CPPFLAGS) -std=c++11 -Wall -fPIC -D_GNU_SOURCE -DHAS_NO_BDROID_BUILDCFG
libbtcore_la_CFLAGS = $(common_CFLAGS)
libbtcore_la_LDFLAGS = $(common_LDFLAGS) -static -version-number @LT_VERSION_NUMBER@
