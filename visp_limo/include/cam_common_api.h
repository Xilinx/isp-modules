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

#ifndef __CAM_COMMON_API_H__
#define __CAM_COMMON_API_H__

#include "cam_common_base.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct CamCommonHdr_s
	{
		bool_t supported;
		bool_t enable;
		uint16_t sensorType;
	} CamCommonHdr_t;

	typedef struct CamCommonRgbir_s
	{
		bool_t supported;
		bool_t enable;
		bool_t irRawOutEnable;
		uint16_t irBayerPattern;
	} CamCommonRgbir_t;

	typedef struct CamCommonContext_s
	{
		bool_t calibrationLoaded;
		CamDeviceSensorModeInfo_t sensorMode;
		CamDeviceInputType_t inputType;
		TDatabaseHandle_t hDatabase;
		CamCommonHdr_t hdr;
		CamCommonRgbir_t rgbir;
	} CamCommonContext_t;

	typedef uintptr_t CamCommonHandle_t;

	RESULT VsiCamCommonParseFile(CamCommonHandle_t *hCamCommon,
								 char const *file);

	RESULT VsiCamCommonSensorGetModeInfo(CamCommonHandle_t hCamCommon,
										 CamDeviceHandle_t hCamDevice);

	RESULT VsiCamCommonCreate(CamCommonHandle_t *hCamCommon,
							  CamDeviceInputType_t inputType);

	RESULT VsiCamCommonRegister3ALib(CamCommonHandle_t hCamCommon,
									 CamDeviceHandle_t hCamDevice);

	RESULT VsiCamCommonUnRegister3ALib(CamCommonHandle_t hCamCommon,
									   CamDeviceHandle_t hCamDevice);

	RESULT VsiCamCommonDestroy(CamCommonHandle_t *hCamCommon);

	int VsiCamCommonInitDatabase(TDatabaseHandle_t *hDatabase);

	int VsiCamCommonReleaseDatabase(TDatabaseHandle_t *hDatabase);

#ifdef __cplusplus
}
#endif

#endif // __CAM_COMMON_API_H__
