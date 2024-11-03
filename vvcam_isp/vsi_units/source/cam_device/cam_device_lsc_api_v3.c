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

#include "cam_device.h"



RESULT VsiCamDeviceLscSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceLscConfig_t *pLscCfg
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pLscCfg) {
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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pLscCfg, sizeof(CamDeviceLscConfig_t));
    packet.payload_size += sizeof(CamDeviceLscConfig_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_LSC_SET_CONFIG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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



RESULT VsiCamDeviceLscGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceLscConfig_t *pLscCfg
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pLscCfg) {
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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pLscCfg, sizeof(CamDeviceLscConfig_t));
    packet.payload_size += sizeof(CamDeviceLscConfig_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_LSC_GET_CONFIG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pLscCfg, p_data, sizeof(CamDeviceLscConfig_t));

	return result;
}



RESULT VsiCamDeviceLscEnable
(
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;

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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_LSC_ENABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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



RESULT VsiCamDeviceLscDisable
(
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;

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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_LSC_DISABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceLscReset
(
    CamDeviceHandle_t      hCamDevice
)
{
    RESULT result = RET_SUCCESS;

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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_LSC_RESET, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceLscGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceLscStatus_t *pLscStatus
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pLscStatus) {
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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pLscStatus, sizeof(CamDeviceLscStatus_t));
    packet.payload_size += sizeof(CamDeviceLscStatus_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_LSC_GET_STATUS, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }
    result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	memcpy(pLscStatus, p_data, sizeof(CamDeviceLscStatus_t));

	return result;
}



RESULT VsiCamDeviceLscGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
)
{
    RESULT result = RET_SUCCESS;

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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pVersion, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_LSC_GET_VERSION, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
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

