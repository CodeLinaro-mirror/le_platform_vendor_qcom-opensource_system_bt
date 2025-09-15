/******************************************************************************
 *
 *  Copyright (C) 2008-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  Changes from Qualcomm Technologies, Inc. are provided under the following license:
 *  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *  SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  this file contains the main GATT server attributes access request
 *  handling functions.
 *
 ******************************************************************************/

#include "bt_target.h"
#include "bt_utils.h"

#include "gatt_api.h"
#include "gatt_int.h"
#include "osi/include/log.h"
#include "osi/include/osi.h"
#include "osi/include/properties.h"

#include "btif_storage.h"
#include "bta_gatt_api.h"
#include "stack/btm/btm_int.h"
#include "stack_config.h"

using base::StringPrintf;
using bluetooth::Uuid;

#define BLE_GATT_CL_SUP_FEAT_CACHING_BITMASK 0x01

#define GATTP_MAX_NUM_INC_SVR 0
#define GATTP_MAX_CHAR_NUM 2
#define GATTP_MAX_ATTR_NUM (GATTP_MAX_CHAR_NUM * 2 + GATTP_MAX_NUM_INC_SVR + 1)
#define GATTP_MAX_CHAR_VALUE_SIZE 50

#ifndef GATTP_ATTR_DB_SIZE
#define GATTP_ATTR_DB_SIZE                                    \
  GATT_DB_MEM_SIZE(GATTP_MAX_NUM_INC_SVR, GATTP_MAX_CHAR_NUM, \
                   GATTP_MAX_CHAR_VALUE_SIZE)
#endif

typedef struct {
  uint16_t handle;
  uint16_t uuid;
  uint16_t value_len;
  uint8_t value[GATT_MAX_ATTR_LEN];
} tGATT_ATTR_INFO;

constexpr int GATT_MAX_CHAR_NUM = 4;
std::array<tGATT_ATTR_INFO, GATT_MAX_CHAR_NUM> gatt_attr_db;

static void gatt_request_cback(uint16_t conn_id, uint32_t trans_id,
                               uint8_t op_code, tGATTS_DATA* p_data);
static void gatt_connect_cback(UNUSED_ATTR tGATT_IF gatt_if,
                               const RawAddress& bda, uint16_t conn_id,
                               bool connected, tGATT_DISCONN_REASON reason,
                               tBT_TRANSPORT transport);
static void gatt_disc_res_cback(uint16_t conn_id, tGATT_DISC_TYPE disc_type,
                                tGATT_DISC_RES* p_data);
static void gatt_disc_cmpl_cback(uint16_t conn_id, tGATT_DISC_TYPE disc_type,
                                 tGATT_STATUS status);
static void gatt_cl_op_cmpl_cback(uint16_t conn_id,
                                  tGATTC_OPTYPE op,
                                  tGATT_STATUS status,
                                  tGATT_CL_COMPLETE* p_data,
                                  uint32_t trans_id);

static void gatt_cl_start_config_ccc(tGATT_PROFILE_CLCB* p_clcb);

static void gatt_cl_write_robust_caching_support(tGATT_PROFILE_CLCB* p_clcb);

static bool gatt_sr_is_robust_caching_enabled();

static tGATT_STATUS gatt_sr_read_db_hash(uint16_t conn_id,
                                         tGATT_VALUE* p_value);
static tGATT_STATUS gatt_sr_read_cl_supp_feat(uint16_t conn_id,
                                              tGATT_VALUE* p_value);
static tGATT_STATUS gatt_sr_write_cl_supp_feat(uint16_t conn_id,
                                               tGATT_WRITE_REQ* p_data);

static tGATT_STATUS gatt_sr_write_cccd(uint16_t conn_id,
                                       tGATT_WRITE_REQ* p_data);

static tGATT_CBACK gatt_profile_cback = {gatt_connect_cback,
                                         gatt_cl_op_cmpl_cback,
                                         gatt_disc_res_cback,
                                         gatt_disc_cmpl_cback,
                                         gatt_request_cback,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL};

/*******************************************************************************
 *
 * Function         gatt_profile_find_conn_id_by_bd_addr
 *
 * Description      Find the connection ID by remote address
 *
 * Returns          Connection ID
 *
 ******************************************************************************/
uint16_t gatt_profile_find_conn_id_by_bd_addr(const RawAddress& remote_bda) {
  uint16_t conn_id = GATT_INVALID_CONN_ID;
  GATT_GetConnIdIfConnected(gatt_cb.gatt_if, remote_bda, &conn_id,
                            BT_TRANSPORT_LE);
  if (conn_id == GATT_INVALID_CONN_ID)
    GATT_GetConnIdIfConnected(gatt_cb.gatt_if, remote_bda, &conn_id,
                              BT_TRANSPORT_BR_EDR);
  return conn_id;
}

/*******************************************************************************
 *
 * Function         gatt_profile_find_clcb_by_conn_id
 *
 * Description      find clcb by Connection ID
 *
 * Returns          Pointer to the found link conenction control block.
 *
 ******************************************************************************/
static tGATT_PROFILE_CLCB* gatt_profile_find_clcb_by_conn_id(uint16_t conn_id) {
  uint8_t i_clcb;
  tGATT_PROFILE_CLCB* p_clcb = NULL;

  for (i_clcb = 0, p_clcb = gatt_cb.profile_clcb; i_clcb < GATT_MAX_APPS;
       i_clcb++, p_clcb++) {
    if (p_clcb->in_use && p_clcb->conn_id == conn_id) return p_clcb;
  }

  return NULL;
}

/*******************************************************************************
 *
 * Function         gatt_profile_find_clcb_by_bd_addr
 *
 * Description      The function searches all LCBs with macthing bd address.
 *
 * Returns          Pointer to the found link conenction control block.
 *
 ******************************************************************************/
static tGATT_PROFILE_CLCB* gatt_profile_find_clcb_by_bd_addr(
    const RawAddress& bda, tBT_TRANSPORT transport) {
  uint8_t i_clcb;
  tGATT_PROFILE_CLCB* p_clcb = NULL;

  for (i_clcb = 0, p_clcb = gatt_cb.profile_clcb; i_clcb < GATT_MAX_APPS;
       i_clcb++, p_clcb++) {
    if (p_clcb->in_use && p_clcb->transport == transport && p_clcb->connected &&
        p_clcb->bda == bda)
      return p_clcb;
  }

  return NULL;
}

/*******************************************************************************
 *
 * Function         gatt_profile_clcb_alloc
 *
 * Description      The function allocates a GATT profile connection link
 *                  control block
 *
 * Returns          NULL if not found. Otherwise pointer to the connection link
 *                  block.
 *
 ******************************************************************************/
tGATT_PROFILE_CLCB* gatt_profile_clcb_alloc(uint16_t conn_id,
                                            const RawAddress& bda,
                                            tBT_TRANSPORT tranport) {
  uint8_t i_clcb = 0;
  tGATT_PROFILE_CLCB* p_clcb = NULL;

  for (i_clcb = 0, p_clcb = gatt_cb.profile_clcb; i_clcb < GATT_MAX_APPS;
       i_clcb++, p_clcb++) {
    if (!p_clcb->in_use) {
      p_clcb->in_use = true;
      p_clcb->conn_id = conn_id;
      p_clcb->connected = true;
      p_clcb->transport = tranport;
      p_clcb->bda = bda;
      break;
    }
  }
  if (i_clcb < GATT_MAX_APPS) return p_clcb;

  return NULL;
}

/*******************************************************************************
 *
 * Function         gatt_profile_clcb_dealloc
 *
 * Description      The function deallocates a GATT profile connection link
 *                  control block
 *
 * Returns          void
 *
 ******************************************************************************/
void gatt_profile_clcb_dealloc(tGATT_PROFILE_CLCB* p_clcb) {
  memset(p_clcb, 0, sizeof(tGATT_PROFILE_CLCB));
}

/*******************************************************************************
 *
 * Function         read_attr_value
 *
 * Description      The function processes GATT Attributes database read request
 *                  callback
 *
 * Returns          status of read
 *
 ******************************************************************************/
tGATT_STATUS read_attr_value(uint16_t conn_id, uint16_t handle,
                             tGATT_VALUE* p_value, bool is_long) {
  uint8_t* p = p_value->value;
  uint8_t i = 0;
  tGATT_PROFILE_CLCB* p_clcb = NULL;
  VLOG(1) << __func__ << " conn_id:" <<+conn_id;

  for (tGATT_ATTR_INFO& db_attr : gatt_attr_db) {
    if (handle == db_attr.handle) {
      if (is_long)
        return GATT_NOT_LONG;

      switch (db_attr.uuid) {
        case GATT_UUID_GATT_SR_SUPP_FEATURES: {
          p_clcb = gatt_profile_find_clcb_by_conn_id(conn_id);
          if (!p_clcb) {
            VLOG(1) << __func__ << " Error during read Server supported features char";
            return GATT_ERROR;
          }
          if (p_clcb->transport == BT_TRANSPORT_LE) {
            for(i=0; i<db_attr.value_len; i++) {
              UINT8_TO_STREAM(p, db_attr.value[i]);
            }
            p_value->len = db_attr.value_len;
          }
          break;
        }

        case GATT_UUID_GATT_CL_SUPP_FEATURES: {
          p_clcb = gatt_profile_find_clcb_by_conn_id(conn_id);
          if (!p_clcb) {
            VLOG(1) << __func__ << " Error during read Client supported features char";
            return GATT_ERROR;
          }

          //Read Client supported features characteristic from btif storage
          uint8_t cl_supp_feat = btif_storage_get_cl_supp_feat(p_clcb->bda);
          UINT8_TO_STREAM(p, cl_supp_feat);
          p_value->len = 1;

          break;
        }

        case GATT_UUID_DATABASE_HASH: {
          /* GATT_UUID_DATABASE_HASH */
          if (is_long) return GATT_NOT_LONG;

          return gatt_sr_read_db_hash(conn_id, p_value);
        }
      }
      return GATT_SUCCESS;
    }
  }
  return GATT_READ_NOT_PERMIT;
}

/*******************************************************************************
 *
 * Function         proc_read
 *
 * Description      The function processes GATT Read request received
 *
 *
 * Returns          status of read
 *
 ******************************************************************************/
tGATT_STATUS proc_read(uint16_t conn_id, tGATT_READ_REQ* p_data,
                       tGATTS_RSP* p_rsp) {
  if (p_data->is_long) p_rsp->attr_value.offset = p_data->offset;

  p_rsp->attr_value.handle = p_data->handle;

  return read_attr_value(conn_id, p_data->handle, &p_rsp->attr_value, p_data->is_long);
}

/*******************************************************************************
 *
 * Function         proc_write_req
 *
 * Description      The function processes GATT Attributes database write request
 *                  callback
 *
 * Returns          status of write
 *
 ******************************************************************************/
tGATT_STATUS proc_write_req(uint16_t conn_id, tGATT_WRITE_REQ* p_data) {
  tGATT_PROFILE_CLCB* p_clcb = NULL;
  uint8_t cl_supp_feat;
  VLOG(1) << __func__ << " conn_id:" << +conn_id;

  for (tGATT_ATTR_INFO& db_attr : gatt_attr_db) {
    if (p_data->handle == db_attr.handle) {
      /* GATT_UUID_GATT_CL_SUPP_FEATURES */
      if (db_attr.uuid == GATT_UUID_GATT_CL_SUPP_FEATURES) {
        return gatt_sr_write_cl_supp_feat(conn_id, p_data);
      }
      /* GATT_UUID_DATABASE_HASH */
      if (db_attr.uuid == GATT_UUID_DATABASE_HASH) {
        return GATT_WRITE_NOT_PERMIT;
      }
      if (stack_config_get_interface()->get_pts_configure_svc_chg_indication()) {
        /* GATT_UUID_CHAR_CLIENT_CONFIG */
        if (db_attr.uuid == GATT_UUID_CHAR_CLIENT_CONFIG) {
          return gatt_sr_write_cccd(conn_id, p_data);
        }
      }
    }
  }
  return GATT_WRITE_NOT_PERMIT;
}

/*******************************************************************************
 *
 * Function         gatt_request_cback
 *
 * Description      GATT profile attribute access request callback.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void gatt_request_cback(uint16_t conn_id, uint32_t trans_id,
                               tGATTS_REQ_TYPE type, tGATTS_DATA* p_data) {
  uint8_t status = GATT_INVALID_PDU;
  tGATTS_RSP rsp_msg;
  bool ignore = false;
  uint8_t tcb_idx = GATT_GET_TCB_IDX(conn_id);
  tGATT_TCB* p_tcb = gatt_get_tcb_by_idx(tcb_idx);
  tGATT_PROFILE_CLCB* p_clcb = NULL;

  memset(&rsp_msg, 0, sizeof(tGATTS_RSP));

  if (p_tcb) {
    p_clcb = gatt_profile_find_clcb_by_bd_addr(p_tcb->peer_bda, p_tcb->transport);
    if (p_clcb == NULL)
      p_clcb = gatt_profile_clcb_alloc(0, p_tcb->peer_bda, p_tcb->transport);

    if (p_clcb == NULL) return;

    if (GATT_GetConnIdIfConnected(gatt_cb.gatt_if, p_tcb->peer_bda, &p_clcb->conn_id,
                                  p_tcb->transport)) {
      p_clcb->connected = true;
    }
  }

  switch (type) {
    case GATTS_REQ_TYPE_READ_CHARACTERISTIC:
      status = proc_read(conn_id, &p_data->read_req, &rsp_msg);
      break;
    case GATTS_REQ_TYPE_READ_DESCRIPTOR:
      status = GATT_READ_NOT_PERMIT;
      break;

    case GATTS_REQ_TYPE_WRITE_CHARACTERISTIC:
    case GATTS_REQ_TYPE_WRITE_DESCRIPTOR:
      if (!p_data->write_req.need_rsp) ignore = true;
      status = proc_write_req(conn_id, &p_data->write_req);
      break;

    case GATTS_REQ_TYPE_WRITE_EXEC:
    case GATT_CMD_WRITE:
      ignore = true;
      VLOG(1) << StringPrintf("Ignore GATT_REQ_EXEC_WRITE/WRITE_CMD");
      break;

    case GATTS_REQ_TYPE_MTU:
      VLOG(1) << StringPrintf("Get MTU exchange new mtu size: %d", p_data->mtu);
      ignore = true;
      break;

    default:
      VLOG(1) << StringPrintf("Unknown/unexpected LE GAP ATT request: 0x%02x",
                              type);
      break;
  }

  if (!ignore) GATTS_SendRsp(conn_id, trans_id, status, &rsp_msg);
}

/*******************************************************************************
 *
 * Function         gatt_connect_cback
 *
 * Description      Gatt profile connection callback.
 *
 * Returns          void
 *
 ******************************************************************************/
static void gatt_connect_cback(UNUSED_ATTR tGATT_IF gatt_if,
                               const RawAddress& bda, uint16_t conn_id,
                               bool connected, tGATT_DISCONN_REASON reason,
                               tBT_TRANSPORT transport) {
  VLOG(1) << __func__ << ": from " << bda
          << StringPrintf(" connected:%d conn_id=%d reason = 0x%04x", connected,
                          conn_id, reason);

  // if the device is not trusted, remove data when the link is disconnected
  if (!connected && !btm_sec_is_a_bonded_dev(bda)) {
    LOG(INFO) << __func__ << ": remove untrusted client status, bda=" << bda;
    btif_storage_remove_gatt_cl_supp_feat(bda);
    btif_storage_remove_gatt_cl_db_hash(bda);
    if (stack_config_get_interface()->get_pts_configure_svc_chg_indication()) {
      btif_storage_remove_svc_chg_cccd(bda);
    }
  }

  tGATT_PROFILE_CLCB* p_clcb =
      gatt_profile_find_clcb_by_bd_addr(bda, transport);
  if (p_clcb == NULL) return;

  if (connected) {
    p_clcb->conn_id = conn_id;
    p_clcb->connected = true;

    if (p_clcb->ccc_stage == GATT_SVC_CHANGED_CONNECTING) {
      p_clcb->ccc_stage++;
      gatt_cl_start_config_ccc(p_clcb);
    }
  } else {
    gatt_profile_clcb_dealloc(p_clcb);
  }
}

/*******************************************************************************
 *
 * Function         gatt_profile_db_init
 *
 * Description      Initializa the GATT profile attribute database.
 *
 ******************************************************************************/
void gatt_profile_db_init(void) {
  uint16_t service_handle = 0;
  uint8_t base_index = 0;
  btgatt_db_element_t service[6];
  int size_of_svc = 0;

  /* Fill our internal UUID with a fixed pattern 0x81 */
  std::array<uint8_t, Uuid::kNumBytes128> tmp;
  tmp.fill(0x81);

  /* Create a GATT profile service */
  gatt_cb.gatt_if = GATT_Register(Uuid::From128BitBE(tmp), &gatt_profile_cback);
  GATT_StartIf(gatt_cb.gatt_if);

  Uuid service_uuid = Uuid::From16Bit(UUID_SERVCLASS_GATT_SERVER);
  Uuid char_uuid = Uuid::From16Bit(GATT_UUID_GATT_SRV_CHGD);

  Uuid sr_supp_feat_char_uuid = Uuid::From16Bit(GATT_UUID_GATT_SR_SUPP_FEATURES);
  Uuid cl_supp_feat_char_uuid = Uuid::From16Bit(GATT_UUID_GATT_CL_SUPP_FEATURES);
  Uuid database_hash_uuid = Uuid::From16Bit(GATT_UUID_DATABASE_HASH);
  Uuid cccd_uuid = Uuid::From16Bit(GATT_UUID_CHAR_CLIENT_CONFIG);

  if (stack_config_get_interface()->get_pts_configure_svc_chg_indication()) {
    btgatt_db_element_t svc[] = {
        {.uuid = service_uuid, .type = BTGATT_DB_PRIMARY_SERVICE},
        {.uuid = char_uuid,
         .type = BTGATT_DB_CHARACTERISTIC,
         .properties = GATT_CHAR_PROP_BIT_INDICATE,
         .permissions = 0},
        {.uuid = cccd_uuid,
         .type = BTGATT_DB_DESCRIPTOR,
         .permissions = (GATT_PERM_READ | GATT_PERM_WRITE)},
        {.uuid = sr_supp_feat_char_uuid,
         .type = BTGATT_DB_CHARACTERISTIC,
         .properties = GATT_CHAR_PROP_BIT_READ,
         .permissions = GATT_PERM_READ},
        {.uuid = cl_supp_feat_char_uuid,
         .type = BTGATT_DB_CHARACTERISTIC,
         .properties = (GATT_CHAR_PROP_BIT_READ | GATT_CHAR_PROP_BIT_WRITE),
         .permissions = (GATT_PERM_READ | GATT_PERM_WRITE)},
        {.uuid = database_hash_uuid,
         .type = BTGATT_DB_CHARACTERISTIC,
         .properties = GATT_CHAR_PROP_BIT_READ,
         .permissions = GATT_PERM_READ,
        }};

    std::copy(std::begin(svc), std::end(svc), std::begin(service));
    size_of_svc = 6;
    base_index = 1;
  }
  else {
    btgatt_db_element_t svc[] = {
        {.uuid = service_uuid, .type = BTGATT_DB_PRIMARY_SERVICE},
        {.uuid = char_uuid,
         .type = BTGATT_DB_CHARACTERISTIC,
         .properties = GATT_CHAR_PROP_BIT_INDICATE,
         .permissions = 0},
        {.uuid = sr_supp_feat_char_uuid,
         .type = BTGATT_DB_CHARACTERISTIC,
         .properties = GATT_CHAR_PROP_BIT_READ,
         .permissions = GATT_PERM_READ},
        {.uuid = cl_supp_feat_char_uuid,
         .type = BTGATT_DB_CHARACTERISTIC,
         .properties = (GATT_CHAR_PROP_BIT_READ | GATT_CHAR_PROP_BIT_WRITE),
         .permissions = (GATT_PERM_READ | GATT_PERM_WRITE)},
        {.uuid = database_hash_uuid,
         .type = BTGATT_DB_CHARACTERISTIC,
         .properties = GATT_CHAR_PROP_BIT_READ,
         .permissions = GATT_PERM_READ,
        }};

    std::copy(std::begin(svc), std::end(svc), std::begin(service));
    size_of_svc = 5;
  }

  GATTS_AddService(gatt_cb.gatt_if, service, size_of_svc);

  service_handle = service[0].attribute_handle;
  gatt_cb.handle_of_h_r = service[1].attribute_handle;

  gatt_attr_db[0].uuid = GATT_UUID_GATT_SR_SUPP_FEATURES;
  gatt_attr_db[0].handle = service[base_index + 2].attribute_handle;
  gatt_attr_db[0].value[0] = 0;
  gatt_attr_db[0].value_len = 1;

  /* Client supported features characteristic */
  gatt_attr_db[1].uuid = GATT_UUID_GATT_CL_SUPP_FEATURES;
  gatt_attr_db[1].handle = service[base_index + 3].attribute_handle;

  /* Database Hash characteristic */
  gatt_attr_db[2].uuid = GATT_UUID_DATABASE_HASH;
  gatt_attr_db[2].handle = service[base_index + 4].attribute_handle;

  if (stack_config_get_interface()->get_pts_configure_svc_chg_indication()) {
    gatt_attr_db[3].uuid = GATT_UUID_CHAR_CLIENT_CONFIG;
    gatt_attr_db[3].handle = service[2].attribute_handle;
  }

  LOG(ERROR) << StringPrintf("gatt_profile_db_init:  gatt_if=%d",
                             gatt_cb.gatt_if);
  if (gatt_sr_is_robust_caching_enabled())
    gatt_cb.gatt_cl_supported_feat_mask |= BLE_GATT_CL_SUP_FEAT_CACHING_BITMASK;
}

/*******************************************************************************
 *
 * Function         gatt_disc_res_cback
 *
 * Description      Gatt profile discovery result callback
 *
 * Returns          void
 *
 ******************************************************************************/
static void gatt_disc_res_cback(uint16_t conn_id, tGATT_DISC_TYPE disc_type,
                                tGATT_DISC_RES* p_data) {
  tGATT_PROFILE_CLCB* p_clcb = gatt_profile_find_clcb_by_conn_id(conn_id);

  if (p_clcb == NULL) return;

  switch (disc_type) {
    case GATT_DISC_SRVC_BY_UUID: /* stage 1 */
      p_clcb->e_handle = p_data->value.group_value.e_handle;
      p_clcb->ccc_result++;

      p_clcb->sr_supp_feat_e_handle = p_data->value.group_value.e_handle;
      p_clcb->sr_supp_feat_result++;
      break;

    case GATT_DISC_CHAR: /* stage 2 */
      p_clcb->s_handle = p_data->value.dclr_value.val_handle;
      p_clcb->ccc_result++;
      break;

    case GATT_DISC_CHAR_DSCPT: /* stage 3 */
      if (p_data->type == Uuid::From16Bit(GATT_UUID_CHAR_CLIENT_CONFIG)) {
        p_clcb->s_handle = p_data->handle;
        p_clcb->ccc_result++;
      }
      break;
  }
}

/*******************************************************************************
 *
 * Function         gatt_disc_cmpl_cback
 *
 * Description      Gatt profile discovery complete callback
 *
 * Returns          void
 *
 ******************************************************************************/
static void gatt_disc_cmpl_cback(uint16_t conn_id, tGATT_DISC_TYPE disc_type,
                                 tGATT_STATUS status) {
  tGATT_PROFILE_CLCB* p_clcb = gatt_profile_find_clcb_by_conn_id(conn_id);

  if (p_clcb == NULL) return;

  VLOG(1) << __func__ << " ccc_stage::" << +p_clcb->ccc_stage;

  if (status != GATT_SUCCESS || p_clcb->ccc_result == 0) {
    LOG(ERROR) << StringPrintf(
        "%s() - Unable to register for service changed indication", __func__);
    return;
  }

  if (p_clcb->ccc_stage > GATT_SVC_CHANGED_CONNECTING) {
    p_clcb->ccc_result = 0;
    p_clcb->ccc_stage++;
    gatt_cl_start_config_ccc(p_clcb);
  }
}

/*******************************************************************************
 *
 * Function         gatt_cl_op_cmpl_cback
 *
 * Description      Gatt profile client operation complete callback
 *
 * Returns          void
 *
 ******************************************************************************/
static void gatt_cl_op_cmpl_cback(uint16_t conn_id,
                                  tGATTC_OPTYPE op,
                                  tGATT_STATUS status,
                                  tGATT_CL_COMPLETE* p_data,
                                  uint32_t trans_id) {
  uint8_t value[GATT_MAX_ATTR_LEN];

  VLOG(1) << __func__ << " op:" << +op;

  tGATT_PROFILE_CLCB* p_clcb = gatt_profile_find_clcb_by_conn_id(conn_id);
  if (p_clcb == NULL) {
    VLOG(1) << __func__ << "p_clcb is NULL";
    return;
  }

  if (status == GATT_DATABASE_OUT_OF_SYNC) {
    // trigger discovery
    LOG(INFO) << __func__ << ": GATT_DATABASE_OUT_OF_SYNC";
    BTA_GATTC_Refresh(p_clcb->bda);
    return;
  }
  if ((op == GATTC_OPTYPE_WRITE) &&
      (p_clcb->ccc_stage == GATT_SVC_CHANGED_CONFIGURE_CCCD)) {
    VLOG(1) << __func__ << "Configure CCC is done";
    return;
  }

  if ((op == GATTC_OPTYPE_READ) && (status == GATT_SUCCESS)) {
    if (p_clcb->robust_caching_stage == GATT_ROBUST_CACHING_CL_SUPP_FEAT_READ) {
      p_clcb->robust_caching_handle = p_data->att_value.handle;
      VLOG(1) << __func__ << " GATT_ROBUST_CACHING_CL_SUPP_FEAT_READ stage completed:";
      p_clcb->robust_caching_stage++;
      gatt_cl_write_robust_caching_support(p_clcb);
    }
  }
}

/*******************************************************************************
 *
 * Function         gatt_cl_start_config_ccc
 *
 * Description      Gatt profile start configure service change CCC
 *
 * Returns          void
 *
 ******************************************************************************/
static void gatt_cl_start_config_ccc(tGATT_PROFILE_CLCB* p_clcb) {
  tGATT_DISC_PARAM srvc_disc_param;
  tGATT_VALUE ccc_value;

  VLOG(1) << StringPrintf("%s() - stage: %d", __func__, p_clcb->ccc_stage);

  memset(&srvc_disc_param, 0, sizeof(tGATT_DISC_PARAM));
  memset(&ccc_value, 0, sizeof(tGATT_VALUE));

  switch (p_clcb->ccc_stage) {
    case GATT_SVC_CHANGED_SERVICE: /* discover GATT service */
      srvc_disc_param.s_handle = 1;
      srvc_disc_param.e_handle = 0xffff;
      srvc_disc_param.service = Uuid::From16Bit(UUID_SERVCLASS_GATT_SERVER);
      GATTC_Discover(p_clcb->conn_id, GATT_DISC_SRVC_BY_UUID, &srvc_disc_param);
      break;

    case GATT_SVC_CHANGED_CHARACTERISTIC: /* discover service change char */
      srvc_disc_param.s_handle = 1;
      srvc_disc_param.e_handle = p_clcb->e_handle;
      srvc_disc_param.service = Uuid::From16Bit(GATT_UUID_GATT_SRV_CHGD);
      GATTC_Discover(p_clcb->conn_id, GATT_DISC_CHAR, &srvc_disc_param);
      break;

    case GATT_SVC_CHANGED_DESCRIPTOR: /* discover service change ccc */
      srvc_disc_param.s_handle = p_clcb->s_handle;
      srvc_disc_param.e_handle = p_clcb->e_handle;
      GATTC_Discover(p_clcb->conn_id, GATT_DISC_CHAR_DSCPT, &srvc_disc_param);
      break;

    case GATT_SVC_CHANGED_CONFIGURE_CCCD: /* write ccc */
      ccc_value.handle = p_clcb->s_handle;
      ccc_value.len = 2;
      ccc_value.value[0] = GATT_CLT_CONFIG_INDICATION;
      GATTC_Write(p_clcb->conn_id, GATT_WRITE, &ccc_value);
      break;
  }
}

/*******************************************************************************
 *
 * Function         GATT_ConfigServiceChangeCCC
 *
 * Description      Configure service change indication on remote device
 *
 * Returns          none
 *
 ******************************************************************************/
void GATT_ConfigServiceChangeCCC(const RawAddress& remote_bda, bool enable,
                                 tBT_TRANSPORT transport) {
  tGATT_PROFILE_CLCB* p_clcb =
      gatt_profile_find_clcb_by_bd_addr(remote_bda, transport);

  if (p_clcb == NULL)
    p_clcb = gatt_profile_clcb_alloc(0, remote_bda, transport);

  if (p_clcb == NULL) return;

  if (GATT_GetConnIdIfConnected(gatt_cb.gatt_if, remote_bda, &p_clcb->conn_id,
                                transport)) {
    p_clcb->connected = true;
  }
  /* hold the link here */
  GATT_Connect(gatt_cb.gatt_if, remote_bda, true, transport, true);
  p_clcb->ccc_stage = GATT_SVC_CHANGED_CONNECTING;

  if (!p_clcb->connected) {
    /* wait for connection */
    return;
  }

  p_clcb->ccc_stage++;
  gatt_cl_start_config_ccc(p_clcb);
}

/*******************************************************************************
 *
 * Function         gatt_cl_write_robust_caching_support
 *
 * Description      Gatt profile check EATT support
 *
 * Returns          void
 *
 ******************************************************************************/
static void gatt_cl_write_robust_caching_support(tGATT_PROFILE_CLCB* p_clcb) {

  VLOG(1) << __func__ << ": stage: " << +p_clcb->robust_caching_stage;

  switch (p_clcb->robust_caching_stage) {
    /* read Client supported features char */
    case GATT_ROBUST_CACHING_CL_SUPP_FEAT_READ:
    {
      tGATT_READ_PARAM read_param;
      memset(&read_param, 0, sizeof(tGATT_READ_BY_TYPE));

      read_param.char_type.s_handle = 0x0001;
      read_param.char_type.e_handle = 0xFFFF;
      read_param.char_type.uuid = Uuid::From16Bit(GATT_UUID_GATT_CL_SUPP_FEATURES);

      GATTC_Read(p_clcb->conn_id, GATT_READ_BY_TYPE, &read_param);
      break;
    }
    /* write Client supported features char */
    case GATT_ROBUST_CACHING_CL_SUPP_FEAT_WRITE:
    {
      tGATT_VALUE write_attr_value;
      memset(&write_attr_value, 0, sizeof(tGATT_VALUE));
      write_attr_value.handle = p_clcb->robust_caching_handle;
      write_attr_value.len = 1;
      write_attr_value.value[0] = GATT_WRITE_ROBUST_CACHING_SUPPORT_VALUE;
      GATTC_Write(p_clcb->conn_id, GATT_WRITE, &write_attr_value);
      break;
    }
  }
}

/*******************************************************************************
 *
 * Function         GATT_EnableRobustCaching
 *
 * Description      Write to client supp features char on remote device
 *                  to enable robust caching
 *
 * Returns          none
 *
 ******************************************************************************/
void GATT_EnableRobustCaching(const RawAddress& remote_bda,
                              tBT_TRANSPORT transport) {
  VLOG(1) << __func__;
  tGATT_PROFILE_CLCB* p_clcb = NULL;
  tGATT_TCB* p_tcb = NULL;

  p_tcb = gatt_find_tcb_by_addr(remote_bda, transport);
  if (p_tcb == NULL) return;

  p_clcb = gatt_profile_find_clcb_by_bd_addr(remote_bda, transport);
  if (p_clcb == NULL)
    p_clcb = gatt_profile_clcb_alloc(0, remote_bda, transport);

  if (p_clcb == NULL) return;

  if (GATT_GetConnIdIfConnected(gatt_cb.gatt_if, remote_bda, &p_clcb->conn_id,
                                transport)) {
    p_clcb->connected = true;
  }

  GATT_Connect(gatt_cb.gatt_if, remote_bda, true, transport, true);
  p_clcb->robust_caching_stage = GATT_ROBUST_CACHING_CL_SUPP_FEAT_CONNECTING;

  if (!p_clcb->connected) {
    /* wait for connection */
    return;
  }

  p_clcb->robust_caching_stage++;
  gatt_cl_write_robust_caching_support(p_clcb);

}

/*******************************************************************************
 *
 * Function         gatt_sr_is_robust_caching_enabled
 *
 * Description      Check if Robust Caching is enabled on server side.
 *
 * Returns          true if enabled in gd flag, otherwise false
 *
 ******************************************************************************/
static bool gatt_sr_is_robust_caching_enabled() {
  char gatt_caching_enabled_prop[PROPERTY_VALUE_MAX] = "false";
  bool is_gatt_robust_caching_enabled = false;
  if (property_get("persist.vendor.btstack.enable.gatt_robust_caching", gatt_caching_enabled_prop, "true")
    && !strcmp(gatt_caching_enabled_prop, "true")) {
    is_gatt_robust_caching_enabled = true;
  }

  VLOG(1) << __func__ << " is_gatt_robust_caching_enabled:" << +is_gatt_robust_caching_enabled;
  return is_gatt_robust_caching_enabled;
}

/*******************************************************************************
 *
 * Function         gatt_sr_is_cl_robust_caching_supported
 *
 * Description      Check if Robust Caching is supported for the connection
 *
 * Returns          true if enabled by client side, otherwise false
 *
 ******************************************************************************/
static bool gatt_sr_is_cl_robust_caching_supported(tGATT_TCB& tcb) {
  // if robust caching is not enabled, should always return false
  if (!gatt_sr_is_robust_caching_enabled()) return false;
  return (tcb.cl_supp_feat & BLE_GATT_CL_SUP_FEAT_CACHING_BITMASK);
}

/*******************************************************************************
 *
 * Function         gatt_sr_is_cl_change_aware
 *
 * Description      Check if the connection is change-aware
 *
 * Returns          true if change aware, otherwise false
 *
 ******************************************************************************/
bool gatt_sr_is_cl_change_aware(tGATT_TCB& tcb) {
  // if robust caching is not supported, should always return true by default
  if (!gatt_sr_is_cl_robust_caching_supported(tcb)) return true;
  return tcb.is_robust_cache_change_aware;
}

/*******************************************************************************
 *
 * Function         gatt_sr_init_cl_status
 *
 * Description      Restore status for trusted device
 *
 * Returns          none
 *
 ******************************************************************************/
void gatt_sr_init_cl_status(tGATT_TCB& tcb) {
  tcb.cl_supp_feat = btif_storage_get_cl_supp_feat(tcb.peer_bda);
  // This is used to reset bit when robust caching is disabled
  if (!gatt_sr_is_robust_caching_enabled()) {
    tcb.cl_supp_feat &= ~BLE_GATT_CL_SUP_FEAT_CACHING_BITMASK;
  }

  if (gatt_sr_is_cl_robust_caching_supported(tcb)) {
    Octet16 stored_hash = btif_storage_get_gatt_cl_db_hash(tcb.peer_bda);
    tcb.is_robust_cache_change_aware = (stored_hash == gatt_cb.database_hash);
  } else {
    // set default value for untrusted device
    tcb.is_robust_cache_change_aware = true;
  }

  LOG(INFO) << __func__ << ": bda=" << tcb.peer_bda
            << ", cl_supp_feat=" << loghex(tcb.cl_supp_feat)
            << ", aware=" << tcb.is_robust_cache_change_aware;
}

/*******************************************************************************
 *
 * Function         gatt_sr_update_cl_status
 *
 * Description      Update change-aware status for the remote device
 *
 * Returns          none
 *
 ******************************************************************************/
void gatt_sr_update_cl_status(tGATT_TCB& tcb, bool chg_aware) {
  // if robust caching is not supported, do nothing
  if (!gatt_sr_is_cl_robust_caching_supported(tcb)) return;

  // only when client status is changed from change-unaware to change-aware, we
  // can then store database hash into btif_storage
  if (!tcb.is_robust_cache_change_aware && chg_aware) {
    btif_storage_set_gatt_cl_db_hash(tcb.peer_bda, gatt_cb.database_hash);
  }

  // only when the status is changed, print the log
  if (tcb.is_robust_cache_change_aware != chg_aware) {
    LOG(INFO) << __func__ << ": bda=" << tcb.peer_bda
              << ", chg_aware=" << chg_aware;
  }

  tcb.is_robust_cache_change_aware = chg_aware;
}

/* handle request for reading database hash */
static tGATT_STATUS gatt_sr_read_db_hash(uint16_t conn_id,
                                         tGATT_VALUE* p_value) {
  LOG(INFO) << __func__ << ": conn_id=" << loghex(conn_id);

  uint8_t* p = p_value->value;
  Octet16& db_hash = gatt_cb.database_hash;
  ARRAY_TO_STREAM(p, db_hash.data(), (uint16_t)db_hash.size());
  p_value->len = (uint16_t)db_hash.size();

  // Every time when database hash is requested, reset flag.
  uint8_t tcb_idx = GATT_GET_TCB_IDX(conn_id);
  gatt_sr_update_cl_status(gatt_cb.tcb[tcb_idx], /* chg_aware= */ true);
  return GATT_SUCCESS;
}

/* handle request for reading client supported features */
static tGATT_STATUS gatt_sr_read_cl_supp_feat(uint16_t conn_id,
                                              tGATT_VALUE* p_value) {
  // Get tcb info
  uint8_t tcb_idx = GATT_GET_TCB_IDX(conn_id);
  tGATT_TCB& tcb = gatt_cb.tcb[tcb_idx];

  uint8_t* p = p_value->value;
  UINT8_TO_STREAM(p, tcb.cl_supp_feat);
  p_value->len = 1;

  return GATT_SUCCESS;
}

/* handle request for writing client supported features */
static tGATT_STATUS gatt_sr_write_cl_supp_feat(uint16_t conn_id,
                                               tGATT_WRITE_REQ* p_data) {
  std::list<uint8_t> tmp;
  uint16_t len = p_data->len;
  uint8_t value, *p = p_data->value;
  // Read all octets into list
  while (len > 0) {
    STREAM_TO_UINT8(value, p);
    tmp.push_back(value);
    len--;
  }
  // Remove trailing zero octets
  while (!tmp.empty()) {
    if (tmp.back() != 0x00) break;
    tmp.pop_back();
  }

  // Get tcb info
  uint8_t tcb_idx = GATT_GET_TCB_IDX(conn_id);
  tGATT_TCB& tcb = gatt_cb.tcb[tcb_idx];

  std::list<uint8_t> feature_list;
  feature_list.push_back(tcb.cl_supp_feat);

  // If input length is zero, return value_not_allowed
  if (tmp.empty()) {
    LOG(INFO) << __func__ << ": zero length, conn_id=" << loghex(conn_id)
              << ", bda=" << tcb.peer_bda;
    return GATT_VALUE_NOT_ALLOWED;
  }
  // if original length is longer than new one, it must be the bit reset case.
  if (feature_list.size() > tmp.size()) {
    LOG(INFO) << __func__ << ": shorter length, conn_id=" << loghex(conn_id)
              << ", bda=" << tcb.peer_bda;
    return GATT_VALUE_NOT_ALLOWED;
  }
  // new length is longer or equals to the original, need to check bits
  // one by one. Here we use bit-wise operation.
  // 1. Use XOR to locate the change bit, val_xor is the change bit mask
  // 2. Use AND for val_xor and *it_new to get val_and
  // 3. If val_and != val_xor, it means the change is from 1 to 0
  auto it_old = feature_list.cbegin();
  auto it_new = tmp.cbegin();
  for (; it_old != feature_list.cend(); it_old++, it_new++) {
    uint8_t val_xor = *it_old ^ *it_new;
    uint8_t val_and = val_xor & *it_new;
    if (val_and != val_xor) {
      LOG(INFO) << __func__
                << ": bit cannot be reset, conn_id=" << loghex(conn_id)
                << ", bda=" << tcb.peer_bda;
      return GATT_VALUE_NOT_ALLOWED;
    }
  }

  // get current robust caching status before setting new one
  bool curr_caching_state = gatt_sr_is_cl_robust_caching_supported(tcb);

  //Client supp feat char has valid values that can be set in bits 0,1,2
  tcb.cl_supp_feat = (tmp.front() & 0x07);
  if (!gatt_sr_is_robust_caching_enabled()) {
    // remove robust caching bit
    tcb.cl_supp_feat &= ~BLE_GATT_CL_SUP_FEAT_CACHING_BITMASK;
    LOG(INFO) << __func__
              << ": reset robust caching bit, conn_id=" << loghex(conn_id)
              << ", bda=" << tcb.peer_bda;
  }
  // TODO(hylo): save data as byte array
  btif_storage_set_cl_supp_feat(tcb.peer_bda, tcb.cl_supp_feat);

  // get new robust caching status after setting new one
  bool new_caching_state = gatt_sr_is_cl_robust_caching_supported(tcb);
  // only when the first time robust caching request, print the log
  if (!curr_caching_state && new_caching_state) {
    LOG(INFO) << __func__ << ": robust caching enabled by client"
              << ", conn_id=" << loghex(conn_id);
  }

//  if (stack_config_get_interface()->get_pts_save_db_hash()) {
    gatt_save_cl_db_hash(tcb);
//  }

  return GATT_SUCCESS;
}

/* handle request for writing CCCD descriptor */
static tGATT_STATUS gatt_sr_write_cccd(uint16_t conn_id,
                                       tGATT_WRITE_REQ* p_data) {
  uint8_t value = 0, *p = p_data->value;
  // Get tcb info
  uint8_t tcb_idx = GATT_GET_TCB_IDX(conn_id);
  tGATT_TCB& tcb = gatt_cb.tcb[tcb_idx];

  STREAM_TO_UINT8(value, p);
  VLOG(1) << __func__ << " conn_id:" << conn_id
          << " value:" << +value;

  tcb.svc_chg_cccd = value;
  btif_storage_set_svc_chg_cccd(tcb.peer_bda, value);

  return GATT_SUCCESS;
}

/*******************************************************************************
 *
 * Function         gatt_get_db_hash_char_handle
 *
 * Description      The function returns DB Hash characteristic handle
 *
 * Returns          Attribute handle of DB hash characteristic
 *
 ******************************************************************************/
uint16_t gatt_get_db_hash_char_handle() {
  VLOG(1) << __func__ << " DB hash handle:" << gatt_attr_db[2].handle;
  return gatt_attr_db[2].handle;
}

/*******************************************************************************
 *
 * Function         gatt_save_cl_db_hash
 *
 * Description      Save DB hash to btif storage
 *
 * Returns          none
 *
 ******************************************************************************/
void gatt_save_cl_db_hash(tGATT_TCB tcb) {
  VLOG(1) << __func__;

  if (gatt_sr_is_cl_robust_caching_supported(tcb)) {
    VLOG(1) << __func__ << " saving DB Hash";
    LOG(INFO) << __func__ << " saving DB Hash";
    btif_storage_set_gatt_cl_db_hash(tcb.peer_bda, gatt_cb.database_hash);
  }
}

