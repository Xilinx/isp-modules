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

#ifndef __VISP_AWB_H__
#define __VISP_AWB_H__

#include "visp_ctrl.h"

#define VISP_CID_AWB_ENABLE (VISP_CID_AWB_BASE + 0x0000)
#define VISP_CID_AWB_STATE (VISP_CID_AWB_BASE + 0x0001)
#define VISP_CID_AWB_MODE (VISP_CID_AWB_BASE + 0x0002)
#define VISP_CID_AWB_USE_CC_OFFSET (VISP_CID_AWB_BASE + 0x0003)
#define VISP_CID_AWB_USE_CC_MATRIX (VISP_CID_AWB_BASE + 0x0004)
#define VISP_CID_AWB_USE_DAMPING (VISP_CID_AWB_BASE + 0x0005)
#define VISP_CID_AWB_ROI_WEIGHT (VISP_CID_AWB_BASE + 0x0006)
#define VISP_CID_AWB_ROI (VISP_CID_AWB_BASE + 0x0007)
#define VISP_CID_AWB_COLOR_TEMP_WEIGHT (VISP_CID_AWB_BASE + 0x0008)
#define VISP_CID_AWB_CONFIDENCE_THRESH (VISP_CID_AWB_BASE + 0x0009)
#define VISP_CID_AWB_TEMP_PREFER_ENABLE (VISP_CID_AWB_BASE + 0x000A)
#define VISP_CID_AWB_TEMP_PREFER_A (VISP_CID_AWB_BASE + 0x000B)
#define VISP_CID_AWB_TEMP_PREFER_CWF (VISP_CID_AWB_BASE + 0x000C)
#define VISP_CID_AWB_TEMP_PREFER_D65 (VISP_CID_AWB_BASE + 0x000D)
#define VISP_CID_AWB_CONFOUND_CWF_ENABLE (VISP_CID_AWB_BASE + 0x000E)
#define VISP_CID_AWB_CONFOUND_CWF_RG (VISP_CID_AWB_BASE + 0x000F)
#define VISP_CID_AWB_CONFOUND_CWF_BG (VISP_CID_AWB_BASE + 0x0010)
#define VISP_CID_AWB_CONFOUND_CWF_THRESH (VISP_CID_AWB_BASE + 0x0011)
#define VISP_CID_AWB_CONFOUND_TL84_ENABLE (VISP_CID_AWB_BASE + 0x0012)
#define VISP_CID_AWB_CONFOUND_TL84_RG (VISP_CID_AWB_BASE + 0x0013)
#define VISP_CID_AWB_CONFOUND_TL84_BG (VISP_CID_AWB_BASE + 0x0014)
#define VISP_CID_AWB_CONFOUND_TL84_THRESH (VISP_CID_AWB_BASE + 0x0015)
#define VISP_CID_AWB_CONFOUND_D65_ENABLE (VISP_CID_AWB_BASE + 0x0016)
#define VISP_CID_AWB_CONFOUND_D65_RG (VISP_CID_AWB_BASE + 0x0017)
#define VISP_CID_AWB_CONFOUND_D65_BG (VISP_CID_AWB_BASE + 0x0018)
#define VISP_CID_AWB_CONFOUND_D65_THRESH (VISP_CID_AWB_BASE + 0x0019)
#define VISP_CID_AWB_LIGHT_WEIGHT_ENABLE (VISP_CID_AWB_BASE + 0x001A)
#define VISP_CID_AWB_LIGHT_WEIGHT_BRIGHT (VISP_CID_AWB_BASE + 0x001B)
#define VISP_CID_AWB_LIGHT_WEIGHT (VISP_CID_AWB_BASE + 0x001C)
#define VISP_CID_AWB_GRAY_PREFER_ENABLE (VISP_CID_AWB_BASE + 0x001D)
#define VISP_CID_AWB_GRAY_PREFER_BRIGHT (VISP_CID_AWB_BASE + 0x001E)
#define VISP_CID_AWB_GRAY_PREFER_R (VISP_CID_AWB_BASE + 0x001F)
#define VISP_CID_AWB_GRAY_PREFER_B (VISP_CID_AWB_BASE + 0x0020)
#define VISP_CID_AWB_FRONT_GROUND_FACE_ENABLE (VISP_CID_AWB_BASE + 0x0021)
#define VISP_CID_AWB_FRONT_GROUND_ROI_NUM (VISP_CID_AWB_BASE + 0x0022)
#define VISP_CID_AWB_FRONT_GROUND_FACE_WEIGHT (VISP_CID_AWB_BASE + 0x0023)
#define VISP_CID_AWB_FRONT_GROUND_RG (VISP_CID_AWB_BASE + 0x0024)
#define VISP_CID_AWB_FRONT_GROUND_BG (VISP_CID_AWB_BASE + 0x0025)
#define VISP_CID_AWB_USE_MANU_DAMP_COEFF (VISP_CID_AWB_BASE + 0x0026)
#define VISP_CID_AWB_MANU_DAMP_COEFF (VISP_CID_AWB_BASE + 0x0027)
#define VISP_CID_AWB_PERFORMANCE_OPTI_MODE (VISP_CID_AWB_BASE + 0x0028)
#define VISP_CID_AWB_LOCK_THRESH (VISP_CID_AWB_BASE + 0x0029)
#define VISP_CID_AWB_UNLOCK_THRESH (VISP_CID_AWB_BASE + 0x002A)

#define VISP_CID_AWB_ALL_CONFIG (VISP_CID_AWB_BASE + 0x0040)
#define VISP_CID_AWB_ALL_ROI (VISP_CID_AWB_BASE + 0x0041)
#define VISP_CID_AWB_ALL_FRONT_GROUND_CONFIG (VISP_CID_AWB_BASE + 0x0042)
#define VISP_CID_AWB_ALL_STATUS (VISP_CID_AWB_BASE + 0x0043)

#ifdef __KERNEL__
int visp_awb_ctrl_count(void);
int visp_awb_ctrl_create(struct visp_dev *isp_dev);
#endif

#endif