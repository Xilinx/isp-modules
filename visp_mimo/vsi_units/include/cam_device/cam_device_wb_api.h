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

#ifndef __CAMDEV_WB_API_H__
#define __CAMDEV_WB_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @cond WB_V1_1
 * 
 * @defgroup cam_device_wb_v1_1 Cam Device WB V1.1 Definitions
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   Cam Device WB manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceManualWbConfig_s {
    float32_t gain[CAMDEV_RAW_CHANNEL_NUM];       /**< WB gain */
}CamDeviceManualWbConfig_t;

 /******************************************************************************/
/**
 * @brief   Cam Device WB configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceWbConfig_s {
    CamDeviceManualWbConfig_t manualCfg;  /**< WB manual configuration */
}CamDeviceWbConfig_t;

/******************************************************************************/
/**
 * @brief   Cam Device WB status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceWbStatus_s {
    bool_t enable;              /**< WB enable status */
    CamDeviceConfigMode_t currentMode;        /**< The run mode: 0--manual, 1--auto */
    CamDeviceManualWbConfig_t currentCfg;  /**< WB current configuration */
}CamDeviceWbStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets WB configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pWbCfg              Pointer to WB configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceWbSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceWbConfig_t *pWbCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets WB configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pWbCfg              Pointer to WB configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceWbGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceWbConfig_t *pWbCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables WB.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceWbEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables WB.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceWbDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets WB status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pWbStatus           Pointer to WB status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceWbGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceWbStatus_t *pWbStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets WB to default parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceWbReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets WB version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to WB version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceWbGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/* @} cam_device_wb_v1_1 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // __CAMDEV_WB_API_H__
