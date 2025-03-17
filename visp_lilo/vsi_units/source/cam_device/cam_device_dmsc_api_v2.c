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

RESULT VsiCamDeviceDmscSetConfig(CamDeviceHandle_t hCamDevice,
								 CamDeviceDmscConfig_t *pConfig)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pConfig, sizeof(CamDeviceDmscConfig_t));
	packet.payload_size += sizeof(CamDeviceDmscConfig_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_SET_CONFIG, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscGetConfig(CamDeviceHandle_t hCamDevice,
								 CamDeviceDmscConfig_t *pConfig)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pConfig, sizeof(CamDeviceDmscConfig_t));
	packet.payload_size += sizeof(CamDeviceDmscConfig_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_GET_CONFIG, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

	memcpy(pConfig, p_data, sizeof(CamDeviceDmscConfig_t));

	return result;
}

RESULT VsiCamDeviceDmscEnable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_ENABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscDisable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_DISABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscCacEnable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_CAC_ENABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscCacDisable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_CAC_DISABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscDemoireEnable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_DEMOIRE_ENABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscDemoireDisable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_DEMOIRE_DISABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscDepurpleEnable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_DEPURPLE_ENABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscDepurpleDisable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_DEPURPLE_DISABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscSharpenEnable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_SHARPEN_ENABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscSharpenDisable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_SHARPEN_DISABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscSharpenLineEnable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_SHARPEN_LINE_ENABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscSharpenLineDisable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_SHARPEN_LINE_DISABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscSkinEnable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_SKIN_ENABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscSkinDisable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_SKIN_DISABLE, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscGetStatus(CamDeviceHandle_t hCamDevice,
								 CamDeviceDmscStatus_t *pDmscStatus)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pDmscStatus)
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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pDmscStatus, sizeof(CamDeviceDmscStatus_t));
	packet.payload_size += sizeof(CamDeviceDmscStatus_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_GET_STATUS, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

	memcpy(pDmscStatus, p_data, sizeof(CamDeviceDmscStatus_t));

	return result;
}

RESULT VsiCamDeviceDmscReset(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_RESET, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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

RESULT VsiCamDeviceDmscGetVersion(CamDeviceHandle_t hCamDevice,
								  uint32_t *pVersion)
{
	RESULT result = RET_SUCCESS;

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
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pVersion, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_DMSC_GET_VERSION, &packet,
						  packet.payload_size + payload_extra_size, dest_cpu_id,
						  src_cpu_id);
	if (0 != result)
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
