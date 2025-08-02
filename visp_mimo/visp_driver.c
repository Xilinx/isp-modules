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
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_graph.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/videobuf2-dma-contig.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-ctrls.h>

#include "visp_v4l2_common.h"
#include "visp_driver.h"
#include "visp_event.h"
#include "visp_ctrl.h"
#include "visp_procfs.h"
#include "visp_v4l2_std_exts.h"
#include <linux/interrupt.h>
#include <linux/delay.h>
#include "sensor_cmd.h"
#include "mbox_api.h"
#include "mbox_cmd.h"
#include "visp_app.h"
#include "visp_mbox_driver.h"

#define VISP_DEFAULT_SENSOR "ox03f10"
#define VISP_DEFAULT_SENSOR_MODE 0
#define VISP_DEFAULT_SENSOR_XML "/run/media/mmcblk0p1/OX03f10.xml"
#define VISP_DEFAULT_SENSOR_MANU_JSON "/run/media/mmcblk0p1/manual_ext.json"
#define VISP_DEFAULT_SENSOR_AUTO_JSON "/run/media/mmcblk0p1/auto.json"

static uint32_t sensor_dev_id[VISP_PORT_NR] = {9, 2, 5, 10};
int media_isp_device_dq_buf_out(struct visp_dev *isp_dev, struct Chn_info *info,
				media_buf *buf, void *packet_from_rpu,
				media_buffer_t *p_media_buffer);
int visp_buf_done(struct v4l2_subdev *sd, void *arg);

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
	strncpy(cap->card, sd->name, sizeof(cap->card));
	cap->driver[sizeof(cap->driver) - 1] = '\0';
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

	if (isp_dev->streamon[pad_buf->pad] == 0) {
		memcpy(&IspChn->bufs[buf.index], &buf, sizeof(media_buf));
	} else {
		media_buffer_t *p_media_buffer = VSI_NULL;

		p_media_buffer = IspChn->cam_device_bufs[buf.index];

		if (p_media_buffer == VSI_NULL) {
			loge("CamDevice queue buf is null");
			return VSI_ERR_NULL_PTR;
		}

		ret_val = vsi_cam_device_en_que_buffer(
			isp_dev, isp_port->cam_device_handle, chn, p_media_buffer);
		if (ret_val != VSI_SUCCESS) {
			loge("CamDevice queue buf failed, ret is %d", ret_val);
			return VSI_ERR_TIMEOUT;
		}
	}

	return ret_val;
}

int media_isp_hal_buf_done(struct v4l2_subdev *sd, int pad, media_buf *buf);

int media_isp_device_dqbuf(struct visp_dev *isp_dev, struct Chn_info *info,
			   media_buf *buf, void *enque_buff_g,
			   media_buffer_t *p_media_buffer);

int handle_frameout_buffer_mimo(struct visp_dev *isp_dev)
{
	media_buf *buf = NULL;
	media_buffer_t *p_media_buffer = NULL;
	struct Chn_info info;
	mbox_post_msg *msg;
	size_t size;
	int ret = 0;
	void *packet_from_rpu;

	/* Validate inputs */
	if (!isp_dev) {
		dev_err(isp_dev->dev,
			"handle_frameout_buffer: isp_dev is NULL\n");
		ret = -EINVAL;
		goto error;
	}

	if (!kfifo_out(&isp_dev->display_fifo, &msg, 1)) {
		pr_err("Failed to queue into kfifo\n");
		ret = -ENOMEM;
		goto error;
	}

	size = msg->size;
	packet_from_rpu = kzalloc(size, GFP_KERNEL);
	if (!packet_from_rpu) {
		dev_err(isp_dev->dev, "Failed to allocate packet_from_rpu\n");
		ret = -ENOMEM;
		goto error;
	}

	memcpy(packet_from_rpu, msg->payload, size);

	/* Allocate memory for the buffer */
	p_media_buffer = kzalloc(sizeof(media_buffer_t), GFP_KERNEL);
	if (!p_media_buffer) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		ret = -ENOMEM;
		goto error;
	}

	buf = kzalloc(sizeof(media_buf), GFP_KERNEL);
	if (!buf) {
		dev_err(isp_dev->dev, "handle_frameout_buffer: Failed to "
					  "allocate memory for media_buf\n");
		ret = -ENOMEM;
		goto error;
	}

	/* Dequeue buffer from the ISP device*/
	ret = media_isp_device_dq_buf_out(isp_dev, &info, buf, packet_from_rpu,
					  p_media_buffer);
	if (ret != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"handle_frameout_buffer: MediaIspDeviceDqbuf failed "
			"with error %d\n",
			ret);
		goto error;
	}

	wake_up(&isp_dev->wq_frame_done_finished);

error:
	if(msg)
		kfree(msg);
	kfree(buf);
	kfree(packet_from_rpu);
	return ret;
}
EXPORT_SYMBOL(handle_frameout_buffer_mimo);

//
int media_isp_device_camera_connect(struct visp_dev *isp_dev, uint8_t index);
int media_isp_device_set_format(struct visp_dev *isp_dev, uint8_t port,
				uint8_t chn);
int media_isp_device_stream_on(struct visp_dev *isp_dev, uint8_t port,
				   uint8_t chn);
int media_isp_stream_off(struct visp_dev *isp_dev, uint8_t port, uint8_t chn);
int isp_device_create(struct visp_dev *isp_dev, uint8_t port);

int media_isp_device_set_frame_rate(struct visp_dev *isp_dev, uint8_t port,
					int *frame_rate);
int media_isp_set_frame_rate(struct visp_dev *isp_dev, int pad,
				 int *frame_rate);

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

	if (pad_stream->status == 1) {// streamon
		/*ENTER PORT Level CRITICAL SECITON*/
		mutex_lock(&isp_dev->port_lock[port]);

		if (isp_dev->isp_ports[port].camera_connect_ref_cnt == 0) {

			isp_dev->isp_ports[port].camera_connect_ref_cnt++;

#ifdef LOAD_CALIB_ENABLE
			ret = visp_l_calib_event(isp_dev, pad_stream->pad);
			if (ret != 0) {
				dev_err(isp_dev->dev, "[EVENT_FAIL] %s %d\n",
					__func__, __LINE__);
				return ret;
			}
#endif

			ret = media_isp_device_camera_connect(isp_dev,
								  pad_stream->pad);
			if (ret != 0) {
				dev_err(isp_dev->dev,
					"%s %d FAiled camera connect\n",
					__func__, __LINE__);
				mutex_unlock(&isp_dev->port_lock[port]);
				return ret;
			}

#ifdef LOAD_CALIB_ENABLE
			ret = visp_l_json_event(isp_dev, pad_stream->pad);
			if (ret != 0) {
				dev_err(isp_dev->dev, "[EVENT_FAIL] %s %d\n",
					__func__, __LINE__);
				dev_err(isp_dev->dev, "[EVENT_FAIL] %s %d\n",
					__func__, __LINE__);
				dev_err(isp_dev->dev, "[EVENT_FAIL] %s %d\n",
					__func__, __LINE__);
				return ret;
			}
#endif
		} else {
			isp_dev->isp_ports[port].camera_connect_ref_cnt++;
		}

		/*EXIT PORT Level CRITICAL SECITON*/
		mutex_unlock(&isp_dev->port_lock[port]);

		/*Set Frame Rate */
		ret = media_isp_device_set_frame_rate(
			isp_dev, port,
			&isp_dev->isp_ports[port].sensor_info.frame_rate);
		if (ret != VSI_SUCCESS) {
			dev_err(
				isp_dev->dev,
				"port %d chn %d Set frame_rate failed, ret is %d",
				port, chn, ret);
			goto ERR_TO_CAMERA_DISCONNECT;
		}

		dev_info(isp_dev->dev, "%s %d Camerrefcnt  =%d\n", __func__,
			 __LINE__,
			 isp_dev->isp_ports[port].camera_connect_ref_cnt);
		ret = media_isp_device_set_format(isp_dev, port, chn);
		if (ret != 0) {
			dev_err(isp_dev->dev, "%s %d FAILED SetFormat\n",
				__func__, __LINE__);
			goto ERR_TO_CAMERA_DISCONNECT;
		}

		ret = media_isp_device_stream_on(isp_dev, port, chn);
		if (ret != 0) {
			dev_err(isp_dev->dev, "%s %d FAILED to stream  on\n",
				__func__, __LINE__);
			goto ERR_TO_CAMERA_DISCONNECT;
		}

	}

	else {// streamoff

		media_isp_stream_off(isp_dev, port, chn);
		isp_dev->streamon[pad_stream->pad] = 0;
	}

	return ret;

ERR_TO_CAMERA_DISCONNECT:
	media_isp_device_camera_dis_connect(isp_dev, port, chn);
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
	struct v4l2_subdev *subdev;
	struct video_device *video;
	struct visp_pad_buf pad_buf;
	int ret;

	memcpy(&ubuf, arg, sizeof(struct visp_buf));
	cur_pad = &isp_dev->pad_data[ubuf.pad];
	if (list_empty(&cur_pad->queue))
		return -EINVAL;
	if ((cur_pad->stream == 0))
		return -EINVAL;

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
		if (is_media_entity_v4l2_subdev(pad->entity)) {
			subdev = media_entity_to_v4l2_subdev(pad->entity);
			pad_buf.pad = pad->index;
			pad_buf.buf = buf;
			ret = v4l2_subdev_call(subdev, core, ioctl,
						   VISP_PAD_BUF_DONE, &pad_buf);
			if (ret)
				return ret;

		} else if (is_media_entity_v4l2_video_device(pad->entity)) {
			video = media_entity_to_video_device(pad->entity);
			if (buf->sequence < video->queue->max_num_buffers) {
				if (buf->vb.vb2_buf.state ==
					VB2_BUF_STATE_ACTIVE) {
					vb2_buffer_done(&buf->vb.vb2_buf,
							VB2_BUF_STATE_DONE);
				}
			}
		}
	}

	return 0;
}

static int visp_queryctrl(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_queryctrl *pad_querctrl =
		(struct visp_pad_queryctrl *)arg;
	logs();
	ret = v4l2_queryctrl(&isp_dev->ctrl_handler, pad_querctrl->query_ctrl);

	return ret;
}

static int visp_query_ext_ctrl(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_query_ext_ctrl *pad_quer_ext_ctrl =
		(struct visp_pad_query_ext_ctrl *)arg;
	logs();
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
	logs();
	ret = v4l2_querymenu(&isp_dev->ctrl_handler, pad_quermenu->querymenu);

	return ret;
}

static int visp_g_ctrl(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_control *pad_ctrl = (struct visp_pad_control *)arg;

	logs();

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

	logs();

	mutex_lock(&isp_dev->ctrl_lock);
	isp_dev->ctrl_pad = pad_ctrl->pad;
	ret = v4l2_s_ctrl(NULL, &isp_dev->ctrl_handler, pad_ctrl->control);
	mutex_unlock(&isp_dev->ctrl_lock);

	return ret;
}

static int visp_g_ext_ctrls(struct v4l2_subdev *sd, void *arg)
{
	int ret;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_ext_controls *pad_ext_ctrls =
		(struct visp_pad_ext_controls *)arg;
	logs();

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
	logs();

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
	logs();
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

	logs();
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
		dev_info(isp_dev->dev, "%s: dma_addr: 0x%x is free\n", __func__,
			 (uint32_t)ext_dma_buf->addr);
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

	dev_err(isp_dev->dev, "%s: allocated dma buffer %p %llx\n", __func__,
		ext_dma_buf->vaddr, ext_dma_buf->addr);

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

static long visp_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd,
				void *arg)
{

	int ret = -EINVAL;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);

	switch (cmd) {
	case VIDIOC_QUERYCAP:

		dev_info(isp_dev->dev, " Case VIDIOC_QUERYCAP\n");
		ret = visp_querycap(sd, arg);
		break;
	case VISP_PAD_REQUBUFS:
		dev_info(isp_dev->dev, "case VISP_PAD_REQUBUFS\n");
		ret = visp_pad_requbufs(sd, arg);
		break;
	case VISP_PAD_BUF_QUEUE:
		dev_info(isp_dev->dev, "ISP_DRV case VISP_PAD_BUF_QUEUE\n");
		ret = visp_pad_buf_queue(sd, arg);
		break;
	case VISP_PAD_S_STREAM:
		dev_info(isp_dev->dev, "VISP_PAD_S_STREAM\n");
		ret = visp_pad_s_stream(sd, arg);
		break;
	case VISP_IOC_BUFDONE:
		dev_info(isp_dev->dev, "case VISP_IOC_BUFDONE\n");
		ret = visp_buf_done(sd, arg);
		break;
	case VISP_PAD_QUERYCTRL:
		dev_info(isp_dev->dev, "case VISP_PAD_QUERYCTRL\n");
		ret = visp_queryctrl(sd, arg);
		break;
	case VISP_PAD_QUERY_EXT_CTRL:
		dev_info(isp_dev->dev, "   case VISP_PAD_QUERY_EXT_CTRL\n");
		ret = visp_query_ext_ctrl(sd, arg);
		break;
	case VISP_PAD_G_CTRL:
		dev_info(isp_dev->dev, " case VISP_PAD_G_CTRL\n");
		ret = visp_g_ctrl(sd, arg);
		break;
	case VISP_PAD_S_CTRL:
		dev_info(isp_dev->dev, "case VISP_PAD_S_CTRL\n");
		ret = visp_s_ctrl(sd, arg);
		break;
	case VISP_PAD_G_EXT_CTRLS:
		dev_info(isp_dev->dev, "case VISP_PAD_G_EXT_CTRLS\n");
		ret = visp_g_ext_ctrls(sd, arg);
		break;
	case VISP_PAD_S_EXT_CTRLS:
		dev_info(isp_dev->dev, "case VISP_PAD_S_EXT_CTRLS\n");
		ret = visp_s_ext_ctrls(sd, arg);
		break;
	case VISP_PAD_TRY_EXT_CTRLS:
		dev_info(isp_dev->dev, "case VISP_PAD_TRY_EXT_CTRLS\n");
		ret = visp_try_ext_ctrls(sd, arg);
		break;
	case VISP_PAD_QUERYMENU:
		dev_info(isp_dev->dev, "case VISP_PAD_QUERYMENU\n");
		ret = visp_querymenu(sd, arg);
		break;
	case VISP_IOC_BUFFER_ALLOC:
		ret = visp_buffer_alloc(sd, arg);
		break;
	case VISP_IOC_BUFFER_FREE:
		ret = visp_buffer_free(sd, arg);
		break;
	default:
		break;
	}
	return ret;
}

int visp_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			 struct v4l2_event_subscription *sub);
int visp_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
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
	int frame_rate = 0.;

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
				   struct v4l2_subdev_frame_interval *fi);
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
	/*.s_stream = visp_s_stream,*/
};

int media_isp_hal_mbus_fmt_to_media_fmt(uint32_t *code, uint32_t *pixel_format,
					uint32_t fourcc_code);
int media_isp_set_format(struct visp_dev *isp_dev, uint32_t pad_index,
			 /*  media_pad_attr *pad,*/ media_fmt * format);

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
	media_isp_set_format(isp_dev, mediapad_t->index, &Format_media);
	if (ret)
		return ret;

	cur_pad->format = format->format;

	return 0;
}

int visp_set_fmt_public(struct visp_dev *isp_dev,
			struct v4l2_subdev_format *format);
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
	media_isp_set_format(isp_dev, mediapad_t->index, &Format_media);

	cur_pad->format = format->format;

	return 0;
}

static int visp_get_fmt(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
			struct v4l2_subdev_format *format)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_data *pad_data = &isp_dev->pad_data[format->pad];

	if (pad_data->sink_detected)
		format->format = pad_data->format;
	else
		return -EINVAL;

	return 0;
}

static int visp_enum_mbus_code(struct v4l2_subdev *sd,
				   struct v4l2_subdev_state *sd_state,
				   struct v4l2_subdev_mbus_code_enum *code)
{
	int ret_val;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_data *pad_data = &isp_dev->pad_data[code->pad];

	int index = isp_dev->pads[code->pad].index;
	int port = index / MEDIA_ISP_PORT_PAD_COUNT;

	if (code->index >= pad_data->num_formats)
		return -EINVAL;

	code->code = pad_data->fmts[code->index].code;

	/*Create Instance*/

	/* ENTER port Level CRITICAL SECTION*/

	mutex_lock(&isp_dev->port_lock[port]);

	// camdevice_create;
	if (!isp_dev->isp_ports[port].ref_count) {

		media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
		// cam_device_config_t* CamConfig;

		if (isp_port->cam_device_handle) {
			/*Exit port Level Critical Section */
			mutex_unlock(&isp_dev->port_lock[port]);
			return VSI_SUCCESS;
		}

		if (!isp_dev->isp_ports[port].ref_count) {

			if (isp_dev->num_streams == 1) {
				dev_err(isp_dev->dev,
					"EXECUTING NON-MCM ports_mask=%d\n",
					isp_dev->ports_mask);
				ret_val = isp_device_create(isp_dev, port);
				if (ret_val != VSI_SUCCESS) {
					mutex_unlock(&isp_dev->port_lock[port]);
					dev_err(
						isp_dev->dev,
						"CamDevice Creat Isp , ret is %d",
						ret_val);
					return ret_val;
				}
			} else if (isp_dev->num_streams > 1) {
				dev_err(isp_dev->dev,
					"EXECUTING MCM ports_mask=%d\n",
					isp_dev->ports_mask);
				if (isp_dev->ports_mask < 0x2)
					pr_err("%s %d\n", __func__, __LINE__);

				ret_val = isp_device_create(isp_dev, port);
				if (ret_val != VSI_SUCCESS) {
					mutex_unlock(&isp_dev->port_lock[port]);
					dev_err(
						isp_dev->dev,
						"CamDevice Creat Isp , ret is %d",
						ret_val);
					return ret_val;
				}
			} else {
				dev_info(
					isp_dev->dev,
					"Check the mode %d (1:Non-MCM 2:MCM)\n",
					isp_dev->num_streams);
			}
		}
	}
	isp_dev->isp_ports[port].ref_count++;

	/*Exit port Level Critical Section */
	mutex_unlock(&isp_dev->port_lock[port]);
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
					dev_info(isp_dev->dev,
						 "%s: dma_addr: 0x%x is free\n",
						 __func__, (uint32_t)pos->addr);
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
		dev_err(dev, "try to create %s:%u -> %s:%u link\n",
			source->name, source_pad, sink->name, sink_pad);
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
	//   v4l2_sync_nf_init(&isp_dev->notifier);
	v4l2_async_subdev_nf_init(&isp_dev->notifier, &isp_dev->sd);
#else
	v4l2_async_notifier_init(&isp_dev->notifier);
#endif

	isp_dev->notifier.ops = &visp_notify_ops;

	if (dev_fwnode(isp_dev->dev) == NULL)
		return 0;

	for (pad = 0; pad < VISP_PAD_NR; pad++) {

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

int visp_pads_init(struct visp_dev *isp_dev);
int visp_pads_init(struct visp_dev *isp_dev)
{
	int pad = 0;

	for (pad = 0; pad < VISP_PAD_NR; pad++) {
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
EXPORT_SYMBOL(visp_pads_init);
//
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

		if (isp_dev->id == 0) {
			iba_index = i;
		} else if (isp_dev->id == 1) {
			iba_index = (num_streams == 1) ? 4 : 3 + i;
		} else {
			dev_err(isp_dev->dev, "Unsupported isp_id: %d\n",
				isp_dev->id);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name), "xlnx,iba%d_ppc",
			 iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[i].ppc)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,iba%d_vcid", iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[i].vcid)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,iba%d_frame_rate", iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[i].frame_rate)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,iba%d_data_format", iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[i].data_format)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,iba%d_max-width", iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[i].max_width)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		snprintf(property_name, sizeof(property_name),
			 "xlnx,iba%d_max-height", iba_index);
		if (of_property_read_u32(np, property_name,
					 &isp_dev->iba[i].max_height)) {
			dev_err(isp_dev->dev, "Failed to read %s\n",
				property_name);
			return -EINVAL;
		}

		dev_info(isp_dev->dev,
			 "IBA%d: ppc=%d, vcid=%d, frame_rate=%d, "
			 "data_format=%d, max_width=%d, max_height=%d\n",
			 iba_index, isp_dev->iba[i].ppc, isp_dev->iba[i].vcid,
			 isp_dev->iba[i].frame_rate,
			 isp_dev->iba[i].data_format, isp_dev->iba[i].max_width,
			 isp_dev->iba[i].max_height);
	}

	return 0;
}
//
static int visp_parse_params(struct visp_dev *isp_dev,
				 struct platform_device *pdev)
{
	int port = 0;
	int ret = 0;
	const char *formats;
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

	fwnode_property_read_u32(of_fwnode_handle(node), "id", &isp_dev->id);
	if (!node) {
		dev_err(&pdev->dev, "No device tree node found\n");
		return -EINVAL;
	}
	if (isp_dev->id < 0 && isp_dev->id > 1) {
		dev_err(&pdev->dev, "Invalid ISP id %d\n", isp_dev->id);
		return -EINVAL;
	}
	dev_dbg(&pdev->dev, "Found visp device in device tree.\n");

	// Read string property for SS-MODE-i0 (LIMO, etc.)
	ret =
		of_property_read_string(node, "xlnx,io_mode", &isp_dev->ss_mode_i0);
	if (ret) {
		dev_err(&pdev->dev, "Failed to read xlnx,io_mode\n");
		return ret;
	} else {
		dev_info(&pdev->dev, "xlnx,io_mode: %s\n", isp_dev->ss_mode_i0);
	}
	dev_dbg(&pdev->dev, "xlnx,io_mode: %s\n", isp_dev->ss_mode_i0);

	// Read stream info (multi-stream, single-stream)
	ret = of_property_read_u32(node, "xlnx,num_streams",
				   &isp_dev->num_streams);
	if (ret) {
		dev_err(&pdev->dev,
			"Failed to read xlnx,num_streams property\n");
		return ret;
	} else {
		dev_info(&pdev->dev, "xlnx,num_streams: %u\n",
			 isp_dev->num_streams);
	}
	dev_dbg(&pdev->dev, "xlnx,num_streams: %u\n", isp_dev->num_streams);

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

	ret = of_property_read_u32(node, "xlnx,dma-align", &isp_dev->dma_align);
	if (ret) {
		dev_err(&pdev->dev, "Failed to read xlnx,dma-align property\n");
		return ret;
	} else {
		dev_dbg(&pdev->dev, "xlnx,dma-align: %u\n", isp_dev->dma_align);
	}

	// Read multiple strings property
	ret = of_property_read_string(node, "xlnx,vid-formats", &formats);
	if (!ret) {
		isp_dev->vid_formats = formats;
		dev_info(&pdev->dev, "xlnx,vid-formats: %s\n",
			 isp_dev->vid_formats);
	} else {
		dev_dbg(&pdev->dev,
			"Failed to read xlnx,vid-formats property\n");
	}

	// Read boolean properties
	isp_dev->tile0_enable_mp0 =
		of_property_read_bool(node, "xlnx,tile0-enable-mp0");
	dev_dbg(&pdev->dev, "xlnx,tile0-enable-mp0: %s\n",
		isp_dev->tile0_enable_mp0 ? "true" : "false");
	isp_dev->tile0_enable_sp0 =
		of_property_read_bool(node, "xlnx,tile0-enable-sp0");
	dev_dbg(&pdev->dev, "xlnx,tile0-enable-sp0: %s\n",
		isp_dev->tile0_enable_sp0 ? "true" : "false");

	if (parse_iba(isp_dev, node)) {
		dev_err(&pdev->dev, "Failed to parse IBA parameters\n");
		return -EINVAL;
	}
	return 0;
}

int xlnx_link_mbox(struct visp_dev *isp_dev);
int xlnx_link_mbox(struct visp_dev *isp_dev)
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
	isp_dev->rpu->isp_dev[isp_dev->id] =
		isp_dev; // Assigning isp_dev structure value to isp_dev present in
			 // rpu_dev struct
	return 0;
}

#define V4L2_SUBDEV_NAME_SIZE 32
static int visp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct visp_dev *isp_dev;
	int ret;
	int port = 0;

	isp_dev = devm_kzalloc(&pdev->dev, sizeof(struct visp_dev), GFP_KERNEL);
	if (!isp_dev)
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

	INIT_KFIFO(isp_dev->display_fifo);

	v4l2_subdev_init(&isp_dev->sd, &visp_subdev_ops);
	snprintf(isp_dev->sd.name, V4L2_SUBDEV_NAME_SIZE, "%s.%d", VISP_NAME,
		 isp_dev->id);

	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	isp_dev->sd.dev = &pdev->dev;
	isp_dev->sd.owner = THIS_MODULE;
	isp_dev->sd.internal_ops = &visp_internal_ops;
	isp_dev->sd.entity.ops = &visp_entity_ops;
	isp_dev->sd.entity.function = MEDIA_ENT_F_IO_V4L;
	isp_dev->sd.entity.obj_type = MEDIA_ENTITY_TYPE_V4L2_SUBDEV;
	isp_dev->sd.entity.name = isp_dev->sd.name;
	v4l2_set_subdevdata(&isp_dev->sd, isp_dev);

	visp_pads_init(isp_dev);
	ret = media_entity_pads_init(&isp_dev->sd.entity, VISP_PAD_NR,
					 isp_dev->pads);
	if (ret)
		return ret;

	ret = visp_async_notifier(isp_dev);
	if (ret)
		goto err_async_notifier;

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

	isp_dev->event_shm.virt_addr = (void *)__get_free_pages(GFP_KERNEL, 3);
	isp_dev->event_shm.size = PAGE_SIZE * 8;
	memset(isp_dev->event_shm.virt_addr, 0, isp_dev->event_shm.size);
	isp_dev->event_shm.phy_addr =
		virt_to_phys(isp_dev->event_shm.virt_addr);
	mutex_init(&isp_dev->event_shm.event_lock);

	visp_ctrl_init(isp_dev);

	for (port = 0; port < VISP_PORT_NR; port++) {
		mutex_init(&isp_dev->port_lock[port]);
		INIT_LIST_HEAD(&isp_dev->mcm_input[port]);
	}

	isp_dev->ports_mask = isp_dev->num_streams;

	dev_info(isp_dev->dev, "[VVCAM-ISP-DRV] %s %d\n", __func__, __LINE__);

	dev_info(&pdev->dev, "vvcam isp driver probe success\n");

	return 0;

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
	free_pages((unsigned long)isp_dev->event_shm.virt_addr, 3);
	visp_ctrl_destroy(isp_dev);
	dev_info(&pdev->dev, "vvcam isp driver remove\n");

}

static const struct dev_pm_ops visp_pm_ops = {};

static const struct of_device_id visp_of_match[] = {

	{
	.compatible = "xlnx,visp-mimo",
	},
	//	{.compatible = "xlnx,visp-ss-mimo-1.0",},
	{/* sentinel */},
};

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
		// class_destroy(mailbox_class);
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
