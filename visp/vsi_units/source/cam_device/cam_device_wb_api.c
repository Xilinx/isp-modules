/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
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
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

#include "cam_device.h"
// #include "mailbox.h"

RESULT VsiCamDeviceWbSetConfig(CamDeviceHandle_t hCamDevice,
							   CamDeviceWbConfig_t *pWbCfg)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
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
	memcpy(p_data, pWbCfg, sizeof(CamDeviceWbConfig_t));
	packet.payload_size += sizeof(CamDeviceWbConfig_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_WB_SET_CONFIG, &packet,
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

RESULT VsiCamDeviceWbGetConfig(CamDeviceHandle_t hCamDevice,
							   CamDeviceWbConfig_t *pWbCfg)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
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
	memcpy(p_data, pWbCfg, sizeof(CamDeviceWbConfig_t));
	packet.payload_size += sizeof(CamDeviceWbConfig_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_WB_GET_CONFIG, &packet,
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

	memcpy(pWbCfg, p_data, sizeof(CamDeviceWbConfig_t));

	return result;
}

RESULT VsiCamDeviceWbEnable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
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

	result = Send_Command(APU_2_RPU_MB_CMD_WB_ENABLE, &packet,
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

RESULT VsiCamDeviceWbDisable(CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
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

	result = Send_Command(APU_2_RPU_MB_CMD_WB_DISABLE, &packet,
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

RESULT VsiCamDeviceWbGetStatus(CamDeviceHandle_t hCamDevice,
							   CamDeviceWbStatus_t *pWbStatus)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pWbStatus)
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
	memcpy(p_data, pWbStatus, sizeof(CamDeviceWbStatus_t));
	packet.payload_size += sizeof(CamDeviceWbStatus_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command(APU_2_RPU_MB_CMD_WB_GET_STATUS, &packet,
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

	memcpy(pWbStatus, p_data, sizeof(CamDeviceWbStatus_t));

	return result;
}

RESULT VsiCamDeviceWbReset(CamDeviceHandle_t hCamDevice)
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

	result = Send_Command(APU_2_RPU_MB_CMD_WB_RESET, &packet,
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

RESULT VsiCamDeviceWbGetVersion(CamDeviceHandle_t hCamDevice,
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

	result = Send_Command(APU_2_RPU_MB_CMD_WB_GET_VERSION, &packet,
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
