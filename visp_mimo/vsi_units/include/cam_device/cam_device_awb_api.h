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
 * @cond AWB_V3
 * 
 * @defgroup cam_device_awb_v3 Cam Device AWB V3 Definitions
 * @{
 *
 */

#ifndef __CAMDEV_AWB_API_H__
#define __CAMDEV_AWB_API_H__

//TODO #include "vsi_3alib_interface.h"
#include "cam_device_common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_AWB_COLOR_TEM_WEIGHT_NUM     7    /**< The number of AWB color temperature weight */
#define CAMDEV_AWB_LIGHT_LEVEL             18    /**< AWB light level */
#define CAMDEV_ILLUPROFILE_NUM             10    /**< The number of illumination profiles */

/*****************************************************************************/
/**
 * @brief    AWB color temperature weight of every lighting
 *
 *****************************************************************************/
typedef float32_t CamDeviceAwbTemWeight_t[CAMDEV_AWB_COLOR_TEM_WEIGHT_NUM];

/*****************************************************************************/
/**
 *
 * @brief   Cam Device AWB status.
 *
 *****************************************************************************/
typedef enum CamDeviceAwbState_e {
    CAMDEV_AWB_STATE_INVALID       = 0,    /**< Invalid status */
    CAMDEV_AWB_STATE_INITIALIZED   = 1,    /**< AWB initialized */
    CAMDEV_AWB_STATE_STOPPED       = 2,    /**< AWB stopped */
    CAMDEV_AWB_STATE_RUNNING       = 3,    /**< AWB running */
    CAMDEV_AWB_STATE_LOCKED        = 4,    /**< AWB locked */
    CAMDEV_AWB_STATE_MAX,
    DUMMY_CAMDEV_AWB_STATE = 0xDEADFEED
} CamDeviceAwbState_t;

/******************************************************************************/
/**
 * @brief   Cam Device AWB mode parameters.
 *
 *****************************************************************************/
typedef enum CamDeviceAwbMode_e {
    CAMDEV_AWB_MODE             = 0,    /**< AWB mode */
    CAMDEV_AWB_METEDATA_MODE    = 1,    /**< AWB METADATA MODE */
    DUMMY_CAMDEV_AWB_MODE = 0xDEADFEED
} CamDeviceAwbMode_t;

/*****************************************************************************/
/**
 * @brief    Cam Device AWB performance optimizationon mode.
 *
 *****************************************************************************/
typedef enum CamDeviceAwbPerformanceOptiMode_s
{
    CAMDEV_AWB_PERFORMANCE_NO_OPTIMIZATION         = 0,    /**< No optimization mode */
    CAMDEV_AWB_PERFORMANCE_GENERAL_OPTIMIZATION    = 1,    /**< General optimization mode */
    CAMDEV_AWB_PERFORMANCE_FAST_OPTIMIZATION       = 2,    /**< Fast optimization mode */
    DUMMY_CAMDEV_AWB_PERFORMANCE = 0xDEADFEED
} CamDeviceAwbPerformanceOptiMode_t;

/******************************************************************************/
/**
 * @brief   Cam Device AWB temperature preference weight parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbTemPreferenceWeight_s {
    bool_t     temptureEnbale;    /**< Temperature weight enable */
    uint16_t preferenceD65;     /**< High temperature preference parameters */
    uint16_t preferenceCwf;     /**< Median temperature preference parameters */
    uint16_t preferenceA;       /**< Low temperature preference parameters */
} CamDeviceAwbTemPreferenceWeight_t;

/******************************************************************************/
/**
 * @brief   Cam Device AWB confidence found position parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbConfoundPosition_s {
    bool_t  customPositionEnable;    /**< Position enable */
    float32_t rg;                      /**< Red green value */
    float32_t bg;                      /**< Blue green value */
    float32_t threshold;               /**< Position threshold */
} CamDeviceAwbConfoundPosition_t;

/******************************************************************************/
/**
 * @brief   Cam Device AWB light weight level parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbLightWeightLevel_s {
    bool_t  lightWeightLevelEnable;                       /**< Light weight level enable */
    float32_t brightnessLevel[CAMDEV_AWB_LIGHT_LEVEL];      /**< Brightness level */
    float32_t weight[CAMDEV_AWB_LIGHT_LEVEL];               /**< Light weight level */
} CamDeviceAwbLightWeightLevel_t;

/******************************************************************************/
/**
 * @brief   Cam Device AWB preference gain parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbPreferenceGain_s {
    bool_t     preferenceGainEnable;                       /**< Grayscale preference enable */
    float32_t    brightnessLevel[CAMDEV_AWB_LIGHT_LEVEL];    /**< Brightness level */
    uint16_t grayRgain[CAMDEV_AWB_LIGHT_LEVEL];          /**< The grayscale of red channel */
    uint16_t grayBgain[CAMDEV_AWB_LIGHT_LEVEL];          /**< The grayscale of green channel */
} CamDeviceAwbPreferenceGain_t;

/******************************************************************************/
/**
 * @brief   Cam Device AWB damping parameters.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbDamping_s
{
    bool_t      useDamping;    /**< Damping mode open */
    bool_t      useManualDampCoff;    /**< Use manual damping coefficient */
    float32_t   manualDampCoff;    /**< Manual damping coefficient */
} CamDeviceAwbDamping_t;

/******************************************************************************/
/**
 * @brief   Cam Device AWB control configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbConfig_s {
    bool_t useCCOffset;    /**< Use color correction offset mask. */
    bool_t useCCMatrix;    /**< Use color correction matrix mask. */
    bool_t useDamping;    /**< Use damping */

    float32_t                         confidenceThreshold[CAMDEV_ILLUPROFILE_NUM];    /**< Light source confidence */
    CamDeviceAwbTemPreferenceWeight_t temPreference;                                  /**< The temperature preference */
    CamDeviceAwbConfoundPosition_t    confoundPointCwf;                               /**< Button function in outdorr with green grass */
    CamDeviceAwbConfoundPosition_t    confoundPointTl84;                              /**< Button function in outdorr with yellow grass */
    CamDeviceAwbConfoundPosition_t    confoundPointD65;                               /**< Button function in outdorr with blue sky */
    CamDeviceAwbLightWeightLevel_t    lightWeightLevel[CAMDEV_ILLUPROFILE_NUM];       /**< The weight of different light source */
    CamDeviceAwbPreferenceGain_t      grayPreference[CAMDEV_ILLUPROFILE_NUM];         /**< Gray scale preference */

    CamDeviceAwbPerformanceOptiMode_t performanceOptiMode;    /** Performance optimization mode */
    float32_t                         awbEnterLockThreshold;    /** Enter lock threshold */
    float32_t                         awbEnterUnlockThreshold;    /** Enter unlock threshold */
    CamDeviceAwbDamping_t             awbDampingCfg;    /** Damping parameters */
} CamDeviceAwbConfig_t;

/*****************************************************************************/
/**
 * @brief    Cam Device AWB front ground configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbFrontGroundConfig_s {
    bool_t     faceAwbEnable;    /**< Enable AWB face function */
    uint8_t  roiNum;           /**< The number of ROI */
    float32_t    faceWeight;       /**< AWB face weight */
    float32_t    rg;               /**< Red green value */
    float32_t    bg;               /**< Blue green value */
} CamDeviceAwbFrontGroundConfig_t;

/*****************************************************************************/
/**
 * @brief   This function registers the AWB library.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pAwbLibHandle       Handle to the AWB library
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceRegisterAwbLib
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function unregisters the AWB library.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceUnRegisterAwbLib
(
    CamDeviceHandle_t  hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function sets the AWB configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to the AWB configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbSetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAwbConfig_t    *pConfig
);


/*****************************************************************************/
/**
 * @brief   This function gets the AWB configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to the AWB configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbGetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceAwbConfig_t   *pConfig
);

/******************************************************************************/
/**
 *
 * @brief   This function sets the AWB mode.
 *
 * @param   hCamDevice   Handle to the CamDevice instance
 * @param   pAwbMode     Pointer to the mode of AWB
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbSetMode
(
    CamDeviceHandle_t              hCamDevice,
    const CamDeviceAwbMode_t      *pAwbMode
);

/*****************************************************************************/
/**
 *
 * @brief   This function gets the AWB mode.
 *
 * @param   hCamDevice   Handle to the CamDevice instance
 * @param   pAwbMode     Pointer to the mode of AWB
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbGetMode
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceAwbMode_t  *pAwbMode
);


/*****************************************************************************/
/**
 * @brief   This function sets the AWB ROI.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pRoi                Pointer to the AWB ROI configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbSetRoi
(
    CamDeviceHandle_t       hCamDevice,
    const CamDeviceRoi_t   *pRoi
);

/*****************************************************************************/
/**
 * @brief   This function gets the AWB ROI.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pRoi                Pointer to the AWB ROI configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbGetRoi
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceRoi_t          *pRoi
);


/*****************************************************************************/
/**
 * @brief   This function gets the AWB temperature weight of every lighting
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pAwbTemWeight       Pointer to the AWB temperature weight
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbGetColorTempWeight
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceAwbTemWeight_t  *pAwbTemWeight
);

/*****************************************************************************/
/**
 * @brief   This function sets front ground configuration.
 *
 * @param   hCamDevice      Handle to the CamDevice instance.
 * @param   pConfig         The pointer to AWB front ground configuration
 *
 * @retval  RET_SUCCESS
 * @retval  RET_INVALID_PARM
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbSetFrontGroundConfig
(
    CamDeviceHandle_t                       hCamDevice,
    const CamDeviceAwbFrontGroundConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets front ground configuration.
 *
 * @param   hCamDevice        Handle to the CamDevice instance.
 * @param   pConfig           The pointer to AWB front ground configuration
 *
 * @retval  RET_SUCCESS
 * @retval  RET_INVALID_PARM
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbGetFrontGroundConfig
(
    CamDeviceHandle_t                  hCamDevice,
    CamDeviceAwbFrontGroundConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables AWB.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_INVALID_PARM    Invalid configuration
 * @retval  RET_OUTOFRANGE      A configuration parameter is out of range
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbEnable
(
    CamDeviceHandle_t           hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables AWB.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets the AWB status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 * @param   pAwbStatus          Pointer to AWB status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbGetStatus
(
    CamDeviceHandle_t           hCamDevice,
    CamDeviceAwbState_t         *pAwbStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets AWB.
 *
 * @param   hCamDevice          Handle to the CamDevice instance.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets the AWB version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to the AWB version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceAwbGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/* @} cam_device_awb_v3 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // __CAMDEV_AWB_API_H__
