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
struct visp_subdev_dma_buf
{
	uint64_t pa;
	int size;
};


static bool visp_video_event_subscribed(struct isp_mimo  *visp_vdev,
                                uint32_t type, uint32_t id)
{
    struct v4l2_fh *fh;
    unsigned long flags;
    struct v4l2_subscribed_event *sev;
    bool subscribed = false;
    spin_lock_irqsave(&visp_vdev->video_dev.fh_lock, flags);

    list_for_each_entry(fh, &visp_vdev->video_dev.fh_list, list) {
        list_for_each_entry(sev, &fh->subscribed, list) {
            if (sev->type == type && sev->id == id) {
                subscribed = true;
	            pr_err(" %s %d\n", __func__, __LINE__);
                break;
            }
        }
        if (subscribed)
        {
            break;
        }
    }

    spin_unlock_irqrestore(&visp_vdev->video_dev.fh_lock, flags);

    return subscribed;
}

int visp_video_post_event(struct isp_mimo  *visp_vdev,
                        struct visp_event_pkg *event_pkg);
int visp_video_post_event(struct isp_mimo  *visp_vdev,
                        struct visp_event_pkg *event_pkg)
{
    struct v4l2_event event;
    int timeout_ms = 4000000;
    unsigned int i = 0;

    memset(&event, 0, sizeof(event));

    event.type   = VISP_VIDEO_DEAMON_EVENT;
	event.id     = event_pkg->head.eid;
    memcpy(event.u.data, &event_pkg->head, sizeof(event_pkg->head));

    if (!visp_video_event_subscribed(visp_vdev, event.type, event.id))
    {
        //return -EINVAL;
    }
    event_pkg->ack = 0;

    v4l2_event_queue(visp_vdev->subdev.devnode, &event);
       if (event_pkg->ack) {
        }

#if 1
    for (i = 0; i < timeout_ms; i++) {
        if (event_pkg->ack) {
            break;
        }
        usleep_range(5, 10);
    }
#endif

    if (event_pkg->ack == 0) {
        pr_err("%s post event %d time out\n",
            visp_vdev->video_dev.name, event.id);
        return -EIO;
    }

    if (event_pkg->result) {
        return -EINVAL;
    }

    return 0;
}

int visp_video_create_pipeline_event(struct isp_mimo *visp_vdev)
{
    struct visp_event_pkg *event_pkg = visp_vdev->event_shm.virt_addr;
    int ret;

	pr_err("%s %d\n", __func__, __LINE__);
    mutex_lock(&visp_vdev->event_shm.event_lock);

    event_pkg->head.eid = VISP_VEVENT_CREATE_PIPELINE;
    event_pkg->head.shm_addr = visp_vdev->event_shm.phy_addr;
    event_pkg->head.shm_size = visp_vdev->event_shm.size;
    event_pkg->head.data_size = 0;
    event_pkg->ack = 0;
    event_pkg->result = 0;

    ret = visp_video_post_event(visp_vdev, event_pkg);

    mutex_unlock(&visp_vdev->event_shm.event_lock);

    return ret;
}

int visp_video_destroy_pipeline_event(struct isp_mimo *visp_vdev)
{
    struct visp_event_pkg *event_pkg = visp_vdev->event_shm.virt_addr;
    int ret;

	pr_err("%s %d\n", __func__, __LINE__);
    mutex_lock(&visp_vdev->event_shm.event_lock);

    event_pkg->head.eid = VISP_VEVENT_DESTROY_PIPELINE;
    event_pkg->head.shm_addr = visp_vdev->event_shm.phy_addr;
    event_pkg->head.shm_size = visp_vdev->event_shm.size;
    event_pkg->head.data_size = 0;
    event_pkg->ack = 0;
    event_pkg->result = 0;

    ret = visp_video_post_event(visp_vdev, event_pkg);

    mutex_unlock(&visp_vdev->event_shm.event_lock);

	pr_err("%s %d\n", __func__, __LINE__);
    return ret;
}

int visp_l_calib_event(struct isp_mimo *isp_dev, int pad, int event)
{

    struct visp_event_pkg *event_pkg = isp_dev->event_shm.virt_addr;
	int ret = 0;
	uint8_t *pdata = event_pkg->data;
	int Port = pad / MEDIA_ISP_PORT_PAD_COUNT;

	mutex_lock(&isp_dev->event_shm.event_lock);
	event_pkg->head.eid = event;
	event_pkg->head.shm_addr = isp_dev->event_shm.phy_addr;
	event_pkg->head.shm_size = isp_dev->event_shm.size;
	event_pkg->head.data_size = 0;
	event_pkg->ack = 0;
	event_pkg->result = 0;

	memcpy(pdata, &Port, sizeof(uint32_t));
	pdata += sizeof(uint32_t);
	event_pkg->head.data_size += sizeof(uint32_t);

	struct  visp_subdev_dma_buf dma_buf ;
	dma_buf.pa = isp_dev->reserve_mem.pa;
	dma_buf.size = isp_dev->reserve_mem.size;
	memcpy(pdata, &dma_buf, sizeof(struct visp_subdev_dma_buf));
	pdata += sizeof(struct visp_subdev_dma_buf);

	event_pkg->head.data_size += sizeof(struct visp_subdev_dma_buf);

	ret = visp_video_post_event(isp_dev, event_pkg);
	if (ret != 0)
	{
		pr_err( "[FAIL] %s %d\n", __func__, __LINE__);
	}
	mutex_unlock(&isp_dev->event_shm.event_lock);

	return ret;
}
