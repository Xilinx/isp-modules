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

#include "cam_device_module_api.h"
//#include <cam_engine/cam_engine_hist_api.h>

USE_TRACER(CAM_DEV_MODULE_INFO);
USE_TRACER(CAM_DEV_MODULE_WARN);
USE_TRACER(CAM_DEV_MODULE_ERR);

RESULT VsiCamDeviceHist256SetConfig(CamDeviceHandle_t hCamDevice,
									CamDeviceHist256Config_t *pConfig)
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
	CamEngineHistWindow_t window;
	MEMSET(&window, 0, sizeof(CamEngineHistWindow_t));

	window.offsetX = pConfig->window.hOffset;
	window.offsetY = pConfig->window.vOffset;
	window.width = pConfig->window.hSize;
	window.height = pConfig->window.vSize;
	result = CamEngineHistSetMeasuringWindow(pCamDeviceCtx->hCamEngine, &window);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHistSetMeasuringWindow failed(%d)\n",  __func__, result);
		return result;
	}
/*2.set weight*/
	CamEngineHistWeights_t weight;
	MEMSET(&weight, 0, sizeof(CamEngineHistWeights_t));

	DCT_ASSERT(sizeof(weight.weights) == sizeof(pConfig->weight));
	MEMCPY(weight.weights, pConfig->weight, sizeof(pConfig->weight));
	result = CamEngineHistSetGridWeights(pCamDeviceCtx->hCamEngine, &weight);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHistSetGridWeights failed(%d)\n",  __func__, result);
		return result;
	}
/*3.set mode*/
	CamEngineHistMode_t mode;
	switch(pConfig->mode) {
		case CAMDEV_HIST256_MODE_RGB_COMBINED:
			mode = CAM_ENGINE_ISP_HIST_MODE_RGB_COMBINED;
			break;
		case CAMDEV_HIST256_MODE_R:
			mode = CAM_ENGINE_ISP_HIST_MODE_R;
			break;
		case CAMDEV_HIST256_MODE_G:
			mode = CAM_ENGINE_ISP_HIST_MODE_G;
			break;
		case CAMDEV_HIST256_MODE_B:
			mode = CAM_ENGINE_ISP_HIST_MODE_B;
			break;
		case CAMDEV_HIST256_MODE_Y:
			mode = CAM_ENGINE_ISP_HIST_MODE_Y;
			break;
		default:
			TRACE(CAM_DEV_MODULE_ERR,  "%s: input select error(%d)\n",  __func__, pConfig->mode);
		return RET_FAILURE;
	}

	result = CamEngineHistSetMeasuringMode(pCamDeviceCtx->hCamEngine, mode);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHistSetMeasuringMode failed(%d)\n",  __func__, result);
		return result;
	}

	pCamDeviceCtx->moduleCtx.histCtx.version = MODULE_V1;

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist256GetConfig(CamDeviceHandle_t hCamDevice,
									CamDeviceHist256Config_t *pConfig)
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
	CamEngineHistWindow_t window;
	MEMSET(&window, 0, sizeof(CamEngineHistWindow_t));

	result = CamEngineHistGetMeasuringWindow(pCamDeviceCtx->hCamEngine, &window);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHistGetMeasuringWindow failed(%d)\n",  __func__, result);
		return result;
	}
	pConfig->window.hOffset = window.offsetX;
	pConfig->window.vOffset = window.offsetY;
	pConfig->window.hSize = window.width;
	pConfig->window.vSize = window.height;
/*2.get weight*/
	CamEngineHistWeights_t weight;
	MEMSET(&weight, 0, sizeof(CamEngineHistWeights_t));

	result = CamEngineHistGetGridWeights(pCamDeviceCtx->hCamEngine, &weight);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHistGetGridWeights failed(%d)\n",  __func__, result);
		return result;
	}
	DCT_ASSERT(sizeof(weight.weights) == sizeof(pConfig->weight));
	MEMCPY(pConfig->weight, weight.weights, sizeof(weight.weights));
/*3.get mode*/
	CamEngineHistMode_t mode;

	result = CamEngineHistGetMeasuringMode(pCamDeviceCtx->hCamEngine, &mode);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHistGetMeasuringMode failed(%d)\n",  __func__, result);
		return result;
	}
	switch(mode) {
		case CAM_ENGINE_ISP_HIST_MODE_RGB_COMBINED:
			pConfig->mode = CAMDEV_HIST256_MODE_RGB_COMBINED;
			break;
		case CAM_ENGINE_ISP_HIST_MODE_R:
			pConfig->mode = CAMDEV_HIST256_MODE_R;
			break;
		case CAM_ENGINE_ISP_HIST_MODE_G:
			pConfig->mode = CAMDEV_HIST256_MODE_G;
			break;
		case CAM_ENGINE_ISP_HIST_MODE_B:
			pConfig->mode = CAMDEV_HIST256_MODE_B;
			break;
		case CAM_ENGINE_ISP_HIST_MODE_Y:
			pConfig->mode = CAMDEV_HIST256_MODE_Y;
			break;
		default:
			TRACE(CAM_DEV_MODULE_ERR,  "%s: input select error(%d)\n",  __func__, mode);
		return RET_FAILURE;
	}
/*4.get HIST bins window*/
	CamEngineHistBins_t bins;
	MEMSET(&bins, 0, sizeof(CamEngineHistBins_t));

	result = CamEngineHistGet(pCamDeviceCtx->hCamEngine, &bins);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHistGet failed(%d)\n",  __func__, result);
		return result;
	}
	DCT_ASSERT(sizeof(pConfig->bins) == sizeof(bins.weight));
	MEMCPY(pConfig->bins, bins.weight, sizeof(bins.weight));

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist256SetMeasureWindow(CamDeviceHandle_t hCamDevice,
										   CamDeviceWindow_t *pWindow)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist256GetMeasureWindow(CamDeviceHandle_t hCamDevice,
										   CamDeviceWindow_t *pWindow)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist256Enable(CamDeviceHandle_t hCamDevice)
{
#if 0
	CamDeviceContext_t *pCamDeviceCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;

	TRACE(CAM_DEV_MODULE_INFO, "%s (enter)\n", __func__);

	if (NULL == pCamDeviceCtx) {
		return (RET_WRONG_HANDLE);
	}

	result = CamEngineHistEnable(pCamDeviceCtx->hCamEngine);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHistEnable failed(%d)\n",  __func__, result);
		return result;
	}

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist256GetBins(CamDeviceHandle_t hCamDevice,
								  CamDeviceHist256Bins_t *pHistBin)
{
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist256Disable(CamDeviceHandle_t hCamDevice)
{
#if 0
	CamDeviceContext_t *pCamDeviceCtx = (CamDeviceContext_t *)hCamDevice;
	RESULT result = RET_SUCCESS;

	TRACE(CAM_DEV_MODULE_INFO, "%s (enter)\n", __func__);

	if (NULL == pCamDeviceCtx) {
		return (RET_WRONG_HANDLE);
	}

	result = CamEngineHistDisable(pCamDeviceCtx->hCamEngine);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHistDisable failed(%d)\n",  __func__, result);
		return result;
	}

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist256GetStatus(CamDeviceHandle_t hCamDevice,
									CamDeviceHist256Status_t *pStatus)
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

	result = CamEngineHistIsEnabled(pCamDeviceCtx->hCamEngine, &enable);
	if (RET_SUCCESS != result) {
		TRACE(CAM_DEV_MODULE_ERR,  "%s: CamEngineHistIsEnabled failed(%d)\n",  __func__, result);
		return result;
	}

	pStatus->enable = enable;

	TRACE(CAM_DEV_MODULE_INFO, "%s (exit)\n", __func__);
#endif
	return RET_SUCCESS;
}

RESULT VsiCamDeviceHist256GetVersion(CamDeviceHandle_t hCamDevice,
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

RESULT VsiCamDeviceHist256Reset(CamDeviceHandle_t hCamDevice)
{
	return RET_SUCCESS;
}
