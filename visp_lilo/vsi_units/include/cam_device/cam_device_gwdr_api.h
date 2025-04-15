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
 * @cond GWDR_DUMMY
 * @defgroup cam_device_gwdr Cam Device GWDR Dummy Definitions
 * @{
 *
 */

#ifndef __CAMDEV_GWDR_API_H__
#define __CAMDEV_GWDR_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/******************************************************************************/
	/**
	 * @brief   Cam Device GWDR Dummy configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceGWdrConfig_s
	{
		uint8_t nop;
	} CamDeviceGWdrConfig_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device GWDR Dummy status structure.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceGWdrStatus_s
	{
		uint8_t nop;
	} CamDeviceGWdrStatus_t;

	/*****************************************************************************/
	/**
	 * @brief   This function sets GWDR Dummy configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pGWdrCfg             Pointer to GWDR Dummy configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGWdrSetConfig(CamDeviceHandle_t hCamDevice,
									 CamDeviceGWdrConfig_t *pGWdrCfg);

	/*****************************************************************************/
	/**
	 * @brief   This function gets GWDR Dummy configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pGWdrCfg             Pointer to GWDR Dummy configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGWdrGetConfig(CamDeviceHandle_t hCamDevice,
									 CamDeviceGWdrConfig_t *pGWdrCfg);

	/*****************************************************************************/
	/**
	 * @brief   This function enables GWDR Dummy.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGWdrEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables GWDR Dummy.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGWdrDisable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets GWDR Dummy status.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pGWdrStatus          Pointer to GWDR status
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGWdrGetStatus(CamDeviceHandle_t hCamDevice,
									 CamDeviceGWdrStatus_t *pGWdrStatus);

	/*****************************************************************************/
	/**
	 * @brief   This function resets GWDR to Dummy
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGWdrReset(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets GWDR Dummy version.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pVersion            Pointer to GWDR version
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGWdrGetVersion(CamDeviceHandle_t hCamDevice,
									  uint32_t *pVersion);

	/* @} cam_device_gwdr dummy */
	/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /* __CAMDEV_GWDR_API_H__ */
