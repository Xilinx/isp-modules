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

#include "cam_device_module_api.h"
//#include <cam_engine/cam_engine_hist64_api.h>

USE_TRACER(CAM_DEV_MODULE_INFO);
USE_TRACER(CAM_DEV_MODULE_WARN);
USE_TRACER(CAM_DEV_MODULE_ERR);

RESULT VsiCamDeviceHist64SetConfig(CamDeviceHandle_t hCamDevice,
								   CamDeviceHist64Config_t *pConfig)
{
#if 0
	CamDeviceContext_t *pCamDeviceCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;

	TRACE(CAM_DEV_MODULE_INFO, "%s (enter)\n", __func__);

	if (NULL == pCamDeviceCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pConfig) {
		return (RET_NULL_POINTER);
	}

/*1.set measure window*/
	CamEngineHist64Window_t window;
	MEMSET(&window, 0, sizeof(CamEngineHist64Window_t));

	window.offsetX = pConfig->window.hOffset;
	window.offsetY = pConfig->window.vOffset;
	window.width = pConfig->window.hSize;
	window.height = pConfig->window.vSize;
	result = CamEngineHist64SetMeasuringWindow(pCamDeviceCtx->hCamEngine, &window);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64SetMeasuringWindow failed(%d)\n",  __func__, result);
		return result;
	}
/*2.set weight*/
	CamEngineHist64Weight_t weight;
	MEMSET(&weight, 0, sizeof(CamEngineHist64Weight_t));

	DCT_ASSERT(sizeof(weight.weight) == sizeof(pConfig->weight));
	MEMCPY(weight.weight, pConfig->weight, sizeof(pConfig->weight));
	result = CamEngineHist64SetGridWeights(pCamDeviceCtx->hCamEngine, &weight);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64SetGridWeights failed(%d)\n",  __func__, result);
		return result;
	}
/*3.set mode*/
	CamEngineHist64Mode_t mode;
	switch(pConfig->mode) {
		case CAMDEV_HIST64_MODE_DISABLE:
			mode = CAM_ENGINE_HIST64_MODE_DISABLE;
			break;
		case CAMDEV_HIST64_MODE_ONE_FROM_YRGB:
			mode = CAM_ENGINE_HIST64_MODE_ONE_FROM_YRGB;
			break;
		case CAMDEV_HIST64_MODE_R:
			mode = CAM_ENGINE_HIST64_MODE_R;
			break;
		case CAMDEV_HIST64_MODE_GR:
			mode = CAM_ENGINE_HIST64_MODE_GR;
			break;
		case CAMDEV_HIST64_MODE_B:
			mode = CAM_ENGINE_HIST64_MODE_B;
			break;
		case CAMDEV_HIST64_MODE_GB:
			mode = CAM_ENGINE_HIST64_MODE_GB;
			break;
		default:
			TRACE(CAM_DEV_MODULE_ERR,  "%s: input hist error(%d)\n",  __func__, pConfig->mode);
		return RET_FAILURE;
	}

	result = CamEngineHist64SetMeasuringMode(pCamDeviceCtx->hCamEngine, mode);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64SetMeasuringMode failed(%d)\n",  __func__, result);
		return result;
	}
/*4.set sub-sample range*/
	uint8_t shift = pConfig->shift;
	uint16_t offset = pConfig->offset;

	result = CamEngineHist64SetSubSampleRange(pCamDeviceCtx->hCamEngine, shift, offset);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64SetSubSampleRange failed(%d)\n",  __func__, result);
		return result;
	}
/*5.set coefficient */
	float rCoeff = pConfig->rCoeff;
	float gCoeff = pConfig->gCoeff;
	float bCoeff = pConfig->bCoeff;

	result = CamEngineHist64SetCoeff(pCamDeviceCtx->hCamEngine, rCoeff, gCoeff, bCoeff);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64SetCoeff failed(%d)\n",  __func__, result);
		return result;
	}
/*6.set channel*/
	CamEngineHist64Channel_t channel;
	switch(pConfig->channel) {
		case CAMDEV_HIST64_CHANNEL_0:
			channel = CAM_ENGINE_HIST64_CHANNEL_0;
			break;
		case CAMDEV_HIST64_CHANNEL_1:
			channel = CAM_ENGINE_HIST64_CHANNEL_1;
			break;
		case CAMDEV_HIST64_CHANNEL_2:
			channel = CAM_ENGINE_HIST64_CHANNEL_2;
			break;
		case CAMDEV_HIST64_CHANNEL_3:
			channel = CAM_ENGINE_HIST64_CHANNEL_3;
			break;
		case CAMDEV_HIST64_CHANNEL_4:
			channel = CAM_ENGINE_HIST64_CHANNEL_4;
			break;
		case CAMDEV_HIST64_CHANNEL_5:
			channel = CAM_ENGINE_HIST64_CHANNEL_5;
			break;
		case CAMDEV_HIST64_CHANNEL_6:
			channel = CAM_ENGINE_HIST64_CHANNEL_6;
			break;
		case CAMDEV_HIST64_CHANNEL_7:
			channel = CAM_ENGINE_HIST64_CHANNEL_7;
			break;
		default:
			TRACE(CAM_DEV_MODULE_ERR,  "%s: input hist error(%d)\n",  __func__, pConfig->channel);
		return RET_FAILURE;
	}

	result = CamEngineHist64SetMeasuringChannel(pCamDeviceCtx->hCamEngine, channel);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64SetMeasuringChannel failed(%d)\n",  __func__, result);
		return result;
	}

	pCamDeviceCtx->moduleCtx.histCtx.version = MODULE_V1;

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist64GetConfig(CamDeviceHandle_t hCamDevice,
								   CamDeviceHist64Config_t *pConfig)
{
#if 0
	CamDeviceContext_t *pCamDeviceCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;

	TRACE(CAM_DEV_MODULE_INFO, "%s (enter)\n", __func__);

	if (NULL == pCamDeviceCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pConfig) {
		return (RET_NULL_POINTER);
	}

/*1.get measure window*/
	CamEngineHist64Window_t window;
	MEMSET(&window, 0, sizeof(CamEngineHist64Window_t));

	result = CamEngineHist64GetMeasuringWindow(pCamDeviceCtx->hCamEngine, &window);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64GetMeasuringWindow failed(%d)\n",  __func__, result);
		return result;
	}
	pConfig->window.hOffset = window.offsetX;
	pConfig->window.vOffset = window.offsetY;
	pConfig->window.hSize = window.width;
	pConfig->window.vSize = window.height;
/*2.get weight*/
	CamEngineHist64Weight_t weight;
	MEMSET(&weight, 0, sizeof(CamEngineHist64Weight_t));

	result = CamEngineHist64GetGridWeights(pCamDeviceCtx->hCamEngine, &weight);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64GetGridWeights failed(%d)\n",  __func__, result);
		return result;
	}
	DCT_ASSERT(sizeof(weight.weight) == sizeof(pConfig->weight));
	MEMCPY(pConfig->weight, weight.weight, sizeof(weight.weight));
/*3.get mode*/
	CamEngineHist64Mode_t mode;

	result = CamEngineHist64GetMeasuringMode(pCamDeviceCtx->hCamEngine, &mode);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64GetMeasuringMode failed(%d)\n",  __func__, result);
		return result;
	}
	switch(mode) {
		case CAM_ENGINE_HIST64_MODE_DISABLE:
			pConfig->mode = CAMDEV_HIST64_MODE_DISABLE;
			break;
		case CAM_ENGINE_HIST64_MODE_ONE_FROM_YRGB:
			pConfig->mode = CAMDEV_HIST64_MODE_ONE_FROM_YRGB;
			break;
		case CAM_ENGINE_HIST64_MODE_R:
			pConfig->mode = CAMDEV_HIST64_MODE_R;
			break;
		case CAM_ENGINE_HIST64_MODE_GR:
			pConfig->mode = CAMDEV_HIST64_MODE_GR;
			break;
		case CAM_ENGINE_HIST64_MODE_B:
			pConfig->mode = CAMDEV_HIST64_MODE_B;
			break;
		case CAM_ENGINE_HIST64_MODE_GB:
			pConfig->mode = CAMDEV_HIST64_MODE_GB;
			break;
		default:
			TRACE(CAM_DEV_MODULE_ERR,  "%s: input hist error(%d)\n",  __func__, mode);
		return RET_FAILURE;
	}
/*4.get sub-sample range*/
	uint8_t shift;
	uint16_t offset;

	result = CamEngineHist64GetSubSampleRange(pCamDeviceCtx->hCamEngine, &shift, &offset);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64GetSubSampleRange failed(%d)\n",  __func__, result);
		return result;
	}
	pConfig->shift = shift;
	pConfig->offset = offset;
/*5.get coefficient */
	float rCoeff;
	float gCoeff;
	float bCoeff;

	result = CamEngineHist64GetCoeff(pCamDeviceCtx->hCamEngine, &rCoeff, &gCoeff, &bCoeff);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64GetCoeff failed(%d)\n",  __func__, result);
		return result;
	}
	pConfig->rCoeff = rCoeff;
	pConfig->gCoeff = gCoeff;
	pConfig->bCoeff = bCoeff;
/*6.get channel*/
	CamEngineHist64Channel_t channel;

	result = CamEngineHist64GetMeasuringChannel(pCamDeviceCtx->hCamEngine, &channel);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64GetMeasuringChannel failed(%d)\n",  __func__, result);
		return result;
	}
	switch(channel) {
		case CAM_ENGINE_HIST64_CHANNEL_0:
			pConfig->channel = CAMDEV_HIST64_CHANNEL_0;
			break;
		case CAM_ENGINE_HIST64_CHANNEL_1:
			pConfig->channel = CAMDEV_HIST64_CHANNEL_1;
			break;
		case CAM_ENGINE_HIST64_CHANNEL_2:
			pConfig->channel = CAMDEV_HIST64_CHANNEL_2;
			break;
		case CAM_ENGINE_HIST64_CHANNEL_3:
			pConfig->channel = CAMDEV_HIST64_CHANNEL_3;
			break;
		case CAM_ENGINE_HIST64_CHANNEL_4:
			pConfig->channel = CAMDEV_HIST64_CHANNEL_4;
			break;
		case CAM_ENGINE_HIST64_CHANNEL_5:
			pConfig->channel = CAMDEV_HIST64_CHANNEL_5;
			break;
		case CAM_ENGINE_HIST64_CHANNEL_6:
			pConfig->channel = CAMDEV_HIST64_CHANNEL_6;
			break;
		case CAM_ENGINE_HIST64_CHANNEL_7:
			pConfig->channel = CAMDEV_HIST64_CHANNEL_7;
			break;
		default:
			TRACE(CAM_DEV_MODULE_ERR,  "%s: input hist error(%d)\n",  __func__, channel);
		return RET_FAILURE;
	}

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist64Enable(CamDeviceHandle_t hCamDevice)
{
#if 0
	CamDeviceContext_t *pCamDeviceCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;

	TRACE(CAM_DEV_MODULE_INFO, "%s (enter)\n", __func__);

	if (NULL == pCamDeviceCtx) {
		return (RET_WRONG_HANDLE);
	}

	result = CamengineHist64Enable(pCamDeviceCtx->hCamEngine);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamengineHist64Enable failed(%d)\n",  __func__, result);
		return result;
	}

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist64Disable(CamDeviceHandle_t hCamDevice)
{
#if 0
	CamDeviceContext_t *pCamDeviceCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;

	TRACE(CAM_DEV_MODULE_INFO, "%s (enter)\n", __func__);

	if (NULL == pCamDeviceCtx) {
		return (RET_WRONG_HANDLE);
	}

	result = CamEngineHist64Disable(pCamDeviceCtx->hCamEngine);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64Disable failed(%d)\n",  __func__, result);
		return result;
	}

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist64SetMeasureWindow(CamDeviceHandle_t hCamDevice,
										  CamDeviceWindow_t *pWindow)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist64GetMeasureWindow(CamDeviceHandle_t hCamDevice,
										  CamDeviceWindow_t *pWindow)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist64ForceUpdate(CamDeviceHandle_t hCamDevice)
{
#if 0
	CamDeviceContext_t *pCamDeviceCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;

	TRACE(CAM_DEV_MODULE_INFO, "%s (enter)\n", __func__);

	if (NULL == pCamDeviceCtx) {
		return (RET_WRONG_HANDLE);
	}

	result = CamEngineHist64FourceUpdate(pCamDeviceCtx->hCamEngine);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64FourceUpdate failed(%d)\n",  __func__, result);
		return result;
	}

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist64GetBins(CamDeviceHandle_t hCamDevice,
								 CamDeviceHist64Bins_t *pHistBin)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist64GetStatus(CamDeviceHandle_t hCamDevice,
								   CamDeviceHist64Status_t *pStatus)
{
#if 0
	CamDeviceContext_t *pCamDeviceCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;

	TRACE(CAM_DEV_MODULE_INFO, "%s (enter)\n", __func__);

	if (NULL == pCamDeviceCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pStatus) {
		return (RET_NULL_POINTER);
	}

	bool_t enable;

	result = CamEngineHist64IsEnabled(pCamDeviceCtx->hCamEngine, &enable);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHist64IsEnabled failed(%d)\n",  __func__, result);
		return result;
	}

	pStatus->enable = enable;

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist64GetVersion(CamDeviceHandle_t hCamDevice,
									uint32_t *pVersion)
{
#if 0
	CamDeviceContext_t *pCamDeviceCtx = (CamDeviceContext_t *)hCamDevice;

	TRACE(CAM_DEV_MODULE_INFO, "%s (enter)\n", __func__);

	if (NULL == pCamDeviceCtx) {
		return (RET_WRONG_HANDLE);
	}
	if (NULL == pVersion) {
		return (RET_NULL_POINTER);
	}

	*pVersion = MODULE_V1;

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist64Reset(CamDeviceHandle_t hCamDevice)
{
	return RET_SUCCESS;
}
