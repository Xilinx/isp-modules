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

#ifndef __VSI_ERRNO_H__
#define __VSI_ERRNO_H__

#define VSI_SUCCESS 0
#define VSI_FAILURE (-1)
#define VSI_NULL ((void *)0)

typedef enum ErrCode_E
{
	VSI_ERR_INVALID_DEVID = 1,
	VSI_ERR_INVALID_PORTID = 2,
	VSI_ERR_INVALID_CHNID = 3,
	VSI_ERR_ILLEGAL_PARAM = 4,
	VSI_ERR_EXIST = 5,
	VSI_ERR_UNEXIST = 6,
	VSI_ERR_NULL_PTR = 7,
	VSI_ERR_NOT_CONFIG = 8,
	VSI_ERR_NOT_SUPPORT = 9,
	VSI_ERR_NOT_PERM = 10,
	VSI_ERR_NOMEM = 11,
	VSI_ERR_NOBUF = 12,
	VSI_ERR_BUF_EMPTY = 13,
	VSI_ERR_BUF_FULL = 14,
	VSI_ERR_NOTREADY = 15,
	VSI_ERR_BADADDR = 17,
	VSI_ERR_BUSY = 18,
	VSI_ERR_TIMEOUT = 19,
	VSI_ERR_BUTT = 256
} VSIErrCode_E;

#endif
