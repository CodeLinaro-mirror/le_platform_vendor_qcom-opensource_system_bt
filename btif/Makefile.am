# sources and intermediate files are separated

external_bt_path := $(top_srcdir)/../../bluetooth_ext

AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -std=c++11 \
        -DHAS_NO_BDROID_BUILDCFG
if MDM_CHEROKEE
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
AM_CFLAGS += -DAPTX_48000
endif

if MDM_HASTINGS
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
AM_CFLAGS += -DAPTX_48000
endif

if MDM_MOSELLE
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
AM_CFLAGS += -DAPTX_48000
endif

if MDM_HAMILTON
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
AM_CFLAGS += -DAPTX_48000
endif

AM_CPPFLAGS = \
        -I../ \
        -I../bta/include \
        -I../bta/sys \
        -I../bta/dm \
        -I../btcore/include \
        -I../include \
        -I../stack/include \
        -I../stack/l2cap \
        -I../stack/a2dp \
        -I../stack/btm \
        -I../stack/avdt \
        -I../hcis \
        -I../types \
        -I../include \
        -I../hcis/include \
        -I../hcis/patchram \
        -I../udrv/include \
        -I../btif/include \
        -I../btif/co \
        -I../hci/include\
        -I../vnd/include \
        -I../brcm/include \
        -I../embdrv/sbc/encoder/include \
        -I../embdrv/sbc/decoder/include \
        -I../audio_a2dp_hw \
        -I../utils/include \
        -I../internal_include 

AM_CPPFLAGS += @BLUETOOTHHOST_CFLAGS@ @BLUETOOTHEXT_CFLAGS@ @LIBCHROME_CFLAGS@

AM_CPPFLAGS += -DUSE_ANDROID_LOGGING -DUSE_LIBHW_AOSP

requiredlibs = @BLUETOOTHEXT_LIBS@
if MDM_CHEROKEE
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
AM_CPPFLAGS += -DAPTX_48000
endif

if MDM_HASTINGS
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
AM_CPPFLAGS += -DAPTX_48000
endif

if MDM_MOSELLE
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
AM_CPPFLAGS += -DAPTX_48000
endif

if MDM_HAMILTON
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
AM_CFLAGS += -DAPTX_48000
endif

# SSR_CLEANUP
if SSR_CLEANUP_SUPPORT
AM_CFLAGS += -DSSR_CLEANUP=TRUE
AM_CPPFLAGS += -DSSR_CLEANUP=TRUE
endif

AM_CPPFLAGS += -Wextra -fpermissive -D_GNU_SOURCE $(Z_LIB) -fPIC
cpp_sources = \
        src/bluetooth.cc \
        src/btif_a2dp.cc \
        src/btif_a2dp_control.cc \
        src/btif_a2dp_sink.cc \
        src/btif_a2dp_source.cc \
        src/btif_av.cc \
        src/btif_ble_advertiser.cc \
        src/btif_ble_scanner.cc \
        src/btif_config.cc \
        src/btif_core.cc \
        src/btif_debug.cc \
        src/btif_debug_btsnoop.cc \
        src/btif_debug_conn.cc \
        src/btif_dm.cc \
        src/btif_gatt.cc \
        src/btif_gatt_client.cc \
        src/btif_gatt_server.cc \
        src/btif_gatt_test.cc \
        src/btif_gatt_util.cc \
        src/btif_hf.cc \
        src/btif_hf_client.cc \
        src/btif_hh.cc \
        src/btif_hd.cc \
        src/btif_hl.cc \
        src/btif_mce.cc \
        src/btif_pan.cc \
        src/btif_profile_queue.cc \
        src/btif_rc.cc \
        src/btif_sdp.cc \
        src/btif_sdp_server.cc \
        src/btif_sm.cc \
        src/btif_sock.cc \
        src/btif_sock_rfc.cc \
        src/btif_sock_l2cap.cc \
        src/btif_sock_sco.cc \
        src/btif_sock_sdp.cc \
        src/btif_sock_thread.cc \
        src/btif_sock_util.cc \
        src/btif_storage.cc \
        src/btif_uid.cc \
        src/btif_util.cc \
        src/stack_manager.cc \
        co/bta_ag_co.cc \
        co/bta_dm_co.cc \
        co/bta_av_co.cc \
        co/bta_hh_co.cc \
        co/bta_hl_co.cc \
        co/bta_pan_co.cc \
        co/bta_gatts_co.cc \
        src/btif_obex_sock.cc

common_LDFLAGS = -lpthread  -lrt -lresolv ../bta/libbtbta.la ../osi/libbtosi.la \
                  ../device/libbtdevice.la ../audio_a2dp_hw/libaudioa2dpdefault.la

common_LDFLAGS += -llog -lcutils

common_CFLAGS =  $(AM_CFLAGS) -D_GNU_SOURCE $(Z_LIB) -fPIC

library_includedir = $(pkgincludedir)\
library_include_HEADERS = $(h_sources)
lib_LTLIBRARIES = libbtif.la
libbtif_la_CC = @CC@
libbtif_la_SOURCES = $(cpp_sources)
libbtif_la_CPPFLAGS = $(AM_CPPFLAGS) -std=c++14 -Wall -DHAS_NO_BDROID_BUILDCFG -DOBEX_SUPPORTED
libbtif_la_CFLAGS = $(common_CFLAGS)
libbtif_la_LDFLAGS = $(common_LDFLAGS) -static -lbttypes
