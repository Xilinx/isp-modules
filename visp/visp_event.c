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
#include <linux/namei.h>
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
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct v4l2_event event;
	unsigned long remaining;

	memset(&event, 0, sizeof(event));

	event.type = VISP_DEAMON_EVENT;
	event.id = event_pkg->head.eid;
	memcpy(event.u.data, &event_pkg->head, sizeof(event_pkg->head));

	if (!visp_event_subscribed(sd, event.type, event.id)) {
		dev_err(sd->dev, "post event %d not subscribed\n", event.id);
		return -EPIPE;
	}

	/* Assign sequence number for stale-ack protection */
	isp_dev->event_shm.seq++;
	event_pkg->seq = isp_dev->event_shm.seq;

	reinit_completion(&isp_dev->event_shm.event_ack);

	v4l2_event_queue(sd->devnode, &event);

	remaining = wait_for_completion_timeout(&isp_dev->event_shm.event_ack,
						msecs_to_jiffies(30000));
	if (!remaining) {
		dev_err(sd->dev, "post event %d time out (seq=%u)\n",
			event.id, event_pkg->seq);
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
	struct v4l2_event event;

	event.type = VISP_DEAMON_EVENT;
	event.id = VISP_EVENT_LOAD_CALIB;

	if (!visp_event_subscribed(&isp_dev->sd, event.type, event.id)) {
		dev_err(isp_dev->dev, "post event %d not subscribed\n", event.id);
		return -EPIPE;
	}

	struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret = 0;
	uint8_t *pdata = event_pkg->data;
	int port = pad / MEDIA_ISP_PORT_PAD_COUNT;
	cam_device_handle_t *readback = NULL;
	media_isp_sensor_info *readback_1 = NULL;
	media_isp_port_attr *isp_port = NULL;

	/* Bounds check: verify total payload fits in event shm buffer */
	size_t required_size = offsetof(struct visp_event_pkg, data) +
			       sizeof(uint32_t) +
			       sizeof(struct visp_subdev_dma_buf) +
			       sizeof(cam_device_context_t) +
			       sizeof(media_isp_sensor_info);
	if (required_size > isp_dev->event_shm.size) {
		dev_err(isp_dev->dev,
			"%s: required %zu exceeds shm buffer %u\n",
			__func__, required_size, isp_dev->event_shm.size);
		return -EOVERFLOW;
	}

	mutex_lock(&isp_dev->event_shm.event_lock);
	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_LOAD_CALIB;

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

	if (!isp_dev->isp_ports[port].sensor_info.calib[0]) {
		dev_err(isp_dev->dev,
			"calib path not set for port %d, aborting calib event\n",
			port);
		mutex_unlock(&isp_dev->event_shm.event_lock);
		return -EINVAL;
	}

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
	}
	mutex_unlock(&isp_dev->event_shm.event_lock);
	return ret;
}

int visp_l_json_event(struct visp_dev *isp_dev, int pad)
{
	struct v4l2_event event;

	event.type = VISP_DEAMON_EVENT;
	event.id = VISP_EVENT_LOAD_JSON;

	if (!visp_event_subscribed(&isp_dev->sd, event.type, event.id)) {
		dev_err(isp_dev->dev, "post event %d not subscribed\n", event.id);
		return -EPIPE;
	}

	struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret = 0;
	int port = pad / MEDIA_ISP_PORT_PAD_COUNT;
	uint8_t *pdata = NULL;

	if (isp_dev->isp_ports[port].one_json_mode) {
		if (!isp_dev->isp_ports[port].sensor_info.one_json[0]) {
			dev_err(isp_dev->dev,
				"one_json path not set for port %d\n", port);
			return -EINVAL;
		}
	} else {
		if (!isp_dev->isp_ports[port].sensor_info.manu_json[0]) {
			dev_err(isp_dev->dev,
				"manu_json path not set for port %d\n", port);
			return -EINVAL;
		}

		if (!isp_dev->isp_ports[port].sensor_info.auto_json[0]) {
			dev_err(isp_dev->dev,
				"auto_json path not set for port %d\n", port);
			return -EINVAL;
		}
	}

	mutex_lock(&isp_dev->event_shm.event_lock);
	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_LOAD_JSON;

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

	/* If stream is not active, control cannot be applied to RPU */
	if (ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port] < 1) {
		dev_info(isp_dev->dev, "unable to set control prior to streamon\n");
		return -EBUSY;
	}

	mutex_lock(&isp_dev->event_shm.event_lock);

	/* Bounds check to prevent writing past event buffer */
	size_t ctrl_data_size = ctrl->elem_size * ctrl->elems;
	size_t required_size = offsetof(struct visp_event_pkg, data) +
			       sizeof(uint32_t) + sizeof(cam_device_context_t) +
			       sizeof(struct visp_ctrl) + ctrl_data_size;
	if (required_size > isp_dev->event_shm.size) {
		dev_err(isp_dev->dev,
			"%s: ctrl data size %zu exceeds shm buffer %u\n",
			__func__, required_size, isp_dev->event_shm.size);
		mutex_unlock(&isp_dev->event_shm.event_lock);
		return -EOVERFLOW;
	}

	event_pkg->head.data_size = 0;
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

	isp_ctrl = (struct visp_ctrl *)pdata;
	isp_ctrl->cid = ctrl->id;
	isp_ctrl->size = ctrl_data_size;
	memcpy(isp_ctrl->data, ctrl->p_new.p_u8, isp_ctrl->size);

	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_S_CTRL;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size += (sizeof(struct visp_ctrl) + isp_ctrl->size);
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

	/* If stream is not active, return default (zero) values */
	if (ISP_DEV_EXTENDED(isp_dev)->subdev_streamon_count[port] < 1) {
		memset(ctrl->p_new.p_u8, 0, ctrl->elem_size * ctrl->elems);
		return 0;
	}

	mutex_lock(&isp_dev->event_shm.event_lock);

	/* Bounds check to prevent writing past event buffer */
	size_t ctrl_data_size = ctrl->elem_size * ctrl->elems;
	size_t required_size = offsetof(struct visp_event_pkg, data) +
			       sizeof(uint32_t) + sizeof(cam_device_context_t) +
			       sizeof(struct visp_ctrl) + ctrl_data_size;
	if (required_size > isp_dev->event_shm.size) {
		dev_err(isp_dev->dev,
			"%s: ctrl data size %zu exceeds shm buffer %u\n",
			__func__, required_size, isp_dev->event_shm.size);
		mutex_unlock(&isp_dev->event_shm.event_lock);
		return -EOVERFLOW;
	}

	event_pkg->head.data_size = 0;
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

	isp_ctrl = (struct visp_ctrl *)pdata;
	isp_ctrl->cid = ctrl->id;
	isp_ctrl->size = ctrl_data_size;

	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_G_CTRL;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size += (sizeof(struct visp_ctrl) + isp_ctrl->size);
	event_pkg->ack = 0;
	event_pkg->result = 0;

	ret = visp_post_event(&isp_dev->sd, event_pkg);

	if (ret == 0) {
		if (isp_ctrl->size > ctrl_data_size) {
			dev_err(isp_dev->dev,
				"%s: returned size %u exceeds ctrl size %zu\n",
				__func__, isp_ctrl->size, ctrl_data_size);
			ret = -EOVERFLOW;
		} else {
			memcpy(ctrl->p_new.p_u8, pdata + sizeof(struct visp_ctrl),
			       isp_ctrl->size);
		}
	}

	mutex_unlock(&isp_dev->event_shm.event_lock);

	return ret;
}

int visp_s_interval_event(struct visp_dev *isp_dev, int pad,
			  struct v4l2_fract *timeperframe)
{
	struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret;

	ret = visp_setup_isp_pipeline(isp_dev, pad);
	if (ret)
		return ret;

	mutex_lock(&isp_dev->event_shm.event_lock);
	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_S_INTERVAL;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size = sizeof(struct v4l2_fract);
	event_pkg->ack = 0;
	event_pkg->result = 0;
	memcpy(event_pkg->data, timeperframe, sizeof(struct v4l2_fract));

	ret = visp_post_event(&isp_dev->sd, event_pkg);

	mutex_unlock(&isp_dev->event_shm.event_lock);

	return ret;
}

int visp_l_fusa_event(struct visp_dev *isp_dev, int pad)
{
	struct v4l2_event event;

	event.type = VISP_DEAMON_EVENT;
	event.id = VISP_EVENT_LOAD_FUSA;

	if (!visp_event_subscribed(&isp_dev->sd, event.type, event.id)) {
		dev_err(isp_dev->dev, "post event %d not subscribed\n", event.id);
		return -EPIPE;
	}

	struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret = 0;
	int port = pad / MEDIA_ISP_PORT_PAD_COUNT;
	uint8_t *pdata = NULL;

	if (!isp_dev->isp_ports[port].fusa_json[0]) {
		dev_err(isp_dev->dev,
			"fusa_json path not set for port %d, aborting fusa event\n",
			port);
		return -EINVAL;
	}

	/* Bounds check: verify fusa_json payload fits in event shm buffer */
	size_t required_size = offsetof(struct visp_event_pkg, data) +
			       sizeof(isp_dev->isp_ports[port].fusa_json);
	if (required_size > isp_dev->event_shm.size) {
		dev_err(isp_dev->dev, "%s: required %zu exceeds shm buffer %u\n",
			__func__, required_size, isp_dev->event_shm.size);
		return -EOVERFLOW;
	}

	mutex_lock(&isp_dev->event_shm.event_lock);
	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_LOAD_FUSA;

	event_pkg->head.shm_fd = isp_dev->event_shm.dmabuf_fd;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size = 0;
	event_pkg->ack = 0;
	event_pkg->result = 0;

	pdata = event_pkg->data;

	memcpy(pdata, &isp_dev->isp_ports[port].fusa_json,
	       sizeof(isp_dev->isp_ports[port].fusa_json));
	pdata += sizeof(isp_dev->isp_ports[port].fusa_json);
	event_pkg->head.data_size += sizeof(isp_dev->isp_ports[port].fusa_json);

	ret = visp_post_event(&isp_dev->sd, event_pkg);
	if (ret != 0) {
		dev_err(isp_dev->dev, "[EVENT_FAIL] %s %d\n", __func__,
			__LINE__);
	}

	mutex_unlock(&isp_dev->event_shm.event_lock);
	return ret;
}

int visp_stop_fusa_event(struct visp_dev *isp_dev, int pad)
{
	struct v4l2_event event;

	event.type = VISP_DEAMON_EVENT;
	event.id = VISP_EVENT_STOP_FUSA;

	if (!visp_event_subscribed(&isp_dev->sd, event.type, event.id)) {
		dev_err(isp_dev->dev, "post event %d not subscribed\n", event.id);
		return -EPIPE;
	}

	struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret = 0;

	mutex_lock(&isp_dev->event_shm.event_lock);
	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_STOP_FUSA;

	event_pkg->head.shm_fd = isp_dev->event_shm.dmabuf_fd;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size = 0;
	event_pkg->ack = 0;
	event_pkg->result = 0;

	ret = visp_post_event(&isp_dev->sd, event_pkg);
	if (ret != 0) {
		dev_err(isp_dev->dev, "[EVENT_FAIL] %s %d\n", __func__,
			__LINE__);
	}

	mutex_unlock(&isp_dev->event_shm.event_lock);
	return ret;
}
