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


RESULT VsiCamDeviceRegisterAeLib
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (RPU_2_APU_MB_CMD_REGISTER_AELIB, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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



RESULT VsiCamDeviceUnRegisterAeLib
(
    CamDeviceHandle_t hCamDevice
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
    packet = (Payload_packet *)kmalloc(sizeof(Payload_packet),GFP_KERNEL);
    memset(packet, 0, sizeof(Payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
#if 0
    os_lock_mutex(&mbox_lock);

    ret = Send_Command (RPU_2_APU_MB_CMD_UNREGISTER_AELIB, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }

    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif
	pr_err("RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_CONNECT_CAMERA);
	pr_err("RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);
	return result;
}



RESULT VsiCamDeviceAeSetConfig
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceAeConfig_t      *pConfig
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
    memcpy(p_data, pConfig, sizeof(CamDeviceAeConfig_t));
    packet.payload_size += sizeof(CamDeviceAeConfig_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_SET_CONFIG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceAeGetConfig
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceAeConfig_t    *pConfig
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
    memcpy(p_data, pConfig, sizeof(CamDeviceAeConfig_t));
    packet.payload_size += sizeof(CamDeviceAeConfig_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_GET_CONFIG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

    memcpy(pConfig, p_data, sizeof(CamDeviceAeConfig_t));

	return result;
}


RESULT VsiCamDeviceAeSetMode
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAeMode_t      *pAeMode
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pAeMode) {
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
    memcpy(p_data, pAeMode, sizeof(CamDeviceAeMode_t));
    packet.payload_size += sizeof(CamDeviceAeMode_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_SET_MODE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceAeGetMode
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAeMode_t      *pAeMode
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pAeMode) {
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
    memcpy(p_data, pAeMode, sizeof(CamDeviceAeMode_t));
    packet.payload_size += sizeof(CamDeviceAeMode_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_GET_MODE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

    memcpy(pAeMode, p_data, sizeof(CamDeviceAeMode_t));

	return result;
}


RESULT VsiCamDeviceAeSetRoi
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceRoi_t         *pRoi
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

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

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_SET_ROI, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceAeGetRoi
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceRoi_t         *pRoi
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

    Payload_packet packet;
    memset(&packet, 0, sizeof(Payload_packet));

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

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_GET_ROI, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceAeEnable
(
    CamDeviceHandle_t hCamDevice
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

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_ENABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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



RESULT VsiCamDeviceAeDisable
(
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    Payload_packet *packet;
    packet = (Payload_packet *)kmalloc(sizeof(Payload_packet),GFP_KERNEL);
    memset(packet, 0, sizeof(Payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload_data;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_DISABLE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
        os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }

    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif
	pr_err("RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_AE_DISABLE);
	pr_err("RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);

	return result;
}


RESULT VsiCamDeviceAeReset
(
    CamDeviceHandle_t hCamDevice
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

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_RESET, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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



RESULT VsiCamDeviceAeGetStatus
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAeState_t     *pAeStatus
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pAeStatus) {
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
    memcpy(p_data, pAeStatus, sizeof(CamDeviceAeState_t));
    packet.payload_size += sizeof(CamDeviceAeState_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_GET_STATUS, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pAeStatus, p_data, sizeof(CamDeviceAeState_t));

	return result;
}


//implementation in aec
RESULT VsiCamDeviceAeGetHistogram
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAeHistBins_t  *pHistogram
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pHistogram) {
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
    memcpy(p_data, pHistogram, sizeof(CamDeviceAeHistBins_t));
    packet.payload_size += sizeof(CamDeviceAeHistBins_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_GET_HISTOGRAM, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pHistogram, p_data, sizeof(CamDeviceAeHistBins_t));

	return result;
}



//implementation in aec
RESULT VsiCamDeviceAeGetLuminance
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAeMeanLuma_t  *pLuma
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pLuma) {
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
    memcpy(p_data, pLuma, sizeof(CamDeviceAeMeanLuma_t));
    packet.payload_size += sizeof(CamDeviceAeMeanLuma_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_GET_LUMINANCE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pLuma, p_data, sizeof(CamDeviceAeMeanLuma_t));

	return result;
}



//implementation in aec
RESULT VsiCamDeviceAeGetObjectRegion
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAeMeanLuma_t  *pObjectRegion
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pObjectRegion) {
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
    memcpy(p_data, pObjectRegion, sizeof(CamDeviceAeMeanLuma_t));
    packet.payload_size += sizeof(CamDeviceAeMeanLuma_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_GET_OBJECT_REGION, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pObjectRegion, p_data, sizeof(CamDeviceAeMeanLuma_t));

	return result;
}


RESULT VsiCamDeviceAeGetVersion
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

    ret = Send_Command (APU_2_RPU_MB_CMD_AE_GET_VERSION, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

