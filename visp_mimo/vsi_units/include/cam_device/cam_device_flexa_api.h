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

#ifndef __CAMDEV_FLEXA_API_H__
#define __CAMDEV_FLEXA_API_H__

#include "cam_device_common.h"

/**
 * @cond FLEXA
 *
 * @defgroup cam_device_flexa Cam Device FLEXA Definitions
 * @{
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************/
/**
 * @brief   Cam Device FLEXA SBI configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceEzsbiMiConfig_s {
    uint32_t available;    /**< Available value */
    uint32_t slice_per_frame;    /**< Slice  of per frame */
    uint32_t Y_segment_entry_size;    /**< Y segment entry size */
    uint32_t Y_segment_entry_count;    /**< Y segment entry count */
    uint32_t CB_segment_entry_size;    /**< CB segment entry size */
    uint32_t CB_segment_entry_count;    /**< CB segment entry count */
    uint32_t CR_segment_entry_size;    /**< CR segment entry size */
    uint32_t CR_segment_entry_count;    /**< CR segment entry count */
    int valid_width;    /**< Valid width */
    uint32_t g_sizeY;    /**< Y size */
    uint32_t g_sizeCb;    /**< CB size */
}CamDeviceEzsbiMiConfig_t;


/*****************************************************************************/
/**
 * @brief   This function sets FLEXA configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to FLEXA configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFlexaSbiMiSetConfig
(
    CamDeviceHandle_t                hCamDevice,
    const CamDeviceEzsbiMiConfig_t   *pConfig
);

/*****************************************************************************/
/**
 * @brief   This function gets FLEXA configuration parameters.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 * @param   pConfig             Pointer to FLEXA configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceFlexaSbiMiGetConfig
(
    CamDeviceHandle_t         hCamDevice,
    CamDeviceEzsbiMiConfig_t *pConfig
);

/* @} cam_device_flexa */
/* @endcond */

#ifdef __cplusplus
}
#endif


#endif

