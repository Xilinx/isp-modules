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
 * @cond YNR_DUMMY
 * @defgroup cam_device_ynr_v1 Cam Device YNR Definitions
 * @{
 *
 */

#ifndef __CAMDEV_YNR_API_H__
#define __CAMDEV_YNR_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************/
/**
 * @brief   Cam Device YNR configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceYnrConfig_s
{
    uint8_t     nop;
} CamDeviceYnrConfig_t;

/******************************************************************************/
/**
 * @brief   Cam Device YNR status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceYnrStatus_s
{
    bool_t  enable;    /**< YNR enable status*/
}CamDeviceYnrStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets YNR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to YNR configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceYnrSetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceYnrConfig_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets YNR configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to YNR configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceYnrGetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceYnrConfig_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables YNR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceYnrEnable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables YNR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceYnrDisable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets YNR status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to YNR status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceYnrGetStatus
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceYnrStatus_t    *pStatus
);


/*****************************************************************************/
/**
 * @brief   This function resets YNR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceYnrReset
(
    CamDeviceHandle_t         hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets YNR version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to YNR version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceYnrGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/* @} cam_device_ynr_v1 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /* __CAMDEV_YNR_API_H__ */

