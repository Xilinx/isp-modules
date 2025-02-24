/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Vivante Corporation
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

#ifndef __CAMDEV_ISP_SYSTEM_API_H__
#define __CAMDEV_ISP_SYSTEM_API_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include "cam_device_common.h"
#include <common/system_module.h>

/**
 * @defgroup cam_device_isp_system Cam Device Isp System Definitions
 * @{
 *
 *
 */

/*****************************************************************************/
/**
 * @brief   This function sets the enqueue buffer callback.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   func                Enqueue buffer callback
 * @param   pUserContext        User context
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
*****************************************************************************/
RESULT VsiCamDeviceSetEnqueueBufCb
(
    CamDeviceHandle_t     hCamDevice,
    IspSystemEnqueueBuf_t func,
    void                  *pUserContext
);

/*****************************************************************************/
/**
 * @brief   This function sets the dequeue buffer callback.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   func                Dequeue buffer callback
 * @param   pUserContext        User context
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
*****************************************************************************/
RESULT VsiCamDeviceSetDequeueBufCb
(
    CamDeviceHandle_t     hCamDevice,
    IspSystemDequeueBuf_t func,
    void                  *pUserContext
);


/*****************************************************************************/
/**
 * @brief   This function starts the isp streaming.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Start streaming params
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
*****************************************************************************/
RESULT VsiCamDeviceStartStreaming
(
    CamDeviceHandle_t hCamDevice,
    void *pConfig
);


/*****************************************************************************/
/**
 * @brief   This function stops the isp streaming.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Stop streaming params
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
*****************************************************************************/
RESULT VsiCamDeviceStopStreaming
(
    CamDeviceHandle_t hCamDevice,
    void *pConfig
);


 /* @} cam_device_isp_system */

#endif    // __ISP_CAMDEV_ISP_SYSTEM_H__


