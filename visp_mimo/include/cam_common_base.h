/******************************************************************************\
|* Copyright (c) 2023 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/


#ifndef __CAM_COMMON_BASE_H__
#define __CAM_COMMON_BASE_H__

//#include <ebase/trace.h>
//#include <ebase/offline_trace.h>
#include <ebase/builtins.h>
#include <oslayer/oslayer.h>
#include <cam_device/cam_device_sensor_api.h>
#include "t_database.h"

#ifdef __cplusplus
extern "C"
{
#endif
/* //RKC REMOVED IN M13 VSI release
typedef struct CamCommonContext_s {
    CamDeviceSensorModeInfo_t sensorMode;
    CamDeviceInputType_t inputType;
    TDatabaseHandle_t hDatabase;
    bool hdrEnabled;
    bool rgbirEnabled;
} CamCommonContext_t;

typedef uintptr_t CamCommonHandle_t;
*/

#ifdef __cplusplus
}
#endif

#endif    // __CAM_COMMON_BASE_H__
