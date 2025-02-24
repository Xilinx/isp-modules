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

int vvcam_isp_post_event(struct v4l2_subdev *sd, struct vvcam_isp_event_pkg *event_pkg);
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
    pr_err("RKC_ISPDRV Status event timeout  %s %d\n",__func__,__LINE__);
        return -EIO;
    }

    if (event_pkg->result) {
        dev_err(sd->dev, "post event %d return error\n", event.id);
    pr_err("RKC_ISPDRV STATUS EVENT FAIL %s %d\n",__func__,__LINE__);
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

typedef struct test
{
int ID2;
int ID;
}test_t;

int vvcam_isp_l_calib_event(struct vvcam_isp_dev *isp_dev, int pad)
{
    struct vvcam_isp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
    int ret = 0;
    uint8_t *pdata=event_pkg->data;
    int Port = pad / MEDIA_ISP_PORT_PAD_COUNT;
    CamDeviceHandle_t *readback = NULL;
    MediaIspSensorInfo *readback_1 = NULL;
	MediaIspPortAttr * IspPort = NULL;//&(isp_dev->IspPorts[Port]);

    mutex_lock(&isp_dev->event_shm.event_lock);
    event_pkg->head.pad = pad;
    event_pkg->head.dev = isp_dev->id;
    event_pkg->head.eid = VVCAM_ISP_EVENT_LOAD_CALIB;

    event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
    event_pkg->head.shm_size = isp_dev->event_shm.size;
    event_pkg->head.data_size = 0;
   // event_pkg->head.data_size = sizeof(MediaIspPortAttr);
    event_pkg->ack = 0;
    event_pkg->result = 0;
    //pr_err("%s %d %s \n",__func__,__LINE__,isp_dev->IspPorts[0].SensorInfo.Name);

    memcpy(pdata, &Port, sizeof(uint32_t));
    pdata+=sizeof(uint32_t);
    event_pkg->head.data_size += sizeof(uint32_t);

    memcpy(pdata,isp_dev->IspPorts[Port].CamDeviceHandle,sizeof(CamDeviceContext_t));
    dev_info(isp_dev->dev,"%s %d %p %d \n",__func__,__LINE__,((CamDeviceContext_t *)isp_dev->IspPorts[Port].CamDeviceHandle),((CamDeviceContext_t *)pdata)->cookie);
    pdata+=sizeof(CamDeviceContext_t);
    event_pkg->head.data_size += sizeof(CamDeviceContext_t);

    memcpy(pdata, &(isp_dev->IspPorts[Port].SensorInfo), sizeof(MediaIspSensorInfo));
    pdata+=sizeof(MediaIspSensorInfo);
    event_pkg->head.data_size +=sizeof(MediaIspSensorInfo);

    readback = ( CamDeviceHandle_t  *)(event_pkg->data);
//

	IspPort = &(isp_dev->IspPorts[Port]);
    dev_err(isp_dev->dev , "LENGTHS= %lu %lu %lu |%s%s%s|",strlen(IspPort->SensorInfo.AutoJson),strlen(IspPort->SensorInfo.ManuJson),strlen(IspPort->SensorInfo.CalibXml)
, IspPort->SensorInfo.AutoJson , IspPort->SensorInfo.ManuJson , IspPort->SensorInfo.CalibXml );
//

    readback_1 = (MediaIspSensorInfo *)(event_pkg->data +sizeof(uint32_t));
    dev_info(isp_dev->dev,"%s %d %ld %s \n",__func__,__LINE__,sizeof(MediaIspPortAttr),readback_1->Name);

    dev_info(isp_dev->dev, "RKC_ISPDRV %s %d Post Event \n",__func__,__LINE__);
    ret = vvcam_isp_post_event(&isp_dev->sd, event_pkg);
    if(ret!=0)
    {
        dev_info(isp_dev->dev, "[RKC-EVENT_FAIL] %s %d\n",__func__,__LINE__);	
        return ret;
    }
    dev_info(isp_dev->dev, "RKC_ISPDRV %s %d Ack got for Event \n",__func__,__LINE__);
    mutex_unlock(&isp_dev->event_shm.event_lock);
    return ret;
}

int vvcam_isp_l_json_event(struct vvcam_isp_dev *isp_dev, int pad)
{
    struct vvcam_isp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
    int ret = 0;
    int Port = pad / MEDIA_ISP_PORT_PAD_COUNT;
    uint8_t *pdata = NULL;

    mutex_lock(&isp_dev->event_shm.event_lock);
    event_pkg->head.pad = pad;
    event_pkg->head.dev = isp_dev->id;
    event_pkg->head.eid = VVCAM_ISP_EVENT_LOAD_JSON;

    event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
    event_pkg->head.shm_size = isp_dev->event_shm.size;
    event_pkg->head.data_size = 0;
   // event_pkg->head.data_size = sizeof(MediaIspPortAttr);
    event_pkg->ack = 0;
    event_pkg->result = 0;

    pdata=event_pkg->data;

    memcpy(pdata, &Port, sizeof(uint32_t));
    pdata+=sizeof(uint32_t);
    event_pkg->head.data_size += sizeof(uint32_t);

	dev_err(isp_dev->dev , "RKC_ISPDRV %s %d Post Event \n",__func__,__LINE__);
	ret = vvcam_isp_post_event(&isp_dev->sd, event_pkg);
    if(ret!=0)
    {
     dev_err(isp_dev->dev , "[RKC-EVENT_FAIL] %s %d\n",__func__,__LINE__);	
        return ret;
    }
	dev_err(isp_dev->dev , "RKC_ISPDRV %s %d Ack got for Event \n",__func__,__LINE__);

    mutex_unlock(&isp_dev->event_shm.event_lock);
    return ret;
}


#if 0
int vvcam_isp_s_stream_event(struct vvcam_isp_dev *isp_dev, int pad, uint32_t status)
{
    struct vvcam_isp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
    int ret = 0;

pr_err("RKC_ISPDRV %s %d\n",__func__,__LINE__);
    mutex_lock(&isp_dev->event_shm.event_lock);
    event_pkg->head.pad = pad;
    event_pkg->head.dev = isp_dev->id;
    if (status) {
pr_err("RKC_ISPDRV %s %d\n",__func__,__LINE__);
        event_pkg->head.eid = VVCAM_ISP_EVENT_STREAMON;
    } else {
pr_err("RKC_ISPDRV %s %d\n",__func__,__LINE__);
        event_pkg->head.eid = VVCAM_ISP_EVENT_STREAMOFF;
    }

    event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
    event_pkg->head.shm_size = isp_dev->event_shm.size;
    event_pkg->head.data_size = 0;
    event_pkg->head.data_size = sizeof(MediaIspPortAttr);
    event_pkg->ack = 0;
    event_pkg->result = 0;
	pr_err("%s %d %s \n",__func__,__LINE__,isp_dev->IspPorts[0].SensorInfo.Name);
    memcpy(event_pkg->data, &isp_dev->IspPorts[0], sizeof(MediaIspPortAttr));
CamDeviceContext_t *pCamDevCtx1 = (CamDeviceContext_t *)&isp_dev->IspPorts[0].CamDeviceHandle;
CamDeviceContext_t *pCamDevCtx2 = (CamDeviceContext_t *)&isp_dev->IspPorts[1].CamDeviceHandle;
pr_err("%s %d cookie = %d VtID = %d  HWID=%d instANCE_ID = %d \n",__func__,__LINE__,pCamDevCtx2->cookie,pCamDevCtx2->ispVtId,pCamDevCtx2->ispHwId,pCamDevCtx1->instanceId);
pr_err("%s %d cookie = %d VtID = %d  HWID=%d instANCE_ID = %d \n",__func__,__LINE__,pCamDevCtx1->cookie,pCamDevCtx1->ispVtId,pCamDevCtx1->ispHwId,pCamDevCtx1->instanceId);
pr_err("RKC_ISPDRV %s %d Post Event \n",__func__,__LINE__);
    ret = vvcam_isp_post_event(&isp_dev->sd, event_pkg);
pr_err("RKC_ISPDRV %s %d Ack got for Event \n",__func__,__LINE__);

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

int vvcam_isp_s_interval_event(struct vvcam_isp_dev *isp_dev, int pad, struct v4l2_fract *timeperframe)
{
    struct vvcam_isp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
    int ret;

    mutex_lock(&isp_dev->event_shm.event_lock);
    event_pkg->head.pad = pad;
    event_pkg->head.dev = isp_dev->id;
    event_pkg->head.eid = VVCAM_ISP_EVENT_S_INTERVAL;
    event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
    event_pkg->head.shm_size = isp_dev->event_shm.size;
    event_pkg->head.data_size = sizeof(struct v4l2_fract);
    event_pkg->ack = 0;
    event_pkg->result = 0;
    memcpy(event_pkg->data, timeperframe, sizeof(struct v4l2_fract));

    ret = vvcam_isp_post_event(&isp_dev->sd, event_pkg);

    mutex_unlock(&isp_dev->event_shm.event_lock);

    return ret;
}
