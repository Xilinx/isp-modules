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

#include <linux/build_bug.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of_graph.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
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
	/*
	 * Set while s_stream(0) is in flight for this slot, i.e. between
	 * refcnt dropping to 0 (lock held) and the slot actually being freed
	 * (lock re-acquired after s_stream(0) returns). sd is deliberately
	 * NOT cleared until then, so visp_shared_subdev_find_free_slot()
	 * cannot hand this slot to a concurrent get() while the hardware is
	 * still mid-disable - closing the window where a racing get() could
	 * issue an overlapping s_stream(1) on the same physical subdev while
	 * this put()'s s_stream(0) is still running. See stream_get()/
	 * stream_put() below.
	 */
	bool disabling;
	struct completion disable_done;
	/*
	 * Symmetric counterpart to disabling/disable_done: set while
	 * s_stream(1) is in flight for this slot, i.e. between refcnt being
	 * published as 1 (lock held) and the actual s_stream(1) call
	 * returning (lock re-acquired afterward). A concurrent get() for the
	 * same sd that lands in this window must not just bump refcnt and
	 * return success - it has to wait for the real result, the same way
	 * a concurrent get() waits out an in-flight disable. See
	 * stream_get() below.
	 */
	bool enabling;
	struct completion enable_done;
};

static DEFINE_MUTEX(visp_shared_subdev_lock);
static struct visp_shared_subdev_ref visp_shared_subdev_refs[VISP_MAX_SHARED_SUBDEVS];
static bool visp_shared_subdev_refs_init_done;

/*
 * struct completion has no static zero-initializer that also satisfies
 * lockdep's init tracking the way DEFINE_MUTEX() does for the mutex above,
 * so each slot's disable_done is initialized lazily, once, under the same
 * lock that already serializes every other access to this table.
 */
static void visp_shared_subdev_refs_ensure_init(void)
{
	int i;

	if (visp_shared_subdev_refs_init_done)
		return;

	for (i = 0; i < VISP_MAX_SHARED_SUBDEVS; i++) {
		init_completion(&visp_shared_subdev_refs[i].disable_done);
		init_completion(&visp_shared_subdev_refs[i].enable_done);
	}

	visp_shared_subdev_refs_init_done = true;
}

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

/*
 * LILO's mbus codes for NV16/NV12 differ from LIMO's (UYVY8_1X16/
 * VYYUYY8_1X24 here vs. YUYV8_2X8/YUYV8_1_5X8 in visp_mp_fmts[] above).
 * Every other entry mirrors visp_mp_fmts[] verbatim - kept as a full
 * separate table (rather than mutating visp_mp_fmts[] in place) since
 * that table is a shared global read by every visp_dev instance,
 * including concurrently-probed LIMO instances; mutating it per-mode
 * would be unsafe. The static_assert() below catches, at build time, the
 * most common way these two tables could drift apart: a future entry
 * added to one and forgotten in the other.
 */
struct visp_format visp_lilo_mp_fmts[] = {
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
		.code = MEDIA_BUS_FMT_RBG888_1X24, /* RBG (not RGB) is intentional, not a typo */
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

static_assert(ARRAY_SIZE(visp_mp_fmts) == ARRAY_SIZE(visp_lilo_mp_fmts),
	      "visp_mp_fmts[] and visp_lilo_mp_fmts[] must stay in sync - see visp_lilo_mp_fmts[] comment");

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

/* LILO variant of visp_sp_fmts[] above - see visp_lilo_mp_fmts[] comment */
struct visp_format visp_lilo_sp_fmts[] = {
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
		.code = MEDIA_BUS_FMT_RBG888_1X24, /* RBG (not RGB) is intentional, not a typo */
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

static_assert(ARRAY_SIZE(visp_sp_fmts) == ARRAY_SIZE(visp_lilo_sp_fmts),
	      "visp_sp_fmts[] and visp_lilo_sp_fmts[] must stay in sync - see visp_lilo_mp_fmts[] comment");

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
 * @fmts: Per-pad format table to search (isp_mode-appropriate - see
 *	  visp_pads_init(), which already picks visp_mp_fmts[]/visp_sp_fmts[]
 *	  for LIMO or visp_lilo_mp_fmts[]/visp_lilo_sp_fmts[] for LILO)
 * @num_formats: Number of entries in @fmts
 * @mbus_code: Media bus format code to search for
 *
 * Returns the first matching fourcc code for the given mbus format,
 * or 0 if no match is found. Searches only the calling pad's own table
 * instead of the global LIMO-only visp_mp_fmts[]/visp_sp_fmts[] tables
 * directly, so it also matches LILO's mbus codes (UYVY8_1X16/
 * VYYUYY8_1X24) when called for a LILO pad - matches the pattern already
 * used by visp_enum_mbus_code() below.
 */
static uint32_t visp_mbus_to_fourcc(struct visp_format *fmts,
				    uint32_t num_formats, uint32_t mbus_code)
{
	int i;

	for (i = 0; i < num_formats; i++) {
		if (fmts[i].code == mbus_code)
			return fmts[i].fourcc;
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
	bool stream_on;

	if (pad_buf->pad >= VISP_PAD_NR)
		return -EINVAL;
	int port = pad_buf->pad / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad_buf->pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	media_isp_chn_attr *IspChn = &isp_port->isp_chns[chn];

	cur_pad = &isp_dev->pad_data[pad_buf->pad];

	spin_lock_irqsave(&cur_pad->qlock, flags);

	/* Stamp this buffer with the streaming session it belongs to. */
	pad_buf->buf->stream_gen = cur_pad->stream_gen;
	list_add_tail(&pad_buf->buf->list, &cur_pad->queue);

	spin_unlock_irqrestore(&cur_pad->qlock, flags);

	buf.index = pad_buf->buf->sequence;
	buf.num_planes = pad_buf->buf->num_planes;

	for (i = 0; i < buf.num_planes; i++) {
		buf.planes[i].dma_addr = pad_buf->buf->planes[i].dma_addr;
		buf.planes[i].dma_size = pad_buf->buf->planes[i].size;
	}

	/*
	 * Serialize the streamon read against port_lock (held by
	 * media_isp_force_stream_off()/visp_pad_s_stream() while flipping it)
	 * so this never dispatches an ENQ off a torn/stale read; the lock is
	 * released before the actual ENQ send/dispatch below so the mailbox
	 * wait never blocks a concurrent stream-off.
	 */
	mutex_lock(&isp_dev->port_lock[port]);
	stream_on = isp_dev->streamon[pad_buf->pad] != 0;
	mutex_unlock(&isp_dev->port_lock[port]);

	if (/*IspChn->ThreadStatus == MEDIA_THREAD_STOPPED*/ !stream_on) {
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
		if (ret_val == -ESHUTDOWN) {
			/* Stream already stopped: silently drop stale frameout buffer. */
			ret_val = 0;
			goto error_free_buf;
		}
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

/*
 * Only used by LILO's visp_pad_s_stream branch (see below): scans forward
 * from `port` for the first sink pad with a resolvable upstream subdev.
 * LIMO does its own upstream discovery via visp_find_subdev_any() /
 * visp_shared_subdev_* instead.
 */
static struct v4l2_subdev *visp_get_input_subdev(struct visp_dev *isp_dev,
						  int port)
{
	struct media_pad *remote_pad;
	struct v4l2_subdev *subdev;
	int pad;

	dev_dbg(isp_dev->dev, "Searching for input sub-device...\n");

	for (pad = port; pad < isp_dev->num_pads; pad++) {
		/* Check if this pad is a SINK (input pad) */
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
		dev_dbg(isp_dev->dev, "Found input sub-device: %s on pad %d\n",
			subdev->name, pad);

		return subdev; /* Return the first valid input sub-device found */
	}

	return NULL;
}

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
		if (!visp_shared_subdev_refs[i].sd && !visp_shared_subdev_refs[i].disabling)
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

retry:
	mutex_lock(&visp_shared_subdev_lock);
	visp_shared_subdev_refs_ensure_init();

	slot = visp_shared_subdev_find_slot(sd);
	if (slot >= 0) {
		if (visp_shared_subdev_refs[slot].disabling) {
			/*
			 * A put() for this same subdev has dropped refcnt to 0
			 * and is mid s_stream(0); the slot is still "sd == this
			 * sd" so it isn't free, but it also isn't safe to just
			 * bump refcnt and treat the subdev as already streaming.
			 * Wait for that disable to finish, then re-run the
			 * whole lookup - by then the slot is genuinely free and
			 * this call does its own fresh s_stream(1).
			 */
			mutex_unlock(&visp_shared_subdev_lock);
			if (sd->dev)
				dev_dbg(sd->dev,
					"Upstream subdev '%s': stream_get waiting for in-flight s_stream(0) on slot=%d\n",
					sd->name, slot);
			wait_for_completion(&visp_shared_subdev_refs[slot].disable_done);
			goto retry;
		}
		if (visp_shared_subdev_refs[slot].enabling) {
			/*
			 * A get() for this same subdev already published
			 * sd/refcnt=1 and is mid s_stream(1); it isn't safe to
			 * just bump refcnt and return success without knowing
			 * whether that s_stream(1) actually succeeds. Wait for
			 * it to finish, then re-run the whole lookup: if it
			 * succeeded, this call will see a normal occupied slot
			 * and just bump refcnt below; if it failed, that
			 * call's own cleanup will have freed the slot by then
			 * and this call does its own fresh s_stream(1).
			 */
			mutex_unlock(&visp_shared_subdev_lock);
			if (sd->dev)
				dev_dbg(sd->dev,
					"Upstream subdev '%s': stream_get waiting for in-flight s_stream(1) on slot=%d\n",
					sd->name, slot);
			wait_for_completion(&visp_shared_subdev_refs[slot].enable_done);
			goto retry;
		}
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
	/*
	 * Mark the slot as enabling rather than leaving it look like an
	 * already-streaming occupied slot: sd/refcnt are published so
	 * find_free_slot() cannot hand this slot to anyone else, but a
	 * concurrent get() for this same sd must wait on enable_done instead
	 * of racing an overlapping s_stream(1) or trusting an unfinished one -
	 * see the "enabling" branch above.
	 */
	visp_shared_subdev_refs[slot].enabling = true;
	reinit_completion(&visp_shared_subdev_refs[slot].enable_done);
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
			/*
			 * Only this get() call ever sets enabling=true for a
			 * given occupancy of the slot (any concurrent get()
			 * for the same sd waits above instead of racing in),
			 * so refcnt is still guaranteed to be exactly 1 here -
			 * always safe to clear unconditionally on failure.
			 */
			visp_shared_subdev_refs[slot].sd = NULL;
			visp_shared_subdev_refs[slot].refcnt = 0;
			visp_shared_subdev_refs[slot].enabling = false;
			mutex_unlock(&visp_shared_subdev_lock);
			complete_all(&visp_shared_subdev_refs[slot].enable_done);
		} else {
			if (sd->dev)
				dev_dbg(sd->dev,
					"Upstream subdev '%s': s_stream(1) success slot=%d\n",
					sd->name, slot);
			mutex_lock(&visp_shared_subdev_lock);
			visp_shared_subdev_refs[slot].enabling = false;
			mutex_unlock(&visp_shared_subdev_lock);
			complete_all(&visp_shared_subdev_refs[slot].enable_done);
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
	visp_shared_subdev_refs_ensure_init();

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
		/*
		 * Mark the slot as disabling rather than clearing sd here:
		 * sd stays set (and disabling=true) for the whole s_stream(0)
		 * call below, so find_free_slot() cannot hand this slot to a
		 * concurrent get() - and a get() for this same sd waits on
		 * disable_done instead of racing s_stream(1) against this
		 * s_stream(0) - until the slot is actually freed further down.
		 */
		visp_shared_subdev_refs[slot].disabling = true;
		reinit_completion(&visp_shared_subdev_refs[slot].disable_done);
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

	if (need_stream_off) {
		mutex_lock(&visp_shared_subdev_lock);
		/*
		 * Only this put() call ever sets disabling=true for a given
		 * occupancy of the slot (refcnt can't leave 0 for a waiting
		 * get() until the slot is freed here), so it's always safe to
		 * clear both fields unconditionally at this point.
		 */
		visp_shared_subdev_refs[slot].sd = NULL;
		visp_shared_subdev_refs[slot].disabling = false;
		mutex_unlock(&visp_shared_subdev_lock);
		complete_all(&visp_shared_subdev_refs[slot].disable_done);
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
	struct visp_isp_dev_extended *ext = ISP_DEV_EXTENDED(isp_dev);
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
	struct visp_isp_dev_extended *ext = ISP_DEV_EXTENDED(isp_dev);
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
 * visp_get_subdev - Find a registered ISP subdev by its own DT node.
 * @np: device_node of the ISP subdev itself (not one of its upstream/downstream
 *      peers, unlike visp_find_subdev_any()).
 *
 * Lets visp_video locate and bind to an ISP subdev's memory-out pad by phandle
 * (LILO mixed mode) without requiring an OF-graph endpoint on that pad - see
 * struct visp_media_dev's phandle_mode comment in visp_video_driver.h.
 *
 * The mdev-readiness check (sd->v4l2_dev && sd->v4l2_dev->mdev) is done here,
 * under visp_dev_global_mutex, rather than left to the caller after this
 * function returns: peer is only guaranteed live while the mutex is held - a
 * concurrent unbind of that specific peer (e.g. a device_release_driver()/
 * device_attach() rebind cycle) could otherwise free it in the window between
 * this function's mutex_unlock() and the caller dereferencing sd->v4l2_dev.
 *
 * Returns the matching v4l2_subdev only once it's actually bound into a media
 * graph, or NULL otherwise (not found, or found but not yet ready) - caller
 * should treat NULL as -EPROBE_DEFER: the owning visp instance may not have
 * probed (far enough) yet.
 */
struct v4l2_subdev *visp_get_subdev(struct device_node *np)
{
	struct visp_dev *peer;
	struct v4l2_subdev *sd = NULL;

	if (!np)
		return NULL;

	mutex_lock(&visp_dev_global_mutex);
	list_for_each_entry(peer, &visp_dev_global_list, global_entry) {
		if (peer->dev->of_node == np) {
			if (peer->sd.v4l2_dev && peer->sd.v4l2_dev->mdev)
				sd = &peer->sd;
			break;
		}
	}
	mutex_unlock(&visp_dev_global_mutex);

	return sd;
}
EXPORT_SYMBOL_GPL(visp_get_subdev);

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
	struct visp_isp_dev_extended *ext = ISP_DEV_EXTENDED(isp_dev);
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
		struct visp_isp_dev_extended *_ext = ISP_DEV_EXTENDED(isp_dev);

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

void visp_wdt_release_port_upstream(struct visp_dev *isp_dev, int port)
{
	if (!isp_dev || port < 0 || port >= MAX_PORTS)
		return;

	visp_stream_pipeline_subdevs(isp_dev, port, 0);
}

void visp_wdt_release_lilo_upstream(struct visp_dev *isp_dev, int port, uint8_t chn)
{
	struct visp_isp_dev_extended *ext;
	struct v4l2_subdev *subdev;

	if (!isp_dev || port < 0 || port >= MAX_PORTS)
		return;

	/*
	 * Mirror the normal streamoff path's gating: this port's shared
	 * upstream subdev only gets exactly one get()/put() pair per
	 * boundary, so only release it once every chn on this port is down,
	 * not once per active chn in the force-teardown loop (that would
	 * over-decrement visp_shared_subdev_refs[], a table shared by every
	 * isp_dev instance, on behalf of a port that still has an active
	 * sibling chn).
	 */
	if (media_isp_mixed_mode_sibling_active(isp_dev, port, chn))
		return;

	ext = ISP_DEV_EXTENDED(isp_dev);
	/* Prefer the cached pointer captured at get() time; plain-LILO (no
	 * per_path_out_type) never populates it, so fall back to a fresh
	 * lookup for that case.
	 */
	subdev = ext->upstream_subdev[port];
	if (!subdev)
		subdev = visp_get_input_subdev(isp_dev, port);
	if (subdev)
		visp_shared_subdev_stream_put(subdev);
	ext->upstream_subdev[port] = NULL;
}

static bool visp_port_has_active_stream(struct visp_dev *isp_dev, int port)
{
	int chn;

	if (!isp_dev || port < 0 || port >= VISP_PORT_NR)
		return false;

	for (chn = 0; chn < MEDIA_ISP_CHN_MAX; chn++) {
		int pad = (port * MEDIA_ISP_PORT_PAD_COUNT) + chn + 1;

		if (isp_dev->streamon[pad] != 0)
			return true;
	}

	return false;
}

static int visp_pad_s_stream(struct v4l2_subdev *sd, void *arg)
{
	struct visp_pad_stream_status *pad_stream =
	    (struct visp_pad_stream_status *)arg;
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_isp_dev_extended *ext = ISP_DEV_EXTENDED(isp_dev);
	unsigned long flags;
	int ret = 0;
	bool upstream_started = false;
	bool was_streaming = false;

	if (pad_stream->pad >= VISP_PAD_NR)
		return -EINVAL;
	int port = pad_stream->pad / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad_stream->pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

	/*
	 * pad_stream->pad is fully userspace-controlled (VISP_PAD_S_STREAM is
	 * a private ioctl on /dev/v4l-subdevX) - a sink pad (pad a multiple
	 * of MEDIA_ISP_PORT_PAD_COUNT) yields chn == -1 here, which the
	 * mixed-mode branch below dereferences directly as
	 * output_type[port][chn] (an out-of-bounds read one element before
	 * the row) and which media_isp_device_set_format()/_stream_on()
	 * would use to index isp_chns[chn] (an out-of-bounds write) if
	 * allowed through. Streaming a sink pad was never a valid request
	 * for any isp_mode - reject it here once, for every branch below.
	 */
	if (chn < 0)
		return -EINVAL;

	if (isp_dev->isp_mode == ISP_MODE_LILO && ext->per_path_out_type) {
		/*
		 * LILO mixed-mode: the live path (xilinx-vipp video.s_stream,
		 * via visp_s_stream() below) and the memory path (visp_video's
		 * VISP_PAD_S_STREAM) stream on/off independently but share one
		 * sensor + ISP pipeline. port_stream_refcnt[port] (dedicated -
		 * deliberately not LIMO's subdev_streamon_count[] above, which
		 * tracks a different thing) does the shared bring-up once
		 * (whichever path starts first) and the shared teardown once
		 * the last path stops; only the triggering pad's own
		 * format/stream-on is touched per call. Upstream subdev
		 * enable/disable is routed through visp_shared_subdev_stream_
		 * get()/_put() (see the plain-LILO branch below for why: the
		 * same shared-refs table is used by every isp_dev in this
		 * module, so a bare v4l2_subdev_call() here could kill another
		 * instance's stream if this port's upstream subdev turns out
		 * to be shared).
		 *
		 * Lock scope matches the plain-LILO/LIMO branches below: held
		 * across the entire streamon-or-streamoff body, not just
		 * around the initial visp_setup_isp_pipeline() call - the
		 * boundary bring-up (set_frame_rate + shared_subdev_stream_
		 * get) and every path's own format/stream-on must complete
		 * before a concurrent caller on the other path can observe
		 * this port as "already up" and start streaming against a
		 * pipeline whose upstream isn't actually enabled yet.
		 */
		bool memory_path = (isp_dev->output_type[port][chn] ==
				    VISP_PATH_OUT_TYPE_MEMORY);
		bool boundary;
		/*
		 * Tracks whether visp_shared_subdev_stream_get() actually
		 * succeeded for this call, so mixed_on_err below only calls
		 * the matching put() when a get() truly happened.
		 * visp_setup_isp_pipeline() and media_isp_device_set_frame_rate()
		 * both run *before* stream_get() and can fail on their own -
		 * without this flag, that failure would fall through to an
		 * unconditional visp_shared_subdev_stream_put(), decrementing
		 * (or forcing s_stream(0) on) a shared upstream subdev's
		 * refcount without this call ever having taken a reference on
		 * it. Since visp_shared_subdev_refs[] is one table shared by
		 * every isp_dev instance, that could silently stream off
		 * another ISP's still-active shared MIPI/broadcaster.
		 */
		bool got_subdev_ref = false;
		struct v4l2_subdev *subdev;

		mutex_lock(&isp_dev->port_lock[port]);

		if (pad_stream->status == 1) {
			isp_dev->pad_data[pad_stream->pad].stream = 1;

			boundary = (ext->port_stream_refcnt[port]++ == 0);
			if (boundary) {
				ret = visp_setup_isp_pipeline(isp_dev,
							      pad_stream->pad);
				if (ret)
					goto mixed_on_err;
			}

			spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock,
					  flags);
			isp_dev->pad_data[pad_stream->pad].sequence = 0;
			spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock,
					       flags);

			if (boundary) {
				ret = media_isp_device_set_frame_rate(
				    isp_dev, port,
				    &isp_dev->isp_ports[port].sensor_info.frame_rate);
				if (ret != VSI_SUCCESS)
					goto mixed_on_err;
				subdev = visp_get_input_subdev(isp_dev, port);
				if (subdev) {
					ret = visp_shared_subdev_stream_get(subdev);
					if (ret)
						goto mixed_on_err;
					got_subdev_ref = true;
					ext->upstream_subdev[port] = subdev;
				}
			}

			if (memory_path) {
				/* memory-out pad (visp_video): set up this pad */
				ret = media_isp_device_set_format(isp_dev, port,
								  chn);
				if (ret)
					goto mixed_on_err;
				ret = media_isp_device_stream_on(isp_dev, port,
								 chn);
				if (ret)
					goto mixed_on_err;
				isp_dev->streamon[pad_stream->pad] = 1;
			} else {
				/* live-out pads: DT-graph walk */
				ret = visp_stream_on(isp_dev);
				if (ret) {
					/*
					 * visp_stream_on() walks every DT-graph
					 * pad for this port and returns on the
					 * first channel that fails - any earlier
					 * channel that already succeeded is left
					 * actively streaming. RPU must stop
					 * before mixed_on_err's shared MIPI
					 * disable (visp_shared_subdev_stream_put())
					 * runs on the port_stream_refcnt boundary
					 * below - same ordering, and same reason,
					 * as the plain-LILO branch's
					 * ERR_TO_CAMERA_DISCONNECT handling of
					 * this identical failure: disabling MIPI
					 * first can hang the RPU waiting on a
					 * source that's already gone.
					 * visp_stream_off() self-guards per-pad
					 * internally, so this is a harmless no-op
					 * for any channel that never got as far
					 * as streaming.
					 */
					visp_stream_off(isp_dev);
					goto mixed_on_err;
				}
			}
			mutex_unlock(&isp_dev->port_lock[port]);
			return 0;

mixed_on_err:
			isp_dev->streamon[pad_stream->pad] = 0;
			isp_dev->pad_data[pad_stream->pad].stream = 0;

			/*
			 * Not currently reachable by any in-flight HW
			 * completion (this streamon attempt never got far
			 * enough to have a buffer in flight), but reset the
			 * pad's queue/sequence/generation the same way every
			 * real streamoff site does, so a failed streamon
			 * leaves this pad in the same clean state a normal
			 * stream-off would - matching invariant symmetry
			 * rather than relying on this path staying
			 * unreachable.
			 */
			spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
			INIT_LIST_HEAD(&isp_dev->pad_data[pad_stream->pad].queue);
			isp_dev->pad_data[pad_stream->pad].sequence = 0;
			isp_dev->pad_data[pad_stream->pad].stream_gen++;
			spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock,
					       flags);

			if (ext->port_stream_refcnt[port] > 0)
				ext->port_stream_refcnt[port]--;
			/*
			 * Whether to actually tear the port's shared camera
			 * connection down must be decided from the real per-chn
			 * streaming state, not port_stream_refcnt[port]: the
			 * live-path's visp_stream_on() (above) can stream on
			 * more than one live chn in a single call, so a sibling
			 * from that same call may already be up even though
			 * port_stream_refcnt[port] was only incremented once
			 * for this whole call - see
			 * media_isp_mixed_mode_sibling_active()'s comment.
			 */
			if (!media_isp_mixed_mode_sibling_active(isp_dev, port, chn)) {
				media_isp_device_camera_dis_connect(isp_dev,
								    port, 0);
				isp_destroy_pipeline(isp_dev, port, 0);
				if (got_subdev_ref) {
					subdev = ext->upstream_subdev[port];
					if (subdev)
						visp_shared_subdev_stream_put(subdev);
					ext->upstream_subdev[port] = NULL;
				}
			}
			mutex_unlock(&isp_dev->port_lock[port]);
			return ret;
		}

		/*
		 * streamoff. Guard the memory path's own teardown against a
		 * duplicate/erroneous call (media_isp_device_stream_off()
		 * sends real RPU buf-pool teardown commands with no internal
		 * streaming check of its own) - the live path's
		 * visp_stream_off() already self-guards per-pad internally.
		 */
		if (memory_path) {
			if (isp_dev->streamon[pad_stream->pad])
				media_isp_device_stream_off(isp_dev, port, chn);
		} else {
			visp_stream_off(isp_dev);
		}

		isp_dev->streamon[pad_stream->pad] = 0;
		isp_dev->pad_data[pad_stream->pad].stream = 0;

		spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
		INIT_LIST_HEAD(&isp_dev->pad_data[pad_stream->pad].queue);
		isp_dev->pad_data[pad_stream->pad].sequence = 0;
		/*
		 * Bump the generation here too - this is the mixed-mode
		 * memory-out chn's own streamoff, distinct from the
		 * plain-LILO and LIMO branches' streamoff sites below, which
		 * each already do this. See visp_driver.h's stream_gen
		 * comment.
		 */
		isp_dev->pad_data[pad_stream->pad].stream_gen++;
		spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock,
				       flags);

		if (ext->port_stream_refcnt[port] > 0)
			ext->port_stream_refcnt[port]--;
		/* See mixed_on_err's comment on why this checks the real
		 * per-chn streaming state instead of port_stream_refcnt[port].
		 */
		if (!media_isp_mixed_mode_sibling_active(isp_dev, port, chn)) {
			media_isp_device_camera_dis_connect(isp_dev, port, 0);
			isp_destroy_pipeline(isp_dev, port, 0);
			subdev = ext->upstream_subdev[port];
			if (subdev)
				visp_shared_subdev_stream_put(subdev);
			ext->upstream_subdev[port] = NULL;
		}
		mutex_unlock(&isp_dev->port_lock[port]);
		return 0;
	}

	if (isp_dev->isp_mode == ISP_MODE_LILO) {
		/*
		 * LILO's own single-upstream-subdev streaming sequence
		 * (visp_get_input_subdev + visp_stream_on/off's OBA mailbox
		 * calls), distinct from LIMO's refcounted multi-path
		 * pipeline-subdev approach below. Not unified into one
		 * algorithm: LILO has no per-port active-path refcounting
		 * today (its own TODO here says as much), and folding it
		 * into LIMO's scheme needs its own validated change, not a
		 * side effect of this file-level merge.
		 *
		 * The upstream subdev itself IS put through LIMO's shared
		 * visp_shared_subdev_stream_get/put() refcounting below,
		 * since visp_shared_subdev_refs[] is one table shared by
		 * every isp_dev in this module - a LILO port and a LIMO
		 * instance sharing the same upstream subdev (e.g. a common
		 * broadcaster) must not each call s_stream() on it directly,
		 * or one mode's stream-off silently kills the other's active
		 * stream. Calling s_stream() directly here would be safe
		 * only if no such shared topology can ever exist, which
		 * isn't something this driver enforces.
		 */
		struct v4l2_subdev *subdev;

		/*
		 * Lock scope matches LIMO's below: held across the entire
		 * streamon-or-streamoff body, released once before return
		 * (including via ERR_TO_CAMERA_DISCONNECT), instead of only
		 * around the initial visp_setup_isp_pipeline() call.
		 */
		mutex_lock(&isp_dev->port_lock[port]);

		if (pad_stream->status == 1) {
			/* streamon */
			isp_dev->pad_data[pad_stream->pad].stream = pad_stream->status;
			ret = visp_setup_isp_pipeline(isp_dev, pad_stream->pad);
			if (ret)
				goto ERR_TO_CAMERA_DISCONNECT;

			/* Reset sequence counter on streamon */
			spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
			isp_dev->pad_data[pad_stream->pad].sequence = 0;
			spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock, flags);

			/*
			 * TODO: set_frame_rate is applied once per port (on first active
			 * path). When independent per-path stream-on lands, replace this
			 * with a per-port "frame_rate_applied" flag, cleared on last
			 * stream-off, so a fresh path arriving after a full port
			 * stream-off re-applies it.
			 */
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
			if (subdev) {
				ret = visp_shared_subdev_stream_get(subdev);
				if (ret) {
					dev_err(isp_dev->dev,
						"visp_shared_subdev_stream_get failed isp:%d port:%d ret:%d\n",
						isp_dev->id, port, ret);
					goto ERR_TO_CAMERA_DISCONNECT;
				}
			}

			ret = visp_stream_on(isp_dev);
			if (ret != 0) {
				dev_err(isp_dev->dev, "visp_stream_on failed isp:%d port:%d\n",
					isp_dev->id, port);
				/*
				 * RPU must stop before the shared MIPI subdev is
				 * disabled - same order as the normal streamoff
				 * path below. Disabling MIPI first can hang the
				 * RPU waiting on a source that's already gone.
				 */
				visp_stream_off(isp_dev);
				if (subdev)
					visp_shared_subdev_stream_put(subdev);
				goto ERR_TO_CAMERA_DISCONNECT;
			}
		} else {
			/* streamoff */
			visp_stream_off(isp_dev);

			chn = 0;

			media_isp_device_camera_dis_connect(isp_dev, port, chn);

			isp_destroy_pipeline(isp_dev, port, chn);

			subdev = visp_get_input_subdev(isp_dev, port);
			if (!subdev) {
				dev_err(isp_dev->dev, "No valid input sub-device found!\n");
			} else {
				visp_shared_subdev_stream_put(subdev);
			}
			isp_dev->streamon[pad_stream->pad] = 0;
			isp_dev->pad_data[pad_stream->pad].stream = pad_stream->status;

			spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
			INIT_LIST_HEAD(&isp_dev->pad_data[pad_stream->pad].queue);
			/* Reset sequence counter on streamoff */
			isp_dev->pad_data[pad_stream->pad].sequence = 0;
			/*
			 * Bump the generation so any buffer already queued
			 * before the next streamon - and every buffer queued
			 * after - is distinguishable from this now-closed
			 * session. See visp_driver.h's stream_gen comment.
			 */
			isp_dev->pad_data[pad_stream->pad].stream_gen++;
			spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
		}

		mutex_unlock(&isp_dev->port_lock[port]);
		return ret;

ERR_TO_CAMERA_DISCONNECT:
		visp_stream_off(isp_dev);
		isp_dev->streamon[pad_stream->pad] = 0;
		isp_dev->pad_data[pad_stream->pad].stream = 0;

		/*
		 * Same invariant-symmetry reset as mixed_on_err above and the
		 * real streamoff sites below - not reachable by any in-flight
		 * completion today (this streamon attempt never got far
		 * enough), but keeps a failed streamon from leaving stale
		 * queue/sequence/generation state behind.
		 */
		spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
		INIT_LIST_HEAD(&isp_dev->pad_data[pad_stream->pad].queue);
		isp_dev->pad_data[pad_stream->pad].sequence = 0;
		isp_dev->pad_data[pad_stream->pad].stream_gen++;
		spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock,
				       flags);

		mutex_unlock(&isp_dev->port_lock[port]);
		return ret;
	}

	mutex_lock(&isp_dev->port_lock[port]);
	if (isp_dev->pad_data[pad_stream->pad].stream == 0 &&
	    pad_stream->status == 1) {
		isp_dev->pad_data[pad_stream->pad].stream = pad_stream->status;
		ret = visp_setup_isp_pipeline(isp_dev, pad_stream->pad);
		if (ret)
			goto ERR_TO_RPU_LOCK;

		/* Reset sequence counter on streamon */
		spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
		isp_dev->pad_data[pad_stream->pad].sequence = 0;
		spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock, flags);

		/*
		 * subdev_streamon_count[port] tracks active output paths on this port.
		 * Start the shared upstream pipeline only for the first active path,
		 * regardless of whether that path is MP, SP1, SP2 or RAW.
		 */
		if (ext->subdev_streamon_count[port] == 0) {
			ret = visp_stream_pipeline_subdevs(isp_dev, port, 1);
			if (ret) {
				dev_err(isp_dev->dev,
					"Failed to start pipeline streaming on port %d: %d\n",
					port, ret);
				goto ERR_TO_RPU_LOCK;
			}
			upstream_started = true;

			ret = media_isp_device_set_frame_rate(isp_dev, port,
					&isp_dev->isp_ports[port].sensor_info.frame_rate);
			if (ret != VSI_SUCCESS) {
				dev_err(isp_dev->dev,
					"%s isp:%d port %d chn %d Set frame_rate failed, ret is %d",
					__func__, isp_dev->id, port, chn, ret);
				ret = -EINVAL;
				goto ERR_TO_RPU_LOCK;
			}
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
		ext->subdev_streamon_count[port]++;
		/* EXIT PORT Level CRITICAL SECTION */
	} else {
		was_streaming = isp_dev->streamon[pad_stream->pad];
		isp_dev->pad_data[pad_stream->pad].stream = pad_stream->status;

		if (was_streaming) {
			if (ext->subdev_streamon_count[port] == 0)
				dev_warn(isp_dev->dev,
					 "ISP:%d port:%d chn:%d stream-off with zero active-path refcount\n",
					 isp_dev->id, port, chn);
			else
				ext->subdev_streamon_count[port]--;

			media_isp_stream_off(isp_dev, port, chn);
			/* subdev_streamon_count
			 * if > 0 implies there are other pipelines still processing streams
			 * from this subdev.
			 * if == 0 implies that the last avaialble pipeline has arrived for
			 * stream off and proceeds to perform complete cleanup of input pipeline.
			 */
			if (ext->subdev_streamon_count[port] == 0 &&
			    !visp_port_has_active_stream(isp_dev, port)) {
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
		} else {
			dev_dbg(isp_dev->dev,
				 "ISP:%d port:%d chn:%d stream-off ignored because path is not active\n",
				 isp_dev->id, port, chn);
		}

		spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
		INIT_LIST_HEAD(&isp_dev->pad_data[pad_stream->pad].queue);
		/* Reset sequence counter on streamoff */
		isp_dev->pad_data[pad_stream->pad].sequence = 0;
		/*
		 * Bump the generation so any buffer already queued before the
		 * next streamon - and every buffer queued after - is
		 * distinguishable from this now-closed session. See
		 * visp_driver.h's stream_gen comment.
		 */
		isp_dev->pad_data[pad_stream->pad].stream_gen++;
		spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
	}

	mutex_unlock(&isp_dev->port_lock[port]);

	return ret;

ERR_TO_RPU_LOCK:
	isp_dev->streamon[pad_stream->pad] = 0;
	isp_dev->pad_data[pad_stream->pad].stream = 0;
	if (upstream_started && ext->subdev_streamon_count[port] == 0) {
		int stop_ret = visp_stream_pipeline_subdevs(isp_dev, port, 0);

		if (stop_ret)
			dev_warn(isp_dev->dev,
				 "Failed to roll back pipeline streaming on port %d: %d\n",
				 port, stop_ret);
	}

	/*
	 * Same invariant-symmetry reset as mixed_on_err/ERR_TO_CAMERA_DISCONNECT
	 * above and the real streamoff sites below - not reachable by any
	 * in-flight completion today, but keeps a failed streamon from
	 * leaving stale queue/sequence/generation state behind.
	 */
	spin_lock_irqsave(&isp_dev->pad_data[pad_stream->pad].qlock, flags);
	INIT_LIST_HEAD(&isp_dev->pad_data[pad_stream->pad].queue);
	isp_dev->pad_data[pad_stream->pad].sequence = 0;
	isp_dev->pad_data[pad_stream->pad].stream_gen++;
	spin_unlock_irqrestore(&isp_dev->pad_data[pad_stream->pad].qlock, flags);

	mutex_unlock(&isp_dev->port_lock[port]);
	return ret;
}

/*
 * The standard v4l2_subdev video_ops.s_stream hook. LIMO's live pipeline
 * is driven exclusively through the private VISP_PAD_S_STREAM ioctl
 * (visp_pad_s_stream above), so this is a no-op there - matching the
 * pre-unification LIMO tree, which left .s_stream unset entirely. LILO
 * needs it active because vipp's v-frmbuf-wr chain calls the standard
 * subdev s_stream op on its live-out path; visp_video_ops is one shared
 * table for both modes, so the mode check has to live here rather than
 * at the ops-table level.
 */
static int visp_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_stream_status pad_stream = {1, enable};
	int ret;

	if (isp_dev->isp_mode != ISP_MODE_LILO)
		return 0;

	/*
	 * xilinx-vipp drives the LIVE path via this subdev-global
	 * video.s_stream. In mixed mode the live path isn't necessarily MP -
	 * target the first path this port actually configured as
	 * VISP_PATH_OUT_TYPE_STREAM instead of assuming pad 1.
	 */
	if (ISP_DEV_EXTENDED(isp_dev)->per_path_out_type) {
		int c;

		for (c = 0; c < VISP_PORT_PAD_NR - 1; c++) {
			if (isp_dev->output_type[0][c] == VISP_PATH_OUT_TYPE_STREAM) {
				pad_stream.pad = c + 1;
				break;
			}
		}
	}

	/*
	 * Propagate the real result instead of always returning 0 - a
	 * caller (xilinx-vipp) that gets a false "success" here has no way
	 * to know its stream-on actually failed underneath it. Coerce a
	 * positive result to a negative errno first: visp_pad_s_stream()'s
	 * failure paths can bubble up a raw VSI_ERR_* code (isp_device_create()
	 * returning VSI_ERR_TIMEOUT etc., all positive per visp_common.h) via
	 * visp_setup_isp_pipeline() - the standard .s_stream() contract checks
	 * "ret < 0" for failure, so an uncoerced positive value would be
	 * silently read as success by the v4l2/xilinx-vipp core, defeating
	 * the point of returning the real result at all.
	 */
	ret = visp_pad_s_stream(sd, &pad_stream);
	return ret > 0 ? -EIO : ret;
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
				bool first_frame;

				/* Set timestamp and sequence before marking done */
				buf->vb.vb2_buf.timestamp = ktime_get_ns();

				spin_lock_irqsave(&cur_pad->qlock, flags);
				/*
				 * Only trust cur_pad->sequence == 0 as "first
				 * frame of THIS session" when the completing
				 * buffer was actually queued under the
				 * current generation - otherwise a late
				 * completion for a prior, already-torn-down
				 * session (whose queue got reinitialized by a
				 * streamoff, then coincidentally reused the
				 * same buffer index in a fresh streamon)
				 * would be misreported as the new session's
				 * first frame.
				 */
				first_frame = (cur_pad->sequence == 0 &&
					       buf->stream_gen == cur_pad->stream_gen);
				buf->vb.sequence = cur_pad->sequence++;
				spin_unlock_irqrestore(&cur_pad->qlock, flags);

				if (first_frame) {
					int port = ubuf.pad / MEDIA_ISP_PORT_PAD_COUNT;
					int chn = (ubuf.pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

					dev_info(isp_dev->dev,
						 "ISP : %d Port : %d Chn : %d First Frame Arrival ts=%llu ns\n",
						 isp_dev->id, port, chn, buf->vb.vb2_buf.timestamp);
				}

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
	ext_buf_info->plane.dma_addr = ext_dma_buf->addr;

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
		/*
		 * visp_pad_s_stream()'s mixed-mode branch can bubble up a raw
		 * positive VSI_ERR_ or RET_ code on failure (isp_device_create(),
		 * media_fmt_to_isp_fmt(), vsi_cam_device_set_out_format(), etc.
		 * are all positive-only error domains). This ioctl is reached
		 * from visp_video_vb2_start_streaming() (VIDIOC_STREAMON on the
		 * mixed-mode memory-out video node), whose own return value
		 * flows unclamped up through vb2 core to the ioctl() syscall's
		 * return code - a positive value there is not "< 0" and would
		 * be silently read as success by standard V4L2 client code.
		 * Coerce here, once, at this shared ioctl dispatch point,
		 * rather than at each individual caller (visp_s_stream() has
		 * its own equivalent coercion for the same reason).
		 */
		ret = visp_pad_s_stream(sd, arg);
		if (ret > 0)
			ret = -EIO;
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
	.s_stream = visp_s_stream,
};

int media_isp_hal_mbus_fmt_to_media_fmt(uint32_t *code, uint32_t *pixel_format,
					 uint32_t fourcc_code, enum isp_mode mode);
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
		case VISP_PORT_PAD_SOURCE_RAW:
			if (isp_dev->isp_mode == ISP_MODE_LILO) {
				/* LILO's memory-out (OBA) path needs a default
				 * format registered with cam_device up front so
				 * vipp's link validation (which runs before
				 * streaming) sees a consistent pad format; LIMO
				 * relies on the normal lazy S_FMT flow instead.
				 */
				uint8_t out_path = (pad % VISP_PORT_PAD_NR) - 1;
				int out_port = pad / VISP_PORT_PAD_NR;
				media_fmt format_media = {0};
				uint32_t def_fourcc;
				/*
				 * RBG888_1X24 vs RGB888_1X24 is a software
				 * labeling convention, not a real channel-order
				 * difference in the pixel data - a memory-out
				 * chn is consumed like a LIMO chn (see
				 * visp_pads_init()'s fmts[] table selection for
				 * this same pad), so it must report the same
				 * mbus code LIMO's table pairs with RGB24
				 * (RGB888_1X24), or visp_mbus_to_fourcc()'s
				 * table-only code match silently fails to
				 * resolve a fourcc for this pad.
				 */
				bool out_is_memory =
					ISP_DEV_EXTENDED(isp_dev)->per_path_out_type &&
					out_path < VISP_PORT_PAD_NR - 1 &&
					isp_dev->output_type[out_port][out_path] ==
						VISP_PATH_OUT_TYPE_MEMORY;

				if (ISP_DEV_EXTENDED(isp_dev)->is_oba_yuv_420[out_path]) {
					pad_data->format.code = MEDIA_BUS_FMT_VYYUYY8_1X24;
					format_media.pixel_format = MEDIA_PIX_FMT_NV12;
					def_fourcc = V4L2_PIX_FMT_NV12;
				} else {
					pad_data->format.code = out_is_memory ?
						MEDIA_BUS_FMT_RGB888_1X24 :
						MEDIA_BUS_FMT_RBG888_1X24;
					format_media.pixel_format = V4L2_PIX_FMT_RGB24;
					def_fourcc = V4L2_PIX_FMT_RGB24;
				}
				pad_data->format.colorspace = V4L2_COLORSPACE_SRGB;
				/*
				 * Carry the V4L2 fourcc in reserved, matching what
				 * enum_mbus_code/set_fmt already do (struct
				 * format_reserved{fourcc,stride}, not a bare
				 * fourcc) - without this the default pad format
				 * has no fourcc hint, so a mixed-mode memory-out
				 * path's visp_video ENUM_FMT (before any SET_FMT)
				 * can't resolve a pixel format. stride is left 0:
				 * it isn't known until a real SET_FMT computes it.
				 */
				{
					struct format_reserved def_res = {
						.fourcc = def_fourcc,
						.stride = 0,
					};
					memcpy(pad_data->format.reserved, &def_res,
					       sizeof(def_res));
				}

				format_media.width = pad_data->format.width;
				format_media.height = pad_data->format.height;
				format_media.color_space = V4L2_COLORSPACE_SRGB;
				format_media.quantization = V4L2_QUANTIZATION_DEFAULT;
				/*
				 * Use `pad` directly, not isp_dev->pads[pad].index:
				 * this runs from visp_pads_init(), called BEFORE
				 * media_entity_pads_init() (which is what actually
				 * assigns pads[i].index = i) in visp_probe. Using
				 * the not-yet-initialized .index (always 0 here)
				 * sent every call to chn=-1 - an out-of-bounds
				 * write that silently corrupted adjacent memory
				 * instead of setting isp_chns[chn].format, leaving
				 * pixel_format at 0 and making media_fmt_to_isp_fmt
				 * fall through to its RAW24 default for every path.
				 */
				media_isp_set_format(isp_dev, pad, format_media);
			} else if ((pad % VISP_PORT_PAD_NR) == VISP_PORT_PAD_SOURCE_RAW) {
				/* Raw output pads preserve sensor format */
				pad_data->format.code = MEDIA_BUS_FMT_SRGGB12_1X12;
				pad_data->format.colorspace = V4L2_COLORSPACE_RAW;
			} else {
				/* Source pads use processed RGB format */
				pad_data->format.code = MEDIA_BUS_FMT_RGB888_1X24;
				pad_data->format.colorspace = V4L2_COLORSPACE_SRGB;
			}
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
	bool fmt_pad_is_memory;

	sink_pad_index = format->pad - (format->pad % VISP_PORT_PAD_NR);
	sink_pad = &isp_dev->pad_data[sink_pad_index];

	/*
	 * The is_oba_yuv_420[]/yuv_420_format_index[] matching below (and its
	 * "not supported on design" fallback-to-index-0/yuv_420_index
	 * behavior) is OBA/live-out-specific design machinery with no LIMO
	 * equivalent - a memory-out chn must instead go through LIMO's own,
	 * simpler exact-match loop further down, same as every other
	 * mixed-mode format decision this chn needs (fmts[] table, mbus
	 * code, media_fmt_to_isp_fmt() mode - see visp_pads_init(),
	 * set_default_pad_config(), and media_isp_device_set_format()).
	 * Using this branch for a memory-out chn is what previously
	 * resolved a requested RGB/UYVY format down to whatever index 0 or
	 * yuv_420_index happened to be in the LILO table, regardless of
	 * what was actually requested.
	 */
	{
		int fmt_pad_port = format->pad / VISP_PORT_PAD_NR;
		int fmt_pad_chn = (format->pad % VISP_PORT_PAD_NR) - 1;

		fmt_pad_is_memory = ISP_DEV_EXTENDED(isp_dev)->per_path_out_type &&
				    fmt_pad_chn >= 0 && fmt_pad_chn < VISP_PORT_PAD_NR - 1 &&
				    isp_dev->output_type[fmt_pad_port][fmt_pad_chn] ==
					    VISP_PATH_OUT_TYPE_MEMORY;
	}

	if (isp_dev->isp_mode == ISP_MODE_LILO && !fmt_pad_is_memory) {
		/*
		 * LILO's format table matching must special-case the
		 * OBA/YUV420 output type (is_oba_yuv_420[]/
		 * yuv_420_format_index[]), which has no LIMO equivalent -
		 * LIMO has no OBA output path in pure mode. Kept as a
		 * distinct body rather than folding the YUV420 branch into
		 * LIMO's simpler matching loop below.
		 */
		media_fmt format_media;
		struct v4l2_mbus_framefmt *mbus_format;
		uint32_t fourcc_code = 0;

		if (sink_pad == cur_pad) {
			cur_pad->sink_detected = 1;
			cur_pad->format = format->format;
			for (i = 1; i < VISP_PORT_PAD_NR; i++) {
				source_pad = &isp_dev->pad_data[sink_pad_index + i];
				source_pad->sink_detected = 1;
			}
			return 0;
		}

		{
			int port = format->pad / MEDIA_ISP_PORT_PAD_COUNT;

			mutex_lock(&isp_dev->port_lock[port]);
			ret = visp_setup_isp_pipeline(isp_dev, format->pad);
			mutex_unlock(&isp_dev->port_lock[port]);
		}
		if (ret)
			return ret;

		w = ALIGN(format->format.width, VISP_WIDTH_ALIGN);
		h = ALIGN(format->format.height, VISP_HEIGHT_ALIGN);
		/*
		 * Clamp the source-pad size to the sink (sensor input) size
		 * only when the sink pad has actually been sized. In LILO
		 * mixed-mode the memory path (visp_video) sets the source-pad
		 * format directly without the sink pad having been configured
		 * first, leaving sink_pad width/height 0 - clamping to 0 would
		 * collapse the format to 0x0 and userspace rejects it.
		 */
		if (sink_pad->format.width)
			w = clamp_t(uint32_t, w, VISP_WIDTH_MIN, sink_pad->format.width);
		else
			w = max_t(uint32_t, w, VISP_WIDTH_MIN);
		if (sink_pad->format.height)
			h = clamp_t(uint32_t, h, VISP_HEIGHT_MIN, sink_pad->format.height);
		else
			h = max_t(uint32_t, h, VISP_HEIGHT_MIN);

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
				if (i == yuv_420_index) {
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

		memset(&format_media, 0, sizeof(format_media));

		mbus_format = (struct v4l2_mbus_framefmt *)&format->format;
		/* Fill the struct to be shared with ISP/RPU*/
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
		ret = media_isp_hal_mbus_fmt_to_media_fmt(
			&mbus_format->code, &format_media.pixel_format, fourcc_code,
			isp_dev->isp_mode);
		if (ret)
			return ret;

		mediapad_t = &isp_dev->pads[format->pad];
		ret = media_isp_set_format(isp_dev, mediapad_t->index, format_media);
		if (ret)
			return ret;

		cur_pad->format = format->format;

		return 0;
	}

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

	{
		int port = format->pad / MEDIA_ISP_PORT_PAD_COUNT;

		mutex_lock(&isp_dev->port_lock[port]);
		ret = visp_setup_isp_pipeline(isp_dev, format->pad);
		mutex_unlock(&isp_dev->port_lock[port]);
	}
	if (ret)
		return ret;

	w = ALIGN(format->format.width, VISP_WIDTH_ALIGN);
	h = ALIGN(format->format.height, VISP_HEIGHT_ALIGN);
	/*
	 * Clamp the source-pad size to the sink (sensor input) size only when
	 * the sink pad has actually been sized. This tail of visp_set_fmt()
	 * runs for LIMO (the isp_mode == ISP_MODE_LILO branch above always
	 * returns before reaching here) and, since fmt_pad_is_memory now
	 * excludes a mixed-mode memory-out chn from that same branch's
	 * condition, for a LILO mixed-mode memory-out chn as well - both have
	 * the identical width/height-collapses-to-0 failure mode when the
	 * source pad is set before its sink is sized, and this is a strict
	 * improvement over the old unconditional clamp either way.
	 */
	if (sink_pad->format.width)
		w = clamp_t(uint32_t, w, VISP_WIDTH_MIN, sink_pad->format.width);
	else
		w = max_t(uint32_t, w, VISP_WIDTH_MIN);
	if (sink_pad->format.height)
		h = clamp_t(uint32_t, h, VISP_HEIGHT_MIN, sink_pad->format.height);
	else
		h = max_t(uint32_t, h, VISP_HEIGHT_MIN);

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

	/*
	 * This tail is reached for LIMO, and now also for a mixed-mode
	 * memory-out chn (see fmt_pad_is_memory above) - pass the effective
	 * mode so a memory-out chn's roundtrip_check inside this call uses
	 * LIMO's mbus<->fourcc mapping consistently with everything else in
	 * this tail, not isp_dev->isp_mode's real (LILO) value.
	 */
	ret = media_isp_hal_mbus_fmt_to_media_fmt(
		&MBusFormat->code, &Format_media.pixel_format, fmt_res.fourcc,
		fmt_pad_is_memory ? ISP_MODE_LIMO : isp_dev->isp_mode);
	if (ret)
		return ret;

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
	fourcc = visp_mbus_to_fourcc(pad_data->fmts, pad_data->num_formats,
				     pad_data->format.code);

	memcpy(format->format.reserved, &fourcc, sizeof(fourcc));

	return 0;
}

static int visp_enum_mbus_code(struct v4l2_subdev *sd,
			       struct v4l2_subdev_state *sd_state,
			       struct v4l2_subdev_mbus_code_enum *code)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_pad_data *pad_data = &isp_dev->pad_data[code->pad];
	int port = code->pad / MEDIA_ISP_PORT_PAD_COUNT;
	int ret = 0;

	mutex_lock(&isp_dev->port_lock[port]);
	ret = visp_setup_isp_pipeline(isp_dev, code->pad);
	mutex_unlock(&isp_dev->port_lock[port]);
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
	/* Only the first opener (isp_media_server) is tracked for WDT kill. */
	if (isp_dev->refcnt == 1)
		isp_dev->event_client_pid = get_pid(task_pid(current));

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
		put_pid(isp_dev->event_client_pid);
		isp_dev->event_client_pid = NULL;
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

static void visp_dev_release(struct kref *kref)
{
	struct visp_dev *isp_dev = container_of(kref, struct visp_dev, ref);

	/*
	 * pads/pad_data/extended_struct are kzalloc'd (not devm) precisely so
	 * they share isp_dev's own kref lifetime: a still-open
	 * /dev/v4l-subdevN fd can keep issuing ioctls that dereference them
	 * right up until this release actually runs, the same UAF class the
	 * isp_dev kref itself already exists to prevent - see visp_remove().
	 * kfree(NULL) is a no-op, so this is safe even on early probe-error
	 * paths where one or more of these was never allocated.
	 */
	kfree(isp_dev->pads);
	kfree(isp_dev->pad_data);
	kfree(isp_dev->extended_struct);
	kfree(isp_dev);
}

/* Last /dev/v4l-subdevN close drops the devnode ref so isp_dev (which embeds
 * ->sd) outlives the fd. Armed only on probe success so a probe-error
 * unregister does not free here.
 */
static void visp_sd_release(struct v4l2_subdev *sd)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);

	if (isp_dev && isp_dev->sd_release_armed)
		kref_put(&isp_dev->ref, visp_dev_release);
}

static struct v4l2_subdev_internal_ops visp_internal_ops = {
	.open = visp_open,
	.close = visp_close,
	.release = visp_sd_release,
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

	if (isp_dev->isp_mode == ISP_MODE_LILO) {
		/*
		 * LILO's own pad-index convention for this link (source_pad
		 * from link.remote_port, sink_pad from link.local_port) is
		 * the inverse of LIMO's below and is load-bearing for LILO's
		 * DT graph authoring - do not "fix" without HW validation.
		 * LILO also does not need LIMO's peer-ISP disambiguation
		 * (no broadcaster shared across multiple LILO ISPs today).
		 */
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
	struct visp_isp_dev_extended *ext = ISP_DEV_EXTENDED(isp_dev);
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

	if (isp_dev->isp_mode == ISP_MODE_LILO) {
		/*
		 * LILO's simpler per-pad endpoint registration. Deliberately
		 * NOT switched to LIMO's upstream_nodes[]/device-fwnode
		 * scheme below without HW validation on LILO's DT graphs:
		 * LILO has no multi-ISP-shares-one-broadcaster deployment
		 * today, so the _remote()-endpoint-fwnode approach (which
		 * LIMO's comment below warns against for that MCM case)
		 * is safe here and is LILO's own proven-working path.
		 */
		struct fwnode_handle *ep;
		struct fwnode_handle *remote_ep;
		int pad;

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
			asc = v4l2_async_notifier_add_fwnode_remote_subdev(
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
		} else {
			/* visp_remove()'s unregister is gated on this flag */
			ISP_DEV_EXTENDED(isp_dev)->notifier_registered = true;
		}

		return ret;
	}

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
	int num_pads = visp_get_num_pads(isp_dev);

	/* Allocate pads dynamically based on num_streams */
	isp_dev->num_pads = num_pads;
	/*
	 * kzalloc'd, not devm: freed by visp_dev_release() alongside isp_dev
	 * itself, so a still-open /dev/v4l-subdevN fd can't outlive this
	 * memory - see visp_remove() and visp_dev_release().
	 */
	isp_dev->pads = kcalloc(num_pads, sizeof(*isp_dev->pads), GFP_KERNEL);
	if (!isp_dev->pads) {
		dev_err(isp_dev->dev, "Failed to allocate pads\n");
		return -ENOMEM;
	}

	isp_dev->pad_data = kcalloc(num_pads, sizeof(*isp_dev->pad_data), GFP_KERNEL);
	if (!isp_dev->pad_data) {
		dev_err(isp_dev->dev, "Failed to allocate pad_data\n");
		return -ENOMEM;
	}

	for (pad = 0; pad < num_pads; pad++) {
		int mp_pad_port = pad / VISP_PORT_PAD_NR;
		int mp_pad_chn = (pad % VISP_PORT_PAD_NR) - 1;
		/*
		 * A LILO mixed-mode memory-out chn is consumed via vb2/
		 * visp_video, exactly like a LIMO chn - not via OBA/FBWR like
		 * every other LILO chn. Its fourcc<->mbus table and the
		 * downstream CAMDEV_PIX_FMT mapping (media_fmt_to_isp_fmt())
		 * must therefore follow LIMO's convention, not LILO's
		 * OBA-oriented one (e.g. LILO's RBG888_1X24 vs LIMO's
		 * RGB888_1X24 mbus code for RGB) - using LILO's own table for
		 * a memory-out chn resolves to the wrong CAMDEV_PIX_FMT and
		 * produces a garbled/tiled memory-out image.
		 */
		bool mp_pad_is_memory = ISP_DEV_EXTENDED(isp_dev)->per_path_out_type &&
					mp_pad_chn >= 0 && mp_pad_chn < VISP_PORT_PAD_NR - 1 &&
					isp_dev->output_type[mp_pad_port][mp_pad_chn] ==
						VISP_PATH_OUT_TYPE_MEMORY;

		if ((pad % VISP_PORT_PAD_NR) == VISP_PORT_PAD_SINK)
			isp_dev->pads[pad].flags = MEDIA_PAD_FL_SINK;
		else
			isp_dev->pads[pad].flags = MEDIA_PAD_FL_SOURCE;

		switch (pad % VISP_PORT_PAD_NR) {
		case VISP_PORT_PAD_SINK:
			break;
		case VISP_PORT_PAD_SOURCE_MP:
			if (isp_dev->isp_mode == ISP_MODE_LILO && !mp_pad_is_memory) {
				isp_dev->pad_data[pad].num_formats =
				    ARRAY_SIZE(visp_lilo_mp_fmts);
				isp_dev->pad_data[pad].fmts = visp_lilo_mp_fmts;
			} else {
				isp_dev->pad_data[pad].num_formats =
				    ARRAY_SIZE(visp_mp_fmts);
				isp_dev->pad_data[pad].fmts = visp_mp_fmts;
			}
			break;
		case VISP_PORT_PAD_SOURCE_SP1:
			if (isp_dev->isp_mode == ISP_MODE_LILO && !mp_pad_is_memory) {
				isp_dev->pad_data[pad].num_formats =
				    ARRAY_SIZE(visp_lilo_sp_fmts);
				isp_dev->pad_data[pad].fmts = visp_lilo_sp_fmts;
			} else {
				isp_dev->pad_data[pad].num_formats =
				    ARRAY_SIZE(visp_sp_fmts);
				isp_dev->pad_data[pad].fmts = visp_sp_fmts;
			}
			break;
		case VISP_PORT_PAD_SOURCE_SP2:
			if (isp_dev->isp_mode == ISP_MODE_LILO && !mp_pad_is_memory) {
				isp_dev->pad_data[pad].num_formats =
				    ARRAY_SIZE(visp_lilo_sp_fmts);
				isp_dev->pad_data[pad].fmts = visp_lilo_sp_fmts;
			} else {
				isp_dev->pad_data[pad].num_formats =
				    ARRAY_SIZE(visp_sp_fmts);
				isp_dev->pad_data[pad].fmts = visp_sp_fmts;
			}
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
 * LILO-only: resolves which entry in a source pad's format table (fmts[])
 * matches the design's YUV420 mbus code, caching the index in
 * yuv_420_format_index[] for visp_set_fmt's LILO branch to use. Must run
 * after visp_pads_init() has populated pad_data[].fmts.
 */
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

/*
 * LILO-only: parses the xlnx,obaN_{mp,sp}_* DT properties (OBA hardware
 * config for the memory/live-out path) and records whether each output
 * path is YUV420 in is_oba_yuv_420[], consumed by set_default_pad_config()
 * and visp_set_fmt()'s LILO branches.
 */
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

static const struct of_device_id visp_of_match[];

/*
 * Parse device tree parameters
 */
static int visp_parse_params(struct visp_dev *isp_dev,
			     struct platform_device *pdev)
{
	int port = 0;
	int ret = 0;
	struct device_node *node = pdev->dev.of_node;

	/* Read string property for SS-MODE-i0 (LIMO, etc.) */
	ret = of_property_read_string(node, "xlnx,io_mode", &isp_dev->ss_mode_i0);
	if (ret) {
		dev_err(&pdev->dev, "Failed to read xlnx,io_mode\n");
		return ret;
	} else {
		dev_dbg(&pdev->dev, "xlnx,io_mode: %s\n", isp_dev->ss_mode_i0);
	}

	/*
	 * isp_mode MUST come from the compatible string that matched this
	 * of_device_id entry (guaranteed correct - it's the reason this
	 * platform_driver bound to this node at all), NOT from xlnx,io_mode.
	 * That DT string was dead/unread before this unification
	 * (get_isp_mode_from_str had zero callers in either original
	 * driver), so overlays can carry a stale/inconsistent io_mode value
	 * that was completely harmless under the old separate single-mode
	 * drivers and only became load-bearing here. Confirmed on real HW:
	 * a node with compatible="xlnx,visp-ss-limo-1.0" (correctly bound as
	 * LIMO) but io_mode="lilo" (stale) - using io_mode as authoritative
	 * silently ran LILO's default-pad-config/output-type logic on a
	 * true LIMO instance. isp_mode is what the rest of the unified
	 * driver (starting right below, with the hw_mcm gate) branches on,
	 * so this must run before anything else in parse_params that checks
	 * isp_mode.
	 */
	{
		const struct of_device_id *match =
			of_match_device(visp_of_match, &pdev->dev);

		if (!match) {
			dev_err(&pdev->dev, "No of_match entry for this device\n");
			return -EINVAL;
		}
		isp_dev->isp_mode = (enum isp_mode)(uintptr_t)match->data;

		if (get_isp_mode_from_str(isp_dev->ss_mode_i0) != isp_dev->isp_mode) {
			dev_warn(&pdev->dev,
				 "xlnx,io_mode='%s' does not match the compatible this node bound with; using the compatible (isp_mode=%u). Fix the DT.\n",
				 isp_dev->ss_mode_i0, isp_dev->isp_mode);
		}
	}

	/*
	 * LILO mixed-mode: per-path live/memory output routing.
	 * "xlnx,path_out_type" is a u32 array indexed by output path
	 * (0=MP, 1=SP1, ...): 1 = live-out (FBWR via xilinx-vipp), 0 =
	 * memory-out (buffers from visp_video). Absent => legacy behavior
	 * (every LILO path is live).
	 */
	if (isp_dev->isp_mode == ISP_MODE_LILO) {
		int n = of_property_count_u32_elems(node, "xlnx,path_out_type");

		if (n > 0) {
			u32 vals[VISP_PORT_PAD_NR - 1] = { 0 };
			int p, c, live = 0;

			if (n > VISP_PORT_PAD_NR - 1)
				n = VISP_PORT_PAD_NR - 1;

			if (of_property_read_u32_array(node, "xlnx,path_out_type",
						       vals, n) == 0) {
				for (p = 0; p < VISP_PORT_NR; p++)
					for (c = 0; c < n; c++)
						isp_dev->output_type[p][c] =
							vals[c] ? VISP_PATH_OUT_TYPE_STREAM :
								  VISP_PATH_OUT_TYPE_MEMORY;

				for (c = 0; c < n; c++)
					if (vals[c])
						live++;
				if (live == 0) {
					dev_warn(&pdev->dev,
						 "xlnx,path_out_type all-memory invalid for LILO; "
						 "forcing path0 live\n");
					for (p = 0; p < VISP_PORT_NR; p++)
						isp_dev->output_type[p][0] =
							VISP_PATH_OUT_TYPE_STREAM;
				}
				ISP_DEV_EXTENDED(isp_dev)->per_path_out_type = true;
				dev_info(&pdev->dev,
					 "mixed-mode: per-path out_type from DT (%d paths)\n",
					 n);
			}
		}
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

	for (port = 0; port < VISP_PORT_NR; port++) {
		strscpy(isp_dev->isp_ports[port].sensor_info.name,
			VISP_DEFAULT_SENSOR,
			sizeof(isp_dev->isp_ports[port].sensor_info.name));

		isp_dev->isp_ports[port].sensor_info.mode =
		    VISP_DEFAULT_SENSOR_MODE;

		isp_dev->isp_ports[port].sensor_info.sensor_id =
		    sensor_dev_id[port];
		/* hw_mcm (multi-camera-mode chaining) is a LIMO-only feature */
		if (isp_dev->isp_mode != ISP_MODE_LILO && isp_dev->num_streams > 1)
			isp_dev->isp_ports[port].hw_mcm = (bool_t)CAMDEV_MCM_OP_HW;
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

	/* LLP (Low Latency Path) is a LIMO-only feature; llp[]/llp_capable[]
	 * are already zero from the extended struct's devm_kzalloc.
	 */
	if (isp_dev->isp_mode != ISP_MODE_LILO) {
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

	if (isp_dev->isp_mode == ISP_MODE_LILO) {
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

	/* Initialise per-(port, path) stream-session counters. */
	for (int port = 0; port < isp_dev->num_streams; port++)
		for (int path = 0; path < 4; path++)
			atomic_set(&ISP_DEV_EXTENDED(isp_dev)->stream_seq[port][path], 0);

	/* Initialize port-level completions for other commands */
	for (int port = 0; port < isp_dev->num_streams; port++) {
		init_completion(&isp_dev->apu_wait_for_cmd_ack[port]);
		mutex_init(&isp_dev->cmd_ack_fifo_lock[port]);
		/* Allocate port-level FIFO (128 entries) */
		if (kfifo_alloc(&isp_dev->cmd_ack_fifo[port], 128, GFP_KERNEL)) {
			dev_err(isp_dev->dev, "Failed to allocate cmd_ack_fifo[%d]\n", port);
			return -ENOMEM;
		}
		/* Per-port data response: completion + private FIFO */
		init_completion(&isp_dev->apu_wait_for_data[port]);
		mutex_init(&isp_dev->data_fifo_lock[port]);
		if (kfifo_alloc(&isp_dev->data_fifo[port], 128, GFP_KERNEL)) {
			dev_err(isp_dev->dev, "Failed to allocate data_fifo[%d]\n", port);
			kfifo_free(&isp_dev->cmd_ack_fifo[port]);
			for (int i = 0; i < port; i++) {
				kfifo_free(&isp_dev->data_fifo[i]);
				kfifo_free(&isp_dev->cmd_ack_fifo[i]);
			}
			return -ENOMEM;
		}
	}

	if (!isp_dev->rpu->tx_chan || !isp_dev->rpu->rx_chan) {
		dev_err(isp_dev->dev, "No TX or RX Channel found on RPU: %d\n",
			isp_dev->isp_rpu);
		return -ENOMEM;
	}

	isp_dev->tx_chan = isp_dev->rpu->tx_chan;
	isp_dev->rx_chan = isp_dev->rpu->rx_chan;
	isp_dev->rpu->isp_dev[isp_dev->id] = isp_dev;
	isp_dev->wdt_expiry_cb = visp_notify_wdt_expiry;
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

/*
 * SSW-18136: quick (hardirq) handler for this ISP instance's FuSa
 * interrupt line. There is no APU-visible register to clear the
 * source, and the line is level-triggered, so mask it here to avoid
 * an interrupt storm and hand off to the threaded handler, which can
 * sleep. The line stays masked until this driver re-probes (i.e.
 * after the target restart the story calls for) rather than being
 * re-armed from within this driver.
 */
static irqreturn_t visp_fusa_irq_quick(int irq, void *dev_id)
{
	struct visp_dev *isp_dev = dev_id;

	disable_irq_nosync(irq);
	dev_crit(isp_dev->dev,
		 "ISP FuSa monitoring disabled for isp_id %d (irq %d) after fault - stays masked until next probe/restart\n",
		 isp_dev->id, irq);
	return IRQ_WAKE_THREAD;
}

/*
 * Threaded handler: detection and logging only for now, per SSW-18136 -
 * it does not parse the interrupt or identify its exact cause. No
 * userspace notification: there is currently no daemon-side handling of
 * a FuSa restart trigger, so this driver does not post an event for
 * one. (An earlier iteration posted a VISP_EVENT_INTERCONNECT_ERR event
 * to the daemon here - dropped since there is nothing on the userspace
 * side to consume it yet.)
 */
static irqreturn_t visp_fusa_irq_thread(int irq, void *dev_id)
{
	struct visp_dev *isp_dev = dev_id;

	dev_alert(isp_dev->dev,
		  "ISP FuSa error detected: isp_id %d, irq %d (fusa_irq)\n",
		  isp_dev->id, irq);

	return IRQ_HANDLED;
}

/*
 * SSW-18136: register for this ISP instance's own FuSa interrupt line,
 * if present. Unlike the isr_irq/xmpu_interrupt lines this driver used
 * to also register (dropped: isr_irq/xmpu_interrupt are owned/enabled
 * by RPU firmware's own hal_irq.c stub handlers on the same physical
 * lines - a real ownership conflict, not ours to claim without
 * firmware-side coordination), fusa_irq is named per ISP instance in
 * DT ("tileN_ispM_fusa_irq", present on both isp0 and isp1 sub-nodes,
 * not just isp0) - so this registers per isp_id, not per tile.
 * platform_get_irq_byname_optional() returns -ENXIO silently if this
 * instance's DT node lacks the property; presence in DT is the sole
 * gate. Applies to both LIMO and LILO instances alike. Failure to
 * register is logged but is not fatal to probe - FuSa-error reporting
 * is best-effort auxiliary functionality, not required for the ISP to
 * operate.
 */
static void visp_register_fusa_irq(struct visp_dev *isp_dev,
				   struct platform_device *pdev)
{
	struct visp_isp_dev_extended *ext = ISP_DEV_EXTENDED(isp_dev);
	struct device *dev = &pdev->dev;
	int tile = isp_dev->id / 2;
	int local_isp = isp_dev->id % 2;
	char *fusa_name;
	int irq, ret;

	ext->fusa_irq = -1;

	fusa_name = devm_kasprintf(dev, GFP_KERNEL, "tile%d_isp%d_fusa_irq",
				   tile, local_isp);
	if (!fusa_name) {
		dev_warn(dev, "failed to allocate fusa irq name\n");
		return;
	}

	irq = platform_get_irq_byname_optional(pdev, fusa_name);
	if (irq >= 0) {
		ret = devm_request_threaded_irq(dev, irq,
						visp_fusa_irq_quick,
						visp_fusa_irq_thread,
						IRQF_ONESHOT, fusa_name, isp_dev);
		if (ret)
			dev_warn(dev, "failed to request %s (irq %d): %d\n",
				 fusa_name, irq, ret);
		else
			ext->fusa_irq = irq;
	} else {
		dev_dbg(dev, "%s not present for isp_id %d (optional)\n",
			fusa_name, isp_dev->id);
	}
}

static int visp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct visp_dev *isp_dev;
	int ret;
	int port = 0;

	isp_dev = kzalloc(sizeof(struct visp_dev), GFP_KERNEL);
	if (!isp_dev)
		return -ENOMEM;
	kref_init(&isp_dev->ref);
	/* Arm the WDT-op kref hook: in-flight mailbox ops now pin isp_dev, so a
	 * proceed-anyway teardown defers the free instead of racing a UAF.
	 */
	isp_dev->wdt_kref_release = visp_dev_release;

	/*
	 * kzalloc'd, not devm: freed by visp_dev_release() alongside isp_dev
	 * itself (see that function) so it shares isp_dev's own kref lifetime
	 * instead of being freed early at unbind while a subdev fd may still
	 * be open.
	 */
	isp_dev->extended_struct =
		kzalloc(sizeof(struct visp_isp_dev_extended), GFP_KERNEL);
	if (!isp_dev->extended_struct) {
		ret = -ENOMEM;
		goto err_free_dev;
	}

	mutex_init(&isp_dev->mlock);
	mutex_init(&isp_dev->wdt_lifetime_lock);
	atomic_set(&isp_dev->wdt_inflight, 0);
	isp_dev->wdt_teardown = false;
	init_completion(&isp_dev->wdt_inflight_zero);
	/* visp_wdt_begin_teardown() wakes this unconditionally; must be valid
	 * even for isp_mode/pipelines that never wait on it.
	 */
	init_waitqueue_head(&isp_dev->wq_frame_done_finished);
	mutex_init(&isp_dev->calib_lock);
	mutex_init(&isp_dev->ctrl_lock);
	mutex_init(&ISP_DEV_EXTENDED(isp_dev)->device_create_lock);
	isp_dev->dev = &pdev->dev;
	platform_set_drvdata(pdev, isp_dev);

	/* Register in the module-wide list for cross-ISP subdev lookup */
	mutex_lock(&visp_dev_global_mutex);
	list_add(&isp_dev->global_entry, &visp_dev_global_list);
	mutex_unlock(&visp_dev_global_mutex);

	ret = visp_parse_params(isp_dev, pdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to parse params\n");
		ret = -EINVAL;
		goto err_del_free;
	}

	if (isp_dev->isp_mode != ISP_MODE_LILO) {
		/* MCM upstream-node discovery is LIMO-only; visp_release_upstream_nodes_dt()
		 * in visp_remove() is safe to call unconditionally against the
		 * resulting empty upstream_nodes[].
		 */
		ret = visp_build_upstream_nodes_dt(isp_dev);
		if (ret && ret != -ENODEV)
			dev_warn(&pdev->dev, "failed to parse upstream DT forward nodes: %d\n", ret);
	}

	ret = xlnx_link_mbox(isp_dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to init mbox\n");
		ret = -EINVAL;
		goto err_del_free;
	}

	for (int i = 0; i < VISP_INPUT_INSTANCES; i++)
		isp_dev->instanceid_port_map[i] = ~0U;

	/* Initialize mutexes for cam_device_bufs arrays (num_streams ports × 4 channels) */
	for (int port = 0; port < isp_dev->num_streams; port++)
		for (int chn = 0; chn < 4; chn++)
			mutex_init(&isp_dev->isp_ports[port]
				       .isp_chns[chn]
				       .cam_device_bufs_lock);

	v4l2_subdev_init(&isp_dev->sd, &visp_subdev_ops);
	snprintf(isp_dev->sd.name, VISP_SUBDEV_NAME_SIZE, "%s.%d",
		 isp_dev->isp_mode == ISP_MODE_LILO ? VISP_NAME_LILO : VISP_NAME,
		 isp_dev->id);

	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	isp_dev->sd.dev = &pdev->dev;
	isp_dev->sd.owner = THIS_MODULE;
	isp_dev->sd.internal_ops = &visp_internal_ops;
	isp_dev->sd.entity.ops = &visp_entity_ops;
	isp_dev->sd.entity.function = isp_dev->isp_mode == ISP_MODE_LILO ?
		MEDIA_ENT_F_V4L2_SUBDEV_UNKNOWN : MEDIA_ENT_F_PROC_VIDEO_ISP;
	isp_dev->sd.entity.obj_type = MEDIA_ENTITY_TYPE_V4L2_SUBDEV;
	isp_dev->sd.entity.name = isp_dev->sd.name;
	v4l2_set_subdevdata(&isp_dev->sd, isp_dev);

	visp_pads_init(isp_dev);
	ret = media_entity_pads_init(&isp_dev->sd.entity, isp_dev->num_pads,
				     isp_dev->pads);
	if (ret)
		goto err_pads_init;

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
				wq_name, WQ_UNBOUND | WQ_HIGHPRI,
				isp_dev->isp_mode == ISP_MODE_LILO ?
					ENQ_WQ_MAX_ACTIVE_LILO : ENQ_WQ_MAX_ACTIVE_LIMO);
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

	if (isp_dev->isp_mode == ISP_MODE_LILO) {
		/*
		 * Plain LILO's OBA/live-out path has no APU-managed vb2 buffer
		 * completion to notify (v-frmbuf-wr's own IRQ/vb2 completion
		 * handles it), so it stays frameout_cb = NULL as before. A
		 * LILO mixed-mode instance (per_path_out_type set) has at
		 * least one memory-out chn whose vb2 buffer completion IS
		 * driven by the RPU's DISPLAY_BUFFER notification (mbox_cmd.c),
		 * same as LIMO - without the callback registered there, that
		 * notification hits mbox_cmd.c's "CALLBACK IS NULL" path and
		 * the completed frame is dropped.
		 * yuv_420_format_index[] must be resolved after visp_pads_init()
		 * has populated pad_data[].fmts[], which it has by this point.
		 */
		isp_dev->frameout_cb = ISP_DEV_EXTENDED(isp_dev)->per_path_out_type ?
					handle_frameout_buffer : NULL;
		get_yuv_420_format_index(isp_dev, CAMDEV_BUFCHAIN_MP);
		get_yuv_420_format_index(isp_dev, CAMDEV_BUFCHAIN_SP1);
	} else {
		/* Register Callback function */
		isp_dev->frameout_cb = handle_frameout_buffer;
	}
	/* sensor_pipeline_init(isp_dev); */

	/*
	 * Probe fully succeeded: take the subdev-node reference and arm the
	 * release hook so isp_dev (embedding ->sd) outlives the last
	 * /dev/v4l-subdevN close, which drops this ref via visp_sd_release().
	 */
	kref_get(&isp_dev->ref);
	isp_dev->sd_release_armed = true;

	visp_register_fusa_irq(isp_dev, pdev);

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

err_pads_init:
	for (port = 0; port < isp_dev->num_streams; port++) {
		kfifo_free(&isp_dev->cmd_ack_fifo[port]);
		kfifo_free(&isp_dev->data_fifo[port]);
	}
err_del_free:
	mutex_lock(&visp_dev_global_mutex);
	list_del(&isp_dev->global_entry);
	mutex_unlock(&visp_dev_global_mutex);
err_free_dev:
	kref_put(&isp_dev->ref, visp_dev_release);
	return ret;
}

static void visp_remove(struct platform_device *pdev)
{
	struct visp_dev *isp_dev;
	struct visp_isp_dev_extended *ext;

	isp_dev = platform_get_drvdata(pdev);
	ext = ISP_DEV_EXTENDED(isp_dev);

	/*
	 * Free the FuSa IRQ explicitly rather than relying on
	 * devm_request_threaded_irq()'s automatic cleanup, which would only
	 * run after this .remove() callback returns.
	 */
	if (ext->fusa_irq >= 0)
		devm_free_irq(&pdev->dev, ext->fusa_irq, isp_dev);

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
	/*
	 * isp_dev embeds ->sd, so it must outlive the last /dev/v4l-subdevN
	 * close. It is kzalloc'd + kref'd (not devm): the driver ref is dropped
	 * at the end of this function and the subdev-node ref is dropped by
	 * visp_sd_release() on last close, whichever is later frees isp_dev.
	 * ->pads/->pad_data/->extended_struct are kzalloc'd (not devm) too and
	 * share that same kref lifetime (freed together in visp_dev_release()),
	 * so a still-open subdev fd can safely keep issuing ioctls that touch
	 * them right up until the last reference actually drops - this no
	 * longer depends on the HAL closing its fd by any particular deadline
	 * relative to this unbind.
	 */
	media_entity_cleanup(&isp_dev->sd.entity);

	/* Free DMA coherent memory for event_shm */
	if (isp_dev->event_shm.virt_addr) {
		/*
		 * dma_buf_fd() (in the ioctl path) installs dmabuf->file directly
		 * into the caller's fd table without taking an extra reference -
		 * that single export reference now belongs to whichever process
		 * has the fd open, not to this driver. Calling dma_buf_put() here
		 * would drop that reference out from under the still-open fd,
		 * leaving its fdtable entry pointing at an already-zero-refcount
		 * file; the process later crashes with "VFS: Close: file count is
		 * 0" / "imbalanced put on file reference count" when it exits and
		 * the kernel tries to close that fd itself.
		 *
		 * Just detach: null dmabuf->priv so visp_dmabuf_release() (which
		 * fires whenever the real owner eventually closes the fd) safely
		 * no-ops instead of touching isp_dev after it's gone.
		 */
		if (isp_dev->event_shm.dmabuf) {
			dev_warn(isp_dev->event_shm.dev,
				 "Dmabuf still active during remove - detaching\n");
			isp_dev->event_shm.dmabuf->priv = NULL;
			isp_dev->event_shm.dmabuf = NULL;
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

	for (int port = 0; port < isp_dev->num_streams; port++) {
		kfifo_free(&isp_dev->cmd_ack_fifo[port]);
		kfifo_free(&isp_dev->data_fifo[port]);
	}

	dev_info(&pdev->dev, "visp isp driver remove\n");

	/* Drop the driver ref; frees isp_dev unless a subdev fd is still open. */
	kref_put(&isp_dev->ref, visp_dev_release);
}

static const struct dev_pm_ops visp_pm_ops = {};

static const struct of_device_id visp_of_match[] = {
	{
		.compatible = "xlnx,visp-ss-limo-1.0",
		.data = (const void *)ISP_MODE_LIMO,
	},
	{
		.compatible = "xlnx,visp-ss-lilo-1.0",
		.data = (const void *)ISP_MODE_LILO,
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
