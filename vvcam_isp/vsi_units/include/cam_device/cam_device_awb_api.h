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

#define CAMDEV_AWB_COLOR_TEM_WEIGHT_NUM 7

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
    DUMMY_0007 = 0xDEADFEED
} CamDeviceAwbState_t;


/******************************************************************************/
/**
 * @brief   Cam Device AWB mode parameters.
 *
 *****************************************************************************/
typedef enum CamDeviceAwbMode_e {
    CAMDEV_AWB_MODE             = 0,    /**< AWB mode */
    CAMDEV_AWB_METEDATA_MODE    = 1,    /**< AWB METADATA MODE */
    DUMMY_0008 = 0xDEADFEED
} CamDeviceAwbMode_t;

/******************************************************************************/
/**
 * @brief   Cam Device AWB control configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceAwbConfig_s {
    bool useCCOffset;    /**< Use color correction offset mask. */
    bool useCCMatrix;    /**< Use color correction matrix mask. */
    bool useDamping;    /**< Use damping */
} CamDeviceAwbConfig_t;


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
    CamDeviceHandle_t              hCamDevice,
    const CamDeviceAwbConfig_t    *pConfig
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
