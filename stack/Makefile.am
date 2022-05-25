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
endif

if MDM_HASTINGS
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
endif

if MDM_MOSELLE
AM_CFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
endif

AM_CPPFLAGS = \
        -I.. \
        -I../include \
        -Ia2dp \
        -Iinclude \
        -Iavct \
        -Ibtm \
        -Iavrc \
        -Il2cap \
        -Iavdt \
        -Igatt \
        -Igap \
        -Ipan \
        -Ibnep \
        -Ihid \
        -Isdp \
        -Ismp \
        -Isrvc \
        -I../btcore/include \
        -I../vnd/include \
        -I../vnd/ble \
        -I../btif/include \
        -I../hci/include \
        -I../include \
        -I../udrv/include \
        -I../rpc/include \
        -I../hcis \
        -I../ctrlr/include \
        -I../bta/include \
        -I../bta/sys \
        -I../utils/include \
        -I../types \
        -I../internal_include

AM_CPPFLAGS += -DUSE_ANDROID_LOGGING -DUSE_LIBHW_AOSP

AM_CPPFLAGS += @BLUETOOTHEXT_CFLAGS@ @LIBCHROME_CFLAGS@
if MDM_CHEROKEE
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
endif

if MDM_HASTINGS
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
endif

if MDM_MOSELLE
AM_CPPFLAGS += -DBTA_AV_SPLIT_A2DP_ENABLED
endif


cpp_sources:= \
       ./a2dp/a2dp_aac.cc \
       ./a2dp/a2dp_api.cc \
       ./a2dp/a2dp_codec_config.cc \
       ./a2dp/a2dp_sbc.cc \
       ./a2dp/a2dp_sbc_encoder.cc \
       ./a2dp/a2dp_sbc_up_sample.cc \
       ./a2dp/a2dp_vendor.cc \
       ./a2dp/a2dp_vendor_aptx_adaptive.cc \
       ./a2dp/a2dp_vendor_aptx_adaptive_encoder.cc \
       ./a2dp/a2dp_vendor_aptx.cc \
       ./a2dp/a2dp_vendor_aptx_hd.cc \
       ./a2dp/a2dp_vendor_aptx_encoder.cc \
       ./a2dp/a2dp_vendor_aptx_hd_encoder.cc \
       ./a2dp/a2dp_vendor_ldac.cc \
       ./avct/avct_api.cc \
       ./avct/avct_bcb_act.cc \
       ./avct/avct_ccb.cc \
       ./avct/avct_l2c.cc \
       ./avct/avct_l2c_br.cc \
       ./avct/avct_lcb.cc \
       ./avct/avct_lcb_act.cc \
       ./avdt/avdt_ad.cc \
       ./avdt/avdt_api.cc \
       ./avdt/avdt_ccb.cc \
       ./avdt/avdt_ccb_act.cc \
       ./avdt/avdt_l2c.cc \
       ./avdt/avdt_msg.cc \
       ./avdt/avdt_scb.cc \
       ./avdt/avdt_scb_act.cc \
       ./avrc/avrc_api.cc \
       ./avrc/avrc_bld_ct.cc \
       ./avrc/avrc_bld_tg.cc \
       ./avrc/avrc_opt.cc \
       ./avrc/avrc_pars_ct.cc \
       ./avrc/avrc_pars_tg.cc \
       ./avrc/avrc_sdp.cc \
       ./avrc/avrc_utils.cc \
       ./bnep/bnep_api.cc \
       ./bnep/bnep_main.cc \
       ./bnep/bnep_utils.cc \
       ./btm/ble_advertiser_hci_interface.cc \
       ./btm/btm_acl.cc \
       ./btm/btm_ble.cc \
       ./btm/btm_ble_addr.cc \
       ./btm/btm_ble_adv_filter.cc \
       ./btm/btm_ble_batchscan.cc \
       ./btm/btm_ble_bgconn.cc \
       ./btm/btm_ble_cont_energy.cc \
       ./btm/btm_ble_gap.cc \
       ./btm/btm_ble_multi_adv.cc \
       ./btm/btm_ble_privacy.cc \
       ./btm/btm_dev.cc \
       ./btm/btm_devctl.cc \
       ./btm/btm_inq.cc \
       ./btm/btm_main.cc \
       ./btm/btm_pm.cc \
       ./btm/btm_sco.cc \
       ./btm/btm_sec.cc \
       ./btu/btu_hcif.cc \
       ./btu/btu_init.cc \
       ./btu/btu_task.cc \
       ./gap/gap_ble.cc \
       ./gap/gap_conn.cc\
       ./gatt/att_protocol.cc \
       ./gatt/gatt_api.cc \
       ./gatt/gatt_attr.cc \
       ./gatt/gatt_auth.cc \
       ./gatt/gatt_cl.cc \
       ./gatt/gatt_db.cc \
       ./gatt/gatt_main.cc \
       ./gatt/gatt_sr.cc \
       ./gatt/gatt_utils.cc \
       ./hcic/hciblecmds.cc \
       ./hcic/hcicmds.cc \
       ./hid/hidh_api.cc \
       ./hid/hidh_conn.cc \
       ./hid/hidd_api.cc \
       ./hid/hidd_conn.cc \
       ./l2cap/l2c_api.cc \
       ./l2cap/l2c_ble.cc \
       ./l2cap/l2c_csm.cc \
       ./l2cap/l2c_fcr.cc \
       ./l2cap/l2c_link.cc \
       ./l2cap/l2c_main.cc \
       ./l2cap/l2c_ucd.cc \
       ./l2cap/l2c_utils.cc \
       ./l2cap/l2cap_client.cc \
       ./mcap/mca_api.cc \
       ./mcap/mca_cact.cc \
       ./mcap/mca_csm.cc \
       ./mcap/mca_dact.cc \
       ./mcap/mca_dsm.cc \
       ./mcap/mca_l2c.cc \
       ./mcap/mca_main.cc \
       ./pan/pan_api.cc \
       ./pan/pan_main.cc \
       ./pan/pan_utils.cc \
       ./rfcomm/port_api.cc \
       ./rfcomm/port_rfc.cc \
       ./rfcomm/port_utils.cc \
       ./rfcomm/rfc_l2cap_if.cc \
       ./rfcomm/rfc_mx_fsm.cc \
       ./rfcomm/rfc_port_fsm.cc \
       ./rfcomm/rfc_port_if.cc \
       ./rfcomm/rfc_ts_frames.cc \
       ./rfcomm/rfc_utils.cc \
       ./sdp/sdp_api.cc \
       ./sdp/sdp_db.cc \
       ./sdp/sdp_discovery.cc \
       ./sdp/sdp_main.cc \
       ./sdp/sdp_server.cc \
       ./sdp/sdp_utils.cc \
       ./smp/aes.cc \
       ./smp/p_256_curvepara.cc \
       ./smp/p_256_ecc_pp.cc \
       ./smp/p_256_multprecision.cc \
       ./smp/smp_act.cc \
       ./smp/smp_api.cc \
       ./smp/smp_br_main.cc \
       ./smp/smp_cmac.cc \
       ./smp/smp_keys.cc \
       ./smp/smp_l2c.cc \
       ./smp/smp_main.cc \
       ./smp/smp_utils.cc \
       ./srvc/srvc_battery.cc \
       ./srvc/srvc_dis.cc \
       ./srvc/srvc_eng.cc

cpp_sources+= \
        $(external_bt_path)/system_bt_ext/stack/a2dp/a2d_aac.cc \
        $(external_bt_path)/system_bt_ext/stack/a2dp/a2d_aptx.cc \
        $(external_bt_path)/system_bt_ext/stack/a2dp/a2d_aptx_ad_decoder.cc \
        $(external_bt_path)/system_bt_ext/stack/a2dp/a2d_aptx_hd.cc \
        $(external_bt_path)/system_bt_ext/stack/a2dp/a2d_mp3.cc \
        $(external_bt_path)/system_bt_ext/stack/a2dp/a2d_sbc.cc \
        $(external_bt_path)/system_bt_ext/stack/btm/btm_csb.cc

if SSR_CLEANUP_SUPPORT
AM_CPPFLAGS += -DSSR_CLEANUP=TRUE
endif

common_LDFLAGS = -lpthread -llog -lcutils
common_CFLAGS =  $(AM_CFLAGS) -fPIC -D_GNU_SOURCE

library_includedir = $(pkgincludedir)\
library_include_HEADERS = $(h_sources)
lib_LTLIBRARIES = libbtstack.la
libbtstack_la_CC = @CC@
libbtstack_la_SOURCES = $(cpp_sources)
libbtstack_la_CPPFLAGS = $(AM_CPPFLAGS) -DHAS_NO_BDROID_BUILDCFG -std=c++11 -Wall -fpermissive -fPIC
libbtstack_la_CFLAGS = $(common_CFLAGS)
libbtstack_la_LDFLAGS = $(common_LDFLAGS) -static -lbttypes
