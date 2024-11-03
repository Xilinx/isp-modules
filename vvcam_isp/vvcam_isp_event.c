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
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <linux/delay.h>
#include "vvcam_isp_event.h"
#include "vvcam_isp_driver.h"

static bool vvcam_isp_event_subscribed(struct v4l2_subdev *sd,
                                uint32_t type, uint32_t id)
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

int vvcam_isp_post_event(struct v4l2_subdev *sd, struct vvcam_isp_event_pkg *event_pkg)
{
//return 0;
    struct v4l2_event event;
    int timeout_ms = 6000000;
    int i = 0;

    memset(&event, 0, sizeof(event));

    event.type   = VVCAM_ISP_DEAMON_EVENT;
	event.id     = event_pkg->head.eid;
    memcpy(event.u.data, &event_pkg->head, sizeof(event_pkg->head));

    if (!vvcam_isp_event_subscribed(sd, event.type, event.id)) {
        dev_err(sd->dev, "post event %d not subscribed\n", event.id);
        return -EINVAL;
    }

    v4l2_event_queue(sd->devnode, &event);

    for (i = 0; i < timeout_ms; i++) {
        if (event_pkg->ack) {
            break;
        }
        usleep_range(5, 10);
	if(i>5000000)
	{
		i=0;
	}
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
typedef struct CamDeviceContext_s {

    uint32_t                    ispHwId;
    uint32_t                    ispVtId;
    uint32_t                    instanceId;
    uint32_t                     cookie;
} CamDeviceContext_t;
int vvcam_isp_l_calib_event(struct vvcam_isp_dev *isp_dev, int pad)
{
    struct vvcam_isp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
    int ret = 0;

    mutex_lock(&isp_dev->event_shm.event_lock);
    event_pkg->head.pad = pad;
    event_pkg->head.dev = isp_dev->id;
    event_pkg->head.eid = VVCAM_ISP_EVENT_LOAD_CALIB;

    event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
    event_pkg->head.shm_size = isp_dev->event_shm.size;
    event_pkg->head.data_size = 0;
    event_pkg->head.data_size = sizeof(MediaIspPortAttr);
    event_pkg->ack = 0;
    event_pkg->result = 0;
    memcpy(event_pkg->data, &isp_dev->IspPorts[0], sizeof(MediaIspPortAttr));
    ret = vvcam_isp_post_event(&isp_dev->sd, event_pkg);

    mutex_unlock(&isp_dev->event_shm.event_lock);
    return ret;
}

int vvcam_isp_l_json_event(struct vvcam_isp_dev *isp_dev, int pad)
{
    struct vvcam_isp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
    int ret = 0;

    mutex_lock(&isp_dev->event_shm.event_lock);
    event_pkg->head.pad = pad;
    event_pkg->head.dev = isp_dev->id;
        event_pkg->head.eid = VVCAM_ISP_EVENT_LOAD_JSON;

    event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
    event_pkg->head.shm_size = isp_dev->event_shm.size;
    event_pkg->head.data_size = 0;
    event_pkg->head.data_size = sizeof(MediaIspPortAttr);
    event_pkg->ack = 0;
    event_pkg->result = 0;
    memcpy(event_pkg->data, &isp_dev->IspPorts[0], sizeof(MediaIspPortAttr));
	    ret = vvcam_isp_post_event(&isp_dev->sd, event_pkg);


    mutex_unlock(&isp_dev->event_shm.event_lock);
    return ret;
}


#if 0
int vvcam_isp_s_stream_event(struct vvcam_isp_dev *isp_dev, int pad, uint32_t status)
{
    struct vvcam_isp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
    int ret = 0;

    mutex_lock(&isp_dev->event_shm.event_lock);
    event_pkg->head.pad = pad;
    event_pkg->head.dev = isp_dev->id;
    if (status) {
        event_pkg->head.eid = VVCAM_ISP_EVENT_STREAMOFF;
    }

    event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
    event_pkg->head.shm_size = isp_dev->event_shm.size;
    event_pkg->head.data_size = 0;
    event_pkg->head.data_size = sizeof(MediaIspPortAttr);
    event_pkg->ack = 0;
    event_pkg->result = 0;
    memcpy(event_pkg->data, &isp_dev->IspPorts[0], sizeof(MediaIspPortAttr));
CamDeviceContext_t *pCamDevCtx1 = (CamDeviceContext_t *)&isp_dev->IspPorts[0].CamDeviceHandle;
CamDeviceContext_t *pCamDevCtx2 = (CamDeviceContext_t *)&isp_dev->IspPorts[1].CamDeviceHandle;
    ret = vvcam_isp_post_event(&isp_dev->sd, event_pkg);

    mutex_unlock(&isp_dev->event_shm.event_lock);
    return ret;
}
#endif
int vvcam_isp_s_ctrl_event(struct vvcam_isp_dev *isp_dev,
            int pad, struct v4l2_ctrl *ctrl)
{
    struct vvcam_isp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
    int ret;
    struct vvcam_isp_ctrl *isp_ctrl;

    mutex_lock(&isp_dev->event_shm.event_lock);

    isp_ctrl = (struct vvcam_isp_ctrl *)event_pkg->data;
    isp_ctrl->cid = ctrl->id;
    isp_ctrl->size = ctrl->elem_size * ctrl->elems;
    memcpy(isp_ctrl->data, ctrl->p_new.p_u8, isp_ctrl->size);

    event_pkg->head.pad = pad;
    event_pkg->head.dev = isp_dev->id;
    event_pkg->head.eid = VVCAM_ISP_EVENT_S_CTRL;
    event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
    event_pkg->head.shm_size = isp_dev->event_shm.size;
    event_pkg->head.data_size = sizeof(isp_ctrl) + isp_ctrl->size;
    event_pkg->ack = 0;
    event_pkg->result = 0;

    ret = vvcam_isp_post_event(&isp_dev->sd, event_pkg);

    mutex_unlock(&isp_dev->event_shm.event_lock);

    return ret;
}

int vvcam_isp_g_ctrl_event(struct vvcam_isp_dev *isp_dev,
            int pad, struct v4l2_ctrl *ctrl)
{
    struct vvcam_isp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
    int ret = 0;
    struct vvcam_isp_ctrl *isp_ctrl;

    mutex_lock(&isp_dev->event_shm.event_lock);

    isp_ctrl = (struct vvcam_isp_ctrl *)event_pkg->data;
    isp_ctrl->cid = ctrl->id;
    isp_ctrl->size = ctrl->elem_size * ctrl->elems;

    event_pkg->head.pad = pad;
    event_pkg->head.dev = isp_dev->id;
    event_pkg->head.eid = VVCAM_ISP_EVENT_G_CTRL;
    event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
    event_pkg->head.shm_size = isp_dev->event_shm.size;
    event_pkg->head.data_size = sizeof(isp_ctrl) + isp_ctrl->size;
    event_pkg->ack = 0;
    event_pkg->result = 0;

    ret = vvcam_isp_post_event(&isp_dev->sd, event_pkg);

    if (ret == 0) {
         memcpy(ctrl->p_new.p_u8, event_pkg->data + sizeof(isp_ctrl),
            isp_ctrl->size);
    }

    mutex_unlock(&isp_dev->event_shm.event_lock);

    return ret;
}

