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

#ifndef __MEDIA_EVENT_H__
#define __MEDIA_EVENT_H__

#define MAX_DATA_SIZE (8 * 4096 - 64)

enum MediaEventId {
    MEDIA_EID_CREATE_PIPELINE = 0,
    MEDIA_EID_DESTROY_PIPELINE,
    MEDIA_EID_SET_FMT,
    MEDIA_EID_REQBUFS,
    MEDIA_EID_QBUF,
    MEDIA_EID_STREAMON,
    MEDIA_EID_STREAMOFF,
    MEDIA_EID_S_CTRL,
    MEDIA_EID_G_CTRL,
    MEDIA_EID_MAX,
};

typedef struct MediaCtrlAttr_s {
    uint32_t Cid;
    uint32_t DataSize;
    uint8_t  CtrlData[MAX_DATA_SIZE];
} MediaCtrlAttr;

#endif
