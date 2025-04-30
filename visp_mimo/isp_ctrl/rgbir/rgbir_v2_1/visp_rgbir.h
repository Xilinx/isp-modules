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

#ifndef __VISP_RGBIR_H__
#define __VISP_RGBIR_H__

#include "visp_ctrl.h"

#define VISP_CID_RGBIR_ENABLE          (VISP_CID_RGBIR_BASE + 0x0000)
#define VISP_CID_RGBIR_RCCC_ENABLE     (VISP_CID_RGBIR_BASE + 0x0001)
#define VISP_CID_RGBIR_IR_RAW_OUT_ENABLE \
                                            (VISP_CID_RGBIR_BASE + 0x0002)
#define VISP_CID_RGBIR_RESET           (VISP_CID_RGBIR_BASE + 0x0003)
#define VISP_CID_RGBIR_CC_MATRIX       (VISP_CID_RGBIR_BASE + 0x0004)
#define VISP_CID_RGBIR_DPCC_MIDDLE_THRESHOLD \
                                            (VISP_CID_RGBIR_BASE + 0x0005)
#define VISP_CID_RGBIR_DPCC_THRESHOLD  (VISP_CID_RGBIR_BASE + 0x0006)
#define VISP_CID_RGBIR_IR_THRESHOLD    (VISP_CID_RGBIR_BASE + 0x0007)
#define VISP_CID_RGBIR_L_THRESHOLD     (VISP_CID_RGBIR_BASE + 0x0008)
#define VISP_CID_RGBIR_OUT_PATTERN     (VISP_CID_RGBIR_BASE + 0x0009)
#define VISP_CID_RGBIR_IR_PATH_SELECT  (VISP_CID_RGBIR_BASE + 0x000A)
#define VISP_CID_RGBIR_STAT_CC_MATRIX  (VISP_CID_RGBIR_BASE + 0x000B)
#define VISP_CID_RGBIR_STAT_DPCC_MIDDLE_THRESHOLD \
                                            (VISP_CID_RGBIR_BASE + 0x000C)
#define VISP_CID_RGBIR_STAT_DPCC_THRESHOLD \
                                            (VISP_CID_RGBIR_BASE + 0x000D)
#define VISP_CID_RGBIR_STAT_IR_THRESHOLD \
                                            (VISP_CID_RGBIR_BASE + 0x000E)
#define VISP_CID_RGBIR_STAT_L_THRESHOLD \
                                            (VISP_CID_RGBIR_BASE + 0x000F)

#define VISP_CID_RGBIR_ALL_CONFIG      (VISP_CID_RGBIR_BASE + 0x0020)
#define VISP_CID_RGBIR_ALL_STATUS      (VISP_CID_RGBIR_BASE + 0x0021)

#ifdef __KERNEL__
int visp_rgbir_ctrl_count(void);
int visp_rgbir_ctrl_create(struct visp_dev *isp_dev);
#endif

#endif
