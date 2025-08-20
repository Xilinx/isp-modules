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

#include "visp_video_register.h"

#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of_graph.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/videobuf2-dma-contig.h>

#include "visp_v4l2_common.h"
#include "visp_v4l2_std_exts.h"

static struct visp_video_fmt_info visp_formats_info[] = {
	{
		.fourcc = V4L2_PIX_FMT_NV16,
		.mbus = MEDIA_BUS_FMT_YUYV8_2X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV12,
		.mbus = MEDIA_BUS_FMT_YUYV8_1_5X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_YUYV,
		.mbus = MEDIA_BUS_FMT_YUYV8_1X16,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR8,
		.mbus = MEDIA_BUS_FMT_SBGGR8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG8,
		.mbus = MEDIA_BUS_FMT_SGBRG8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG8,
		.mbus = MEDIA_BUS_FMT_SGRBG8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB8,
		.mbus = MEDIA_BUS_FMT_SRGGB8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR10,
		.mbus = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG10,
		.mbus = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG10,
		.mbus = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB10,
		.mbus = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR12,
		.mbus = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG12,
		.mbus = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG12,
		.mbus = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB12,
		.mbus = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_P010,
		.mbus = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_GREY,
		.mbus = MEDIA_BUS_FMT_Y8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_Y10BPACK,
		.mbus = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_Y10DWA,
		.mbus = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_Y10,
		.mbus = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_P00BPACK,
		.mbus = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_P00DWA,
		.mbus = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_P02BPACK,
		.mbus = MEDIA_BUS_FMT_YUYV12_2X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_P20BPACK,
		.mbus = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_P20DWA,
		.mbus = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_P210,
		.mbus = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_P22BPACK,
		.mbus = MEDIA_BUS_FMT_YUYV12_2X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_I20BPACK,
		.mbus = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_I210,
		.mbus = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_M48BPACK,
		.mbus = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_I48BPACK,
		.mbus = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_I48DWA,
		.mbus = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_I40DWA,
		.mbus = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB24,
		.mbus = MEDIA_BUS_FMT_RGB888_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB24DWA,
		.mbus = MEDIA_BUS_FMT_RGB888_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB24P,
		.mbus = MEDIA_BUS_FMT_RGB888_3X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR10BPACK,
		.mbus = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG10BPACK,
		.mbus = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG10BPACK,
		.mbus = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB10BPACK,
		.mbus = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR10DWA,
		.mbus = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG10DWA,
		.mbus = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG10DWA,
		.mbus = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB10DWA,
		.mbus = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR12BPACK,
		.mbus = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG12BPACK,
		.mbus = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG12BPACK,
		.mbus = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB12BPACK,
		.mbus = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR12DWA,
		.mbus = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG12DWA,
		.mbus = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG12DWA,
		.mbus = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB12DWA,
		.mbus = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR14BPACK,
		.mbus = MEDIA_BUS_FMT_SBGGR14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG14BPACK,
		.mbus = MEDIA_BUS_FMT_SGBRG14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG14BPACK,
		.mbus = MEDIA_BUS_FMT_SGRBG14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB14BPACK,
		.mbus = MEDIA_BUS_FMT_SRGGB14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR14DWA,
		.mbus = MEDIA_BUS_FMT_SBGGR14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG14DWA,
		.mbus = MEDIA_BUS_FMT_SGBRG14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG14DWA,
		.mbus = MEDIA_BUS_FMT_SGRBG14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB14DWA,
		.mbus = MEDIA_BUS_FMT_SRGGB14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR14,
		.mbus = MEDIA_BUS_FMT_SBGGR14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG14,
		.mbus = MEDIA_BUS_FMT_SGBRG14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG14,
		.mbus = MEDIA_BUS_FMT_SGRBG14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB14,
		.mbus = MEDIA_BUS_FMT_SRGGB14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR16,
		.mbus = MEDIA_BUS_FMT_SBGGR16_1X16,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG16,
		.mbus = MEDIA_BUS_FMT_SGBRG16_1X16,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG16,
		.mbus = MEDIA_BUS_FMT_SGRBG16_1X16,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB16,
		.mbus = MEDIA_BUS_FMT_SRGGB16_1X16,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR24,
		.mbus = MEDIA_BUS_FMT_SBGGR24_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG24,
		.mbus = MEDIA_BUS_FMT_SGBRG24_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG24,
		.mbus = MEDIA_BUS_FMT_SGRBG24_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB24,
		.mbus = MEDIA_BUS_FMT_SRGGB24_1X24,
	},
};

static int visp_video_mbus_to_fourcc(uint32_t mbus, uint32_t *fourcc,
				     uint32_t fmt)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(visp_formats_info); i++) {
		if ((visp_formats_info[i].mbus == mbus) &&
		    (visp_formats_info[i].fourcc == fmt)) {
			*fourcc = visp_formats_info[i].fourcc;
			return 0;
		}
	}

	return -EINVAL;
}

static int visp_video_fourcc_to_mbus(uint32_t fourcc, uint32_t *mbus)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(visp_formats_info); i++) {
		if (visp_formats_info[i].fourcc == fourcc) {
			*mbus = visp_formats_info[i].mbus;
			return 0;
		}
	}

	return -EINVAL;
}

static int visp_video_vfmt_to_mfmt(struct v4l2_format *f,
				   struct v4l2_subdev_format *mfmt)
{
	int ret;

	mfmt->format.width = f->fmt.pix.width;
	mfmt->format.height = f->fmt.pix.height;
	mfmt->format.field = f->fmt.pix.field;
	mfmt->format.colorspace = f->fmt.pix.colorspace;
	mfmt->format.quantization = f->fmt.pix.quantization;

	ret = visp_video_fourcc_to_mbus(f->fmt.pix.pixelformat,
					&mfmt->format.code);
	memcpy(mfmt->format.reserved, &f->fmt.pix.pixelformat,
	       sizeof(f->fmt.pix.pixelformat));

	return ret;
}

static const struct v4l2_format_info *visp_video_vfmt_info(u32 format)
{
	/* format info for user define or supported by later versions */
	static const struct v4l2_format_info formats[] = {
	{
		.format = V4L2_PIX_FMT_GREY,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {1, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {1, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_Y10BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {5, 0, 0, 0},
		.bpp_div = {4, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_Y10DWA,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 0, 0, 0},
		.bpp_div = {3, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_Y10,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	    // 420
	{
		.format = V4L2_PIX_FMT_P00BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 2,
		.hdiv = 2,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {5, 10, 0, 0},
		.bpp_div = {4, 4, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_P00DWA,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 2,
		.hdiv = 2,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 8, 0, 0},
		.bpp_div = {3, 3, 1, 1},
#else
		.bpp = {2, 2, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_P010,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 2,
		.hdiv = 1,
		.vdiv = 2,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 2, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 2, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_P02BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 2,
		.hdiv = 2,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {3, 6, 0, 0},
		.bpp_div = {2, 2, 1, 1},
#else
		.bpp = {2, 2, 0, 0},
#endif
	},
	    // 422
	{
		.format = V4L2_PIX_FMT_P20BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 2,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {5, 10, 0, 0},
		.bpp_div = {4, 4, 1, 1},
#else
		.bpp = {2, 2, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_P20DWA,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 2,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 8, 0, 0},
		.bpp_div = {3, 3, 1, 1},
#else
		.bpp = {2, 2, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_P210,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 2,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 4, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 2, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_P22BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 2,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {3, 6, 0, 0},
		.bpp_div = {2, 2, 1, 1},
#else
		.bpp = {2, 2, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_I20BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {15, 0, 0, 0},
		.bpp_div = {4, 1, 1, 1},
#else
		.bpp = {4, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_I210,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {4, 0, 0, 0},
#endif
	},
	    // 444
	{
		.format = V4L2_PIX_FMT_M48BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 3,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {1, 1, 1, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {1, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_I48BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {3, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {3, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_I48DWA,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {4, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_I40DWA,
		.pixel_enc = V4L2_PIXEL_ENC_YUV,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {4, 0, 0, 0},
#endif
	},
	    // rgb
	{
		.format = V4L2_PIX_FMT_RGB24,
		.pixel_enc = V4L2_PIXEL_ENC_RGB,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {3, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {3, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_RGB24DWA,
		.pixel_enc = V4L2_PIXEL_ENC_RGB,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {4, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_RGB24P,
		.pixel_enc = V4L2_PIXEL_ENC_RGB,
		.mem_planes = 1,
		.comp_planes = 3,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {1, 1, 1, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {1, 1, 1, 0},
#endif
	},
	    // RAW
	{
		.format = V4L2_PIX_FMT_SBGGR10BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {5, 0, 0, 0},
		.bpp_div = {4, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGBRG10BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {5, 0, 0, 0},
		.bpp_div = {4, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGRBG10BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {5, 0, 0, 0},
		.bpp_div = {4, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SRGGB10BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {5, 0, 0, 0},
		.bpp_div = {4, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SBGGR10DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 0, 0, 0},
		.bpp_div = {3, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGBRG10DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 0, 0, 0},
		.bpp_div = {3, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGRBG10DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 0, 0, 0},
		.bpp_div = {3, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SRGGB10DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {4, 0, 0, 0},
		.bpp_div = {3, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SBGGR12BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {3, 0, 0, 0},
		.bpp_div = {2, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGBRG12BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {3, 0, 0, 0},
		.bpp_div = {2, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGRBG12BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {3, 0, 0, 0},
		.bpp_div = {2, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SRGGB12BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {3, 0, 0, 0},
		.bpp_div = {2, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SBGGR12DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {8, 0, 0, 0},
		.bpp_div = {5, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGBRG12DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {8, 0, 0, 0},
		.bpp_div = {5, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGRBG12DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {8, 0, 0, 0},
		.bpp_div = {5, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SRGGB12DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {8, 0, 0, 0},
		.bpp_div = {5, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SBGGR14BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {7, 0, 0, 0},
		.bpp_div = {4, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGBRG14BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {7, 0, 0, 0},
		.bpp_div = {4, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGRBG14BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {7, 0, 0, 0},
		.bpp_div = {4, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SRGGB14BPACK,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {7, 0, 0, 0},
		.bpp_div = {4, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SBGGR14DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {16, 0, 0, 0},
		.bpp_div = {9, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGBRG14DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {16, 0, 0, 0},
		.bpp_div = {9, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGRBG14DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {16, 0, 0, 0},
		.bpp_div = {9, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SRGGB14DWA,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {16, 0, 0, 0},
		.bpp_div = {9, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SBGGR14,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGBRG14,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGRBG14,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SRGGB14,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SBGGR16,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGBRG16,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGRBG16,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SRGGB16,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {2, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SBGGR24,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {3, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGBRG24,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {3, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SGRBG24,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {3, 0, 0, 0},
#endif
	},
	{
		.format = V4L2_PIX_FMT_SRGGB24,
		.pixel_enc = V4L2_PIXEL_ENC_BAYER,
		.mem_planes = 1,
		.comp_planes = 1,
		.hdiv = 1,
		.vdiv = 1,
#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
		.bpp = {2, 0, 0, 0},
		.bpp_div = {1, 1, 1, 1},
#else
		.bpp = {3, 0, 0, 0},
#endif
	},
};

	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		if (formats[i].format == format)
			return &formats[i];
	}

	return NULL;
}

static int visp_video_mfmt_to_vfmt(struct v4l2_subdev_format *mfmt,
				   struct v4l2_format *f)
{
	int ret;
	const struct v4l2_format_info *info;
	uint32_t bytesperline;
	uint32_t sizeimage = 0;
	uint32_t width;
	uint32_t height;
	int i;
	int private_fmt = 0;
	uint32_t fourcc_code = 0;

	memcpy(&fourcc_code, mfmt->format.reserved, sizeof(fourcc_code));

	f->fmt.pix.width = mfmt->format.width;
	f->fmt.pix.height = mfmt->format.height;
	f->fmt.pix.field = mfmt->format.field;
	f->fmt.pix.colorspace = mfmt->format.colorspace;
	f->fmt.pix.quantization = mfmt->format.quantization;
	ret = visp_video_mbus_to_fourcc(mfmt->format.code,
					&f->fmt.pix.pixelformat, fourcc_code);
	if (ret)
		return ret;

	width = f->fmt.pix.width;
	height = f->fmt.pix.height;
	info = v4l2_format_info(f->fmt.pix.pixelformat);
	if (info != NULL) {
		bytesperline = info->bpp[0] * width;
	} else {
		private_fmt = 1;
		info = visp_video_vfmt_info(f->fmt.pix.pixelformat);
		if (info == NULL)
			return -EINVAL;

		// calculate size information here for private format
		switch (info->format) {
		/* YUV format */
		// 8bit unalign
		case V4L2_PIX_FMT_GREY:
		case V4L2_PIX_FMT_M48BPACK:
		case V4L2_PIX_FMT_I48BPACK:
			bytesperline = width;
			if (info->format == V4L2_PIX_FMT_I48BPACK)
				bytesperline *= 3;
			break;

		// 8bit dwa
		case V4L2_PIX_FMT_I48DWA:
		case V4L2_PIX_FMT_I40DWA:
			bytesperline = ((width * 4) / 3) * 3;
			break;

		// 10bit unalign
		case V4L2_PIX_FMT_Y10BPACK:
		case V4L2_PIX_FMT_P00BPACK:
		case V4L2_PIX_FMT_P20BPACK:
		case V4L2_PIX_FMT_I20BPACK:
			bytesperline = DIV_ROUND_UP(width * 10, 128) * 16;
			if (info->format == V4L2_PIX_FMT_I20BPACK)
				bytesperline *= 4;
			break;

		// 10 bit double align
		case V4L2_PIX_FMT_Y10DWA:
		case V4L2_PIX_FMT_P00DWA:
		case V4L2_PIX_FMT_P20DWA:
			bytesperline = width * 4 / 3;
			break;

		// 10 bit mode 1, per pixel 16bits
		case V4L2_PIX_FMT_Y10:
		case V4L2_PIX_FMT_P010:
		case V4L2_PIX_FMT_P210:
		case V4L2_PIX_FMT_I210:
			bytesperline = DIV_ROUND_UP(width, 8) * 16;
			if (info->format == V4L2_PIX_FMT_I210)
				bytesperline *= 2;
			break;

		// 12bit unalign
		case V4L2_PIX_FMT_P02BPACK:
		case V4L2_PIX_FMT_P22BPACK:
			bytesperline = DIV_ROUND_UP(width * 12, 128) * 16;
			break;

		// rgb
		case V4L2_PIX_FMT_RGB24:
			bytesperline = width * 3;
			break;

		case V4L2_PIX_FMT_RGB24P:
			bytesperline = width;
			break;

		case V4L2_PIX_FMT_RGB24DWA:
			bytesperline = (width * 4 / 3) * 3;
			break;

		// RAW
		// 10 bit-unalign
		case V4L2_PIX_FMT_SBGGR10BPACK:
		case V4L2_PIX_FMT_SGBRG10BPACK:
		case V4L2_PIX_FMT_SGRBG10BPACK:
		case V4L2_PIX_FMT_SRGGB10BPACK:
			bytesperline = DIV_ROUND_UP(width * 10, 128) * 16;
			break;

		case V4L2_PIX_FMT_SBGGR10DWA:
		case V4L2_PIX_FMT_SGBRG10DWA:
		case V4L2_PIX_FMT_SGRBG10DWA:
		case V4L2_PIX_FMT_SRGGB10DWA:
			bytesperline = DIV_ROUND_UP(width, 12) * 16;
			break;

		// 12bit
		case V4L2_PIX_FMT_SBGGR12BPACK:
		case V4L2_PIX_FMT_SGBRG12BPACK:
		case V4L2_PIX_FMT_SGRBG12BPACK:
		case V4L2_PIX_FMT_SRGGB12BPACK:
			bytesperline = DIV_ROUND_UP(width * 12, 128) * 16;
			break;

		case V4L2_PIX_FMT_SBGGR12DWA:
		case V4L2_PIX_FMT_SGBRG12DWA:
		case V4L2_PIX_FMT_SGRBG12DWA:
		case V4L2_PIX_FMT_SRGGB12DWA:
			bytesperline = DIV_ROUND_UP(width, 10) * 16;
			break;

		// 14 bit
		case V4L2_PIX_FMT_SBGGR14BPACK:
		case V4L2_PIX_FMT_SGBRG14BPACK:
		case V4L2_PIX_FMT_SGRBG14BPACK:
		case V4L2_PIX_FMT_SRGGB14BPACK:
			bytesperline = DIV_ROUND_UP(width * 14, 128) * 16;
			break;

		case V4L2_PIX_FMT_SBGGR14DWA:
		case V4L2_PIX_FMT_SGBRG14DWA:
		case V4L2_PIX_FMT_SGRBG14DWA:
		case V4L2_PIX_FMT_SRGGB14DWA:
			bytesperline = DIV_ROUND_UP(width, 9) * 16;
			break;

		case V4L2_PIX_FMT_SBGGR14:
		case V4L2_PIX_FMT_SGBRG14:
		case V4L2_PIX_FMT_SGRBG14:
		case V4L2_PIX_FMT_SRGGB14:
			bytesperline = DIV_ROUND_UP(width, 8) * 16;
			break;

		case V4L2_PIX_FMT_SBGGR16:
		case V4L2_PIX_FMT_SGBRG16:
		case V4L2_PIX_FMT_SGRBG16:
		case V4L2_PIX_FMT_SRGGB16:
			bytesperline = DIV_ROUND_UP(width * 16, 128) * 16;
			break;

		case V4L2_PIX_FMT_SBGGR24:
		case V4L2_PIX_FMT_SGBRG24:
		case V4L2_PIX_FMT_SGRBG24:
		case V4L2_PIX_FMT_SRGGB24:
			bytesperline = DIV_ROUND_UP(width * 24, 128) * 16;
			break;

		default:
			break;
		}
	}

	sizeimage = bytesperline * height;

	if (info->comp_planes == 1) {
		f->fmt.pix.bytesperline = bytesperline;
		f->fmt.pix.sizeimage = sizeimage;
		return 0;
	}

	f->fmt.pix.bytesperline = bytesperline;
	f->fmt.pix.sizeimage = sizeimage;

	for (i = 1; i < info->comp_planes; i++) {
		if (private_fmt) {
			bytesperline = DIV_ROUND_UP(bytesperline, info->hdiv);
		} else {
			bytesperline =
			    info->bpp[i] * DIV_ROUND_UP(width, info->hdiv);
		}
		sizeimage = bytesperline * DIV_ROUND_UP(height, info->vdiv);
		f->fmt.pix.sizeimage += sizeimage;
	}

	return 0;
}

static struct v4l2_subdev *
visp_video_remote_subdev(struct visp_video_dev *visp_vdev)
{
	struct media_pad *pad;
	struct v4l2_subdev *subdev;

#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
	pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
	pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
	if (!pad || !is_media_entity_v4l2_subdev(pad->entity))
		return NULL;

	subdev = media_entity_to_v4l2_subdev(pad->entity);

	return subdev;
}

static int visp_video_try_create_pipeline(struct visp_video_dev *visp_vdev)
{
	int ret;
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct v4l2_subdev_format sd_fmt;
	struct v4l2_subdev_pad_config pad_cfg;
	struct v4l2_subdev_state sd_state = {
	    .pads = &pad_cfg,
	};

	if (visp_vdev->pipeline)
		return 0;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (!subdev) {
		dev_err(visp_vdev->visp_mdev->dev, "try_create_pipeline: no remote subdev found\n");
		return -EINVAL;
	}

#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
	pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
	pad = media_entity_remote_pad(&visp_vdev->pad);
#endif

	if (!pad) {
		dev_err(visp_vdev->visp_mdev->dev, "try_create_pipeline: no remote pad found\n");
		return -EINVAL;
	}

	sd_fmt.pad = pad->index;
	sd_fmt.which = V4L2_SUBDEV_FORMAT_TRY;

	ret = v4l2_subdev_call(subdev, pad, get_fmt, &sd_state, &sd_fmt);
	if (ret) {
		dev_err(visp_vdev->visp_mdev->dev, "try_create_pipeline: get_fmt failed: %d\n", ret);
		return ret;
	}

	ret = visp_video_mfmt_to_vfmt(&sd_fmt, &visp_vdev->format);
	if (ret) {
		dev_err(visp_vdev->visp_mdev->dev, "try_create_pipeline: mfmt_to_vfmt failed: %d\n", ret);
		return ret;
	}

	visp_vdev->pipeline = 1;
	return 0;
}

static int visp_video_destroy_pipeline(struct visp_video_dev *visp_vdev)
{
	visp_vdev->pipeline = 0;
	return 0;
}

static int visp_videoc_querycap(struct file *file, void *priv,
				struct v4l2_capability *cap)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);

	strncpy(cap->driver, visp_vdev->video->name, sizeof(cap->driver));
	cap->driver[sizeof(cap->driver) - 1] = '\0';
	strncpy(cap->card, visp_vdev->video->name, sizeof(cap->card));
	cap->card[sizeof(cap->card) - 1] = '\0';
	snprintf(cap->bus_info, sizeof(cap->bus_info), "platform:%s",
		 visp_vdev->video->name);

	return 0;
}

static int visp_videoc_enum_fmt_vid_cap(struct file *file, void *priv,
					struct v4l2_fmtdesc *f)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct v4l2_subdev_mbus_code_enum mbus_code;
	struct v4l2_subdev_pad_config pad_cfg;
	struct v4l2_subdev_state sd_state = {
	    .pads = &pad_cfg,
	};
	int ret = -EINVAL;

	ret = visp_video_try_create_pipeline(visp_vdev);
	if (ret) {
		dev_err(visp_vdev->visp_mdev->dev, "enum_fmt: visp_video_try_create_pipeline failed: %d\n", ret);
		return ret;
	}

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		if (!pad) {
			dev_err(visp_vdev->visp_mdev->dev, "enum_fmt: no remote pad found\n");
			return -EINVAL;
		}

		memset(&mbus_code, 0, sizeof(mbus_code));
		mbus_code.pad = pad->index;
		mbus_code.index = f->index;
		ret = v4l2_subdev_call(subdev, pad, enum_mbus_code, &sd_state,
				       &mbus_code);
		if (ret) {
			dev_err(visp_vdev->visp_mdev->dev, "enum_fmt: enum_mbus_code failed: %d\n", ret);
			return ret;
		}

		ret = visp_video_mbus_to_fourcc(mbus_code.code, &f->pixelformat,
						(uint32_t)mbus_code.reserved[0]);

		if (ret)
			return ret;
		memcpy(f->description, &f->pixelformat, sizeof(f->pixelformat));
	}

	return ret;
}

static int visp_videoc_try_fmt_vid_cap(struct file *file, void *priv,
				       struct v4l2_format *f)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct v4l2_subdev_format sd_fmt;
	struct v4l2_subdev_pad_config pad_cfg;
	struct v4l2_subdev_state sd_state = {
	    .pads = &pad_cfg,
	};
	int ret;

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	ret = visp_video_try_create_pipeline(visp_vdev);
	if (ret)
		return ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (!subdev)
		return -EINVAL;
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
	pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
	pad = media_entity_remote_pad(&visp_vdev->pad);
#endif

	sd_fmt.pad = pad->index;
	sd_fmt.which = V4L2_SUBDEV_FORMAT_TRY;

	visp_video_vfmt_to_mfmt(f, &sd_fmt);
	ret = v4l2_subdev_call(subdev, pad, set_fmt, &sd_state, &sd_fmt);
	if (ret)
		return ret;

	ret = visp_video_mfmt_to_vfmt(&sd_fmt, f);

	return ret;
}

static int visp_videoc_s_fmt_vid_cap(struct file *file, void *priv,
				     struct v4l2_format *f)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct vb2_queue *queue = &visp_vdev->queue;
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct v4l2_subdev_format sd_fmt;
	struct v4l2_subdev_pad_config pad_cfg;
	struct v4l2_subdev_state sd_state = {
	    .pads = &pad_cfg,
	};
	int ret;

	if (vb2_is_busy(queue))
		return -EBUSY;
	ret = visp_videoc_try_fmt_vid_cap(file, priv, f);
	if (ret)
		return ret;
	subdev = visp_video_remote_subdev(visp_vdev);
	if (!subdev)
		return -EINVAL;
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
	pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
	pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
	sd_fmt.pad = pad->index;
	sd_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;

	visp_video_vfmt_to_mfmt(f, &sd_fmt);

	ret = v4l2_subdev_call(subdev, pad, set_fmt, &sd_state, &sd_fmt);
	if (ret)
		return ret;

	visp_vdev->format = *f;

	//   printk("%d x %d size %d fmt %s\n",
	//   f->fmt.pix.width,f->fmt.pix.height,f->fmt.pix.sizeimage, (char
	//   *)&f->fmt.pix.pixelformat);

	return 0;
}

static int visp_videoc_g_fmt_vid_cap(struct file *file, void *fh,
				     struct v4l2_format *f)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);

	int ret;

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	ret = visp_video_try_create_pipeline(visp_vdev);
	if (ret)
		return ret;

	*f = visp_vdev->format;
	return 0;
}

static int visp_videoc_reqbufs(struct file *file, void *priv,
			       struct v4l2_requestbuffers *p)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_reqbufs pad_requbufs;

	int ret;

	ret = visp_video_try_create_pipeline(visp_vdev);
	if (ret)
		return ret;
	ret = vb2_ioctl_reqbufs(file, priv, p);
	if (ret)
		return ret;
	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		pad_requbufs.pad = pad->index;
		pad_requbufs.num_buffers = p->count;
		v4l2_subdev_call(subdev, core, ioctl, VISP_PAD_REQUBUFS,
				 &pad_requbufs);
	}

	return ret;
}

static int visp_videoc_enum_input(struct file *file, void *fh,
				  struct v4l2_input *input)
{
	if (input->index > 0)
		return -EINVAL;

	strscpy(input->name, "camera", sizeof(input->name));
	input->type = V4L2_INPUT_TYPE_CAMERA;

	return 0;
}

static int visp_videoc_g_input(struct file *file, void *fh,
			       unsigned int *input)
{
	*input = 0;
	return 0;
}

static int visp_videoc_s_input(struct file *file, void *fh,
			       unsigned int input)
{
	return input == 0 ? 0 : -EINVAL;
}

static int visp_videoc_s_parm(struct file *file, void *fh,
			      struct v4l2_streamparm *a)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct v4l2_subdev_frame_interval sd_fi;
	struct v4l2_subdev_pad_config pad_cfg;
	struct v4l2_subdev_state sd_state = {
	    .pads = &pad_cfg,
	};
	int ret = -EINVAL;

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return ret;

	ret = visp_video_try_create_pipeline(visp_vdev);
	if (ret)
		return ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (!subdev)
		return -EINVAL;
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
	pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
	pad = media_entity_remote_pad(&visp_vdev->pad);
#endif

	memset(&sd_fi, 0, sizeof(sd_fi));
	sd_fi.pad = pad->index;
#if KERNEL_VERSION(6, 8, 0) <= LINUX_VERSION_CODE
	sd_fi.which = V4L2_SUBDEV_FORMAT_ACTIVE;
#endif
	sd_fi.interval = a->parm.capture.timeperframe;

	ret = v4l2_subdev_call(subdev, /*video*/ pad, set_frame_interval,
			       &sd_state, &sd_fi);

	a->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	a->parm.capture.timeperframe = sd_fi.interval;

	return ret;
}

static int visp_videoc_g_parm(struct file *file, void *fh,
			      struct v4l2_streamparm *a)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct v4l2_subdev_frame_interval sd_fi;
	struct v4l2_subdev_pad_config pad_cfg;
	struct v4l2_subdev_state sd_state = {
	    .pads = &pad_cfg,
	};
	int ret = -EINVAL;

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return ret;

	ret = visp_video_try_create_pipeline(visp_vdev);
	if (ret)
		return ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (!subdev)
		return -EINVAL;
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
	pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
	pad = media_entity_remote_pad(&visp_vdev->pad);
#endif

	memset(&sd_fi, 0, sizeof(sd_fi));
	sd_fi.pad = pad->index;
#if KERNEL_VERSION(6, 8, 0) <= LINUX_VERSION_CODE
	sd_fi.which = V4L2_SUBDEV_FORMAT_TRY;
#endif

	ret = v4l2_subdev_call(subdev, /*video*/ pad, get_frame_interval,
			       &sd_state, &sd_fi);

	a->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	a->parm.capture.timeperframe = sd_fi.interval;

	return ret;
}

static int visp_videoc_queryctrl(struct file *file, void *fh,
				 struct v4l2_queryctrl *a)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_queryctrl pad_query_ctrl;
	int ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		pad_query_ctrl.pad = pad->index;
		pad_query_ctrl.query_ctrl = a;
		ret = v4l2_subdev_call(subdev, core, ioctl, VISP_PAD_QUERYCTRL,
				       &pad_query_ctrl);
	} else {
		return -ENOTTY;
	}

	return ret;
}

static int visp_videoc_query_ext_ctrl(struct file *file, void *fh,
				      struct v4l2_query_ext_ctrl *a)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_query_ext_ctrl pad_query_ext_ctrl;
	int ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		pad_query_ext_ctrl.pad = pad->index;
		pad_query_ext_ctrl.query_ext_ctrl = a;
		ret = v4l2_subdev_call(subdev, core, ioctl,
				       VISP_PAD_QUERY_EXT_CTRL,
				       &pad_query_ext_ctrl);
	} else {
		return -ENOTTY;
	}

	return ret;
}

static int visp_vidioc_g_ctrl(struct file *file, void *fh,
			      struct v4l2_control *a)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_control pad_control;
	int ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		pad_control.pad = pad->index;
		pad_control.control = a;
		ret = v4l2_subdev_call(subdev, core, ioctl, VISP_PAD_G_CTRL,
				       &pad_control);
	} else {
		return -ENOTTY;
	}

	return ret;
}

static int visp_vidioc_s_ctrl(struct file *file, void *fh,
			      struct v4l2_control *a)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_control pad_control;
	int ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		pad_control.pad = pad->index;
		pad_control.control = a;
		ret = v4l2_subdev_call(subdev, core, ioctl, VISP_PAD_S_CTRL,
				       &pad_control);
	} else {
		return -ENOTTY;
	}

	return ret;
}

static int visp_vidioc_g_ext_ctrls(struct file *file, void *fh,
				   struct v4l2_ext_controls *a)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_ext_controls pad_ext_controls;
	int ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		pad_ext_controls.pad = pad->index;
		pad_ext_controls.ext_controls = a;
		ret = v4l2_subdev_call(subdev, core, ioctl,
				       VISP_PAD_G_EXT_CTRLS, &pad_ext_controls);
	} else {
		return -ENOTTY;
	}

	return ret;
}

static int visp_vidioc_s_ext_ctrls(struct file *file, void *fh,
				   struct v4l2_ext_controls *a)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_ext_controls pad_ext_controls;
	int ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		pad_ext_controls.pad = pad->index;
		pad_ext_controls.ext_controls = a;
		ret = v4l2_subdev_call(subdev, core, ioctl,
				       VISP_PAD_S_EXT_CTRLS, &pad_ext_controls);
	} else {
		return -ENOTTY;
	}

	return ret;
}

static int visp_vidioc_try_ext_ctrls(struct file *file, void *fh,
				     struct v4l2_ext_controls *a)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_ext_controls pad_ext_controls;
	int ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		pad_ext_controls.pad = pad->index;
		pad_ext_controls.ext_controls = a;
		ret =
		    v4l2_subdev_call(subdev, core, ioctl,
				     VISP_PAD_TRY_EXT_CTRLS, &pad_ext_controls);
	} else {
		return -ENOTTY;
	}

	return ret;
}

static int visp_vidioc_querymenu(struct file *file, void *fh,
				 struct v4l2_querymenu *a)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_querymenu pad_querymenu;
	int ret;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		pad_querymenu.pad = pad->index;
		pad_querymenu.querymenu = a;
		ret = v4l2_subdev_call(subdev, core, ioctl, VISP_PAD_QUERYMENU,
				       &pad_querymenu);
	} else {
		return -ENOTTY;
	}

	return ret;
}

static int
visp_videoc_subscribe_event(struct v4l2_fh *fh,
			    const struct v4l2_event_subscription *sub)
{
	int ret;

	switch (sub->type) {
	case V4L2_EVENT_CTRL:
		ret = v4l2_ctrl_subscribe_event(fh, sub);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static const struct v4l2_ioctl_ops visp_video_ioctl_ops = {
	.vidioc_querycap = visp_videoc_querycap,
	.vidioc_enum_fmt_vid_cap = visp_videoc_enum_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap = visp_videoc_try_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap = visp_videoc_g_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap = visp_videoc_s_fmt_vid_cap,

	.vidioc_reqbufs = visp_videoc_reqbufs,
	.vidioc_querybuf = vb2_ioctl_querybuf,
	.vidioc_create_bufs = vb2_ioctl_create_bufs,
	.vidioc_qbuf = vb2_ioctl_qbuf,
	.vidioc_expbuf = vb2_ioctl_expbuf,
	.vidioc_dqbuf = vb2_ioctl_dqbuf,
	.vidioc_prepare_buf = vb2_ioctl_prepare_buf,
	.vidioc_streamon = vb2_ioctl_streamon,
	.vidioc_streamoff = vb2_ioctl_streamoff,

	.vidioc_enum_input = visp_videoc_enum_input,
	.vidioc_g_input = visp_videoc_g_input,
	.vidioc_s_input = visp_videoc_s_input,
	//.vidioc_g_selection         = visp_videoc_g_selection,
	//.vidioc_s_selection         = visp_videoc_s_selection,
	.vidioc_g_parm = visp_videoc_g_parm,
	.vidioc_s_parm = visp_videoc_s_parm,
	//.vidioc_enum_framesizes     = visp_videoc_enum_framesizes,
	// .vidioc_enum_frameintervals = visp_videoc_enum_frmaeintervals,
	.vidioc_queryctrl = visp_videoc_queryctrl,
	.vidioc_query_ext_ctrl = visp_videoc_query_ext_ctrl,
	.vidioc_g_ctrl = visp_vidioc_g_ctrl,
	.vidioc_s_ctrl = visp_vidioc_s_ctrl,
	.vidioc_g_ext_ctrls = visp_vidioc_g_ext_ctrls,
	.vidioc_s_ext_ctrls = visp_vidioc_s_ext_ctrls,
	.vidioc_try_ext_ctrls = visp_vidioc_try_ext_ctrls,
	.vidioc_querymenu = visp_vidioc_querymenu,
	.vidioc_subscribe_event = visp_videoc_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
};

static __poll_t visp_video_poll(struct file *file,
				struct poll_table_struct *wait)
{
	struct v4l2_fh *fh = file->private_data;

	if (!list_empty(&fh->subscribed))
		return v4l2_ctrl_poll(file, wait);
	else
		return vb2_fop_poll(file, wait);
}

static int visp_video_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	struct v4l2_fh *fh = file->private_data;
	int ret = 0;

	if (visp_vdev->video->queue->owner &&
	    (visp_vdev->video->queue->owner == fh)) {
		return vb2_fop_mmap(file, vma);
	} else if (vma->vm_pgoff == visp_vdev->reserve_mem.pa >> PAGE_SHIFT) {
		vma->vm_pgoff = 0;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		ret = dma_mmap_coherent(visp_vdev->video->queue->dev, vma,
					visp_vdev->reserve_mem.va,
					visp_vdev->reserve_mem.pa,
					vma->vm_end - vma->vm_start);
	} else {
		ret = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
				      vma->vm_end - vma->vm_start,
				      vma->vm_page_prot);
		return ret;
	}
	return ret;
}

static int visp_video_open(struct file *file) { return v4l2_fh_open(file); }

static int visp_video_release(struct file *file)
{
	struct visp_video_dev *visp_vdev = video_drvdata(file);
	int ret;

	ret = vb2_fop_release(file);
	if (visp_vdev->video->queue->owner == NULL) {
		if (visp_vdev->pipeline)
			visp_video_destroy_pipeline(visp_vdev);
	}

	return ret;
}

static long visp_video_ioctl(struct file *file, unsigned int cmd,
			     unsigned long arg)
{
	long ret = 0;

	switch (cmd) {
	default:
		ret = video_ioctl2(file, cmd, arg);
		break;
	}

	return ret;
}

static const struct v4l2_file_operations visp_video_fops = {
	.owner = THIS_MODULE,
	.open = visp_video_open,
	.release = visp_video_release,
	.poll = visp_video_poll,
	.unlocked_ioctl = visp_video_ioctl,
	.mmap = visp_video_mmap,
};

static int visp_video_vb2_queue_setup(struct vb2_queue *queue,
				      unsigned int *num_buffers,
				      unsigned int *num_planes,
				      unsigned int sizes[],
				      struct device *alloc_devs[])
{
	struct visp_video_dev *visp_vdev = queue->drv_priv;
	struct v4l2_format *format = &visp_vdev->format;
	unsigned int i;

	if (format->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		if (*num_planes) {
			if (*num_planes != 1)
				return -EINVAL;
			if (sizes[0] < format->fmt.pix.sizeimage)
				return -EINVAL;
		} else {
			*num_planes = 1;
			sizes[0] = format->fmt.pix.sizeimage;
		}
	} else if (format->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		if (*num_planes) {
			if (*num_planes != format->fmt.pix_mp.num_planes)
				return -EINVAL;
			for (i = 0; i < format->fmt.pix_mp.num_planes; i++) {
				if (sizes[i] <
				    format->fmt.pix_mp.plane_fmt[i].sizeimage)
					return -EINVAL;
			}
		} else {
			*num_planes = format->fmt.pix_mp.num_planes;
			for (i = 0; i < format->fmt.pix_mp.num_planes; i++) {
				sizes[i] =
				    format->fmt.pix_mp.plane_fmt[i].sizeimage;
			}
		}
	} else {
		return -EINVAL;
	}

	return 0;
}

static int visp_video_vb2_buf_prepare(struct vb2_buffer *vb)
{
	struct visp_video_dev *visp_vdev = vb->vb2_queue->drv_priv;
	struct v4l2_format *format = &visp_vdev->format;
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct visp_vb2_buffer *buf =
	    container_of(vbuf, struct visp_vb2_buffer, vb);
	int i;

	if (format->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		if (vb2_plane_size(vb, 0) < format->fmt.pix.sizeimage)
			return -EINVAL;

		buf->num_planes = 1;
		buf->planes[0].dma_addr = vb2_dma_contig_plane_dma_addr(vb, 0);
		buf->planes[0].size = format->fmt.pix.sizeimage;
		buf->sequence = vb->index;

		vb2_set_plane_payload(vb, 0, buf->planes[0].size);
	} else if (format->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		buf->num_planes = format->fmt.pix_mp.num_planes;
		buf->sequence = vb->index;
		for (i = 0; i < format->fmt.pix_mp.num_planes; i++) {
			if (vb2_plane_size(vb, i) <
			    format->fmt.pix_mp.plane_fmt[i].sizeimage)
				return -EINVAL;

			buf->planes[i].dma_addr =
			    vb2_dma_contig_plane_dma_addr(vb, i);
			buf->planes[i].size =
			    format->fmt.pix_mp.plane_fmt[i].sizeimage;

			vb2_set_plane_payload(vb, i, buf->planes[i].size);
		}
	} else {
		return -EINVAL;
	}

	return 0;
}

static void visp_video_vb2_buf_queue(struct vb2_buffer *vb)
{
	struct visp_video_dev *visp_vdev = vb->vb2_queue->drv_priv;
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct visp_vb2_buffer *buf =
	    container_of(vbuf, struct visp_vb2_buffer, vb);
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_buf pad_buf;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		pad_buf.pad = pad->index;
		pad_buf.buf = buf;
		v4l2_subdev_call(subdev, core, ioctl, VISP_PAD_BUF_QUEUE,
				 &pad_buf);
	}
}

static int visp_video_vb2_start_streaming(struct vb2_queue *queue,
					  unsigned int count)
{
	struct visp_video_dev *visp_vdev = queue->drv_priv;
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_stream_status stream_status;
	int ret = -EINVAL;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		stream_status.pad = pad->index;
		stream_status.status = 1;
		ret = v4l2_subdev_call(subdev, core, ioctl, VISP_PAD_S_STREAM,
				       &stream_status);
	}

	return ret;
}

static void visp_video_vb2_stop_streaming(struct vb2_queue *queue)
{
	struct visp_video_dev *visp_vdev = queue->drv_priv;
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	struct visp_pad_stream_status stream_status;
	int i;

	subdev = visp_video_remote_subdev(visp_vdev);
	if (subdev) {
#if KERNEL_VERSION(6, 0, 0) <= LINUX_VERSION_CODE
		pad = media_pad_remote_pad_first(&visp_vdev->pad);
#else
		pad = media_entity_remote_pad(&visp_vdev->pad);
#endif
		stream_status.pad = pad->index;
		stream_status.status = 0;
		v4l2_subdev_call(subdev, core, ioctl, VISP_PAD_S_STREAM,
				 &stream_status);
	}

	for (i = 0; i < queue->max_num_buffers; i++) {
		if (queue->bufs[i] == NULL)
			return; // RET if max_numbuffers > allocated buffers.
		if (queue->bufs[i]->state == VB2_BUF_STATE_ACTIVE)
			vb2_buffer_done(queue->bufs[i], VB2_BUF_STATE_ERROR);
	}
}

static const struct vb2_ops visp_video_queue_ops = {
	.queue_setup = visp_video_vb2_queue_setup,
	.buf_prepare = visp_video_vb2_buf_prepare,
	.buf_queue = visp_video_vb2_buf_queue,
	.wait_prepare = vb2_ops_wait_prepare,
	.wait_finish = vb2_ops_wait_finish,
	.start_streaming = visp_video_vb2_start_streaming,
	.stop_streaming = visp_video_vb2_stop_streaming,
};

static int visp_video_queue_init(struct visp_video_dev *visp_vdev)
{
	int ret = 0;
	struct vb2_queue *queue;

	queue = &visp_vdev->queue;
	queue->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	queue->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	queue->drv_priv = visp_vdev;
	queue->ops = &visp_video_queue_ops;
	queue->mem_ops = &vb2_dma_contig_memops;
	queue->buf_struct_size = sizeof(struct visp_vb2_buffer);
	queue->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	queue->lock = &visp_vdev->video_lock;
	queue->dev = visp_vdev->visp_mdev->dev;
	ret = vb2_queue_init(queue);
	if (ret) {
		dev_err(visp_vdev->visp_mdev->dev, "vb2 queue init failed\n");
		return ret;
	}
	visp_vdev->video->queue = queue;

	return 0;
}

static int visp_video_link_setup(struct media_entity *entity,
				 const struct media_pad *local,
				 const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations visp_video_entity_ops = {
	.link_setup = visp_video_link_setup,
	.link_validate = v4l2_subdev_link_validate,
};

int visp_video_register(struct visp_media_dev *visp_mdev, int port)
{
	int ret = 0;
	struct visp_video_dev *visp_vdev;

	visp_vdev = devm_kzalloc(visp_mdev->dev, sizeof(struct visp_video_dev),
				 GFP_KERNEL);
	if (!visp_vdev)
		return -ENOMEM;

	mutex_init(&visp_vdev->video_lock);
	visp_vdev->visp_mdev = visp_mdev;
	visp_vdev->video_params = visp_mdev->video_params[port];

	visp_vdev->video = video_device_alloc();
	if (!visp_vdev->video) {
		dev_err(visp_mdev->dev, "could not alloc video device\n");
		ret = -ENOMEM;
		goto error_free_visp_vdev;
	}
	snprintf(visp_vdev->video->name, sizeof(visp_vdev->video->name),
		 "%s.%d.%d", VISP_VIDEO_NAME, visp_mdev->id, port);

	visp_vdev->video->fops = &visp_video_fops;
	visp_vdev->video->ioctl_ops = &visp_video_ioctl_ops;
	visp_vdev->video->release = video_device_release_empty;
	visp_vdev->video->v4l2_dev = &visp_mdev->v4l2_dev;
	// visp_vdev->video->lock          = &visp_vdev->video_lock;
	visp_vdev->video->device_caps =
	    V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	visp_vdev->video->minor = -1;

	video_set_drvdata(visp_vdev->video, visp_vdev);

	visp_vdev->video->entity.name = visp_vdev->video->name;
	visp_vdev->video->entity.obj_type = MEDIA_ENTITY_TYPE_VIDEO_DEVICE;
	visp_vdev->video->entity.function = MEDIA_ENT_F_IO_V4L;
	visp_vdev->video->entity.ops = &visp_video_entity_ops;
	visp_vdev->pad.flags = MEDIA_PAD_FL_SINK;
	ret = media_entity_pads_init(&visp_vdev->video->entity, 1,
				     &visp_vdev->pad);
	if (ret) {
		dev_err(visp_mdev->dev, "entity pad init error\n");
		goto error_video_device_release;
	}

	ret = visp_video_queue_init(visp_vdev);
	if (ret) {
		dev_err(visp_mdev->dev, "queue init error\n");
		goto err_media_entity_cleanup;
	}

	ret = video_register_device(visp_vdev->video, VFL_TYPE_VIDEO, -1);
	if (ret) {
		dev_err(visp_mdev->dev, "video register device error\n");
		goto err_media_entity_cleanup;
	}

	visp_mdev->video_devs[port] = visp_vdev;
	return 0;

err_media_entity_cleanup:
	media_entity_cleanup(&visp_vdev->video->entity);

error_video_device_release:
	video_device_release(visp_vdev->video);
	visp_vdev->video = NULL;

error_free_visp_vdev:
	devm_kfree(visp_mdev->dev, visp_vdev);

	return ret;
}

int visp_video_unregister(struct visp_media_dev *visp_mdev, int port)
{
	struct visp_video_dev *visp_vdev = visp_mdev->video_devs[port];

	if (visp_vdev == NULL)
		return 0;

	video_unregister_device(visp_vdev->video);
	media_entity_cleanup(&visp_vdev->video->entity);
	video_device_release(visp_vdev->video);
	devm_kfree(visp_mdev->dev, visp_vdev);
	visp_mdev->video_devs[port] = NULL;

	return 0;
}
