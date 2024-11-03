/******************************************************************************\
|* Copyright (c) 2023-2024 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")  *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __MEDIA_ISP_H__
#define __MEDIA_ISP_H__

#include "media_device.h"
#include "media_event.h"
#include "vsi_errno.h"
#include "log.h"
//#include "cam_device_api.h"
#include "mc_isp_hal.h"
#include "cam_device_buffer_api.h"
#include "cam_device_common.h"
#include "cam_device_sensor_api.h"
//#include "vvcam_isp_event.h"

#define MEDIA_ISP_BUF_FRAME_MAX 32
#define MEDIA_ISP_CHAR_LENGTH_MAX 64
#define MEDIA_ISP_PATH_LENGTH_MAX 128
enum media_vvcam_isp_port_pad_e {
	MEDIA_ISP_PORT_PAD_SINK = 0,
	MEDIA_ISP_PORT_PAD_SOURCE_MP,
	MEDIA_ISP_PORT_PAD_SOURCE_SP1,
	MEDIA_ISP_PORT_PAD_SOURCE_SP2,
	MEDIA_ISP_PORT_PAD_SOURCE_RAW,
	MEDIA_ISP_PORT_PAD_COUNT,
};
#define MEDIA_ISP_PORT_MAX  4
#define MEDIA_ISP_CHN_MAX   (MEDIA_ISP_PORT_PAD_COUNT - 1)
#define MEDIA_ISP_PAD_NR    (MEDIA_ISP_PORT_MAX * MEDIA_ISP_PORT_PAD_COUNT)

int MediaIspEventCreate(MediaEntityAttr *MediaEntity);
int MediaIspEventDestroy(MediaEntityAttr *MediaEntity);

typedef struct MediaIspChn_s
{
	//os_pthread_attr_t PthreadAttr;
    //os_pthread_t Pthread;
   // MediaThreadStatus ThreadStatus;
	MediaFmt Format;
	uint8_t NumBufs;
	MediaBuf Bufs[MEDIA_ISP_BUF_FRAME_MAX];
	void *CamDeviceBufs[MEDIA_ISP_BUF_FRAME_MAX];
} MediaIspChnAttr;

typedef struct MediaIspMcm_s
{
	uint8_t NumBufs;
	uint32_t BufAddrs[MEDIA_ISP_BUF_FRAME_MAX];
} MediaIspMcmAttr;

typedef struct MediaIspSensorInfo_s
{
	uint8_t Mode;
	char Name[MEDIA_ISP_CHAR_LENGTH_MAX];
	char CalibXml[MEDIA_ISP_CHAR_LENGTH_MAX];
	char ManuJson[MEDIA_ISP_PATH_LENGTH_MAX];
	char AutoJson[MEDIA_ISP_PATH_LENGTH_MAX];
	CamDeviceSensorModeInfo_t ModeInfo;
} MediaIspSensorInfo;

typedef struct MediaIspPort_s
{
	uint32_t RefCount;
	uint32_t CamDeviceRefMask;
	CamDeviceHandle_t CamDeviceHandle;
	MediaIspChnAttr IspChns[MEDIA_ISP_CHN_MAX];
	MediaSinkInfo SinkInfo;
	MediaIspSensorInfo SensorInfo;
	uint32_t CameraConnectRefCnt;
	MediaIspMcmAttr McmAttr;
} MediaIspPortAttr;

typedef struct MediaIspEventDev_s
{
	int DevId;
	uint32_t PortsMask;
	uint32_t RefCount;
	MediaIspPortAttr IspPorts[MEDIA_ISP_PORT_MAX];
	McIspHalHandle HalHandle;
//	os_pthread_attr_t PthreadAttr;
 //   os_pthread_t      Pthread;
//    MediaThreadStatus ThreadStatus;
	MediaEntityAttr *MediaEntity;
} MediaIspEventDev;


#endif
