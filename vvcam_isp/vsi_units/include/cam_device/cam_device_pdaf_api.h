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
RESULT VsiCamDevicePdafSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePdafConfig_t *pPdafCfg
);

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
RESULT VsiCamDevicePdafGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePdafConfig_t *pPdafCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables PDAF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables PDAF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafDisable
(
    CamDeviceHandle_t hCamDevice
);

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
RESULT VsiCamDevicePdafGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePdafStatus_t *pPdafStatus
);

/*****************************************************************************/
/**
 * @brief   This function enables the PDAF correction.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafCorrectEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function resets PDAF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafReset
(
    CamDeviceHandle_t             hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables the PDAF correction.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDevicePdafCorrectDisable
(
    CamDeviceHandle_t hCamDevice
);

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
RESULT VsiCamDevicePdafGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/* @} cam_device_pdaf */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // __CAMDEV_PDAF_API_H__
