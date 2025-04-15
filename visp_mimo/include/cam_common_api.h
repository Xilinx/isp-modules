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


#ifndef __CAM_COMMON_API_H__
#define __CAM_COMMON_API_H__

#include "cam_common_base.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct CamCommonHdr_s {
    bool_t supported;
    bool_t enable;
    uint16_t sensorType;
} CamCommonHdr_t;

typedef struct CamCommonRgbir_s {
    bool_t supported;
    bool_t enable;
    bool_t irRawOutEnable;
    uint16_t irBayerPattern;
} CamCommonRgbir_t;

typedef struct CamCommonContext_s {
    bool_t calibrationLoaded;
    CamDeviceSensorModeInfo_t sensorMode;
    CamDeviceInputType_t inputType;
    TDatabaseHandle_t hDatabase;
    CamCommonHdr_t hdr;
    CamCommonRgbir_t rgbir;
} CamCommonContext_t;

typedef uintptr_t CamCommonHandle_t;

RESULT VsiCamCommonParseFile
(
    CamCommonHandle_t *hCamCommon,
    char const *file
);

RESULT VsiCamCommonSensorGetModeInfo
(
    CamCommonHandle_t  hCamCommon,
    CamDeviceHandle_t  hCamDevice
);

RESULT VsiCamCommonCreate
(
    CamCommonHandle_t    *hCamCommon,
    CamDeviceInputType_t  inputType
);

RESULT VsiCamCommonRegister3ALib
(
    CamCommonHandle_t  hCamCommon,
    CamDeviceHandle_t  hCamDevice
);

RESULT VsiCamCommonUnRegister3ALib
(
    CamCommonHandle_t  hCamCommon,
    CamDeviceHandle_t  hCamDevice
);

RESULT VsiCamCommonDestroy
(
    CamCommonHandle_t *hCamCommon
);

int VsiCamCommonInitDatabase
(
    TDatabaseHandle_t *hDatabase
);

int VsiCamCommonReleaseDatabase
(
    TDatabaseHandle_t *hDatabase
);

#ifdef __cplusplus
}
#endif

#endif    // __CAM_COMMON_API_H__
