# sources and intermediate files are separated

external_bt_path := $(top_srcdir)/../../bluetooth_ext

AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -std=c++11 \
        -DHAS_NO_BDROID_BUILDCFG

AM_CPPFLAGS= \
        -I../ \
        -I../audio_a2dp_hw/include \
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
        -I../types \
        -I../internal_include

AM_CPPFLAGS += -DUSE_ANDROID_LOGGING -DUSE_LIBHW_AOSP

AM_CPPFLAGS += @BLUETOOTHHOST_CFLAGS@ @LIBCHROME_CFLAGS@

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
# platform specific
cpp_sources = \
        bte_main.cc \
        bte_init.cc \
        bte_logmsg.cc \
        bte_conf.cc \
        bte_init_cpp_logging.cc \
        stack_config.cc

# SSR_CLEANUP
if SSR_CLEANUP_SUPPORT
AM_CFLAGS += -DSSR_CLEANUP=TRUE
AM_CPPFLAGS += -DSSR_CLEANUP=TRUE
endif

# sbc encoder and decoder
c_sources = \
    ../embdrv/sbc/encoder/srce/sbc_analysis.c \
    ../embdrv/sbc/encoder/srce/sbc_dct.c \
    ../embdrv/sbc/encoder/srce/sbc_dct_coeffs.c \
    ../embdrv/sbc/encoder/srce/sbc_enc_bit_alloc_mono.c \
    ../embdrv/sbc/encoder/srce/sbc_enc_bit_alloc_ste.c \
    ../embdrv/sbc/encoder/srce/sbc_enc_coeffs.c \
    ../embdrv/sbc/encoder/srce/sbc_encoder.c \
    ../embdrv/sbc/encoder/srce/sbc_packing.c \
    ../embdrv/sbc/decoder/srce/alloc.c \
    ../embdrv/sbc/decoder/srce/bitalloc.c \
    ../embdrv/sbc/decoder/srce/bitalloc-sbc.c \
    ../embdrv/sbc/decoder/srce/bitstream-decode.c \
    ../embdrv/sbc/decoder/srce/decoder-oina.c \
    ../embdrv/sbc/decoder/srce/decoder-private.c \
    ../embdrv/sbc/decoder/srce/decoder-sbc.c \
    ../embdrv/sbc/decoder/srce/dequant.c \
    ../embdrv/sbc/decoder/srce/framing.c \
    ../embdrv/sbc/decoder/srce/framing-sbc.c \
    ../embdrv/sbc/decoder/srce/oi_codec_version.c \
    ../embdrv/sbc/decoder/srce/synthesis-sbc.c \
    ../embdrv/sbc/decoder/srce/synthesis-dct8.c \
    ../embdrv/sbc/decoder/srce/synthesis-8-generated.c

cpp_sources += \
    ../udrv/ulinux/uipc.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_avk_media_task.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_avk_split_media_task.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_avk.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_avk_split.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_avk_rc.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_avk_media_aac.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_avk_media_mp3.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_avk_media_aptx.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_avk_media_aptx_ad.cc \
    $(external_bt_path)/system_bt_ext/btif/co/bta_avk_co.cc \
    $(external_bt_path)/system_bt_ext/udrv/ulinux/uipc_avk.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_gap.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_gatt_qual.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_l2cap.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_mcap.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_rfcomm.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_smp.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_tws_plus.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_twsp_hf.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_vendor.cc \
    $(external_bt_path)/system_bt_ext/btif/src/btif_vendor_socket.cc

# BTC
if BTC_SUPPORT
AM_CFLAGS += -DBTC_INCLUDED=TRUE
AM_CPPFLAGS += -DBTC_INCLUDED=TRUE -I$(external_bt_path)/system_bt_ext/btc/include
c_sources += \
        $(external_bt_path)/system_bt_ext/btc/src/btc_common.c
endif

#if ($(BOARD_USES_WIPOWER), true)
#c_sources += \
#   ../wipowerif/src/wipower.c
#endif

if USING_HIDL
AM_CFLAGS += -DUSING_HIDL
AM_CPPFLAGS += -DUSING_HIDL
endif

if USING_SOCKET
AM_CFLAGS += -DUSING_SOCKET
AM_CPPFLAGS += -DUSING_SOCKET
endif

if USING_STATIC_LINK
AM_CFLAGS += -DUSING_STATIC_LINK
AM_CPPFLAGS += -DUSING_STATIC_LINK
endif

common_LDFLAGS = -ldl -lrt -lresolv -lz -ldiag -lbtnv @QMIFW_LIBS@ -lqmi_cci \
                 -Wl,--whole-archive,--no-whole-archive \
                 -Wl,--whole-archive,../bta/.libs/libbtbta.a,--no-whole-archive -Wl,--whole-archive,../btif/.libs/libbtif.a,--no-whole-archive \
                 -Wl,--whole-archive,../stack/.libs/libbtstack.a,--no-whole-archive \
                 -Wl,--whole-archive,../utils/.libs/libbtutils.a,--no-whole-archive -Wl,--whole-archive,../btcore/.libs/libbtcore.a,--no-whole-archive \
                 -Wl,--whole-archive,../osi/.libs/libbtosi.a,--no-whole-archive -Wl,--whole-archive,../device/.libs/libbtdevice.a,--no-whole-archive \
                 -Wl,--whole-archive,../types/.libs/libbttypes.a,--no-whole-archive \
                 -Wl,--whole-archive,../hci/.libs/libbthci.a,--no-whole-archive \
                 -laudioutils -lchrome -llog -lcutils -lpthread

common_CFLAGS =  $(AM_CFLAGS) -fPIC -DUSE_AUDIO_TRACK -D_GNU_SOURCE $(Z_LIB) -DUSE_ANDROID_LOGGING
common_CPPFLAGS =  $(AM_CFLAGS) -fPIC -DUSE_AUDIO_TRACK -D_GNU_SOURCE $(Z_LIB) -DUSE_ANDROID_LOGGING
library_includedir = $(pkgincludedir)
lib_LTLIBRARIES = libbluetoothdefault.la
libbluetoothdefault_la_CC = @CC@
libbluetoothdefault_la_SOURCES = $(c_sources) $(cpp_sources)
libbluetoothdefault_la_CPPFLAGS = $(AM_CPPFLAGS) $(common_CPPFLAGS) -std=c++11 -fpermissive -O2
libbluetoothdefault_la_CFLAGS = $(common_CFLAGS)
libbluetoothdefault_la_LDFLAGS = $(common_LDFLAGS)

