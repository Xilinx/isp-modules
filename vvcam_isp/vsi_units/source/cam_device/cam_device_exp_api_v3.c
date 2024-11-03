/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2022 Vivante Corporation
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
 ****************************************************************************/

#include "cam_device.h"

RESULT VsiCamDeviceExpV3SetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceExpV3Config_t    *pConfig
){
    RESULT result = RET_SUCCESS;

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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pConfig, sizeof(CamDeviceExpV3Config_t));
    packet.payload_size += sizeof(CamDeviceExpV3Config_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_EXP_V3_SET_CONFIG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceExpV3GetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceExpV3Config_t    *pConfig
){
    RESULT result = RET_SUCCESS;

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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pConfig, sizeof(CamDeviceExpV3Config_t));
    packet.payload_size += sizeof(CamDeviceExpV3Config_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_EXP_V3_GET_CONFIG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pConfig, p_data, sizeof(CamDeviceExpV3Config_t));

	return result;
}



RESULT VsiCamDeviceExpV3SetMeasureWindow
(
    CamDeviceHandle_t              hCamDevice,
    CamDeviceExpV3WindowConfig_t     *pWindow
){
    RESULT result = RET_SUCCESS;

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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pWindow, sizeof(CamDeviceExpV3WindowConfig_t));
    packet.payload_size += sizeof(CamDeviceExpV3WindowConfig_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_EXP_V3_SET_MEASURE_WINDOW, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceExpV3GetMeasureWindow
(
    CamDeviceHandle_t               hCamDevice,
    CamDeviceExpV3WindowConfig_t     *pWindow
){
    RESULT result = RET_SUCCESS;

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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pWindow, sizeof(CamDeviceExpV3WindowConfig_t));
    packet.payload_size += sizeof(CamDeviceExpV3WindowConfig_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_EXP_V3_GET_MEASURE_WINDOW, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pWindow, p_data, sizeof(CamDeviceExpV3WindowConfig_t));

	return result;
}


RESULT VsiCamDeviceExpV3GetStatistics
(
    CamDeviceHandle_t          hCamDevice,
    CamDeviceExpV3Statistics_t  *pExpStatic
){
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pExpStatic) {
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
    memcpy(p_data, pExpStatic, sizeof(CamDeviceExpV3Statistics_t));
    packet.payload_size += sizeof(CamDeviceExpV3Statistics_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_EXP_V3_GET_STATISTICS, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pExpStatic, p_data, sizeof(CamDeviceExpV3Statistics_t));

	return result;
}


RESULT VsiCamDeviceExpV3Enable
(
    CamDeviceHandle_t   hCamDevice
){
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

    result = Send_Command (APU_2_RPU_MB_CMD_EXP_V3_ENABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceExpV3Disable
(
    CamDeviceHandle_t   hCamDevice
){
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

    result = Send_Command (APU_2_RPU_MB_CMD_EXP_V3_DISABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceExpV3GetStatus
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceExpV3Status_t    *pStatus
){
    RESULT result = RET_SUCCESS;

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
    p_data += sizeof(uint32_t);
    packet.payload_size += sizeof(uint32_t);
    memcpy(p_data, pStatus, sizeof(CamDeviceExpV3Status_t));
    packet.payload_size += sizeof(CamDeviceExpV3Status_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_EXP_V3_GET_STATUS, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pStatus, p_data, sizeof(CamDeviceExpV3Status_t));

	return result;
}


RESULT VsiCamDeviceExpV3GetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
){
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

    result = Send_Command (APU_2_RPU_MB_CMD_EXP_V3_GET_VERSION, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceExpV3Reset
(
    CamDeviceHandle_t        hCamDevice
){
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

    result = Send_Command (APU_2_RPU_MB_CMD_EXP_V3_RESET, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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
