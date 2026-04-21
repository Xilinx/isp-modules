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
#include <linux/version.h>
#include <linux/workqueue.h>

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

/* Runtime configurable alignment bytes for stride calculation (default 16) */
unsigned int visp_align_bytes = 16;

/* Validate power of 2, range 4-1024 */
static int visp_align_bytes_set(const char *val, const struct kernel_param *kp)
{
	unsigned int n;

	if (kstrtouint(val, 0, &n) || n < 4 || n > 1024 || (n & (n - 1))) {
		pr_err("visp: Invalid align_bytes=%s (must be power of 2, range 4-1024). "
		       "Keeping current value: %u\n", val, visp_align_bytes);
		return -EINVAL;
	}

	visp_align_bytes = n;
	pr_info("visp: align_bytes updated to %u\n", n);
	return 0;
}

static const struct kernel_param_ops visp_align_ops = {
	.set = visp_align_bytes_set,
	.get = param_get_uint,
};

module_param_cb(align_bytes, &visp_align_ops, &visp_align_bytes, 0644);
MODULE_PARM_DESC(align_bytes, "DMA buffer stride alignment (power of 2, 4-1024, default 16)");
EXPORT_SYMBOL_GPL(visp_align_bytes);

static uint32_t sensor_dev_id[VISP_PORT_NR] = {2, 6, 5, 10};

#define VISP_MAX_SHARED_SUBDEVS 32

struct visp_shared_subdev_ref {
	struct v4l2_subdev *sd;
	u32 refcnt;
};

static DEFINE_MUTEX(visp_shared_subdev_lock);
static struct visp_shared_subdev_ref visp_shared_subdev_refs[VISP_MAX_SHARED_SUBDEVS];

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

	strscpy(cap->driver, sd->name, sizeof(cap->driver));
	strscpy(cap->card, sd->name, sizeof(cap->card));
	snprintf(cap->bus_info, sizeof(cap->bus_info), "platform:%s", sd->name);

	return 0;
}

static int visp_pad_requbufs(struct v4l2_subdev *sd, void *arg)
{
	struct visp_pad_reqbufs *pad_requbufs = (struct visp_pad_reqbufs *)arg;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	int status = 0;

	if (pad_requbufs->pad >= VISP_PAD_NR)
		return -EINVAL;
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
	int ret_val = 0;
	unsigned long flags;
	struct visp_pad_data *cur_pad;
	int i = 0;
	media_buf buf;

	if (pad_buf->pad >= VISP_PAD_NR)
		return -EINVAL;
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

	/* Dequeue buffer from the ISP device - pass payload directly.
	 * read_dq_buf_info() validates buf_index and reads p_owner from payload.
	 */
	uint32_t p_owner_val;
	ret_val = read_dq_buf_info(msg->payload, isp_dev, &info, &buf_index, &p_owner_val);
	if (ret_val) {
		dev_err(isp_dev->dev,
			"%s: Invalid buffer info from RPU\n", __func__);
		goto error_free_buf;
	}

	/* Lock to read cam_device_bufs and write p_owner atomically.
	 * Without lock, destroy_buf_pool could free the buffer between
	 * pointer read and p_owner write, causing use-after-free.
	 */
	mutex_lock(&isp_dev->isp_ports[info.vt_id]
		   .isp_chns[info.path]
		   .cam_device_bufs_lock);
	output_buffer = isp_dev->isp_ports[info.vt_id]
			    .isp_chns[info.path]
			    .cam_device_bufs[buf_index];
	if (output_buffer) {
		output_buffer->p_owner = p_owner_val;
	}
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
	dev_dbg(isp_dev->dev, "Found input sub-device: %s on pad %d\n",
		 subdev->name, pad);

	return subdev; /* Return the first valid input sub-device found */
}
#endif

static int visp_shared_subdev_find_slot(struct v4l2_subdev *sd)
{
	int i;

	for (i = 0; i < VISP_MAX_SHARED_SUBDEVS; i++) {
		if (visp_shared_subdev_refs[i].sd == sd)
			return i;
	}

	return -1;
}

static int visp_shared_subdev_find_free_slot(void)
{
	int i;

	for (i = 0; i < VISP_MAX_SHARED_SUBDEVS; i++) {
		if (!visp_shared_subdev_refs[i].sd)
			return i;
	}

	return -1;
}

static int visp_shared_subdev_stream_get(struct v4l2_subdev *sd)
{
	int slot;
	int ret = 0;
	bool need_stream_on = false;
	u32 refcnt = 0;

	if (!sd || !sd->ops || !sd->ops->video || !sd->ops->video->s_stream)
		return 0;

	mutex_lock(&visp_shared_subdev_lock);

	slot = visp_shared_subdev_find_slot(sd);
	if (slot >= 0) {
		visp_shared_subdev_refs[slot].refcnt++;
		refcnt = visp_shared_subdev_refs[slot].refcnt;
		mutex_unlock(&visp_shared_subdev_lock);
		if (sd->dev)
			dev_dbg(sd->dev,
				 "Upstream subdev '%s': stream_get ref++ slot=%d refcnt=%u (s_stream skipped)\n",
				 sd->name, slot, refcnt);
		return 0;
	}

	slot = visp_shared_subdev_find_free_slot();
	if (slot < 0) {
		mutex_unlock(&visp_shared_subdev_lock);
		return -ENOSPC;
	}

	visp_shared_subdev_refs[slot].sd = sd;
	visp_shared_subdev_refs[slot].refcnt = 1;
	need_stream_on = true;
	mutex_unlock(&visp_shared_subdev_lock);

	if (need_stream_on) {
		if (sd->dev)
			dev_dbg(sd->dev,
				 "Upstream subdev '%s': stream_get slot=%d refcnt=1, calling s_stream(1)\n",
				 sd->name, slot);
		ret = sd->ops->video->s_stream(sd, 1);
		if (ret) {
			if (sd->dev)
				dev_err(sd->dev,
					"Upstream subdev '%s': s_stream(1) failed: %d\n",
					sd->name, ret);
			mutex_lock(&visp_shared_subdev_lock);
			if (visp_shared_subdev_refs[slot].sd == sd &&
			    visp_shared_subdev_refs[slot].refcnt == 1) {
				visp_shared_subdev_refs[slot].sd = NULL;
				visp_shared_subdev_refs[slot].refcnt = 0;
			}
			mutex_unlock(&visp_shared_subdev_lock);
		} else if (sd->dev) {
			dev_dbg(sd->dev,
				 "Upstream subdev '%s': s_stream(1) success slot=%d\n",
				 sd->name, slot);
		}
	}

	return ret;
}

static int visp_shared_subdev_stream_put(struct v4l2_subdev *sd)
{
	int slot;
	int ret = 0;
	bool need_stream_off = false;
	u32 refcnt = 0;

	if (!sd || !sd->ops || !sd->ops->video || !sd->ops->video->s_stream)
		return 0;

	mutex_lock(&visp_shared_subdev_lock);

	slot = visp_shared_subdev_find_slot(sd);
	if (slot < 0) {
		mutex_unlock(&visp_shared_subdev_lock);
		if (sd->dev)
			dev_dbg(sd->dev,
				"Upstream subdev '%s': stream_put without tracked ref (already zero)\n",
				sd->name);
		return 0;
	}

	if (visp_shared_subdev_refs[slot].refcnt > 0)
		visp_shared_subdev_refs[slot].refcnt--;

	refcnt = visp_shared_subdev_refs[slot].refcnt;

	if (visp_shared_subdev_refs[slot].refcnt == 0) {
		visp_shared_subdev_refs[slot].sd = NULL;
		need_stream_off = true;
	}

	mutex_unlock(&visp_shared_subdev_lock);

	if (sd->dev)
		dev_dbg(sd->dev,
			 "Upstream subdev '%s': stream_put slot=%d new_refcnt=%u%s\n",
			 sd->name, slot, refcnt,
			 need_stream_off ? ", calling s_stream(0)" : " (s_stream skipped)");

	if (need_stream_off)
		ret = sd->ops->video->s_stream(sd, 0);

	if (need_stream_off && sd->dev) {
		if (ret)
			dev_warn(sd->dev,
				 "Upstream subdev '%s': s_stream(0) failed: %d\n",
				 sd->name, ret);
		else
			dev_dbg(sd->dev,
				 "Upstream subdev '%s': s_stream(0) success\n",
				 sd->name);
	}

	return ret;
}

static struct device_node *visp_get_remote_parent_for_port(struct device_node *np,
							    u32 port)
{
	struct device_node *ep;

	for_each_endpoint_of_node(np, ep) {
		struct device_node *remote;
		struct of_endpoint endpoint = {};

		if (of_graph_parse_endpoint(ep, &endpoint))
			continue;

		if (endpoint.port != port)
			continue;

		remote = of_graph_get_remote_port_parent(ep);
		of_node_put(ep);
		return remote;
	}

	return NULL;
}

/*
 * visp_get_remote_endpoint_port - Return the port number on the remote side
 * of a local endpoint identified by (np, local_port).
 * e.g. ISP1 port@0 connects to broadcaster port@2 → returns 2.
 */
static int visp_get_remote_endpoint_port(struct device_node *np, u32 local_port)
{
	struct device_node *ep;
	int remote_port = -1;

	for_each_endpoint_of_node(np, ep) {
		struct of_endpoint local_ep_info = {};
		struct device_node *remote_ep;
		struct of_endpoint remote_ep_info = {};

		if (of_graph_parse_endpoint(ep, &local_ep_info))
			continue;

		if (local_ep_info.port != local_port)
			continue;

		remote_ep = of_graph_get_remote_endpoint(ep);
		if (remote_ep) {
			if (!of_graph_parse_endpoint(remote_ep, &remote_ep_info))
				remote_port = (int)remote_ep_info.port;
			of_node_put(remote_ep);
		}
		of_node_put(ep);
		break;
	}

	return remote_port;
}

static void visp_release_upstream_nodes_dt(struct visp_dev *isp_dev)
{
	struct visp_limo_isp_dev_extended *ext = ISP_DEV_EXTENDED(isp_dev);
	u32 port;

	for (port = 0; port < VISP_PORT_NR; port++) {
		u32 i;

		for (i = 0; i < ext->upstream_node_count[port]; i++) {
			of_node_put(ext->upstream_nodes[port][i]);
			ext->upstream_nodes[port][i] = NULL;
		}

		ext->upstream_node_count[port] = 0;
	}
}

static bool visp_node_already_parsed(struct device_node **nodes, u32 count,
				     struct device_node *np)
{
	u32 i;

	for (i = 0; i < count; i++) {
		if (nodes[i] == np)
			return true;
	}

	return false;
}

static int visp_build_upstream_nodes_dt(struct visp_dev *isp_dev)
{
	struct visp_limo_isp_dev_extended *ext = ISP_DEV_EXTENDED(isp_dev);
	struct device_node *isp_np = isp_dev->dev->of_node;
	u32 port;

	visp_release_upstream_nodes_dt(isp_dev);

	if (!isp_np)
		return -ENODEV;

	for (port = 0; port < min_t(u32, isp_dev->num_streams, VISP_PORT_NR); port++) {
		struct device_node *cur_node;
		bool first_hop = true;
		/*
		 * MCM mapping: logical ISP port N occupies DT port@(N*VISP_PORT_PAD_NR).
		 * e.g. for num_streams=4: port@0, port@5, port@10, port@15 are the
		 * four sink ports; single-stream ISPs only use port@0 (N=0, 0*5=0).
		 */
		u32 dt_port = port * VISP_PORT_PAD_NR;

		cur_node = visp_get_remote_parent_for_port(isp_np, dt_port);
		if (cur_node) {
			int bc_port = visp_get_remote_endpoint_port(isp_np, dt_port);

			dev_dbg(isp_dev->dev,
				"ISP %d logical port %u (DT port@%u) -> '%s' via its port %d\n",
				isp_dev->id, port, dt_port,
				cur_node->full_name ? cur_node->full_name : cur_node->name,
				bc_port);
		}

		while (cur_node && ext->upstream_node_count[port] < VISP_MAX_UPSTREAM_NODES) {
			struct device_node *next;
			u32 idx = ext->upstream_node_count[port];

			if (cur_node == isp_np ||
			    visp_node_already_parsed(ext->upstream_nodes[port], idx, cur_node)) {
				of_node_put(cur_node);
				break;
			}

			ext->upstream_nodes[port][idx] = cur_node;
			ext->upstream_node_count[port]++;
			dev_dbg(isp_dev->dev,
				 "ISP %d logical port %u (DT port@%u) upstream_node[%u]: %s\n",
				 isp_dev->id, port, dt_port, idx,
				 cur_node->full_name ? cur_node->full_name : cur_node->name);

			/*
			 * On the first hop (ISP → broadcaster) use the broadcaster's
			 * sink port (port 0 per Xilinx DT convention) to walk further
			 * upstream toward the sensor. Subsequent hops always use port 0
			 * (each IP's sink is port@0 in Xilinx generated DTs).
			 */
			if (first_hop) {
				dev_dbg(isp_dev->dev,
					"  Traversing upstream from '%s' via its sink port 0\n",
					cur_node->full_name ? cur_node->full_name : cur_node->name);
				first_hop = false;
			}

			next = visp_get_remote_parent_for_port(cur_node, 0);
			if (!next)
				break;

			if (ext->upstream_node_count[port] >= VISP_MAX_UPSTREAM_NODES) {
				of_node_put(next);
				break;
			}

			cur_node = next;
		}

		dev_dbg(isp_dev->dev, "Parsed %u upstream DT nodes for ISP %d logical port %u (DT port@%u)\n",
			 ext->upstream_node_count[port], isp_dev->id, port, dt_port);
	}

	return 0;
}

/* Module-level list of all probed visp_dev instances.
 * Used for cross-ISP subdev lookup: when ISP1 needs to s_stream broadcaster/MIPI/sensor
 * that live in ISP0's media_device (because ISP1's sub-notifier returned -EEXIST),
 * visp_find_subdev_any() searches all registered ISP media devices.
 */
static LIST_HEAD(visp_dev_global_list);
static DEFINE_MUTEX(visp_dev_global_mutex);

static struct v4l2_subdev *visp_find_subdev_by_of_node(struct visp_dev *isp_dev,
						struct device_node *np)
{
	struct media_device *mdev = isp_dev->sd.entity.graph_obj.mdev;
	struct media_entity *entity;
	struct v4l2_subdev *sd;

	if (!mdev || !np)
		return NULL;

	media_device_for_each_entity(entity, mdev) {
		if (!is_media_entity_v4l2_subdev(entity))
			continue;

		sd = media_entity_to_v4l2_subdev(entity);
		if (!sd || sd == &isp_dev->sd)
			continue;

		if ((sd->dev && sd->dev->of_node == np) ||
		    (sd->fwnode && to_of_node(sd->fwnode) == np))
			return sd;
	}

	return NULL;
}

/**
 * visp_find_subdev_any - Find a v4l2_subdev by OF node across ALL ISP media devices.
 *
 * ISP0 owns the upstream chain (broadcaster, MIPI, sensor) in its media_device.
 * ISP1's sub-notifier gets -EEXIST (shared nodes) so those subdevs are absent from
 * ISP1's media_device.  This function falls back to searching every registered
 * visp_dev instance so ISP1 can still find and s_stream the shared subdevs.
 *
 * Returns the matching v4l2_subdev, or NULL if not found.
 */
static struct v4l2_subdev *visp_find_subdev_any(struct device_node *np)
{
	struct visp_dev *peer;
	struct v4l2_subdev *sd = NULL;

	if (!np)
		return NULL;

	mutex_lock(&visp_dev_global_mutex);
	list_for_each_entry(peer, &visp_dev_global_list, global_entry) {
		sd = visp_find_subdev_by_of_node(peer, np);
		if (sd)
			break;
	}
	mutex_unlock(&visp_dev_global_mutex);

	return sd;
}

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

	dev_dbg(isp_dev->dev, "=== Discovering pipeline subdevices for port %d ===\n", port);

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
				dev_dbg(isp_dev->dev,
					"  Candidate upstream subdev for port %d: %s (func=0x%x) via enabled link\n",
					port, sd->name, sd->entity.function);

				/* Avoid duplicates */
				for (i = 0; i < count; i++) {
					if (subdevs[i] == sd)
						break;
				}

				if (i == count && count < max_subdevs) {
					subdevs[count] = sd;
					dev_dbg(isp_dev->dev,
						 "  Identified upstream subdev[%d] for port %d: %s (function: 0x%x)\n",
						 count, port, sd->name, sd->entity.function);
					count++;
				} else if (i < count) {
					dev_dbg(isp_dev->dev,
						"  Duplicate upstream subdev for port %d ignored: %s\n",
						port, sd->name);
				} else {
					dev_warn(isp_dev->dev,
						 "  Upstream subdev list full for port %d, dropping %s (max=%d)\n",
						 port, sd->name, max_subdevs);
				}
				break;
			}
		}
	}

	dev_dbg(isp_dev->dev, "=== Pipeline discovery complete: found %d subdevices ===\n", count);
	return count;
}

static int visp_collect_upstream_subdevs(struct visp_dev *isp_dev, int port,
					 struct v4l2_subdev **subdevs, int max_subdevs)
{
	struct visp_limo_isp_dev_extended *ext = ISP_DEV_EXTENDED(isp_dev);
	int count = 0;
	int i;

	if (!isp_dev || !subdevs || max_subdevs <= 0 || port < 0 || port >= VISP_PORT_NR)
		return 0;

	if (ext->upstream_node_count[port] == 0)
		return visp_discover_pipeline_subdevs(isp_dev, port, subdevs, max_subdevs);

	for (i = 0; i < ext->upstream_node_count[port] && count < max_subdevs; i++) {
		struct v4l2_subdev *sd;
		int j;
		struct device_node *np = ext->upstream_nodes[port][i];

		sd = visp_find_subdev_by_of_node(isp_dev, np);
		if (!sd) {
			/*
			 * Not found in this ISP's media device.  Try the global
			 * list — upstream nodes shared with a peer ISP (e.g.
			 * broadcaster/MIPI/sensor registered in ISP0's media
			 * device when ISP1's notifier returned -EEXIST) are still
			 * reachable this way for s_stream ref-counting purposes.
			 */
			sd = visp_find_subdev_any(np);
		}
		if (!sd) {
			/*
			 * Subdev not found in any registered media device.
			 * For secondary ISPs (notifier_registered=false) this means
			 * the primary ISP's notifier has not yet bound this upstream
			 * subdev.  At user-space stream time this should not happen;
			 * log a warning so it is visible in dmesg.
			 */
			dev_warn(isp_dev->dev,
				 "ISP %d port %d: upstream_node[%d] '%s' has no subdev%s\n",
				 isp_dev->id, port, i,
				 np ? (np->full_name ? np->full_name : np->name) : "NULL",
				 ISP_DEV_EXTENDED(isp_dev)->notifier_registered ?
				 "" : " (secondary ISP)");
			continue;
		}

		for (j = 0; j < count; j++) {
			if (subdevs[j] == sd)
				break;
		}

		if (j == count) {
			subdevs[count++] = sd;
			dev_dbg(isp_dev->dev,
				 "ISP %d port %d upstream_node[%d] (%s) -> subdev[%d] '%s'\n",
				 isp_dev->id, port, i,
				 np ? (np->full_name ? np->full_name : np->name) : "NULL",
				 count - 1, sd->name);
		} else {
			dev_dbg(isp_dev->dev,
				"ISP %d port %d upstream_node[%d] maps to duplicate subdev '%s' (ignored)\n",
				isp_dev->id, port, i, sd->name);
		}
	}

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

	/* Use OF-node based collection (falls back to graph discovery if no DT nodes) */
	count = visp_collect_upstream_subdevs(isp_dev, port, subdevs, ARRAY_SIZE(subdevs));

	dev_dbg(isp_dev->dev, "Pipeline discovery for port %d found %d subdevices\n", port, count);

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

	/* Use OF-node based collection (falls back to graph discovery if no DT nodes) */
	count = visp_collect_upstream_subdevs(isp_dev, port, subdevs, ARRAY_SIZE(subdevs));

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
	int i, j, ret = 0;
	int count = visp_get_pipeline_subdev_count(isp_dev, port);

	dev_dbg(isp_dev->dev, "=== Pipeline Streaming %s for port %d ===\n",
		 enable ? "ON" : "OFF", port);

	if (count == 0) {
		struct visp_limo_isp_dev_extended *_ext = ISP_DEV_EXTENDED(isp_dev);

		if (_ext->upstream_node_count[port] > 0) {
			/*
			 * DT upstream nodes are known but no subdevs were found.
			 * For secondary ISPs (notifier_registered=false) this means
			 * the primary ISP's upstream chain is not yet available.
			 * Streaming without upstream will result in no sensor data.
			 */
			dev_err(isp_dev->dev,
				"ISP %d port %d: %u upstream DT nodes but no subdevs - %s\n",
				isp_dev->id, port, _ext->upstream_node_count[port],
				_ext->notifier_registered ?
				"primary ISP: check notifier completion" :
				"secondary ISP: peer ISP notifier may not have completed");
		} else {
			dev_warn(isp_dev->dev,
				 "ISP %d port %d: no pipeline subdevices found (no DT upstream nodes)\n",
				 isp_dev->id, port);
		}
		return 0;
	}

	dev_dbg(isp_dev->dev, "Found %d pipeline subdevices for port %d\n", count, port);

	if (enable) {
		/* Stream on: Start from the furthest upstream (sensor) to downstream (ISP) */
		dev_dbg(isp_dev->dev, "Starting streaming from sensor to ISP (reverse order):\n");
		for (i = count - 1; i >= 0; i--) {
			struct v4l2_subdev *subdev = visp_get_pipeline_subdev(isp_dev, port, i);
			dev_dbg(isp_dev->dev, "  [%d] Attempting to stream ON: %s\n",
				 i, subdev ? subdev->name : "NULL");

			if (subdev && subdev->ops && subdev->ops->video && subdev->ops->video->s_stream) {
				ret = visp_shared_subdev_stream_get(subdev);
				if (ret) {
					dev_err(isp_dev->dev,
						"Failed to start streaming on '%s': %d\n",
						subdev->name, ret);
					/* Drop refs for already-started devices. */
					for (j = i + 1; j < count; j++) {
						struct v4l2_subdev *prev;

						prev = visp_get_pipeline_subdev(isp_dev, port, j);
						if (prev && prev->ops &&
						    prev->ops->video &&
						    prev->ops->video->s_stream)
							visp_shared_subdev_stream_put(prev);
					}
					return ret;
				}
				dev_dbg(isp_dev->dev,
					 "  [%d] Upstream streaming ref acquired on '%s'\n",
					 i, subdev->name);
			} else {
				dev_warn(isp_dev->dev, "  [%d] Subdev %s has no s_stream operation\n",
					 i, subdev ? subdev->name : "NULL");
			}
		}
	} else {
		/* Stream off: Start from downstream (ISP) to upstream (sensor) */
		dev_dbg(isp_dev->dev, "Stopping streaming from ISP to sensor (forward order):\n");
		for (i = 0; i < count; i++) {
			struct v4l2_subdev *subdev = visp_get_pipeline_subdev(isp_dev, port, i);
			dev_dbg(isp_dev->dev, "  [%d] Attempting to stream OFF: %s\n",
				 i, subdev ? subdev->name : "NULL");

			if (subdev && subdev->ops && subdev->ops->video && subdev->ops->video->s_stream) {
				ret = visp_shared_subdev_stream_put(subdev);
				if (ret) {
					dev_warn(isp_dev->dev, "Failed to stop streaming on '%s': %d\n",
						 subdev->name, ret);
					/* Continue trying to stop other devices */
				} else {
					dev_dbg(isp_dev->dev,
						 "  [%d] Upstream streaming ref released on '%s'\n",
						 i, subdev->name);
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
	unsigned long flags;
	int ret = 0;

	if (pad_stream->pad >= VISP_PAD_NR)
		return -EINVAL;
	int port = pad_stream->pad / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad_stream->pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;
	/*
	 * The SP1 path (video1 / "self path", chn == MEDIA_ISP_PORT_PAD_SOURCE_SP1-1)
	 * taps the ISP's internal data flow.  Its upstream pipeline connections
	 * (sensor → MIPI → ISP) are already managed by the MP / RAW paths and
	 * the firmware stream_id mechanism.  SP1 must therefore NOT call
	 * visp_stream_pipeline_subdevs() and must NOT touch subdev_streamon_count,
	 * which is the reference counter that gates those calls.
	 */
	bool is_self_path = (chn == (MEDIA_ISP_PORT_PAD_SOURCE_SP1 - 1));

	if (pad_stream->status == 1) {

		isp_dev->pad_data[pad_stream->pad].stream = pad_stream->status;
		ret = visp_setup_isp_pipeline(isp_dev, pad_stream->pad);
		if (ret)
			return ret;

		/* Reset sequence counter on streamon */
		spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
		isp_dev->pad_data[pad_stream->pad].sequence = 0;
		spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock, flags);

		/*
		 * subdev_streamon_count[port] counts only non-self-path channels
		 * (MP, SP2, RAW).  SP1 bypasses this entirely - its upstream
		 * pipeline is controlled via stream_id, not V4L2 s_stream.
		 */
		if (!is_self_path) {
			if (ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port] == 0) {
				/* Stream on all pipeline subdevices */
				ret = visp_stream_pipeline_subdevs(isp_dev, port, 1);
				if (ret) {
					dev_err(isp_dev->dev, "Failed to start pipeline streaming on port %d: %d\n",
						port, ret);
					goto ERR_TO_RPU_LOCK;
				}
			}
		} else {
			dev_dbg(isp_dev->dev,
				 "ISP:%d port:%d chn:%d (SP1/self-path) stream-on: skipping upstream pipeline\n",
				 isp_dev->id, port, chn);
		}

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
		/* Only non-self-path channels increment the upstream pipeline refcount */
		if (!is_self_path)
			ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port]++;
		/* EXIT PORT Level CRITICAL SECTION */
	} else {
		if (!is_self_path &&
		    ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port]) {
			ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port]--;
			media_isp_stream_off(isp_dev, port, chn);
			/* subdev_streamon_count
			 * if > 0 implies there are other pipelines still processing streams
			 * from this subdev.
			 * if == 0 implies that the last avaialble pipeline has arrived for
			 * stream off and proceeds to perform complete cleanup of input pipeline.
			 */
			if (ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port] == 0) {
				/* clean sink pad sink_detect */
				isp_dev->pad_data[port * VISP_PORT_PAD_NR].sink_detected = 0;
				memset(&isp_dev->pad_data[port * VISP_PORT_PAD_NR].format, 0,
				       sizeof(struct v4l2_mbus_framefmt));

				/* Stream off all pipeline subdevices */
				ret = visp_stream_pipeline_subdevs(isp_dev, port, 0);
				if (ret) {
					dev_warn(isp_dev->dev, "Failed to stop pipeline streaming on isp: %d port: %d ret: %d\n",
						 isp_dev->id, port, ret);
				}
			}
		} else if (is_self_path) {
			dev_dbg(isp_dev->dev,
				 "ISP:%d port:%d chn:%d (SP1/self-path) stream-off: skipping upstream pipeline\n",
				 isp_dev->id, port, chn);
			media_isp_stream_off(isp_dev, port, chn);
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

ERR_TO_RPU_LOCK:
	isp_dev->streamon[pad_stream->pad] = 0;
	isp_dev->pad_data[pad_stream->pad].stream = 0;
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
	if (ubuf.pad >= VISP_PAD_NR)
		return -EINVAL;
	cur_pad = &isp_dev->pad_data[ubuf.pad];

	spin_lock_irqsave(&cur_pad->qlock, flags);

	if (cur_pad->stream == 0) {
		int port = ubuf.pad / MEDIA_ISP_PORT_PAD_COUNT;
		int chn = (ubuf.pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

		dev_dbg(isp_dev->dev,
			"ISP : %d Port : %d Chn : %d Pipeline in Streamoff state, buffer will be dropped\n",
			isp_dev->id, port, chn);
		spin_unlock_irqrestore(&cur_pad->qlock, flags);
		return -EINVAL;
	}

	if (list_empty(&cur_pad->queue)) {
		int port = ubuf.pad / MEDIA_ISP_PORT_PAD_COUNT;
		int chn = (ubuf.pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

		dev_dbg(isp_dev->dev,
			"ISP : %d Port : %d Chn : %d buffer queue is empty, buffer will be dropped\n",
			isp_dev->id, port, chn);
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

	if (pad_ctrl->pad >= VISP_PAD_NR)
		return -EINVAL;
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

	if (pad_ctrl->pad >= VISP_PAD_NR)
		return -EINVAL;
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

	if (pad_ext_ctrls->pad >= VISP_PAD_NR)
		return -EINVAL;
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

	if (pad_ext_ctrls->pad >= VISP_PAD_NR)
		return -EINVAL;
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
		kfree(ext_dma_buf);
		return -ENOMEM;
	}

		dev_dbg(isp_dev->dev, "RDMA Buffer is %llx\n", ext_dma_buf->addr);

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
		kfree(ext_dma_buf);
		return -ENOMEM;
	}

dev_dbg(isp_dev->dev, "ISP:%d Port:%d RDMA Buffer is 0x%llx\n",
				isp_dev->id, ext_buf_info->port, ext_dma_buf->addr);

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
		dev_dbg(isp_dev->dev, "%s: dma_addr: 0x%x is free\n", __func__,
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
		dev_dbg(event_shm->dev, "DMA-BUF released, will re-export on next use\n");
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

	dev_dbg(dev, "DMA-BUF exported successfully: virt=%p dma=%pad size=%d\n",
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
			dev_dbg(isp_dev->event_shm.dev, "DMA-BUF exported on first use\n");
		}

		/* Create per-process fd - dma_buf_fd() takes the reference from export */
		*(int *)arg = dma_buf_fd(isp_dev->event_shm.dmabuf, O_RDWR | O_CLOEXEC);
		if (*(int *)arg < 0) {
			dev_err(isp_dev->event_shm.dev,
				"VISP_GET_EVENT_SHM_FD: dma_buf_fd failed: %d\n", *(int *)arg);
			ret = *(int *)arg;
		} else {
			dev_dbg(isp_dev->event_shm.dev,
				 "VISP_GET_EVENT_SHM_FD: returned fd=%d\n", *(int *)arg);
			ret = 0;
		}
		break;
	}
	case VISP_EVENT_ACK: {
		struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
		uint32_t ack_seq = *(uint32_t *)arg;

		if (ack_seq == isp_dev->event_shm.seq) {
			complete(&isp_dev->event_shm.event_ack);
			ret = 0;
		} else {
			dev_warn(isp_dev->dev, "Stale ack: got seq=%u, expected=%u\n",
				 ack_seq, isp_dev->event_shm.seq);
			ret = -EINVAL;
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
	struct visp_pad_data *source_pad;
	int i;
	int ret = 0;
	media_fmt Format_media;
	struct v4l2_mbus_framefmt *MBusFormat;
	struct media_pad *mediapad_t;
	struct format_reserved fmt_res;

	sink_pad_index = format->pad - (format->pad % VISP_PORT_PAD_NR);
	sink_pad = &isp_dev->pad_data[sink_pad_index];

	/* Apply format to the current pad being set */
	cur_pad->sink_detected = 1;
	cur_pad->format = format->format;
	if (sink_pad == cur_pad) {
		for (i = 1; i < VISP_PORT_PAD_NR; i++) {
			source_pad = &isp_dev->pad_data[sink_pad_index + i];
			switch (i) {
			case VISP_PORT_PAD_SOURCE_MP:
			case VISP_PORT_PAD_SOURCE_SP1:
			case VISP_PORT_PAD_SOURCE_SP2:
			case VISP_PORT_PAD_SOURCE_RAW:
				source_pad->format = format->format;
				source_pad->format.code = source_pad->fmts[0].code;
				source_pad->format.field = V4L2_FIELD_NONE;
				source_pad->format.quantization = V4L2_QUANTIZATION_DEFAULT;
				source_pad->format.colorspace = V4L2_COLORSPACE_DEFAULT;

				memset(&fmt_res, 0, sizeof(fmt_res));
				fmt_res.fourcc = source_pad->fmts[0].fourcc;
				visp_get_format_stride_public(isp_dev, fmt_res.fourcc,
						      format->format.width,
						      format->format.height,
						      &fmt_res.stride);
				memcpy(source_pad->format.reserved, &fmt_res,
				       sizeof(fmt_res));
				break;
			default:
				break;
			}
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

	memset(&fmt_res, 0, sizeof(fmt_res));
	memcpy(&fmt_res, format->format.reserved, sizeof(fmt_res));
	for (i = 0; i < cur_pad->num_formats; i++) {
		if (format->format.code == cur_pad->fmts[i].code &&
		    fmt_res.fourcc == cur_pad->fmts[i].fourcc)
			break;
	}

	if (i >= cur_pad->num_formats) {
		format->format.code = cur_pad->fmts[0].code;
		memset(&fmt_res, 0, sizeof(fmt_res));
		fmt_res.fourcc = cur_pad->fmts[0].fourcc;
		visp_get_format_stride_public(isp_dev, fmt_res.fourcc,
				      format->format.width,
				      format->format.height,
				      &fmt_res.stride);
		memcpy(format->format.reserved, &fmt_res, sizeof(fmt_res));
	}

	memset(&Format_media, 0, sizeof(Format_media));

	MBusFormat = (struct v4l2_mbus_framefmt *)&format->format;
	Format_media.width = MBusFormat->width;
	Format_media.height = MBusFormat->height;
	Format_media.color_space = MBusFormat->colorspace;
	Format_media.quantization = MBusFormat->quantization;
	Format_media.stride = fmt_res.stride;

	media_isp_hal_mbus_fmt_to_media_fmt(
		&MBusFormat->code, &Format_media.pixel_format, fmt_res.fourcc);

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
	struct v4l2_subdev_pad_config pad_cfg;

	memset(&pad_cfg, 0, sizeof(pad_cfg));
	struct v4l2_subdev_state sd_state = {
	    .pads = &pad_cfg,
	};

	return visp_set_fmt(&isp_dev->sd, &sd_state, format);
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
	int ret = 0;

	ret = visp_setup_isp_pipeline(isp_dev, code->pad);
	if (ret)
		return ret;

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

	dev_dbg(dev, "ISP %d notifier_bound: '%s'\n", isp_dev->id, sd->name);

	if (!sd->fwnode) {
		dev_err(dev, "Subdev %s has no fwnode, skipping link creation\n",
			sd->name);
		return -EINVAL;
	}

	while ((ep = fwnode_graph_get_next_endpoint(sd->fwnode, ep))) {
		ret = v4l2_fwnode_parse_link(ep, &link);
		if (ret < 0) {
			dev_err(dev, "Failed to parse link for %pOF: %d\n",
				to_of_node(ep), ret);
			/* ep reference will be released by
			 * fwnode_graph_get_next_endpoint on the next
			 * iteration or when the loop terminates.
			 */
			continue;
		}

		/* Guard against DT port numbers exceeding the entity's pad count */
		if (link.local_port >= sd->entity.num_pads) {
			v4l2_fwnode_put_link(&link);
			continue;
		}

		/* Only process source pads on the bound subdev */
		if (sd->entity.pads[link.local_port].flags & MEDIA_PAD_FL_SINK) {
			v4l2_fwnode_put_link(&link);
			continue;
		}

		/*
		 * Only create a link if the remote end of this source endpoint
		 * is THIS ISP instance.  The broadcaster has multiple source
		 * ports (one per ISP); without this check every broadcaster
		 * source pad would be linked to this ISP's sink pad, producing
		 * duplicate/wrong links like:
		 *   broadcaster:1 -> ISP0:0  (correct)
		 *   broadcaster:2 -> ISP0:0  (wrong - belongs to ISP1)
		 */
		if (link.remote_node != dev_fwnode(dev)) {
			/*
			 * The remote endpoint is a peer ISP (not this one).
			 * Try to find that ISP subdev via the global list and
			 * create the broadcaster→peer_ISP link.  Both entities
			 * must share the same media device for media_create_pad_link
			 * to succeed; if they are in different devices the call
			 * will fail and is silently ignored.
			 *
			 * snk_pad = link.remote_port directly because the DT
			 * port@N number IS the pad index on the ISP entity.
			 * e.g. port@0  → pad 0  (single-stream sink)
			 *      port@5  → pad 5  (MCM logical port 1 sink)
			 *      port@10 → pad 10 (MCM logical port 2 sink)
			 * Using link.remote_port * VISP_PORT_PAD_NR would give
			 * wrong results for MCM (port@5 → 5*5+0=25, out of range).
			 */
			struct v4l2_subdev *peer_isp =
				visp_find_subdev_any(to_of_node(link.remote_node));

			if (peer_isp &&
			    peer_isp->entity.graph_obj.mdev ==
			    sd->entity.graph_obj.mdev) {
				unsigned int src_pad = link.local_port;
				unsigned int snk_pad = link.remote_port;

				v4l2_fwnode_put_link(&link);

				ret = media_create_pad_link(&sd->entity, src_pad,
							    &peer_isp->entity,
							    snk_pad,
							    MEDIA_LNK_FL_ENABLED);
				if (ret && ret != -EEXIST)
					dev_warn(dev,
						 "Peer ISP link %s:%u -> %s:%u failed: %d\n",
						 sd->name, src_pad,
						 peer_isp->entity.name, snk_pad,
						 ret);
				else if (ret == 0)
					dev_dbg(dev,
						 "Peer ISP linked %s:%u -> %s:%u\n",
						 sd->name, src_pad,
						 peer_isp->entity.name, snk_pad);
				ret = 0;
			} else {
				dev_dbg(dev,
					"Notifier bound: %s:%u -> remote is not this ISP, skipping\n",
					sd->name, link.local_port);
				v4l2_fwnode_put_link(&link);
			}
			continue;
		}

		source     = &sd->entity;
		source_pad = link.local_port;
		sink       = &isp_dev->sd.entity;
		sink_pad   = link.remote_port;

		v4l2_fwnode_put_link(&link);

		ret = media_create_pad_link(source, source_pad, sink, sink_pad,
					    MEDIA_LNK_FL_ENABLED);
		if (ret && ret != -EEXIST) {
			dev_err(dev, "Failed to create link: %s:%u -> %s:%u: %d\n",
				source->name, source_pad, sink->name, sink_pad,
				ret);
			fwnode_handle_put(ep);
			return ret;
		}
		if (ret == 0)
			dev_dbg(dev, "Linked %s:%u -> %s:%u\n",
				 source->name, source_pad, sink->name, sink_pad);
		ret = 0;
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
	struct v4l2_async_connection *asc;
	int ret = 0;

	dev_dbg(isp_dev->dev, "=== Async notifier complete - creating inter-subdev links ===\n");

	/*
	 * For every bound upstream subdev (broadcaster, MIPI, sensor), walk its
	 * SOURCE endpoints and create media pad links to whatever is on the other
	 * side, as long as the remote is also a bound subdev in this notifier.
	 *
	 * Links into the ISP itself (remote_node == our ISP device) are skipped
	 * here because visp_notifier_bound() already created those.
	 *
	 * Expected results (sensor pad0:src → MIPI pad1:sink,
	 *                   MIPI pad1:src  → broadcaster pad0:sink).
	 */
	list_for_each_entry(asc, &notifier->done_list, asc_entry) {
		struct device_node *src_np = to_of_node(asc->match.fwnode);
		struct v4l2_subdev *src_sd;
		struct fwnode_handle *ep = NULL;
		struct v4l2_fwnode_link link;

		src_sd = visp_find_subdev_by_of_node(isp_dev, src_np);
		if (!src_sd || !src_sd->fwnode)
			continue;

		while ((ep = fwnode_graph_get_next_endpoint(src_sd->fwnode, ep))) {
			struct v4l2_subdev *remote_sd;
			unsigned int source_pad, sink_pad;

			if (v4l2_fwnode_parse_link(ep, &link) < 0)
				continue;

			/* Only process source pads */
			if (src_sd->entity.pads[link.local_port].flags &
			    MEDIA_PAD_FL_SINK) {
				v4l2_fwnode_put_link(&link);
				continue;
			}

			/* Skip: this link terminates at our ISP, handled by bound() */
			if (link.remote_node == dev_fwnode(isp_dev->dev)) {
				v4l2_fwnode_put_link(&link);
				continue;
			}

			remote_sd = visp_find_subdev_by_of_node(
					isp_dev, to_of_node(link.remote_node));
			if (!remote_sd) {
				/*
				 * Not found in this ISP's media graph.
				 * Fall back to the global list - the remote may
				 * be owned by a peer ISP that registered the
				 * shared upstream chain first.
				 */
				remote_sd = visp_find_subdev_any(
						to_of_node(link.remote_node));
			}
			if (!remote_sd) {
				dev_dbg(isp_dev->dev,
					"complete: remote '%pOF' not found anywhere, skipping\n",
					to_of_node(link.remote_node));
				v4l2_fwnode_put_link(&link);
				continue;
			}

			source_pad = link.local_port;
			sink_pad   = link.remote_port;
			v4l2_fwnode_put_link(&link);

			ret = media_create_pad_link(&src_sd->entity, source_pad,
						    &remote_sd->entity, sink_pad,
						    MEDIA_LNK_FL_ENABLED);
			if (ret && ret != -EEXIST) {
				dev_warn(isp_dev->dev,
					 "complete: link %s:%u -> %s:%u failed: %d\n",
					 src_sd->entity.name, source_pad,
					 remote_sd->entity.name, sink_pad, ret);
			} else {
				if (ret == 0)
					dev_dbg(isp_dev->dev,
						 "complete: linked %s:%u -> %s:%u\n",
						 src_sd->entity.name, source_pad,
						 remote_sd->entity.name, sink_pad);
				ret = 0;
			}
		}
	}

	return ret;
}

/*
 * visp_notifier_has_fwnode - check if fwnode is already in this ISP's notifier
 *
 * v4l2_async_nf_add_fwnode() does NOT deduplicate — it always appends a new
 * entry to waiting_list.  Duplicate entries for the same device fwnode (e.g.
 * broadcaster_0 shared by logical ports 0 and 1 in MCM) cause
 * v4l2_async_nf_register() to return -EEXIST when it validates the list, even
 * though the duplicates are within our OWN notifier.  Perform explicit dedup
 * before calling v4l2_async_nf_add_fwnode to avoid this.
 */
static bool visp_notifier_has_fwnode(struct visp_dev *isp_dev,
				     struct fwnode_handle *fwnode)
{
	struct v4l2_async_connection *asc;

	list_for_each_entry(asc, &isp_dev->notifier.waiting_list, asc_entry) {
		if (asc->match.fwnode == fwnode)
			return true;
	}
	return false;
}

static const struct v4l2_async_notifier_operations visp_notify_ops = {
	.bound  = visp_notifier_bound,
	.unbind = visp_notifier_unbound,
	.complete = visp_notifier_complete,
};

static int visp_async_notifier(struct visp_dev *isp_dev)
{
	struct visp_limo_isp_dev_extended *ext = ISP_DEV_EXTENDED(isp_dev);
	struct v4l2_async_connection *asc;
	struct device *dev = isp_dev->dev;
	int ret = 0;
	u32 port, i;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	v4l2_async_subdev_nf_init(&isp_dev->notifier, &isp_dev->sd);
#else
	v4l2_async_notifier_init(&isp_dev->notifier);
#endif

	isp_dev->notifier.ops = &visp_notify_ops;

	if (dev_fwnode(isp_dev->dev) == NULL)
		return 0;

	/*
	 * Add all upstream nodes (broadcaster, MIPI CSI-2 RX, sensor) by their
	 * DEVICE fwnodes so each subdev is represented exactly once in the
	 * notifier.  upstream_nodes[] is populated by visp_build_upstream_nodes_dt:
	 *   [port][0] = broadcaster (direct upstream of the ISP's DT sink port)
	 *   [port][1] = MIPI CSI-2 RX
	 *   [port][2] = sensor
	 *
	 * MCM case (num_streams > 1): multiple logical ports may share the same
	 * upstream node (e.g. broadcaster_0 feeds both ISP logical port 0 and 1).
	 * -EEXIST from v4l2_async_nf_add_fwnode is silently ignored so each
	 * unique device node is tracked exactly once regardless of how many
	 * logical ports reference it.
	 *
	 * IMPORTANT: Do NOT use v4l2_async_nf_add_fwnode_remote() here.  That
	 * API stores the remote ENDPOINT fwnode, which is a different fwnode
	 * object from the device fwnode stored in upstream_nodes[].  Having both
	 * endpoint-fwnode and device-fwnode entries for the same broadcaster
	 * causes the async framework to call visp_notifier_bound() TWICE for
	 * that subdev, creating a duplicate broadcaster→ISP pad link.
	 */
	for (port = 0; port < min_t(u32, isp_dev->num_streams, VISP_PORT_NR); port++) {
		for (i = 0; i < ext->upstream_node_count[port]; i++) {
			struct device_node *np = ext->upstream_nodes[port][i];
			struct fwnode_handle *fwh;

			if (!np)
				continue;

			fwh = of_fwnode_handle(np);

			/*
			 * Explicit dedup: v4l2_async_nf_add_fwnode() does NOT
			 * deduplicate — it appends unconditionally.  In MCM,
			 * logical ports 0 and 1 share broadcaster_0 (and MIPI_0,
			 * sensor_0), so the same fwnode would be added twice.
			 * v4l2_async_nf_register() then sees the duplicate within
			 * our own notifier and returns -EEXIST, causing ISP 0 to
			 * incorrectly take the "secondary ISP path".
			 */
			if (visp_notifier_has_fwnode(isp_dev, fwh)) {
				dev_dbg(dev, "ISP %d: skip dup upstream node '%s'\n",
					isp_dev->id,
					np->full_name ? np->full_name : np->name);
				continue;
			}

			asc = v4l2_async_nf_add_fwnode(
				&isp_dev->notifier,
				fwh,
				struct v4l2_async_connection);
			if (IS_ERR(asc)) {
				ret = PTR_ERR(asc);
				dev_err(dev,
					"Failed to add upstream node '%s': %d\n",
					np->full_name ? np->full_name : np->name,
					ret);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
				v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
				v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
				return ret;
			}
			dev_dbg(dev, "ISP %d: watching upstream node '%s' for async bind\n",
				 isp_dev->id,
				 np->full_name ? np->full_name : np->name);
		}
	}

	dev_dbg(dev, "ISP %d: %u subdev(s) queued in notifier waiting list\n",
		 isp_dev->id,
		 (unsigned int)list_count_nodes(&isp_dev->notifier.waiting_list));

	ret = v4l2_async_nf_register(&isp_dev->notifier);
	if (ret == 0) {
		/* Primary ISP: sub-notifier registered successfully. */
		ISP_DEV_EXTENDED(isp_dev)->notifier_registered = true;
		dev_dbg(dev, "ISP %d: upstream sub-notifier registered (primary ISP path)\n",
			 isp_dev->id);
	}
	if (ret) {
		/*
		 * -EEXIST (-17): one or more upstream fwnodes are already tracked
		 * by ISP0's notifier.  ISP0 and ISP1 share the same broadcaster /
		 * MIPI / sensor chain, so this is expected on the second ISP to
		 * probe.  The shared subdevs are bound through ISP0; treat as
		 * success so ISP1 probes correctly.
		 *
		 * When this ISP's notifier is skipped, the broadcaster→this_ISP
		 * sink link is not created by visp_notifier_bound().  Create it
		 * here directly using the global subdev list.  The broadcaster is
		 * already registered and bound to a peer ISP's media device;
		 * media_create_pad_link() will only succeed if both the broadcaster
		 * and this ISP entity share the same media device.
		 */
		if (ret == -EEXIST) {
			dev_dbg(isp_dev->dev,
				 "Async notifier: shared subdevs already claimed by peer ISP (secondary ISP path)\n");
			dev_dbg(isp_dev->dev,
				 "ISP %d will use visp_find_subdev_any() to locate shared upstream subdevs at stream time\n",
				 isp_dev->id);

			/*
			 * Clean up the notifier entries that were added via
			 * v4l2_async_nf_add_fwnode() but never registered.
			 * Without this cleanup the waiting_list entries would
			 * leak until visp_remove() calls v4l2_async_nf_cleanup().
			 * visp_remove() still calls unregister+cleanup safely
			 * (unregister is a no-op for a notifier not in the list).
			 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
			v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
			v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
			/* notifier_registered stays false - secondary ISP path */
			return 0;
		}
		dev_err(isp_dev->dev, "Async notifier register error: %d\n", ret);
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

	for (port = 0; port < VISP_PORT_NR; port++) {
		strscpy(isp_dev->isp_ports[port].sensor_info.name,
			VISP_DEFAULT_SENSOR,
			sizeof(isp_dev->isp_ports[port].sensor_info.name));

		isp_dev->isp_ports[port].sensor_info.mode =
		    VISP_DEFAULT_SENSOR_MODE;

		isp_dev->isp_ports[port].sensor_info.sensor_id =
		    sensor_dev_id[port];
		if(isp_dev->num_streams > 1)
		{
			isp_dev->isp_ports[port].hw_mcm = (bool_t)CAMDEV_MCM_OP_HW;

		}
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

	/* Read LLP (Low Latency Path) device tree parameters - optional */
	{
		u32 llpath0_iba = 0;
		u32 llpath0_oba = 0;
		bool llpath0_tile0_enabled = false;
		unsigned int i;

		/* Initialize all LLP arrays to 0 (no capability, disabled) */
		for (i = 0; i < 4; i++) {
			ISP_DEV_EXTENDED(isp_dev)->llp_capable[i] = 0;
			ISP_DEV_EXTENDED(isp_dev)->llp[i] = 0;
		}

		/* Read xlnx,llpath0-iba (optional, no error if missing) */
		ret = of_property_read_u32(node, "xlnx,llpath0-iba", &llpath0_iba);
		if (!ret) {
			dev_dbg(&pdev->dev, "xlnx,llpath0-iba: %u\n", llpath0_iba);
		} else {
			dev_dbg(&pdev->dev, "xlnx,llpath0-iba not found (optional)\n");
		}

		/* Read xlnx,llpath0-oba (optional, no error if missing) */
		ret = of_property_read_u32(node, "xlnx,llpath0-oba", &llpath0_oba);
		if (!ret) {
			dev_dbg(&pdev->dev, "xlnx,llpath0-oba: %u\n", llpath0_oba);

			/* Check if xlnx,llpath0-tile0-enabled is present (boolean property) */
			llpath0_tile0_enabled = of_property_read_bool(node, "xlnx,llpath0-tile0-enabled");

			/* If enabled flag is set and OBA index is valid, mark path as LLP-capable and enable it */
			if (llpath0_tile0_enabled && llpath0_oba < 4) {
				ISP_DEV_EXTENDED(isp_dev)->llp_capable[llpath0_oba] = 1;
				ISP_DEV_EXTENDED(isp_dev)->llp[llpath0_oba] = 1;
				dev_dbg(&pdev->dev,
					"LLP capable and enabled for OBA path %u (llpath0-tile0-enabled is set)\n",
					llpath0_oba);
			} else if (llpath0_tile0_enabled && llpath0_oba >= 4) {
				dev_warn(&pdev->dev,
					"Invalid OBA index %u (must be 0-3), LLP not enabled\n",
					llpath0_oba);
			}
		} else {
			dev_dbg(&pdev->dev, "xlnx,llpath0-oba not found (optional)\n");
		}
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
	if (isp_dev->num_streams < 1 || isp_dev->num_streams > MAX_PORTS) {
		dev_err(isp_dev->dev,
			"Invalid num_streams=%u (valid 1-%u)\n",
			isp_dev->num_streams, MAX_PORTS);
		return -EINVAL;
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
	isp_dev->rpu->isp_dev[isp_dev->id] = isp_dev;
	/*
	 * Assigning isp_dev structure value to isp_dev present in
	 * rpu_dev struct
	 */

	return 0;
}

static void visp_destroy_enq_wqs(struct visp_dev *isp_dev)
{
	int port, chain;

	if (!isp_dev)
		return;

	for (port = 0; port < isp_dev->num_streams; port++) {
		for (chain = 0; chain < ENQ_WQ_CHAIN_MAX; chain++) {
			if (isp_dev->enq_wq_chain[port][chain]) {
				destroy_workqueue(isp_dev->enq_wq_chain[port][chain]);
				isp_dev->enq_wq_chain[port][chain] = NULL;
			}
		}
		if (isp_dev->enq_wq[port]) {
			destroy_workqueue(isp_dev->enq_wq[port]);
			isp_dev->enq_wq[port] = NULL;
		}
	}
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

	/* Register in the module-wide list for cross-ISP subdev lookup */
	mutex_lock(&visp_dev_global_mutex);
	list_add(&isp_dev->global_entry, &visp_dev_global_list);
	mutex_unlock(&visp_dev_global_mutex);

	ret = visp_parse_params(isp_dev, pdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to parse params\n");
		return -EINVAL;
	}

	ret = visp_build_upstream_nodes_dt(isp_dev);
	if (ret && ret != -ENODEV)
		dev_warn(&pdev->dev, "failed to parse upstream DT forward nodes: %d\n", ret);

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

	dev_dbg(dev, "V4L2 subdev registered: '%s' entity='%s' pads=%u\n",
		 isp_dev->sd.name, isp_dev->sd.entity.name,
		 isp_dev->sd.entity.num_pads);

	/* Assign the CALIB path to sensor_info[0].calib */
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
	init_completion(&isp_dev->event_shm.event_ack);
	isp_dev->event_shm.seq = 0;

	/* DMA-BUF will be exported lazily on first VISP_GET_EVENT_SHM_FD ioctl */
	isp_dev->event_shm.dmabuf = NULL;
	isp_dev->event_shm.dmabuf_fd = -1;

	isp_dev->reserve_mem.va =
	    ioremap_wc(isp_dev->reserve_mem.pa, isp_dev->reserve_mem.size);
	if (!isp_dev->reserve_mem.va) {
		dev_err(dev, "Failed to ioremap reserved memory\n");
		ret = -ENOMEM;
		goto err_ioremap;
	}

	visp_ctrl_init(isp_dev);

	for (port = 0; port < VISP_PORT_NR; port++) {
		mutex_init(&isp_dev->port_lock[port]);
		INIT_LIST_HEAD(&isp_dev->mcm_input[port]);
	}

	for (port = 0; port < isp_dev->num_streams; port++) {
		char wq_name[16];

		/* Per-port MP/SP workqueues: chain 0 = MP, chain 1 = SP */
		for (int chain = 0; chain < ENQ_WQ_CHAIN_MAX; chain++) {
			snprintf(wq_name, sizeof(wq_name), "visp-enq-%d-%d",
				 port, chain);
			isp_dev->enq_wq_chain[port][chain] = alloc_workqueue(
				wq_name, WQ_UNBOUND | WQ_HIGHPRI, ENQ_WQ_MAX_ACTIVE);
			if (!isp_dev->enq_wq_chain[port][chain]) {
				dev_err(dev,
					"Failed to create enqueue workqueue for port %d chain %d\n",
					port, chain);
				ret = -ENOMEM;
				goto err_destroy_enq_wq;
			}
		}
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

err_destroy_enq_wq:
	visp_destroy_enq_wqs(isp_dev);

err_ioremap:
	/* Free DMA memory if it was allocated */
	if (isp_dev->event_shm.virt_addr) {
		dma_free_coherent(isp_dev->event_shm.dev, isp_dev->event_shm.size,
				  isp_dev->event_shm.virt_addr,
				  isp_dev->event_shm.dma_handle);
		isp_dev->event_shm.virt_addr = NULL;
	}

err_alloc_event_shm:
	visp_procfs_unregister(isp_dev->pde);

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
	visp_release_upstream_nodes_dt(isp_dev);

	/* Unregister from the module-wide list */
	mutex_lock(&visp_dev_global_mutex);
	list_del(&isp_dev->global_entry);
	mutex_unlock(&visp_dev_global_mutex);

	visp_destroy_enq_wqs(isp_dev);

	visp_procfs_unregister(isp_dev->pde);
	v4l2_async_unregister_subdev(&isp_dev->sd);
	/*
	 * For the primary ISP (notifier_registered=true) unregister the
	 * sub-notifier before cleanup.  For the secondary ISP the notifier
	 * was already cleaned up at -EEXIST time inside visp_async_notifier();
	 * v4l2_async_nf_unregister() is a no-op for an unregistered notifier
	 * but v4l2_async_nf_cleanup() must still be called to free any entries
	 * that may remain.
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	if (ISP_DEV_EXTENDED(isp_dev)->notifier_registered)
		v4l2_async_nf_unregister(&isp_dev->notifier);
	v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
	if (ISP_DEV_EXTENDED(isp_dev)->notifier_registered)
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
		dev_dbg(isp_dev->event_shm.dev, "Freed DMA coherent memory: %u bytes\n",
			 isp_dev->event_shm.size);
		isp_dev->event_shm.virt_addr = NULL;
	}

	visp_ctrl_destroy(isp_dev);

	if (isp_dev->reserve_mem.va)
		iounmap(isp_dev->reserve_mem.va);

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
MODULE_INFO(import_ns, "DMA_BUF");
