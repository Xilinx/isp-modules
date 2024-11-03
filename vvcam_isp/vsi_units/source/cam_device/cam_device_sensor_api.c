/******************************************************************************\
|* Copyright (c) 2022 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/


#include "cam_device_sensor_api.h"
#include "cam_device_api.h"
#include "cam_device.h"
//#include "mailbox.h" //added by ranjith
#include "sensor_cmd.h"
#include <isi/isi_iss.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "kmbox.h"
#include "vvcam_isp_driver.h"

RESULT VsiCamDeviceSensorOpen
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice,
    uint32_t          modeIndex
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
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

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);
    memcpy(p_data, &modeIndex, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_OPEN, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif


#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command(APU_2_RPU_MB_CMD_SENSOR_OPEN , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif

//	pr_err("RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
//	int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_SENSOR_OPEN);
//	pr_err("RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);

    xlnx_mbox_apu_wait_for_ack();

    kfree(packet);

    return result;
}

RESULT VsiCamDeviceSensorDrvHandleRegister
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorDrvHandle_t sensorDrvHandle
)
{
    pr_err("RKC %s %d \n",__func__,__LINE__);
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx || NULL == sensorDrvHandle) {
        return (RET_WRONG_HANDLE);
    }

    pr_err("RKC %s %d \n",__func__,__LINE__);
    pCamDevCtx->cookie ++;
    //pCamDevCtx->cookie = 44;

    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie =pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);
    memcpy(p_data, sensorDrvHandle, sizeof(IsiCamDrvConfig_t));
    packet->payload_size += sizeof(IsiCamDrvConfig_t);
    pr_err("RKC %s %d \n",__func__,__LINE__);
    IsiCamDrvConfig_t* pcamcfg = (IsiCamDrvConfig_t *)p_data;
    pr_err("%s %d cameraDriverID: %di pIsiGetSensorIss: %x\n",__func__,__LINE__,((IsiCamDrvConfig_t* )p_data)->cameraDriverID,((IsiCamDrvConfig_t* )p_data)->pIsiGetSensorIss);
    pr_err("[APU]-sensorDrvHandle->cameraDriverID: %d.\r\n", pcamcfg->cameraDriverID);
    pr_err("sensorDrvHandle->pIsiGetSensorIss: %x.\r\n", pcamcfg->pIsiGetSensorIss);
    pr_err("sensorDrvHandle->i2cBusId: %d.\r\n", pcamcfg->i2cBusId);

	if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_REG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif

#if 1
    mutex_lock(&isp_dev->mlock);
    int ret ;// Send_Command(APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_REG , packet,packet->payload_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    ret = Send_Command (APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_REG,packet, packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif





//	pr_err("RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
//int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_REG);
//	pr_err("RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);

    xlnx_mbox_apu_wait_for_ack();

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

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
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

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_UNREG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif


#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command(APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_UNREG , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
        kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif




//	pr_err("RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
//int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_UNREG);
//	pr_err("RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);

    xlnx_mbox_apu_wait_for_ack();
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

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
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

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_CLOSE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif


#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command(APU_2_RPU_MB_CMD_SENSOR_CLOSE , packet,packet->payload_size + payload_extra_size  ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif



//    pr_err("RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
  //  int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_SENSOR_CLOSE);
//	pr_err("RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);

    xlnx_mbox_apu_wait_for_ack();
kfree(packet);
	return result;
}

RESULT VsiCamDeviceSensorMapping
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t   hCamDevice,
    char               *pSensorName,
    CamDeviceSensorDrvHandle_t *pSensorDrvhandle
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pSensorName || NULL == pSensorDrvhandle) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
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

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);
    memcpy(p_data, pSensorName, SENSOR_NAME_LEN);
    p_data += SENSOR_NAME_LEN;
    packet->payload_size += SENSOR_NAME_LEN;
   // memcpy(p_data, pSensorDrvhandle, sizeof(uint32_t));
    packet->payload_size += sizeof(IsiCamDrvConfig_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_MAPPING, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif


#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command(APU_2_RPU_MB_CMD_SENSOR_MAPPING , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif
//	pr_err("RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
  //  int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_SENSOR_MAPPING);
//	pr_err("RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);
	uint8_t __result = xlnx_mbox_apu_wait_for_data(packet->payload);
	if(__result)
	{
	pr_err("FAiled in Sensor Mapping\n");
	return -1;
	}
	*pSensorDrvhandle = kzalloc(sizeof(IsiCamDrvConfig_t),GFP_KERNEL);

    if(*pSensorDrvhandle == NULL) {
    	// xil_printf("APU Failed to allocate memory for sensor mapping.\r\n");
    	return RET_OUTOFMEM;
    }
    memcpy(*pSensorDrvhandle, p_data, sizeof(IsiCamDrvConfig_t));

    p_data -= SENSOR_NAME_LEN;
    memcpy(pSensorName, p_data, SENSOR_NAME_LEN);
	//RKC - DOUBT free above psensorDrvhanlde ?
	kfree(packet);

    IsiCamDrvConfig_t* pcamcfg = (IsiCamDrvConfig_t *)*pSensorDrvhandle;

    pr_err("[APU]-sensorDrvHandle->cameraDriverID: %d.\r\n", pcamcfg->cameraDriverID);
    pr_err("sensorDrvHandle->pIsiGetSensorIss: %x.\r\n", pcamcfg->pIsiGetSensorIss);
    pr_err("sensorDrvHandle->i2cBusId: %d.\r\n", pcamcfg->i2cBusId);
//IsiCamDrvConfig_t *pSensorDrvhandle_r=  (IsiCamDrvConfig_t *) pSensorDrvhandle;
//	pr_err("pSensorName= %s cameraDrievrId = %d isi_ptr=%p , BusId= %d \n",pSensorName,pSensorDrvhandle_r->cameraDriverID,pSensorDrvhandle_r->pIsiGetSensorIss,pSensorDrvhandle_r->i2cBusId);

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

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pQuery) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
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

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);
    memcpy(p_data, pQuery, sizeof(CamDeviceSensorQuery_t));
    packet->payload_size += sizeof(CamDeviceSensorQuery_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_QUERY, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pQuery, p_data, sizeof(CamDeviceSensorQuery_t));
#endif


#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command(APU_2_RPU_MB_CMD_SENSOR_QUERY , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif



//	pr_err("RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
  //  int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_SENSOR_QUERY);
//	pr_err("RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);
    uint8_t __result = xlnx_mbox_apu_wait_for_data(packet->payload);
	if(__result)
    {
        pr_err("FAILED TO QUERY SENSOR\n");
        return -1;
    }


	memcpy(pQuery, p_data, sizeof(CamDeviceSensorQuery_t));
	pr_err("RKC-DRV %s %d done \n",__func__,__LINE__);
    kfree(packet);
	return result;
}

#if 0
RESULT VsiCamDeviceSensorGetInfo
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorInfo_t *pSensorInfo
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pSensorInfo) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    //memcpy(p_data, pSensorInfo, sizeof(CamDeviceSensorInfo_t));
    packet.payload_size += sizeof(CamDeviceSensorInfo_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_INFO, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pSensorInfo, p_data, sizeof(CamDeviceSensorInfo_t));

	return result;
}


RESULT VsiCamDeviceSensorSetTestPattern
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorTestPattern_t *pTestPattern
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pTestPattern) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pTestPattern, sizeof(CamDeviceSensorTestPattern_t));
    packet.payload_size += sizeof(CamDeviceSensorTestPattern_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_TP, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}


RESULT VsiCamDeviceSensorSetRegister
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorRegister_t *pRegister
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pRegister) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pRegister, sizeof(CamDeviceSensorRegister_t));
    packet.payload_size += sizeof(CamDeviceSensorRegister_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_REG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}


RESULT VsiCamDeviceSensorGetRegister
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorRegister_t *pRegister
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pRegister) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pRegister, sizeof(CamDeviceSensorRegister_t));
    packet.payload_size += sizeof(CamDeviceSensorRegister_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_REG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pRegister, p_data, sizeof(CamDeviceSensorRegister_t));

	return result;
}


RESULT VsiCamDeviceSensorSetFrameRate
(
    CamDeviceHandle_t hCamDevice,
    float *pFps
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pFps) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pFps, sizeof(float));
    packet.payload_size += sizeof(float);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_FRAMERATE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}


RESULT VsiCamDeviceSensorGetFrameRate
(
    CamDeviceHandle_t hCamDevice,
    float *pFps
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pFps) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pFps, sizeof(float));
    packet.payload_size += sizeof(float);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_FRAMERATE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pFps, p_data, sizeof(float));

	return result;
}


RESULT VsiCamDeviceSensorGetModeInfo
(
    CamDeviceHandle_t          hCamDevice,
    CamDeviceSensorModeInfo_t *pMode
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pMode) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pMode, sizeof(CamDeviceSensorModeInfo_t));
    packet.payload_size += sizeof(CamDeviceSensorModeInfo_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_MODE_INFO, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pMode, p_data, sizeof(CamDeviceSensorModeInfo_t));
	return result;
}


RESULT VsiCamDeviceSensorGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorStatus_t *pStatus
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pStatus) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pStatus, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_STATUS, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pStatus, p_data, sizeof(CamDeviceSensorStatus_t));

	return result;
}


RESULT VsiCamDeviceSensorSetGain
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorGain_t *pGain
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pGain) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pGain, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_GAIN, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}


RESULT VsiCamDeviceSensorGetGain
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorGain_t *pGain
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pGain) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pGain, sizeof(CamDeviceSensorGain_t));
    packet.payload_size += sizeof(CamDeviceSensorGain_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_GAIN, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pGain, p_data, sizeof(CamDeviceSensorGain_t));

	return result;
}


RESULT VsiCamDeviceSensorSetExposureControl
(
    CamDeviceHandle_t hCamDevice,
	CamDeviceSensorExposureControl_t  *pExpCtrl
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pExpCtrl) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pExpCtrl, sizeof(CamDeviceSensorExposureControl_t));
    packet.payload_size += sizeof(CamDeviceSensorExposureControl_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_EXP_CTL, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}

RESULT VsiCamDeviceSensorGetExposureControl
(
   CamDeviceHandle_t                  hCamDevice,
   CamDeviceSensorExposureControl_t  *pExpCtrl

)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pExpCtrl) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pExpCtrl, sizeof(CamDeviceSensorExposureControl_t));
    packet.payload_size += sizeof(float);

    if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_EXP_CTL, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pExpCtrl, p_data, sizeof(CamDeviceSensorExposureControl_t));

	return result;
}


RESULT VsiCamDeviceSensorSetFocusPositon
(
   CamDeviceHandle_t hCamDevice,
   CamDeviceSensorFocusPos_t  *pFocusPos
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pFocusPos) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pFocusPos, sizeof(CamDeviceSensorFocusPos_t));
    packet.payload_size += sizeof(CamDeviceSensorFocusPos_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SetFocusPositon, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);

    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}

RESULT VsiCamDeviceSensorGetFocusPositon
(
   CamDeviceHandle_t hCamDevice,
   CamDeviceSensorFocusPos_t  *pFocusPos,
   CamDeviceSensorIntegerRange_t *pRangeLimit
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pFocusPos || NULL == pRangeLimit) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);

    memcpy(p_data, pFocusPos, sizeof(CamDeviceSensorFocusPos_t));
    p_data += sizeof(CamDeviceSensorFocusPos_t);
    packet.payload_size += sizeof(CamDeviceSensorFocusPos_t);

    memcpy(p_data, pRangeLimit, sizeof(CamDeviceSensorIntegerRange_t));
    packet.payload_size += sizeof(CamDeviceSensorIntegerRange_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GetFocusPositon, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pRangeLimit, p_data, sizeof(CamDeviceSensorIntegerRange_t));

	p_data -= sizeof(CamDeviceSensorFocusPos_t);
	memcpy(pFocusPos, p_data, sizeof(CamDeviceSensorFocusPos_t));

	return result;
}


RESULT VsiCamDeviceSensorSetStreaming
(
    CamDeviceHandle_t hCamDevice,
    bool_t isEnable
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, &isEnable, sizeof(bool_t));
    packet.payload_size += sizeof(bool_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_STREAMING, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}


RESULT VsiCamDeviceSensorGetOtpInfo
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorOtpModuleInfo_t *pOtpModuleInfo
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pOtpModuleInfo) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pOtpModuleInfo, sizeof(CamDeviceSensorOtpModuleInfo_t));
    packet.payload_size += sizeof(CamDeviceSensorOtpModuleInfo_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_OPT_INFO, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pOtpModuleInfo, p_data, sizeof(CamDeviceSensorOtpModuleInfo_t));

	return result;
}

RESULT VsiCamDeviceSensorGetMetadataAttr
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorMetadataAttr_t *pMetaAttr
){
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pMetaAttr) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pMetaAttr, sizeof(CamDeviceSensorMetadataAttr_t));
    packet.payload_size += sizeof(CamDeviceSensorMetadataAttr_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_METADATA_ATTR, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pMetaAttr, p_data, sizeof(CamDeviceSensorMetadataAttr_t));

	return result;
}


RESULT VsiCamDeviceSensorSetMetadataAttr
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorMetadataAttr_t metaAttr
){
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, &metaAttr, sizeof(CamDeviceSensorMetadataAttr_t));
    packet.payload_size += sizeof(CamDeviceSensorMetadataAttr_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_METADATA_ATTR, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}

RESULT VsiCamDeviceSensorGetMetadataWin
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorMetadataWin_t *pMetaWin
){
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pMetaWin) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pMetaWin, sizeof(CamDeviceSensorMetadataWin_t));
    packet.payload_size += sizeof(CamDeviceSensorMetadataWin_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_METADATA_WIN, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pMetaWin, p_data, sizeof(CamDeviceSensorMetadataWin_t));

	return result;
}

#endif
