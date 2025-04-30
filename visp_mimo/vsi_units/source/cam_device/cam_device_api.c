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
#include "cam_device_api.h"
#include "sensor_cmd.h"
#include <linux/delay.h>
#include "visp_driver.h"
#include "visp_app.h"
#include "visp_common.h"
#include <linux/string.h>
#include "kmbox.h"
uint8_t  *cam_load_calib = NULL; //[160010] __attribute__((section(".cam_load_calib")));

RESULT VsiCamDeviceCreate
(
    struct visp_dev *isp_dev,
    CamDeviceConfig_t *pCamConfig,
    CamDeviceHandle_t *pHandleCamDevice
)
{
    RESULT result = RET_SUCCESS;
    uint32_t hwId = pCamConfig->ispHwId;
    uint32_t virtualId = 0;
    uint32_t instanceId = 0;
    uint8_t *p_data = NULL;
    CamDeviceHandle_t hCamDevice = NULL;
    CamDeviceContext_t *pCamDevCtx=NULL; 
    payload_packet *packet = NULL;

    if (NULL == pCamConfig || NULL == pHandleCamDevice) {
	dev_err(isp_dev->dev , "DRV %s %d\n",__func__,__LINE__);
        return RET_NULL_POINTER;
    }

    /*
     * AMD : Multi-Core Changes
     * Verifying Multi-core
     */

#ifdef DEBUG_FLAG
#else
    if (hwId >= CAMDEV_HARDWARE_ID_MAX) {
        return RET_OUTOFRANGE;
    }
#endif
    CamDeviceIspcoreInit();
    /*Get Cam Device Handle*/
    result = CamDeviceRequestInstance(hwId, &hCamDevice, &virtualId);
    if(RET_SUCCESS != result || NULL == hCamDevice) {
        return RET_FAILURE;
    }
    memset(hCamDevice, 0, sizeof(CamDeviceContext_t));

    /*Mapping instance id for a single device*/
    result = CamDeviceInstanceIdMapping(hwId, virtualId, &instanceId);
    if(RET_SUCCESS != result) {
        return RET_UNSUPPORT_ID;
    }

    pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    pCamDevCtx->ispHwId =hwId;
    pCamDevCtx->ispVtId =virtualId;
    pCamDevCtx->instanceId = instanceId;
    pCamDevCtx->cookie = 0;
    packet= kzalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
        dev_err(isp_dev->dev , "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
        return -ENOMEM;
    } 

    packet->cookie = 0;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;
    memcpy(p_data, &instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pCamConfig, sizeof(CamDeviceConfig_t));
    packet->payload_size += sizeof(CamDeviceConfig_t);

	if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
    	return RET_OUTOFRANGE;
    }

    mutex_lock(&isp_dev->mlock);
    result = Send_Command (APU_2_RPU_MB_CMD_CREATE_INSTANCE,packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        loge("Mailbox Send message failed %s %d\n",__func__,__LINE__);
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

    *pHandleCamDevice = hCamDevice;

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);

    kfree(packet);
    return result;
}

EXPORT_SYMBOL(VsiCamDeviceCreate);

RESULT VsiCamDeviceDestroy
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;
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
	    dev_err(isp_dev->dev , "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
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
    ret = Send_Command (APU_2_RPU_MB_CMD_DESTORY , packet, packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
    if(0 != ret) {
        mutex_unlock(&isp_dev->mlock);
    	kfree(packet);
        return ret;
    }
	mbox_send_message(isp_dev->tx_chan,NULL);

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);

    kfree(packet);
	return result;
}
EXPORT_SYMBOL(VsiCamDeviceDestroy);

RESULT VsiCamDeviceSetOutFormat
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeOutPathType_t path,
    CamDevicePipeOutFmt_t *pFmt
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;
    payload_packet *packet=NULL;
    uint8_t *p_data=NULL; 


    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (pCamDevCtx==NULL || pFmt==NULL)
    {
        dev_err(isp_dev->dev , "ISP_APP %s %d  \n",__func__,__LINE__);
        return 0;
    }
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pFmt) {
        return (RET_NULL_POINTER);
    }
   pCamDevCtx->cookie ++;

    packet = kzalloc(sizeof(payload_packet),GFP_KERNEL);
    if(!packet)
    {
        dev_err(isp_dev->dev , "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
        return -ENOMEM;
    } 

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &path, sizeof(CamDevicePipeOutPathType_t));
    packet->payload_size += sizeof(CamDevicePipeOutPathType_t);
    p_data += sizeof(CamDevicePipeOutPathType_t);

    memcpy(p_data, pFmt, sizeof(CamDevicePipeOutFmt_t));
    packet->payload_size += sizeof(CamDevicePipeOutFmt_t);

    if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
        return RET_OUTOFRANGE;
    }
    mutex_lock(&isp_dev->mlock);
    ret = Send_Command (APU_2_RPU_MB_CMD_SET_OUT_FORMAT, packet, packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
    if(0 != ret) {
        loge("Failed to send_command %s %d\n",__func__,__LINE__);
        kfree(packet);
        return ret;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);


    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);

	kfree(packet);
	return result;
}

RESULT VsiCamDeviceSetInFormat
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeInPathType_t path,
    CamDevicePipeInFmt_t *pFmt
)
{
    int ret = 0;
    RESULT result = RET_SUCCESS;
    uint8_t *p_data=NULL; 
    payload_packet *packet=NULL;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pFmt) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    packet= kzalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
    	dev_err(isp_dev->dev , "FAILED TO KZALLOC %s %d\n",__func__,__LINE__);
	    return -ENOMEM;
    } 

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &path, sizeof(CamDevicePipeInPathType_t));
    p_data += sizeof(CamDevicePipeInPathType_t);
    packet->payload_size += sizeof(CamDevicePipeInPathType_t);

    memcpy(p_data, pFmt, sizeof(CamDevicePipeInFmt_t));
    packet->payload_size += sizeof(CamDevicePipeInFmt_t);

    if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
    	return RET_OUTOFRANGE;
    }

     mutex_lock(&isp_dev->mlock);
    ret = Send_Command( APU_2_RPU_MB_CMD_SET_IN_FORMAT, packet, packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
    if(0 != result) {
        mutex_unlock(&isp_dev->mlock);
        kfree(packet);
        return ret;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);

    kfree(packet);



	return result;
}

RESULT VsiCamDeviceConnectCamera
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    const CamDevicePipeSubmoduleCtrl_u *pSubCtrl
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    payload_packet *packet=NULL;
    uint8_t *p_data=NULL; 
    int ret=0;

	if (pCamDevCtx==NULL || pSubCtrl==NULL)
	{
		dev_err(isp_dev->dev , "ISP_APP NULL HANDLE %s %d  \n",__func__,__LINE__);
        return (RET_WRONG_HANDLE);
	}
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pSubCtrl) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    packet = kzalloc(sizeof(payload_packet),GFP_KERNEL);
    if(!packet)
    {
        dev_err(isp_dev->dev , "FAILED TO KZALLOC %s %d\n",__func__,__LINE__);
        return -ENOMEM;
    } 

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;

    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, pSubCtrl, sizeof(CamDevicePipeSubmoduleCtrl_u));
    packet->payload_size += sizeof(CamDevicePipeSubmoduleCtrl_u);

    if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
        return RET_OUTOFRANGE;
    }
#if 1
    mutex_lock(&isp_dev->mlock);
    ret = Send_Command(APU_2_RPU_MB_CMD_CONNECT_CAMERA , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
        kfree(packet);
        mutex_unlock(&isp_dev->mlock);
        return ret;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);
    kfree(packet);

	return result;
}

RESULT VsiCamDeviceDisconnectCamera
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    int ret=0;
    uint8_t *p_data=NULL;
    payload_packet *packet=NULL;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    packet = kzalloc(sizeof(payload_packet),GFP_KERNEL);
    if(!packet)
    {
        dev_err(isp_dev->dev , "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
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
    ret = Send_Command(APU_2_RPU_MB_CMD_DISCONNECT_CAMERA , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
        kfree(packet);
        mutex_unlock(&isp_dev->mlock);
        return ret;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);
    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);
    kfree(packet);
	return result;

}
RESULT VsiCamDeviceStartStreaming
(
    CamDeviceConfig_t *pCamConfig,
    CamDeviceHandle_t hCamDevice,
    uint32_t frames                //0-continue
);

RESULT VsiCamDeviceStartStreaming
(
    CamDeviceConfig_t *pCamConfig,
    CamDeviceHandle_t hCamDevice,
    uint32_t frames                //0-continue
)
{
    RESULT result = RET_SUCCESS;

    payload_packet *packet=NULL;
    uint8_t *p_data=NULL;
    struct visp_dev *isp_dev = pCamConfig->isp_dev;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    packet = kzalloc(sizeof(payload_packet),GFP_KERNEL);
    if(!packet)
    {
        dev_err(isp_dev->dev , "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
        return -ENOMEM;
    } 

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &frames, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

    if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
    	return RET_OUTOFRANGE;
    }
    mutex_lock(&isp_dev->mlock);
    result = Send_Command(APU_2_RPU_MB_CMD_START_STREAMING , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        loge("Mailbox FAILed to send messge %s %d\n",__func__,__LINE__);
        kfree(packet);
        mutex_unlock(&isp_dev->mlock);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);
    kfree(packet);
	return result;
}

RESULT VsiCamDeviceSetPathStreaming
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    CamDevicePathStreamingCfg_t *pConfig
){
    RESULT result = RET_SUCCESS;
    uint8_t *p_data=NULL;
    payload_packet *packet=NULL;
    int path_enable=-1;
    
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pConfig) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    packet = kzalloc(sizeof(payload_packet),GFP_KERNEL);
    if(!packet)
    {
    	dev_err(isp_dev->dev , "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
	    return -ENOMEM;
    } 

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;

    *(packet->payload) =pCamDevCtx->instanceId;//(int *)0x0;
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    path_enable=pConfig->outPathEnable;
    *(p_data) = path_enable;//(int *)pConfig->outPathEnable;//0x1;

    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    
    if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
    	return RET_OUTOFRANGE;
    }
    mutex_lock(&isp_dev->mlock);
    result = Send_Command (APU_2_RPU_MB_CMD_SET_PATH_STREAMING, packet, packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
    if(0 != result) {
        mutex_unlock(&isp_dev->mlock);
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);
    kfree(packet);
	return result;
}

RESULT VsiCamDeviceGetPathStreaming
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    CamDevicePathStreamingCfg_t *pConfig
){
    RESULT result = RET_SUCCESS;
    uint8_t *p_data=NULL;
    payload_packet *packet=NULL;
    uint8_t __result;
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pConfig) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    packet = kzalloc(sizeof(payload_packet),GFP_KERNEL);
    if(!packet)
    {
    	dev_err(isp_dev->dev , "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
	    return -ENOMEM;
    } 

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;

    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    memcpy(p_data, pConfig, sizeof(CamDevicePathStreamingCfg_t));
    packet->payload_size += sizeof(CamDevicePathStreamingCfg_t);

	if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
    	return RET_OUTOFRANGE;
    }

    mutex_lock(&isp_dev->mlock);
    result = Send_Command( APU_2_RPU_MB_CMD_GET_PATH_STREAMING , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(result != 0) {
        mutex_unlock(&isp_dev->mlock);
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

    __result = xlnx_mbox_apu_wait_for_data(isp_dev,packet->payload);
	if(__result)
    {
        dev_err(isp_dev->dev , "FAILED TO SET PATH STREAMING\n");
        mutex_unlock(&isp_dev->mlock);
        kfree(packet);
        return -1;
    }
	memcpy(pConfig, p_data, sizeof(CamDevicePathStreamingCfg_t));
    mutex_unlock(&isp_dev->mlock);
    kfree(packet);
	return result;
}

RESULT VsiCamDeviceGetHardwareId
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pHwId
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    if (NULL == pCamDevCtx) {
        //TRACE(CAM_DEV_API_ERR, " %s: init Cam Device Context firstly!\n", __func__);
        return RET_WRONG_HANDLE;
    }
    if (NULL == pHwId) {
        //TRACE(CAM_DEV_API_ERR, " %s: invalid null pointer!\n", __func__);
        return RET_NULL_POINTER;
    }
    if (pCamDevCtx->ispHwId > CAMDEV_HARDWARE_ID_MAX) {
        return RET_OUTOFRANGE;
    }else {
       *pHwId = pCamDevCtx->ispHwId;
        return RET_SUCCESS;
    }
}

RESULT VsiCamDeviceAllocResMemory
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t   hCamDevice,
    uint32_t            byteSize,
    uint32_t           *pBaseAddress,
    void              **pIplAddress
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    RESULT result = RET_SUCCESS;
    payload_packet *packet=NULL;
    uint8_t *p_data = NULL;
    uint8_t __result ;
    if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie ++;

    packet = kzalloc(sizeof(payload_packet),GFP_KERNEL);
    if(!packet)
    {
    	dev_err(isp_dev->dev , "FAILED TO KZALLOC %s %d\n",__func__,__LINE__);
	    return -ENOMEM;
    } 
	
	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &byteSize, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	p_data += sizeof(uint32_t);
	memcpy(p_data, pBaseAddress, sizeof(uint32_t));
	packet->payload_size += 2 * sizeof(uint32_t);
    dev_info(isp_dev->dev , "[ISP] %s %d %d %p\n",__func__,__LINE__,byteSize,pBaseAddress);

	if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
    	return RET_OUTOFRANGE;
    }

    mutex_lock(&isp_dev->mlock);
    result = Send_Command( APU_2_RPU_MB_CMD_ALLOC_RES_MEMORY , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        mutex_unlock(&isp_dev->mlock);
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);
 
    __result = xlnx_mbox_apu_wait_for_data(isp_dev,packet->payload);
    if(__result)
    {
        mutex_unlock(&isp_dev->mlock);
        kfree(packet);
        dev_err(isp_dev->dev , "FAILED TO ALLOC RES MEMORY\n");
        return -1;
    }

    dev_info(isp_dev->dev , "[ISP] %s %d %x %x\n",__func__,__LINE__,*p_data,*(p_data+sizeof(uint32_t)));
    memcpy(pBaseAddress, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	memcpy(pIplAddress, p_data, sizeof(uint32_t));

    mutex_unlock(&isp_dev->mlock);

    kfree(packet);
    return result;
}

RESULT VsiCamDeviceFreeResMemory
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t   hCamDevice,
    uint32_t            baseAddress
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    RESULT result = RET_SUCCESS;
    uint8_t *p_data=NULL;
	payload_packet *packet=NULL;

    if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie ++;

    packet = kzalloc(sizeof(payload_packet),GFP_KERNEL);
    if(!packet)
    {
        dev_err(isp_dev->dev , "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
        return -ENOMEM;
    } 

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;

	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &baseAddress, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

    if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
    	return RET_OUTOFRANGE;
    }
#if 1
    mutex_lock(&isp_dev->mlock);
    result = Send_Command( APU_2_RPU_MB_CMD_FREE_RES_MEMORY , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        mutex_unlock(&isp_dev->mlock);
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);
    kfree(packet);
    return RET_SUCCESS;
}

