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

#ifndef __CAMDEV_PDAF_API_H__
#define __CAMDEV_PDAF_API_H__

/**
 * @cond PDAF_DUMMY
 * @defgroup cam_device_pdaf Cam Device PDAF Dummy Definitions
 * @{
 *
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif

	/******************************************************************************/
	/**
	 * @brief   Cam Device PDAF configuration.
	 *
	 *****************************************************************************/
	typedef unsigned char CamDevicePdafConfig_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device PDAF status structure.
	 *
	 *****************************************************************************/
	typedef unsigned char CamDevicePdafStatus_t;

	//*****************************************************************************/
	/**
	 * @brief   This function sets the PDAF configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pPdafCfg            This pointer of PDAF configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevicePdafSetConfig(CamDeviceHandle_t hCamDevice,
									 CamDevicePdafConfig_t *pPdafCfg);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the PDAF configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pPdafCfg            This pointer of PDAF configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevicePdafGetConfig(CamDeviceHandle_t hCamDevice,
									 CamDevicePdafConfig_t *pPdafCfg);

	/*****************************************************************************/
	/**
	 * @brief   This function enables PDAF.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevicePdafEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables PDAF.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevicePdafDisable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the PDAF status.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pPdafStatus         This pointer of PDAF status
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevicePdafGetStatus(CamDeviceHandle_t hCamDevice,
									 CamDevicePdafStatus_t *pPdafStatus);

	/*****************************************************************************/
	/**
	 * @brief   This function enables the PDAF correction.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevicePdafCorrectEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function resets PDAF.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevicePdafReset(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables the PDAF correction.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevicePdafCorrectDisable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the PDAF version.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pVersion            This pointer of PDAF version
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDevicePdafGetVersion(CamDeviceHandle_t hCamDevice,
									  uint32_t *pVersion);

	/* @} cam_device_pdaf */
	/* @endcond */

#ifdef __cplusplus
}
#endif

#endif // __CAMDEV_PDAF_API_H__
