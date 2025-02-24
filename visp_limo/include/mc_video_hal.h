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

#ifndef __MC_HAL_H__
#define __MC_HAL_H__
#include "media_device.h"
#include "media_event.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct
	{
		int VideoFd;
	} McVideoHalHandle;

	int MediaVideoHalCreate(MediaEntityAttr *MediaEntity);
	int MediaVideoHalDestroy(MediaEntityAttr *MediaEntity);
	void *MediaVideoHalEventListenThread(void *Param);

#ifdef __cplusplus
}
#endif

#endif