/******************************************************************************\
|* Copyright (c) 2022 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include "cam_device_module_api.h"


USE_TRACER(CAM_DEV_MODULE_INFO);
USE_TRACER(CAM_DEV_MODULE_WARN);
USE_TRACER(CAM_DEV_MODULE_ERR);

RESULT VsiCamDeviceDpfSetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceDpfConfig_t   *pDpfCfg
)
{

    return RET_SUCCESS;
}

RESULT VsiCamDeviceDpfGetConfig
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceDpfConfig_t *pDpfCfg
)
{
    return RET_SUCCESS;
}

RESULT VsiCamDeviceDpfEnable
(
    CamDeviceHandle_t hCamDevice
)
{
    return RET_SUCCESS;
}

RESULT VsiCamDeviceDpfDisable
(
    CamDeviceHandle_t hCamDevice
)
{
    return RET_SUCCESS;
}


RESULT VsiCamDeviceDpfGetStatus
(
    CamDeviceHandle_t       hCamDevice,
    CamDeviceDpfStatus_t   *pDpfStatus
)
{
    return RET_SUCCESS;

}

RESULT VsiCamDeviceDpfReset
(
    CamDeviceHandle_t      hCamDevice
)
{
    return RET_SUCCESS;
}

RESULT VsiCamDeviceDpfGetVersion
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pVersion
)
{

    return RET_SUCCESS;
}

RESULT VsiCamDeviceDpfRawOutputEnable
(
    CamDeviceHandle_t hCamDevice
)
{
    return RET_SUCCESS;
}

RESULT VsiCamDeviceDpfRawOutputDisable
(
    CamDeviceHandle_t hCamDevice
)
{
    return RET_SUCCESS;
}
