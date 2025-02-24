/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
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
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
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

#ifndef __VISP_DPCC_H__
#define __VISP_DPCC_H__

#include "visp_ctrl.h"

#define VISP_CID_DPCC_ENABLE (VISP_CID_DPCC_BASE + 0x0000)
#define VISP_CID_DPCC_RESET (VISP_CID_DPCC_BASE + 0x0001)
#define VISP_CID_DPCC_MODE (VISP_CID_DPCC_BASE + 0x0002)

#define VISP_CID_DPCC_AUTO_MODE_SELECT (VISP_CID_DPCC_BASE + 0x0003)

#define VISP_CID_DPCC_MANU_BPT_ENABLE (VISP_CID_DPCC_BASE + 0x000F)
#define VISP_CID_DPCC_MANU_BPT_NUM (VISP_CID_DPCC_BASE + 0x0010)
#define VISP_CID_DPCC_MANU_BPT_OUT_MODE (VISP_CID_DPCC_BASE + 0x0011)
#define VISP_CID_DPCC_MANU_OUT_MODE (VISP_CID_DPCC_BASE + 0x0012)
#define VISP_CID_DPCC_MANU_SET_USE (VISP_CID_DPCC_BASE + 0x0013)
#define VISP_CID_DPCC_MANU_BPT_POS_X (VISP_CID_DPCC_BASE + 0x0014)
#define VISP_CID_DPCC_MANU_BPT_POS_Y (VISP_CID_DPCC_BASE + 0x0015)
#define VISP_CID_DPCC_MANU_METHODS_SET (VISP_CID_DPCC_BASE + 0x0016)
#define VISP_CID_DPCC_MANU_LINE_MAD_FACTOR (VISP_CID_DPCC_BASE + 0x0017)
#define VISP_CID_DPCC_MANU_LINE_THRESHOLD (VISP_CID_DPCC_BASE + 0x0018)
#define VISP_CID_DPCC_MANU_PG_FACTOR (VISP_CID_DPCC_BASE + 0x0019)
#define VISP_CID_DPCC_MANU_RG_FACTOR (VISP_CID_DPCC_BASE + 0x001A)
#define VISP_CID_DPCC_MANU_RND_OFFSETS (VISP_CID_DPCC_BASE + 0x001B)
#define VISP_CID_DPCC_MANU_RND_THRESHOLD (VISP_CID_DPCC_BASE + 0x001C)
#define VISP_CID_DPCC_MANU_RO_LIMITS (VISP_CID_DPCC_BASE + 0x001D)
#define VISP_CID_DPCC_STAT_BPT_ENABLE (VISP_CID_DPCC_BASE + 0x001E)
#define VISP_CID_DPCC_STAT_BPT_NUM (VISP_CID_DPCC_BASE + 0x001F)
#define VISP_CID_DPCC_STAT_BPT_OUT_MODE (VISP_CID_DPCC_BASE + 0x0020)
#define VISP_CID_DPCC_STAT_OUT_MODE (VISP_CID_DPCC_BASE + 0x0021)
#define VISP_CID_DPCC_STAT_SET_USE (VISP_CID_DPCC_BASE + 0x0022)
#define VISP_CID_DPCC_STAT_BPT_POS_X (VISP_CID_DPCC_BASE + 0x0023)
#define VISP_CID_DPCC_STAT_BPT_POS_Y (VISP_CID_DPCC_BASE + 0x0024)
#define VISP_CID_DPCC_STAT_METHODS_SET (VISP_CID_DPCC_BASE + 0x0025)
#define VISP_CID_DPCC_STAT_LINE_MAD_FACTOR (VISP_CID_DPCC_BASE + 0x0026)
#define VISP_CID_DPCC_STAT_LINE_THRESHOLD (VISP_CID_DPCC_BASE + 0x0027)
#define VISP_CID_DPCC_STAT_PG_FACTOR (VISP_CID_DPCC_BASE + 0x0028)
#define VISP_CID_DPCC_STAT_RG_FACTOR (VISP_CID_DPCC_BASE + 0x0029)
#define VISP_CID_DPCC_STAT_RND_OFFSETS (VISP_CID_DPCC_BASE + 0x002A)
#define VISP_CID_DPCC_STAT_RND_THRESHOLD (VISP_CID_DPCC_BASE + 0x002B)
#define VISP_CID_DPCC_STAT_RO_LIMITS (VISP_CID_DPCC_BASE + 0x002C)

#define VISP_CID_DPCC_ALL_CONFIG (VISP_CID_DPCC_BASE + 0x0030)
#define VISP_CID_DPCC_ALL_STATUS (VISP_CID_DPCC_BASE + 0x0031)

#ifdef __KERNEL__
int visp_dpcc_ctrl_count(void);
int visp_dpcc_ctrl_create(struct visp_dev *isp_dev);
#endif

#endif
