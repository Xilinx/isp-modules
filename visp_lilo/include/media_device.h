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
 
#ifndef __MEDIA_DEVICE_H__
#define __MEDIA_DEVICE_H__
#include <linux/types.h>

#include "list.h"
#include <linux/mutex.h>


#define MEDIA_DEV_MAX 2

#define VISP_VIDEO "visp-video"
#define VISP "visp-isp"

#define media_fourcc(a, b, c, d)                                    \
	((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | \
	 ((uint32_t)(d) << 24))

#define MEDIA_PIX_FMT_YUYV media_fourcc('Y', 'U', 'Y', 'V')
#define MEDIA_PIX_FMT_NV16 media_fourcc('N', 'V', '1', '6')
#define MEDIA_PIX_FMT_NV12 media_fourcc('N', 'V', '1', '2')

#define MEDIA_PIX_FMT_GREY media_fourcc('G', 'R', 'E', 'Y')
#define MEDIA_PIX_FMT_Y10BPACK media_fourcc('Y', '1', '0', 'B')
#define MEDIA_PIX_FMT_Y10DWA media_fourcc('Y', '1', '0', 'D')
#define MEDIA_PIX_FMT_Y10 media_fourcc('Y', '1', '0', ' ')

#define MEDIA_PIX_FMT_P00BPACK media_fourcc('P', '0', '0', 'B')
#define MEDIA_PIX_FMT_P00DWA media_fourcc('P', '0', '0', 'D')
#define MEDIA_PIX_FMT_P010 media_fourcc('P', '0', '1', '0')
#define MEDIA_PIX_FMT_P02BPACK media_fourcc('P', '0', '2', 'B')

#define MEDIA_PIX_FMT_P20BPACK media_fourcc('P', '2', '0', 'B')
#define MEDIA_PIX_FMT_P20DWA media_fourcc('P', '2', '0', 'D')
#define MEDIA_PIX_FMT_P210 media_fourcc('P', '2', '1', '0')
#define MEDIA_PIX_FMT_P22BPACK media_fourcc('P', '2', '2', 'B')
#define MEDIA_PIX_FMT_I20BPACK media_fourcc('I', '2', '0', 'B')
#define MEDIA_PIX_FMT_I210 media_fourcc('I', '2', '1', '0')

#define MEDIA_PIX_FMT_M48BPACK media_fourcc('M', '4', '8', 'B')
#define MEDIA_PIX_FMT_I48BPACK media_fourcc('I', '4', '8', 'B')
#define MEDIA_PIX_FMT_I48DWA media_fourcc('I', '4', '8', 'D')
#define MEDIA_PIX_FMT_I40DWA media_fourcc('I', '4', '0', 'D')

#define MEDIA_PIX_FMT_RGB24 media_fourcc('R', 'G', 'B', '3')
#define MEDIA_PIX_FMT_RGB24DWA media_fourcc('R', 'G', '3', 'D')
#define MEDIA_PIX_FMT_RGB24P media_fourcc('R', 'G', '3', 'P')

#define MEDIA_PIX_FMT_SBGGR8 media_fourcc('B', 'A', '8', '1')
#define MEDIA_PIX_FMT_SGBRG8 media_fourcc('G', 'B', 'R', 'G')
#define MEDIA_PIX_FMT_SGRBG8 media_fourcc('G', 'R', 'B', 'G')
#define MEDIA_PIX_FMT_SRGGB8 media_fourcc('R', 'G', 'G', 'B')

#define MEDIA_PIX_FMT_SBGGR10 media_fourcc('B', 'G', '1', '0')
#define MEDIA_PIX_FMT_SGBRG10 media_fourcc('G', 'B', '1', '0')
#define MEDIA_PIX_FMT_SGRBG10 media_fourcc('B', 'A', '1', '0')
#define MEDIA_PIX_FMT_SRGGB10 media_fourcc('R', 'G', '1', '0')

#define MEDIA_PIX_FMT_SBGGR10BPACK media_fourcc('B', 'G', '0', 'B')
#define MEDIA_PIX_FMT_SGBRG10BPACK media_fourcc('G', 'B', '0', 'B')
#define MEDIA_PIX_FMT_SGRBG10BPACK media_fourcc('G', 'R', '0', 'B')
#define MEDIA_PIX_FMT_SRGGB10BPACK media_fourcc('R', 'G', '0', 'B')
#define MEDIA_PIX_FMT_SBGGR10DWA media_fourcc('B', 'G', '0', 'D')
#define MEDIA_PIX_FMT_SGBRG10DWA media_fourcc('G', 'B', '0', 'D')
#define MEDIA_PIX_FMT_SGRBG10DWA media_fourcc('G', 'R', '0', 'D')
#define MEDIA_PIX_FMT_SRGGB10DWA media_fourcc('R', 'G', '0', 'D')

#define MEDIA_PIX_FMT_SBGGR12 media_fourcc('B', 'G', '1', '2')
#define MEDIA_PIX_FMT_SGBRG12 media_fourcc('G', 'B', '1', '2')
#define MEDIA_PIX_FMT_SGRBG12 media_fourcc('B', 'A', '1', '2')
#define MEDIA_PIX_FMT_SRGGB12 media_fourcc('R', 'G', '1', '2')

#define MEDIA_PIX_FMT_SBGGR12BPACK media_fourcc('B', 'G', '2', 'B')
#define MEDIA_PIX_FMT_SGBRG12BPACK media_fourcc('G', 'B', '2', 'B')
#define MEDIA_PIX_FMT_SGRBG12BPACK media_fourcc('G', 'R', '2', 'B')
#define MEDIA_PIX_FMT_SRGGB12BPACK media_fourcc('R', 'G', '2', 'B')
#define MEDIA_PIX_FMT_SBGGR12DWA media_fourcc('B', 'G', '2', 'D')
#define MEDIA_PIX_FMT_SGBRG12DWA media_fourcc('G', 'B', '2', 'D')
#define MEDIA_PIX_FMT_SGRBG12DWA media_fourcc('G', 'R', '2', 'D')
#define MEDIA_PIX_FMT_SRGGB12DWA media_fourcc('R', 'G', '2', 'D')

#define MEDIA_PIX_FMT_SBGGR14BPACK media_fourcc('B', 'G', '4', 'B')
#define MEDIA_PIX_FMT_SGBRG14BPACK media_fourcc('G', 'B', '4', 'B')
#define MEDIA_PIX_FMT_SGRBG14BPACK media_fourcc('G', 'R', '4', 'B')
#define MEDIA_PIX_FMT_SRGGB14BPACK media_fourcc('R', 'G', '4', 'B')
#define MEDIA_PIX_FMT_SBGGR14DWA media_fourcc('B', 'G', '4', 'D')
#define MEDIA_PIX_FMT_SGBRG14DWA media_fourcc('G', 'B', '4', 'D')
#define MEDIA_PIX_FMT_SGRBG14DWA media_fourcc('G', 'R', '4', 'D')
#define MEDIA_PIX_FMT_SRGGB14DWA media_fourcc('R', 'G', '4', 'D')

#define MEDIA_PIX_FMT_SBGGR14 media_fourcc('B', 'G', '1', '4')
#define MEDIA_PIX_FMT_SGBRG14 media_fourcc('G', 'B', '1', '4')
#define MEDIA_PIX_FMT_SGRBG14 media_fourcc('G', 'R', '1', '4')
#define MEDIA_PIX_FMT_SRGGB14 media_fourcc('R', 'G', '1', '4')

#define MEDIA_PIX_FMT_SBGGR16 media_fourcc('B', 'Y', 'R', '2')
#define MEDIA_PIX_FMT_SGBRG16 media_fourcc('G', 'B', '1', '6')
#define MEDIA_PIX_FMT_SGRBG16 media_fourcc('G', 'R', '1', '6')
#define MEDIA_PIX_FMT_SRGGB16 media_fourcc('R', 'G', '1', '6')

#define MEDIA_PIX_FMT_SBGGR24 media_fourcc('B', 'G', '2', '4')
#define MEDIA_PIX_FMT_SGBRG24 media_fourcc('G', 'B', '2', '4')
#define MEDIA_PIX_FMT_SGRBG24 media_fourcc('G', 'R', '2', '4')
#define MEDIA_PIX_FMT_SRGGB24 media_fourcc('R', 'G', '2', '4')

typedef struct MediaDeviceAttr_s
{
	void *HalHandle;
	int DevId;
	char DevName[32];
	struct mutex mutex;
	struct ListHead_s EntitiesPool;
} MediaDeviceAttr;

typedef struct MediaPadAttr_s
{
	uint32_t Flags;
	uint32_t Index;
	struct MediaPadAttr_s *RemotePad;
	struct MediaEntityAttr_s *Entity;
} MediaPadAttr;

typedef struct MediaEventModule_s
{
	int (*MediaEventProcess)(struct MediaPadAttr_s *Pad, int cmd, void *data);
	void *Private;
} MediaEventModule;

typedef struct MediaEntityAttr_s
{
	uint32_t Id;
	char Name[32];
	char DevName[32];
	uint32_t PadsCount;
	uint32_t LinksCount;
	struct MediaPadAttr_s *Pads;
	MediaDeviceAttr *MediaDev;
	struct ListHead_s List;
	struct MediaEventModule_s EventModule;
} MediaEntityAttr;

typedef struct MediaLink_s
{
	MediaEntityAttr *Source;
	int SourcePad;
	MediaEntityAttr *Sink;
	int SinkPad;
} MediaLink;

typedef enum MediaThreadStatus_e
{
	MEDIA_THREAD_STOPPED = 0,
	MEDIA_THREAD_PREPARE,
	MEDIA_THREAD_RUNNING,
	MEDIA_THREAD_EXCEPTION,
} MediaThreadStatus;

typedef struct
{
	uint32_t Left;
	uint32_t Top;
	uint32_t Width;
	uint32_t Height;
} MediaRect;

typedef struct
{
	uint32_t Numerator;
	uint32_t Denominator;
} MediaFract;

typedef struct
{
	uint32_t Fourcc;
	MediaRect Rect;
	MediaFract FrmivalMin;
	MediaFract FrmivalMax;
} MediaSinkInfo;

typedef struct
{
	uint32_t Width;
	uint32_t Height;
	uint32_t PixelFormat;
	uint32_t ColorSpace;
	uint32_t Quantization;
} MediaFmt;

typedef struct
{
	uint64_t DmaAddr;
	uint32_t DmaSize;
} MediaPlane;

typedef struct
{
	uint32_t Index;
	uint32_t NumPlanes;
	MediaPlane Planes[8];
} MediaBuf;

int MediaDeviceCreate(int Dev);
int MediaDeviceRelease(int Dev);

#endif
