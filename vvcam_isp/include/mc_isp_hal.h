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

#ifndef __MC_ISP_HAL_H__
#define __MC_ISP_HAL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int IspFd;
} McIspHalHandle;

int MediaIspHalCreate(MediaEntityAttr *MediaEntity);
int MediaIspHalDestroy(MediaEntityAttr *MediaEntity);
void* MediaIspHalEventListenThread(void *Param);
//int MediaIspHalBufDone(MediaEntityAttr *MediaEntity, int pad, MediaBuf *Buf);

#ifdef __cplusplus
}
#endif

#endif
