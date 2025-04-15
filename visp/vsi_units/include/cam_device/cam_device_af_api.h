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
 * @cond AF_V3
 *
 * @defgroup cam_device_af_v3 Cam Device AF V3 Definitions
 * @{
 *
 */

#ifndef __CAMDEV_AF_API_H__
#define __CAMDEV_AF_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_AF_WINDOWNUM 9	   /**< The number of AF window */
#define CAMDEV_AF_FILTERNUM 5	   /**< The number of AF filter */
#define CAMDEV_PD_FOCAL_NUM_MAX 48 /**< The number of AF focal PD */
#define CAMDEV_PD_ROI_INDEX_MAX 48 /**< Maximum index of PDAF ROI */
#define CAMDEV_AFMV3_BLOCK_NUM 225 /**< The number of AFM V3 blocks */

	/*****************************************************************************/
	/**
	 * @brief   Cam Device AF status.
	 *
	 *****************************************************************************/
	typedef enum CamDeviceAfCtrlState_s
	{
		CAMDEV_AF_STATE_INVALID = 0,	 /**< Invalid AF status */
		CAMDEV_AF_STATE_INITIALIZED = 1, /**< AF initialized */
		CAMDEV_AF_STATE_STOPPED = 2,	 /**< AF stopped */
		CAMDEV_AF_STATE_RUNNING = 3,	 /**< AF running */
		CAMDEV_AF_STATE_TRACKING = 4,	 /**< AF tracking */
		CAMDEV_AF_STATE_LOCKED = 5,		 /**< AF locked */
		CAMDEV_AF_STATE_MAX,
		DUMMY_CAMDEV_AF_STATE = 0xDEADFEED
	} CamDeviceAfState_t;

	/*****************************************************************************/
	/**
	 * @brief   Cam Device AF mode parameters.
	 *
	 *****************************************************************************/
	typedef enum CamDeviceAfMode_e
	{
		CAMDEV_CDAF_INDIVIDUAL_MODE = 0,  /**< CDAF focus */
		CAMDEV_PDAF_INDIVIDUAL_MODE = 1,  /**< PDAF focus */
		CAMDEV_PDAF_CDAF_HYBRID_MODE = 2, /**< PDAF and CDAF hybrid */
		CAMDEV_AF_MODE_MAX,
		DUMMY_CAMDEV_AF_MODE = 0xDEADFEED
	} CamDeviceAfMode_t;

	/*****************************************************************************/
	/**
	 * @brief   CamDevice PDAF and CDAF hybrid focus parameters
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfPcdfHybridConfig_s
	{
		uint8_t defocusFrameNum; /**< The number of defocus frames */
		uint8_t
			lossConfidenceFrameNum; /**< The number of loss confidence frames */
		uint8_t accurateFocusStep;	/**< Accurate focus step */
		bool_t accurateFocusEnable; /**< Accurate focus enable */
	} CamDeviceAfPcdfHybridConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   Cam Device AF library configuration, which is required by the AF library calculation.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfConfig_s
	{
		// CDAF params
		float32_t weightWindow[CAMDEV_AF_WINDOWNUM]; /**< Weight window */
		float32_t cStableTolerance;					 /**< Range: [0, 1] */
		uint8_t cPointsOfCurve;						 /**< Range: [5, 100] */
		uint16_t maxFocal;							 /**< Maximum focal point */
		uint16_t minFocal;							 /**< Minimum focal point*/
		float32_t cMotionThreshold;					 /**< Motion threshold */

		// PDAF params
		float32_t cPdConfThreshold; /**< PDAF configuration threshold */
		float32_t
			pdShiftThreshold;	  /**< Phase Detection shift stable threshold */
		uint8_t pdStableCountMax; /**< Maximum focusing to which PDAF locks */

		// PDAF and CDAF hybrid params
		CamDeviceAfPcdfHybridConfig_t
			pcdafHybridConfig; /**< PDAF and CDAF hybrid configuration */
	} CamDeviceAfConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   CamDevice AF status.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfStatus_s
	{
		CamDeviceAfState_t state; /**< AF state*/
	} CamDeviceAfStatus_t;

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function registers AF.
	 *
	 * @param   hCamDevice     Handle to the CamDevice instance
	 * @param   pAfLibHandle   Pointer to the AF library handle
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfRegister(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function unregisters AF.
	 *
	 * @param   hCamDevice     Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfUnRegister(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function sets AF calculation mode parameters.
	 *
	 * @param   hCamDevice      Handle to the CamDevice instance
	 * @param   pMode           Pointer to the AF calculation mode
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfSetMode(CamDeviceHandle_t hCamDevice,
								 CamDeviceAfMode_t *pMode);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function gets AF calculation mode parameters.
	 *
	 * @param   hCamDevice     Handle to the CamDevice instance
	 * @param   pMode          Pointer to the AF calculation mode
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_INVALID_PARM    Invalid configuration
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfGetMode(CamDeviceHandle_t hCamDevice,
								 CamDeviceAfMode_t *pMode);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function sets AF calculation parameters
	 *
	 * @param   hCamDevice       Handle to the CamDevice instance
	 * @param   pConfig          Pointer to the AF calculation parameters

	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfSetConfig(CamDeviceHandle_t hCamDevice,
								   const CamDeviceAfConfig_t *pConfig);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function gets AF calculation parameters
	 *
	 * @param   hCamDevice       Handle to the CamDevice instance
	 * @param   pConfig          Pointer to the AF calculation configuration parameters
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfGetConfig(CamDeviceHandle_t hCamDevice,
								   CamDeviceAfConfig_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function sets AF PDAF ROI index
	 *
	 * @param   hCamDevice       Handle to the CamDevice instance
	 * @param   roiIndex         AF ROI index, from 0 to 48
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfSetRoiIndex(CamDeviceHandle_t hCamDevice,
									 uint8_t roiIndex);

	/*****************************************************************************/
	/**
	 * @brief   This function gets AF PDAF ROI index
	 *
	 * @param   hCamDevice       Handle to the CamDevice instance
	 * @param   pRoiIndex        Pointer to the AF ROI index
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfGetRoiIndex(CamDeviceHandle_t hCamDevice,
									 uint8_t *pRoiIndex);

	/*****************************************************************************/
	/**
	 * @brief   This function sets AFM V11 ROI.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance.
	 * @param   pRoi                Pointer to the ROI.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfSetAfmV11Roi(CamDeviceHandle_t hCamDevice,
									  const CamDeviceRoi_t *pRoi);

	/*****************************************************************************/
	/**
	 * @brief   This function gets AFM V11 ROI.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance.
	 * @param   pRoi                Pointer to the ROI.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfGetAfmV11Roi(CamDeviceHandle_t hCamDevice,
									  CamDeviceRoi_t *pRoi);

	/*****************************************************************************/
	/**
	 * @brief   This function sets the weight of AFM V3.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance.
	 * @param   weightWindow        AFM weight window.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfSetAfmV3Weight(
		CamDeviceHandle_t hCamDevice,
		float32_t weightWindow[CAMDEV_AFMV3_BLOCK_NUM]);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the weight of AFM V3.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance.
	 * @param   weightWindow        AFM weight window.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfGetAfmV3Weight(
		CamDeviceHandle_t hCamDevice,
		float32_t weightWindow[CAMDEV_AFMV3_BLOCK_NUM]);

	/*****************************************************************************/
	/**
	 * @brief   This function enables AF.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables AF.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfDisable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function gets the AF status.
	 *
	 * @param   hCamDevice     Handle to the CamDevice instance
	 * @param   pStatus        Pointer to the AF status
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfGetStatus(CamDeviceHandle_t hCamDevice,
								   CamDeviceAfStatus_t *pStatus);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the AF version.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pVersion            Pointer to the AF version
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfGetVersion(CamDeviceHandle_t hCamDevice,
									uint32_t *pVersion);

	/*****************************************************************************/
	/**
	 * @brief   This function resets AF.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfReset(CamDeviceHandle_t hCamDevice);

	/* @} cam_device_af_v3 */
	/* @endcond */

#ifdef __cplusplus
}
#endif

#endif
