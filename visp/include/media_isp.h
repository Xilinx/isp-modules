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

#ifndef __MEDIA_ISP_H__
#define __MEDIA_ISP_H__

#include "cam_device_api.h"
#include "cam_device_buffer_api.h"
#include "cam_device_common.h"
#include "cam_device_sensor_api.h"
#include "media_device.h"
#include "mc_isp_hal.h"

#define MEDIA_ISP_BUF_FRAME_MAX 32
#define MEDIA_ISP_CHAR_LENGTH_MAX 64
#define MEDIA_ISP_PATH_LENGTH_MAX 128

enum media_isp_port_pad_e
{
	MEDIA_ISP_PORT_PAD_SINK = 0,
	MEDIA_ISP_PORT_PAD_SOURCE_MP,
	MEDIA_ISP_PORT_PAD_SOURCE_SP1,
	MEDIA_ISP_PORT_PAD_SOURCE_SP2,
	MEDIA_ISP_PORT_PAD_SOURCE_RAW,
	MEDIA_ISP_PORT_PAD_COUNT,
};

typedef enum MediaIspMcmInputSelect_e
{
	MEDIA_ISP_MCM_INPUT_SELECT_RPU =
		0, /**< alloc mcm input buffer memory from rpu*/
	MEDIA_ISP_MCM_INPUT_SELECT_APU =
		1, /**< alloc mcm input buffer memory from apu */
	MEDIA_ISP_MCM_INPUT_SELECT_MAX, /**< Maximum mcm input buffer memory selection. */
} MediaIspMcmInputSelect;

#define MEDIA_ISP_PORT_MAX 4
#define MEDIA_ISP_CHN_MAX (MEDIA_ISP_PORT_PAD_COUNT - 1)
#define MEDIA_ISP_PAD_NR (MEDIA_ISP_PORT_MAX * MEDIA_ISP_PORT_PAD_COUNT)

int MediaIspEventCreate(MediaEntityAttr *MediaEntity);
int MediaIspEventDestroy(MediaEntityAttr *MediaEntity);

typedef struct AtmInfo_s
{
	uint32_t PortsMask;
	uint32_t RefCount;
} AtmInfo;



typedef struct MediaIspChn_s
{
	MediaFmt Format;
	uint8_t NumBufs;
	MediaBuf Bufs[MEDIA_ISP_BUF_FRAME_MAX];
	void *CamDeviceBufs[MEDIA_ISP_BUF_FRAME_MAX];
} MediaIspChnAttr;

typedef struct MediaIspMcm_s
{
	uint8_t NumBufs;
	MediaIspMcmInputSelect InputSelect;
	MediaBuf Bufs[MEDIA_ISP_BUF_FRAME_MAX];
} MediaIspMcmAttr;

typedef struct MediaIspSensorInfo_s
{
	uint8_t Mode;
	char Name[MEDIA_ISP_CHAR_LENGTH_MAX];
	char CalibXml[MEDIA_ISP_CHAR_LENGTH_MAX];
	char ManuJson[MEDIA_ISP_PATH_LENGTH_MAX];
	char AutoJson[MEDIA_ISP_PATH_LENGTH_MAX];
	char OneJson[MEDIA_ISP_PATH_LENGTH_MAX];
	struct CamDeviceSensorModeInfo_s ModeInfo;
	uint32_t FrameRate;
	uint8_t vc_id;
	uint32_t sensor_id;
	uint32_t i2c_id;
} MediaIspSensorInfo;

#define BUF_MODE_SIZE 16
typedef struct MediaIspPort_s
{
	uint32_t RefCount;
	uint32_t CamDeviceRefMask;
	CamDeviceHandle_t CamDeviceHandle;
	MediaIspChnAttr IspChns[MEDIA_ISP_CHN_MAX];
	MediaSinkInfo SinkInfo;
	MediaIspSensorInfo SensorInfo;
	uint32_t pathOutType[MEDIA_ISP_CHN_MAX];
	uint32_t CameraConnectRefCnt;
	MediaIspMcmAttr McmAttr;
	bool_t OneJsonMode;
	bool_t SensorDrvRegistered;
	struct mutex MainLock;
	char bufmode[BUF_MODE_SIZE];
} MediaIspPortAttr;

typedef struct MediaIspEventDev_s
{
	int DevId;
	uint32_t PortsMask;
	uint32_t RefCount;
	MediaIspPortAttr IspPorts[MEDIA_ISP_PORT_MAX];
	McIspHalHandle HalHandle;
	MediaEntityAttr *MediaEntity;
} MediaIspEventDev;



#endif
