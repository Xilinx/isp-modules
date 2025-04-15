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

/**
 * @cond EXP_DUMMY
 *
 * @defgroup cam_device_exp_v3 Cam Device EXP V3 Definitions
 * @{
 *
 */

#ifndef __CAMDEV_EXP_V3_API_H__
#define __CAMDEV_EXP_V3_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_EXP_WINDOW 25
#define CAMDEV_EXPV3_GRID_ITEMS 25
#define CAMDEV_EXPV3_PIXEL_CHANNEL 2

	typedef uint32_t CamDeviceExpV3Statistics_t[CAMDEV_EXPV3_GRID_ITEMS *
												CAMDEV_EXPV3_PIXEL_CHANNEL];

	/******************************************************************************/
	/**
	 * @brief   Cam Device ISP EXP input select.
	 *
	 *****************************************************************************/
	typedef enum CamDeviceExpV3InputSelect_e
	{
		CAMDEV_ISP_EXPV3_INPUT_SELECT_INVALID = 0,
		CAMDEV_ISP_EXPV3_INPUT_SELECT_MAX,
		DUMMY_0048 = 0xDEADFEED
	} CamDeviceExpV3InputSelect_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device EXP windows configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceExpV3WindowConfig_s
	{
		uint8_t nop;
	} CamDeviceExpV3WindowConfig_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device EXP configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceExpV3Config_s
	{
		uint8_t nop;
	} CamDeviceExpV3Config_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device EXP status structure.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceExpV3Status_s
	{
		uint8_t nop;
	} CamDeviceExpV3Status_t;

	/*****************************************************************************/
	/**
	 * @brief   This function sets EXP configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pConfig             Pointer to EXP configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceExpV3SetConfig(CamDeviceHandle_t hCamDevice,
									  CamDeviceExpV3Config_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function gets EXP configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pConfig             Pointer to EXP configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceExpV3GetConfig(CamDeviceHandle_t hCamDevice,
									  CamDeviceExpV3Config_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function sets EXP measure window.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   window              Pointer to EXP measure window
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceExpV3SetMeasureWindow(
		CamDeviceHandle_t hCamDevice, CamDeviceExpV3WindowConfig_t *pWindow);

	/*****************************************************************************/
	/**
	 * @brief   This function gets EXP measure window.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pWindow             Pointer to EXP measure window
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceExpV3GetMeasureWindow(
		CamDeviceHandle_t hCamDevice, CamDeviceExpV3WindowConfig_t *pWindow);

	/*****************************************************************************/
	/**
	 * @brief   This function gets EXP statistic parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @Param   pExpStatic          Pointer to the EXP statistics
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceExpV3GetStatistics(
		CamDeviceHandle_t hCamDevice, CamDeviceExpV3Statistics_t *pExpStatic);

	/*****************************************************************************/
	/**
	 * @brief   This function enables EXP.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceExpV3Enable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables EXP.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceExpV3Disable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets EXP status.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pStatus             Pointer to EXP status
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceExpV3GetStatus(CamDeviceHandle_t hCamDevice,
									  CamDeviceExpV3Status_t *pStatus);

	/*****************************************************************************/
	/**
	 * @brief   This function gets EXP version.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pVersion            Pointer to EXP version
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceExpV3GetVersion(CamDeviceHandle_t hCamDevice,
									   uint32_t *pVersion);

	/*****************************************************************************/
	/**
	 * @brief   This function resets EXP
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceExpV3Reset(CamDeviceHandle_t hCamDevice);

	/* @} cam_device_exp_v3 */
	/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /* __CAMDEV_EXP_V3_API_H__ */
