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

#include "cam_device_sensor_api.h"
#include "cam_device_api.h"
#include "cam_device.h"
// #include "mailbox.h" //added by ranjith
#include "isi_iss.h"
#include "visp_mbox_driver.h"
#include "sensor_cmd.h"
#include <linux/slab.h>
#include <linux/string.h>
#include "kmbox.h"
#include "visp_driver.h"

#define SENSOR_NAME_LEN 20

RESULT VsiCamDeviceSensorOpen(struct visp_dev *isp_dev,
							  CamDeviceHandle_t hCamDevice, uint32_t modeIndex)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev,"FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;

	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, &modeIndex, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM)
	{
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
				packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_OPEN, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);

	return result;
}

RESULT VsiCamDeviceSensorDrvHandleRegister(
	struct visp_dev *isp_dev, CamDeviceHandle_t hCamDevice,
	const CamDeviceSensorDrvCfg_t *pSensorDrv)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;
	IsiCamDrvConfigMbox_t *pcamcfg = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx || NULL == pSensorDrv)
	{
		return (RET_WRONG_HANDLE);
	}

	pCamDevCtx->cookie++;
	//pCamDevCtx->cookie = 44;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev,"FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	pcamcfg = (IsiCamDrvConfigMbox_t *)(pSensorDrv->sensorDrvHandle);

	pcamcfg->instanceId = pCamDevCtx->instanceId;
	p_data = packet->payload;

	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, pSensorDrv->sensorDrvHandle, sizeof(IsiCamDrvConfigMbox_t));
	p_data += sizeof(IsiCamDrvConfigMbox_t);
	packet->payload_size += sizeof(IsiCamDrvConfigMbox_t);

	memcpy(p_data, &pSensorDrv->sensorDevId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	if (packet->payload_size > MAX_ITEM)
	{
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
				packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_REG, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);
	return result;
}

RESULT VsiCamDeviceSensorDrvHandleUnRegister(struct visp_dev *isp_dev,
											 CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev,"FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM)
	{
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
				packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_UNREG, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);
	return result;
}

RESULT VsiCamDeviceSensorClose(struct visp_dev *isp_dev,
							   CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie++;

	packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev,"FAILED TO KMALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	memset(packet, 0, sizeof(payload_packet));

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM)
	{
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
				packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_CLOSE, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }
	kfree(packet);
	return result;
}
RESULT VsiCamDeviceSensorMapping(struct visp_dev *isp_dev,
								 CamDeviceHandle_t hCamDevice,
								 const char *pSensorName,
								 CamDeviceSensorDrvHandle_t *pSensorDrvhandle)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;
	IsiCamDrvConfigMbox_t camcfg; //= NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pSensorName || NULL == pSensorDrvhandle)
	{
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev,"FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, pSensorName, SENSOR_NAME_LEN);
	p_data += SENSOR_NAME_LEN;
	packet->payload_size += SENSOR_NAME_LEN;

	// memcpy(p_data, pSensorDrvhandle, sizeof(uint32_t));
	packet->payload_size += sizeof(IsiCamDrvConfigMbox_t);

	if (packet->payload_size > MAX_ITEM)
	{
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
				packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_MAPPING, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);

#if 0
	mutex_lock(&isp_dev->rpu->write_lock);
	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_MAPPING, packet,
						  packet->payload_size + payload_extra_size,
						  isp_dev->isp_rpu, MBOX_CORE_APU);
	if (0 != result)
	{
		kfree(packet);
		mutex_unlock(&isp_dev->rpu->write_lock);
		return result;
	}
	mbox_send_message(isp_dev->tx_chan, NULL);

	__result = xlnx_mbox_apu_wait_for_data(isp_dev, packet->payload);
	if (__result)
	{
		kfree(packet);
		pr_err("FAiled in Sensor Mapping\n");
		mutex_unlock(&isp_dev->rpu->write_lock);
		return -1;
	}
	mutex_unlock(&isp_dev->rpu->write_lock);
#endif
	*pSensorDrvhandle = kzalloc(sizeof(IsiCamDrvConfigMbox_t), GFP_KERNEL);

	if (*pSensorDrvhandle == NULL)
	{
		kfree(packet);
		dev_err(isp_dev->dev,
				"APU Failed to allocate memory for sensor mapping.\n");
	//	mutex_unlock(&isp_dev->rpu->write_lock);
		return RET_OUTOFMEM;
	}
	memcpy(*pSensorDrvhandle, p_data, sizeof(IsiCamDrvConfigMbox_t));

	//RKC - free above psensorDrvhanlde ?
	kfree(packet);

	//camcfg = **(IsiCamDrvConfigMbox_t **)pSensorDrvhandle;
	camcfg = **(IsiCamDrvConfigMbox_t **)pSensorDrvhandle;

	return result;
}

RESULT VsiCamDeviceSensorQuery(struct visp_dev *isp_dev,
							   CamDeviceHandle_t hCamDevice,
							   CamDeviceSensorQuery_t *pQuery)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pQuery)
	{
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;

	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, pQuery, sizeof(CamDeviceSensorQuery_t));
	packet->payload_size += sizeof(CamDeviceSensorQuery_t);

	if (packet->payload_size > MAX_ITEM)
	{
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
				packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_QUERY, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);

#if 0
	mutex_lock(&isp_dev->rpu->write_lock);
	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_QUERY, packet,
						  packet->payload_size + payload_extra_size,
						  isp_dev->isp_rpu, MBOX_CORE_APU);
	if (0 != result)
	{
		kfree(packet);
		mutex_unlock(&isp_dev->rpu->write_lock);
		return result;
	}

	mbox_send_message(isp_dev->tx_chan, NULL);

	__result = xlnx_mbox_apu_wait_for_data(isp_dev, packet->payload);
	if (__result)
	{
		kfree(packet);
		mutex_unlock(&isp_dev->rpu->write_lock);
		pr_err("FAILED TO QUERY SENSOR\n");
		return -1;
	}

	mutex_unlock(&isp_dev->rpu->write_lock);
#endif
	memcpy(pQuery, p_data, sizeof(CamDeviceSensorQuery_t));

	kfree(packet);
	return result;
}

#if 0
RESULT VsiCamDeviceSensorGetInfo
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorInfo_t *pSensorInfo
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pSensorInfo) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	//memcpy(p_data, pSensorInfo, sizeof(CamDeviceSensorInfo_t));
	packet.payload_size += sizeof(CamDeviceSensorInfo_t);

		if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_INFO, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pSensorInfo, p_data, sizeof(CamDeviceSensorInfo_t));

	return result;
}
#endif

RESULT VsiCamDeviceSensorSetTestPattern(
	struct visp_dev *isp_dev, CamDeviceHandle_t hCamDevice,
	const CamDeviceSensorTestPattern_t *pTestPattern)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pTestPattern)
	{
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev,"FAILED TO KMALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;

	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, pTestPattern, sizeof(CamDeviceSensorTestPattern_t));
	packet->payload_size += sizeof(CamDeviceSensorTestPattern_t);

	if (packet->payload_size > MAX_ITEM)
	{
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
				packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_SET_TP, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);
	return result;
}

#if 0

RESULT VsiCamDeviceSensorSetRegister
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorRegister_t *pRegister
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pRegister) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pRegister, sizeof(CamDeviceSensorRegister_t));
	packet.payload_size += sizeof(CamDeviceSensorRegister_t);

		if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_REG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceSensorGetRegister
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorRegister_t *pRegister
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pRegister) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pRegister, sizeof(CamDeviceSensorRegister_t));
	packet.payload_size += sizeof(CamDeviceSensorRegister_t);

		if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_REG, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pRegister, p_data, sizeof(CamDeviceSensorRegister_t));

	return result;
}
#endif

RESULT VsiCamDeviceSensorSetFrameRate(struct visp_dev *isp_dev,
									  CamDeviceHandle_t hCamDevice,
									  uint32_t * /*float **/ pFps)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pFps)
	{
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie++;

	packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev,"FAILED TO KMALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	memset(packet, 0, sizeof(payload_packet));

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, pFps, sizeof(/*float*/ uint32_t));
	packet->payload_size += sizeof(/*float*/ uint32_t);

	if (packet->payload_size > MAX_ITEM)
	{
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
				packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_SET_FRAMERATE, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);

	return result;
}

#if 0
RESULT VsiCamDeviceSensorGetFrameRate
(
	CamDeviceHandle_t hCamDevice,
	float *pFps
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pFps) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pFps, sizeof(float));
	packet.payload_size += sizeof(float);

		if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_FRAMERATE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pFps, p_data, sizeof(float));

	return result;
}


RESULT VsiCamDeviceSensorGetModeInfo
(
	CamDeviceHandle_t          hCamDevice,
	CamDeviceSensorModeInfo_t *pMode
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pMode) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pMode, sizeof(CamDeviceSensorModeInfo_t));
	packet.payload_size += sizeof(CamDeviceSensorModeInfo_t);

		if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_MODE_INFO, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pMode, p_data, sizeof(CamDeviceSensorModeInfo_t));
	return result;
}


RESULT VsiCamDeviceSensorGetStatus
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorStatus_t *pStatus
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pStatus) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pStatus, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

		if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_STATUS, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pStatus, p_data, sizeof(CamDeviceSensorStatus_t));

	return result;
}


RESULT VsiCamDeviceSensorSetGain
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorGain_t *pGain
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pGain) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pGain, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

		if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_GAIN, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceSensorGetGain
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorGain_t *pGain
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pGain) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pGain, sizeof(CamDeviceSensorGain_t));
	packet.payload_size += sizeof(CamDeviceSensorGain_t);

		if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_GAIN, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pGain, p_data, sizeof(CamDeviceSensorGain_t));

	return result;
}


RESULT VsiCamDeviceSensorSetExposureControl
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorExposureControl_t  *pExpCtrl
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pExpCtrl) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pExpCtrl, sizeof(CamDeviceSensorExposureControl_t));
	packet.payload_size += sizeof(CamDeviceSensorExposureControl_t);

	if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_EXP_CTL, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceSensorGetExposureControl
(
   CamDeviceHandle_t                  hCamDevice,
   CamDeviceSensorExposureControl_t  *pExpCtrl

)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pExpCtrl) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pExpCtrl, sizeof(CamDeviceSensorExposureControl_t));
	packet.payload_size += sizeof(float);

	if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_EXP_CTL, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pExpCtrl, p_data, sizeof(CamDeviceSensorExposureControl_t));

	return result;
}


RESULT VsiCamDeviceSensorSetFocusPositon
(
   CamDeviceHandle_t hCamDevice,
   CamDeviceSensorFocusPos_t  *pFocusPos
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pFocusPos) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pFocusPos, sizeof(CamDeviceSensorFocusPos_t));
	packet.payload_size += sizeof(CamDeviceSensorFocusPos_t);

	if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SetFocusPositon, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);

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

RESULT VsiCamDeviceSensorGetFocusPositon
(
   CamDeviceHandle_t hCamDevice,
   CamDeviceSensorFocusPos_t  *pFocusPos,
   CamDeviceSensorIntegerRange_t *pRangeLimit
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pFocusPos || NULL == pRangeLimit) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);

	memcpy(p_data, pFocusPos, sizeof(CamDeviceSensorFocusPos_t));
	p_data += sizeof(CamDeviceSensorFocusPos_t);
	packet.payload_size += sizeof(CamDeviceSensorFocusPos_t);

	memcpy(p_data, pRangeLimit, sizeof(CamDeviceSensorIntegerRange_t));
	packet.payload_size += sizeof(CamDeviceSensorIntegerRange_t);

	if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GetFocusPositon, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pRangeLimit, p_data, sizeof(CamDeviceSensorIntegerRange_t));

	p_data -= sizeof(CamDeviceSensorFocusPos_t);
	memcpy(pFocusPos, p_data, sizeof(CamDeviceSensorFocusPos_t));

	return result;
}


RESULT VsiCamDeviceSensorSetStreaming
(
	CamDeviceHandle_t hCamDevice,
	bool_t isEnable
)
{
	RESULT result = RET_SUCCESS;

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

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, &isEnable, sizeof(bool_t));
	packet.payload_size += sizeof(bool_t);

		if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_STREAMING, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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


RESULT VsiCamDeviceSensorGetOtpInfo
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorOtpModuleInfo_t *pOtpModuleInfo
)
{
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pOtpModuleInfo) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pOtpModuleInfo, sizeof(CamDeviceSensorOtpModuleInfo_t));
	packet.payload_size += sizeof(CamDeviceSensorOtpModuleInfo_t);

		if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_OPT_INFO, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pOtpModuleInfo, p_data, sizeof(CamDeviceSensorOtpModuleInfo_t));

	return result;
}

RESULT VsiCamDeviceSensorGetMetadataAttr
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorMetadataAttr_t *pMetaAttr
){
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pMetaAttr) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pMetaAttr, sizeof(CamDeviceSensorMetadataAttr_t));
	packet.payload_size += sizeof(CamDeviceSensorMetadataAttr_t);

	if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_METADATA_ATTR, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pMetaAttr, p_data, sizeof(CamDeviceSensorMetadataAttr_t));

	return result;
}


RESULT VsiCamDeviceSensorSetMetadataAttr
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorMetadataAttr_t metaAttr
){
	RESULT result = RET_SUCCESS;

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

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, &metaAttr, sizeof(CamDeviceSensorMetadataAttr_t));
	packet.payload_size += sizeof(CamDeviceSensorMetadataAttr_t);

	if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_SET_METADATA_ATTR, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

RESULT VsiCamDeviceSensorGetMetadataWin
(
	CamDeviceHandle_t hCamDevice,
	CamDeviceSensorMetadataWin_t *pMetaWin
){
	RESULT result = RET_SUCCESS;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pMetaWin) {
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet.payload_size += sizeof(uint32_t);
	memcpy(p_data, pMetaWin, sizeof(CamDeviceSensorMetadataWin_t));
	packet.payload_size += sizeof(CamDeviceSensorMetadataWin_t);

	if(packet.payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	os_lock_mutex(&mbox_lock);

	result = Send_Command (APU_2_RPU_MB_CMD_SENSOR_GET_METADATA_WIN, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(0 != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_mb_data(packet.cookie, packet.payload);
	if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
	}

	os_unlock_mutex(&mbox_lock);

	memcpy(pMetaWin, p_data, sizeof(CamDeviceSensorMetadataWin_t));

	return result;
}

#endif

RESULT VsiCamDeviceSensorGetConnectPortInfo(
	struct visp_dev *isp_dev, CamDeviceHandle_t hCamDevice,
	CamDeviceMcmPortId_t portId, CamDeviceSensorConnectPortInfo_t *pPortInfo)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pPortInfo)
	{
		return (RET_NULL_POINTER);
	}
	pCamDevCtx->cookie++;

	packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev,"FAILED TO KMALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	memset(packet, 0, sizeof(payload_packet));

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, &portId, sizeof(CamDeviceMcmPortId_t));
	p_data += sizeof(CamDeviceMcmPortId_t);
	packet->payload_size += sizeof(CamDeviceMcmPortId_t);

	memcpy(p_data, pPortInfo, sizeof(CamDeviceSensorConnectPortInfo_t));
	packet->payload_size += sizeof(CamDeviceSensorConnectPortInfo_t);

	if (packet->payload_size > MAX_ITEM)
	{
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
				packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_GET_ConnectPortInfo, packet,
			packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
#if 0
	mutex_lock(&isp_dev->rpu->write_lock);
	result = Send_Command(APU_2_RPU_MB_CMD_SENSOR_GET_ConnectPortInfo, packet,
						  packet->payload_size + payload_extra_size,
						  isp_dev->isp_rpu, MBOX_CORE_APU);
	if (0 != result)
	{
		kfree(packet);
		mutex_unlock(&isp_dev->rpu->write_lock);
		return result;
	}
	mbox_send_message(isp_dev->tx_chan, NULL);

	result = xlnx_mbox_apu_wait_for_data(isp_dev, packet->payload);
	if (result)
	{
		kfree(packet);
		mutex_unlock(&isp_dev->rpu->write_lock);
		pr_err("FAiled in APU_2_RPU_MB_CMD_SENSOR_GET_ConnectPortInfo\n");
		return -1;
	}
	mutex_unlock(&isp_dev->rpu->write_lock);
#endif

	memcpy(pPortInfo, p_data, sizeof(CamDeviceSensorConnectPortInfo_t));
	pPortInfo->name[sizeof(pPortInfo->name) - 1] = '\0';

	kfree(packet);

	return result;
}
