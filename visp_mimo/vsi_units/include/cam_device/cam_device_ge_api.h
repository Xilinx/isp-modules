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

#ifndef __CAMDEV_GE_API_H__
#define __CAMDEV_GE_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @cond GE
 * 
 * @defgroup cam_device_ge Cam Device GE Definitions
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   Cam Device GE manual configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGeManualConfig_s {
    float32_t threshold;                     /**< GE threshold */
}CamDeviceGeManualConfig_t;

/******************************************************************************/
/**
 * @brief   Cam Device GE configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceGeConfig_s {
    CamDeviceGeManualConfig_t manualCfg; /**< GE manual configuration */
}CamDeviceGeConfig_t;

/******************************************************************************/
/**
 * @brief   Cam Device GE status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceGeStatus_s {
    bool_t enable;                      /**< GE enable status */
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceGeManualConfig_t currentCfg; /**< GE current configuration */
}CamDeviceGeStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets GE configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pGeCfg              Pointer to GE configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGeSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceGeConfig_t *pGeCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets GE configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pGeCfg              Pointer to GE configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGeGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceGeConfig_t *pGeCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables GE.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGeEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables GE.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGeDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets the GE status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pGeStatus           Pointer to GE status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGeGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceGeStatus_t *pGeStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets GE.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGeReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets the GE version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to GE version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceGeGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/* @} cam_device_ge */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // __CAMDEV_GE_API_H__
