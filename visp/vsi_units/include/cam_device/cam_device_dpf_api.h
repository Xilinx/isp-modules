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

#ifndef __CAMDEV_DPF_API_H__
#define __CAMDEV_DPF_API_H__

#include "cam_device_common.h"

/**
 * @cond DPF_DUMMY
 *
 * @defgroup cam_device_dpf dummy Cam Device DPF Definitions
 * @{
 *
 */

#define CAMDEV_DPF_NOISE_CURVE_SIZE 17 /**< Bin number of noise level curve */

#ifdef __cplusplus
extern "C"
{
#endif

	/******************************************************************************/
	/**
	 * @brief   Cam Device DPF configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceDpfConfig_s
	{
		uint8_t nop; /**< DPF nop */
	} CamDeviceDpfConfig_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device DPF status structure.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceDpfStatus_s
	{
		bool_t enable; /**< DPF enable status */
	} CamDeviceDpfStatus_t;

	/*****************************************************************************/
	/**
	 * @brief   This function sets DPF configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pDpfCfg             Pointer to DPF configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDpfSetConfig(CamDeviceHandle_t hCamDevice,
									CamDeviceDpfConfig_t *pDpfCfg);

	/*****************************************************************************/
	/**
	 * @brief   This function gets DPF configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pDpfCfg             Pointer to DPF configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDpfGetConfig(CamDeviceHandle_t hCamDevice,
									CamDeviceDpfConfig_t *pDpfCfg);

	/*****************************************************************************/
	/**
	 * @brief   This function enables DPF RAW output.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDpfRawOutputEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables DPF  RAW output.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDpfRawOutputDisable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function enables DPF.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDpfEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables DPF.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDpfDisable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets DPF status.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pDpfStatus          Pointer to DPF status
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDpfGetStatus(CamDeviceHandle_t hCamDevice,
									CamDeviceDpfStatus_t *pDpfStatus);

	/*****************************************************************************/
	/**
	 * @brief   This function resets DPF.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDpfReset(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets DPF version.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pVersion            Pointer to DPF version
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDpfGetVersion(CamDeviceHandle_t hCamDevice,
									 uint32_t *pVersion);

	/* @} cam_device_dpf */
	/* @endcond */

#ifdef __cplusplus
}
#endif

#endif // __CAMDEV_DPF_API_H__
