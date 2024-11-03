/******************************************************************************\
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2022> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2022 */

/**
 * @file isi_iss.h
 *
 * @brief Interface description for image sensor specific implementation (iss).
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup isi_iss CamerIc Driver API
 * @{
 *
 */
#ifndef __ISI_ISS_H__
#define __ISI_ISS_H__

//#include <ebase/types.h>
//#include <common/return_codes.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint32_t my_isi_func_pointer;

typedef struct IsiCamDrvConfig_s
{
    uint32_t                     cameraDriverID;
    my_isi_func_pointer            pIsiGetSensorIss;
    uint8_t                      i2cBusId;
} IsiCamDrvConfig_t;

#ifdef __cplusplus
}
#endif

#endif /* __ISI_ISS_H__ */

