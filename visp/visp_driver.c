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
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/v4l2-mediabus.h>
#include <media/videobuf2-dma-contig.h>
#include <linux/version.h>

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

#define VISP_DEFAULT_SENSOR "ox03f10"
#define VISP_DEFAULT_SENSOR_MODE 0
#define VISP_DEFAULT_SENSOR_XML "/usr/share/limo_example_jsons/OX03f10.xml"
#define VISP_DEFAULT_SENSOR_MANU_JSON "/usr/share/limo_example_jsons/manual_ext.json"
#define VISP_DEFAULT_SENSOR_AUTO_JSON "/usr/share/limo_example_jsons/auto.json"

static uint32_t sensor_dev_id[VISP_PORT_NR] = {2, 6, 5, 10};

struct visp_format visp_mp_fmts[] = {
	{
		.fourcc = V4L2_PIX_FMT_NV16,
		.code = MEDIA_BUS_FMT_YUYV8_2X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV12,
		.code = MEDIA_BUS_FMT_YUYV8_1_5X8,
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
	/* {
	 *	.fourcc	= V4L2_PIX_FMT_P02BPACK,
	 *	.code	= MEDIA_BUS_FMT_YUYV12_2X12,
	 * },
	 */
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
	/* {
	 *	.fourcc	= V4L2_PIX_FMT_P22BPACK,
	 *	.code	= MEDIA_BUS_FMT_YUYV12_2X12,
	 * },
	 */
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
		.code = MEDIA_BUS_FMT_RGB888_1X24,
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
		.code = MEDIA_BUS_FMT_YUYV8_2X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV12,
		.code = MEDIA_BUS_FMT_YUYV8_1_5X8,
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
	/* {
	 *	.fourcc	= V4L2_PIX_FMT_P02BPACK,
	 *	.code	= MEDIA_BUS_FMT_YUYV12_2X12,
	 * },
	 */
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
	/* {
	 *	.fourcc	= V4L2_PIX_FMT_P22BPACK,
	 *	.code	= MEDIA_BUS_FMT_YUYV12_2X12,
	 * },
	 */
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
		.code = MEDIA_BUS_FMT_RGB888_1X24,
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

/* main path */
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

/**
 * visp_mbus_to_fourcc - Find the first fourcc code for a given mbus format
 * @mbus_code: Media bus format code to search for
 *
 * Returns the first matching fourcc code for the given mbus format,
 * or 0 if no match is found.
 */
static uint32_t visp_mbus_to_fourcc(uint32_t mbus_code)
{
	int i;

	/* Search in main port formats */
	for (i = 0; i < ARRAY_SIZE(visp_mp_fmts); i++) {
		if (visp_mp_fmts[i].code == mbus_code)
			return visp_mp_fmts[i].fourcc;
	}

	/* Search in source port formats */
	for (i = 0; i < ARRAY_SIZE(visp_sp_fmts); i++) {
		if (visp_sp_fmts[i].code == mbus_code)
			return visp_sp_fmts[i].fourcc;
	}

	return 0; /* No match found */
}

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
	if (/*IspChn->ThreadStatus == MEDIA_THREAD_STOPPED*/ isp_dev->streamon[pad_buf->pad] == 0) {
		memcpy(&IspChn->bufs[buf.index], &buf, sizeof(media_buf));
	} else {
		output_buffer_t *p_media_buffer = VSI_NULL;

		p_media_buffer = IspChn->cam_device_bufs[buf.index];

		if (p_media_buffer == VSI_NULL) {
			dev_err(isp_dev->dev, "CamDevice queue buf is null\n");
			return VSI_ERR_NULL_PTR;
		}

		ret_val = vsi_cam_device_en_que_buffer(isp_dev,
						       isp_port->cam_device_handle,
						       chn, p_media_buffer);
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

static int handle_frameout_buffer(struct visp_dev *isp_dev)
{
	output_buffer_t *output_buffer = NULL;
	struct Chn_info info;
	mbox_post_msg *msg;
	size_t size;
	void *packet_from_rpu;
	uint8_t buf_index;
	int pad = -1;
	int ret_val = 0;

	/* Validate inputs */
	if (!isp_dev) {
		dev_err(isp_dev->dev,
			"handle_frameout_buffer: isp_dev is NULL\n");
		return -EINVAL;
	}

	if (!kfifo_out(&isp_dev->display_fifo, &msg, 1)) {
		pr_err("Failed to queue into kfifo\n");
		return -ENOMEM;
	}

	if (!msg) {
		dev_err(isp_dev->dev, "%s: invalid msg or payload\n", __func__);
		ret_val = -EINVAL;
		goto error_free_buf;
	}

	size = msg->size;
	packet_from_rpu = kzalloc(size, GFP_KERNEL);
	if (!packet_from_rpu) {
		dev_err(isp_dev->dev, "Failed to allocate packet_from_rpu\n");
		return -ENOMEM;
	}

	memcpy(packet_from_rpu, msg->payload, size);

	/* Dequeue buffer from the ISP device*/
	read_dq_buf_info(packet_from_rpu, isp_dev, &info, &buf_index);

	output_buffer = isp_dev->isp_ports[info.vt_id]
			    .isp_chns[info.path]
			    .cam_device_bufs[buf_index];
	if (!output_buffer) {
		dev_err(isp_dev->dev,
			"handle_frameout_buffer: Outputbuffer is NULL...!\n");
		return -EINVAL;
	}

	/* Calculate the pad index */
	pad = (info.vt_id * MEDIA_ISP_PORT_PAD_COUNT) + (info.path + 1);
	if (pad <= 0) {
		dev_err(isp_dev->dev,
			"handle_frameout_buffer: Invalid pad value %d\n", pad);
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
			"handle_frameout_buffer: MediaIspHalBufDone failed "
			"with error %d\n",
			ret_val);
		dev_dbg(
		    isp_dev->dev,
		    "Skip buf: ret_val=%d, ISP=%d, port=%d, chn=%d, BUF=0x%x\n",
		    ret_val, isp_dev->id, info.vt_id, info.path,
		    output_buffer ? output_buffer->base_address : 0);
		goto error_free_buf;
	}
	// dev_err(isp_dev->dev,"[ISPDRV] Received DQ BUFF : Isp : %d port : %d
	// Path : %d BufAddr : 0x%x\n",info.hw_id,info.vt_id, info.path,
	// output_buffer->base_address);

	/* Free allocated buffer after successful processing*/
	kfree(packet_from_rpu);
	return 0;

error_free_buf:
	/* Free buffer in case of any error*/
	return ret_val;
}

/**
 * visp_get_input_subdev - Retrieve the remote sub-device connected to the ISP
 *			  input pad
 * @isp_dev: Pointer to the ISP device structure
 * @port: Port number to search for input subdev
 *
 * This function iterates over all media pads of the ISP device and identifies
 * the input pad (MEDIA_PAD_FL_SINK). It then checks for a remote connection
 * using media_pad_remote_pad_first() (for kernel 6.0 and later) or
 * media_entity_remote_pad().
 *
 * If a valid sub-device is connected to the input pad, it is returned.
 * Otherwise, the function logs an error and returns NULL.
 *
 * Return: Pointer to the v4l2_subdev structure if found, NULL otherwise.
 */

#if 0  // Disable unused function
static struct v4l2_subdev *visp_get_input_subdev(struct visp_dev *isp_dev,
						  int port)
{
	struct media_pad *remote_pad;
	struct v4l2_subdev *subdev;
	int pad;

	dev_dbg(isp_dev->dev, "Searching for input sub-device...\n");

	pad = port * VISP_PORT_PAD_NR;
	/* Check if this pad is a SINK (input pad) */
	if (!(isp_dev->pads[pad].flags & MEDIA_PAD_FL_SINK)) {
		dev_dbg(isp_dev->dev, "pad %d is not a sink, skipping...\n",
			pad);
		return NULL;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	remote_pad = media_pad_remote_pad_first(&isp_dev->pads[pad]);
#else
	remote_pad = media_entity_remote_pad(&isp_dev->pads[pad]);
#endif

	if (!remote_pad) {
		dev_dbg(isp_dev->dev, "pad %d has no remote connection.\n",
			pad);
		return NULL;
	}

	if (!is_media_entity_v4l2_subdev(remote_pad->entity)) {
		dev_dbg(isp_dev->dev,
			"pad %d remote entity is not a sub-device.\n", pad);
		return NULL;
	}

	subdev = media_entity_to_v4l2_subdev(remote_pad->entity);
	dev_info(isp_dev->dev, "Found input sub-device: %s on pad %d\n",
		 subdev->name, pad);

	return subdev; /* Return the first valid input sub-device found */
}
#endif

/**
 * visp_discover_pipeline_subdevs - Discover all subdevices in the pipeline
 * @isp_dev: ISP device
 * @port: ISP port number
 * @subdevs: Array to store discovered subdevices
 * @max_subdevs: Maximum number of subdevices to discover
 *
 * Traverses the media graph to find all subdevices connected to the ISP pipeline.
 * Returns the number of subdevices found.
 */
static int visp_discover_pipeline_subdevs(struct visp_dev *isp_dev, int port,
					  struct v4l2_subdev **subdevs, int max_subdevs)
{
	struct media_device *mdev = isp_dev->sd.entity.graph_obj.mdev;
	struct media_entity *entity;
	struct media_link *link;
	struct v4l2_subdev *sd;
	int count = 0;
	int i;

	if (!mdev || !subdevs || max_subdevs <= 0)
		return 0;

	dev_info(isp_dev->dev, "=== Discovering pipeline subdevices for port %d ===\n", port);

	/* Walk through all entities in the media device */
	media_device_for_each_entity(entity, mdev) {
		/* Only interested in V4L2 subdevices */
		if (!is_media_entity_v4l2_subdev(entity))
			continue;

		sd = media_entity_to_v4l2_subdev(entity);
		if (!sd || sd == &isp_dev->sd) /* Skip ISP itself */
			continue;

		/* Check if this subdevice is connected to our ISP */
		list_for_each_entry(link, &entity->links, list) {
			struct media_entity *remote_entity = NULL;

			if (link->source->entity == entity &&
			    (link->flags & MEDIA_LNK_FL_ENABLED)) {
				remote_entity = link->sink->entity;
			} else if (link->sink->entity == entity &&
				   (link->flags & MEDIA_LNK_FL_ENABLED)) {
				remote_entity = link->source->entity;
			}

			/* Check if connected to our ISP entity */
			if (remote_entity == &isp_dev->sd.entity) {
				/* Avoid duplicates */
				for (i = 0; i < count; i++) {
					if (subdevs[i] == sd)
						break;
				}

				if (i == count && count < max_subdevs) {
					subdevs[count] = sd;
					dev_info(isp_dev->dev, "  Found subdev[%d]: %s (function: 0x%x)\n",
						 count, sd->name, sd->entity.function);
					count++;
				}
				break;
			}
		}
	}

	dev_info(isp_dev->dev, "=== Pipeline discovery complete: found %d subdevices ===\n", count);
	return count;
}

/**
 * visp_get_pipeline_subdev_count - Get number of subdevices in pipeline for a port
 * @isp_dev: ISP device structure
 * @port: Port number
 *
 * Return: Number of subdevices in the pipeline for the specified port
 */
int visp_get_pipeline_subdev_count(struct visp_dev *isp_dev, int port)
{
	struct v4l2_subdev *subdevs[16]; /* Support up to 16 subdevices */
	int count;

	if (!isp_dev || port < 0 || port >= VISP_PORT_NR) {
		return 0;
	}

	/* Use discovery function to count subdevices */
	count = visp_discover_pipeline_subdevs(isp_dev, port, subdevs, ARRAY_SIZE(subdevs));

	dev_info(isp_dev->dev, "Pipeline discovery for port %d found %d subdevices\n", port, count);

	return count;
}

/**
 * visp_get_pipeline_subdev - Get a specific subdevice from pipeline
 * @isp_dev: ISP device structure
 * @port: Port number
 * @index: Subdevice index in the pipeline
 *
 * Return: Pointer to v4l2_subdev if found, NULL otherwise
 */
struct v4l2_subdev *visp_get_pipeline_subdev(struct visp_dev *isp_dev, int port, int index)
{
	struct v4l2_subdev *subdevs[16]; /* Support up to 16 subdevices */
	int count;

	if (!isp_dev || port < 0 || port >= VISP_PORT_NR || index < 0) {
		return NULL;
	}

	/* Use discovery function to get subdevices */
	count = visp_discover_pipeline_subdevs(isp_dev, port, subdevs, ARRAY_SIZE(subdevs));

	if (index >= count) {
		dev_dbg(isp_dev->dev, "Request for subdev index %d >= count %d for port %d\n",
			index, count, port);
		return NULL;
	}

	dev_dbg(isp_dev->dev, "Returning subdev[%d] = %s for port %d\n",
		index, subdevs[index] ? subdevs[index]->name : "NULL", port);

	return subdevs[index];
}

/**
 * visp_stream_pipeline_subdevs - Stream on/off all subdevices in a pipeline
 * @isp_dev: ISP device structure
 * @port: Port number
 * @enable: 1 to enable streaming, 0 to disable
 *
 * This function calls s_stream on all subdevices in the pipeline for the specified port.
 * It streams in reverse order for stream-on (from sensor to ISP) and forward order for stream-off.
 */
static int visp_stream_pipeline_subdevs(struct visp_dev *isp_dev, int port, int enable)
{
	int i, ret = 0;
	int count = visp_get_pipeline_subdev_count(isp_dev, port);

	dev_info(isp_dev->dev, "=== Pipeline Streaming %s for port %d ===\n",
		 enable ? "ON" : "OFF", port);

	if (count == 0) {
		dev_warn(isp_dev->dev, "No pipeline subdevices found for port %d\n", port);
		return 0;
	}

	dev_info(isp_dev->dev, "Found %d pipeline subdevices for port %d\n", count, port);

	if (enable) {
		/* Stream on: Start from the furthest upstream (sensor) to downstream (ISP) */
		dev_info(isp_dev->dev, "Starting streaming from sensor to ISP (reverse order):\n");
		for (i = count - 1; i >= 0; i--) {
			struct v4l2_subdev *subdev = visp_get_pipeline_subdev(isp_dev, port, i);
			dev_info(isp_dev->dev, "  [%d] Attempting to stream ON: %s\n",
				 i, subdev ? subdev->name : "NULL");

			if (subdev && subdev->ops && subdev->ops->video && subdev->ops->video->s_stream) {
				dev_info(isp_dev->dev, "  [%d] Calling s_stream(1) on %s\n", i, subdev->name);
				ret = subdev->ops->video->s_stream(subdev, 1);
				if (ret) {
					dev_err(isp_dev->dev, "Failed to start streaming on '%s': %d\n",
						subdev->name, ret);
					/* Stream off the devices we already started */
					{
						int j;
						for (j = i + 1; j < count; j++) {
							struct v4l2_subdev *prev_subdev = visp_get_pipeline_subdev(isp_dev, port, j);
							if (prev_subdev && prev_subdev->ops && prev_subdev->ops->video &&
							    prev_subdev->ops->video->s_stream) {
								prev_subdev->ops->video->s_stream(prev_subdev, 0);
							}
						}
					}
					return ret;
				}
				dev_info(isp_dev->dev, "  [%d] Successfully started streaming on '%s'\n", i, subdev->name);
			} else {
				dev_warn(isp_dev->dev, "  [%d] Subdev %s has no s_stream operation\n",
					 i, subdev ? subdev->name : "NULL");
			}
		}
	} else {
		/* Stream off: Start from downstream (ISP) to upstream (sensor) */
		dev_info(isp_dev->dev, "Stopping streaming from ISP to sensor (forward order):\n");
		for (i = 0; i < count; i++) {
			struct v4l2_subdev *subdev = visp_get_pipeline_subdev(isp_dev, port, i);
			dev_info(isp_dev->dev, "  [%d] Attempting to stream OFF: %s\n",
				 i, subdev ? subdev->name : "NULL");

			if (subdev && subdev->ops && subdev->ops->video && subdev->ops->video->s_stream) {
				dev_info(isp_dev->dev, "  [%d] Calling s_stream(0) on %s\n", i, subdev->name);
				ret = subdev->ops->video->s_stream(subdev, 0);
				if (ret) {
					dev_warn(isp_dev->dev, "Failed to stop streaming on '%s': %d\n",
						 subdev->name, ret);
					/* Continue trying to stop other devices */
				} else {
					dev_info(isp_dev->dev, "  [%d] Successfully stopped streaming on '%s'\n", i, subdev->name);
				}
			} else {
				dev_warn(isp_dev->dev, "  [%d] Subdev %s has no s_stream operation\n",
					 i, subdev ? subdev->name : "NULL");
			}
		}
	}

	return 0;
}

static int visp_pad_s_stream(struct v4l2_subdev *sd, void *arg)
{
	struct visp_pad_stream_status *pad_stream =
	    (struct visp_pad_stream_status *)arg;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	int ret = 0;
	int port = pad_stream->pad / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad_stream->pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

	isp_dev->pad_data[pad_stream->pad].stream = pad_stream->status;

	if (pad_stream->status == 0)
		INIT_LIST_HEAD(&isp_dev->pad_data[pad_stream->pad].queue);

	if (pad_stream->status == 1) {

		ret = visp_setup_isp_pipeline(isp_dev, pad_stream->pad);
		if (ret)
			return ret;
		/* ENTER PORT Level CRITICAL SECTION */
		mutex_lock(&isp_dev->rpu->rpu_lock);
		/*
		 * the subdev_streamon_count[port] holds the count of number of
		 * piplines MP/SP/RAW are up on a port
		 */
		if (ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port] == 0) {
			ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port]++;
		/* Stream on all pipeline subdevices */
			ret = visp_stream_pipeline_subdevs(isp_dev, port, 1);
			if (ret) {
				dev_err(isp_dev->dev, "Failed to start pipeline streaming on port %d: %d\n",
					port, ret);
				goto ERR_TO_RPU_LOCK;
			}

		} else {
			ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port]++;
		}

		/* EXIT PORT Level CRITICAL SECTION */

		ret = media_isp_device_set_frame_rate(isp_dev, port,
				&isp_dev->isp_ports[port].sensor_info.frame_rate);
		if (ret != VSI_SUCCESS) {
			dev_err(isp_dev->dev,
				"%s isp:%d port %d chn %d Set frame_rate failed, ret is %d",
				__func__, isp_dev->id, port, chn, ret);
			ret = -EINVAL;
			goto ERR_TO_RPU_LOCK;
		}

		ret = media_isp_device_set_format(isp_dev, port, chn);
		if (ret != 0) {
			dev_err(isp_dev->dev, "%s isp_id : %d FAILED SetFormat\n",
				__func__, isp_dev->id);
			ret = -EINVAL;
			goto ERR_TO_RPU_LOCK;
		}

		ret = media_isp_device_stream_on(isp_dev, port, chn);
		if (ret != 0) {
			dev_err(isp_dev->dev, "%s isp: %d port : %d  FAILED to stream on\n",
				__func__, isp_dev->id, port);
			ret = -EINVAL;
			goto ERR_TO_RPU_LOCK;
		}
		mutex_unlock(&isp_dev->rpu->rpu_lock);
	} else {
		if (ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port]) {
			ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port]--;
			media_isp_stream_off(isp_dev, port, chn);
			/* subdev_streamon_count
			 * if > 0 implies there are other pipelines still processing streams
			 * from this subdev.
			 * if == 0 implies that the last avaialble pipeline has arrived for
			 * stream off and proceeds to perform complete cleanup of input pipeline.
			 */
			if (ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port] == 0) {
				/* Stream off all pipeline subdevices */
				ret = visp_stream_pipeline_subdevs(isp_dev, port, 0);
				if (ret) {
					dev_warn(isp_dev->dev, "Failed to stop pipeline streaming on isp: %d port: %d ret: %d\n",
						 isp_dev->id, port, ret);
				}
			}
		}
		isp_dev->streamon[pad_stream->pad] = 0;
	}

	return ret;

ERR_TO_RPU_LOCK:
	mutex_unlock(&isp_dev->rpu->rpu_lock);
	isp_dev->streamon[pad_stream->pad] = 0;
	return ret;
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

	if (list_empty(&cur_pad->queue)) {
		dev_dbg(isp_dev->dev, "Empty list\n");
		return -EINVAL;
	}
	if ((cur_pad->stream == 0)) {
		dev_dbg(isp_dev->dev, "Streamoff\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&cur_pad->qlock, flags);
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
	if (ext_dma_buf == NULL) {
		dev_err(isp_dev->dev, "%s: buffer alloc kzalloc failed!\n",
			__func__);
		return -ENOMEM;
	}

	ext_dma_buf->vaddr =
	    dma_alloc_coherent(isp_dev->dev, ext_buf_info->plane.size,
			       &(ext_dma_buf->addr), GFP_KERNEL);
	if (ext_dma_buf->vaddr == NULL) {
		dev_err(isp_dev->dev, "%s: failed to alloc dma buffer!\n",
			__func__);
		return -ENOMEM;
	}

	dev_info(isp_dev->dev, "RDMA Buffer is %llx\n",ext_dma_buf->addr);

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
	if (ext_dma_buf == NULL) {
		dev_err(isp_dev->dev, "%s: buffer alloc kzalloc failed!\n",
			__func__);
		return -ENOMEM;
	}

	ext_dma_buf->vaddr =
	    dma_alloc_coherent(isp_dev->dev, ext_buf_info->plane.size,
			       &(ext_dma_buf->addr), GFP_KERNEL);
	if (ext_dma_buf->vaddr == NULL) {
		dev_err(isp_dev->dev, "%s: failed to alloc dma buffer!\n",
			__func__);
		return -ENOMEM;
	}

	dev_info(isp_dev->dev, "ISP:%d Port:%d RDMA Buffer is 0x%llx\n", isp_dev->id, ext_buf_info->port, ext_dma_buf->addr);

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

static long visp_return_rpu_id(struct v4l2_subdev *sd, void *arg)
{
	long ret = 0;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);

	if (!arg)
		dev_err(isp_dev->dev, "%s %d NULL ARG\n", __func__, __LINE__);

	struct isp_rpu *temp = (struct isp_rpu *)arg;

	temp->rpu = isp_dev->isp_rpu;
	temp->isp = isp_dev->id;

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
	/* .s_stream = visp_s_stream, */
};

int media_isp_hal_mbus_fmt_to_media_fmt(uint32_t *code, uint32_t *pixel_format,
					 uint32_t fourcc_code);
static void set_default_pad_config(struct visp_dev *isp_dev)
{
	int pad;
	struct visp_pad_data *pad_data;

	/* Initialize default formats for all pads */
	for (pad = 0; pad < isp_dev->num_pads; pad++) {
		pad_data = &isp_dev->pad_data[pad];

		/* Common format properties */
		pad_data->format.field = V4L2_FIELD_NONE;
		pad_data->format.quantization = V4L2_QUANTIZATION_DEFAULT;
		pad_data->format.width = 1920;
		pad_data->format.height = 1080;

		switch (pad % VISP_PORT_PAD_NR) {
		case VISP_PORT_PAD_SINK:
			/* Sink pads use raw sensor format */
			pad_data->format.code = MEDIA_BUS_FMT_SRGGB12_1X12;
			pad_data->format.colorspace = V4L2_COLORSPACE_RAW;
			dev_dbg(isp_dev->dev, "Init pad %d (sink): %ux%u code=0x%x\n",
				pad, pad_data->format.width, pad_data->format.height,
				pad_data->format.code);
			break;
		case VISP_PORT_PAD_SOURCE_MP:
		case VISP_PORT_PAD_SOURCE_SP1:
		case VISP_PORT_PAD_SOURCE_SP2:
			/* Source pads use processed RGB format */
			pad_data->format.code = MEDIA_BUS_FMT_RGB888_1X24;
			pad_data->format.colorspace = V4L2_COLORSPACE_SRGB;
			break;
		case VISP_PORT_PAD_SOURCE_RAW:
			/* Raw output pads preserve sensor format */
			pad_data->format.code = MEDIA_BUS_FMT_SRGGB12_1X12;
			pad_data->format.colorspace = V4L2_COLORSPACE_RAW;
			break;
		default:
			/* Default to RGB format */
			pad_data->format.code = MEDIA_BUS_FMT_RGB888_1X24;
			pad_data->format.colorspace = V4L2_COLORSPACE_SRGB;
			break;
		}
	}
}

static int visp_set_fmt(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
			struct v4l2_subdev_format *format)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	uint32_t w, h;
	uint32_t sink_pad_index;
	struct visp_pad_data *cur_pad = &isp_dev->pad_data[format->pad];
	struct visp_pad_data *sink_pad;
	int i;
	int ret = 0;
	media_fmt Format_media;
	struct v4l2_mbus_framefmt *MBusFormat;
	struct media_pad *mediapad_t;
	uint32_t fourcc_code = 0;

	ret = visp_setup_isp_pipeline(isp_dev, format->pad);
	if (ret)
		return ret;

	sink_pad_index = format->pad - (format->pad % VISP_PORT_PAD_NR);
	sink_pad = &isp_dev->pad_data[sink_pad_index];

	/* Apply format to the current pad being set */
	cur_pad->sink_detected = 1;
	cur_pad->format = format->format;

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

	memset(&Format_media, 0, sizeof(Format_media));

	MBusFormat = (struct v4l2_mbus_framefmt *)&format->format;
	Format_media.width = MBusFormat->width;
	Format_media.height = MBusFormat->height;
	Format_media.color_space = MBusFormat->colorspace;
	Format_media.quantization = MBusFormat->quantization;
	fourcc_code = 0;

	if (sizeof(MBusFormat->reserved) == (sizeof(uint16_t) * 10)) {
		memcpy(&fourcc_code, &MBusFormat->reserved,
		       sizeof(fourcc_code));
	} else {
		memcpy(&fourcc_code, &MBusFormat->reserved[1],
		       sizeof(fourcc_code));
	}
	media_isp_hal_mbus_fmt_to_media_fmt(
	    &MBusFormat->code, &Format_media.pixel_format, fourcc_code);

	mediapad_t = &isp_dev->pads[format->pad];
	ret = media_isp_set_format(isp_dev, mediapad_t->index, Format_media);
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
	int ret;
	media_fmt Format_media;
	struct v4l2_mbus_framefmt *MBusFormat;
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

			switch (i) {
			case VISP_PORT_PAD_SOURCE_MP:
			case VISP_PORT_PAD_SOURCE_SP1:
			case VISP_PORT_PAD_SOURCE_SP2:
			case VISP_PORT_PAD_SOURCE_RAW:
				source_pad->format = format->format;
				source_pad->format.code =
				    source_pad->fmts[0].code;
				memcpy(source_pad->format.reserved,
				       &source_pad->fmts[0].fourcc,
				       sizeof(uint32_t));
				source_pad->format.field = V4L2_FIELD_NONE;
				source_pad->format.quantization =
				    V4L2_QUANTIZATION_DEFAULT;
				source_pad->format.colorspace =
				    V4L2_COLORSPACE_DEFAULT;
				break;
			default:
				break;
			}
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

	if (ret)
		return ret;

	memset(&Format_media, 0, sizeof(Format_media));

	MBusFormat = (struct v4l2_mbus_framefmt *)&format->format;
	Format_media.width = MBusFormat->width;
	Format_media.height = MBusFormat->height;
	Format_media.color_space = MBusFormat->colorspace;
	Format_media.quantization = MBusFormat->quantization;
	fourcc_code = 0;

	if (sizeof(MBusFormat->reserved) == (sizeof(uint16_t) * 10)) {
		memcpy(&fourcc_code, &MBusFormat->reserved,
		       sizeof(fourcc_code));
	} else {
		memcpy(&fourcc_code, &MBusFormat->reserved[1],
		       sizeof(fourcc_code));
	}
	media_isp_hal_mbus_fmt_to_media_fmt(
	    &MBusFormat->code, &Format_media.pixel_format, fourcc_code);

	mediapad_t = &isp_dev->pads[format->pad];
	media_isp_set_format(isp_dev, mediapad_t->index, Format_media);

	cur_pad->format = format->format;

	return 0;
}

static int visp_get_fmt(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
			struct v4l2_subdev_format *format)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_data *pad_data = &isp_dev->pad_data[format->pad];
	uint32_t fourcc;

	format->format = pad_data->format;

	/* Find the corresponding fourcc for this mbus format */
	fourcc = visp_mbus_to_fourcc(pad_data->format.code);

	memcpy(format->format.reserved, &fourcc, sizeof(fourcc));

	return 0;
}

static int visp_enum_mbus_code(struct v4l2_subdev *sd,
			       struct v4l2_subdev_state *sd_state,
			       struct v4l2_subdev_mbus_code_enum *code)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_data *pad_data = &isp_dev->pad_data[code->pad];

	visp_setup_isp_pipeline(isp_dev, code->pad);

	if (code->index >= pad_data->num_formats)
		return 0;

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

int sensor_pipeline_init(struct visp_dev *isp_dev);
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
	struct visp_ext_dma_buf *pos, *next;
	int port = 0;

	mutex_lock(&isp_dev->mlock);

	isp_dev->refcnt--;

	if (isp_dev->refcnt == 0) {
		for (port = 0; port < VISP_PORT_NR; port++) {
			list_for_each_entry_safe(
			    pos, next, &isp_dev->mcm_input[port], entry) {
				if (pos) {
					dma_free_coherent(isp_dev->dev,
							  pos->size, pos->vaddr,
							  pos->addr);
					list_del(&pos->entry);
					kfree(pos);
				}
			}
		}
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

/* Forward declaration */
#if 0  // Disable unused function
static struct media_entity *visp_find_entity_by_fwnode(struct visp_dev *isp_dev,
							struct fwnode_handle *fwnode);
#endif

/**
 * visp_find_entity_by_fwnode - Find media entity by its fwnode
 * @isp_dev: ISP device structure
 * @fwnode: Firmware node to search for
 *
 * Returns: Pointer to media entity if found, NULL otherwise
 */
#if 0  // Disable unused function
static struct media_entity *visp_find_entity_by_fwnode(struct visp_dev *isp_dev,
						       struct fwnode_handle *fwnode)
{
	struct media_device *mdev = isp_dev->sd.entity.graph_obj.mdev;
	struct media_entity *entity;
	struct v4l2_subdev *subdev;

	if (!mdev || !fwnode)
		return NULL;

	media_device_for_each_entity(entity, mdev) {
		if (is_media_entity_v4l2_subdev(entity)) {
			subdev = media_entity_to_v4l2_subdev(entity);
			/* Direct fwnode comparison */
			if (subdev->fwnode == fwnode) {
				return entity;
			}
		}
	}

	return NULL;
}
#endif

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

	if (!sd->fwnode) {
		dev_err(dev,
			"Subdev %s has no fwnode, skipping link creation\n",
			sd->name);
		return -EINVAL;
	}

	/* Loop through all available endpoints */
	while ((ep = fwnode_graph_get_next_endpoint(sd->fwnode, ep))) {
		ret = v4l2_fwnode_parse_link(ep, &link);
		if (ret < 0) {
			dev_err(dev, "Failed to parse link for %pOF: %d\n",
				to_of_node(ep), ret);
			continue;
		}

		/* Ensure the subdev is actually a source */
		if (sd->entity.pads[link.local_port].flags &
		    MEDIA_PAD_FL_SINK) {
			v4l2_fwnode_put_link(&link);
			continue;
		}

		source = &sd->entity; // The remote source (MIPI)
		source_pad =
		    link.local_port; // The pad number on the source subdev
		sink = &isp_dev->sd.entity;  // The ISP subdev
		sink_pad = link.remote_port; // The corresponding pad on the ISP

		v4l2_fwnode_put_link(&link);

		/* Create media pad link */
		ret = media_create_pad_link(source, source_pad, sink, sink_pad,
					    MEDIA_LNK_FL_ENABLED);
		if (ret) {
			dev_err(dev, "Failed to create link: %s:%u -> %s:%u\n",
				source->name, source_pad, sink->name, sink_pad);
			break;
		}

		dev_info(dev, "Successfully linked %s:%u -> %s:%u\n",
			 source->name, source_pad, sink->name, sink_pad);

		/* Update ports_mask */
		fwnode_handle_put(ep);
	}

	return ret;
}

static void visp_notifier_unbound(struct v4l2_async_notifier *notifier,
				  struct v4l2_subdev *sd,
				  struct v4l2_async_connection *asc)
{
	struct visp_dev *isp_dev = container_of(notifier, struct visp_dev, notifier);

	dev_dbg(isp_dev->dev, "Notifier unbound: subdev %s\n", sd->name);
}

static int visp_notifier_complete(struct v4l2_async_notifier *notifier)
{
	struct visp_dev *isp_dev = container_of(notifier, struct visp_dev, notifier);
	int i;

	dev_info(isp_dev->dev, "=== Async notifier complete - all subdevices bound ===\n");
	dev_info(isp_dev->dev, "ISP entity: %s (num_pads=%u)\n",
		 isp_dev->sd.entity.name, isp_dev->sd.entity.num_pads);

	/* Print ISP pad information */
	for (i = 0; i < isp_dev->num_pads; i++) {
		dev_info(isp_dev->dev, "  ISP pad[%d]: %s\n", i,
			 (isp_dev->pads[i].flags & MEDIA_PAD_FL_SOURCE) ? "source" : "sink");
	}

	/* Now that all subdevices are bound, discover the complete pipeline */
	dev_info(isp_dev->dev, "=== Pipeline discovery after all bindings ===\n");

	return 0;
}

static const struct v4l2_async_notifier_operations visp_notify_ops = {
	.bound = visp_notifier_bound,
	.unbind = visp_notifier_unbound,
	.complete = visp_notifier_complete,
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
	int num_pads = visp_get_num_pads(isp_dev->num_streams);

	/* Allocate pads dynamically based on num_streams */
	isp_dev->num_pads = num_pads;
	isp_dev->pads = devm_kzalloc(isp_dev->dev, sizeof(struct media_pad) * num_pads, GFP_KERNEL);
	if (!isp_dev->pads) {
		dev_err(isp_dev->dev, "Failed to allocate pads\n");
		return -ENOMEM;
	}

	isp_dev->pad_data = devm_kzalloc(isp_dev->dev, sizeof(struct visp_pad_data) * num_pads, GFP_KERNEL);
	if (!isp_dev->pad_data) {
		dev_err(isp_dev->dev, "Failed to allocate pad_data\n");
		return -ENOMEM;
	}

	for (pad = 0; pad < num_pads; pad++) {
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

	/* Initialize default formats for all pads */
	set_default_pad_config(isp_dev);

	return 0;
}

/*
 * Parse IBA parameters
 */
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
		if (of_property_read_u32(np, property_name,
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

		dev_dbg(isp_dev->dev,
			"IBA%d: ppc=%d, vcid=%d, frame_rate=%d, data_format=%d, max_width=%d, max_height=%d\n",
			iba_index, isp_dev->iba[iba_index].ppc,
			isp_dev->iba[iba_index].vcid,
			isp_dev->iba[iba_index].frame_rate,
			isp_dev->iba[iba_index].data_format,
			isp_dev->iba[iba_index].max_width,
			isp_dev->iba[iba_index].max_height);
	}

	return 0;
}

/*
 * Parse device tree parameters
 */
static int visp_parse_params(struct visp_dev *isp_dev,
			     struct platform_device *pdev)
{
	int port = 0;
	int ret = 0;
	struct device_node *node = pdev->dev.of_node;

	for (port = 0; port < VISP_PORT_NR; port++) {
		strncpy(isp_dev->isp_ports[port].sensor_info.name,
			VISP_DEFAULT_SENSOR, strlen(VISP_DEFAULT_SENSOR) + 1);

		strncpy(isp_dev->isp_ports[port].sensor_info.calib_xml,
			VISP_DEFAULT_SENSOR_XML,
			strlen(VISP_DEFAULT_SENSOR_XML) + 1);

		isp_dev->isp_ports[port].sensor_info.mode =
		    VISP_DEFAULT_SENSOR_MODE;
		isp_dev->isp_ports[port].sensor_info.sensor_id =
		    sensor_dev_id[port];

		strncpy(isp_dev->isp_ports[port].sensor_info.manu_json,
			VISP_DEFAULT_SENSOR_MANU_JSON,
			strlen(VISP_DEFAULT_SENSOR_MANU_JSON) + 1);

		strncpy(isp_dev->isp_ports[port].sensor_info.auto_json,
			VISP_DEFAULT_SENSOR_AUTO_JSON,
			strlen(VISP_DEFAULT_SENSOR_AUTO_JSON) + 1);
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

	/* Read string property for SS-MODE-i0 (LIMO, etc.) */
	ret = of_property_read_string(node, "xlnx,io_mode", &isp_dev->ss_mode_i0);
	if (ret) {
		dev_err(&pdev->dev, "Failed to read xlnx,io_mode\n");
		return ret;
	} else {
		dev_dbg(&pdev->dev, "xlnx,io_mode: %s\n", isp_dev->ss_mode_i0);
	}

	/* Read stream info (multi-stream, single-stream) */
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
	int i;

	if (num_mems < 0) {
		dev_err(isp_dev->dev, "%s no memory for calibration\n",
			__func__);
		return -ENOMEM;
	}

	for (i = 0; i < num_mems; i++) {
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
	init_completion(&isp_dev->apu_wait_for_ack);
	init_completion(&isp_dev->apu_wait_for_data);

	if (!isp_dev->rpu->tx_chan || !isp_dev->rpu->rx_chan) {
		dev_err(isp_dev->dev, "No TX or RX Channel found on RPU: %d\n",
			isp_dev->isp_rpu);
		return -ENOMEM;
	}

	isp_dev->tx_chan = isp_dev->rpu->tx_chan;
	isp_dev->rx_chan = isp_dev->rpu->rx_chan;
	isp_dev->rpu->isp_dev[isp_dev->id] = isp_dev;
	/*
	 * Assigning isp_dev structure value to isp_dev present in
	 * rpu_dev struct
	 */

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
		devm_kzalloc(&pdev->dev, sizeof(struct visp_limo_isp_dev_extended), GFP_KERNEL);
	if (!isp_dev->extended_struct)
		return -ENOMEM;

	mutex_init(&isp_dev->mlock);
	mutex_init(&isp_dev->calib_lock);
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

	INIT_KFIFO(isp_dev->display_fifo);

	v4l2_subdev_init(&isp_dev->sd, &visp_subdev_ops);
	snprintf(isp_dev->sd.name, VISP_SUBDEV_NAME_SIZE, "%s.%d", VISP_NAME,
		 isp_dev->id);

	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	isp_dev->sd.dev = &pdev->dev;
	isp_dev->sd.owner = THIS_MODULE;
	isp_dev->sd.internal_ops = &visp_internal_ops;
	isp_dev->sd.entity.ops = &visp_entity_ops;
	isp_dev->sd.entity.function = MEDIA_ENT_F_PROC_VIDEO_ISP;
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
		dev_err(dev, "Failed to register V4L2 async subdev: %d\n", ret);
		goto err_async_register_subdev;
	}

	dev_info(dev, "ISP subdev registered successfully with name: %s\n", isp_dev->sd.name);
	dev_info(dev, "ISP entity function: 0x%x, obj_type: %d\n",
		 isp_dev->sd.entity.function, isp_dev->sd.entity.obj_type);
	dev_info(dev, "ISP subdev flags: 0x%x\n", isp_dev->sd.flags);

	dev_info(dev, "ISP: Subdevice registered: %s (entity: %s)\n",
		 isp_dev->sd.name, isp_dev->sd.entity.name);
	dev_info(dev, "ISP: Entity function: 0x%x, pads: %u\n",
		 isp_dev->sd.entity.function, isp_dev->sd.entity.num_pads);

	/* Assign the XML path to sensor_info[0].xml */
	ret = visp_procfs_register(isp_dev, &isp_dev->pde);
	if (ret) {
		dev_err(dev, "register procfs failed.\n");
		goto err_register_procfs;
	}

	isp_dev->event_shm.virt_addr = (void *)__get_free_pages(GFP_KERNEL, 3);
	isp_dev->event_shm.size = PAGE_SIZE * 8;
	memset(isp_dev->event_shm.virt_addr, 0, isp_dev->event_shm.size);
	isp_dev->event_shm.phy_addr =
	    virt_to_phys(isp_dev->event_shm.virt_addr);
	mutex_init(&isp_dev->event_shm.event_lock);

	isp_dev->reserve_mem.va =
	    ioremap_wc(isp_dev->reserve_mem.pa, isp_dev->reserve_mem.size);

	visp_ctrl_init(isp_dev);

	for (port = 0; port < VISP_PORT_NR; port++) {
		mutex_init(&isp_dev->port_lock[port]);
		INIT_LIST_HEAD(&isp_dev->mcm_input[port]);
	}

	isp_dev->ports_mask = isp_dev->num_streams;

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret) {
		dev_err(&pdev->dev, "dma_set_mask_and_coherent: %d\n", ret);
		/* goto error; */
	}

	/* Register Callback function */
	isp_dev->frameout_cb = handle_frameout_buffer;
	/* sensor_pipeline_init(isp_dev); */

	dev_info(&pdev->dev, "visp isp driver probe success\n");

	return 0;

err_register_procfs:
	v4l2_async_unregister_subdev(&isp_dev->sd);

err_async_register_subdev:
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
	free_pages((unsigned long)isp_dev->event_shm.virt_addr, 3);
	visp_ctrl_destroy(isp_dev);
	dev_info(&pdev->dev, "visp isp driver remove\n");

}

static const struct dev_pm_ops visp_pm_ops = {};

static const struct of_device_id visp_of_match[] = {
	{
		.compatible = "xlnx,visp-ss-limo-1.0",
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
	}
};

static int __init visp_init_module(void)
{
	int ret;

	ret = platform_driver_register(&visp_driver);
	if (ret) {
		pr_err("Failed to register isp driver\n");
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
