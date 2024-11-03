/******************************************************************************\
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) 2020-2024 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")  *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __MEDIA_ISP_CALIB_H__
#define __MEDIA_ISP_CALIB_H__
#include "media_isp.h"
//#include "cam_common_api.h"
#if 0
int MediaIspCalibGetCalibXml(MediaIspEventDev *IspEventDev, uint8_t Port, char *CalibXml);
int MediaIspCalibGetAutoJson(MediaIspEventDev *IspEventDev, uint8_t Port, char *AutoJson);
int MediaIspCalibGetManuJson(MediaIspEventDev *IspEventDev, uint8_t Port, char *ManuJson);
int MediaIspCalibGetSensorName(MediaIspEventDev *IspEventDev, uint8_t Port, char *SensorName);
int MediaIspCalibGetSensorMode(MediaIspEventDev *IspEventDev, uint8_t Port, uint8_t *SensorMode);
int MediaIspCalibGetModeInfo(MediaIspEventDev *IspEventDev, uint8_t Port,
                             CamDeviceSensorModeInfo_t *ModeInfo);
int MediaIspCalibQuerySensor(MediaIspEventDev *IspEventDev, uint8_t Port);
int MediaIspCalibLoadSensorInfo(MediaIspEventDev *IspEventDev, uint8_t Port);
#endif
#if 0
int MediaIspCalibLoadCalibration(MediaIspEventDev *IspEventDev, uint8_t Port,
                                CamCommonHandle_t CamCommon);
int MediaIspCalibLoadJsonControl(MediaIspEventDev *IspEventDev, uint8_t Port,
                                CamCommonHandle_t CamCommon);

#endif
#endif
