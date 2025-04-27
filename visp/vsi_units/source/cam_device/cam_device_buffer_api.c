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

/*Copy from cam_device.cpp*/

#include "cam_device_buffer_api.h"
#include "cam_device.h"
#include "cam_device_api.h"
#include "visp_mbox_driver.h"
#include "sensor_cmd.h"
#include "visp_app.h"
#include "kmbox.h"
#include "visp_driver.h"
extern uint32_t cookie;

#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/time.h>
#include <linux/timekeeping.h>

RESULT VsiCamDeviceInitBufChain(struct visp_dev *isp_dev,
								CamDeviceHandle_t hCamDevice,
								CamDeviceBufChainId_t bufId,
								CamDeviceBufChainConfig_t *pConfig)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx || NULL == pConfig)
	{
		return RET_NULL_POINTER;
	}
	pCamDevCtx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet->payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);

	memcpy(p_data, pConfig, sizeof(CamDeviceBufChainConfig_t));
	packet->payload_size += sizeof(CamDeviceBufChainConfig_t);

	if (packet->payload_size > MAX_ITEM)
	{
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_INIT_BUF_CHAIN, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);
	return result;
}

RESULT VsiCamDeviceDeInitBufChain(struct visp_dev *isp_dev,
								  CamDeviceHandle_t hCamDevice,
								  CamDeviceBufChainId_t bufId)
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
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet->payload_size += sizeof(CamDeviceBufChainId_t);

	if (packet->payload_size > MAX_ITEM)
	{
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_DEINIT_BUF_CHAIN, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);

	return result;
}

RESULT VsiCamDeviceCreateBufPool(struct visp_dev *isp_dev,
								 CamDeviceHandle_t hCamDevice,
								 CamDeviceBufChainId_t bufId,
								 CamDeviceBufPoolConfig_t *hBufferPoolCfg)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx || NULL == hBufferPoolCfg)
	{
		return RET_NULL_POINTER;
	}
	pCamDevCtx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	// to solve 64-bit space -> 32-bit space problem, we can not directly copy 	CamDeviceBufPoolSetupCfg_t

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet->payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);

	memcpy(p_data, &(hBufferPoolCfg->bufMode), sizeof(CamDeviceBufMode_t));
	packet->payload_size += sizeof(CamDeviceBufMode_t);
	p_data += sizeof(CamDeviceBufMode_t);

	memcpy(p_data, &(hBufferPoolCfg->bufNum), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(hBufferPoolCfg->bufSize), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	// 6 physical address
	memcpy(p_data, hBufferPoolCfg->pBaseAddrList,
		   hBufferPoolCfg->bufNum * sizeof(uint32_t));
	packet->payload_size += hBufferPoolCfg->bufNum * sizeof(uint32_t);
	p_data += hBufferPoolCfg->bufNum * sizeof(uint32_t);

	memcpy(p_data, &(hBufferPoolCfg->is_mapped), sizeof(bool_t));
	packet->payload_size += sizeof(bool_t);
	p_data += sizeof(bool_t);

	memcpy(p_data, hBufferPoolCfg->pIplAddrList,
		   hBufferPoolCfg->bufNum * sizeof(uint32_t));
	packet->payload_size += hBufferPoolCfg->bufNum * sizeof(uint32_t);
	p_data += hBufferPoolCfg->bufNum * sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM)
	{
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_CREATE_BUFFER_POOL, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);

	return result;
}

RESULT VsiCamDeviceDestroyBufPool(struct visp_dev *isp_dev,
								  CamDeviceHandle_t hCamDevice,
								  CamDeviceBufChainId_t bufId)
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
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet->payload_size += sizeof(CamDeviceBufChainId_t);

	if (packet->payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_DESTORY_BUFFER_POOL, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);

	return result;
}

RESULT VsiCamDeviceSetupBufMgmt(struct visp_dev *isp_dev,
								CamDeviceHandle_t hCamDevice,
								CamDeviceBufChainId_t bufId)
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
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet->payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);

	if (packet->payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_SETUP_BUF_MGMT, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);

	return result;
}

/*****************************************************************************/
/**
 * @brief   This function releases buffer management.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceReleaseBufMgmt(struct visp_dev *isp_dev,
								  CamDeviceHandle_t hCamDevice,
								  CamDeviceBufChainId_t bufId)
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
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet->payload_size += sizeof(CamDeviceBufChainId_t);

	if (packet->payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_RELEASE_BUF_MGMT, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	kfree(packet);

	return result;
}

int DRV_DQ_CNT = 0;
RESULT VsiCamDeviceDeQueBuffer(struct visp_dev *isp_dev,
							   CamDeviceHandle_t hCamDevice,
							   CamDeviceBufChainId_t bufId,
							   MediaBuffer_t **pMediaBuf)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;
	CamDeviceContext_t *pCamDevCtx = NULL;

	DRV_DQ_CNT++;

	*pMediaBuf = kzalloc(sizeof(MediaBuffer_t), GFP_KERNEL);
	if (!pMediaBuf)
	{
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx || NULL == *pMediaBuf)
	{
		return RET_NULL_POINTER;
	}
	pCamDevCtx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	(*pMediaBuf)->pMetaData = kzalloc(sizeof(PicBufMetaData_t), GFP_KERNEL);

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet->payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);

	memcpy(p_data, (*pMediaBuf)->pMetaData, sizeof(PicBufMetaData_t));
	packet->payload_size += sizeof(PicBufMetaData_t);
	p_data += sizeof(PicBufMetaData_t);

	memcpy(p_data, &((*pMediaBuf)->baseAddress), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &((*pMediaBuf)->baseSize), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &((*pMediaBuf)->lockCount), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &((*pMediaBuf)->isFull), sizeof(bool_t));
	packet->payload_size += sizeof(bool_t);
	p_data += sizeof(bool_t);

	memcpy(p_data, &((*pMediaBuf)->index), sizeof(uint8_t));
	packet->payload_size += sizeof(uint8_t);
	p_data += sizeof(uint8_t);

	memcpy(p_data, &((*pMediaBuf)->bufMode), sizeof(BUFF_MODE));
	packet->payload_size += sizeof(BUFF_MODE);
	p_data += sizeof(BUFF_MODE);

	memcpy(p_data, &((*pMediaBuf)->pIplAddress), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &((*pMediaBuf)->pOwner), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_DEQUE_BUFFER, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);

	p_data = packet->payload;
	p_data += (sizeof(uint32_t) + sizeof(CamDeviceBufChainId_t));

	memcpy((*pMediaBuf)->pMetaData, p_data, sizeof(PicBufMetaData_t));
	p_data += sizeof(PicBufMetaData_t);

	memcpy(&((*pMediaBuf)->baseAddress), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&((*pMediaBuf)->baseSize), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&((*pMediaBuf)->lockCount), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&((*pMediaBuf)->isFull), p_data, sizeof(bool_t));
	p_data += sizeof(bool_t);

	memcpy(&((*pMediaBuf)->index), p_data, sizeof(uint8_t));
	p_data += sizeof(uint8_t);

	memcpy(&((*pMediaBuf)->bufMode), p_data, sizeof(BUFF_MODE));
	p_data += sizeof(BUFF_MODE);

	memcpy(&((*pMediaBuf)->pIplAddress), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&((*pMediaBuf)->pOwner), p_data, sizeof(uint32_t));
	kfree(packet);

	return result;
}

int DRV_ENQ_CNT = 0;

#define ENQ_CUSTOM_PAYLOAD_SIZE 48
typedef struct payload_template_enq {
    Payload_type type;
    uint32_t cookie;
    uint32_t payload_size;
    response_field_t resp_field;
    uint8_t payload[ENQ_CUSTOM_PAYLOAD_SIZE];
}payload_packet_enq;


RESULT VsiCamDeviceEnQueBuffer(struct visp_dev *isp_dev,
							   CamDeviceHandle_t hCamDevice,
							   CamDeviceBufChainId_t bufId,
							   OutputBuffer_t *pMediaBuf)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;

	payload_packet_enq packet = {0};

	DRV_ENQ_CNT++;

	if (NULL == pCamDevCtx)
	{
		dev_err(isp_dev->dev, "Null Pcamdevctx\n");
		return RET_NULL_POINTER;
	}
	if (NULL == pMediaBuf)
	{
		dev_err(isp_dev->dev, "Null pMediaBuf\n");
		return RET_NULL_POINTER;
	}

	pCamDevCtx->cookie++;

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	packet.payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);
#if 0

	if(pMediaBuf->pMetaData) {
		kfree(pMediaBuf->pMetaData);
	}
	memcpy(p_data, &(pMediaBuf->baseAddress), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(pMediaBuf->baseSize), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(pMediaBuf->lockCount), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(pMediaBuf->isFull), sizeof(bool_t));
	packet->payload_size += sizeof(bool_t);
	p_data += sizeof(bool_t);
#endif
	memcpy(p_data, &(pMediaBuf->index), sizeof(uint8_t));
	packet.payload_size += sizeof(uint8_t);
	p_data += sizeof(uint8_t);
#if 0
	memcpy(p_data, &(pMediaBuf->bufMode), sizeof(BUFF_MODE));
	packet->payload_size += sizeof(BUFF_MODE);
	p_data += sizeof(BUFF_MODE);

	//dev_err(isp_dev->dev , "The value of pOwner is 0x%x pMediaBuf->pOwner 0x%x\n", *((uint32_t*)p_data),(pMediaBuf)->pOwner);
	memcpy(p_data, &((pMediaBuf)->pIplAddress), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
#endif

	memcpy(p_data, &((pMediaBuf)->pOwner), sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	mutex_lock(&isp_dev->rpu->rpu_lock);

	result = xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_ENQUE_BUFFER, &packet,
		packet.payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
	if (RET_SUCCESS != result )
	{
      return RET_FAILURE;
   }

	mutex_unlock(&isp_dev->rpu->rpu_lock);

	return result;
}

RESULT VsiCamDeviceGetBufferSize(struct visp_dev *isp_dev,
								 CamDeviceHandle_t hCamDevice,
								 CamDeviceBufChainId_t bufId,
								 uint32_t *pBufSize)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	CamDeviceContext_t *pCamDevCtx = NULL;
	uint8_t *p_data = NULL;

	if (NULL == hCamDevice || NULL == pBufSize)
	{
		return RET_NULL_POINTER;
	}

	pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	pCamDevCtx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
	dev_info(isp_dev->dev, "RDMA p_data=%d==%d\n", *p_data, bufId);
	packet->payload_size += sizeof(CamDeviceBufChainId_t);
	p_data += sizeof(CamDeviceBufChainId_t);

	memcpy(p_data, pBufSize, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) return RET_OUTOFRANGE;

	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_GET_BUFFER_SIZE, packet,
            packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);

	memcpy(pBufSize, p_data, sizeof(uint32_t));
	if ((*pBufSize) == 0)
	{
		dev_err(isp_dev->dev, "INVALID BUF SIZE = 0 %d \n", -EINVAL);
		return -EINVAL;
	}

	kfree(packet);
	return result;
}
