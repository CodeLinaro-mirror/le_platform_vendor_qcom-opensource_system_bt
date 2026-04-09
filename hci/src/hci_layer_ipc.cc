/******************************************************************************
 *  Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 *  Not a Contribution.
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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <sys/ioctl.h>
#include <base/bind.h>
#include <base/location.h>
#include <base/logging.h>
#include <base/threading/thread.h>
#include "buffer_allocator.h"
#include "osi/include/log.h"
#include "osi/include/properties.h"
#include "hci_hal.h"
#include <mutex>
#include <vector>
#include "bt_transport_hal.h"
#include "HidlSupport.h"
#define BT_VND_START "wc_transport.start_hci"

using android::hardware::hidl_vec;

#define LOG_PATH "/etc/bluetooth/firmware_events.log"
#define LAST_LOG_PATH "/etc/bluetooth/firmware_events.log.last"
using android::hardware::bluetooth::V1_0::HciPacket;
#define CTRL_SOCK "/data/misc/bluetooth/ctrl_sock"
#define BT_SOCK "/data/misc/bluetooth/bt_sock"
#define STOP_CHANNEL 0xDD
int connect_to_local_socket(char* name);
uint16_t transmit_data(serial_data_type_t type, uint8_t *data, uint16_t length);
void monitor_socket(int ctrl_fd, int fd);

int fd_btsock = -1;

extern void initialization_complete();
extern void hci_event_received(const tracked_objects::Location& from_here,
                               BT_HDR* packet);
extern void acl_event_received(BT_HDR* packet);
extern void sco_data_received(BT_HDR* packet);

static std::mutex bthci_mutex;

void hci_initialize() {
  LOG_INFO(LOG_TAG, "%s", __func__);
  std::lock_guard<std::mutex> lock(bthci_mutex);
  char value[PROPERTY_VALUE_MAX] = {'\0'};
  property_get_bt(BT_VND_START, value, false);

  if (strcmp(value, "false") == 0) {
       ALOGE("%s: Daemon has not started", __func__);
  }

  if (fd_btsock < 0) {
       fd_btsock = connect_to_local_socket(BT_SOCK);
  } else {
       ALOGW("%s: BT channel already up", __func__);
  }
}

int connect_to_local_socket(char* name) {
   socklen_t len; int sk = -1;
   struct sockaddr_un addr;

   ALOGE("%s: ACCEPT ", __func__);
   sk  = socket(AF_LOCAL, SOCK_STREAM, 0);
   if (sk < 0) {
       ALOGE("Socket creation failure");
       return -1;
   }
   memset(&addr, 0, sizeof(addr));
   addr.sun_family = AF_LOCAL;
   memcpy(addr.sun_path, name, strlen(name));
   ALOGE("connect_to_local_socket: addr.sun_path = %s", addr.sun_path);
   if (connect(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0)
   {
       ALOGE("failed to connect (%s)", strerror(errno));
       close(sk);
       sk = -1;
   } else {
       ALOGE("%s: Connection succeeded\n", __func__);
   }
  int sv[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
    LOG(FATAL) << "socketpair failed: " << strerror(errno);
  }

  reader_thread_ctrl_fd = sv[0];
  reader_thread = new Thread("hci_sock_reader");
  reader_thread->Start();
  reader_thread->task_runner()->PostTask(
      FROM_HERE, base::Bind(&monitor_socket, sv[1], bt_vendor_fd));

   LOG(INFO) << "HCI device ready";
}

void monitor_socket(int ctrl_fd, int fd) {
  const allocator_t* buffer_allocator = buffer_allocator_get_interface();
  const size_t buf_size = 2000;
  uint8_t buf[buf_size];
  ssize_t len = read(fd, buf, buf_size);

  while (len > 0) {
    if (len == buf_size)
      LOG(FATAL) << "This packet filled buffer, if it have continuation we "
                    "don't know how to merge it, increase buffer size!";

    uint8_t type = buf[0];

    size_t packet_size = buf_size + BT_HDR_SIZE;
    BT_HDR* packet =
        reinterpret_cast<BT_HDR*>(buffer_allocator->alloc(packet_size));
    packet->offset = 0;
    packet->layer_specific = 0;
    packet->len = len - 1;
    memcpy(packet->data, buf + 1, len - 1);

    switch (type) {
      case HCI_PACKET_TYPE_COMMAND:
        packet->event = MSG_HC_TO_STACK_HCI_EVT;
        hci_event_received(FROM_HERE, packet);
        break;
      case HCI_PACKET_TYPE_ACL_DATA:
        packet->event = MSG_HC_TO_STACK_HCI_ACL;
        acl_event_received(packet);
        break;
      case HCI_PACKET_TYPE_SCO_DATA:
        packet->event = MSG_HC_TO_STACK_HCI_SCO;
        sco_data_received(packet);
        break;
      case HCI_PACKET_TYPE_EVENT:
        packet->event = MSG_HC_TO_STACK_HCI_EVT;
        hci_event_received(FROM_HERE, packet);
        break;
      default:
        LOG(FATAL) << "Unexpected event type: " << +type;
        break;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(ctrl_fd, &fds);
    FD_SET(fd, &fds);
    int res = select(std::max(fd, ctrl_fd) + 1, &fds, NULL, NULL, NULL);
    if (res <= 0) LOG(INFO) << "Nothing more to read";

    if (FD_ISSET(ctrl_fd, &fds)) {
      LOG(INFO) << "exitting";
      return;
    }

    len = read(fd, buf, buf_size);
  }
}

void hci_close() {
  LOG_INFO("%s", __func__);

  std::lock_guard<std::mutex> lock(bthci_mutex);
  close(fd_btsock);
  fd_btsock = -1;
}

void hci_transmit(BT_HDR* packet) {
  LOG_DEBUG(LOG_TAG, "%s", __func__);
  HciPacket data;
  serial_data_type_t type;
  std::lock_guard<std::mutex> lock(bthci_mutex);

  LOG_INFO(LOG_TAG, "%s: %d", __func__,data.size());
  uint16_t event = packet->event & MSG_EVT_MASK;
  switch (event & MSG_EVT_MASK) {
    case MSG_STACK_TO_HC_HCI_CMD:
    {
      LOG_DEBUG(LOG_TAG, "%s MSG_STACK_TO_HC_HCI_CMD", __func__);
      type = DATA_TYPE_COMMAND;
      transmit_data(type, packet->data + packet->offset, packet->len);
      break;
    }
    case MSG_STACK_TO_HC_HCI_ACL:
    {
      LOG_DEBUG(LOG_TAG, "%s MSG_STACK_TO_HC_HCI_ACL", __func__);
      type = DATA_TYPE_ACL;
      transmit_data(type, packet->data + packet->offset, packet->len);
      break;
    }
    case MSG_STACK_TO_HC_HCI_SCO:
    {
      LOG_DEBUG(LOG_TAG, "%s MSG_STACK_TO_HC_HCI_SCO", __func__);
      type = DATA_TYPE_SCO;
      transmit_data(type, packet->data + packet->offset, packet->len);
      break;
    }
    default:
      LOG_ERROR(LOG_TAG, "Unknown packet type (%d)", event);
      break;
  }
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

uint16_t transmit_data(serial_data_type_t type, uint8_t *data, uint16_t length) {
  assert(data != NULL);
  assert(length > 0);

  if (type < DATA_TYPE_COMMAND || type > DATA_TYPE_SCO) {
    LOG_ERROR(LOG_TAG, "%s invalid data type: %d", __func__, type);
    return 0;
  }

  // Write the signal byte right before the data
  --data;
  uint8_t previous_byte = *data;
  *(data) = type;
  ++length;

  uint16_t transmitted_length = 0;
  while (length > 0) {
    ssize_t ret;
    ret = write(fd_btsock, data + transmitted_length, length);
    switch (ret) {
      case -1:
        LOG_ERROR(LOG_TAG, "In %s, error writing to BT channel: %s", __func__, strerror(errno));
        goto done;
      case 0:
        // If we wrote nothing, don't loop more because we
        // can't go to infinity or beyond
        goto done;
      default:
        transmitted_length += ret;
        length -= ret;
        break;
    }
  }

done:;
  // Be nice and restore the old value of that byte
  *(data) = previous_byte;

  // Remove the signal byte from our transmitted length, if it was actually written
  if (transmitted_length > 0)
    --transmitted_length;

  return transmitted_length;
}
