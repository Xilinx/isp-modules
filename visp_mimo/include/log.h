/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

#ifndef __VSI_LOG_H__
#define __VSI_LOG_H__
#if 0
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ISP_LOG_LEVEL_NONE = 0, //No debug information is output.
    ISP_LOG_LEVEL_ERROR ,   //Logs all fatal errors.
    ISP_LOG_LEVEL_WARNING,  //Logs all warnings.
    ISP_LOG_LEVEL_FIXME,    //Logs all "fixme" messages. (Reserved)
    ISP_LOG_LEVEL_INFO,     //Logs all informational messages.
    ISP_LOG_LEVEL_DEBUG,    //Logs all debug messages.
    ISP_LOG_LEVEL_LOG,      //Logs all log messages.     (Reserved)
    ISP_LOG_LEVEL_TRACE,    //Logs all trace messages.   (Reserved)
    ISP_LOG_LEVEL_VERBOSE,  //Logs all level messages.
};

static int isp_log_level();
static inline int isp_log_level() {
    char* szLogLevel = getenv("ISP_LOG_LEVEL");
    if (szLogLevel)
        return atoi(szLogLevel);
    else
        return ISP_LOG_LEVEL_WARNING;
}

// c++11 workaround empty variadic macro
#define __ALOG_INT(format, ...) "[%s] " format "\033[0m%s", LOGTAG, __VA_ARGS__

#ifdef ANDROID

#include <android/log.h>
#define ISP_INFO "VSI_ISP"

#define ALOGV(...)\
    if (isp_log_level() >= ISP_LOG_LEVEL_VERBOSE) __android_log_print(ANDROID_LOG_INFO, ISP_INFO, "\033[1;30;37mVERBOSE : " __ALOG_INT(__VA_ARGS__, "\n"))  // white

#define ALOGD(...)\
    if (isp_log_level() >= ISP_LOG_LEVEL_DEBUG) __android_log_print(ANDROID_LOG_INFO, ISP_INFO, "\033[1;30;37mDEBUG  : " __ALOG_INT(__VA_ARGS__, "\n"))  // white

#define ALOGI(...)\
    if (isp_log_level() >= ISP_LOG_LEVEL_INFO) __android_log_print(ANDROID_LOG_INFO, ISP_INFO, "\033[1;30;32mINFO   : " __ALOG_INT(__VA_ARGS__, "\n"))  // green

#define ALOGW(...)\
    if (isp_log_level() >= ISP_LOG_LEVEL_WARNING) __android_log_print(ANDROID_LOG_INFO, ISP_INFO, "\033[1;30;33mWARN   : " __ALOG_INT(__VA_ARGS__, "\n"))  // yellow

#define ALOGE(...)\
    if (isp_log_level() >= ISP_LOG_LEVEL_ERROR) __android_log_print(ANDROID_LOG_INFO, ISP_INFO, "\033[1;30;31mERROR  : " __ALOG_INT(__VA_ARGS__, "\n"))  // red

#else

#define ALOGV(...)\
    if (isp_log_level() >= ISP_LOG_LEVEL_VERBOSE) printf("\033[1;30;37mVERBOSE : " __ALOG_INT(__VA_ARGS__, "\n"))  // white

#define ALOGD(...)\
    if (isp_log_level() >= ISP_LOG_LEVEL_DEBUG) printf("\033[1;30;37mDEBUG  : " __ALOG_INT(__VA_ARGS__, "\n"))  // white

#define ALOGI(...)\
    if (isp_log_level() >= ISP_LOG_LEVEL_INFO) printf("\033[1;30;32mINFO   : " __ALOG_INT(__VA_ARGS__, "\n"))  // green

#define ALOGW(...)\
    if (isp_log_level() >= ISP_LOG_LEVEL_WARNING) printf("\033[1;30;33mWARN   : " __ALOG_INT(__VA_ARGS__, "\n"))  // yellow

#define ALOGE(...)\
    if (isp_log_level() >= ISP_LOG_LEVEL_ERROR) printf("\033[1;30;31mERROR  : " __ALOG_INT(__VA_ARGS__, "\n"))  // red

#endif

//#define TRACE_IN  ALOGI("enter %s", __PRETTY_FUNCTION__)
//#define TRACE_OUT ALOGI("leave %s", __PRETTY_FUNCTION__)

#ifdef __cplusplus
}
#endif

#endif
#endif  // __VSI_LOG_H__
