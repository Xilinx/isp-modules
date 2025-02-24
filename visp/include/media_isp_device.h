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

#ifndef __MEDIA_ISP_DEVICE_H__
#define __MEDIA_ISP_DEVICE_H__
#include "media_isp.h"
#if 0
int MediaIspDeviceCreate(MediaIspEventDev *IspEventDev, uint8_t Port);
int MediaIspDeviceDestroy(MediaIspEventDev *IspEventDev, uint8_t Port);
int MediaIspDeviceSetFormat(MediaIspEventDev *IspEventDev, uint8_t Port, uint8_t Chn, MediaFmt *Format);
int MediaIspDeviceQbuf(MediaIspEventDev *IspEventDev, uint8_t Port, uint8_t Chn, MediaBuf *Buf);
int MediaIspDeviceDqbuf(MediaIspEventDev *IspEventDev, uint8_t Port, uint8_t Chn, MediaBuf *Buf);
int MediaIspDeviceStreamOn(MediaIspEventDev *IspEventDev, uint8_t Port, uint8_t Chn);
int MediaIspDeviceStreamOff(MediaIspEventDev *IspEventDev, uint8_t Port, uint8_t Chn);
int MediaIspDeviceCameraConnect(MediaIspEventDev *IspEventDev, uint8_t Port, uint8_t Chn);
int MediaIspDeviceCameraDisConnect(MediaIspEventDev *IspEventDev, uint8_t Port, uint8_t Chn);
int MediaIspDeviceSetCtrl(MediaIspEventDev *IspEventDev, uint8_t Port, MediaCtrlAttr *IspCtrlAttr);
int MediaIspDeviceGetCtrl(MediaIspEventDev *IspEventDev, uint8_t Port, MediaCtrlAttr *IspCtrlAttr);
//int MediaIspDeviceSetFrameRate(MediaIspEventDev *IspEventDev, uint8_t Port, float *FrameRate);

#endif

#endif
