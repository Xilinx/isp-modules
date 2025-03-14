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

#ifndef __VISP_VIDEO_DRIVER_H__
#define __VISP_VIDEO_DRIVER_H__

#include <linux/list.h>
#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/videobuf2-dma-contig.h>

#ifdef VISP_PLATFORM_REGISTER
struct visp_v4l2_link
{
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

struct visp_video_params
{
	bool m2m;
};

struct visp_video_event_shm
{
	struct mutex event_lock;
	uint64_t phy_addr;
	void *virt_addr;
	uint32_t size;
};

struct visp_video_reserve_mem
{
	dma_addr_t pa;
	int size;
	void *va;
};

struct visp_video_dev
{
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

struct visp_media_dev
{
	int id;
	struct device *dev;
	struct media_device mdev;
	struct v4l2_device v4l2_dev;
	struct v4l2_async_notifier notifier;
	int ports;
	struct visp_video_reserve_mem reserve_mem;
	struct visp_video_params video_params[VISP_VIDEO_PORT_MAX];
	struct visp_video_dev *video_devs[VISP_VIDEO_PORT_MAX];
#ifdef VISP_PLATFORM_REGISTER
	struct visp_v4l2_link *pipeline_link;
	uint32_t pipeline_link_size;
#endif
};

struct visp_video_fmt_info
{
	uint32_t fourcc;
	uint32_t mbus;
};

struct visp_video_dma_buf
{
	uint64_t pa;
	int size;
};


#endif
