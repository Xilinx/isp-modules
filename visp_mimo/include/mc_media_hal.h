/******************************************************************************\
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __MC_MEDIA_HAL_H__
#define __MC_MEDIA_HAL_H__
#include "media_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int MediaFd;
} McMediaHalHandle;

int MediaDeviceHalCreate(MediaDeviceAttr *MediaDev);
int MediaDeviceHalRelease(MediaDeviceAttr *MediaDev);
int MediaDeviceHalCreateEntities(MediaDeviceAttr *MediaDev);
int MediaDeviceHalDestroyEntities(MediaDeviceAttr *MediaDev);
int MediaDeviceHalCreateLinks(MediaDeviceAttr *MediaDev);

#ifdef __cplusplus
}
#endif

#endif