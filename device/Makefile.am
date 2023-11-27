# sources and intermediate files are separated

external_bt_path := $(WORKSPACE)/bluetooth/stack/bluetooth_ext

AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -std=c++11 \
        -DHAS_NO_BDROID_BUILDCFG

AM_CPPFLAGS = \
        -I../ \
        -Iinclude \
        -I../btcore/include \
        -I../hci/include \
        -I../include \
        -I../types \
        -I../internal_include \
        -I../stack/include \
        -I$(external_bt_path)/system_bt_ext \
        -I$(external_bt_path)/vhal/include \
        -I$(external_bt_path)/system_bt_ext/device/include \
        -I$(WORKSPACE)/vendor/qcom/opensource/bluetooth/hal/include

cpp_sources = \
        src/controller.cc \
        src/esco_parameters.cc \
        ${external_bt_path}/system_bt_ext/device/src/interop.cc \
        ${external_bt_path}/system_bt_ext/device/src/profile_config.cc

common_LDFLAGS = -lpthread
common_CFLAGS =  $(AM_CFLAGS) -fPIC -D_GNU_SOURCE

library_includedir = $(pkgincludedir)\
library_include_HEADERS = $(h_sources)
lib_LTLIBRARIES = libbtdevice.la
libbtdevice_la_CC = @CC@
libbtdevice_la_SOURCES = $(cpp_sources)
libbtdevice_la_CPPFLAGS = $(AM_CPPFLAGS) -DHAS_NO_BDROID_BUILDCFG -fPIC
libbtdevice_la_CPPFLAGS += $(AM_CFLAGS)
libbtdevice_la_CFLAGS = $(common_CFLAGS)
libbtdevice_la_LDFLAGS = $(common_LDFLAGS) -static -lbttypes
