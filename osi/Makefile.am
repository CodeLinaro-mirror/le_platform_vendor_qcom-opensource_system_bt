# sources and intermediate files are separated

external_bt_path := $(BT_SOURCE)/stack/bluetooth_ext

AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -std=c++11 \
        -DHAS_NO_BDROID_BUILDCFG \
        -D__GLIBC__

AM_CPPFLAGS = \
        -I$(srcdir)/.. \
        -I$(srcdir)/../utils/include \
        -I$(srcdir)/../stack/include \
        -I$(srcdir)/../include \
        -I$(srcdir)/../types \
        -I$(BT_SOURCE)/bt_audio/hal/include \
        -I$(external_bt_path)/vhal/include

cpp_sources = \
        src/alarm.cc \
        src/allocation_tracker.cc \
        src/allocator.cc \
        src/array.cc \
        src/buffer.cc \
        src/config.cc \
        src/compat.cc \
        src/fixed_queue.cc \
        src/future.cc \
        src/hash_map_utils.cc \
        src/list.cc \
        src/mutex.cc \
        src/osi.cc \
        src/properties.cc \
        src/reactor.cc \
        src/ringbuffer.cc \
        src/semaphore.cc \
        src/socket.cc \
        src/socket_utils/socket_local_client.cc \
        src/socket_utils/socket_local_server.cc \
        src/thread.cc \
        src/time.cc \
        src/wakelock.cc \
        ${external_bt_path}/system_bt_ext/osi/src/vnd_log.cc

common_LDFLAGS = -lpthread
common_CFLAGS =  $(AM_CFLAGS) -fPIC -D_GNU_SOURCE

library_includedir = $(pkgincludedir)\
library_include_HEADERS = $(h_sources)
lib_LTLIBRARIES = libbtosi.la
libbtosi_la_CC = @CC@
libbtosi_la_SOURCES = $(cpp_sources)
libbtosi_la_CPPFLAGS = $(AM_CPPFLAGS)
libbtosi_la_CPPFLAGS += $(common_CFLAGS) @GLIB_CFLAGS@
libbtosi_la_CFLAGS = $(common_CFLAGS)
libbtosi_la_LDFLAGS = $(common_LDFLAGS) @GLIB_LIBS@ -lpropertyvault -static  -version-number @LT_VERSION_NUMBER@
