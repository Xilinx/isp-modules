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
 * @cond AFM_V1
 *
 * @defgroup cam_device_afm_v1 Cam Device AFM V1 Definitions
 * @{
 *
 */

#ifndef __CAMDEV_AFM_V1_API_H__
#define __CAMDEV_AFM_V1_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/******************************************************************************/
	/**
	 * @brief   Enumeration type to identify the AF measuring window.
	 *
	 *****************************************************************************/
	typedef enum CamDeviceAfmWindowId_e
	{
		CAMDEV_AFM_WINDOW_INVALID =
			0, /**< Lower border (only for an internal evaluation) */
		CAMDEV_AFM_WINDOW_A = 1, /**< Window A (1st window) */
		CAMDEV_AFM_WINDOW_B = 2, /**< Window B (2nd window) */
		CAMDEV_AFM_WINDOW_C = 3, /**< Window C (3rd window) */
		CAMDEV_AFM_WINDOW_MAX, /**< Upper border (only for an internal evaluation) */
		DUMMY_CAMDEV_AFM_WINDOW = 0xDEADFEED
	} CamDeviceAfmWindowId_t;

	/*****************************************************************************/
	/**
	 * @brief   Cam Device AFM measure results structure.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfmMeasureResult_s
	{
		uint32_t sharpnessA; /**< Sharpness of window A */
		uint32_t sharpnessB; /**< Sharpness of window B */
		uint32_t sharpnessC; /**< Sharpness of window C */

		uint32_t luminanceA; /**< Luminance of window A */
		uint32_t luminanceB; /**< Luminance of window B */
		uint32_t luminanceC; /**< Luminance of window C */

		uint32_t pixelCntA; /**< Pixel counts of window A */
		uint32_t pixelCntB; /**< Pixel counts of window B */
		uint32_t pixelCntC; /**< Pixel counts of window C */
	} CamDeviceAfmMeasureResult_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device AFM window configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfmWindowConfig_s
	{
		CamDeviceWindow_t window;  /**< AFM window */
		CamDeviceAfmWindowId_t id; /**< AFM window index */
	} CamDeviceAfmWindowConfig_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device AFM status structure.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfmStatus_s
	{
		bool_t enable; /**< AFM enable status */
	} CamDeviceAfmStatus_t;

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function sets the threshold in the AFM module.
	 *
	 * @param   hCamDevice     Handle to the CamDevice instance
	 * @param   threshold      Threshold value
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmSetThreshold(CamDeviceHandle_t hCamDevice,
									   const uint32_t threshold

	);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function gets the threshold in the AFM module.
	 *
	 * @param   hCamDevice     Handle to the CamDevice instance
	 * @param   pThreshold     Threshold value pointer
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmGetThreshold(CamDeviceHandle_t hCamDevice,
									   uint32_t *pThreshold);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function gets the AFM statistic result.
	 *
	 * @param   hCamDevice       Handle to the CamDevice instance
	 * @param   pResult          Measure results pointer
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmGetStatistics(CamDeviceHandle_t hCamDevice,
										CamDeviceAfmMeasureResult_t *pResult);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function sets the AFM statistics window
	 *
	 * @param   hCamDevice   Handle to the CamDevice instance
	 * @param   pWindow      Pointer to window configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmSetMeasureWindow(CamDeviceHandle_t hCamDevice,
										   CamDeviceAfmWindowConfig_t *pWindow);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function gets the AFM statistics window
	 *
	 * @param   hCamDevice   Handle to the CamDevice instance
	 * @param   pWindow      Pointer to window configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmGetMeasureWindow(CamDeviceHandle_t hCamDevice,
										   CamDeviceAfmWindowConfig_t *pWindow);

	/*****************************************************************************/
	/**
	 * @brief   This function enables AFM.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables AFM.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmDisable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function gets AFM status.
	 *
	 * @param   hCamDevice     Handle to the CamDevice instance
	 * @param   pStatus        Pointer to AFM status
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmGetStatus(CamDeviceHandle_t hCamDevice,
									CamDeviceAfmStatus_t *pStatus);

	/*****************************************************************************/
	/**
	 * @brief   This function gets AFM version.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pVersion            Pointer to AFM version
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmGetVersion(CamDeviceHandle_t hCamDevice,
									 uint32_t *pVersion);

	/*****************************************************************************/
	/**
	 * @brief   This function resets AFM.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmReset(CamDeviceHandle_t hCamDevice);

	/* @} cam_device_afm_v1 */
	/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /*__CAMDEV_AFM_V1_API_H__*/
