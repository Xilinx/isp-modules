/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2022 Vivante Corporation
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

/* VeriSilicon 2020 */


/**
 * @cond HDR_V3_1
 *
 * @defgroup cam_device_hdr_v3_1 Cam Device HDR V3.1 Definitions
 * @{
 *
 */

#ifndef __CAMDEV_HDR_V3_1_API_H__
#define __CAMDEV_HDR_V3_1_API_H__

#include "cam_device_common.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_HDR_RATIO_FRAME_NUM         3      /**< Three sets of exposure merge */
#define CAMDEV_HDR_COLOR_CHANNEL_NUM       3      /**< Three color channels, r/g/b */
#define CAMDEV_HDR_COMB_CURVE_NUM          6      /**< Six HDR combination curves */
#define CAMDEV_HDR_TRANS_VALUE_NUM         2      /**< Transform start and end value */
#define CAMDEV_HDR_DIGITAL_GAIN_NUM        4      /**< gain l, gain s, gain vs, gain e3 */



/******************************************************************************/
/**
 * @brief   Cam Device HDR bypass select.
 *
 *****************************************************************************/
typedef enum CamDeviceHdrBypassSelect_s
{
    CAMDEV_HDR_BYPASS_SELECT_L = 0,   /**< Long frame */
    CAMDEV_HDR_BYPASS_SELECT_S,       /**< Short frame */
    CAMDEV_HDR_BYPASS_SELECT_VS,      /**< Very short frame */
    CAMDEV_HDR_BYPASS_SELECT_E3,       /**< Exposure three frame */
    DUMMY_0056 = 0xDEADFEED
} CamDeviceHdrBypassSelect_t;

/******************************************************************************/
/**
 * @brief   Cam Device HDR configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceHdrStitchConfig_s
{
    CamDeviceHdrBypassSelect_t     bypassSelect;    /**< Bypass select */
    uint8_t                        colorWeight[CAMDEV_HDR_COLOR_CHANNEL_NUM];    /**< Color weight */
    float32_t                          ratio[CAMDEV_HDR_RATIO_FRAME_NUM];    /**< Exposure ratio: [longExp/shortExp], [shortExp/veryShortExp] */
    float32_t                          transRange[CAMDEV_HDR_COMB_CURVE_NUM][CAMDEV_HDR_TRANS_VALUE_NUM];    /**< HDR combine transrange */
} CamDeviceHdrConfig_t;


/******************************************************************************/
/**
 * @brief   Cam Device HDR status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceHdrStatus_s {
    bool_t enable;                  /**< HDR V3.1 enable status */
    CamDeviceConfigMode_t currentMode;       /**< The run mode: 0--manual, 1--auto */
    CamDeviceHdrConfig_t currentCfg;   /**< HDR current configuration*/
}CamDeviceHdrStatus_t;


/*****************************************************************************/
/**
 * @brief   This function sets HDR Lite configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to Hdr configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrSetConfig
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceHdrConfig_t      *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets HDR Lite configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to Hdr configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrGetConfig
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceHdrConfig_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function enables HDR Lite.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrEnable
(
    CamDeviceHandle_t       hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables HDR Lite.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrDisable
(
    CamDeviceHandle_t       hCamDevice
);


/*****************************************************************************/
/**
 * @brief   This function gets HDR Lite status
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pHdrStatus          Pointer to HDR status

 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrGetStatus
(
    CamDeviceHandle_t                       hCamDevice,
    CamDeviceHdrStatus_t                    *pHdrStatus
);

/*****************************************************************************/
/**
 * @brief   This function resets HDR.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrReset
(
    CamDeviceHandle_t             hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets HDR version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to HDR version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHdrGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   (For debug only) This function disables HDR combination mode and sets the HDR bypass frame.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   bypassFrame         The selected bypass frame
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceSetBypassSelectEnable
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceHdrBypassSelect_t  bypassFrame
);

#ifdef __cplusplus
}
#endif


/* @} cam_device_hdr_v3_1 */
/* @endcond */

#endif /* __CAMDEV_HDR_V3_1_API_H__ */

