/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2022 Vivante Corporation
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
 ****************************************************************************/

/**
 * @cond DNR3_V2_1
 *
 * @defgroup cam_device_3dnr_v2_1 Cam Device 3DNR V2.1 Definitions
 * @{
 *
 */

#ifndef __CAMDEV_3DNRV2_1_API_H__
#define __CAMDEV_3DNRV2_1_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_3DNR_TEMPORAL_CURVE_NUM \
	17 /**< The number of 3DNR temporal curves */

	/******************************************************************************/
	/**
	 * @brief   Cam Device 3DNR temporal manual configuraiton.
	 *
	 *****************************************************************************/
	typedef struct CamDevice3DnrTemporalManualConfig_s
	{
		uint16_t updateTemporal;		/**< Temporal update */
		uint16_t strengthCurveTemporal; /**< Temporal strength curve */
		uint16_t temporalCurve
			[CAMDEV_3DNR_TEMPORAL_CURVE_NUM]; /**< Temporal curve */
		uint8_t rangeT;						  /**< Temporal range */
	} CamDevice3DnrTemporalManualConfig_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device 3DNR temporal auto configuraiton.
	 *
	 *****************************************************************************/
	typedef struct CamDevice3DnrTemporalAutoConfig_s
	{
		uint8_t motionDilateEn
			[CAMDEV_ISO_STRENGTH_NUM]; /**< Motion dilation enable */
		uint16_t
			updateTemporal[CAMDEV_ISO_STRENGTH_NUM]; /**< Temporal update */
		uint16_t strengthCurveTemporal
			[CAMDEV_ISO_STRENGTH_NUM]; /**< Temporal strength curve */
		uint16_t temporalCurve
			[CAMDEV_ISO_STRENGTH_NUM]
			[CAMDEV_3DNR_TEMPORAL_CURVE_NUM];	 /**< Temporal curve */
		uint8_t rangeT[CAMDEV_ISO_STRENGTH_NUM]; /**< Temporal range */
	} CamDevice3DnrTemporalAutoConfig_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device 3DNR V2.1 configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDevice3DnrAutoConfig_s
	{
		uint8_t autoLevel;						   /**< The auto level */
		float32_t gains[CAMDEV_ISO_STRENGTH_NUM];  /**< 3DNR gains */
		uint8_t strength[CAMDEV_ISO_STRENGTH_NUM]; /**< General strength */
		int32_t motionInv
			[CAMDEV_ISO_STRENGTH_NUM]; /**< Motion factor. Smaller values mean larger motion image weight */
		uint16_t delta
			[CAMDEV_ISO_STRENGTH_NUM]; /**< Temporal delta. Smaller values mean more merged ref image */

		CamDevice3DnrTemporalAutoConfig_t
			temporalCfg; /**< Auto temporal configuration */
	} CamDevice3DnrAutoConfig_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device 3DNR V2.1 current configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDevice3DnrManualConfig_s
	{
		uint8_t strength; /**< General strength */
		int32_t
			motionInv; /**< Motion factor. Smaller values mean larger motion image weight */
		uint16_t
			delta; /**< Temporal delta. Smaller values mean more merged ref image */

		CamDevice3DnrTemporalManualConfig_t
			temporalCfg; /**< Manual temporal configuration */
	} CamDevice3DnrManualConfig_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device 3DNR V2.1 configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDevice3DnrConfig_s
	{
		CamDeviceConfigMode_t
			configMode; /**< The run mode: 0--manual, 1--auto */
		CamDevice3DnrAutoConfig_t autoCfg;	   /**< 3DNR auto configuration*/
		CamDevice3DnrManualConfig_t manualCfg; /**< 3DNR current configuration*/
	} CamDevice3DnrConfig_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device 3DNR status structure.
	 *
	 *****************************************************************************/
	typedef struct CamDevice3DnrStatus_s
	{
		bool_t enable;		   /**< 3DNR enable status*/
		bool_t motionDilateEn; /**< 3DNR motion dilation enable status */
		CamDeviceConfigMode_t
			currentMode; /**< The run mode: 0--manual, 1--auto */
		CamDevice3DnrManualConfig_t
			currentCfg; /**< 3DNR current configuration*/
	} CamDevice3DnrStatus_t;

	/*****************************************************************************/
	/**
	 * @brief   This function sets 3DNR configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pConfig             Configuration of 3DNR
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 * @retval  RET_NOTAVAILABLE    Module is not available
	 *
	 *****************************************************************************/
	RESULT VsiCamDevice3DnrSetConfig(CamDeviceHandle_t hCamDevice,
									 CamDevice3DnrConfig_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function gets 3DNR configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pConfig             Configuration of 3DNR
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 * @retval  RET_NOTAVAILABLE    Module is not available
	 *
	 *****************************************************************************/
	RESULT VsiCamDevice3DnrGetConfig(CamDeviceHandle_t hCamDevice,
									 CamDevice3DnrConfig_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function enables 3DNR.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDevice3DnrEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables 3DNR.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDevice3DnrDisable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function enables 3DNR motion dilation.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDevice3DnrMotionDilationEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables 3DNR motion dilation.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDevice3DnrMotionDilationDisable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets 3DNR status.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   p3DnrStatus         The pointer of 3DNR status
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevice3DnrGetStatus(CamDeviceHandle_t hCamDevice,
									 CamDevice3DnrStatus_t *p3DnrStatus);

	/*****************************************************************************/
	/**
	 * @brief   This function resets 3DNR.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevice3DnrReset(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets 3DNR version.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pVersion            The pointer of 3DNR version
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevice3DnrGetVersion(CamDeviceHandle_t hCamDevice,
									  uint32_t *pVersion);

#ifdef __cplusplus
}
#endif

/* @} cam_device_3dnr_v2_1 */
/* @endcond */

#endif /* __CAMDEV_3DNRV2_1_API_H__ */
