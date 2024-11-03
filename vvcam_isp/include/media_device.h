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
#ifndef __MEDIA_DEVICE_H__
#define __MEDIA_DEVICE_H__
#include <linux/types.h>
#include "list.h"
//#include "mc_oslayer.h"
#include <linux/mutex.h>

// Declare a mutex
// struct mutex my_mutex;

#define MEDIA_DEV_MAX       2
#if 0
#define MEDIA_PAD_FL_SINK   (1 << 0)
#define MEDIA_PAD_FL_SOURCE (1 << 1)
#endif

#define VVCAM_VIDEO "vvcam-video"
#define VVCAM_ISP  "vvcam-isp"

#define media_fourcc(a, b , c , d) \
    ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#define MEDIA_PIX_FMT_YUYV media_fourcc('Y', 'U', 'Y', 'V')
#define MEDIA_PIX_FMT_NV16 media_fourcc('N', 'V', '1', '6')
#define MEDIA_PIX_FMT_NV12 media_fourcc('N', 'V', '1', '2')

#define MEDIA_PIX_FMT_SBGGR8 media_fourcc('B', 'A', '8', '1')
#define MEDIA_PIX_FMT_SGBRG8 media_fourcc('G', 'B', 'R', 'G')
#define MEDIA_PIX_FMT_SGRBG8 media_fourcc('G', 'R', 'B', 'G')
#define MEDIA_PIX_FMT_SRGGB8 media_fourcc('R', 'G', 'G', 'B')

#define MEDIA_PIX_FMT_SBGGR10 media_fourcc('B', 'G', '1', '0')
#define MEDIA_PIX_FMT_SGBRG10 media_fourcc('G', 'B', '1', '0')
#define MEDIA_PIX_FMT_SGRBG10 media_fourcc('G', 'R', '1', '0')
#define MEDIA_PIX_FMT_SRGGB10 media_fourcc('R', 'G', '1', '0')

#define MEDIA_PIX_FMT_SBGGR12 media_fourcc('B', 'G', '1', '2')
#define MEDIA_PIX_FMT_SGBRG12 media_fourcc('G', 'B', '1', '2')
#define MEDIA_PIX_FMT_SGRBG12 media_fourcc('G', 'R', '1', '2')
#define MEDIA_PIX_FMT_SRGGB12 media_fourcc('R', 'G', '1', '2')

#define MEDIA_PIX_FMT_SBGGR14 media_fourcc('B', 'G', '1', '4')
#define MEDIA_PIX_FMT_SGBRG14 media_fourcc('G', 'B', '1', '4')
#define MEDIA_PIX_FMT_SGRBG14 media_fourcc('G', 'R', '1', '4')
#define MEDIA_PIX_FMT_SRGGB14 media_fourcc('R', 'G', '1', '4')

#define MEDIA_PIX_FMT_SBGGR16 media_fourcc('B', 'Y', 'R', '2')
#define MEDIA_PIX_FMT_SGBRG16 media_fourcc('G', 'B', '1', '6')
#define MEDIA_PIX_FMT_SGRBG16 media_fourcc('G', 'R', '1', '6')
#define MEDIA_PIX_FMT_SRGGB16 media_fourcc('R', 'G', '1', '6')

typedef struct MediaDeviceAttr_s {
    void *HalHandle;
    int DevId;
    char DevName[32];
    struct  mutex mutex;
    struct ListHead_s EntitiesPool;
} MediaDeviceAttr;

typedef struct MediaPadAttr_s {
    uint32_t Flags;
    uint32_t Index;
    struct MediaPadAttr_s *RemotePad;
    struct MediaEntityAttr_s *Entity;
} MediaPadAttr;

typedef struct MediaEventModule_s {
    int (*MediaEventProcess)(struct MediaPadAttr_s *Pad, int cmd, void *data);
    void *Private;
} MediaEventModule;

typedef struct MediaEntityAttr_s {
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

typedef struct MediaLink_s {
    MediaEntityAttr *Source;
    int SourcePad;
    MediaEntityAttr *Sink;
    int SinkPad;
} MediaLink;

typedef enum MediaThreadStatus_e {
    MEDIA_THREAD_STOPPED = 0,
    MEDIA_THREAD_PREPARE,
    MEDIA_THREAD_RUNNING,
    MEDIA_THREAD_EXCEPTION,
} MediaThreadStatus;

typedef struct {
    uint32_t Left;
    uint32_t Top;
    uint32_t Width;
    uint32_t Height;
} MediaRect;

typedef struct {
    uint32_t Numerator;
    uint32_t Denominator;
} MediaFract;

typedef struct {
    uint32_t Fourcc;
    MediaRect Rect;
    MediaFract FrmivalMin;
    MediaFract FrmivalMax;
} MediaSinkInfo;

typedef struct {
    uint32_t Width;
    uint32_t Height;
    uint32_t PixelFormat;
    uint32_t ColorSpace;
    uint32_t Quantization;
} MediaFmt;

typedef struct {
    uint32_t DmaAddr;
	uint32_t DmaSize;
} MediaPlane;

typedef struct {
    uint32_t Index;
    uint32_t NumPlanes;
	MediaPlane Planes[8];
} MediaBuf;

int MediaDeviceCreate(int Dev);
int MediaDeviceRelease(int Dev);

#endif
