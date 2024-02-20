/******************************************************************************
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

#pragma once
//#include <base/logging.h>
#include <string.h>
#include <iostream>
#include <sstream>

#include "include/bt_logger_lib.h" // gghai, add file

extern bt_logger_interface_t *logger_interface;
extern bool bt_logger_enabled;

/*
 * TODO(armansito): Work-around until we figure out a way to generate logs in a
 * platform-independent manner.
 */

class LogMessage {
 public:
  LogMessage(const char* file, unsigned int line, const char* severity) {
  }

  LogMessage(const char* file,
             unsigned int line,
             const char* severity,
             int error) {
             }
  ~LogMessage();

  // Returns the stream associated with the message, the LogMessage performs
  // output when it goes out of scope.
  std::ostream& stream() { return stream_; }

 private:
  std::ostringstream stream_;
  int error_;  // The saved errno value.
};

//#define LOG(severity) LogMessage(__FILE__, __LINE__, #severity).stream()

#define android_errorWriteLog(tag, subTag) 2
#if defined(OS_GENERIC)

/* syslog didn't work well here since we would be redefining LOG_DEBUG. */
#include <stdio.h>

#define LOGWRAPPER(tag, fmt, args...) \
  fprintf(stderr, "%s: " fmt "\n", tag, ##args)

#define LOG_VERBOSE(...) LOGWRAPPER(__VA_ARGS__)
#define LOG_DEBUG(...) LOGWRAPPER(__VA_ARGS__)
#define LOG_INFO(...) LOGWRAPPER(__VA_ARGS__)
#define LOG_WARN(...) LOGWRAPPER(__VA_ARGS__)
#define LOG_ERROR(...) LOGWRAPPER(__VA_ARGS__)

#define LOG_EVENT_INT(...)

#else /* !defined(OS_GENERIC) */

#include <syslog.h>
#define LOG_EVENT_INT(...)
/**
 * These log statements are effectively executing only ALOG(_________, tag, fmt,
 * ## args ).
 * fprintf is only to cause compilation error when LOG_TAG is not provided,
 * which breaks build on Linux (for OS_GENERIC).
 */
#ifdef ANDROID
#if LOG_NDEBUG
#define LOG_VERBOSE(tag, fmt, args...)                          \
  do {                                                          \
    (true) ? ((int)0) : fprintf(stderr, "%s" fmt, tag, ##args); \
  } while (0)
#else  // LOG_NDEBUG
#define LOG_VERBOSE(tag, fmt, args...)               \
  do {                                               \
    (true) ? ALOG(LOG_VERBOSE, tag, fmt, ##args)     \
           : fprintf(stderr, "%s" fmt, tag, ##args); \
  } while (0)
#endif  // !LOG_NDEBUG

#define LOG_DEBUG(tag, fmt, args...)                     \
  do {                                                   \
    (true) ? ALOG(LOG_DEBUG, tag, fmt, ##args)           \
           : fprintf(stderr, "%s" fmt, tag, ##args);     \
  } while (0);
#define LOG_INFO(tag, fmt, args...)                      \
  do {                                                   \
    (true) ? ALOG(LOG_INFO, tag, fmt, ##args)            \
           : fprintf(stderr, "%s" fmt, tag, ##args);     \
  } while (0);
#define LOG_WARN(tag, fmt, args...)                      \
  do {                                                   \
    (true) ? ALOG(LOG_WARN, tag, fmt, ##args)            \
           : fprintf(stderr, "%s" fmt, tag, ##args);     \
  } while (0);
#define LOG_ERROR(tag, fmt, args...)                     \
  do {                                                   \
    (true) ? ALOG(LOG_ERROR, tag, fmt, ##args)           \
           : fprintf(stderr, "%s" fmt, tag, ##args);     \
  } while (0);

#else
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#ifdef USE_ANDROID_LOGGING
#include <syslog.h>
#define LOG_TAG "bt_stack"
#define ALOGV(fmt, arg...) syslog (LOG_WARNING, fmt, ##arg)
#define ALOGD(fmt, arg...) syslog (LOG_NOTICE, fmt, ##arg)
#define ALOGI(fmt, arg...) syslog (LOG_NOTICE, fmt, ##arg)
#define ALOGW(fmt, arg...) syslog (LOG_WARNING, fmt, ##arg)
#define ALOGE(fmt, arg...) syslog (LOG_ERR, fmt, ##arg)
#define LOG_VERBOSE(tag, fmt, args...) syslog (LOG_WARNING, fmt, ##args)
#define LOG_DEBUG(tag, fmt, args...)   syslog (LOG_NOTICE, fmt, ##args)
#define LOG_INFO(tag, fmt, args...)   syslog (LOG_NOTICE, fmt, ##args)
#define LOG_WARN(tag, fmt, args...)   syslog (LOG_WARNING, fmt, ##args)
#define LOG_ERROR(tag, fmt, args...)   syslog (LOG_ERR, fmt, ##args)
#else
#include <syslog.h>
#define LOG_TAG "bt_stack : "
#define PRI_INFO " I"
#define PRI_WARN " W"
#define PRI_ERROR " E"
#define PRI_DEBUG " D"
#define PRI_VERB " V"
#define ALOGV(fmt, arg...) syslog (LOG_WARNING, fmt, ##arg)
#define ALOGD(fmt, arg...) syslog (LOG_NOTICE, fmt, ##arg)
#define ALOGI(fmt, arg...) syslog (LOG_NOTICE, fmt, ##arg)
#define ALOGW(fmt, arg...) syslog (LOG_WARNING, fmt, ##arg)
#define ALOGE(fmt, arg...) syslog (LOG_ERR, fmt, ##arg)
#define LOG_VERBOSE(fmt, arg...) syslog (LOG_WARNING, LOG_TAG fmt, ##arg)
#define LOG_DEBUG(fmt, arg...) syslog (LOG_NOTICE, LOG_TAG fmt, ##arg)
#define LOG_INFO(fmt, arg...)  syslog (LOG_NOTICE, LOG_TAG fmt, ##arg)
#define LOG_WARN(fmt, arg...)  syslog (LOG_WARNING, LOG_TAG fmt, ##arg)
#define LOG_ERROR(fmt, arg...) syslog (LOG_ERR, LOG_TAG fmt, ##arg)
#endif
#endif
#endif /* defined(OS_GENERIC) */
