/******************************************************************************\
|* Copyright (c) 2022 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

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

