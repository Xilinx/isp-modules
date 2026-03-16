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

#ifndef __VISP_AE_H__
#define __VISP_AE_H__

#include "visp_ctrl.h"

#define VISP_CID_AE_ENABLE                      (VISP_CID_AE_BASE + 0x0000)
#define VISP_CID_AE_RESET                       (VISP_CID_AE_BASE + 0x0001)
#define VISP_CID_AE_STATE                       (VISP_CID_AE_BASE + 0x0002)
#define VISP_CID_AE_SEM_MODE                    (VISP_CID_AE_BASE + 0x0004)
#define VISP_CID_AE_FLICKER_PERIOD              (VISP_CID_AE_BASE + 0x0005)
#define VISP_CID_AE_SETPOINT                    (VISP_CID_AE_BASE + 0x0006)
#define VISP_CID_AE_TOLORENCE                   (VISP_CID_AE_BASE + 0x0007)
#define VISP_CID_AE_DAMPOVER                    (VISP_CID_AE_BASE + 0x0008)
#define VISP_CID_AE_DAMPOVER_RATIO              (VISP_CID_AE_BASE + 0x0009)
#define VISP_CID_AE_DAMPOVER_GAIN               (VISP_CID_AE_BASE + 0x000A)
#define VISP_CID_AE_DAMPUNDER                   (VISP_CID_AE_BASE + 0x000B)
#define VISP_CID_AE_DAMPUNDER_RATIO             (VISP_CID_AE_BASE + 0x000C)
#define VISP_CID_AE_DAMPUNDER_GAIN              (VISP_CID_AE_BASE + 0x000D)
#define VISP_CID_AE_MOTION_FILTER               (VISP_CID_AE_BASE + 0x000E)
#define VISP_CID_AE_MOTION_THRESHOLD            (VISP_CID_AE_BASE + 0x000F)
#define VISP_CID_AE_TARGET_FILTER               (VISP_CID_AE_BASE + 0x0010)
#define VISP_CID_AE_LOWLIGHT_LINEAR_REPRESS     (VISP_CID_AE_BASE + 0x0011)
#define VISP_CID_AE_LOWLIGHT_LINEAR_GAIN        (VISP_CID_AE_BASE + 0x0012)
#define VISP_CID_AE_LOWLIGHT_LINEAR_LEVEL       (VISP_CID_AE_BASE + 0x0013)
#define VISP_CID_AE_LOWLIGHT_HDR_REPRESS        (VISP_CID_AE_BASE + 0x0014)
#define VISP_CID_AE_LOWLIGHT_HDR_GAIN           (VISP_CID_AE_BASE + 0x0015)
#define VISP_CID_AE_LOWLIGHT_HDR_LEVEL          (VISP_CID_AE_BASE + 0x0016)
#define VISP_CID_AE_WDR_CONTRAST_MIN            (VISP_CID_AE_BASE + 0x0017)
#define VISP_CID_AE_WDR_CONTRAST_MAX            (VISP_CID_AE_BASE + 0x0018)
#define VISP_CID_AE_ROI_WEIGHT                  (VISP_CID_AE_BASE + 0x001A)
#define VISP_CID_AE_ROI                         (VISP_CID_AE_BASE + 0x001B)
#define VISP_CID_AE_HIST                        (VISP_CID_AE_BASE + 0x001C)
#define VISP_CID_AE_LUMA                        (VISP_CID_AE_BASE + 0x001D)
#define VISP_CID_AE_OBJECT_REGION               (VISP_CID_AE_BASE + 0x001E)
#define VISP_CID_AE_FRAME_CALC_ENABLE           (VISP_CID_AE_BASE + 0x001F)
#define VISP_CID_AE_EXPV2_WINDOW_WEIGHT         (VISP_CID_AE_BASE + 0x0020)
#define VISP_CID_EC_AGAIN                       (VISP_CID_AE_BASE + 0x0021)
#define VISP_CID_EC_AGAIN_MAX                   (VISP_CID_AE_BASE + 0x0022)
#define VISP_CID_EC_AGAIN_MIN                   (VISP_CID_AE_BASE + 0x0023)
#define VISP_CID_EC_AGAIN_STEP                  (VISP_CID_AE_BASE + 0x0024)
#define VISP_CID_EC_DGAIN                       (VISP_CID_AE_BASE + 0x0025)
#define VISP_CID_EC_DGAIN_MAX                   (VISP_CID_AE_BASE + 0x0026)
#define VISP_CID_EC_DGAIN_MIN                   (VISP_CID_AE_BASE + 0x0027)
#define VISP_CID_EC_DGAIN_STEP                  (VISP_CID_AE_BASE + 0x0028)
#define VISP_CID_EC_INTEGRATION_TIME            (VISP_CID_AE_BASE + 0x0029)
#define VISP_CID_EC_INTEGRATION_TIME_MAX        (VISP_CID_AE_BASE + 0x002A)
#define VISP_CID_EC_INTEGRATION_TIME_MIN        (VISP_CID_AE_BASE + 0x002B)
#define VISP_CID_EC_INTEGRATION_TIME_STEP       (VISP_CID_AE_BASE + 0x002C)
#define VISP_CID_AE_EXP_TABLE                   (VISP_CID_AE_BASE + 0x002D)
#define VISP_CID_AE_EXP_DECOMP_CUSTOM           (VISP_CID_AE_BASE + 0x002E)
#define VISP_CID_AE_PERFORMANCE_OPTI_MODE       (VISP_CID_AE_BASE + 0x002F)
#define VISP_CID_AE_EXT_LIGHT_ENABLE            (VISP_CID_AE_BASE + 0x0030)
#define VISP_CID_AE_EXT_LIGHT_TYPE              (VISP_CID_AE_BASE + 0x0031)
#define VISP_CID_AE_EXT_LIGHT_EXP_MAX           (VISP_CID_AE_BASE + 0x0032)
#define VISP_CID_AE_EXT_LIGHT_EXP_MIN           (VISP_CID_AE_BASE + 0x0033)
#define VISP_CID_AE_RESULT_MEAN_LUMA            (VISP_CID_AE_BASE + 0x0034)
#define VISP_CID_AE_RESULT_LUMA_INDEX           (VISP_CID_AE_BASE + 0x0035)
#define VISP_CID_AE_STRATEGY_MODE               (VISP_CID_AE_BASE + 0x0036)
#define VISP_CID_AE_HIGHLIGHT_LUMA_THR          (VISP_CID_AE_BASE + 0x0037)
#define VISP_CID_AE_HIST_RATIO_SLOPE            (VISP_CID_AE_BASE + 0x0038)
#define VISP_CID_AE_MAX_HIST_OFFSET             (VISP_CID_AE_BASE + 0x0039)
#define VISP_CID_AE_LOWLIGHT_LUMA_THR           (VISP_CID_AE_BASE + 0x003A)
#define VISP_CID_AE_LUMA_INDEX_CALIB            (VISP_CID_AE_BASE + 0x003B)

#define VISP_CID_AE_ALL_MODE                    (VISP_CID_AE_BASE + 0x0040)
#define VISP_CID_AE_ALL_CONFIG                  (VISP_CID_AE_BASE + 0x0041)
#define VISP_CID_AE_ALL_ROI                     (VISP_CID_AE_BASE + 0x0042)
#define VISP_CID_AE_ALL_STATUS                  (VISP_CID_AE_BASE + 0x0043)
#define VISP_CID_EC_ALL_GAIN                    (VISP_CID_AE_BASE + 0x0044)
#define VISP_CID_EC_ALL_RANGE                   (VISP_CID_AE_BASE + 0x0045)
#define VISP_CID_AE_ALL_RESULT                  (VISP_CID_AE_BASE + 0x0046)
#define VISP_CID_AE_ALL_EXP_TBL                 (VISP_CID_AE_BASE + 0x0047)
#define VISP_CID_EC_ALL_EXP_CONTROL             (VISP_CID_AE_BASE + 0x0048)

#ifdef __KERNEL__
int visp_ae_ctrl_count(void);
int visp_ae_ctrl_create(struct visp_dev *isp_dev);
#endif

#endif
