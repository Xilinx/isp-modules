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


#include "cam_device.h"
#include "sensor_cmd.h"
#include <linux/delay.h>
#include "visp_driver.h"
#include <linux/string.h>
#include "kmbox.h"
#include "visp_common.h"

RESULT VsiCamDeviceUnRegisterAeLib
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;
    payload_packet *packet=NULL;
    uint8_t *p_data=NULL;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    packet= kzalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
    	pr_err("FAILED TO KZALLOC %s %d\n",__func__,__LINE__);
	    return -ENOMEM;
    } 

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

     mutex_lock(&isp_dev->mlock);
    result = Send_Command( RPU_2_APU_MB_CMD_UNREGISTER_AELIB, packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        mutex_unlock(&isp_dev->mlock);
        kfree(packet);
        return result;
    }
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);

    kfree(packet);


	return result;
}

RESULT VsiCamDeviceAeDisable
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    uint8_t *p_data=NULL;
    payload_packet *packet=NULL;


    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    packet= kzalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
    	pr_err("FAILED TO KZALLOC %s %d\n",__func__,__LINE__);
	    return -ENOMEM;
    } 

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

    if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
    	return RET_OUTOFRANGE;
    }

    mutex_lock(&isp_dev->mlock);
    result = Send_Command( APU_2_RPU_MB_CMD_AE_DISABLE , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);
    kfree(packet);

	return result;
}

