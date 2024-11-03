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

#ifndef __CAMDEV_TPG_API_H__
#define __CAMDEV_TPG_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @cond TPG
 * 
 * @defgroup cam_device_tpg Cam Device TPG Definitions
 * @{
 *
 */

/******************************************************************************/
/**
 * @brief   Enumeration of TPG picture type.
 *
 *****************************************************************************/
typedef enum CamDeviceISPTpgImageType_e
{
    CAMDEV_ISP_TPG_IMAGE_TYPE_3X3COLORBLOCK    = 0,    /**< 3 x 3 color block */
    CAMDEV_ISP_TPG_IMAGE_TYPE_COLORBAR         = 1,    /**< Color bar */
    CAMDEV_ISP_TPG_IMAGE_TYPE_GRAYBAR          = 2,    /**< Gray bar */
    CAMDEV_ISP_TPG_IMAGE_TYPE_GRID             = 3,    /**< Highlighted grid */
    CAMDEV_ISP_TPG_IMAGE_TYPE_RANDOM           = 4,    /**< Random data */
    CAMDEV_ISP_TPG_IMAGE_TYPE_MAX,
    DUMMY_0067 = 0xDEADFEED
} CamDeviceISPTpgImageType_t;

/******************************************************************************/
/**
* @brief   Enumeration type to configure the bayer pattern in the TPG
*
*****************************************************************************/
typedef enum CamDeviceIspTpgBayerPattern_e
{
    CAMDEV_ISP_TPG_BAYER_PATTERN_RGGB      = 0,        /**< 1st line: RGRG... , 2nd line GBGB... , etc. */
    CAMDEV_ISP_TPG_BAYER_PATTERN_GRBG      = 1,        /**< 1st line: GRGR... , 2nd line BGBG... , etc. */
    CAMDEV_ISP_TPG_BAYER_PATTERN_GBRG      = 2,        /**< 1st line: GBGB... , 2nd line RGRG... , etc. */
    CAMDEV_ISP_TPG_BAYER_PATTERN_BGGR      = 3,        /**< 2st line: BGBG... , 2nd line GRGR... , etc. */
    CAMDEV_ISP_TPG_BAYER_PATTERN_MAX,                   /**< upper border (only for an internal evaluation) */
    DUMMY_0068 = 0xDEADFEED
} CamDeviceIspTpgBayerPattern_t;

/******************************************************************************/
/**
* @brief   Enumeration of TPG data width.
*
*****************************************************************************/
typedef enum CamDeviceIspTpgColorDepth_e
{
    CAMDEV_ISP_TPG_8BIT            = 0,        /**< 8-Bit output */
    CAMDEV_ISP_TPG_10BIT           = 1,        /**< 10-Bit output */
    CAMDEV_ISP_TPG_12BIT           = 2,        /**< 12-Bit output */
    DUMMY_0069 = 0xDEADFEED
} CamDeviceIspTpgColorDepth_t;

/******************************************************************************/
/**
* @brief   Enumeration of TPG resolution.
*
*****************************************************************************/
typedef enum CamDeviceIspTpgResolution_e
{
    CAMDEV_ISP_TPG_1080P           = 0,        /**< 1920 x 1080 resolution */
    CAMDEV_ISP_TPG_720P            = 1,        /**< 1280 x 720 resolution */
    CAMDEV_ISP_TPG_4K              = 2,        /**< 3840 x 2160 resolution */
    CAMDEV_ISP_TPG_USER_DEFINED    = 3,        /**< User-defined resolution */
    DUMMY_0070 = 0xDEADFEED
} CamDeviceIspTpgResolution_t;

/******************************************************************************/
/**
* @brief   TPG user-defined mode.
*
*****************************************************************************/
typedef struct CamDeviceIspTpgUserDefineMode_s
{
    struct
    {
        uint16_t    total;    /**< The total clock */
        uint16_t    fp;    /**< The first valid frame */
        uint16_t    syncHeaderWidth;    /**< Synchronous header width */
        uint16_t    bp;    /**< The distance btween positive edge of vs with positive edge of hde */
        uint16_t    act;    /**< The available clock */
    }H,V;

} CamDeviceIspTpgUserDefineMode_t;

 /******************************************************************************/
/**
 * @brief   Cam Device TPG configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceTpgConfig_s
{
    CamDeviceISPTpgImageType_t        imageType;    /**< Image type */
    CamDeviceIspTpgBayerPattern_t     bayerPattern;    /**< Bayer pattern */
    CamDeviceIspTpgColorDepth_t       colorDepth;    /**< Color depth */
    CamDeviceIspTpgResolution_t       resolution;    /**< TPG resolution */

    uint16_t                        pixleGap;    /**< Pixel gap: image 0 (3x3 block) */
    uint16_t                        lineGap;    /**< Line gap: image 0 (3x3 block) */
    uint16_t                        gapStandard;    /**< Gap standard: image 1, 2, 3 */
    uint32_t                        randomSeed;    /**< Random seed: image 4 */ 
    uint32_t                        frameNum;    /**< Frame number */

    CamDeviceIspTpgUserDefineMode_t   userMode;    /**< User-defined mode */
} CamDeviceTpgConfig_t;


/*****************************************************************************/
/**
 * @brief   This function sets TPG configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pTpgCfg             Pointer to TPG configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgSetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceTpgConfig_t *pTpgCfg
);

/*****************************************************************************/
/**
 * @brief   This function gets TPG configuration.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pTpgCfg             Pointer to TPG configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgGetConfig
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceTpgConfig_t *pTpgCfg
);

/*****************************************************************************/
/**
 * @brief   This function enables TPG.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgEnable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables TPG.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgDisable
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function resets TPG to default parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgReset
(
    CamDeviceHandle_t hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets TPG version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to TPG version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceTpgGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/* @} cam_device_tpg */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif   // __CAMDEV_TPG_API_H__
