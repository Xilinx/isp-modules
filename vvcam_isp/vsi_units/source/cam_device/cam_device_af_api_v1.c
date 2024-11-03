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


RESULT VsiCamDeviceAfRegister
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

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

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_REGISTER, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceAfUnRegister
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_UNREGISTER, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceAfSetMode
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceAfMode_t   *pMode
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pMode) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pMode, sizeof(CamDeviceAfMode_t));
    packet.payload_size += sizeof(CamDeviceAfMode_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_SET_MODE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceAfGetMode
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceAfMode_t   *pMode
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pMode) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pMode, sizeof(CamDeviceAfMode_t));
    packet.payload_size += sizeof(CamDeviceAfMode_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_GET_MODE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pMode, p_data, sizeof(CamDeviceAfMode_t));

	return result;
}

RESULT VsiCamDeviceAfSetConfig
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAfConfig_t      *pConfig
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pConfig, sizeof(CamDeviceAfConfig_t));
    packet.payload_size += sizeof(CamDeviceAfConfig_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_SET_CONFIG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceAfGetConfig
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAfConfig_t      *pConfig
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pConfig, sizeof(CamDeviceAfConfig_t));
    packet.payload_size += sizeof(CamDeviceAfConfig_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_GET_CONFIG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pConfig, p_data, sizeof(CamDeviceAfConfig_t));

	return result;
}

RESULT VsiCamDeviceAfSetRoiIndex
(
    CamDeviceHandle_t  hCamDevice,
    uint8_t            roiIndex
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &roiIndex, sizeof(uint8_t));
    packet.payload_size += sizeof(uint8_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_SET_ROI_INDEX, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceAfGetRoiIndex
(
    CamDeviceHandle_t  hCamDevice,
    uint8_t            *pRoiIndex
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pRoiIndex) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pRoiIndex, sizeof(uint8_t));
    packet.payload_size += sizeof(uint8_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_GET_ROI_INDEX, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pRoiIndex, p_data, sizeof(uint8_t));

	return result;
}

RESULT VsiCamDeviceAfEnable
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_ENABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceAfDisable
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_DISABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceAfGetStatus
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceAfStatus_t  *pStatus
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pStatus) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pStatus, sizeof(CamDeviceAfState_t));
    packet.payload_size += sizeof(CamDeviceAfState_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_GET_STATUS, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pStatus, p_data, sizeof(CamDeviceAfState_t));

	return result;
}

RESULT VsiCamDeviceAfGetVersion
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

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

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_GET_VERSION, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceAfReset
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AF_RESET, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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
