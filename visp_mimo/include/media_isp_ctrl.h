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

#ifndef __MEDIA_ISP_CTRL_H__
#define __MEDIA_ISP_CTRL_H__

#include <linux/videodev2.h>

#define ISP_CID_CTRL          (V4L2_CID_USER_BASE + 0x2000)
#define ISP_CID_AE_BASE       (V4L2_CID_USER_BASE + 0x2000)
#define ISP_CID_AWB_BASE      (V4L2_CID_USER_BASE + 0x2100)
#define ISP_CID_GC_BASE       (V4L2_CID_USER_BASE + 0x2200)
#define ISP_CID_2DNR_BASE     (V4L2_CID_USER_BASE + 0x2300)
#define ISP_CID_3DNR_BASE     (V4L2_CID_USER_BASE + 0x2400)
#define ISP_CID_BLS_BASE      (V4L2_CID_USER_BASE + 0x2500)
#define ISP_CID_CCM_BASE      (V4L2_CID_USER_BASE + 0x2600)
#define ISP_CID_CNR_BASE      (V4L2_CID_USER_BASE + 0x2700)
#define ISP_CID_CPD_BASE      (V4L2_CID_USER_BASE + 0x2800)
#define ISP_CID_CPROC_BASE    (V4L2_CID_USER_BASE + 0x2900)
#define ISP_CID_DG_BASE       (V4L2_CID_USER_BASE + 0x2A00)
#define ISP_CID_DMSC_BASE     (V4L2_CID_USER_BASE + 0x2B00)
#define ISP_CID_DPCC_BASE     (V4L2_CID_USER_BASE + 0x2C00)
#define ISP_CID_DPF_BASE      (V4L2_CID_USER_BASE + 0x2D00)
#define ISP_CID_EE_BASE       (V4L2_CID_USER_BASE + 0x2E00)
#define ISP_CID_GE_BASE       (V4L2_CID_USER_BASE + 0x2F00)
#define ISP_CID_GTM_BASE      (V4L2_CID_USER_BASE + 0x3000)
#define ISP_CID_HDR_BASE      (V4L2_CID_USER_BASE + 0x3100)
#define ISP_CID_LSC_BASE      (V4L2_CID_USER_BASE + 0x3200)
#define ISP_CID_LUT3D_BASE    (V4L2_CID_USER_BASE + 0x3300)
#define ISP_CID_PDAF_BASE     (V4L2_CID_USER_BASE + 0x3400)
#define ISP_CID_AF_BASE       (V4L2_CID_USER_BASE + 0x3500)
#define ISP_CID_RGBIR_BASE    (V4L2_CID_USER_BASE + 0x3600)
#define ISP_CID_WB_BASE       (V4L2_CID_USER_BASE + 0x3700)
#define ISP_CID_WDR_BASE      (V4L2_CID_USER_BASE + 0x3800)
#define ISP_CID_YNR_BASE      (V4L2_CID_USER_BASE + 0x3900)
#define ISP_CID_AFM_BASE      (V4L2_CID_USER_BASE + 0x3A00)
#define ISP_CID_EXP_BASE      (V4L2_CID_USER_BASE + 0x3B00)
#define ISP_CID_GWDR_BASE     (V4L2_CID_USER_BASE + 0x3C00)
#define ISP_CID_EXP_V3_BASE   (V4L2_CID_USER_BASE + 0x3D00)

#define ISP_CID_SENSOR_BASE   (V4L2_CID_USER_BASE + 0x3E00)

#define ISP_FLOAT_TRANS_STEP_COMMON         100
#define ISP_FLOAT_TRANS_STEP_COMMON_AWB     10000
#define ISP_FLOAT_TRANS_STEP_COMMON_HDR     10000
#define ISP_FLOAT_TRANS_STEP_GAIN           1024
#define ISP_FLOAT_TRANS_STEP_GAIN_WB        256
#define ISP_FLOAT_TRANS_STEP_TIME           1000000
#define ISP_FLOAT_TRANS_STEP_SHIFT_CAC      16
#define ISP_FLOAT_TRANS_STEP_HUE_CPROC      1
#define ISP_FLOAT_TRANS_STEP_CONTRAST_CPROC 128
#define ISP_FLOAT_TRANS_STEP_BRIGHT_CPROC   1
#define ISP_FLOAT_TRANS_STEP_SATURA_CPROC   128
#define ISP_FLOAT_TRANS_STEP_THRESH_GE      128
#define ISP_FLOAT_TRANS_STEP_SIGMA_2DNR     10
#define ISP_FLOAT_TRANS_STEP_VST_2DNR       10
#define ISP_FLOAT_TRANS_STEP_BLEND_2DNR     10
#define ISP_FLOAT_TRANS_STEP_NOISE_3DNR     10
#define ISP_FLOAT_TRANS_STEP_MOTION_3DNR    10
#define ISP_FLOAT_TRANS_STEP_VST_3DNR       10
#define ISP_FLOAT_TRANS_STEP_DILATE_3DNR    1
#define ISP_FLOAT_TRANS_STEP_FILTER_3DNR    10
#define ISP_FLOAT_TRANS_STEP_LEVEL_3DNR     1
#define ISP_FLOAT_TRANS_STEP_WEIGHT_3DNR    1
#define ISP_FLOAT_TRANS_STEP_RANGE_3DNR     1
#define ISP_FLOAT_TRANS_STEP_THR_MOT_3DNR   1
#define ISP_FLOAT_TRANS_STEP_BLEND_3DNR     10
#define ISP_FLOAT_TRANS_STEP_SIGMA_3DNR     10
#define ISP_FLOAT_TRANS_STEP_THRESH_AF      1000
#define ISP_FLOAT_TRANS_STEP_CONF_PDAF      10
#define ISP_FLOAT_TRANS_STEP_SHIFT_PDAF     10
#define ISP_FLOAT_TRANS_STEP_BWCOR_GTM      10
#define ISP_FLOAT_TRANS_STEP_LOG_GTM        10
#define ISP_FLOAT_TRANS_STEP_HIST_GTM       10
#define ISP_FLOAT_TRANS_STEP_HIST_HLC_FAC_GTM   100
#define ISP_FLOAT_TRANS_STEP_HIST_COMP_WT_GTM   100
#define ISP_FLOAT_TRANS_STEP_HIST_PRES_WT_GTM   100
#define ISP_FLOAT_TRANS_STEP_HIST_MIN_GAIN_GTM  100
#define ISP_FLOAT_TRANS_STEP_HIST_MAX_GAIN_GTM  100
#define ISP_FLOAT_TRANS_STEP_HIST_STRENGTH_GTM  100
#define ISP_FLOAT_TRANS_STEP_HIST_DAMP_COEF_GTM 100
#define ISP_FLOAT_TRANS_STEP_DEGAMMA_EE     100
#define ISP_FLOAT_TRANS_STEP_GAIN_DPF       10
#define ISP_FLOAT_TRANS_STEP_GRADIENT_DPF   10
#define ISP_FLOAT_TRANS_STEP_OFFSET_DPF     10
#define ISP_FLOAT_TRANS_STEP_MIN_DPF        10
#define ISP_FLOAT_TRANS_STEP_DIV_DPF        10
#define ISP_FLOAT_TRANS_SIGMA_DIV_DPF       10
#define ISP_FLOAT_TRANS_STEP_GAMMA_VAL_AFM  100
#define ISP_FLOAT_TRANS_STEP_IIR_HIGH_PASS_WEIGHT_AFM  16
#define ISP_FLOAT_TRANS_STEP_IIR_LOW_PASS_WEIGHT_AFM   16
#define ISP_FLOAT_TRANS_STEP_FIR_HIGH_PASS_WEIGHT_AFM  16
#define ISP_FLOAT_TRANS_STEP_FIR_LOW_PASS_WEIGHT_AFM   16
#define ISP_FLOAT_TRANS_STEP_AE_EXP_EXLIGHT            1000

#ifndef MAX
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#endif

int MediaIspFloatToInt(float src, int step);
uint32_t MediaIspFloatToUInt(float src, uint32_t step);
float MediaIspIntToFloat(int src, int step);
float MediaIspUIntToFloat(uint32_t src, uint32_t step);
int MediaIspFloatCompare(float a, float b);

#endif