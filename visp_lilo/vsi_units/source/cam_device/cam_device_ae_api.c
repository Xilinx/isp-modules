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

#include <linux/delay.h>
#include "cam_device.h"
#include "visp_mbox_driver.h"
#include "visp_common.h"
#include "sensor_cmd.h"
#include "visp_driver.h"
#include <linux/string.h>
#include "kmbox.h"

RESULT VsiCamDeviceUnRegisterAeLib(struct visp_dev *isp_dev,
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
		pr_err("FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	xlnx_send_mbox_acked_cmd(isp_dev, RPU_2_APU_MB_CMD_UNREGISTER_AELIB, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);

	kfree(packet);

	return result;
}

RESULT VsiCamDeviceAeDisable(struct visp_dev *isp_dev,
							 CamDeviceHandle_t hCamDevice)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;

	CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
	if (NULL == pCamDevCtx)
	{
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
	{
		pr_err("FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM)
	{
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_AE_DISABLE, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);

	kfree(packet);

	return result;
}
