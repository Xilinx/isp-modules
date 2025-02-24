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

RESULT VsiCamDevice3DnrSetConfig(CamDeviceHandle_t hCamDevice,
								 CamDevice3DnrConfig_t *pConfig)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pConfig)
	{
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, pConfig, sizeof(CamDevice3DnrConfig_t));
	packet.payload_size += sizeof(CamDevice3DnrConfig_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	ret = Send_Command(APU_2_RPU_MB_CMD_3DNR_SET_CONFIG, &packet,
					   packet.payload_size + payload_extra_size, dest_cpu_id,
					   src_cpu_id);
	if (0 != ret)
	{
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}

	result = apu_wait_for_ACK(packet.cookie);
	if (result)
	{
		result = RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	return result;
}

RESULT VsiCamDevice3DnrGetConfig(CamDeviceHandle_t hCamDevice,
								 CamDevice3DnrConfig_t *pConfig)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pConfig)
	{
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, pConfig, sizeof(CamDevice3DnrConfig_t));
	packet.payload_size += sizeof(CamDevice3DnrConfig_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	ret = Send_Command(APU_2_RPU_MB_CMD_3DNR_GET_CONFIG, &packet,
					   packet.payload_size + payload_extra_size, dest_cpu_id,
					   src_cpu_id);
	if (0 != ret)
	{
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
	if (result)
	{
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pConfig, p_data, sizeof(CamDevice3DnrConfig_t));

	return result;
}

RESULT VsiCamDevice3DnrEnable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	ret = Send_Command(APU_2_RPU_MB_CMD_3DNR_ENABLE, &packet,
					   packet.payload_size + payload_extra_size, dest_cpu_id,
					   src_cpu_id);
	if (0 != ret)
	{
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}

	result = apu_wait_for_ACK(packet.cookie);
	if (result)
	{
		result = RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	return result;
}

RESULT VsiCamDevice3DnrDisable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	ret = Send_Command(APU_2_RPU_MB_CMD_3DNR_DISABLE, &packet,
					   packet.payload_size + payload_extra_size, dest_cpu_id,
					   src_cpu_id);
	if (0 != ret)
	{
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}

	result = apu_wait_for_ACK(packet.cookie);
	if (result)
	{
		result = RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	return result;
}

RESULT VsiCamDevice3DnrGetStatus(CamDeviceHandle_t hCamDevice,
								 CamDevice3DnrStatus_t *p3DnrStatus)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == p3DnrStatus)
	{
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, p3DnrStatus, sizeof(CamDevice3DnrStatus_t));
	packet.payload_size += sizeof(CamDevice3DnrStatus_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	ret = Send_Command(APU_2_RPU_MB_CMD_3DNR_GET_STATUS, &packet,
					   packet.payload_size + payload_extra_size, dest_cpu_id,
					   src_cpu_id);
	if (0 != ret)
	{
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
	if (result)
	{
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(p3DnrStatus, p_data, sizeof(CamDevice3DnrStatus_t));

	return result;
}

RESULT VsiCamDevice3DnrReset(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie++;

	Payload_packet packet;
	memset(&packet, 0, sizeof(Payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload_data;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	ret = Send_Command(APU_2_RPU_MB_CMD_3DNR_RESET, &packet,
					   packet.payload_size + payload_extra_size, dest_cpu_id,
					   src_cpu_id);
	if (0 != ret)
	{
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}

	result = apu_wait_for_ACK(packet.cookie);
	if (result)
	{
		result = RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	return result;
}

RESULT VsiCamDevice3DnrGetVersion(CamDeviceHandle_t hCamDevice,
								  uint32_t *pVersion)
{
	RESULT result = RET_SUCCESS;
	int ret = 0;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pVersion)
	{
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie++;

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

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	ret = Send_Command(APU_2_RPU_MB_CMD_3DNR_GET_VERSION, &packet,
					   packet.payload_size + payload_extra_size, dest_cpu_id,
					   src_cpu_id);
	if (0 != ret)
	{
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_mb_data(packet.cookie, packet.payload_data);
	if (result)
	{
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pVersion, p_data, sizeof(uint32_t));

	return result;
}
