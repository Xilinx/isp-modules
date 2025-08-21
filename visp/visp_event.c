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

#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <linux/delay.h>
#include "visp_event.h"
#include "visp_driver.h"

static bool visp_event_subscribed(struct v4l2_subdev *sd, uint32_t type,
				  uint32_t id)
{
	struct v4l2_fh *fh;
	unsigned long flags;
	struct v4l2_subscribed_event *sev;
	bool subscribed = false;

	spin_lock_irqsave(&sd->devnode->fh_lock, flags);

	list_for_each_entry(fh, &sd->devnode->fh_list, list) {
		list_for_each_entry(sev, &fh->subscribed, list) {
			if (sev->type == type && sev->id == id) {
				subscribed = true;
				break;
			}
		}
		if (subscribed)
			break;
	}

	spin_unlock_irqrestore(&sd->devnode->fh_lock, flags);

	return subscribed;
}

static int visp_post_event(struct v4l2_subdev *sd,
			   struct visp_event_pkg *event_pkg)
{
	struct v4l2_event event;
	int timeout_ms = 6000000;
	int i = 0;

	memset(&event, 0, sizeof(event));

	event.type = VISP_DEAMON_EVENT;
	event.id = event_pkg->head.eid;
	memcpy(event.u.data, &event_pkg->head, sizeof(event_pkg->head));

	if (!visp_event_subscribed(sd, event.type, event.id)) {
		dev_err(sd->dev, "post event %d not subscribed\n", event.id);
		return -EINVAL;
	}

	v4l2_event_queue(sd->devnode, &event);

	for (i = 0; i < timeout_ms; i++) {
		if (event_pkg->ack)
			break;
		usleep_range(5, 10);
		if (i > 5000000)
			i = 0;
	}

	if (event_pkg->ack == 0) {
		dev_err(sd->dev, "post event %d time out\n", event.id);
		return -EIO;
	}

	if (event_pkg->result) {
		dev_err(sd->dev, "post event %d return error\n", event.id);
		return -EINVAL;
	}

	return 0;
}
typedef struct cam_device_context_s {
	uint32_t isp_hw_id;
	uint32_t isp_vt_id;
	uint32_t instance_id;
	uint32_t cookie;
} cam_device_context_t;

typedef struct test {
	int id2;
	int id;
} test_t;

int visp_l_calib_event(struct visp_dev *isp_dev, int pad)
{
	struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret = 0;
	uint8_t *pdata = event_pkg->data;
	int port = pad / MEDIA_ISP_PORT_PAD_COUNT;
	cam_device_handle_t *readback = NULL;
	media_isp_sensor_info *readback_1 = NULL;
	media_isp_port_attr *isp_port = NULL;

	mutex_lock(&isp_dev->event_shm.event_lock);
	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_LOAD_CALIB;

	event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size = 0;
	event_pkg->ack = 0;
	event_pkg->result = 0;

	memcpy(pdata, &port, sizeof(uint32_t));
	pdata += sizeof(uint32_t);
	event_pkg->head.data_size += sizeof(uint32_t);

	struct visp_subdev_dma_buf dma_buf;

	dma_buf.pa = isp_dev->reserve_mem.pa;
	dma_buf.size = isp_dev->reserve_mem.size;
	memcpy(pdata, &dma_buf, sizeof(struct visp_subdev_dma_buf));
	pdata += sizeof(struct visp_subdev_dma_buf);
	event_pkg->head.data_size += sizeof(struct visp_subdev_dma_buf);

	memcpy(pdata, isp_dev->isp_ports[port].cam_device_handle,
	       sizeof(cam_device_context_t));
	pdata += sizeof(cam_device_context_t);
	event_pkg->head.data_size += sizeof(cam_device_context_t);

	memcpy(pdata, &(isp_dev->isp_ports[port].sensor_info),
	       sizeof(media_isp_sensor_info));
	pdata += sizeof(media_isp_sensor_info);
	event_pkg->head.data_size += sizeof(media_isp_sensor_info);

	readback = (cam_device_handle_t *)(event_pkg->data);

	isp_port = &(isp_dev->isp_ports[port]);
	readback_1 =
	    (media_isp_sensor_info *)(event_pkg->data + sizeof(uint32_t));

	ret = visp_post_event(&isp_dev->sd, event_pkg);
	if (ret != 0) {
		dev_info(isp_dev->dev, "[FAIL] %s %d\n", __func__, __LINE__);
		return ret;
	}
	mutex_unlock(&isp_dev->event_shm.event_lock);
	return ret;
}

int visp_l_json_event(struct visp_dev *isp_dev, int pad)
{
	struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret = 0;
	int port = pad / MEDIA_ISP_PORT_PAD_COUNT;
	uint8_t *pdata = NULL;

	mutex_lock(&isp_dev->event_shm.event_lock);
	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_LOAD_JSON;

	event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size = 0;
	event_pkg->ack = 0;
	event_pkg->result = 0;

	pdata = event_pkg->data;

	memcpy(pdata, &port, sizeof(uint32_t));
	pdata += sizeof(uint32_t);
	event_pkg->head.data_size += sizeof(uint32_t);

	ret = visp_post_event(&isp_dev->sd, event_pkg);
	if (ret != 0) {
		dev_err(isp_dev->dev, "[EVENT_FAIL] %s %d\n", __func__,
			__LINE__);
		return ret;
	}

	mutex_unlock(&isp_dev->event_shm.event_lock);
	return ret;
}

int visp_s_ctrl_event(struct visp_dev *isp_dev, int pad,
		      struct v4l2_ctrl *ctrl)
{
	struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret;
	struct visp_ctrl *isp_ctrl;
	u8 *pdata = event_pkg->data;
	int port = pad / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

	if (isp_dev->streamon[pad] == 0) {
		dev_err(isp_dev->dev,
			"%s %d Should not use v4l2-ctrl for this node, Device is not streamed on ispid : %d, port %d Chn:%d\n",
			 __func__, __LINE__, isp_dev->id, port, chn);
		return -1;
	}

	mutex_lock(&isp_dev->event_shm.event_lock);

	memcpy(pdata, &port, sizeof(uint32_t));
	pdata += sizeof(uint32_t);
	event_pkg->head.data_size += sizeof(uint32_t);

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)isp_dev->isp_ports[port].cam_device_handle;
	p_cam_dev_ctx->cookie++;

	memcpy(pdata, isp_dev->isp_ports[port].cam_device_handle,
	       sizeof(cam_device_context_t));
	pdata += sizeof(cam_device_context_t);
	event_pkg->head.data_size += sizeof(cam_device_context_t);

	isp_ctrl = (struct visp_ctrl *)pdata/*event_pkg->data*/;
	isp_ctrl->cid = ctrl->id;
	isp_ctrl->size = ctrl->elem_size * ctrl->elems;
	memcpy(isp_ctrl->data, ctrl->p_new.p_u8, isp_ctrl->size);

	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_S_CTRL;
	event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size += (sizeof(isp_ctrl) + isp_ctrl->size);
	event_pkg->ack = 0;
	event_pkg->result = 0;

	ret = visp_post_event(&isp_dev->sd, event_pkg);

	mutex_unlock(&isp_dev->event_shm.event_lock);

	return ret;
}

int visp_g_ctrl_event(struct visp_dev *isp_dev, int pad,
		      struct v4l2_ctrl *ctrl)
{
	struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret = 0;
	struct visp_ctrl *isp_ctrl;
	u8 *pdata = event_pkg->data;
	int port = pad / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

	if (isp_dev->streamon[pad] == 0) {
		dev_err(isp_dev->dev,
			"%s %d Should not use v4l2-ctrl for this node, Device is not streamed on ispid : %d, port %d Chn:%d\n",
			 __func__, __LINE__, isp_dev->id, port, chn);
		return -1;
	}

	mutex_lock(&isp_dev->event_shm.event_lock);

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)isp_dev->isp_ports[port].cam_device_handle;

	p_cam_dev_ctx->cookie++;

	memcpy(pdata, &port, sizeof(uint32_t));
	pdata += sizeof(uint32_t);
	event_pkg->head.data_size += sizeof(uint32_t);

	memcpy(pdata, isp_dev->isp_ports[port].cam_device_handle,
	       sizeof(cam_device_context_t));
	pdata += sizeof(cam_device_context_t);
	event_pkg->head.data_size += sizeof(cam_device_context_t);

	isp_ctrl = (struct visp_ctrl *)pdata;//(event_pkg->data);
	isp_ctrl->cid = ctrl->id;
	isp_ctrl->size = ctrl->elem_size * ctrl->elems;

	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_G_CTRL;
	event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size += (sizeof(isp_ctrl) + isp_ctrl->size);
	event_pkg->ack = 0;
	event_pkg->result = 0;

	ret = visp_post_event(&isp_dev->sd, event_pkg);

	if (ret == 0) {
		memcpy(ctrl->p_new.p_u8, /*event_pkg->data*/pdata + sizeof(isp_ctrl),
		       isp_ctrl->size);
	}

	mutex_unlock(&isp_dev->event_shm.event_lock);

	return ret;
}

int visp_s_interval_event(struct visp_dev *isp_dev, int pad,
			  struct v4l2_fract *timeperframe)
{
	struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret;

	mutex_lock(&isp_dev->event_shm.event_lock);
	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_S_INTERVAL;
	event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size = sizeof(struct v4l2_fract);
	event_pkg->ack = 0;
	event_pkg->result = 0;
	memcpy(event_pkg->data, timeperframe, sizeof(struct v4l2_fract));

	ret = visp_post_event(&isp_dev->sd, event_pkg);

	mutex_unlock(&isp_dev->event_shm.event_lock);

	return ret;
}
