// SPDX-License-Identifier: MIT
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

#include <linux/module.h>
#include <linux/of_graph.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/v4l2-mediabus.h>
#include <media/videobuf2-dma-contig.h>

#include "visp_ctrl.h"
#include "visp_driver.h"
#include "visp_event.h"
#include "visp_procfs.h"
#include "visp_v4l2_common.h"
#include "visp_v4l2_std_exts.h"

#include "sensor_cmd.h"
#include "mbox_api.h"
#include "mbox_cmd.h"
#include "visp_app.h"
#include "visp_common.h"
#include "visp_mbox_driver.h"
#include "oba.h"

#define VISP_DEFAULT_SENSOR "ox03f10"
#define VISP_DEFAULT_SENSOR_MODE 0
#define VISP_DEFAULT_SENSOR_CALIB "/usr/share/limo_example_jsons/OX03f10.json"
#define VISP_DEFAULT_SENSOR_MANU_JSON "/usr/share/limo_example_jsons/manual_ext.json"
#define VISP_DEFAULT_SENSOR_AUTO_JSON "/usr/share/limo_example_jsons/auto.json"

static uint32_t sensor_dev_id[VISP_PORT_NR] = {2, 6, 5, 10}; // Sesnor

#define ISP_DEV_EXTENDED(isp_dev) \
((struct visp_lilo_isp_dev_extended *)((isp_dev)->extended_struct))

struct visp_format visp_mp_fmts[] = {
	{
	.fourcc = V4L2_PIX_FMT_NV16,
	.code = MEDIA_BUS_FMT_UYVY8_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_NV12,
	.code = MEDIA_BUS_FMT_VYYUYY8_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_YUYV,
	.code = MEDIA_BUS_FMT_YUYV8_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR8,
	.code = MEDIA_BUS_FMT_SBGGR8_1X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG8,
	.code = MEDIA_BUS_FMT_SGBRG8_1X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG8,
	.code = MEDIA_BUS_FMT_SGRBG8_1X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB8,
	.code = MEDIA_BUS_FMT_SRGGB8_1X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR10,
	.code = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG10,
	.code = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG10,
	.code = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB10,
	.code = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR12,
	.code = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG12,
	.code = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG12,
	.code = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB12,
	.code = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_P010,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_GREY,
	.code = MEDIA_BUS_FMT_Y8_1X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_Y10BPACK,
	.code = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_Y10DWA,
	.code = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_Y10,
	.code = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_P00BPACK,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_P00DWA,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	// {
	//	 .fourcc	= V4L2_PIX_FMT_P02BPACK,
	//	 .code	  = MEDIA_BUS_FMT_YUYV12_2X12,
	// },
	{
	.fourcc = V4L2_PIX_FMT_P20BPACK,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_P20DWA,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_P210,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	// {
	//	 .fourcc	= V4L2_PIX_FMT_P22BPACK,
	//	 .code	  = MEDIA_BUS_FMT_YUYV12_2X12,
	// },
	{
	.fourcc = V4L2_PIX_FMT_I20BPACK,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_I210,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_M48BPACK,
	.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_I48BPACK,
	.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_I48DWA,
	.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_I40DWA,
	.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_RGB24,
	.code = MEDIA_BUS_FMT_RBG888_1X24,   //RBG
	},
	{
	.fourcc = V4L2_PIX_FMT_RGB24DWA,
	.code = MEDIA_BUS_FMT_RGB888_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_RGB24P,
	.code = MEDIA_BUS_FMT_RGB888_3X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR10BPACK,
	.code = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG10BPACK,
	.code = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG10BPACK,
	.code = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB10BPACK,
	.code = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR10DWA,
	.code = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG10DWA,
	.code = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG10DWA,
	.code = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB10DWA,
	.code = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR12BPACK,
	.code = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG12BPACK,
	.code = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG12BPACK,
	.code = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB12BPACK,
	.code = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR12DWA,
	.code = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG12DWA,
	.code = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG12DWA,
	.code = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB12DWA,
	.code = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR14BPACK,
	.code = MEDIA_BUS_FMT_SBGGR14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG14BPACK,
	.code = MEDIA_BUS_FMT_SGBRG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG14BPACK,
	.code = MEDIA_BUS_FMT_SGRBG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB14BPACK,
	.code = MEDIA_BUS_FMT_SRGGB14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR14DWA,
	.code = MEDIA_BUS_FMT_SBGGR14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG14DWA,
	.code = MEDIA_BUS_FMT_SGBRG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG14DWA,
	.code = MEDIA_BUS_FMT_SGRBG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB14DWA,
	.code = MEDIA_BUS_FMT_SRGGB14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR14,
	.code = MEDIA_BUS_FMT_SBGGR14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG14,
	.code = MEDIA_BUS_FMT_SGBRG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG14,
	.code = MEDIA_BUS_FMT_SGRBG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB14,
	.code = MEDIA_BUS_FMT_SRGGB14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR16,
	.code = MEDIA_BUS_FMT_SBGGR16_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG16,
	.code = MEDIA_BUS_FMT_SGBRG16_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG16,
	.code = MEDIA_BUS_FMT_SGRBG16_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB16,
	.code = MEDIA_BUS_FMT_SRGGB16_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR24,
	.code = MEDIA_BUS_FMT_SBGGR24_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG24,
	.code = MEDIA_BUS_FMT_SGBRG24_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG24,
	.code = MEDIA_BUS_FMT_SGRBG24_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB24,
	.code = MEDIA_BUS_FMT_SRGGB24_1X24,
	},
};

struct visp_format visp_sp_fmts[] = {
	{
	.fourcc = V4L2_PIX_FMT_NV16,
	.code = MEDIA_BUS_FMT_UYVY8_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_NV12,
	.code = MEDIA_BUS_FMT_VYYUYY8_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_YUYV,
	.code = MEDIA_BUS_FMT_YUYV8_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_P010,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_GREY,
	.code = MEDIA_BUS_FMT_Y8_1X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_Y10BPACK,
	.code = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_Y10DWA,
	.code = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_Y10,
	.code = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_P00BPACK,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_P00DWA,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	// {
	//	 .fourcc	= V4L2_PIX_FMT_P02BPACK,
	//	 .code	  = MEDIA_BUS_FMT_YUYV12_2X12,
	// },
	{
	.fourcc = V4L2_PIX_FMT_P20BPACK,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_P20DWA,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_P210,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	// {
	//	 .fourcc	= V4L2_PIX_FMT_P22BPACK,
	//	 .code	  = MEDIA_BUS_FMT_YUYV12_2X12,
	// },
	{
	.fourcc = V4L2_PIX_FMT_I210,
	.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_M48BPACK,
	.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_I48BPACK,
	.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_I48DWA,
	.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_I40DWA,
	.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_RGB24,
	.code = MEDIA_BUS_FMT_RBG888_1X24,   //RBG
	},
	{
	.fourcc = V4L2_PIX_FMT_RGB24DWA,
	.code = MEDIA_BUS_FMT_RGB888_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_RGB24P,
	.code = MEDIA_BUS_FMT_RGB888_3X8,
	},
};

// main path
struct visp_format visp_raw_fmts[] = {
	{
	.fourcc = V4L2_PIX_FMT_SBGGR8,
	.code = MEDIA_BUS_FMT_SBGGR8_1X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG8,
	.code = MEDIA_BUS_FMT_SGBRG8_1X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG8,
	.code = MEDIA_BUS_FMT_SGRBG8_1X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB8,
	.code = MEDIA_BUS_FMT_SRGGB8_1X8,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR10,
	.code = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG10,
	.code = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG10,
	.code = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB10,
	.code = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR12,
	.code = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG12,
	.code = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG12,
	.code = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB12,
	.code = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR10BPACK,
	.code = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG10BPACK,
	.code = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG10BPACK,
	.code = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB10BPACK,
	.code = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR10DWA,
	.code = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG10DWA,
	.code = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG10DWA,
	.code = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB10DWA,
	.code = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR12BPACK,
	.code = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG12BPACK,
	.code = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG12BPACK,
	.code = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB12BPACK,
	.code = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR12DWA,
	.code = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG12DWA,
	.code = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG12DWA,
	.code = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB12DWA,
	.code = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR14BPACK,
	.code = MEDIA_BUS_FMT_SBGGR14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG14BPACK,
	.code = MEDIA_BUS_FMT_SGBRG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG14BPACK,
	.code = MEDIA_BUS_FMT_SGRBG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB14BPACK,
	.code = MEDIA_BUS_FMT_SRGGB14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR14DWA,
	.code = MEDIA_BUS_FMT_SBGGR14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG14DWA,
	.code = MEDIA_BUS_FMT_SGBRG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG14DWA,
	.code = MEDIA_BUS_FMT_SGRBG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB14DWA,
	.code = MEDIA_BUS_FMT_SRGGB14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR14,
	.code = MEDIA_BUS_FMT_SBGGR14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG14,
	.code = MEDIA_BUS_FMT_SGBRG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG14,
	.code = MEDIA_BUS_FMT_SGRBG14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB14,
	.code = MEDIA_BUS_FMT_SRGGB14_1X14,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR16,
	.code = MEDIA_BUS_FMT_SBGGR16_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG16,
	.code = MEDIA_BUS_FMT_SGBRG16_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG16,
	.code = MEDIA_BUS_FMT_SGRBG16_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB16,
	.code = MEDIA_BUS_FMT_SRGGB16_1X16,
	},
	{
	.fourcc = V4L2_PIX_FMT_SBGGR24,
	.code = MEDIA_BUS_FMT_SBGGR24_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGBRG24,
	.code = MEDIA_BUS_FMT_SGBRG24_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_SGRBG24,
	.code = MEDIA_BUS_FMT_SGRBG24_1X24,
	},
	{
	.fourcc = V4L2_PIX_FMT_SRGGB24,
	.code = MEDIA_BUS_FMT_SRGGB24_1X24,
	},

};

static int visp_querycap(struct v4l2_subdev *sd, void *arg)
{
	struct v4l2_capability *cap = (struct v4l2_capability *)arg;

	strncpy(cap->driver, sd->name, sizeof(cap->driver));
	cap->driver[sizeof(cap->driver) - 1] = '\0';
	strncpy(cap->card, sd->name, sizeof(cap->card));
	cap->card[sizeof(cap->card) - 1] = '\0';
	snprintf(cap->bus_info, sizeof(cap->bus_info), "platform:%s", sd->name);

	return 0;
}

static int visp_pad_requbufs(struct v4l2_subdev *sd, void *arg)
{
	struct visp_pad_reqbufs *pad_requbufs = (struct visp_pad_reqbufs *)arg;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	int status = 0;

	int port = pad_requbufs->pad / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad_requbufs->pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

	isp_dev->isp_ports[port].isp_chns[chn].num_bufs =
		pad_requbufs->num_buffers;
	return status;
}

static int visp_pad_buf_queue(struct v4l2_subdev *sd, void *arg)
{
	struct visp_pad_buf *pad_buf = (struct visp_pad_buf *)arg;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	int ret_val;
	unsigned long flags;
	struct visp_pad_data *cur_pad;
	int i = 0;
	media_buf buf;
	int port = pad_buf->pad / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad_buf->pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	media_isp_chn_attr *IspChn = &isp_port->isp_chns[chn];

	cur_pad = &isp_dev->pad_data[pad_buf->pad];

	spin_lock_irqsave(&cur_pad->qlock, flags);

	list_add_tail(&pad_buf->buf->list, &cur_pad->queue);

	spin_unlock_irqrestore(&cur_pad->qlock, flags);

	buf.index = pad_buf->buf->sequence;
	buf.num_planes = pad_buf->buf->num_planes;

	for (i = 0; i < buf.num_planes; i++) {
		buf.planes[i].dma_addr = pad_buf->buf->planes[i].dma_addr;
		buf.planes[i].dma_size = pad_buf->buf->planes[i].size;
	}
	// ENQUE BUFFER
	if (isp_dev->streamon[pad_buf->pad] == 0) {
		memcpy(&IspChn->bufs[buf.index], &buf, sizeof(media_buf));
	} else {
		output_buffer_t *p_media_buffer = VSI_NULL;

		p_media_buffer = IspChn->cam_device_bufs[buf.index];

		if (p_media_buffer == VSI_NULL) {
			dev_err(isp_dev->dev, "CamDevice queue buf is null\n");
			return VSI_ERR_NULL_PTR;
		}

		ret_val = vsi_cam_device_en_que_buffer(
			isp_dev, isp_port->cam_device_handle, chn, p_media_buffer);
		if (ret_val != VSI_SUCCESS) {
			dev_err(isp_dev->dev,
				"CamDevice queue buf failed, ret is %d\n",
				ret_val);
			return VSI_ERR_TIMEOUT;
		}
	}

	return ret_val;
}

int media_isp_hal_buf_done(struct v4l2_subdev *sd, int pad,
			   const media_buf *buf);

int media_isp_device_dqbuf(struct visp_dev *isp_dev, struct Chn_info *info,
			   media_buf *buf, void *enque_buff_g,
			   output_buffer_t *output_buffer);

static int handle_frameout_buffer(struct visp_dev *isp_dev, int port, mbox_post_msg *msg)
{
	output_buffer_t *output_buffer = NULL;
	struct Chn_info info;
	uint8_t buf_index;
	int pad = -1;
	int ret_val = 0;

	/* Validate inputs */
	if (!isp_dev) {
		dev_err(isp_dev->dev,
			"handle_frameout_buffer: isp_dev is NULL\n");
		return -EINVAL;
	}

	if (port < 0 || port >= isp_dev->num_streams) {
		dev_err(isp_dev->dev,
			"%s: Invalid port %d (must be 0-%d)\n",
			__func__, port, isp_dev->num_streams - 1);
		return -EINVAL;
	}

	if (!msg) {
		dev_err(isp_dev->dev,
			"%s: msg is NULL\n", __func__);
		return -EINVAL;
	}

	/* Dequeue buffer from the ISP device - pass payload directly */
	read_dq_buf_info(msg->payload, isp_dev, &info, &buf_index);

	/* Validate buffer index to detect RPU reusing stale indices */
	if (buf_index >= 32) {
		dev_err(isp_dev->dev,
			"%s: Invalid buf_index %u (max 31) - RPU may be reusing stale buffer\n",
			__func__,
			buf_index);
		ret_val = -EINVAL;
		goto error_free_buf;
	}

	/* Lock to read cam_device_bufs - protects against concurrent p_owner
	 * writes
	 */
	mutex_lock(&isp_dev->isp_ports[info.vt_id]
		   .isp_chns[info.path]
		   .cam_device_bufs_lock);
	output_buffer = isp_dev->isp_ports[info.vt_id]
			    .isp_chns[info.path]
			    .cam_device_bufs[buf_index];
	mutex_unlock(&isp_dev->isp_ports[info.vt_id]
		     .isp_chns[info.path]
		     .cam_device_bufs_lock);
	if (!output_buffer) {
		dev_warn(isp_dev->dev,
			 "%s: Outputbuffer is NULL for buf_index %u - RPU likely using "
			 "stale index due to missed enqueue\n",
			 __func__,
			buf_index);
		ret_val = -EINVAL;
		goto error_free_buf;
	}

	/* Calculate the pad index */
	pad = (info.vt_id * MEDIA_ISP_PORT_PAD_COUNT) + (info.path + 1);
	if (pad <= 0) {
		dev_err(isp_dev->dev,
			"%s: Invalid pad value %d\n", __func__, pad);
		ret_val = -EINVAL;
		goto error_free_buf;
	}

	/* Mark buffer as done*/
	ret_val = media_isp_hal_buf_done(&isp_dev->sd, pad,
					 &(isp_dev->isp_ports[info.vt_id]
					 .isp_chns[info.path]
					 .bufs[output_buffer->index]));
	if (ret_val != 0) {
		dev_dbg(isp_dev->dev,
			"%s: MediaIspHalBufDone failed with error %d\n",
			__func__, ret_val);
		dev_dbg(
		    isp_dev->dev,
		    "Skip buf: ret_val=%d, ISP=%d, port=%d, chn=%d, BUF=0x%x\n",
		    ret_val, isp_dev->id, info.vt_id, info.path,
		    output_buffer ? output_buffer->base_address : 0);
		goto error_free_buf;
	}

	/* Free message after successful processing */
	visp_free_rx_buffer(isp_dev->rpu, msg);
	return 0;

error_free_buf:
	/* Free message in case of any error */
	visp_free_rx_buffer(isp_dev->rpu, msg);
	return ret_val;
}
// EXPORT_SYMBOL_GPL(handle_frameout_buffer);

//

/**
 * visp_get_input_subdev - Retrieve the remote sub-device connected to the ISP input pad.
 * @isp_dev: Pointer to the ISP device structure.
 *
 * This function iterates over all media pads of the ISP device and identifies the
 * input pad (MEDIA_PAD_FL_SINK). It then checks for a remote connection using
 * media_pad_remote_pad_first() (for kernel 6.0 and later) or media_entity_remote_pad().
 *
 * If a valid sub-device is connected to the input pad, it is returned. Otherwise,
 * the function logs an error and returns NULL.
 *
 * Return: Pointer to the v4l2_subdev structure if found, NULL otherwise.
 */

static struct v4l2_subdev *visp_get_input_subdev(struct visp_dev *isp_dev, int port)
{
    struct media_pad *remote_pad;
    struct v4l2_subdev *subdev;
    int pad;

    dev_dbg(isp_dev->dev, "Searching for input sub-device...\n");

    for (pad = port; pad < isp_dev->num_pads; pad++)
    {
        // Check if this pad is a SINK (input pad)
        if (!(isp_dev->pads[pad].flags & MEDIA_PAD_FL_SINK)) {
            dev_dbg(isp_dev->dev, "Pad %d is not a sink, skipping...\n", pad);
            continue;
        }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        remote_pad = media_pad_remote_pad_first(&isp_dev->pads[pad]);
#else
        remote_pad = media_entity_remote_pad(&isp_dev->pads[pad]);
#endif

        if (!remote_pad) {
            dev_dbg(isp_dev->dev, "Pad %d has no remote connection.\n", pad);
            continue;
        }

        if (!is_media_entity_v4l2_subdev(remote_pad->entity)) {
            dev_dbg(isp_dev->dev, "Pad %d remote entity is not a sub-device.\n", pad);
            continue;
        }

        subdev = media_entity_to_v4l2_subdev(remote_pad->entity);
        dev_info(isp_dev->dev, "Found input sub-device: %s on pad %d\n", subdev->name, pad);

        return subdev; // Return the first valid input sub-device found
}

    //dev_err(isp_dev->dev, "No input sub-device found!\n");
    return NULL;
}


static int visp_pad_s_stream(struct v4l2_subdev *sd, void *arg)
{
	struct visp_pad_stream_status *pad_stream =
		(struct visp_pad_stream_status *)arg;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	unsigned long flags;
	int ret = 0;
	int port = pad_stream->pad / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad_stream->pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;
	struct v4l2_subdev *subdev;


	if (pad_stream->status == 1) {// streamon

		isp_dev->pad_data[pad_stream->pad].stream = pad_stream->status;
		ret = visp_setup_isp_pipeline(isp_dev, pad_stream->pad);
		if (ret)
			return ret;

		/* Reset sequence counter on streamon */
		spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
		isp_dev->pad_data[pad_stream->pad].sequence = 0;
		spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock, flags);

		ret = media_isp_device_set_frame_rate(
			isp_dev, port,
			&isp_dev->isp_ports[port].sensor_info.frame_rate);
		if (ret != VSI_SUCCESS) {
			dev_err(isp_dev->dev,
				"%s isp:%d port %d chn %d Set frame_rate failed, ret is %d",
				__func__, isp_dev->id, port, chn, ret);
			goto ERR_TO_CAMERA_DISCONNECT;
		}

		subdev = visp_get_input_subdev(isp_dev, port);
		if (!subdev) {
			//dev_err(isp_dev->dev, "No valid input sub-device found!\n");
		}else{
			// call s_stream
			v4l2_subdev_call(subdev, video, s_stream, 1);
		}
		ret = visp_stream_on(isp_dev);
		if (ret != 0) {
			dev_err(isp_dev->dev, "visp_stream_on failed isp:%d port:%d\n",
				isp_dev->id, port);
			goto ERR_TO_CAMERA_DISCONNECT;
		}
	}

	else {
		/* streamoff */
		visp_stream_off(isp_dev);

		chn = 0;

		media_isp_device_camera_dis_connect(isp_dev, port, chn);

		isp_destroy_pipeline(isp_dev, port, chn);

		subdev = visp_get_input_subdev(isp_dev, port);
		if (!subdev) {
			dev_err(isp_dev->dev, "No valid input sub-device found!\n");
		} else {
			/* call s_stream */
			v4l2_subdev_call(subdev, video, s_stream, 0);
		}
		isp_dev->streamon[pad_stream->pad] = 0;
		isp_dev->pad_data[pad_stream->pad].stream = pad_stream->status;

		spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
		INIT_LIST_HEAD(&isp_dev->pad_data[pad_stream->pad].queue);
		/* Reset sequence counter on streamoff */
		isp_dev->pad_data[pad_stream->pad].sequence = 0;
		spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock, flags);

	}

	return ret;

ERR_TO_CAMERA_DISCONNECT:
	visp_stream_off(isp_dev);
	isp_dev->streamon[pad_stream->pad] = 0;
	isp_dev->pad_data[pad_stream->pad].stream = 0;
	return ret;
}
static int visp_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct visp_pad_stream_status pad_stream = {1, enable};

	visp_pad_s_stream(sd, &pad_stream);
	return 0;
}

int visp_buf_done(struct v4l2_subdev *sd, void *arg)
{
	struct visp_buf ubuf;
	struct visp_pad_data *cur_pad;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	unsigned long flags;
	struct visp_vb2_buffer *pos, *next;
	struct visp_vb2_buffer *buf = NULL;
	struct media_pad *pad;
	struct video_device *video;

	memcpy(&ubuf, arg, sizeof(struct visp_buf));
	cur_pad = &isp_dev->pad_data[ubuf.pad];

	spin_lock_irqsave(&cur_pad->qlock, flags);

	if (cur_pad->stream == 0) {
		int chn = (ubuf.pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

		dev_dbg(isp_dev->dev,
			"ISP : %d Chn : %d Pipeline in Streamoff state, buffer will be dropped\n",
			isp_dev->id, chn);
		spin_unlock_irqrestore(&cur_pad->qlock, flags);
		return -EINVAL;
	}

	if (list_empty(&cur_pad->queue)) {
		int chn = (ubuf.pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

		dev_dbg(isp_dev->dev,
			"ISP : %d Chn : %d buffer queue is empty, buffer will be dropped\n",
			isp_dev->id, chn);
		spin_unlock_irqrestore(&cur_pad->qlock, flags);
		return -EINVAL;
	}


	list_for_each_entry_safe(pos, next, &cur_pad->queue, list) {
		if (pos && (pos->sequence == ubuf.index)) {
			buf = pos;
			list_del(&pos->list);
			break;
		}
	}
	spin_unlock_irqrestore(&cur_pad->qlock, flags);

	if (buf) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
		pad = media_pad_remote_pad_first(&isp_dev->pads[ubuf.pad]);
#else
		pad = media_entity_remote_pad(&isp_dev->pads[ubuf.pad]);
#endif
		if (!pad)
			return -EINVAL;
		video = media_entity_to_video_device(pad->entity);
		if (buf->sequence < video->queue->max_num_buffers) {
			if (buf->vb.vb2_buf.state == VB2_BUF_STATE_ACTIVE) {
				/* Set timestamp and sequence before marking done */
				buf->vb.vb2_buf.timestamp = ktime_get_ns();

				spin_lock_irqsave(&cur_pad->qlock, flags);
				buf->vb.sequence = cur_pad->sequence++;
				spin_unlock_irqrestore(&cur_pad->qlock, flags);

				vb2_buffer_done(&buf->vb.vb2_buf,
						VB2_BUF_STATE_DONE);
			}
		}
	}

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 18, 0)
static int visp_queryctrl(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_queryctrl *pad_querctrl =
		(struct visp_pad_queryctrl *)arg;
	ret = v4l2_queryctrl(&isp_dev->ctrl_handler, pad_querctrl->query_ctrl);

	return ret;
}
#endif

static int visp_query_ext_ctrl(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_query_ext_ctrl *pad_quer_ext_ctrl =
		(struct visp_pad_query_ext_ctrl *)arg;
	ret = v4l2_query_ext_ctrl(&isp_dev->ctrl_handler,
				  pad_quer_ext_ctrl->query_ext_ctrl);

	return ret;
}

static int visp_querymenu(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_querymenu *pad_quermenu =
		(struct visp_pad_querymenu *)arg;
	ret = v4l2_querymenu(&isp_dev->ctrl_handler, pad_quermenu->querymenu);

	return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 18, 0)
static int visp_g_ctrl(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_control *pad_ctrl = (struct visp_pad_control *)arg;

	mutex_lock(&isp_dev->ctrl_lock);
	isp_dev->ctrl_pad = pad_ctrl->pad;
	ret = v4l2_g_ctrl(&isp_dev->ctrl_handler, pad_ctrl->control);
	mutex_unlock(&isp_dev->ctrl_lock);

	return ret;
}

static int visp_s_ctrl(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_control *pad_ctrl = (struct visp_pad_control *)arg;

	mutex_lock(&isp_dev->ctrl_lock);
	isp_dev->ctrl_pad = pad_ctrl->pad;
	ret = v4l2_s_ctrl(NULL, &isp_dev->ctrl_handler, pad_ctrl->control);
	mutex_unlock(&isp_dev->ctrl_lock);

	return ret;
}
#endif

static int visp_g_ext_ctrls(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_ext_controls *pad_ext_ctrls =
		(struct visp_pad_ext_controls *)arg;

	mutex_lock(&isp_dev->ctrl_lock);
	isp_dev->ctrl_pad = pad_ext_ctrls->pad;
	ret = v4l2_g_ext_ctrls(&isp_dev->ctrl_handler, sd->devnode,
				   sd->v4l2_dev->mdev, pad_ext_ctrls->ext_controls);
	mutex_unlock(&isp_dev->ctrl_lock);

	return ret;
}

static int visp_s_ext_ctrls(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_ext_controls *pad_ext_ctrls =
		(struct visp_pad_ext_controls *)arg;

	mutex_lock(&isp_dev->ctrl_lock);
	isp_dev->ctrl_pad = pad_ext_ctrls->pad;
	ret = v4l2_s_ext_ctrls(NULL, &isp_dev->ctrl_handler, sd->devnode,
				   sd->v4l2_dev->mdev, pad_ext_ctrls->ext_controls);
	mutex_unlock(&isp_dev->ctrl_lock);

	return ret;
}

static int visp_try_ext_ctrls(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_ext_controls *pad_ext_ctrls =
		(struct visp_pad_ext_controls *)arg;

	ret =
		v4l2_try_ext_ctrls(&isp_dev->ctrl_handler, sd->devnode,
				   sd->v4l2_dev->mdev, pad_ext_ctrls->ext_controls);

	return ret;
}

static int visp_buffer_alloc(struct v4l2_subdev *sd, void *arg)
{
	int ret = 0;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_ext_buf_info *ext_buf_info =
		(struct visp_ext_buf_info *)arg;
	struct visp_ext_dma_buf *ext_dma_buf = NULL;

	if (ext_buf_info->port >= VISP_PORT_NR) {
		dev_err(isp_dev->dev,
			"%s: invalid port number, range: 0 ~ %d\n", __func__,
			VISP_PORT_NR - 1);
		return -EINVAL;
	}

	ext_dma_buf = kzalloc(sizeof(struct visp_ext_dma_buf), GFP_KERNEL);
	if (NULL == ext_dma_buf) {
		dev_err(isp_dev->dev, "%s: buffer alloc kzalloc failed!\n",
			__func__);
		return -ENOMEM;
	}

	ext_dma_buf->vaddr =
		dma_alloc_coherent(isp_dev->dev, ext_buf_info->plane.size,
				   &(ext_dma_buf->addr), GFP_KERNEL);
	if (NULL == ext_dma_buf->vaddr) {
		dev_err(isp_dev->dev, "%s: failed to alloc dma buffer!\n",
			__func__);
		return -ENOMEM;
	}

	ext_dma_buf->size = ext_buf_info->plane.size;
	ext_buf_info->plane.dma_addr = (uint32_t)ext_dma_buf->addr;

	list_add_tail(&ext_dma_buf->entry,
			  &isp_dev->mcm_input[ext_buf_info->port]);

	return ret;
}

static int visp_buffer_free(struct v4l2_subdev *sd, void *arg)
{
	int ret = 0;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_ext_dma_buf *pos, *next;
	struct visp_ext_dma_buf *ext_dma_buf = NULL;
	struct visp_ext_buf_info *ext_buf_info =
		(struct visp_ext_buf_info *)arg;

	if (ext_buf_info->port >= VISP_PORT_NR) {
		dev_err(isp_dev->dev,
			"%s: invalid port number, range: 0 ~ %d\n", __func__,
			VISP_PORT_NR - 1);
		return -EINVAL;
	}

	list_for_each_entry_safe(
		pos, next, &isp_dev->mcm_input[ext_buf_info->port], entry) {
		if (pos && (pos->addr == ext_buf_info->plane.dma_addr)) {
			ext_dma_buf = pos;
			list_del(&pos->entry);
			break;
		}
	}

	if (ext_dma_buf) {
		dma_free_coherent(isp_dev->dev, ext_dma_buf->size,
				  ext_dma_buf->vaddr, ext_dma_buf->addr);
		kfree(ext_dma_buf);
	}

	return ret;
}

int visp_buffer_free_public_wrapper(struct visp_dev *isp_dev, void *arg);
int visp_buffer_free_public_wrapper(struct visp_dev *isp_dev, void *arg)
{
	int ret = visp_buffer_free(&isp_dev->sd, arg);
	return ret;
}

int visp_buffer_alloc_public(struct visp_dev *isp_dev,
				 struct visp_ext_buf_info *ext_buf_info);
int visp_buffer_alloc_public(struct visp_dev *isp_dev,
				 struct visp_ext_buf_info *ext_buf_info)
{
	int ret = 0;
	struct visp_ext_dma_buf *ext_dma_buf = NULL;

	if (ext_buf_info->port >= VISP_PORT_NR) {
		dev_err(isp_dev->dev,
			"%s: invalid port number, range: 0 ~ %d\n", __func__,
			VISP_PORT_NR - 1);
		return -EINVAL;
	}

	ext_dma_buf = kzalloc(sizeof(struct visp_ext_dma_buf), GFP_KERNEL);
	if (NULL == ext_dma_buf) {
		dev_err(isp_dev->dev, "%s: buffer alloc kzalloc failed!\n",
			__func__);
		return -ENOMEM;
	}

	ext_dma_buf->vaddr =
		dma_alloc_coherent(isp_dev->dev, ext_buf_info->plane.size,
				   &(ext_dma_buf->addr), GFP_KERNEL);
	if (NULL == ext_dma_buf->vaddr) {
		dev_err(isp_dev->dev, "%s: failed to alloc dma buffer!\n",
			__func__);
		return -ENOMEM;
	}

	ext_dma_buf->size = ext_buf_info->plane.size;
	ext_buf_info->plane.dma_addr = (uint32_t)ext_dma_buf->addr;

	list_add_tail(&ext_dma_buf->entry,
			  &isp_dev->mcm_input[ext_buf_info->port]);

	return ret;
}

int visp_buffer_free_public(struct visp_dev *isp_dev,
				struct visp_ext_buf_info *ext_buf_info);
int visp_buffer_free_public(struct visp_dev *isp_dev,
				struct visp_ext_buf_info *ext_buf_info)
{
	int ret = 0;
	struct visp_ext_dma_buf *pos, *next;
	struct visp_ext_dma_buf *ext_dma_buf = NULL;

	if (ext_buf_info->port >= VISP_PORT_NR) {
		dev_err(isp_dev->dev,
			"%s: invalid port number, range: 0 ~ %d\n", __func__,
			VISP_PORT_NR - 1);
		return -EINVAL;
	}

	list_for_each_entry_safe(
		pos, next, &isp_dev->mcm_input[ext_buf_info->port], entry) {
		if (pos && (pos->addr == ext_buf_info->plane.dma_addr)) {
			ext_dma_buf = pos;
			list_del(&pos->entry);
			break;
		}
	}

	if (ext_dma_buf) {
		dma_free_coherent(isp_dev->dev, ext_dma_buf->size,
				  ext_dma_buf->vaddr, ext_dma_buf->addr);
		dev_info(isp_dev->dev, "%s: dma_addr: 0x%x is free\n", __func__,
			 (uint32_t)ext_dma_buf->addr);
		kfree(ext_dma_buf);
	}

	return ret;
}

/* DMA-BUF attachment structure to track per-attachment state */
struct visp_dmabuf_attachment {
	struct sg_table *sgt;
	enum dma_data_direction dir;
};

static int visp_dmabuf_attach(struct dma_buf *dmabuf,
			      struct dma_buf_attachment *attachment)
{
	struct visp_dmabuf_attachment *visp_attach;

	/* Prevent module unload while dmabuf is attached/mapped */
	if (!try_module_get(THIS_MODULE))
		return -ENODEV;

	visp_attach = kzalloc(sizeof(*visp_attach), GFP_KERNEL);
	if (!visp_attach) {
		module_put(THIS_MODULE);
		return -ENOMEM;
	}

	visp_attach->dir = DMA_NONE;
	attachment->priv = visp_attach;
	return 0;
}

static void visp_dmabuf_detach(struct dma_buf *dmabuf,
			       struct dma_buf_attachment *attachment)
{
	struct visp_dmabuf_attachment *visp_attach = attachment->priv;

	if (visp_attach) {
		/* If mapped, unmap first */
		if (visp_attach->sgt && visp_attach->dir != DMA_NONE)
			dma_buf_unmap_attachment(attachment, visp_attach->sgt, visp_attach->dir);

		if (visp_attach->sgt) {
			sg_free_table(visp_attach->sgt);
			kfree(visp_attach->sgt);
		}
		kfree(visp_attach);
		attachment->priv = NULL;
	}

	/* Allow module unload after detach */
	module_put(THIS_MODULE);
}

static struct sg_table *visp_dmabuf_map(struct dma_buf_attachment *attachment,
					enum dma_data_direction dir)
{
	struct visp_event_shm *event_shm = attachment->dmabuf->priv;
	struct visp_dmabuf_attachment *visp_attach = attachment->priv;
	struct sg_table *sgt;
	struct page *page;
	int ret;

	if (!visp_attach)
		return ERR_PTR(-EINVAL);

	if (!event_shm || !event_shm->virt_addr) {
		pr_err("visp: dmabuf_map failed - event_shm invalid\n");
		return ERR_PTR(-EINVAL);
	}

	sgt = kzalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(sgt, 1, GFP_KERNEL);
	if (ret) {
		kfree(sgt);
		return ERR_PTR(ret);
	}

	page = virt_to_page(event_shm->virt_addr);
	sg_set_page(sgt->sgl, page, event_shm->size, 0);
	sg_dma_address(sgt->sgl) = event_shm->dma_handle;
	sg_dma_len(sgt->sgl) = event_shm->size;

	visp_attach->sgt = sgt;
	visp_attach->dir = dir;
	return sgt;
}

static void visp_dmabuf_unmap(struct dma_buf_attachment *attachment,
			      struct sg_table *sgt,
			      enum dma_data_direction dir)
{
	/* No-op: memory is DMA coherent, no sync needed */
}

static void visp_dmabuf_release(struct dma_buf *dmabuf)
{
	struct visp_event_shm *event_shm = dmabuf->priv;

	/* Clear dmabuf pointer so it can be re-created on next use */
	if (event_shm) {
		event_shm->dmabuf = NULL;
		dev_info(event_shm->dev, "DMA-BUF released, will re-export on next use\n");
	}

	/* Note: DMA memory is NOT freed here - it persists for re-use */
	/* DMA memory will be freed in visp_remove() when module unloads */
}

static int visp_dmabuf_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct visp_event_shm *event_shm = dmabuf->priv;

	if (!event_shm || !event_shm->virt_addr || !event_shm->dev) {
		pr_err("visp: dmabuf_mmap failed - event_shm invalid\n");
		return -EINVAL;
	}

	return dma_mmap_coherent(event_shm->dev, vma, event_shm->virt_addr,
				  event_shm->dma_handle, event_shm->size);
}

static const struct dma_buf_ops visp_dmabuf_ops = {
	.attach = visp_dmabuf_attach,
	.detach = visp_dmabuf_detach,
	.map_dma_buf = visp_dmabuf_map,
	.unmap_dma_buf = visp_dmabuf_unmap,
	.release = visp_dmabuf_release,
	.mmap = visp_dmabuf_mmap,
};

static int visp_export_dmabuf(struct device *dev, struct visp_event_shm *event_shm)
{
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct dma_buf *dmabuf;

	exp_info.ops = &visp_dmabuf_ops;
	exp_info.size = event_shm->size;
	exp_info.flags = O_RDWR | O_CLOEXEC;
	exp_info.priv = event_shm;

	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		dev_err(dev, "Failed to export DMA-BUF: %ld\n", PTR_ERR(dmabuf));
		return PTR_ERR(dmabuf);
	}

	event_shm->dmabuf = dmabuf;
	event_shm->dmabuf_fd = -1; /* fd created per-process via ioctl */

	dev_info(dev, "DMA-BUF exported successfully: virt=%p dma=%pad size=%d\n",
		 event_shm->virt_addr, &event_shm->dma_handle, event_shm->size);

	return 0;
}

static long visp_return_rpu_id(struct v4l2_subdev *sd, void *arg)
{
	long ret = 0;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);

	if (!arg)
		dev_err(isp_dev->dev, "%s %d NULL ARG\n", __func__, __LINE__);

	struct isp_rpu *temp = (struct isp_rpu *)arg;

	temp->rpu = isp_dev->isp_rpu;
	temp->isp = isp_dev->id;
	temp->io_mode = isp_dev->isp_mem;

	dev_dbg(isp_dev->dev, "%s %d returning RPU id: %d for ISP : %d\n",
		__func__, __LINE__, isp_dev->isp_rpu, isp_dev->id);

	return ret;
}

static long visp_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd,
				void *arg)
{
	int ret = -EINVAL;

	switch (cmd) {
	case VIDIOC_QUERYCAP:
		ret = visp_querycap(sd, arg);
		break;
	case VISP_PAD_REQUBUFS:
		ret = visp_pad_requbufs(sd, arg);
		break;
	case VISP_PAD_BUF_QUEUE:
		ret = visp_pad_buf_queue(sd, arg);
		break;
	case VISP_PAD_S_STREAM:
		ret = visp_pad_s_stream(sd, arg);
		break;
	case VISP_IOC_BUFDONE:
		ret = visp_buf_done(sd, arg);
		break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 18, 0)
	case VISP_PAD_QUERYCTRL:
		ret = visp_queryctrl(sd, arg);
		break;
#endif
	case VISP_PAD_QUERY_EXT_CTRL:
		ret = visp_query_ext_ctrl(sd, arg);
		break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 18, 0)
	case VISP_PAD_G_CTRL:
		ret = visp_g_ctrl(sd, arg);
		break;
	case VISP_PAD_S_CTRL:
		ret = visp_s_ctrl(sd, arg);
		break;
#endif
	case VISP_PAD_G_EXT_CTRLS:
		ret = visp_g_ext_ctrls(sd, arg);
		break;
	case VISP_PAD_S_EXT_CTRLS:
		ret = visp_s_ext_ctrls(sd, arg);
		break;
	case VISP_PAD_TRY_EXT_CTRLS:
		ret = visp_try_ext_ctrls(sd, arg);
		break;
	case VISP_PAD_QUERYMENU:
		ret = visp_querymenu(sd, arg);
		break;
	case VISP_IOC_BUFFER_ALLOC:
		ret = visp_buffer_alloc(sd, arg);
		break;
	case VISP_IOC_BUFFER_FREE:
		ret = visp_buffer_free(sd, arg);
		break;
	case VISP_GET_RPU_ID:
		ret = visp_return_rpu_id(sd, arg);
		break;
	case VISP_GET_EVENT_SHM_FD: {
		struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);

		if (!isp_dev->event_shm.virt_addr) {
			dev_err(isp_dev->event_shm.dev,
				"VISP_GET_EVENT_SHM_FD: event_shm memory not allocated\n");
			ret = -EINVAL;
			break;
		}

		/* Lazy creation: export DMA-BUF on first FD request */
		if (!isp_dev->event_shm.dmabuf) {
			ret = visp_export_dmabuf(isp_dev->event_shm.dev, &isp_dev->event_shm);
			if (ret) {
				dev_err(isp_dev->event_shm.dev,
					"VISP_GET_EVENT_SHM_FD: dmabuf export failed: %d\n", ret);
				break;
			}
			dev_info(isp_dev->event_shm.dev, "DMA-BUF exported on first use\n");
		}

		/* Create per-process fd - dma_buf_fd() takes the reference from export */
		*(int *)arg = dma_buf_fd(isp_dev->event_shm.dmabuf, O_RDWR | O_CLOEXEC);
		if (*(int *)arg < 0) {
			dev_err(isp_dev->event_shm.dev,
				"VISP_GET_EVENT_SHM_FD: dma_buf_fd failed: %d\n", *(int *)arg);
			ret = *(int *)arg;
		} else {
			dev_info(isp_dev->event_shm.dev,
				 "VISP_GET_EVENT_SHM_FD: returned fd=%d\n", *(int *)arg);
			ret = 0;
		}
		break;
	}
	default:
		break;
	}
	return ret;
}

static int visp_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
				struct v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_CTRL:
		return v4l2_ctrl_subdev_subscribe_event(sd, fh, sub);
	case VISP_DEAMON_EVENT:
		return v4l2_event_subscribe(fh, sub, 2, NULL);
	default:
		return -EINVAL;
	}
}

static struct v4l2_subdev_core_ops visp_core_ops = {
	.ioctl = visp_priv_ioctl,
	.subscribe_event = visp_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

/*
 * When the requested pad is the sink pad, the interval is applied directly and
 * propagated to all source pads without calling media_isp_set_frame_rate().
 * For source pads, the interval is clamped to the sink pad's time-per-frame
 * if invalid. media_isp_set_frame_rate() updates the framerate changes in ISP
 * structs which will be passed on to hardware.
 */
static int visp_set_frame_interval(struct v4l2_subdev *sd,
				   struct v4l2_subdev_state *sd_state,
				   struct v4l2_subdev_frame_interval *fi)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_data *cur_pad = &isp_dev->pad_data[fi->pad];
	struct visp_pad_data *sink_pad;
	struct visp_pad_data *source_pad;
	uint32_t sink_pad_index;
	int ret = 0;
	int i = 0;
	struct v4l2_fract *timeperframe;
	uint32_t frame_rate = 0;

	sink_pad_index = fi->pad - (fi->pad % VISP_PORT_PAD_NR);
	sink_pad = &isp_dev->pad_data[sink_pad_index];

	if (sink_pad == cur_pad) {
		cur_pad->timeperframe = fi->interval;
		for (i = 1; i < VISP_PORT_PAD_NR; i++) {
			source_pad = &isp_dev->pad_data[sink_pad_index + i];
			source_pad->timeperframe = fi->interval;
		}
		return 0;
	}

	if (fi->interval.denominator == 0 || fi->interval.numerator == 0 ||
		fi->interval.denominator > sink_pad->timeperframe.denominator) {
		fi->interval = sink_pad->timeperframe;
	}

	timeperframe = &(fi->interval);
	frame_rate = timeperframe->denominator / timeperframe->numerator;

	ret = media_isp_set_frame_rate(isp_dev, fi->pad, &frame_rate);
	if (ret) {
		dev_err(isp_dev->dev, "pad:%d Setfraemrate failed", fi->pad);
	} else {
		for (i = 0; i < VISP_PORT_PAD_NR; i++) {
			source_pad = &isp_dev->pad_data[sink_pad_index + i];
			source_pad->timeperframe = fi->interval;
		}
	}

	return ret;
}

int visp_set_frame_interval_public(struct visp_dev *isp_dev,
				   struct v4l2_subdev_frame_interval *fi)
{
	struct v4l2_subdev_pad_config pad_cfg;
	struct v4l2_subdev_state sd_state = {
		.pads = &pad_cfg,
	};

	return visp_set_frame_interval(&(isp_dev->sd), &sd_state, fi);
}

static int visp_get_frame_interval(struct v4l2_subdev *sd,
				   struct v4l2_subdev_state *sd_state,
				   struct v4l2_subdev_frame_interval *fi)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_data *pad_data = &isp_dev->pad_data[fi->pad];

	if (pad_data->sink_detected)
		fi->interval = pad_data->timeperframe;
	else
		return -EINVAL;

	return 0;
}

static struct v4l2_subdev_video_ops visp_video_ops = {
	.s_stream = visp_s_stream,
};

int media_isp_hal_mbus_fmt_to_media_fmt(uint32_t *code, uint32_t *pixel_format,
					uint32_t fourcc_code);

/*
 * Applications (v4l2/gst) send fourcc; media-ctl sends mbus code. These are
 * agreed types between application and kernel, but the pixel_format is unique
 * to the application and can change.
 *
 * With the passed values of code and fourcc, the corresponding pix format is
 * identified and stored.
 */
static int visp_set_fmt(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
			struct v4l2_subdev_format *format)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	uint32_t w, h;
	uint32_t sink_pad_index;
	struct visp_pad_data *cur_pad = &isp_dev->pad_data[format->pad];
	struct visp_pad_data *sink_pad;
	struct visp_pad_data *source_pad;
	int i;
	int ret = 0;
	media_fmt format_media;
	struct v4l2_mbus_framefmt *mbus_format;
	struct media_pad *mediapad_t;
	uint32_t fourcc_code = 0;

	sink_pad_index = format->pad - (format->pad % VISP_PORT_PAD_NR);
	sink_pad = &isp_dev->pad_data[sink_pad_index];

	if (sink_pad == cur_pad) {
		cur_pad->sink_detected = 1;
		cur_pad->format = format->format;
		for (i = 1; i < VISP_PORT_PAD_NR; i++) {
			source_pad = &isp_dev->pad_data[sink_pad_index + i];
			source_pad->sink_detected = 1;
		}
		return 0;
	}

	ret = visp_setup_isp_pipeline(isp_dev, format->pad);
	if (ret)
		return ret;

	w = ALIGN(format->format.width, VISP_WIDTH_ALIGN);
	h = ALIGN(format->format.height, VISP_HEIGHT_ALIGN);
	w = clamp_t(uint32_t, w, VISP_WIDTH_MIN, sink_pad->format.width);
	h = clamp_t(uint32_t, h, VISP_HEIGHT_MIN, sink_pad->format.height);

	format->format.width = w;
	format->format.height = h;

	memcpy(&fourcc_code, format->format.reserved, sizeof(uint32_t));

	int yuv_420_index =
		ISP_DEV_EXTENDED(isp_dev)->yuv_420_format_index
		[CAMDEV_BUFCHAIN_MP];

	for (i = 0; i < cur_pad->num_formats; i++) {
		if (ISP_DEV_EXTENDED(isp_dev)->is_oba_yuv_420[format->pad - 1]) {
			if (yuv_420_index < 0) {
				dev_err(isp_dev->dev,
					"yuv420 format is not avaialable from driver\n");
				i = 0;
			} else if (format->format.code == cur_pad->fmts[yuv_420_index].code) {
				dev_info(isp_dev->dev,
					 "%s %d MATCH CODE index : %d code:%x:%x\n",
					 __func__, __LINE__,
					 yuv_420_index,
					 cur_pad->fmts[yuv_420_index].code,
					 cur_pad->fmts[yuv_420_index].fourcc);

				i = yuv_420_index;

			} else {
				i = yuv_420_index;
				dev_info(isp_dev->dev,
					 "The received format is not supported on design, setting the supported format index : %d code:%x:%x\n",
					 i,
					 cur_pad->fmts[i].code,
					 cur_pad->fmts[i].fourcc);
			}
			break;
		} else if (format->format.code == cur_pad->fmts[i].code ||
			fourcc_code == cur_pad->fmts[i].fourcc) {
			if (format->format.code == cur_pad->fmts[i].code) {
				dev_err(isp_dev->dev,
					"%s %d MATCH CODE index:%d num_fmts : %d code:%x:%x\n",
					__func__, __LINE__, i, cur_pad->num_formats,
					cur_pad->fmts[i].code, fourcc_code);
			}
			if (fourcc_code == cur_pad->fmts[i].fourcc) {
				dev_err(isp_dev->dev,
					"%s %d MATCH FOURCE index:%d fourcc : %x\n",
					__func__, __LINE__, i, cur_pad->fmts[i].fourcc);
			}
			if (i == yuv_420_index)	{
				i = 0;
				dev_info(isp_dev->dev,
					 "The received format is not supported on design, setting the default supported format index : %d code:%x:%x\n",
					 i,
					 cur_pad->fmts[i].code,
					 cur_pad->fmts[i].fourcc);
			}
			break;
		}
	}

	if (i >= cur_pad->num_formats) {
		dev_info(isp_dev->dev, "Format not found rolling to 1st avaialble FMT\n");
		format->format.code = cur_pad->fmts[0].code;
		memcpy(format->format.reserved, &cur_pad->fmts[0].fourcc,
		       sizeof(uint32_t));
	} else {
		format->format.code = cur_pad->fmts[i].code;
		memcpy(format->format.reserved, &cur_pad->fmts[i].fourcc,
		       sizeof(uint32_t));
	}

	if (ret)
		return ret;

	memset(&format_media, 0, sizeof(format_media));

	mbus_format = (struct v4l2_mbus_framefmt *)&format->format;
	/* Fill the struct to be shared with ISP/RPU*/
	format_media.width = mbus_format->width;
	format_media.height = mbus_format->height;
	format_media.color_space = mbus_format->colorspace;
	format_media.quantization = mbus_format->quantization;
	fourcc_code = 0;
	dev_info(isp_dev->dev, "%s %d format->pad:%d width:%d height :%d\n",
		 __func__, __LINE__, format->pad, mbus_format->width, mbus_format->height);

	if (sizeof(mbus_format->reserved) == (sizeof(uint16_t) * 10)) {
		memcpy(&fourcc_code, &mbus_format->reserved,
		       sizeof(fourcc_code));
	} else {
		memcpy(&fourcc_code, &mbus_format->reserved[1],
		       sizeof(fourcc_code));
	}
	media_isp_hal_mbus_fmt_to_media_fmt(
		&mbus_format->code, &format_media.pixel_format, fourcc_code);

	mediapad_t = &isp_dev->pads[format->pad];
	ret = media_isp_set_format(isp_dev, mediapad_t->index, format_media);
	if (ret)
		return ret;

	cur_pad->format = format->format;

	return 0;
}

int visp_set_fmt_public(struct visp_dev *isp_dev,
			struct v4l2_subdev_format *format)
{
	uint32_t w, h;
	uint32_t sink_pad_index;
	struct visp_pad_data *cur_pad = &isp_dev->pad_data[format->pad];
	struct visp_pad_data *sink_pad;
	struct visp_pad_data *source_pad;
	int i;
	int ret = 0;
	media_fmt format_media;
	struct v4l2_mbus_framefmt *mbus_format;
	struct media_pad *mediapad_t;
	uint32_t fourcc_code = 0;

	sink_pad_index = format->pad - (format->pad % VISP_PORT_PAD_NR);
	sink_pad = &isp_dev->pad_data[sink_pad_index];

	if (sink_pad == cur_pad) {
		cur_pad->sink_detected = 1;
		cur_pad->format = format->format;
		for (i = 1; i < VISP_PORT_PAD_NR; i++) {
			source_pad = &isp_dev->pad_data[sink_pad_index + i];
			source_pad->sink_detected = 1;
		}
		return 0;
	}

	w = ALIGN(format->format.width, VISP_WIDTH_ALIGN);
	h = ALIGN(format->format.height, VISP_HEIGHT_ALIGN);
	w = clamp_t(uint32_t, w, VISP_WIDTH_MIN, sink_pad->format.width);
	h = clamp_t(uint32_t, h, VISP_HEIGHT_MIN, sink_pad->format.height);

	format->format.width = w;
	format->format.height = h;

	memcpy(&fourcc_code, format->format.reserved, sizeof(uint32_t));
	for (i = 0; i < cur_pad->num_formats; i++) {
		if (format->format.code == cur_pad->fmts[i].code &&
			fourcc_code == cur_pad->fmts[i].fourcc)
			break;
	}

	if (i >= cur_pad->num_formats) {
		format->format.code = cur_pad->fmts[0].code;
		memcpy(format->format.reserved, &cur_pad->fmts[0].fourcc,
			   sizeof(uint32_t));
	}

	memset(&format_media, 0, sizeof(format_media));

	mbus_format = (struct v4l2_mbus_framefmt *)&format->format;
	format_media.width = mbus_format->width;
	format_media.height = mbus_format->height;
	format_media.color_space = mbus_format->colorspace;
	format_media.quantization = mbus_format->quantization;
	fourcc_code = 0;

	if (sizeof(mbus_format->reserved) == (sizeof(uint16_t) * 10)) {
		memcpy(&fourcc_code, &mbus_format->reserved,
		       sizeof(fourcc_code));
	} else {
		memcpy(&fourcc_code, &mbus_format->reserved[1],
		       sizeof(fourcc_code));
	}
	media_isp_hal_mbus_fmt_to_media_fmt(
		&mbus_format->code, &format_media.pixel_format, fourcc_code);

	mediapad_t = &isp_dev->pads[format->pad];
	ret = media_isp_set_format(isp_dev, mediapad_t->index, format_media);
	if (ret)
		return ret;

	cur_pad->format = format->format;

	return 0;
}

static int visp_get_fmt(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
			struct v4l2_subdev_format *format)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_data *pad_data = &isp_dev->pad_data[format->pad];
	int ret;

	ret = visp_setup_isp_pipeline(isp_dev, format->pad);
	if (ret)
		return ret;
	format->format = pad_data->format;
	dev_info(isp_dev->dev, "%s %d GET_FMT pad=:%d format->format.code=:%x\n",
		 __func__, __LINE__, format->pad, format->format.code);

	return 0;
}

static int visp_enum_mbus_code(struct v4l2_subdev *sd,
				   struct v4l2_subdev_state *sd_state,
				   struct v4l2_subdev_mbus_code_enum *code)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_data *pad_data = &isp_dev->pad_data[code->pad];

	if (code->index >= pad_data->num_formats) {
		dev_dbg(isp_dev->dev, "%s %d pad name %s index %d\n", __func__,
			__LINE__, isp_dev->pads[code->pad].entity->name,
			isp_dev->pads[code->pad].index);
		return -EINVAL;
	}

	code->code = pad_data->fmts[code->index].code;
	code->reserved[0] = pad_data->fmts[code->index].fourcc;

	return 0;
}

static const struct v4l2_subdev_pad_ops visp_pad_ops = {
	.set_fmt = visp_set_fmt,
	.get_fmt = visp_get_fmt,
	.enum_mbus_code = visp_enum_mbus_code,
	.get_frame_interval = visp_get_frame_interval,
	.set_frame_interval = visp_set_frame_interval,

};

struct v4l2_subdev_ops visp_subdev_ops = {
	.core = &visp_core_ops,
	.video = &visp_video_ops,
	.pad = &visp_pad_ops,
};

static int visp_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&isp_dev->mlock);

	isp_dev->refcnt++;

	mutex_unlock(&isp_dev->mlock);
	return 0;
}

static int visp_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&isp_dev->mlock);

	isp_dev->refcnt--;

	if (isp_dev->refcnt == 0) {
/*
*#if 0
 *		for (port = 0; port < VISP_PORT_NR; port++)
 *		{
 *			list_for_each_entry_safe(pos, next, &isp_dev->mcm_input[port],
 *									 entry)
 *			{
 *				if (pos)
 *				{
 *					dma_free_coherent(isp_dev->dev, pos->size, pos->vaddr,
 *									  pos->addr);
 *					list_del(&pos->entry);
 *					kfree(pos);
 *				}
 *			}
 *		}
 *#endif
 */
	}

	mutex_unlock(&isp_dev->mlock);

	return 0;
}

static struct v4l2_subdev_internal_ops visp_internal_ops = {
	.open = visp_open,
	.close = visp_close,
};

static int visp_link_setup(struct media_entity *entity,
			   const struct media_pad *local,
			   const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations visp_entity_ops = {
	.link_setup = visp_link_setup,
	.link_validate = v4l2_subdev_link_validate,
	.get_fwnode_pad = v4l2_subdev_get_fwnode_pad_1_to_1,

};

static int visp_notifier_bound(struct v4l2_async_notifier *notifier,
				   struct v4l2_subdev *sd,
				   struct v4l2_async_connection *asc)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(notifier, struct visp_dev, notifier);
	struct device *dev = isp_dev->dev;

	struct fwnode_handle *ep = NULL;
	struct v4l2_fwnode_link link;
	struct media_entity *source, *sink;
	unsigned int source_pad, sink_pad;

	while (1) {
		ep = fwnode_graph_get_next_endpoint(sd->fwnode, ep);
		if (!ep)
			break;

		ret = v4l2_fwnode_parse_link(ep, &link);
		if (ret < 0) {
			dev_err(dev, "failed to parse link for %pOF: %d\n",
				to_of_node(ep), ret);
			continue;
		}

		if (sd->entity.pads[link.local_port].flags == MEDIA_PAD_FL_SINK)
			continue;

		source = &sd->entity;
		source_pad = link.remote_port;
		sink = &isp_dev->sd.entity;
		sink_pad = link.local_port;
		v4l2_fwnode_put_link(&link);
		ret = media_create_pad_link(source, source_pad, sink, sink_pad,
						MEDIA_LNK_FL_ENABLED);
		isp_dev->ports_mask |=
			(1 << (source_pad / MEDIA_ISP_PORT_PAD_COUNT));
		if (ret) {
			dev_err(dev, "failed to create %s:%u -> %s:%u link\n",
				source->name, source_pad, sink->name, sink_pad);
			break;
		}
	}

	fwnode_handle_put(ep);

	return ret;
}

static void visp_notifier_unbound(struct v4l2_async_notifier *notifier,
				  struct v4l2_subdev *sd,
				  struct v4l2_async_connection *asc)
{
}

static const struct v4l2_async_notifier_operations visp_notify_ops = {
	.bound = visp_notifier_bound,
	.unbind = visp_notifier_unbound,
};

static int visp_async_notifier(struct visp_dev *isp_dev)
{
	struct fwnode_handle *ep;
	struct fwnode_handle *remote_ep;
	struct v4l2_async_connection *asc;
	struct device *dev = isp_dev->dev;
	int ret = 0;
	int pad = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	v4l2_async_subdev_nf_init(&isp_dev->notifier, &isp_dev->sd);
#else
	v4l2_async_notifier_init(&isp_dev->notifier);
#endif

	isp_dev->notifier.ops = &visp_notify_ops;

	if (dev_fwnode(isp_dev->dev) == NULL)
		return 0;

	for (pad = 0; pad < isp_dev->num_pads; pad++) {
		if (isp_dev->pads[pad].flags != MEDIA_PAD_FL_SINK)
			continue;

		ep = fwnode_graph_get_endpoint_by_id(
			dev_fwnode(dev), pad, 0, FWNODE_GRAPH_ENDPOINT_NEXT);
		if (!ep)
			continue;
		remote_ep = fwnode_graph_get_remote_endpoint(ep);
		if (!remote_ep) {
			fwnode_handle_put(ep);
			continue;
		}
		fwnode_handle_put(remote_ep);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
		asc = v4l2_async_nf_add_fwnode_remote(
			&isp_dev->notifier, ep, struct v4l2_async_connection);
#else
		asd = v4l2_async_notifier_add_fwnode_remote_subdev(
			&isp_dev->notifier, ep, struct v4l2_async_subdev);
#endif

		fwnode_handle_put(ep);

		if (IS_ERR(asc)) {
			ret = PTR_ERR(asc);
			if (ret != -EEXIST) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
				v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
				v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
				return ret;
			}
		}
	}

	ret = v4l2_async_nf_register(&isp_dev->notifier);
	if (ret) {
		dev_err(isp_dev->dev, "Async notifier register error\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
		v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
		v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
	}

	return ret;
}

static int visp_pads_init(struct visp_dev *isp_dev)
{
	int pad = 0;

	isp_dev->num_pads = visp_get_num_pads(isp_dev);
	isp_dev->pads = devm_kzalloc(isp_dev->dev, sizeof(struct media_pad) * isp_dev->num_pads, GFP_KERNEL);
	if (!isp_dev->pads)
		return -ENOMEM;

	isp_dev->pad_data = devm_kzalloc(isp_dev->dev, sizeof(struct visp_pad_data) * isp_dev->num_pads, GFP_KERNEL);
	if (!isp_dev->pad_data)
		return -ENOMEM;

	for (pad = 0; pad < isp_dev->num_pads; pad++) {
		if ((pad % VISP_PORT_PAD_NR) == VISP_PORT_PAD_SINK)
			isp_dev->pads[pad].flags = MEDIA_PAD_FL_SINK;
		else
			isp_dev->pads[pad].flags = MEDIA_PAD_FL_SOURCE;

		switch (pad % VISP_PORT_PAD_NR) {
		case VISP_PORT_PAD_SINK:
			break;
		case VISP_PORT_PAD_SOURCE_MP:
			isp_dev->pad_data[pad].num_formats =
				ARRAY_SIZE(visp_mp_fmts);
			isp_dev->pad_data[pad].fmts = visp_mp_fmts;
			break;
		case VISP_PORT_PAD_SOURCE_SP1:
			isp_dev->pad_data[pad].num_formats =
				ARRAY_SIZE(visp_sp_fmts);
			isp_dev->pad_data[pad].fmts = visp_sp_fmts;
			break;
		case VISP_PORT_PAD_SOURCE_SP2:
			isp_dev->pad_data[pad].num_formats =
				ARRAY_SIZE(visp_sp_fmts);
			isp_dev->pad_data[pad].fmts = visp_sp_fmts;
			break;
		case VISP_PORT_PAD_SOURCE_RAW:
			isp_dev->pad_data[pad].num_formats =
				ARRAY_SIZE(visp_raw_fmts);
			isp_dev->pad_data[pad].fmts = visp_raw_fmts;
			break;
		default:
			break;
		}
		INIT_LIST_HEAD(&isp_dev->pad_data[pad].queue);
		spin_lock_init(&isp_dev->pad_data[pad].qlock);
	}

	return 0;
}

static int parse_iba(struct visp_dev *isp_dev, struct device_node *np)
{
	int num_streams = isp_dev->num_streams;
	int i;

	if (num_streams > MAX_IBA_PER_ISP) {
		dev_err(isp_dev->dev,
			"num_streams exceeds maximum allowed (%d)\n",
			MAX_IBA_PER_ISP);
		return -EINVAL;
	}

	for (i = 0; i < num_streams; i++) {
		char property_name[64];
		int iba_index;

		if ((isp_dev->id % 2) == 0) {
			iba_index = i;
		} else if ((isp_dev->id % 2) == 1) {
			iba_index = (num_streams == 1) ? 4 : 3 + i;
		} else {
			dev_err(isp_dev->dev, "Unsupported isp_id: %d\n",
				isp_dev->id);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name), "xlnx,iba%d_ppc",
			 iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[iba_index].ppc)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,iba%d_vcid", iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[iba_index].vcid)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,iba%d_frame_rate", iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[iba_index].frame_rate)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,iba%d_data_format", iba_index);
		if (of_property_read_u32(
			np, property_name,
			&isp_dev->iba[iba_index].data_format)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,iba%d_max-width", iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[iba_index].max_width)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,iba%d_max-height", iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[iba_index].max_height)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		dev_dbg(
			isp_dev->dev,
			"IBA%d: ppc=%d, vcid=%d, frame_rate=%d, data_format=%d, "
			"max_width=%d, max_height=%d\n",
			iba_index, isp_dev->iba[iba_index].ppc,
			isp_dev->iba[iba_index].vcid,
			isp_dev->iba[iba_index].frame_rate,
			isp_dev->iba[iba_index].data_format,
			isp_dev->iba[iba_index].max_width,
			isp_dev->iba[iba_index].max_height);
	}

	return 0;
}

static void get_yuv_420_format_index(struct visp_dev *isp_dev, int path)
{
	struct visp_pad_data *cur_pad = &isp_dev->pad_data[path + 1];
	int i = 0;

	for (i = 0; i < cur_pad->num_formats; i++) {
		if (cur_pad->fmts[i].code == MEDIA_BUS_FMT_VYYUYY8_1X24) {
			dev_info(isp_dev->dev, "%s %d MATCH CODE index:%d num_fmts : %d code:%x\n",
				 __func__, __LINE__, i, cur_pad->num_formats,
				 cur_pad->fmts[i].code);
			break;
		}
	}

	if (i >= cur_pad->num_formats) {
		i = -1;
		dev_err(isp_dev->dev, "The format in design is not supported isp:%d fmt:%x\n",
			isp_dev->id, MEDIA_BUS_FMT_VYYUYY8_1X24);
	} else {
		ISP_DEV_EXTENDED(isp_dev)->yuv_420_format_index[path] = i;
	}
}
static int parse_oba(struct visp_dev *isp_dev, struct device_node *np)
{
	int num_streams = isp_dev->num_streams;
	int i;

	if (num_streams > 1) {
		dev_err(isp_dev->dev,
			"num_streams exceeds maximum allowed (%d)\n",
			MAX_IBA_PER_ISP);
		return -EINVAL;
	}

	for (i = 0; i < num_streams; i++) {
		char property_name[64];
		int oba_index;

		if ((isp_dev->id % 2) == 0) {
			oba_index = 0;
		} else if ((isp_dev->id % 2) == 1) {
			oba_index = 1;
		} else {
			dev_err(isp_dev->dev, "Unsupported isp_id: %d\n",
				isp_dev->id);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,oba%d_mp_ppc", oba_index);
		if (of_property_read_u32(
			np, property_name,
			&isp_dev->oba[CAMDEV_BUFCHAIN_MP].ppc)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,oba%d_mp_bpp", oba_index);
		if (of_property_read_u32(
			np, property_name,
			&isp_dev->oba[CAMDEV_BUFCHAIN_MP].bpp)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,oba%d_mp_data_format", oba_index);
		if (of_property_read_string(
			np, property_name,
			&isp_dev->oba[CAMDEV_BUFCHAIN_MP].data_format)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}
		if (strcasecmp(isp_dev->oba[CAMDEV_BUFCHAIN_MP].data_format, "YUV420") == 0)
			ISP_DEV_EXTENDED(isp_dev)->is_oba_yuv_420[CAMDEV_BUFCHAIN_MP] = true;
		else
			ISP_DEV_EXTENDED(isp_dev)->is_oba_yuv_420[CAMDEV_BUFCHAIN_MP] = false;

		snprintf(property_name, sizeof(property_name),
			 "xlnx,oba%d_sp_ppc", oba_index);
		if (of_property_read_u32(
			np, property_name,
			&isp_dev->oba[CAMDEV_BUFCHAIN_SP1].ppc)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,oba%d_sp_bpp", oba_index);
		if (of_property_read_u32(
			np, property_name,
			&isp_dev->oba[CAMDEV_BUFCHAIN_SP1].bpp)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,oba%d_sp_data_format", oba_index);
		if (of_property_read_string(
			np, property_name,
			&isp_dev->oba[CAMDEV_BUFCHAIN_SP1].data_format)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}
		if (strcasecmp(isp_dev->oba[CAMDEV_BUFCHAIN_SP1].data_format, "YUV420") == 0)
			ISP_DEV_EXTENDED(isp_dev)->is_oba_yuv_420[CAMDEV_BUFCHAIN_SP1] = true;
		else
			ISP_DEV_EXTENDED(isp_dev)->is_oba_yuv_420[CAMDEV_BUFCHAIN_SP1] = false;

		dev_dbg(
			isp_dev->dev, "OBA%d: ppc=%d, bpp=%d, data_format=%s\n",
			CAMDEV_BUFCHAIN_MP, isp_dev->oba[CAMDEV_BUFCHAIN_MP].ppc,
			isp_dev->oba[CAMDEV_BUFCHAIN_MP].bpp,
			isp_dev->oba[CAMDEV_BUFCHAIN_MP].data_format);
		dev_dbg(
			isp_dev->dev, "OBA%d: ppc=%d, bpp=%d, data_format=%s\n",
			CAMDEV_BUFCHAIN_SP1, isp_dev->oba[CAMDEV_BUFCHAIN_SP1].ppc,
			isp_dev->oba[CAMDEV_BUFCHAIN_SP1].bpp,
			isp_dev->oba[CAMDEV_BUFCHAIN_SP1].data_format);
	}

	return 0;
}

static int visp_parse_params(struct visp_dev *isp_dev,
				 struct platform_device *pdev)
{
	int port = 0;
	int ret = 0;
	struct device_node *node = pdev->dev.of_node;

	for (port = 0; port < VISP_PORT_NR; port++) {
		strscpy(isp_dev->isp_ports[port].sensor_info.name,
			VISP_DEFAULT_SENSOR,
			sizeof(isp_dev->isp_ports[port].sensor_info.name));

		strscpy(isp_dev->isp_ports[port].sensor_info.calib,
			VISP_DEFAULT_SENSOR_CALIB,
			sizeof(isp_dev->isp_ports[port].sensor_info.calib));

		isp_dev->isp_ports[port].sensor_info.mode =
		    VISP_DEFAULT_SENSOR_MODE;
		isp_dev->isp_ports[port].sensor_info.sensor_id =
		    sensor_dev_id[port];

		strscpy(isp_dev->isp_ports[port].sensor_info.manu_json,
			VISP_DEFAULT_SENSOR_MANU_JSON,
			sizeof(isp_dev->isp_ports[port].sensor_info.manu_json));

		strscpy(isp_dev->isp_ports[port].sensor_info.auto_json,
			VISP_DEFAULT_SENSOR_AUTO_JSON,
			sizeof(isp_dev->isp_ports[port].sensor_info.auto_json));
	}

	fwnode_property_read_u32(of_fwnode_handle(node), "isp_id",
				 &isp_dev->id);
	if (!node) {
		dev_err(&pdev->dev, "No device tree node found\n");
		return -EINVAL;
	}
	if (isp_dev->id < 0 || isp_dev->id > 5) {
		dev_err(&pdev->dev, "Invalid ISP id %d\n", isp_dev->id);
		return -EINVAL;
	}

	// Read string property for SS-MODE-i0 (LIMO, etc.)
	ret =
		of_property_read_string(node, "xlnx,io_mode", &isp_dev->ss_mode_i0);
	if (ret) {
		dev_err(&pdev->dev, "Failed to read xlnx,io_mode\n");
		return ret;
	} else {
		dev_dbg(&pdev->dev, "xlnx,io_mode: %s\n", isp_dev->ss_mode_i0);
	}

	// Read stream info (multi-stream, single-stream)
	ret = of_property_read_u32(node, "xlnx,num_streams",
				   &isp_dev->num_streams);
	if (ret) {
		dev_err(&pdev->dev,
			"Failed to read xlnx,num_streams property\n");
		return ret;
	} else {
		dev_dbg(&pdev->dev, "xlnx,num_streams: %u\n",
			isp_dev->num_streams);
	}

	ret = of_property_read_u32(node, "xlnx,mem_inputs", &isp_dev->isp_mem);
	if (ret) {
		dev_err(&pdev->dev,
			"Failed to read xlnx,mem_inputs property\n");
		return ret;
	} else {
		dev_dbg(&pdev->dev, "xlnx,mem_inputs: %u\n", isp_dev->isp_mem);
	}

	ret = of_property_read_u32(node, "xlnx,rpu", &isp_dev->isp_rpu);
	if (ret) {
		dev_err(&pdev->dev, "Failed to read xlnx,rpu property\n");
		return ret;
	} else {
		dev_dbg(&pdev->dev, "xlnx,rpu: %u\n", isp_dev->isp_rpu);
	}

	if (parse_iba(isp_dev, node)) {
		dev_err(&pdev->dev, "Failed to parse IBA parameters\n");
		return -EINVAL;
	}

	uint32_t num_mems = of_count_phandle_with_args(pdev->dev.of_node,
							   "memory-region", NULL);

	if (num_mems < 0) {
		dev_err(isp_dev->dev, "%s no memory for calibration\n",
			__func__);
		return -ENOMEM;
	}

	for (int i = 0; i < num_mems; i++) {
		struct device_node *node;
		struct reserved_mem *rmem;

		node = of_parse_phandle(pdev->dev.of_node, "memory-region", i);
		if (!node)
			return -EINVAL;

		rmem = of_reserved_mem_lookup(node);
		if (!rmem)
			return -EINVAL;
		isp_dev->reserve_mem.pa = rmem->base;
		isp_dev->reserve_mem.size = rmem->size;
		dev_dbg(isp_dev->dev, "%s %d pa:%llu size :%u\n", __func__,
			__LINE__, isp_dev->reserve_mem.pa,
			isp_dev->reserve_mem.size);
	}

	int len = strlen(isp_dev->ss_mode_i0);
	if (strncmp(&isp_dev->ss_mode_i0[len - 2], "lo", 2) == 0) {
		if (parse_oba(isp_dev, node)) {
			dev_err(&pdev->dev, "Failed to parse OBA parameters\n");
			return -EINVAL;
		}
	}
	return 0;
}

static int xlnx_link_mbox(struct visp_dev *isp_dev)
{
	/* Find or create a new RPU with the given rpu_id */
	isp_dev->rpu = visp_mbox_get_rpu_dev(isp_dev->isp_rpu);
	if (!isp_dev->rpu) {
		dev_err(isp_dev->dev, "Failed to find or create RPU: %d\n",
			isp_dev->isp_rpu);
		return -ENOMEM;
	}
	/* initialise completion used in while waiting for ack & data*/
	/* Initialize 3D completion array for ENQUE_BUFFER [port][path][buffer] */
	for (int port = 0; port < isp_dev->num_streams; port++)
		for (int path = 0; path < 4; path++)
			for (int buf = 0; buf < 32; buf++)
				init_completion(&isp_dev->apu_wait_for_enq_ack[port][path][buf]);

	/* Initialize port-level completions for other commands */
	for (int port = 0; port < isp_dev->num_streams; port++) {
		init_completion(&isp_dev->apu_wait_for_cmd_ack[port]);
		mutex_init(&isp_dev->cmd_ack_fifo_lock[port]);
		/* Allocate port-level FIFO (128 entries) */
		if (kfifo_alloc(&isp_dev->cmd_ack_fifo[port], 128, GFP_KERNEL)) {
			dev_err(isp_dev->dev, "Failed to allocate cmd_ack_fifo[%d]\n", port);
			return -ENOMEM;
		}
	}
	init_completion(&isp_dev->apu_wait_for_data);

	if (!isp_dev->rpu->tx_chan || !isp_dev->rpu->rx_chan) {
		dev_err(isp_dev->dev, "No TX or RX Channel found on RPU: %d\n",
			isp_dev->isp_rpu);
		return -ENOMEM;
	}

	isp_dev->tx_chan = isp_dev->rpu->tx_chan;
	isp_dev->rx_chan = isp_dev->rpu->rx_chan;
	isp_dev->rpu->isp_dev[isp_dev->id] =
		isp_dev; // Assigning isp_dev structure value to isp_dev present in
			 // rpu_dev struct

	return 0;
}
static int visp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct visp_dev *isp_dev;
	int ret;
	int port = 0;

	isp_dev = devm_kzalloc(&pdev->dev, sizeof(struct visp_dev), GFP_KERNEL);
	if (!isp_dev)
		return -ENOMEM;

	isp_dev->extended_struct =
		devm_kzalloc(&pdev->dev, sizeof(struct visp_lilo_isp_dev_extended), GFP_KERNEL);
	if (!isp_dev->extended_struct)
		return -ENOMEM;

	mutex_init(&isp_dev->mlock);
	mutex_init(&isp_dev->ctrl_lock);
	isp_dev->dev = &pdev->dev;
	platform_set_drvdata(pdev, isp_dev);

	ret = visp_parse_params(isp_dev, pdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to parse params\n");
		return -EINVAL;
	}
	ret = xlnx_link_mbox(isp_dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to init mbox\n");
		return -EINVAL;
	}

	/* Initialize mutexes for cam_device_bufs arrays (num_streams ports × 4 channels) */
	for (int port = 0; port < isp_dev->num_streams; port++)
		for (int chn = 0; chn < 4; chn++)
			mutex_init(&isp_dev->isp_ports[port]
				       .isp_chns[chn]
				       .cam_device_bufs_lock);

	v4l2_subdev_init(&isp_dev->sd, &visp_subdev_ops);
	snprintf(isp_dev->sd.name, VISP_SUBDEV_NAME_SIZE, "%s.%d", VISP_NAME,
		 isp_dev->id);

	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	isp_dev->sd.dev = &pdev->dev;
	isp_dev->sd.owner = THIS_MODULE;
	isp_dev->sd.internal_ops = &visp_internal_ops;
	isp_dev->sd.entity.ops = &visp_entity_ops;
	isp_dev->sd.entity.function =
		/*MEDIA_ENT_F_IO_V4L*/ MEDIA_ENT_F_V4L2_SUBDEV_UNKNOWN;
	isp_dev->sd.entity.obj_type = MEDIA_ENTITY_TYPE_V4L2_SUBDEV;
	isp_dev->sd.entity.name = isp_dev->sd.name;
	v4l2_set_subdevdata(&isp_dev->sd, isp_dev);

	visp_pads_init(isp_dev);
	ret = media_entity_pads_init(&isp_dev->sd.entity, isp_dev->num_pads,
					 isp_dev->pads);
	if (ret)
		return ret;

	ret = visp_async_notifier(isp_dev);
	if (ret) {
		dev_err(dev, "register visp_async_notifier error\n");
		goto err_async_notifier;
	}

	ret = v4l2_async_register_subdev(&isp_dev->sd);
	if (ret) {
		dev_err(dev, "register subdev error\n");
		goto error_regiter_subdev;
	}
	// Assign the XML path to sensor_info[0].xml
	//
	ret = visp_procfs_register(isp_dev, &isp_dev->pde);
	if (ret) {
		dev_err(dev, "register procfs failed.\n");
		goto err_register_procfs;
	}

	/* Allocate DMA coherent memory for event shared memory */
	isp_dev->event_shm.size = PAGE_SIZE * 8;  /* 32KB (8 pages) */
	isp_dev->event_shm.dev = dev;
	isp_dev->event_shm.virt_addr = dma_alloc_coherent(dev,
							 isp_dev->event_shm.size,
							 &isp_dev->event_shm.dma_handle,
							 GFP_KERNEL);
	if (!isp_dev->event_shm.virt_addr) {
		dev_err(dev, "Failed to allocate DMA coherent memory for event_shm\n");
		ret = -ENOMEM;
		goto err_alloc_event_shm;
	}

	memset(isp_dev->event_shm.virt_addr, 0, isp_dev->event_shm.size);
	mutex_init(&isp_dev->event_shm.event_lock);

	/* DMA-BUF will be exported lazily on first VISP_GET_EVENT_SHM_FD ioctl */
	isp_dev->event_shm.dmabuf = NULL;
	isp_dev->event_shm.dmabuf_fd = -1;

	isp_dev->reserve_mem.va =
		ioremap_wc(isp_dev->reserve_mem.pa, isp_dev->reserve_mem.size);

	visp_ctrl_init(isp_dev);

	for (port = 0; port < VISP_PORT_NR; port++) {
		mutex_init(&isp_dev->port_lock[port]);
		INIT_LIST_HEAD(&isp_dev->mcm_input[port]);
	}

	isp_dev->ports_mask = isp_dev->num_streams;
	/* Register Callback function*/
	isp_dev->frameout_cb = handle_frameout_buffer;

	get_yuv_420_format_index(isp_dev, CAMDEV_BUFCHAIN_MP);
	get_yuv_420_format_index(isp_dev, CAMDEV_BUFCHAIN_SP1);
	set_default_pad_config(isp_dev);

	dev_info(&pdev->dev, "visp isp driver probe success\n");

	return 0;

err_alloc_event_shm:
	visp_procfs_unregister(isp_dev->pde);

err_register_procfs:
	v4l2_async_unregister_subdev(&isp_dev->sd);

error_regiter_subdev:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	v4l2_async_nf_unregister(&isp_dev->notifier);
	v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
	v4l2_async_notifier_unregister(&isp_dev->notifier);
	v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif

err_async_notifier:
	media_entity_cleanup(&isp_dev->sd.entity);
	return ret;
}

static void visp_remove(struct platform_device *pdev)
{
	struct visp_dev *isp_dev;

	isp_dev = platform_get_drvdata(pdev);

	visp_procfs_unregister(isp_dev->pde);
	v4l2_async_unregister_subdev(&isp_dev->sd);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	v4l2_async_nf_unregister(&isp_dev->notifier);
	v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
	v4l2_async_notifier_unregister(&isp_dev->notifier);
	v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
	media_entity_cleanup(&isp_dev->sd.entity);

	/* Free DMA coherent memory for event_shm */
	if (isp_dev->event_shm.virt_addr) {
		/* If dmabuf still exists (fds still open), release it first */
		if (isp_dev->event_shm.dmabuf) {
			dev_warn(isp_dev->event_shm.dev,
				 "Dmabuf still active during remove - cleaning up\n");
			dma_buf_put(isp_dev->event_shm.dmabuf);
			isp_dev->event_shm.dmabuf = NULL;
			/* visp_dmabuf_release() will be called, but it won't free memory anymore */
		}
		/* Free the underlying DMA memory */
		dma_free_coherent(isp_dev->event_shm.dev, isp_dev->event_shm.size,
				  isp_dev->event_shm.virt_addr,
				  isp_dev->event_shm.dma_handle);
		dev_info(isp_dev->event_shm.dev, "Freed DMA coherent memory: %u bytes\n",
			 isp_dev->event_shm.size);
		isp_dev->event_shm.virt_addr = NULL;
	}

	visp_ctrl_destroy(isp_dev);
	dev_info(&pdev->dev, "visp isp driver remove\n");
}

static const struct dev_pm_ops visp_pm_ops = {};

static const struct of_device_id visp_of_match[] = {
	{
	.compatible = "xlnx,visp-ss-lilo-1.0",
	},
	{/* sentinel */},
};

MODULE_DEVICE_TABLE(of, visp_of_match);

static struct platform_driver visp_driver = {
	.probe = visp_probe,
	.remove = visp_remove,
	.driver = {
	.name = VISP_NAME,
	.owner = THIS_MODULE,
	.of_match_table = visp_of_match,

	}};

static int __init visp_init_module(void)
{
	int ret;

	ret = platform_driver_register(&visp_driver);
	if (ret) {
		printk(KERN_ERR "Failed to register isp driver\n");
		return ret;
	}

	return ret;
}

static void __exit visp_exit_module(void)
{
	platform_driver_unregister(&visp_driver);
}

module_init(visp_init_module);
module_exit(visp_exit_module);

MODULE_DESCRIPTION("Verisilicon isp v4l2 driver");
MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL");
MODULE_INFO(import_ns, "DMA_BUF");
