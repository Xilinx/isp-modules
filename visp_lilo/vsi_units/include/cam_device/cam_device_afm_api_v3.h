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
 * @cond AFM_DUMMY
 *
 * @defgroup cam_device_afm_v3 Cam Device AFM V3 Definitions
 * @{
 *
 */

#ifndef __CAMDEV_AFM_V3_API_H__
#define __CAMDEV_AFM_V3_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cam_device_common.h"

	/*****************************************************************************/
	/**
	 * @brief   Measuring result.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfmV3MeasureResult_s
	{
		uint32_t nop;
	} CamDeviceAfmV3MeasureResult_t;

	/*****************************************************************************/
	/**
	 * @brief   CamDevice AFM LDG(Level Depend Gain) configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfmV3LdgConfig_s
	{
		float32_t nop;
	} CamDeviceAfmV3LdgConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   CamDevice AFM Coring configuration
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfmV3CoringConfig_s
	{
		uint8_t nop;
	} CamDeviceAfmV3CoringConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   CamDevice AFM IIR filter configuration
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfmV3IirConfig_s
	{
		uint8_t nop;
	} CamDeviceAfmV3IirConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   CamDevice AFM IIR filter configuration
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfmV3FirConfig_s
	{
		uint8_t nop;
	} CamDeviceAfmV3FirConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   CamDevice AFM configuration
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfmV3Config_s
	{
		uint16_t nop;
	} CamDeviceAfmV3Config_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device AFM status structure.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceAfmV3Status_s
	{
		bool_t enable; /**< AFM enable status */
	} CamDeviceAfmV3Status_t;

	/*****************************************************************************/
	/**
	 * @brief   This function configures the AFM module.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pConfig             Pointer to AFM configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmV3SetConfig(CamDeviceHandle_t hCamDevice,
									  CamDeviceAfmV3Config_t *pConfig);

	/*****************************************************************************/
	/**
	 *
	 * @brief   This function gets AFM module configurations.
	 *
	 * @param   hCamDevice     Handle to the CamDevice instance
	 * @param   pConfig        Pointer to AFM configuration
	 *
	 * @retval  RET_SUCCESS    Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmV3GetConfig(CamDeviceHandle_t hCamDevice,
									  CamDeviceAfmV3Config_t *pConfig);

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
	RESULT VsiCamDeviceAfmV3GetStatistics(
		CamDeviceHandle_t hCamDevice, CamDeviceAfmV3MeasureResult_t *pResult);

	/*****************************************************************************/
	/**
	 * @brief   This function enables AFM.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmV3Enable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables AFM.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   hCamDevice          AFM instance index
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmV3Disable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function enables AFM gamma.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmV3GammaEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables AFM gamma.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAfmV3GammaDisable(CamDeviceHandle_t hCamDevice);

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
	RESULT VsiCamDeviceAfmV3GetStatus(CamDeviceHandle_t hCamDevice,
									  CamDeviceAfmV3Status_t *pStatus);

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
	RESULT VsiCamDeviceAfmV3GetVersion(CamDeviceHandle_t hCamDevice,
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
	RESULT VsiCamDeviceAfmV3Reset(CamDeviceHandle_t hCamDevice);

#ifdef __cplusplus
}
#endif

/* @} cam_device_afm_v3 */
/* @endcond */

#endif /*__CAMDEV_AFM_V3_API_H__*/
