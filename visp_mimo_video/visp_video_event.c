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

#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <linux/delay.h>
#include "visp_video_event.h"
#include "visp_mimo_driver.h"
#include <visp_event.h>
struct visp_subdev_dma_buf {
	uint64_t pa;
	int size;
};

static bool visp_subdev_event_subscribed(struct visp_mimo_device *visp_vdev,
							uint32_t type, uint32_t id)
{
	struct v4l2_fh *fh;
	unsigned long flags;
	struct v4l2_subscribed_event *sev;
	bool subscribed = false;
	if (!visp_vdev->subdev.devnode) {
		pr_err("%s: subdev devnode not initialized\n", __func__);
		return false;
	}
	spin_lock_irqsave(&visp_vdev->subdev.devnode->fh_lock, flags);

	list_for_each_entry(fh, &visp_vdev->subdev.devnode->fh_list, list) {
		list_for_each_entry(sev, &fh->subscribed, list) {
			if (sev->type == type && sev->id == id) {
				subscribed = true;
				break;
			}
		}
		if (subscribed)
			break;
	}

	spin_unlock_irqrestore(&visp_vdev->subdev.devnode->fh_lock, flags);

	return subscribed;
}

int visp_subdev_post_event(struct visp_mimo_device *visp_vdev,
			  struct visp_event_pkg *event_pkg);
int visp_subdev_post_event(struct visp_mimo_device *visp_vdev,
			  struct visp_event_pkg *event_pkg)
{
	struct v4l2_event event;
	unsigned long remaining;

	memset(&event, 0, sizeof(event));

	event.type = VISP_DEAMON_EVENT;
	event.id = event_pkg->head.eid;
	memcpy(event.u.data, &event_pkg->head, sizeof(event_pkg->head));

	if (!visp_subdev_event_subscribed(visp_vdev, event.type, event.id)) {
		return -EINVAL;
	}

	/* Assign sequence number for stale-ack protection */
	visp_vdev->event_shm.seq++;
	event_pkg->seq = visp_vdev->event_shm.seq;

	reinit_completion(&visp_vdev->event_shm.event_ack);

	v4l2_event_queue(visp_vdev->subdev.devnode, &event);

	remaining = wait_for_completion_timeout(&visp_vdev->event_shm.event_ack,
						msecs_to_jiffies(30000));
	if (!remaining) {
		pr_err("%s post event %d time out (seq=%u)\n", visp_vdev->video_dev.name,
		       event.id, event_pkg->seq);
		return -EIO;
	}

	if (event_pkg->result) {
		pr_err("%s post event %d return error\n", visp_vdev->video_dev.name,
		       event.id);
		return -EINVAL;
	}

	return 0;
}

int visp_l_calib_event(struct visp_mimo_device *device, int pad, int event)
{

	struct visp_event_pkg *event_pkg = device->event_shm.virt_addr;
	int ret = 0;
	uint8_t *pdata = event_pkg->data;
	int port = pad / MEDIA_ISP_PORT_PAD_COUNT;

	mutex_lock(&device->event_shm.event_lock);
	event_pkg->head.eid = event;
	event_pkg->head.shm_fd = -1;
	event_pkg->head.shm_size = device->event_shm.size;
	event_pkg->head.data_size = 0;
	event_pkg->ack = 0;
	event_pkg->result = 0;

	memcpy(pdata, &port, sizeof(uint32_t));
	pdata += sizeof(uint32_t);
	event_pkg->head.data_size += sizeof(uint32_t);

	struct visp_subdev_dma_buf dma_buf;

	dma_buf.pa = device->reserve_mem.pa;
	dma_buf.size = device->reserve_mem.size;
	memcpy(pdata, &dma_buf, sizeof(struct visp_subdev_dma_buf));
	pdata += sizeof(struct visp_subdev_dma_buf);

	event_pkg->head.data_size += sizeof(struct visp_subdev_dma_buf);

	ret = visp_subdev_post_event(device, event_pkg);
	if (ret != 0)
		pr_err("[FAIL] %s %d\n", __func__, __LINE__);
	mutex_unlock(&device->event_shm.event_lock);

	return ret;
}


int visp_s_ctrl_event(struct visp_dev *isp_dev, int pad,
		      struct v4l2_ctrl *ctrl)
{
	struct visp_mimo_device *device =
		(struct visp_mimo_device *)ISP_DEV_EXTENDED_MIMO_VIDEO(isp_dev)->mimo_device;
	struct visp_event_pkg *event_pkg = device->event_shm.virt_addr;
	int ret;
	struct visp_ctrl *isp_ctrl;
	u8 *pdata;
	if (!event_pkg) {
		dev_err(isp_dev->dev, "%s: event_shm not allocated\n", __func__);
		return -ENOMEM;
	}

	if (!ctrl->p_new.p_u8) {
		dev_err(isp_dev->dev, "%s: ctrl pointer invalid\n", __func__);
		return -EINVAL;
	}

	pdata = event_pkg->data;

	mutex_lock(&device->event_shm.event_lock);

	/* Bounds check to prevent writing past buffer */
	size_t required_size = sizeof(struct visp_event_pkg_head) + sizeof(struct visp_ctrl) +
			       (ctrl->elem_size * ctrl->elems);
	if (required_size > device->event_shm.size) {
		dev_err(isp_dev->dev, "%s: ctrl data size %zu exceeds buffer size %u\n",
			__func__, required_size, device->event_shm.size);
		mutex_unlock(&device->event_shm.event_lock);
		return -EOVERFLOW;
	}

	isp_ctrl = (struct visp_ctrl *)pdata;
	isp_ctrl->cid = ctrl->id;
	isp_ctrl->size = ctrl->elem_size * ctrl->elems;
	memcpy(isp_ctrl->data, ctrl->p_new.p_u8, isp_ctrl->size);

	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_S_CTRL;
	event_pkg->head.shm_fd = -1;  /* Not used with DMA-BUF */
	event_pkg->head.shm_size = device->event_shm.size;
	event_pkg->head.data_size = sizeof(struct visp_ctrl) + isp_ctrl->size;
	event_pkg->ack = 0;
	event_pkg->result = 0;

	ret = visp_subdev_post_event(device, event_pkg);

	mutex_unlock(&device->event_shm.event_lock);
	return ret;
}

int visp_g_ctrl_event(struct visp_dev *isp_dev, int pad,
		      struct v4l2_ctrl *ctrl)
{
	struct visp_mimo_device *device =
		(struct visp_mimo_device *)ISP_DEV_EXTENDED_MIMO_VIDEO(isp_dev)->mimo_device;
	struct visp_event_pkg *event_pkg = device->event_shm.virt_addr;
	int ret = 0;
	struct visp_ctrl *isp_ctrl;
	u8 *pdata;

	if (!event_pkg) {
		dev_err(isp_dev->dev, "%s: event_shm not allocated\n", __func__);
		return -ENOMEM;
	}

	if (!ctrl->p_new.p_u8) {
		dev_err(isp_dev->dev, "%s: ctrl pointer invalid\n", __func__);
		return -EINVAL;
	}

	pdata = event_pkg->data;

	mutex_lock(&device->event_shm.event_lock);

	isp_ctrl = (struct visp_ctrl *)pdata;
	isp_ctrl->cid = ctrl->id;
	isp_ctrl->size = ctrl->elem_size * ctrl->elems;

	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_G_CTRL;
	event_pkg->head.shm_fd = -1;  /* Not used with DMA-BUF */
	event_pkg->head.shm_size = device->event_shm.size;
	event_pkg->head.data_size = sizeof(struct visp_ctrl) + isp_ctrl->size;
	event_pkg->ack = 0;
	event_pkg->result = 0;

	ret = visp_subdev_post_event(device, event_pkg);

	if (ret == 0) {
		/* Bounds check to prevent reading past buffer */
		size_t available_size = device->event_shm.size -
					(sizeof(struct visp_event_pkg_head) + sizeof(struct visp_ctrl));
		size_t copy_size = isp_ctrl->size;
		if (copy_size > available_size) {
			dev_err(isp_dev->dev, "%s: ctrl data size %zu exceeds buffer size %zu\n",
				__func__, copy_size, available_size);
			ret = -EOVERFLOW;
		} else if (copy_size > ctrl->elem_size * ctrl->elems) {
			dev_err(isp_dev->dev, "%s: returned size %zu exceeds ctrl size %u\n",
				__func__, copy_size, ctrl->elem_size * ctrl->elems);
			ret = -EOVERFLOW;
		} else {
			memcpy(ctrl->p_new.p_u8, pdata + sizeof(struct visp_ctrl),
			       copy_size);
		}
	}

	mutex_unlock(&device->event_shm.event_lock);
	return ret;
}

int visp_l_fusa_event(struct visp_dev *isp_dev, int pad)
{
	struct visp_mimo_device *device =
		(struct visp_mimo_device *)ISP_DEV_EXTENDED_MIMO_VIDEO(isp_dev)->mimo_device;
	struct visp_event_pkg *event_pkg = device->event_shm.virt_addr;
	int ret = 0;
	int port = pad / MEDIA_ISP_PORT_PAD_COUNT;
	uint8_t *pdata;

	mutex_lock(&device->event_shm.event_lock);
	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_LOAD_FUSA;
	event_pkg->head.shm_fd = -1;
	event_pkg->head.shm_size = device->event_shm.size;
	event_pkg->head.data_size = 0;
	event_pkg->ack = 0;
	event_pkg->result = 0;

	pdata = event_pkg->data;

	memcpy(pdata, &isp_dev->isp_ports[port].fusa_json,
	       sizeof(isp_dev->isp_ports[port].fusa_json));
	pdata += sizeof(isp_dev->isp_ports[port].fusa_json);
	event_pkg->head.data_size += sizeof(isp_dev->isp_ports[port].fusa_json);

	ret = visp_subdev_post_event(device, event_pkg);
	if (ret != 0)
		pr_err("[FAIL] %s %d\n", __func__, __LINE__);
	mutex_unlock(&device->event_shm.event_lock);

	return ret;
}

int visp_stop_fusa_event(struct visp_dev *isp_dev, int pad)
{
	struct visp_mimo_device *device =
		(struct visp_mimo_device *)ISP_DEV_EXTENDED_MIMO_VIDEO(isp_dev)->mimo_device;
	struct visp_event_pkg *event_pkg = device->event_shm.virt_addr;
	int ret = 0;

	mutex_lock(&device->event_shm.event_lock);
	event_pkg->head.pad = pad;
	event_pkg->head.dev = isp_dev->id;
	event_pkg->head.eid = VISP_EVENT_STOP_FUSA;
	event_pkg->head.shm_fd = -1;
	event_pkg->head.shm_size = device->event_shm.size;
	event_pkg->head.data_size = 0;
	event_pkg->ack = 0;
	event_pkg->result = 0;

	ret = visp_subdev_post_event(device, event_pkg);
	if (ret != 0)
		pr_err("[FAIL] %s %d\n", __func__, __LINE__);

	mutex_unlock(&device->event_shm.event_lock);
	return ret;
}
