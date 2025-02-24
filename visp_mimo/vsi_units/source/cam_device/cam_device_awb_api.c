/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include "cam_device.h"
#include "sensor_cmd.h"
#include <linux/delay.h>
#include "vvcam_isp_driver.h"
//#include <linux/xlnx_isp_def.h>
#include <linux/string.h>
#include "kmbox.h"
#include "visp_common.h"

#if 0
RESULT VsiCamDeviceRegisterAwbLib
(
    CamDeviceHandle_t   hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

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

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_REGISTER_AWB_LIB, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
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
#endif


RESULT VsiCamDeviceUnRegisterAwbLib
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
    packet->payload_size += sizeof(uint32_t);

    if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
    	return RET_OUTOFRANGE;
    }
    mutex_lock(&isp_dev->mlock);
    result = Send_Command( APU_2_RPU_MB_CMD_UNREGISTER_AWB_LIB , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != result) {
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

#if 0

RESULT VsiCamDeviceAwbSetConfig
(
    CamDeviceHandle_t       hCamDevice,
    const CamDeviceAwbConfig_t   *pConfig
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pConfig) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pConfig, sizeof(CamDeviceAwbConfig_t));
    packet.payload_size += sizeof(CamDeviceAwbConfig_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AWB_SET_CONFIG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
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



RESULT VsiCamDeviceAwbGetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAwbConfig_t   *pConfig
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pConfig) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pConfig, sizeof(CamDeviceAwbConfig_t));
    packet.payload_size += sizeof(CamDeviceAwbConfig_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AWB_GET_CONFIG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pConfig, p_data, sizeof(CamDeviceAwbConfig_t));

	return result;
}


RESULT VsiCamDeviceAwbSetMode
(
    CamDeviceHandle_t       hCamDevice,
    const CamDeviceAwbMode_t      *pAwbMode
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pAwbMode) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pAwbMode, sizeof(CamDeviceAwbMode_t));
    packet.payload_size += sizeof(CamDeviceAwbMode_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AWB_SET_MODE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
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


RESULT VsiCamDeviceAwbGetMode
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAwbMode_t      *pAwbMode
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pAwbMode) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pAwbMode, sizeof(CamDeviceAwbMode_t));
    packet.payload_size += sizeof(CamDeviceAwbMode_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AWB_GET_MODE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pAwbMode, p_data, sizeof(CamDeviceAwbMode_t));

	return result;
}


RESULT VsiCamDeviceAwbSetRoi
(
    CamDeviceHandle_t     hCamDevice,
    const CamDeviceRoi_t  *pRoi
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pRoi) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pRoi, sizeof(CamDeviceRoi_t));
    packet.payload_size += sizeof(CamDeviceRoi_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AWB_SET_ROI, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
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


RESULT VsiCamDeviceAwbGetRoi
(
    CamDeviceHandle_t  hCamDevice,
    CamDeviceRoi_t     *pRoi
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pRoi) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pRoi, sizeof(CamDeviceRoi_t));
    packet.payload_size += sizeof(CamDeviceRoi_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AWB_GET_ROI, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pRoi, p_data, sizeof(CamDeviceRoi_t));

	return result;
}

RESULT VsiCamDeviceAwbGetColorTempWeight
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAwbTemWeight_t  *pAwbTemWeight
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pAwbTemWeight) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pAwbTemWeight, sizeof(CamDeviceAwbTemWeight_t));
    packet.payload_size += sizeof(CamDeviceAwbTemWeight_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AWB_GET_ColorTempWeight, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pAwbTemWeight, p_data, sizeof(CamDeviceAwbTemWeight_t));

	return result;
}


RESULT VsiCamDeviceAwbEnable
(
    CamDeviceHandle_t  hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

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

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AWB_ENABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
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
#endif


RESULT VsiCamDeviceAwbDisable
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t  hCamDevice
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
    packet->payload_size += sizeof(uint32_t);

    if(packet->payload_size > MAX_ITEM)
    {
        kfree(packet);
    	return RET_OUTOFRANGE;
    }

    mutex_lock(&isp_dev->mlock);
    result = Send_Command( APU_2_RPU_MB_CMD_AWB_DISABLE, packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
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

#if 0

RESULT VsiCamDeviceAwbGetStatus
(
    CamDeviceHandle_t     hCamDevice,
    CamDeviceAwbState_t   *pAwbStatus
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pAwbStatus) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pAwbStatus, sizeof(CamDeviceAwbState_t));
    packet.payload_size += sizeof(CamDeviceAwbState_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AWB_GET_STATUS, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pAwbStatus, p_data, sizeof(CamDeviceAwbState_t));

	return result;
}


RESULT VsiCamDeviceAwbGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pVersion) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pVersion, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AWB_GET_VERSION, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pVersion, p_data, sizeof(uint32_t));

	return result;
}
#endif
