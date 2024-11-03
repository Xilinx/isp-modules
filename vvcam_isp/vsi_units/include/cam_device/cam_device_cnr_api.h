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
 * @cond CNR_DUMMY
 * @defgroup cam_device_cnr_api CamDevice CNR Dummy API
 * @{
 *
 */

#ifndef __CAMDEV_CNR_API_H__
#define __CAMDEV_CNR_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct CamDeviceCnrConfig_s {
    uint8_t nop;
} CamDeviceCnrConfig_t;

/******************************************************************************/
/**
 * @brief   Cam Device CNR status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceCnrStatus_s {
    bool_t enable;    /**< EE enable status */
}CamDeviceCnrStatus_t;

/*****************************************************************************/
/**
 * @brief   This function sets the CNR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pEeCfg              This pointer of CNR configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceCnrConfig_t *pCnrCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets the CNR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pEeCfg              This pointer of CNR configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceCnrConfig_t *pCnrCfg
);

/*****************************************************************************/
/**
 * @brief   This function enable denoise of C channel.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrEnable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disable denoise of C channel.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrDisable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function get CNR status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             pointer to the CNR configuration
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceCnrStatus_t *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets CNR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrReset
(
    CamDeviceHandle_t    hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function get CNR version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            pointer to the CNR version
 *
 * @retval  RET_SUCCESS         function succeed
 * @retval  RET_WRONG_HANDLE    invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceCnrGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

#ifdef __cplusplus
}
#endif


/* @} cam_device_cnr_api dummy*/
/* @endcond */

#endif /* __CAMDEV_CNR_API_H__ */

