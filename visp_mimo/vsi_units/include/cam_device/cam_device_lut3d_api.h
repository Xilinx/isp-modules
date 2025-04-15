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
 *
 * @cond LUT3D_DUMMY
 * @defgroup cam_device_lut3d_api dummy CamDevice ISP API
 * @{
 *
 */

#ifndef __CAMDEV_LUT3D_API_H__
#define __CAMDEV_LUT3D_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_LUT3D_TABLE_SIZE (17*17*17)

/*****************************************************************************/
/**
 * @brief   Lut3d configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceLut3dConfig_s {
    uint8_t nop;
} CamDeviceLut3dConfig_t;

typedef struct CamDeviceLut3dStatus_s
{
    bool_t  enable;
} CamDeviceLut3dStatus_t;


/*****************************************************************************/
/**
 * @brief   This function enable the LUT3D module.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT VsiCamDeviceLut3dEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disable the LUT3D module.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT VsiCamDeviceLut3dDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function configuration the LUT3D module.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig           configuration of LUT3D
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT VsiCamDeviceLut3dSetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceLut3dConfig_t  *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets the configuration of LUT3D module.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig           configuration of LUT3D
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT VsiCamDeviceLut3dGetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceLut3dConfig_t  *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets the status of LUT3D.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             LUT3D status
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 * @retval  RET_WRONG_CONFIG    image effects isn't configured
 * @retval  RET_NOTAVAILABLE    module not available by driver or hardware
 *
 *****************************************************************************/
RESULT VsiCamDeviceLut3dGetStatus
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceLut3dStatus_t   *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets LUT3D.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceLut3dReset
(
    CamDeviceHandle_t        hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets Lut3d version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            This pointer of Lut3d version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceLut3dGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
);

#ifdef __cplusplus
}
#endif


/* @} cam_device_lut3d_api dummy */
/* @endcond */

#endif /* __CAMDEV_LUT3D_API_H__ */

