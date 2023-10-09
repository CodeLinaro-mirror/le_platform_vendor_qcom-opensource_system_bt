/******************************************************************************
 *
 *  Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 *  Not a Contribution.
 *
 *  Copyright (C) 2014 Google, Inc.
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
 ******************************************************************************/

#define LOG_TAG "bt_obex_sock"

#include <base/logging.h>
#include <hardware/bluetooth.h>
#include <hardware/bt_sock.h>

#include <stdlib.h>
#include "hardware/bt_obex_sock.h"
#include "btif_sock.h"
#include "osi/include/log.h"

using bluetooth::Uuid;
static const Uuid uuid_empty = Uuid::kEmpty;
static const btsock_interface_t *sock_interface;
static bt_status_t btsock_obex_listen(btsock_type_t type, const char* service_name,
                                 const uint8_t* service_uuid, int channel, int* sock_fd,
                                 int flags, int app_uid);

static bt_status_t btsock_obex_connect(const bt_bdaddr_t_v1 *bd_addr, btsock_type_t type,
                                   const uint8_t* uuid, int channel, int* sock_fd,
                                  int flags, int app_uid);

btsock_interface_t_v1* btif_obex_get_interface(void) {
  static btsock_interface_t_v1 interface = {sizeof(interface), btsock_obex_listen,
                                          btsock_obex_connect
                                        };
  return &interface;
}

static bt_status_t btsock_obex_listen(btsock_type_t type, const char* service_name,
                                 const uint8_t* service_uuid, int channel, int* sock_fd,
                                 int flags, int app_uid) {
  LOG_DEBUG(LOG_TAG, "%s", __func__);
  Uuid uuid_obj;
  Uuid* serv_uuid = &uuid_obj;
  if ((flags & BTSOCK_FLAG_NO_SDP) == 0) {
    CHECK(sock_fd != NULL);
  }

  bt_status_t status = BT_STATUS_FAIL;
  if (sock_fd == NULL) {
    LOG_DEBUG(LOG_TAG, "%s Return if sock_fd is NULL", __func__);
    return status;
  }
  LOG_DEBUG(LOG_TAG, "%s Wrapper around UUID", __func__);
  if (service_uuid != NULL) {
     LOG_DEBUG(LOG_TAG, "assigning uuid from the pointer", __func__);
     *serv_uuid  = Uuid::From128BitLE(service_uuid );
     LOG_DEBUG(LOG_TAG, "UUID assigned", __func__);
  } else {
     LOG_DEBUG(LOG_TAG, "UUID is NULL", __func__);
     serv_uuid = &uuid_empty;
     LOG_DEBUG(LOG_TAG, "UUID made empty", __func__);
  }
  status = sock_interface->listen(type, service_name, serv_uuid, channel,
                                 sock_fd, flags, app_uid);
  return status;
}

static bt_status_t btsock_obex_connect(const bt_bdaddr_t_v1 *bd_addr, btsock_type_t type,
                                   const uint8_t* uuid, int channel, int* sock_fd,
                                   int flags, int app_uid) {
  LOG_DEBUG(LOG_TAG, "%s", __func__);
  bt_status_t status = BT_STATUS_FAIL;
  if (sock_fd == NULL || bd_addr == NULL)
    return status;

  Uuid uuid_obj;
  Uuid* serv_uuid = &uuid_obj;
  const RawAddress *bd_address = new RawAddress(bd_addr->address);

  if (uuid == NULL) {
     LOG_DEBUG(LOG_TAG, "UUID is NULL", __func__);
     serv_uuid = &uuid_empty;
     LOG_DEBUG(LOG_TAG, "UUID made empty", __func__);
  } else {
     LOG_DEBUG(LOG_TAG, "assigning uuid from the pointer", __func__);
     *serv_uuid  = Uuid::From128BitLE(uuid);
     LOG_DEBUG(LOG_TAG, "UUID assigned", __func__);
  }

  status = sock_interface->connect(bd_address, type, serv_uuid, channel,
                                  sock_fd, flags, app_uid);

  return status;
}

bt_status_t btif_obex_sock_init(void) {
  if ((sock_interface = btif_sock_get_interface()) == NULL) {
     LOG_ERROR(LOG_TAG, "%s couldn't create a socket interface", __func__);
     return BT_STATUS_FAIL;
  }
  return BT_STATUS_SUCCESS;
}
void btif_obex_sock_cleanup(void) {
  sock_interface = NULL;
}
