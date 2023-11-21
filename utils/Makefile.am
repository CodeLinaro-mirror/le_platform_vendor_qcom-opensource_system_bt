# sources and intermediate files are separated

external_bt_path := $(WORKSPACE)/bluetooth/stack/bluetooth_ext

AM_CFLAGS = -Wno-gnu-variable-sized-type-not-at-end \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -std=c++11 \
        -Wall \
        -Wextra \
        -Wno-typedef-redefinition \
        -Wno-unused-parameter \
        -DHAS_NO_BDROID_BUILDCFG

AM_CPPFLAGS= \
        -Iinclude \
        -I../btcore/include \
        -I../stack/include \
        -I../utils/include \
        -I../bta/include \
        -I../internal_include \
        -I$(external_bt_path)/vhal/include \
        -I../include \
        -I../types \
        -I../ \
        -I$(WORKSPACE)/bluetooth/bt_audio/hal/include \
        -I$(bdroid_C_INCLUDES)

if MDM_ROME
AM_CFLAGS += -DBT_SOC_TYPE_ROME
AM_CPPFLAGS += -DBT_SOC_TYPE_ROME
else
if MDM_PRONTO
AM_CFLAGS += -DHCI_USE_MCT
AM_CPPFLAGS += -DHCI_USE_MCT
endif
endif

if MDM_CHEROKEE
AM_CFLAGS += -DBT_SOC_TYPE_CHEROKEE
AM_CPPFLAGS += -DBT_SOC_TYPE_CHEROKEE
endif

if ACL_TIMING_CALC_SUPPORT
AM_CFLAGS += -DBT_ACL_TIMIMG_ENABLED=TRUE
AM_CPPFLAGS += -DBT_ACL_TIMIMG_ENABLED=TRUE
endif

if ACL_TIMING_CALC_SUPPORT
cpp_sources = \
        src/bt_utils.cc \
        src/bt_acl_timestamps.cc
else
cpp_sources = \
        src/bt_utils.cc
endif

common_LDFLAGS = -lpthread
common_CFLAGS =  $(AM_CFLAGS) -fPIC -DHAS_NO_BDROID_BUILDCFG

library_includedir = $(pkgincludedir)
lib_LTLIBRARIES = libbtutils.la
libbtutils_la_CC = @CC@
libbtutils_la_SOURCES = $(cpp_sources)
libbtutils_la_CPPFLAGS =$(AM_CPPFLAGS) -std=c++11 -Wall -DHAS_NO_BDROID_BUILDCFG -fPIC
libbtutils_la_CFLAGS = $(common_CFLAGS)
libbtutils_la_LDFLAGS = $(common_LDFLAGS) -static
