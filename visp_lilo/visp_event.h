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

#ifndef __VISP_EVENT_H__
#define __VISP_EVENT_H__
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>

#define VISP_DEAMON_EVENT (V4L2_EVENT_PRIVATE_START + 2000)

enum visp_vevent_id
{
	VISP_EVENT_SET_FMT,
	VISP_EVENT_REQBUFS,
	VISP_EVENT_QBUF,
	VISP_EVENT_BUF_DONE,
	VISP_EVENT_STREAMON,
	VISP_EVENT_STREAMOFF,
	VISP_EVENT_S_CTRL,
	VISP_EVENT_G_CTRL,
	VISP_EVENT_LOAD_CALIB,
	VISP_EVENT_LOAD_JSON,
	VISP_EVENT_S_INTERVAL,
	VISP_EVENT_MAX,
};

struct visp_plane
{
	uint32_t dma_addr;
	uint32_t size;
};

struct visp_buf
{
	uint32_t pad;
	uint32_t index;
	uint32_t num_planes;
	struct visp_plane planes[VIDEO_MAX_PLANES];
};

struct visp_ctrl
{
	uint32_t cid;
	uint32_t size;
#ifdef __KERNEL__
	uint8_t data[0];
#endif
};

struct visp_event_pkg_head
{
	uint32_t pad;
	uint8_t dev;
	uint32_t eid;
	uint64_t shm_addr;
	uint32_t shm_size;
	uint32_t data_size;
};

struct visp_event_pkg
{
	struct visp_event_pkg_head head;
	uint8_t ack;
	int32_t result;
	uint8_t data[2048];
};

struct visp_ext_buf_info
{
	uint8_t port;
	struct visp_plane plane;
};

#define VISP_IOC_BUFDONE _IOWR('I', BASE_VIDIOC_PRIVATE + 0, struct visp_buf)
#define VISP_IOC_BUFFER_ALLOC \
	_IOWR('I', BASE_VIDIOC_PRIVATE + 1, struct visp_ext_buf_info)
#define VISP_IOC_BUFFER_FREE \
	_IOWR('I', BASE_VIDIOC_PRIVATE + 2, struct visp_ext_buf_info)

#define VISP_GET_RPU_ID \
    _IOWR('I', BASE_VIDIOC_PRIVATE + 3, uint32_t )

#ifdef __KERNEL__
#include "visp_driver.h"

int visp_set_fmt_event(struct visp_dev *isp_dev, int pad,
					   struct v4l2_mbus_framefmt *format);
int visp_requebus_event(struct visp_dev *isp_dev, int pad,
						uint32_t num_buffers);
int visp_qbuf_event(struct visp_dev *isp_dev, int pad,
					struct visp_vb2_buffer *buf);
int visp_s_stream_event(struct visp_dev *isp_dev, int pad, uint32_t status);
int visp_s_ctrl_event(struct visp_dev *isp_dev, int pad,
					  struct v4l2_ctrl *ctrl);
int visp_g_ctrl_event(struct visp_dev *isp_dev, int pad,
					  struct v4l2_ctrl *ctrl);
int visp_l_calib_event(struct visp_dev *isp_dev, int pad);
int visp_l_json_event(struct visp_dev *isp_dev, int pad);
int visp_s_interval_event(struct visp_dev *isp_dev, int pad,
						  struct v4l2_fract *timeperframe);

#endif

#endif
