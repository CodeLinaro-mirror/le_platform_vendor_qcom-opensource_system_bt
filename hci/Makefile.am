# sources and intermediate files are separated

external_bt_path := $(top_srcdir)/../../bluetooth_ext

AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -std=c++11 \
        -DHAS_NO_BDROID_BUILDCFG

AM_CPPFLAGS = -Iinclude -I../include \
        -I../btcore/include \
        -I../stack/include \
        -I../utils/include \
        -I../bta/include \
        -I../types \
        -I../internal_include \
        -I$(bdroid_C_INCLUDES) \
        -I${D}${includedir}

AM_CPPFLAGS += @LIBCHROME_CFLAGS@

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

if MDM_HASTINGS
AM_CFLAGS += -DBT_SOC_TYPE_HASTINGS
AM_CPPFLAGS += -DBT_SOC_TYPE_HASTINGS
endif

if MDM_MOSELLE
AM_CFLAGS += -DBT_SOC_TYPE_MOSELLE
AM_CPPFLAGS += -DBT_SOC_TYPE_MOSELLE
endif

if MDM_HAMILTON
AM_CFLAGS += -DBT_SOC_TYPE_HAMILTON
AM_CPPFLAGS += -DBT_SOC_TYPE_HAMILTON
endif

if BTC_SUPPORT
AM_CFLAGS += -DBTC_INCLUDED=TRUE
AM_CPPFLAGS += -DBTC_INCLUDED=TRUE
endif

if SSR_CLEANUP_SUPPORT
AM_CFLAGS += -DSSR_CLEANUP=TRUE
AM_CPPFLAGS += -DSSR_CLEANUP=TRUE
endif

cpp_sources = \
        src/btsnoop.cc \
        src/btsnoop_mem.cc \
        src/btsnoop_net.cc \
        src/buffer_allocator.cc \
        src/hci_inject.cc \
        src/hci_layer.cc \
        src/hci_packet_factory.cc \
        src/hci_packet_parser.cc \
        src/packet_fragmenter.cc

if USING_HIDL
AM_CFLAGS += -DUSING_HIDL
AM_CPPFLAGS += -DUSING_HIDL
cpp_sources += src/hci_layer_android.cc
common_LDFLAGS = -lpthread -lrt
endif

if USING_SOCKET
AM_CFLAGS += -DUSING_SOCKET
AM_CPPFLAGS += -DUSING_SOCKET
cpp_sources += src/hci_layer_ipc.cc
common_LDFLAGS = -lpthread -lrt
endif

if USING_STATIC_LINK
AM_CFLAGS += -DUSING_STATIC_LINK
AM_CPPFLAGS += -DUSING_STATIC_LINK
cpp_sources += src/hci_layer_le.cc
common_LDFLAGS = -lpthread -lrt
endif


common_CFLAGS =  $(AM_CFLAGS) -fPIC -D_GNU_SOURCE -DBTSNOOP_DEFAULT=TRUE -DBT_NET_DEBUG=TRUE
common_CPPFLAGS = $(common_CFLAGS) -DHAS_NO_BDROID_BUILDCFG -fPIC -DBTSNOOP_DEFAULT=TRUE -DBT_NET_DEBUG=TRUE
library_includedir = $(pkgincludedir)\
library_include_HEADERS = $(h_sources)
btHci_includedir = $(includedir)/
btHci_include_HEADERS = $(srcdir)/include/bt_vendor_lib.h
lib_LTLIBRARIES = libbthci.la
libbthci_la_CC = @CC@
libbthci_la_SOURCES = $(cpp_sources)
libbthci_la_CPPFLAGS = $(AM_CPPFLAGS) -std=c++11 $(common_CPPFLAGS)
libbthci_la_CFLAGS = $(common_CFLAGS)
libbthci_la_LDFLAGS := $(common_LDFLAGS) -static
