/* SPDX-License-Identifier: MIT */
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

#ifndef __VISP_VIDEO_DRIVER_H__
#define __VISP_VIDEO_DRIVER_H__

#include <linux/list.h>
#include <linux/kref.h>
#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/videobuf2-dma-contig.h>

#ifdef VISP_PLATFORM_REGISTER
struct visp_v4l2_link {
	struct v4l2_subdev **local_subdev;
	bool local_is_video;
	uint32_t video_index;
	int local_pad;
	struct v4l2_subdev **remote_subdev;
	int remote_pad;
};
#endif

#define VISP_VIDEO_NAME "visp-video"
#define VISP_VIDEO_PORT_MAX 64

#define VISP_VIDEO_WIDTH_ALIGN 16
#define VISP_VIDEO_HEIGHT_ALIGN 8

#define VISP_VIDEO_MIN_WIDTH 32
#define VISP_VIDEO_MIN_HEIGHT 16

struct visp_video_params {
	bool m2m;
};

struct visp_video_event_shm {
	struct mutex event_lock;
	uint64_t phy_addr;
	void *virt_addr;
	uint32_t size;
};

struct visp_video_reserve_mem {
	dma_addr_t pa;
	int size;
	void *va;
};

struct visp_video_dev {
	struct visp_video_params video_params;
	struct visp_media_dev *visp_mdev;
	struct video_device *video;
	struct media_pad pad;
	struct vb2_queue queue;
	struct mutex video_lock;
	struct v4l2_format format;
	uint32_t pipeline;
	struct visp_video_reserve_mem reserve_mem;
	struct visp_video_event_shm event_shm;
};

struct visp_media_dev {
	int id;
	struct device *dev;
	struct media_device mdev;
	struct v4l2_device v4l2_dev;
	/*
	 * Keeps this struct (and the embedded media_device) alive until the
	 * last video/media fd closes, so a runtime unbind (WDT recovery) can't
	 * free it out from under an open fd. Held by the driver plus one ref
	 * per registered video node.
	 */
	struct kref ref;
	struct v4l2_async_notifier notifier;
	int ports;
	struct visp_video_reserve_mem reserve_mem;
	struct visp_video_params video_params[VISP_VIDEO_PORT_MAX];
	struct visp_video_dev *video_devs[VISP_VIDEO_PORT_MAX];
#ifdef VISP_PLATFORM_REGISTER
	struct visp_v4l2_link *pipeline_link;
	uint32_t pipeline_link_size;
#endif
	/*
	 * LILO mixed-mode (memory-out) binding. When the visp_video DT node
	 * declares its ISP source(s) by phandle ("visp,source-subdev" /
	 * "visp,source-pad") instead of an OF-graph endpoint, the media link
	 * to the ISP subdev source pad is created programmatically at
	 * notifier-complete. This keeps the ISP subdev's memory-path source
	 * pad free of any OF-graph endpoint, so the in-kernel xilinx-vipp
	 * bridge never walks (and never aborts on) that link. Legacy
	 * endpoint-based binding (LIMO, and plain LILO) is used when
	 * phandle_mode is false.
	 */
	bool phandle_mode;
	struct device_node *src_subdev_np[VISP_VIDEO_PORT_MAX];
	u32 src_subdev_pad[VISP_VIDEO_PORT_MAX];
	/*
	 * Mixed mode reuses the source subdev's existing v4l2_device /
	 * media_device (the one xilinx-vipp created and the ISP subdev is
	 * already bound into) instead of creating its own. shared_v4l2_dev
	 * points there; in legacy mode it points at our own v4l2_dev.
	 */
	struct v4l2_device *shared_v4l2_dev;
	/*
	 * Resolved once in visp_video_probe() and pinned there (visp.ko's
	 * module refcount is held for as long as each entry is non-NULL,
	 * released in visp_video_put_src_subdevs()) - the module-level pin
	 * only protects against visp.ko being unloaded, not against the
	 * specific owning ISP instance being torn down independently (its
	 * own visp_remove() while visp.ko stays resident for other
	 * instances). Currently safe because nothing dereferences an entry
	 * after probe (visp_video_put_src_subdevs() only null-checks and
	 * releases the pin). Any future code that dereferences src_sd[i]
	 * post-probe (e.g. a v4l2_subdev_call()) MUST re-resolve/validate
	 * it first rather than trusting this cached pointer indefinitely.
	 */
	struct v4l2_subdev *src_sd[VISP_VIDEO_PORT_MAX];
};

/* kref release for struct visp_media_dev; frees the struct once the last
 * reference (driver + per-node) is dropped.
 */
void visp_media_dev_free(struct kref *kref);

struct visp_video_fmt_info {
	uint32_t fourcc;
	uint32_t mbus;
};

struct visp_video_dma_buf {
	uint64_t pa;
	int size;
};

#endif
