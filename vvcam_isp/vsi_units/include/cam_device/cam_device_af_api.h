/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2023 Vivante Corporation
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

/**
 * @cond AF_V3
 *
 * @defgroup cam_device_af_v3 Cam Device AF V3 Definitions
 * @{
 *
 */

#ifndef __CAMDEV_AF_API_H__
#define __CAMDEV_AF_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define CAMDEV_AF_WINDOWNUM     3    /**< The number of AF window */
#define CAMDEV_AF_FILTERNUM     5    /**< The number of AF filter */
#define CAMDEV_PD_FOCAL_NUM_MAX 48    /**< The number of AF focal PD */
#define CAMDEV_PD_ROI_INDEX_MAX 48    /**< Maximum index of PDAF ROI */

/*****************************************************************************/
/**
 * @brief   Cam Device AF status.
 *
 *****************************************************************************/
typedef enum CamDeviceAfCtrlState_s{
    CAMDEV_AF_STATE_INVALID       = 0,    /**< Invalid AF status */
    CAMDEV_AF_STATE_INITIALIZED   = 1,    /**< AF initialized */
    CAMDEV_AF_STATE_STOPPED       = 2,    /**< AF stopped */
    CAMDEV_AF_STATE_RUNNING       = 3,    /**< AF running */
    CAMDEV_AF_STATE_TRACKING      = 4,    /**< AF tracking */
    CAMDEV_AF_STATE_LOCKED        = 5,    /**< AF locked */
    CAMDEV_AF_STATE_MAX,
    DUMMY_0004 = 0xDEADFEED
} CamDeviceAfState_t;

/*****************************************************************************/
/**
 * @brief   Cam Device AF mode parameters.
 *
 *****************************************************************************/
typedef enum CamDeviceAfMode_e{
    CAMDEV_AF_MODE_AUTO        = 0,    /**< Auto mode */
    CAMDEV_AF_MODE_MAX,
    DUMMY_0005 = 0xDEADFEED
} CamDeviceAfMode_t;

/*****************************************************************************/
/**
 * @brief   Cam Device AF library configuration, which is required by the AF library calculation.
 *
 *****************************************************************************/
typedef struct CamDeviceAfConfig_s
{
	// CDAF params
    float32_t    weightWindow[CAMDEV_AF_WINDOWNUM];    /**< Weight window */
    float32_t    cStableTolerance;      /**< Stable tolerance, range: [0, 1] */
    uint8_t  cPointsOfCurve;         /**< Points of curve, range: [5, 100] */
    float32_t    focalFilter[CAMDEV_AF_FILTERNUM];    /**< Focal filter */
    float32_t    shapFilter[CAMDEV_AF_FILTERNUM];    /**< Shape filter */
	uint16_t maxFocal;    /**< Maximum focal point */
    uint16_t minFocal;    /**< Minimum focal point*/
    float32_t    cMotionThreshold;    /**< Motion threshold */
    float32_t    focalStableThreshold;    /**< Stable threshold */

	// PDAF params
	float32_t    cMseTolerance;    /**< MSE tolerance  */
	float32_t    cPdConfThreshold;    /**< PDAF configuration threshold */
    int      pdFocal[CAMDEV_PD_FOCAL_NUM_MAX];  /**< Phase Detection focal spot */
    int      pdDistance;           /**< Phase Detection distance */
    float32_t    pdShiftThreshold;     /**< Phase Detection shift stable threshold */
    uint8_t  pdStableCountMax;    /**< Maximum focusing to which PDAF locks */
} CamDeviceAfConfig_t;

/*****************************************************************************/
/**
 * @brief   CamDevice AF status.
 *
 *****************************************************************************/
typedef struct CamDeviceAfStatus_s {
    CamDeviceAfState_t state;  /**< AF state*/
} CamDeviceAfStatus_t;

/*****************************************************************************/
/**
 *
 * @brief   This function registers AF.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 * @param   pAfLibHandle   Pointer to the AF library handle
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfRegister
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 *
 * @brief   This function unregisters AF.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfUnRegister
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 *
 * @brief   This function sets AF calculation mode parameters.
 *
 * @param   hCamDevice      Handle to the CamDevice instance
 * @param   pMode           Pointer to the AF calculation mode
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfSetMode
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceAfMode_t   *pMode
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets AF calculation mode parameters.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 * @param   pMode          Pointer to the AF calculation mode
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_INVALID_PARM    Invalid configuration
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetMode
(
    CamDeviceHandle_t    hCamDevice,
    CamDeviceAfMode_t    *pMode
);

/*****************************************************************************/
/**
 *
 * @brief   This function sets AF calculation parameters
 *
 * @param   hCamDevice       Handle to the CamDevice instance
 * @param   pConfig          Pointer to the AF calculation parameters

 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfSetConfig
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAfConfig_t      *pConfig
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets AF calculation parameters
 *
 * @param   hCamDevice       Handle to the CamDevice instance
 * @param   pConfig          Pointer to the AF calculation configuration parameters
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetConfig
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAfConfig_t      *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function sets AF PDAF ROI index
 *
 * @param   hCamDevice       Handle to the CamDevice instance
 * @param   roiIndex         AF ROI index, from 0 to 48
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfSetRoiIndex
(
    CamDeviceHandle_t  hCamDevice,
    uint8_t            roiIndex
);


/*****************************************************************************/
/**
 * @brief   This function gets AF PDAF ROI index
 *
 * @param   hCamDevice       Handle to the CamDevice instance
 * @param   pRoiIndex        Pointer to the AF ROI index
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetRoiIndex
(
    CamDeviceHandle_t  hCamDevice,
    uint8_t            *pRoiIndex
);


/*****************************************************************************/
/**
 * @brief   This function enables AF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfEnable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables AF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfDisable
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets the AF status.
 *
 * @param   hCamDevice     Handle to the CamDevice instance
 * @param   pStatus        Pointer to the AF status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetStatus
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAfStatus_t       *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets the AF version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to the AF version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets AF.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAfReset
(
    CamDeviceHandle_t     hCamDevice
);

/* @} cam_device_af_v3 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif

