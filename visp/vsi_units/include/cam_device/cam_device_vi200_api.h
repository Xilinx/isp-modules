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

#ifndef __CAMDEV_VI200_DUMMY_API_H__
#define __CAMDEV_VI200_DUMMY_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @cond VI200_DUMMY
 *
 * @defgroup cam_device_vi200_dummy Cam Device VI200 DUMMY Definitions
 * @{
 *
 */
#define CAMDEV_VI200_ADAPTER_NUM 1

	/******************************************************************************/
	/**
	 * @brief  cam device VI200 adapter channel config structure
	 *
	 *****************************************************************************/
	typedef struct CamDeviceVi200AptChannelCfg_e
	{
		uint8_t nop;
	} CamDeviceVi200AptChannelCfg_t;

	typedef struct CamDeviceVi200AptProperty_e
	{
		uint8_t nop;
	} CamDeviceVi200AptProperty_t;

	/******************************************************************************/
	/**
	 * @brief cam device VI200 adapter status
	 *
	 *****************************************************************************/
	typedef struct CamDeviceVi200Status_e
	{
		uint8_t nop;
	} CamDeviceVi200Status_t;

	/*****************************************************************************/
	/**
	 * @brief   This function set VI200 adapter config
	 *
	 * @param   hCamDevice          handle to the CamDevice instance
	 * @param   pCfg                adapter config pointer
	 *
	 * @return                      Return the result of the function call.
	 * @retval  RET_SUCCESS         operation succeded
	 * @retval  RET_FAILURE         common error occured
	 * @retval  RET_BUSY            already a callback registered
	 * @retval  RET_WRONG_HANDLE    given handle is invalid
	 * @retval  RET_INVALID_PARM    given parameter is invalid
	 * @retval  RET_WRONG_STATE     driver is in wrong state to register a
	 *                              request callback
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceVi200SetAdapterCfg(CamDeviceHandle_t hCamDevice,
										  CamDeviceVi200AptChannelCfg_t *pCfg);

	/*****************************************************************************/
	/**
	 * @brief   This function get VI200 adapter config
	 *
	 * @param   hCamDevice          handle to the CamDevice instance
	 * @param   pCfg                adapter config pointer
	 *
	 * @return                      Return the result of the function call.
	 * @retval  RET_SUCCESS         operation succeded
	 * @retval  RET_FAILURE         common error occured
	 * @retval  RET_BUSY            already a callback registered
	 * @retval  RET_WRONG_HANDLE    given handle is invalid
	 * @retval  RET_INVALID_PARM    given parameter is invalid
	 * @retval  RET_WRONG_STATE     driver is in wrong state to register a
	 *                              request callback
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceVi200GetAdapterCfg(CamDeviceHandle_t hCamDevice,
										  CamDeviceVi200AptChannelCfg_t *pCfg);

	/*****************************************************************************/
	/**
	 * @brief   This function get VI200 adapter status
	 *
	 * @param   hCamDevice          handle to the CamDevice instance
	 * @param   pCfg                adapter config pointer
	 *
	 * @return                      Return the result of the function call.
	 * @retval  RET_SUCCESS         operation succeded
	 * @retval  RET_FAILURE         common error occured
	 * @retval  RET_BUSY            already a callback registered
	 * @retval  RET_WRONG_HANDLE    given handle is invalid
	 * @retval  RET_INVALID_PARM    given parameter is invalid
	 * @retval  RET_WRONG_STATE     driver is in wrong state to register a
	 *                              request callback
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceVi200GetStatus(CamDeviceHandle_t hCamDevice,
									  CamDeviceVi200Status_t *pStatus);

	/*****************************************************************************/
	/**
	 * @brief   This function resets VI200 to default parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceVi200Reset(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets VI200 version.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pVersion            Pointer to VI200 version
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceVi200GetVersion(CamDeviceHandle_t hCamDevice,
									   uint32_t *pVersion);

	/*****************************************************************************/
	/**
	 * @brief   This function gets VI200 hw enable status.
	 *
	 * @param   pVersion            Pointer to VI200 version
	 *
	 * @retval  bool_t              Hw enable status
	 *
	 *****************************************************************************/
	bool_t VsiCamDeviceVi200HwIsEnable(void);

#ifdef __cplusplus
}
#endif

/* @} cam_device_vi200_dummy */
/* @endcond */

#endif /* __CAMDEV_VI200_DUMMY_API_H__ */
