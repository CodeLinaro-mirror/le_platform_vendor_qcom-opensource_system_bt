/******************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#if (BT_ACL_TIMIMG_ENABLED == TRUE)

/*****************************************************************************
 *
 *  Filename:      bt_acl_timestamps.cc
 *
 *  Description:   Bluetooth A2DP timestamps implementation
 *
 *****************************************************************************/
#include <errno.h>
#include <sys/time.h>
#include "bt_types.h"
#include "hcidefs.h"
#include "bt_acl_timestamps.h"
#include "btif/include/btif_common.h"
#include "btif/include/btif_dm.h"
#include "stack/btm/btm_int.h"
#include "stack/avdt/avdt_int.h"
#include "osi/include/log.h"
#include "osi/include/allocator.h"
#include "osi/include/fixed_queue.h"

#define LOG_TAG "bt_acl_timestamps"

/*******************************************************************************
**  Constants & Macros
********************************************************************************/
#define MAX_CONNECTIONS     6
typedef struct {
    bool isA2dpPkt;
    struct timeval timestamp;
} time_stamp_t;

typedef struct {
    uint16_t handle;
    uint16_t length;
    uint16_t pduLen;
    uint16_t lcid;
} __attribute__((packed)) l2cap_hdr_t;

typedef struct {
    uint16_t handle;                  // connection handle
    uint16_t lcid;                    // cid of the AVDTP Media
} cid_info_t;

typedef struct {
    uint16_t handle;                 // connection handle
    bool flush_occured;
    fixed_queue_t *pktQ;           // timestamp queue for ACL packet
} packet_time_info_t;

/*****************************************************************************
**  variables
******************************************************************************/
static cid_info_t cid_info[MAX_CONNECTIONS];
static packet_time_info_t packet_time_info[MAX_CONNECTIONS];

/*****************************************************************************
**  Static functions
******************************************************************************/

/*****************************************************************************
**  Externs
******************************************************************************/
extern uint8_t avdt_ad_tcid_to_type(uint8_t tcid);
/*****************************************************************************
**  Functions
******************************************************************************/

/*******************************************************************************
 **
 ** Function         bt_acl_find_cid_by_handle
 **
 ** Description      find the AVDTP Media cid by handle
 **
 ** Returns          lcid
 **
 *******************************************************************************/
static uint16_t bt_acl_find_cid_by_handle(uint16_t handle)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        /*LOG_ERROR(LOG_TAG,"%s handle %x.", __func__, cid_info[i].handle, cid_info[i].lcid);*/
        if (cid_info[i].handle == handle)
            return cid_info[i].lcid;
    }
    LOG_ERROR(LOG_TAG, "%s cid for handle %d is not found.", __func__, handle);
    return 0;
}

/*******************************************************************************
 **
 ** Function         bt_acl_find_queue_by_handle
 **
 ** Description      find the ACL packet queue by handle
 **
 ** Returns          lcid
 **
 *******************************************************************************/
static fixed_queue_t *bt_acl_find_queue_by_handle(uint16_t handle)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (packet_time_info[i].handle == handle)
            return packet_time_info[i].pktQ;
    }
    LOG_ERROR(LOG_TAG, "%s packet queue is not found.", __func__);
    return NULL;
}

/*******************************************************************************
 **
 ** Function         bt_acl_init_timestamps_info
 **
 ** Description      int the variables for timestamps
 **
 ** Returns          Nothing
 **
 *******************************************************************************/
void bt_acl_init_timestamps_info(void)
{
    LOG_VERBOSE(LOG_TAG, "%s ", __func__);
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        cid_info[i].handle = HCI_INVALID_HANDLE;
        packet_time_info[i].handle = HCI_INVALID_HANDLE;
        packet_time_info[i].flush_occured = FALSE;
        fixed_queue_free(packet_time_info[i].pktQ, osi_free);
        packet_time_info[i].pktQ = NULL;
    }
}

/*******************************************************************************
 **
 ** Function         bt_acl_save_acl_timestamps
 **
 ** Description      save the timestamps for each ACL packet
 **
 ** Returns          void
 **
 *******************************************************************************/
void bt_acl_save_acl_timestamps(BT_HDR *packet)
{
    l2cap_hdr_t *hdr = (l2cap_hdr_t *)(packet->data + packet->offset);
    uint16_t handle = hdr->handle & 0x3f;
    /*LOG_ERROR(LOG_TAG, "%s handle:%d, cid:%x.", __func__, handle, hdr->lcid);*/
    fixed_queue_t *pktQ = bt_acl_find_queue_by_handle(handle);
    if (pktQ != NULL) {
        time_stamp_t* pktItem = (time_stamp_t *)osi_malloc(sizeof(time_stamp_t));
        //gettimeofday(&pktItem->timestamp, NULL);
        pktItem->isA2dpPkt = false;
        uint16_t lcid = bt_acl_find_cid_by_handle(handle);
        if (lcid == hdr->lcid) {
            pktItem->isA2dpPkt = true;
        }

        fixed_queue_enqueue(pktQ, pktItem);
        LOG_VERBOSE(LOG_TAG, "%s handle:%d, queue size:%d isA2dpPkt:%d.", __func__, handle, fixed_queue_length(pktQ), pktItem->isA2dpPkt);
    }
}

/*******************************************************************************
 **
 ** Function         bt_acl_update_lcid
 **
 ** Description      save the lcid info for AVDTP stream
 **
 ** Returns          Nothing
 **
 *******************************************************************************/
void bt_acl_update_lcid(uint16_t handle, uint16_t local_cid, uint16_t remote_cid)
{
    LOG_VERBOSE(LOG_TAG, "%s add local_cid %d remote_cid %d for handle %d.", __func__, local_cid, remote_cid, handle);
    /* look up info for this channel */
    tAVDT_TC_TBL *p_tbl = avdt_ad_tc_tbl_by_lcid(local_cid);
    if (p_tbl == NULL || avdt_ad_tcid_to_type(p_tbl->tcid) != AVDT_CHAN_MEDIA) {
        uint8_t tcid = 0;
        if (p_tbl != NULL) {
            tcid = p_tbl->tcid;
        }
        LOG_WARN(LOG_TAG, "%s :local_cid :%x and remote_cid %x is not for A2DP Meida, tcid: %d", __func__, local_cid, remote_cid, tcid);
        return;
    }

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (cid_info[i].handle == handle) {
            cid_info[i].lcid = remote_cid;
            return;
        }
    }

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (cid_info[i].handle == HCI_INVALID_HANDLE) {
            cid_info[i].handle = handle;
            cid_info[i].lcid = remote_cid;
            return;
        }
    }
    LOG_WARN(LOG_TAG, "%s cid_info is full.", __func__);
}

/*******************************************************************************
 **
 ** Function         bt_acl_init_timestamps_by_handle
 **
 ** Description      init the timestamps info by handle.
 **
 ** Returns          None
 **
 *******************************************************************************/
void bt_acl_init_timestamps_by_handle(uint16_t handle)
{
    bt_acl_remove_timestamps_by_handle(handle);
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (packet_time_info[i].handle == HCI_INVALID_HANDLE) {
            packet_time_info[i].handle = handle;
            packet_time_info[i].pktQ = fixed_queue_new(SIZE_MAX);
            return;
        }
    }
    LOG_WARN(LOG_TAG, "%s packet_time_info is full.", __func__);
}

/*******************************************************************************
 **
 ** Function         bt_acl_remove_timestamps_by_handle
 **
 ** Description      remove the timestamps info by handle.
 **
 ** Returns          None
 **
 *******************************************************************************/
void bt_acl_remove_timestamps_by_handle(uint16_t handle)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (cid_info[i].handle == handle) {
            cid_info[i].handle = HCI_INVALID_HANDLE;
            LOG_VERBOSE(LOG_TAG, "%s free lcid_info, handle %d", __func__, handle);
            break;
        }
    }

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (packet_time_info[i].handle == handle) {
            packet_time_info[i].handle = HCI_INVALID_HANDLE;
            LOG_VERBOSE(LOG_TAG, "%s handle %d. queue size: %d", __func__, handle, fixed_queue_length(packet_time_info[i].pktQ));
            fixed_queue_free(packet_time_info[i].pktQ, osi_free);
            packet_time_info[i].pktQ = NULL;
            return;
        }
    }
}

/*******************************************************************************
 **
 ** Function         bt_acl_set_flush_occur_status
 **
 ** Description      set the f/w flush occur status.
 **
 ** Returns          None
 **
 *******************************************************************************/
void bt_acl_set_flush_occur_status(uint16_t handle, bool status)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (packet_time_info[i].handle == handle) {
            packet_time_info[i].flush_occured = status;
            return;
        }
    }
    LOG_WARN(LOG_TAG, "%s handle is not found.", __func__);
}

/*******************************************************************************
 **
 ** Function         bt_acl_get_flush_occur_status
 **
 ** Description      get the flush occur status
 **
 ** Returns          status
 **
 *******************************************************************************/

static bool bt_acl_get_flush_occur_status(uint16_t handle)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (packet_time_info[i].handle == handle) {
            return packet_time_info[i].flush_occured;
        }
    }
    LOG_WARN(LOG_TAG, "%s handle is not found.", __func__);
    return FALSE;
}

/*******************************************************************************
 **
 ** Function         bt_acl_calc_timestamps_by_handle
 **
 ** Description      calculate the timestamps info by handle.
 **
 ** Returns          None
 **
 *******************************************************************************/
void bt_acl_calc_timestamps_by_handle(uint16_t handle, uint8_t num)
{
    int time_delta = 0;
    fixed_queue_t *pktQ = bt_acl_find_queue_by_handle(handle);
    if (pktQ == NULL) {
        LOG_ERROR(LOG_TAG, "%s pktQ is not found, handle :%d.", __func__, handle);
        return;
    }

    bool flush_occured = bt_acl_get_flush_occur_status(handle);
    tBTM_SEC_DEV_REC *p_dev_rec = btm_find_dev_by_handle (handle);
    if (p_dev_rec == NULL) {
        LOG_ERROR(LOG_TAG, "%s device is not found, handle :%d.", __func__, handle);
        return;
    }
    struct timeval curTime;
    RawAddress bd_addr;

    bd_addr = p_dev_rec->bd_addr;

    gettimeofday(&curTime, NULL);
    if (!flush_occured) {
        for (int i = 0; i < num; i++) {
            time_stamp_t* pktItem = (time_stamp_t *)fixed_queue_dequeue(pktQ);
            time_delta = (curTime.tv_sec - pktItem->timestamp.tv_sec) * 1000000 + (curTime.tv_usec - pktItem->timestamp.tv_usec);
            LOG_ERROR(LOG_TAG, "%s handle:%d, A2DP: %d, Tx time_delta:%d.", __func__, handle, pktItem->isA2dpPkt, time_delta);
            if (pktItem->isA2dpPkt)
                HAL_CBACK(bt_vendor_callbacks, a2dp_tx_complete_cb, &bd_addr, FALSE);
            osi_free(pktItem);
        }
    }
    else {
        // firmware flush occur
        bt_acl_set_flush_occur_status(handle, FALSE);
        for (int i = 0; i < num; i++) {
            time_stamp_t* pktItem = (time_stamp_t *)fixed_queue_dequeue(pktQ);
            if (pktItem->isA2dpPkt) {
                LOG_WARN(LOG_TAG, "%s firmware flush occur :", __func__);
                HAL_CBACK(bt_vendor_callbacks, a2dp_tx_complete_cb, &bd_addr, TRUE);
            }
            osi_free(pktItem);
        }
    }
}
#endif