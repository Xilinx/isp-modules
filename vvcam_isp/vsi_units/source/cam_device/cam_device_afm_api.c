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

RESULT VsiCamDeviceAfmSetThreshold
(
    CamDeviceHandle_t  hCamDevice,
    const uint32_t     threshold

){
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
    memcpy(p_data, &threshold, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AFM_SET_THRESHOLD, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceAfmGetThreshold
(
    CamDeviceHandle_t  hCamDevice,
    uint32_t          *pThreshold
){
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pThreshold) {
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
    memcpy(p_data, pThreshold, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AFM_GET_THRESHOLD, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pThreshold, p_data, sizeof(uint32_t));

	return result;
}



RESULT VsiCamDeviceAfmGetStatistics
(
    CamDeviceHandle_t              hCamDevice,
    CamDeviceAfmMeasureResult_t   *pResult
){
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pResult) {
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
    memcpy(p_data, pResult, sizeof(CamDeviceAfmMeasureResult_t));
    packet.payload_size += sizeof(CamDeviceAfmMeasureResult_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AFM_GET_STATISTICS, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pResult, p_data, sizeof(CamDeviceAfmMeasureResult_t));

	return result;
}



RESULT VsiCamDeviceAfmSetMeasureWindow
(
	CamDeviceHandle_t            hCamDevice,
	CamDeviceAfmWindowConfig_t  *pWindow
){
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pWindow) {
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

    memcpy(p_data, pWindow, sizeof(CamDeviceWindow_t));
    packet.payload_size += sizeof(CamDeviceWindow_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AFM_SET_MEASURE_WINDOW, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceAfmGetMeasureWindow
(
	CamDeviceHandle_t                 hCamDevice,
	CamDeviceAfmWindowConfig_t       *pWindow
){
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pWindow) {
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

    memcpy(p_data, pWindow, sizeof(CamDeviceWindow_t));
    packet.payload_size += sizeof(CamDeviceWindow_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AFM_GET_MEASURE_WINDOW, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pWindow, p_data, sizeof(CamDeviceWindow_t));

	return result;
}



RESULT VsiCamDeviceAfmEnable
(
    CamDeviceHandle_t  hCamDevice
){
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

    ret = Send_Command (APU_2_RPU_MB_CMD_AFM_ENABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceAfmDisable
(
    CamDeviceHandle_t  hCamDevice
){
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

    ret = Send_Command (APU_2_RPU_MB_CMD_AFM_DISABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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



RESULT VsiCamDeviceAfmGetStatus
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAfmStatus_t     *pStatus
){
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
    memcpy(p_data, pStatus, sizeof(CamDeviceAfmStatus_t));
    packet.payload_size += sizeof(CamDeviceAfmStatus_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AFM_GET_STATUS, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pStatus, p_data, sizeof(CamDeviceAfmStatus_t));

	return result;
}


RESULT VsiCamDeviceAfmGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
){
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

    ret = Send_Command (APU_2_RPU_MB_CMD_AFM_GET_VERSION, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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



RESULT VsiCamDeviceAfmReset
(
    CamDeviceHandle_t             hCamDevice
){
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

    ret = Send_Command (APU_2_RPU_MB_CMD_AFM_RESET, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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
