# sources and intermediate files are separated


AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -std=c++11 \
        -DHAS_NO_BDROID_BUILDCFG

AM_CPPFLAGS= \
        -I$(srcdir) \
        -I$(srcdir)/.. \
        -I$(srcdir)/../include \
        -I$(srcdir)/../types \
        -I$(srcdir)/../utils/include \
        -I$(srcdir)/../audio_a2dp_hw/include \
        -I$(bdroid_C_INCLUDES)

cpp_sources = \
        src/audio_a2dp_hw.cc \
        src/audio_a2dp_hw_utils.cc

common_LDFLAGS = -lpthread  -lrt ../osi/libbtosi.la -lchrome
common_CFLAGS =  $(AM_CFLAGS) -fPIC -D_GNU_SOURCE

library_includedir = $(pkgincludedir)
lib_LTLIBRARIES = libaudioa2dpdefault.la
libaudioa2dpdefault_la_CC = @CC@

libaudioa2dpdefault_la_SOURCES = $(cpp_sources)
libaudioa2dpdefault_la_CPPFLAGS =$(AM_CPPFLAGS)  $(common_CFLAGS)
libaudioa2dpdefault_la_CFLAGS = $(common_CFLAGS)
libaudioa2dpdefault_la_LDFLAGS = $(common_LDFLAGS)

