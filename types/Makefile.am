# sources and intermediate files are separated

external_bt_path := $(WOEKSPACE)/bluetooth/stack/bluetooth_ext

AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -std=c++11 \
        -DHAS_NO_BDROID_BUILDCFG

AM_CPPFLAGS = \
        -I../

cpp_sources = \
    raw_address.cc \
    bluetooth/uuid.cc

common_LDFLAGS = -lpthread
common_CFLAGS =  $(AM_CFLAGS) -fPIC -D_GNU_SOURCE

library_includedir = $(pkgincludedir)\
library_include_HEADERS = $(h_sources)
lib_LTLIBRARIES = libbttypes.la
libbttypes_la_CC = @CC@
libbttypes_la_SOURCES = $(cpp_sources)
libbttypes_la_CPPFLAGS = $(AM_CPPFLAGS) -std=c++11 -Wall -fPIC
libbttypes_la_CPPFLAGS += $(AM_CFLAGS)
libbttypes_la_CFLAGS = $(common_CFLAGS)
libbttypes_la_LDFLAGS = $(common_LDFLAGS) -static

fluoridetypesincludesdir = $(includedir)/fluoride/types
fluoridetypesbluetoothincludesdir = $(includedir)/fluoride/bluetooth

fluoridetypesincludes_HEADERS = $(wildcard *.h)
fluoridetypesbluetoothincludes_HEADERS = $(wildcard bluetooth/*.h)
