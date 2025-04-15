/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
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
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

/**
 * @cond HIST256
 *
 * @defgroup cam_device_hist256 Cam Device HIST256 Definitions
 * @{
 *
 */

#ifndef __CAMDEV_HIST256_API_H__
#define __CAMDEV_HIST256_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_HIST256_GRID_ITEMS         25  /**< The number of grid sub windows */
#define CAMDEV_HIST256_NUM_BINS           256  /**< number of bins */

typedef uint8_t CamDeviceHist256Bins_t[CAMDEV_HIST256_NUM_BINS];      /**< HIST bins */


/******************************************************************************/
/**
 * @brief   Cam Device ISP HIST mode.
 *
 *****************************************************************************/
typedef enum CamDeviceHist256Mode_e {
    CAMDEV_HIST256_MODE_RGB_COMBINED  = 0,    /**< RGB combined histogram */
    CAMDEV_HIST256_MODE_R             = 1,    /**< R histogram */
    CAMDEV_HIST256_MODE_G             = 2,    /**< G histogram */
    CAMDEV_HIST256_MODE_B             = 3,    /**< B histogram */
    CAMDEV_HIST256_MODE_Y             = 4,    /**< Luminance histogram */
    CAMDEV_HIST256_MODE_MAX,        /**< upper border (only for an internal evaluation) */
    DUMMY_0059 = 0xDEADFEED
} CamDeviceHist256Mode_t;

/******************************************************************************/
/**
 * @brief   Cam Device HIST configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceHist256Config_s
{
    CamDeviceHist256Mode_t mode;                  /**< HIST mode */
    uint8_t gridWeight[CAMDEV_HIST256_GRID_ITEMS];    /**< HIST weight */
} CamDeviceHist256Config_t;

/******************************************************************************/
/**
 * @brief   Cam Device HIST status structure.
 *
 *****************************************************************************/
typedef struct CamDeviceHist256Status_s
{
    bool_t  enable;    /**< HIST enable status*/
}CamDeviceHist256Status_t;


/*****************************************************************************/
/**
 * @brief   This function sets HIST configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to HIST configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256SetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceHist256Config_t    *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to HIST configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256GetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceHist256Config_t    *pConfig
);


/*****************************************************************************/
/**
 * @brief   This function sets HIST64 measure window.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pWindow             Pointer to HIST256 measure window
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256SetMeasureWindow
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceWindow_t     *pWindow
);


/*****************************************************************************/
/**
 * @brief   This function gets HIST256 measure window.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pWindow             Pointer to HIST256 measure window
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256GetMeasureWindow
(
    CamDeviceHandle_t      hCamDevice,
    CamDeviceWindow_t     *pWindow
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST256 bins.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pHistBin            Pointer to HIST256 bins
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256GetBins
(
    CamDeviceHandle_t        hCamDevice,
    CamDeviceHist256Bins_t  *pHistBin
);


/*****************************************************************************/
/**
 * @brief   This function enables HIST.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256Enable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function disables HIST.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256Disable
(
    CamDeviceHandle_t   hCamDevice
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST status.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pStatus             Pointer to HIST status
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256GetStatus
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceHist256Status_t    *pStatus
);

/*****************************************************************************/
/**
 * @brief   This function gets HIST version.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pVersion            Pointer to HIST version
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256GetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t *pVersion
);

/*****************************************************************************/
/**
 * @brief   This function resets HIST256.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceHist256Reset
(
    CamDeviceHandle_t         hCamDevice
);

/* @} cam_device_hist256 */
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /* __CAMDEV_HIST256_API_H__ */

