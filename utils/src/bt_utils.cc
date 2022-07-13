/******************************************************************************
 *
 *  Copyright (C) 2012 Broadcom Corporation
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

/*******************************************************************************
 *
 *  Filename:      bt_utils.cc
 *
 *  Description:   Miscellaneous helper functions
 *
 *
 ******************************************************************************/

#define LOG_TAG "bt_utils"

#include "bt_utils.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <mutex>

#define A2DP_RT_PRIORITY 1
#ifdef ANDROID
#include <cutils/sched_policy.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <sys/time.h>
#include <fcntl.h>
#include <inttypes.h>
#define SOCKETNAME  "/data/misc/bluetooth/btprop"
#endif

#include "bt_types.h"
#include "btcore/include/module.h"
#include "osi/include/compat.h"

#include "osi/include/log.h"
#include "osi/include/properties.h"


typedef struct {
  bt_soc_type soc_type;
  char* soc_name;
} soc_type_node;

static soc_type_node soc_type_entries[] = {
                           { BT_SOC_SMD , (char *)"smd" },
                           { BT_SOC_AR3K , (char *)"ath3k" },
                           { BT_SOC_ROME , (char *)"rome" },
                           { BT_SOC_CHEROKEE , (char *)"cherokee" },
                           { BT_SOC_NAPIER , (char *) "napier" },
                           { BT_SOC_HASTINGS , (char *)"hastings" },
                           { BT_SOC_MOSELLE , (char *)"moselle" },
                           { BT_SOC_HAMILTON, (char *)"hamilton" },
                           { BT_SOC_RESERVED , (char *)"" }
                                       };

/*******************************************************************************
 *  Type definitions for callback functions
 ******************************************************************************/
static pthread_once_t g_DoSchedulingGroupOnce[TASK_HIGH_MAX];
static bool g_DoSchedulingGroup[TASK_HIGH_MAX];
static std::mutex gIdxLock;
static int g_TaskIdx;
static int g_TaskIDs[TASK_HIGH_MAX];
#define INVALID_TASK_ID (-1)
static bt_soc_type soc_type;
static void init_soc_type();

static future_t* init(void) {
  int i;

  for (i = 0; i < TASK_HIGH_MAX; i++) {
    g_DoSchedulingGroupOnce[i] = PTHREAD_ONCE_INIT;
    g_DoSchedulingGroup[i] = true;
    g_TaskIDs[i] = INVALID_TASK_ID;
  }

  init_soc_type();
  return NULL;
}

static future_t* clean_up(void) {
  return NULL;
}

#ifndef ANDROID
#define EXPORT_SYMBOL   __attribute__((visibility("default")))
#endif
EXPORT_SYMBOL extern const module_t bt_utils_module = {.name = BT_UTILS_MODULE,
                                                       .init = init,
                                                       .start_up = NULL,
                                                       .shut_down = NULL,
                                                       .clean_up = clean_up,
                                                       .dependencies = {NULL}};

/*****************************************************************************
 *
 * Function        check_do_scheduling_group
 *
 * Description     check if it is ok to change schedule group
 *
 * Returns         void
 *
 ******************************************************************************/
static void check_do_scheduling_group(void) {
  char buf[PROPERTY_VALUE_MAX];
  int len = osi_property_get("debug.sys.noschedgroups", buf, "");
  if (len > 0) {
    int temp;
    if (sscanf(buf, "%d", &temp) == 1) {
      g_DoSchedulingGroup[g_TaskIdx] = temp == 0;
    }
  }
}

#ifndef ANDROID
static intmax_t property_get_bt_imax(const char *key, intmax_t lower_bound,
                intmax_t upper_bound, intmax_t default_value, bool i32) {
    if (!key) {
        return default_value;
    }

    intmax_t result = default_value;
    char buf[PROPERTY_VALUE_MAX] = {'\0'};
    char *end = NULL;

    int len = property_get(key, buf, "");
    if (len > 0) {
        int tmp = errno;
        errno = 0;

        // Infer base automatically
        if (i32)
            sscanf(key, "%"SCNd32, &result);
        else
            sscanf(key, "%"SCNd64, &result);
        if (( result == INTMAX_MIN || result == INTMAX_MAX) && errno == ERANGE) {
            // Over or underflow
            result = default_value;
            ALOGE("%s(%s,%" PRIdMAX ") - overflow", __FUNCTION__, key,
                default_value);
        } else if (result < lower_bound || result > upper_bound) {
            // Out of range of requested bounds
            result = default_value;
            ALOGE("%s(%s,%" PRIdMAX ") - out of range", __FUNCTION__, key,
                default_value);
        } else if (end == buf) {
            // Numeric conversion failed
            result = default_value;
            ALOGE("%s(%s,%" PRIdMAX ") - conversion failed", __FUNCTION__, key,
                default_value);
        }
        errno = tmp;
    }

    return result;
}

int64_t property_get_bt_int64(const char *key, int64_t default_value) {
    return (int64_t)property_get_bt_imax(key, INT64_MIN, INT64_MAX, default_value, 0);
}

int32_t property_get_bt_int32(const char *key, int32_t default_value) {
    return (int32_t)property_get_bt_imax(key, INT32_MIN, INT32_MAX, default_value, 1);
}
#endif
/*****************************************************************************
 *
 * Function        raise_priority_a2dp
 *
 * Description     Raise task priority for A2DP streaming
 *
 * Returns         void
 *
 ******************************************************************************/
void raise_priority_a2dp(tHIGH_PRIORITY_TASK high_task) {
#ifdef ANDROID
  int rc = 0;
  int tid = gettid();

  {
    std::lock_guard<std::mutex> lock(gIdxLock);
    g_TaskIdx = high_task;

// TODO(armansito): Remove this conditional check once we find a solution
// for system/core on non-Android platforms.
#if defined(OS_GENERIC)
    rc = -1;
#else   // !defined(OS_GENERIC)
#ifdef ANDROID
    pthread_once(&g_DoSchedulingGroupOnce[g_TaskIdx],
                 check_do_scheduling_group);
    if (g_TaskIdx < TASK_HIGH_MAX && g_DoSchedulingGroup[g_TaskIdx]) {
      // set_sched_policy does not support tid == 0
      rc = set_sched_policy(tid, SP_AUDIO_SYS);
    }
#endif
#endif  // defined(OS_GENERIC)

    g_TaskIDs[high_task] = tid;
  }

  if (rc) {
    LOG_WARN(LOG_TAG, "failed to change sched policy, tid %d, err: %d", tid,
             errno);
  }

  // make A2DP threads use RT scheduling policy since they are part of the
  // audio pipeline
  {
    struct sched_param rt_params;
    rt_params.sched_priority = A2DP_RT_PRIORITY;

    const int rc = sched_setscheduler(tid, SCHED_FIFO, &rt_params);
    if (rc != 0) {
      LOG_ERROR(LOG_TAG,
                "%s unable to set SCHED_FIFO priority %d for tid %d, error %s",
                __func__, A2DP_RT_PRIORITY, tid, strerror(errno));
    }
  }
#endif
}

/*****************************************************************************
**
** Function        init_soc_type
**
** Description     Get Bluetooth SoC type from system setting and stores it
**                 in soc_type.
**
** Returns         void.
**
*******************************************************************************/
static void init_soc_type() {
  int ret = 0;
  char bt_soc_type[PROPERTY_VALUE_MAX];
  LOG_INFO("bt_utils: %s", __func__);
  ALOGI("init_soc_type");

  soc_type = BT_SOC_DEFAULT;
#if defined(ANDROID)
  ret = property_get("qcom.bluetooth.soc", bt_soc_type, NULL);

  if (ret == 0) {
    ALOGI("qcom.bluetooth.soc prop not set");
    ret = property_get("vendor.bluetooth.soc", bt_soc_type, NULL);
  }
  if (ret != 0) {
    int i;
    ALOGI("vendor.qcom.bluetooth.soc set to %s\n", bt_soc_type);
    for ( i = BT_SOC_AR3K ; i < BT_SOC_RESERVED ; i++ ) {
      char* soc_name = soc_type_entries[i].soc_name;
      if (!strcmp(bt_soc_type, soc_name)) {
        soc_type = soc_type_entries[i].soc_type;
        break;
      }
    }
  }
#elif defined(BT_SOC_TYPE_ROME)
    soc_type = BT_SOC_ROME;
#elif defined(BT_SOC_TYPE_CHEROKEE)
    soc_type = BT_SOC_CHEROKEE;
#elif defined(BT_SOC_TYPE_HASTINGS)
    soc_type = BT_SOC_HASTINGS;
#elif defined(BT_SOC_TYPE_MOSELLE)
    soc_type = BT_SOC_MOSELLE;
#elif defined(BT_SOC_TYPE_HAMILTON)
    soc_type = BT_SOC_HAMILTON;
#endif
}

/*****************************************************************************
**
** Function        get_soc_type
**
** Description     This function is used to get the Bluetooth SoC type.
**
** Returns         bt_soc_type.
**
*******************************************************************************/
bt_soc_type get_soc_type() {
  return soc_type;
}
