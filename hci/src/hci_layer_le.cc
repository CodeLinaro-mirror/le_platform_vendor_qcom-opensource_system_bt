/******************************************************************************
 *
 *  Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 *  Not a Contribution.
 *
 *
 *  Copyright (C) 2017 Google, Inc.
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

#define LOG_TAG "bt_hci_le"

#include "hci_layer.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utils/StrongPointer.h>
#include <base/location.h>
#include <base/logging.h>
#include "buffer_allocator.h"
#include "osi/include/log.h"
#include <mutex>
#include <vector>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include "bt_transport_hal.h"
#include "HidlSupport.h"
using android::hardware::hidl_vec;

#define LOG_PATH "/data/misc/bluetooth/firmware_events.log"
#define LAST_LOG_PATH "/data/misc/bluetooth/firmware_events.log.last"

static void *lib_handle;
static const char TRANSPORT_LIBRARY_NAME[] = "bttransport.so";
static const char TRANSPORT_LIBRARY_SYMBOL_NAME[] = "BLUETOOTH_VENDOR_HCI_INTERFACE";

using android::hardware::bluetooth::V1_0::HciPacket;
using android::hardware::bluetooth::V1_0::Status;
using android::hardware::bluetooth::V1_0::implementation::BluetoothHci;
using android::hardware::bluetooth::V1_0::implementation::BluetoothHciCallbacks;

extern void initialization_complete();
extern void hci_event_received(const tracked_objects::Location& from_here,
                               BT_HDR* packet);
extern void acl_event_received(BT_HDR* packet);
extern void sco_data_received(BT_HDR* packet);
void vnd_interface_open(void);
static std::mutex bthci_mutex;
bt_vnd_interface_t* btHci;
BluetoothHciCallbacks* callbacks;

static void init_cb(android::hardware::bluetooth::V1_0::implementation::Status status) {
    callbacks = new BluetoothHciCallbacks();
    callbacks->initializationComplete(status);
}
static void hci_evt_rcvd_cb(const hidl_vec<uint8_t>& data) {
    callbacks->hciEventReceived(data);
}
static void acl_cmd_rcvd_cb(const hidl_vec<uint8_t>& data) {
    callbacks->aclDataReceived(data);
}
static void sco_data_rcvd_cb(const hidl_vec<uint8_t>& data) {
    callbacks->scoDataReceived(data);
}

static const bt_vnd_cb_t hci_callbacks = {
  sizeof(hci_callbacks),
  init_cb,
  hci_evt_rcvd_cb,
  acl_cmd_rcvd_cb,
  sco_data_rcvd_cb
};

BluetoothHciCallbacks::BluetoothHciCallbacks() {
    buffer_allocator = buffer_allocator_get_interface();
}

BT_HDR* BluetoothHciCallbacks::WrapPacketAndCopy(uint16_t event, const hidl_vec<uint8_t>& data) {
    size_t packet_size = data.size() + BT_HDR_SIZE;
    BT_HDR* packet =
        reinterpret_cast<BT_HDR*>(buffer_allocator->alloc(packet_size));
    packet->offset = 0;
    packet->len = data.size();
    packet->layer_specific = 0;
    packet->event = event;
    // TODO(eisenbach): Avoid copy here; if BT_HDR->data can be ensured to
    // be the only way the data is accessed, a pointer could be passed here...
    memcpy(packet->data, data.data(), data.size());
    return packet;
}

void BluetoothHciCallbacks::initializationComplete(Status status) {
    CHECK(status == Status::SUCCESS);
    LOG_INFO(LOG_TAG, "%s", __func__);
    initialization_complete();
}

void BluetoothHciCallbacks::hciEventReceived(const hidl_vec<uint8_t>& event) {
    BT_HDR* packet = WrapPacketAndCopy(MSG_HC_TO_STACK_HCI_EVT, event);
    hci_event_received(FROM_HERE, packet);
}

void BluetoothHciCallbacks::aclDataReceived(const hidl_vec<uint8_t>& data) {
    BT_HDR* packet = WrapPacketAndCopy(MSG_HC_TO_STACK_HCI_ACL, data);
    acl_event_received(packet);
}

void BluetoothHciCallbacks::scoDataReceived(const hidl_vec<uint8_t>& data) {
    BT_HDR* packet = WrapPacketAndCopy(MSG_HC_TO_STACK_HCI_SCO, data);
    sco_data_received(packet);
}

void hci_initialize() {
  LOG_INFO(LOG_TAG, "%s", __func__);
  vnd_interface_open();
  CHECK(btHci != nullptr);
  btHci->init(&hci_callbacks);
}

void hci_close() {
  LOG_INFO("%s", __func__);

  std::lock_guard<std::mutex> lock(bthci_mutex);
  if (btHci != nullptr) {
    auto hidl_daemon_status = btHci->cleanup();
    if(!hidl_daemon_status)
      LOG_ERROR(LOG_TAG, "%s: HIDL daemon is dead", __func__);

    btHci = nullptr;
  }
  if (callbacks != nullptr) {
    delete callbacks;
    callbacks = nullptr;
  }
}

bool hci_transmit(BT_HDR* packet) {
  HciPacket data;
  hci_transmit_status_t status = HCI_TRANSMIT_SUCCESS;
  std::lock_guard<std::mutex> lock(bthci_mutex);

  if(btHci == nullptr) {
    LOG_INFO(LOG_TAG, "%s: Link with Bluetooth HIDL service is closed", __func__);
    return HCI_TRANSMIT_DAEMON_CLOSED;
  }

    data.insert(data.begin(), (packet->data + packet->offset), (packet->data + packet->offset + packet->len));
    /*To print data
    for(int n : data) {
       LOG_INFO(LOG_TAG, "%s: %x", __func__,n);
    }*/
  uint16_t event = packet->event & MSG_EVT_MASK;
  switch (event & MSG_EVT_MASK) {
    case MSG_STACK_TO_HC_HCI_CMD:
    {
      auto hidl_daemon_status = btHci->send_hci_cmd(data);
      if(hidl_daemon_status) {
        LOG_ERROR(LOG_TAG, "%s: send Command failed, HIDL daemon is dead", __func__);
        status = HCI_TRANSMIT_DAEMON_DIED;
      }
      break;
    }
    case MSG_STACK_TO_HC_HCI_ACL:
    {
      auto hidl_daemon_status = btHci->send_acl_cmd(data);
      if(hidl_daemon_status) {
        LOG_ERROR(LOG_TAG, "%s: send acl packet failed, HIDL daemon is dead", __func__);
        status = HCI_TRANSMIT_DAEMON_DIED;
      }
      break;
    }
    case MSG_STACK_TO_HC_HCI_SCO:
    {
      auto hidl_daemon_status = btHci->send_sco_data(data);
      if(hidl_daemon_status) {
        LOG_ERROR(LOG_TAG, "%s: send sco data failed, HIDL daemon is dead", __func__);
        status = HCI_TRANSMIT_DAEMON_DIED;
      }
      break;
    }
    default:
      status = HCI_TRANSMIT_INVALID_PKT;
      LOG_ERROR(LOG_TAG, "Unknown packet type (%d)", event);
      break;
  }
  return status;
}

int hci_open_firmware_log_file() {
  if (rename(LOG_PATH, LAST_LOG_PATH) == -1 && errno != ENOENT) {
    LOG_ERROR(LOG_TAG, "%s unable to rename '%s' to '%s': %s", __func__,
              LOG_PATH, LAST_LOG_PATH, strerror(errno));
  }

  mode_t prevmask = umask(0);
  int logfile_fd = open(LOG_PATH, O_WRONLY | O_CREAT | O_TRUNC,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  umask(prevmask);
  if (logfile_fd == INVALID_FD) {
    LOG_ERROR(LOG_TAG, "%s unable to open '%s': %s", __func__, LOG_PATH,
              strerror(errno));
  }

  return logfile_fd;
}

void hci_close_firmware_log_file(int fd) {
  if (fd != INVALID_FD) close(fd);
}

void hci_log_firmware_debug_packet(int fd, BT_HDR* packet) {
  TEMP_FAILURE_RETRY(write(fd, packet->data, packet->len));
}

void vnd_interface_open(void)
{
  if(btHci != NULL)
  {
    LOG_ERROR(LOG_TAG, "%s, Vendor Interface is already initialized",  __func__);
    return;
  }

  lib_handle = dlopen(TRANSPORT_LIBRARY_NAME, RTLD_LAZY);

  if (!lib_handle) {
    LOG_ERROR(LOG_TAG, "%s unable to open %s: %s", __func__, TRANSPORT_LIBRARY_NAME, dlerror());
    return;
  }

  btHci = (bt_vnd_interface_t *)dlsym(lib_handle, TRANSPORT_LIBRARY_SYMBOL_NAME);
  if (btHci == NULL) {
    LOG_ERROR(LOG_TAG, "%s unable to find symbol %s in %s: %s", __func__, TRANSPORT_LIBRARY_SYMBOL_NAME, TRANSPORT_LIBRARY_NAME, dlerror());
    return;
  }
}
