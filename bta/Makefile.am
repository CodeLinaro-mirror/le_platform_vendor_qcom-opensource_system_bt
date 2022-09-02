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
endif

if MDM_HASTINGS
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
endif

if MDM_MOSELLE
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
endif

if MDM_HAMILTON
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
endif

AM_CPPFLAGS = -Iinclude \
        -Isys \
        -Idm \
        -Ihh \
        -Iag \
        -I../ \
        -I../btcore/include \
        -I../hci/include \
        -I../include \
        -I../stack/include \
        -I../stack/avdt \
        -I../stack/btm \
        -I../udrv/include \
        -I../vnd/include \
        -I../utils/include \
        -I../types \
        -I../internal_include

AM_CPPFLAGS += -DUSE_ANDROID_LOGGING -DUSE_LIBHW_AOSP

AM_CPPFLAGS += @BLUETOOTHEXT_CFLAGS@ @LIBCHROME_CFLAGS@
if MDM_CHEROKEE
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
endif

if MDM_HASTINGS
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
endif

if MDM_MOSELLE
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
endif

if MDM_HAMILTON
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_DEF_FREQ_48KHZ
endif

cpp_sources = \
        ag/bta_ag_act.cc \
        ag/bta_ag_api.cc \
        ag/bta_ag_at.cc \
        ag/bta_ag_cfg.cc \
        ag/bta_ag_ci.cc \
        ag/bta_ag_cmd.cc \
        ag/bta_ag_main.cc \
        ag/bta_ag_rfc.cc \
        ag/bta_ag_sco.cc \
        ag/bta_ag_sdp.cc \
        ar/bta_ar.cc \
        av/bta_av_aact.cc \
        av/bta_av_act.cc \
        av/bta_av_api.cc \
        av/bta_av_cfg.cc \
        av/bta_av_ci.cc \
        av/bta_av_main.cc \
        av/bta_av_ssm.cc \
        dm/bta_dm_act.cc \
        dm/bta_dm_api.cc \
        dm/bta_dm_cfg.cc \
        dm/bta_dm_ci.cc \
        dm/bta_dm_main.cc \
        dm/bta_dm_pm.cc \
        dm/bta_dm_sco.cc \
        gatt/bta_gattc_act.cc \
        gatt/bta_gattc_api.cc \
        gatt/bta_gattc_cache.cc \
        gatt/bta_gattc_main.cc \
        gatt/bta_gattc_utils.cc \
        gatt/bta_gatts_act.cc \
        gatt/bta_gatts_api.cc \
        gatt/bta_gatts_main.cc \
        gatt/bta_gatts_utils.cc \
        hf_client/bta_hf_client_act.cc \
        hf_client/bta_hf_client_api.cc \
        hf_client/bta_hf_client_at.cc \
        hf_client/bta_hf_client_main.cc \
        hf_client/bta_hf_client_rfc.cc \
        hf_client/bta_hf_client_sco.cc \
        hf_client/bta_hf_client_sdp.cc \
        hh/bta_hh_act.cc \
        hh/bta_hh_api.cc \
        hh/bta_hh_cfg.cc \
        hh/bta_hh_le.cc \
        hh/bta_hh_main.cc \
        hh/bta_hh_utils.cc \
        hl/bta_hl_act.cc \
        hl/bta_hl_api.cc \
        hl/bta_hl_ci.cc \
        hl/bta_hl_main.cc \
        hl/bta_hl_sdp.cc \
        hl/bta_hl_utils.cc \
        hd/bta_hd_act.cc \
        hd/bta_hd_api.cc \
        hd/bta_hd_main.cc \
        jv/bta_jv_act.cc \
        jv/bta_jv_api.cc \
        jv/bta_jv_cfg.cc \
        jv/bta_jv_main.cc \
        mce/bta_mce_act.cc \
        mce/bta_mce_api.cc \
        mce/bta_mce_cfg.cc \
        mce/bta_mce_main.cc \
        pan/bta_pan_act.cc \
        pan/bta_pan_api.cc \
        pan/bta_pan_ci.cc \
        pan/bta_pan_main.cc \
        sdp/bta_sdp.cc \
        sdp/bta_sdp_act.cc \
        sdp/bta_sdp_api.cc \
        sdp/bta_sdp_cfg.cc \
        sys/bta_sys_conn.cc \
        sys/bta_sys_main.cc \
        sys/utl.cc

cpp_sources+= \
        $(external_bt_path)/system_bt_ext/bta/avk/bta_avk_act.cc \
        $(external_bt_path)/system_bt_ext/bta/avk/bta_avk_ci.cc \
        $(external_bt_path)/system_bt_ext/bta/avk/bta_avk_api.cc \
        $(external_bt_path)/system_bt_ext/bta/avk/bta_avk_aact.cc \
        $(external_bt_path)/system_bt_ext/bta/avk/bta_avk_main.cc \
        $(external_bt_path)/system_bt_ext/bta/avk/bta_avk_cfg.cc \
        $(external_bt_path)/system_bt_ext/bta/avk/bta_avk_ssm.cc \
        $(external_bt_path)/system_bt_ext/bta/avk/bta_avk_sbc.cc \
        $(external_bt_path)/system_bt_ext/bta/avk/bta_avk_aac.cc \
        $(external_bt_path)/system_bt_ext/bta/avk/bta_avk_mp3.cc\
        $(external_bt_path)/system_bt_ext/bta/ar/bta_ar_ext.cc \
        $(external_bt_path)/system_bt_ext/bta/tws_plus/bta_tws_plus_api.cc \
        $(external_bt_path)/system_bt_ext/bta/tws_plus/bta_tws_plus_act.cc \
        $(external_bt_path)/system_bt_ext/bta/tws_plus/bta_tws_plus_main.cc \
        $(external_bt_path)/system_bt_ext/bta/tws_plus/ag/bta_ag_twsp_dev.cc \
        $(external_bt_path)/system_bt_ext/bta/tws_plus/ag/bta_ag_twsp_sco.cc

library_includedir = $(pkgincludedir)\
library_include_HEADERS = $(h_sources)
lib_LTLIBRARIES = libbtbta.la
libbtbta_la_CC = @CC@
libbtbta_la_SOURCES = $(cpp_sources)
libbtbta_la_CPPFLAGS =$(AM_CPPFLAGS) -std=c++11 -DHAS_NO_BDROID_BUILDCFG -Wall -fPIC
libbtbta_la_CFLAGS = $(common_CFLAGS)
libbtbta_la_LDFLAGS := $(common_LDFLAGS) -static -lbttypes
