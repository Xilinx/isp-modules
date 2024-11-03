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

#ifndef __CAMDEV_DPF_API_H__
#define __CAMDEV_DPF_API_H__

#include "cam_device_common.h"

/**
 * @cond DPF_DUMMY
 *
 * @defgroup cam_device_dpf dummy Cam Device DPF Definitions
 * @{
 *
 */

#define CAMDEV_DPF_NOISE_CURVE_SIZE 17        /**< Bin number of noise level curve */


#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************/
/**
 * @brief   Cam Device DPF configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceDpfConfig_s {
    uint8_t nop;    /**< DPF nop */
}CamDeviceDpfConfig_t;

/******************************************************************************/
/**
 * @brief   Cam Device DPF status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceDpfStatus_s {
    bool_t enable;      /**< DPF enable status */
}CamDeviceDpfStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets DPF configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pDpfCfg             Pointer to DPF configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpfSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceDpfConfig_t *pDpfCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets DPF configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pDpfCfg             Pointer to DPF configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpfGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceDpfConfig_t *pDpfCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables DPF RAW output.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpfRawOutputEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DPF  RAW output.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpfRawOutputDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function enables DPF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpfEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables DPF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpfDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DPF status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pDpfStatus          Pointer to DPF status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpfGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceDpfStatus_t *pDpfStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets DPF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpfReset
(
    CamDeviceHandle_t      hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets DPF version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to DPF version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceDpfGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/* @} cam_device_dpf */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // __CAMDEV_DPF_API_H__
