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

static RESULT CamDeviceRgbirSetInternalConfig(CamDeviceHandle_t hCamDevice)
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

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_SET_INTERNAL_CONFIG, &packet,
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

RESULT VsiCamDeviceRgbirSetConfig(CamDeviceHandle_t hCamDevice,
								  CamDeviceRgbirConfig_t *pRgbirCfg)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pRgbirCfg)
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
	memcpy(p_data, pRgbirCfg, sizeof(CamDeviceRgbirConfig_t));
	packet.payload_size += sizeof(CamDeviceRgbirConfig_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_SET_CONFIG, &packet,
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

RESULT VsiCamDeviceRgbirGetConfig(CamDeviceHandle_t hCamDevice,
								  CamDeviceRgbirConfig_t *pRgbirCfg)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pRgbirCfg)
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
	memcpy(p_data, pRgbirCfg, sizeof(CamDeviceRgbirConfig_t));
	packet.payload_size += sizeof(CamDeviceRgbirConfig_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_GET_CONFIG, &packet,
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

	memcpy(pRgbirCfg, p_data, sizeof(CamDeviceRgbirConfig_t));

	return result;
}

RESULT VsiCamDeviceRgbirEnable(CamDeviceHandle_t hCamDevice)
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

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_ENABLE, &packet,
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

RESULT VsiCamDeviceRgbirDisable(CamDeviceHandle_t hCamDevice)
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

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_DISABLE, &packet,
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

RESULT VsiCamDeviceRgbirRcccEnable(CamDeviceHandle_t hCamDevice)
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

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_RCCC_ENABLE, &packet,
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

RESULT VsiCamDeviceRgbirRcccDisable(CamDeviceHandle_t hCamDevice)
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

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_RCCC_DISABLE, &packet,
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

RESULT VsiCamDeviceRgbirIrRawOutEnable(CamDeviceHandle_t hCamDevice)
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

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_IR_RAWOUT_ENABLE, &packet,
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

RESULT VsiCamDeviceRgbirIrRawOutDisable(CamDeviceHandle_t hCamDevice)
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

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_IR_RAWOUT_DISABLE, &packet,
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

RESULT VsiCamDeviceRgbirSetOutPattern(CamDeviceHandle_t hCamDevice,
									  CamDeviceRgbirOutPat_t outPattern)
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
	memcpy(p_data, &outPattern, sizeof(CamDeviceRgbirOutPat_t));
	packet.payload_size += sizeof(CamDeviceRgbirOutPat_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_SET_OutPattern, &packet,
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

RESULT VsiCamDeviceRgbirGetOutPattern(CamDeviceHandle_t hCamDevice,
									  CamDeviceRgbirOutPat_t *pOutPattern)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pOutPattern)
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
	memcpy(p_data, pOutPattern, sizeof(CamDeviceRgbirOutPat_t));
	packet.payload_size += sizeof(CamDeviceRgbirOutPat_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_GET_OutPattern, &packet,
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

	memcpy(pOutPattern, p_data, sizeof(CamDeviceRgbirOutPat_t));

	return result;
}

RESULT VsiCamDeviceRgbirSetIrPathSelect(CamDeviceHandle_t hCamDevice,
										CamDeviceRgbirIrPathSel_t irPathSelect)
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
	memcpy(p_data, &irPathSelect, sizeof(CamDeviceRgbirIrPathSel_t));
	packet.payload_size += sizeof(CamDeviceRgbirIrPathSel_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_SET_IrPathSelect, &packet,
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

RESULT VsiCamDeviceRgbirGetIrPathSelect(
	CamDeviceHandle_t hCamDevice, CamDeviceRgbirIrPathSel_t *pIrPathSelect)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pIrPathSelect)
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
	memcpy(p_data, pIrPathSelect, sizeof(CamDeviceRgbirIrPathSel_t));
	packet.payload_size += sizeof(CamDeviceRgbirIrPathSel_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_GET_IrPathSelect, &packet,
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

	memcpy(pIrPathSelect, p_data, sizeof(CamDeviceRgbirIrPathSel_t));

	return result;
}

RESULT VsiCamDeviceRgbirGetStatus(CamDeviceHandle_t hCamDevice,
								  CamDeviceRgbirStatus_t *pRgbirStatus)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pRgbirStatus)
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
	memcpy(p_data, pRgbirStatus, sizeof(CamDeviceRgbirStatus_t));
	packet.payload_size += sizeof(CamDeviceRgbirStatus_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_GET_STATUS, &packet,
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

	memcpy(pRgbirStatus, p_data, sizeof(CamDeviceRgbirStatus_t));

	return result;
}

RESULT VsiCamDeviceRgbirGetVersion(CamDeviceHandle_t hCamDevice,
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

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_GET_VERSION, &packet,
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

RESULT VsiCamDeviceRgbirReset(CamDeviceHandle_t hCamDevice)
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

	result = Send_Command(APU_2_RPU_MB_CMD_RGBIR_RESET, &packet,
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
