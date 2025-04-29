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


#include "cam_device_sensor_api.h"
#include "cam_device_api.h"
#include "cam_device.h"
#include "sensor_cmd.h"
#include "isi_iss.h"
#include <linux/slab.h>
#include <linux/string.h>
#include "kmbox.h"
#include "vvcam_isp_driver.h"

#define SENSOR_NAME_LEN 20

RESULT VsiCamDeviceSensorOpen
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice,
    uint32_t          modeIndex
)
{
    RESULT result = RET_SUCCESS;
    payload_packet *packet = NULL;
    uint8_t *p_data = NULL; 

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
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    memcpy(p_data, &modeIndex, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

    if(packet->payload_size > MAX_ITEM)
    {
        dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",packet->payload_size , MAX_ITEM);
        kfree(packet);
    	return RET_OUTOFRANGE;
    }
#if 1
    mutex_lock(&isp_dev->mlock);
    result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_OPEN , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);

    kfree(packet);

    return result;
}

RESULT VsiCamDeviceSensorDrvHandleRegister
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice,
	const CamDeviceSensorDrvCfg_t *pSensorDrv
)
{
    RESULT result = RET_SUCCESS;
    payload_packet *packet = NULL;
    uint8_t *p_data = NULL;
    IsiCamDrvConfigMbox_t* pcamcfg = NULL;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx || NULL == pSensorDrv) {
        return (RET_WRONG_HANDLE);
    }

    pCamDevCtx->cookie ++;
    //pCamDevCtx->cookie = 44;

    packet= kzalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
    	pr_err("FAILED TO KZALLOC %s %d\n",__func__,__LINE__);
	    return -ENOMEM;
    } 

    packet->cookie       = pCamDevCtx->cookie;
    packet->type         = CMD;
    packet->payload_size = 0;

    pcamcfg = (IsiCamDrvConfigMbox_t *)(pSensorDrv->sensorDrvHandle);

	// printf("pSensorDrv->sensorDrvHandle: %x.\r\n", pSensorDrv->sensorDrvHandle);
	dev_err(isp_dev->dev , "pSensorDrv->sensorDevId: %d.\r\n", pSensorDrv->sensorDevId);
	dev_err(isp_dev->dev ,"pcamcfg->cameraDriverID: %x.\r\n", pcamcfg->cameraDriverID);
	dev_err(isp_dev->dev ,"pcamcfg->pIsiGetSensorIss: %x.\r\n", pcamcfg->pIsiGetSensorIss);
	pcamcfg->instanceId = pCamDevCtx->instanceId;    
	p_data = packet->payload;

    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    memcpy(p_data, pSensorDrv->sensorDrvHandle, sizeof(IsiCamDrvConfigMbox_t));
    p_data += sizeof(IsiCamDrvConfigMbox_t);
    packet->payload_size += sizeof(IsiCamDrvConfigMbox_t);

    memcpy(p_data, &pSensorDrv->sensorDevId, sizeof(uint32_t));
    packet->payload_size +=  sizeof(uint32_t);

	if(packet->payload_size > MAX_ITEM)
    {
        dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",packet->payload_size , MAX_ITEM);
        kfree(packet);
    	return RET_OUTOFRANGE;
    }
#if 1
    mutex_lock(&isp_dev->mlock);
    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_REG,packet, packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        kfree(packet);
        return result;
    }
    pr_err("%s %d\n",__func__,__LINE__);
    mbox_send_message(isp_dev->tx_chan,NULL);
    pr_err("%s %d\n",__func__,__LINE__);

#endif

    pr_err("%s %d\n",__func__,__LINE__);
    xlnx_mbox_apu_wait_for_ack(isp_dev);
    pr_err("%s %d\n",__func__,__LINE__);
    mutex_unlock(&isp_dev->mlock);

    kfree(packet);
    return result;
}


RESULT VsiCamDeviceSensorDrvHandleUnRegister
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    payload_packet *packet = NULL;
    uint8_t *p_data = NULL; 

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
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    if(packet->payload_size > MAX_ITEM)
    {
        dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",packet->payload_size , MAX_ITEM);
        kfree(packet);
    	return RET_OUTOFRANGE;
    }

#if 1
    mutex_lock(&isp_dev->mlock);
    result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_UNREG , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        kfree(packet);
        mutex_unlock(&isp_dev->mlock);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);

    kfree(packet);
    return result;
}

RESULT VsiCamDeviceSensorClose
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    payload_packet *packet=NULL;
    uint8_t *p_data = NULL;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
    	pr_err("FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
	return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    if(packet->payload_size > MAX_ITEM)
    {
        dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",packet->payload_size , MAX_ITEM);
        kfree(packet);
    	return RET_OUTOFRANGE;
    }
#if 1
    mutex_lock(&isp_dev->mlock);
    result= Send_Command(APU_2_RPU_MB_CMD_SENSOR_CLOSE , packet,packet->payload_size + payload_extra_size  ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);

    kfree(packet);
	return result;
}
RESULT VsiCamDeviceSensorMapping
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t   hCamDevice,
    const char           *pSensorName,
    CamDeviceSensorDrvHandle_t *pSensorDrvhandle
)
{
    RESULT result = RET_SUCCESS;
    uint8_t *p_data=NULL; 
    payload_packet *packet=NULL;
    uint8_t __result;
    IsiCamDrvConfigMbox_t camcfg ;//= NULL;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pSensorName || NULL == pSensorDrvhandle) {
        return (RET_NULL_POINTER);
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
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    memcpy(p_data, pSensorName, SENSOR_NAME_LEN);
    p_data += SENSOR_NAME_LEN;
    packet->payload_size += SENSOR_NAME_LEN;

   // memcpy(p_data, pSensorDrvhandle, sizeof(uint32_t));
    packet->payload_size += sizeof(IsiCamDrvConfigMbox_t);

    if(packet->payload_size > MAX_ITEM)
    {
        dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",packet->payload_size , MAX_ITEM);
        kfree(packet);
        return RET_OUTOFRANGE;
    }

#if 1
    mutex_lock(&isp_dev->mlock);
    result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_MAPPING, packet, packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
    if(0 != result) {
    kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan, NULL);

#endif
	__result = xlnx_mbox_apu_wait_for_data(isp_dev, packet->payload);
	if(__result)
	{
        kfree(packet);
    	pr_err("FAiled in Sensor Mapping\n");
	    return -1;
	}
	*pSensorDrvhandle = kzalloc(sizeof(IsiCamDrvConfigMbox_t), GFP_KERNEL);

    if(*pSensorDrvhandle == NULL) {
        kfree(packet);
    	dev_err(isp_dev->dev,"APU Failed to allocate memory for sensor mapping.\n");
    	return RET_OUTOFMEM;
    }
    memcpy(*pSensorDrvhandle, p_data, sizeof(IsiCamDrvConfigMbox_t));

    mutex_unlock(&isp_dev->mlock);
	kfree(packet);

    //camcfg = **(IsiCamDrvConfigMbox_t **)pSensorDrvhandle;
    camcfg = **(IsiCamDrvConfigMbox_t **)pSensorDrvhandle;
	dev_err(isp_dev->dev , "APU mapping get cameraDriverID: %d.\r\n", camcfg.cameraDriverID);
    dev_err(isp_dev->dev , "APU mapping get pIsiGetSensorIss: %d.\r\n", camcfg.pIsiGetSensorIss);
    dev_err(isp_dev->dev , "APU mapping get cameraDevId: %d.\r\n", camcfg.cameraDevId);
    dev_err(isp_dev->dev , "APU mapping get instanceId: %d.\r\n", camcfg.instanceId);

	return result;
}

RESULT VsiCamDeviceSensorQuery
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorQuery_t *pQuery
)
{
    RESULT result = RET_SUCCESS;
    payload_packet *packet=NULL;
    uint8_t *p_data=NULL; 
    uint8_t __result; 

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pQuery) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    packet= kzalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
    	pr_err("FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
	    return -ENOMEM;
    } 

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;

    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    memcpy(p_data, pQuery, sizeof(CamDeviceSensorQuery_t));
    packet->payload_size += sizeof(CamDeviceSensorQuery_t);

    if(packet->payload_size > MAX_ITEM)
    {
        dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",packet->payload_size , MAX_ITEM);
        kfree(packet);
    	return RET_OUTOFRANGE;
    }
    mutex_lock(&isp_dev->mlock);
    result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_QUERY , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);


    __result = xlnx_mbox_apu_wait_for_data(isp_dev,packet->payload);
	if(__result)
    {
        kfree(packet);
        pr_err("FAILED TO QUERY SENSOR\n");
        return -1;
    }


	memcpy(pQuery, p_data, sizeof(CamDeviceSensorQuery_t));
    mutex_unlock(&isp_dev->mlock);
    kfree(packet);
	return result;
}


RESULT VsiCamDeviceSensorSetTestPattern
(
	struct vvcam_isp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    const CamDeviceSensorTestPattern_t *pTestPattern
)
{
    RESULT result = RET_SUCCESS;
    payload_packet *packet = NULL;
    uint8_t *p_data = NULL; 

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pTestPattern) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    packet= kzalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
        pr_err("FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
        return -ENOMEM;
    }

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;

    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    memcpy(p_data, pTestPattern, sizeof(CamDeviceSensorTestPattern_t));
    packet->payload_size += sizeof(CamDeviceSensorTestPattern_t);

    if(packet->payload_size > MAX_ITEM)
    {
        dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",packet->payload_size , MAX_ITEM);
        kfree(packet);
        return RET_OUTOFRANGE;
    }
#if 1
    mutex_lock(&isp_dev->mlock);
    result = Send_Command( APU_2_RPU_MB_CMD_SENSOR_SET_TP , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
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
    return result;
}




RESULT VsiCamDeviceSensorSetFrameRate
(
    struct vvcam_isp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    uint32_t * pFps
)
{
    RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
    uint8_t *p_data = NULL; 
	
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pFps) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
    	pr_err("FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
	    return -ENOMEM;
    } 

    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    memcpy(p_data, pFps, sizeof(/*float*/uint32_t));
    packet->payload_size += sizeof(/*float*/uint32_t);

    if(packet->payload_size > MAX_ITEM)
    {
        dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",packet->payload_size , MAX_ITEM);
        kfree(packet);
    	return RET_OUTOFRANGE;
    }
#if 1
    mutex_lock(&isp_dev->mlock);
    result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_SET_FRAMERATE , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack(isp_dev);
    mutex_unlock(&isp_dev->mlock);

    kfree(packet);
	
	return result;
}


RESULT VsiCamDeviceSensorGetConnectPortInfo
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t   hCamDevice,
    CamDeviceMcmPortId_t  portId,
    CamDeviceSensorConnectPortInfo_t *pPortInfo
){
    RESULT result = RET_SUCCESS;
    uint8_t *p_data=NULL; 
    payload_packet *packet=NULL;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pPortInfo) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
    	pr_err("FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
	    return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    memcpy(p_data, &portId, sizeof(CamDeviceMcmPortId_t));
    p_data += sizeof(CamDeviceMcmPortId_t);
    packet->payload_size += sizeof(CamDeviceMcmPortId_t);

    memcpy(p_data, pPortInfo, sizeof(CamDeviceSensorConnectPortInfo_t));
    packet->payload_size += sizeof(CamDeviceSensorConnectPortInfo_t);

    if(packet->payload_size > MAX_ITEM)
    {
        dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",packet->payload_size , MAX_ITEM);
        kfree(packet);
        return RET_OUTOFRANGE;
    }

#if 1
    mutex_lock(&isp_dev->mlock);
    result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_ConnectPortInfo , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
        kfree(packet);
        return result;
    }
    mbox_send_message(isp_dev->tx_chan,NULL);

#endif
	result = xlnx_mbox_apu_wait_for_data(isp_dev,packet->payload);
	if(result)
	{
        kfree(packet);
	    pr_err("FAiled in APU_2_RPU_MB_CMD_SENSOR_GET_ConnectPortInfo\n");
	    return -1;
	}
	mutex_unlock(&isp_dev->mlock);

    memcpy(pPortInfo, p_data, sizeof(CamDeviceSensorConnectPortInfo_t));
    pPortInfo->name[sizeof(pPortInfo->name)-1]='\0';

    dev_err(isp_dev->dev, "%s %d \n",__func__,__LINE__);
    dev_err(isp_dev->dev, "%s %d %d %s\n",__func__,__LINE__,pPortInfo->chipId,pPortInfo->name); 
    dev_err(isp_dev->dev, "%s %d \n",__func__,__LINE__);
    
    kfree(packet);

    return result;
}
                                    
