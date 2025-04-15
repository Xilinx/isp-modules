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

#ifndef CAMDEV_SENSOR_API_H
#define CAMDEV_SENSOR_API_H

#include "cam_device_api.h"

/**
 * @defgroup cam_device_sensor Cam Device Sensor Definitions
 * @{
 *
 *
 */

#define SENSOR_QUERY_NUM 12   /**< Sensor query number */
#define FILE_NAME_LEN 30      /**< Sensor file name length */
#define COMPAND_CURVE_LEN 65  /**< Sensor compand curve length */
#define SENSOR_ID_LEN 11      /**< Image sensor ID length */
#define MODULE_SN_LEN 12      /**< Barcode-Module SN length */
#define CAMDEV_SENSOR_EXP_NUM_MAX 4  /**< Maximum exposure number of sensor*/
#define CAMDEV_SENSOR_METADATA_WIN_NUM_MAX 3  /**< Maximum sensor metedata window number */

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor type.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorType_e {
    CAMDEV_SENSOR_TYPE_LINEAR = 0,          /**< Sensor type linear*/
    CAMDEV_SENSOR_TYPE_STITCHING_HDR,       /**< Sensor type stitching HDR */
    CAMDEV_SENSOR_TYPE_NATIVE_HDR,           /**< Sensor type native HDR */
    DUMMY_CAMDEV_0035 = 0xdeadfeed
}CamDeviceSensorType_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor native HDR mode.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorNativeMode_e {
    CAMDEV_SENSOR_NATIVE_MODE_DCG = 0,          /**< HCG and LCG combined in sensor */
    CAMDEV_SENSOR_NATIVE_MODE_L_AND_S,          /**< L+S combined in sensor */
    CAMDEV_SENSOR_NATIVE_MODE_3DOL,             /**< 3DOL combined in sensor */
    CAMDEV_SENSOR_NATIVE_MODE_4DOL,             /**< 4DOL combined in sensor*/
    CAMDEV_SENSOR_NATIVE_MODE_DCG_SPD_VS,       /**< 4DOL combined in sensor*/
    CAMDEV_SENSOR_NATIVE_MODE_MAX,
    DUMMY_CAMDEV_0036 = 0xdeadfeed
}CamDeviceSensorNativeMode_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor AF mode.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorAutoFocusMode_s {
    CAMDEV_SENSOR_AF_NOTSUPP,       /**< AF not support */
    CAMDEV_SENSOR_AF_MODE_CDAF,     /**< CDAF mode */
    CAMDEV_SENSOR_AF_MODE_PDAF,      /**< PDAF mode */
    DUMMY_CAMDEV_0037 = 0xdeadfeed
}CamDeviceSensorAutoFocusMode_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor focusing position mode.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorFocusPosMode_e {
    CAMDEV_SENSOR_FOCUS_POS_ABSOLUTE = 0,    /**< Absolute focusing mode */
    CAMDEV_SENSOR_FOCUS_POS_RELATIVE,        /**< Relative focusing mode */
    CAMDEV_SENSOR_FOCUS_POS_MAX,
    DUMMY_CAMDEV_0038 = 0xdeadfeed
} CamDeviceSensorFocusPosMode_t;

/******************************************************************************/
/**
 * @brief   Cam Device sensor output data type.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorDataType_e
{
    CAMDEV_SENSOR_DATA_TYPE_BAYER   = 0,   /**< Bayer data type */
    CAMDEV_SENSOR_DATA_TYPE_YUV     = 1,   /**< YUV data type */
    CAMDEV_SENSOR_DATA_TYPE_MAX,
    DUMMY_CAMDEV_0039 = 0xdeadfeed
} CamDeviceSensorDataType_t;

/******************************************************************************/
/**
 * @brief   Cam Device sensor interface type.
 *
 *****************************************************************************/
typedef enum CamDeviceSensorItfType_e
{
    CAMDEV_SENSOR_ITF_TYPE_MIPI_1LANES     = 0,   /**< 1 lane mipi mode */
    CAMDEV_SENSOR_ITF_TYPE_MIPI_2LANES     = 1,   /**< 2 lane mipi mode */
    CAMDEV_SENSOR_ITF_TYPE_MIPI_4LANES     = 2,   /**< 4 lane mipi mode */
    CAMDEV_SENSOR_ITF_TYPE_BT601           = 3,   /**< BT.601 type */
    CAMDEV_SENSOR_ITF_TYPE_MAX,
    DUMMY_CAMDEV_0040 = 0xdeadfeed
} CamDeviceSensorItfType_t;

/*****************************************************************************/
/**
 * @brief   CamDevice isi sensor driver configuration.
 *
 *****************************************************************************/
typedef void* CamDeviceSensorDrvHandle_t;

/*****************************************************************************/
/**
 * @brief   CamDevice isi sensor driver configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorDrvCfg_s{
	CamDeviceSensorDrvHandle_t sensorDrvHandle;
	uint32_t sensorDevId;
}CamDeviceSensorDrvCfg_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor compand configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorCompandCurve_s {
    bool_t   enable;                          /**< Enable value */
    uint8_t  inBit;                           /**< Compand curve input bit value */
    uint8_t  outBit;                          /**< Compand curve output bit value */
    uint8_t  px[COMPAND_CURVE_LEN-1];         /**< X axis interval */
    uint32_t xData[COMPAND_CURVE_LEN];        /**< Compand curve X axis - 65 points */
    uint32_t yData[COMPAND_CURVE_LEN];        /**< Compand curve Y axis - 65 points */
}CamDeviceSensorCompandCurve_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor test pattern configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorTestPattern_s {
    uint32_t enable;                        /**< Test pattern enable value */
    uint32_t pattern;                       /**< Sensor pattern type */
}CamDeviceSensorTestPattern_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor resolution structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorResolution_s {
    uint32_t width;                   /**< Sensor resoulution width */
    uint32_t height;                  /**< Sensor resoulution height */
}CamDeviceSensorResolution_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor resolution collection information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorResolutionCollection_s {
    int index;                                    /**< Sensor mode index */
    CamDeviceSensorResolution_t resolution;       /**< Sensor resolution */
}CamDeviceSensorResolutionCollection_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor register configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorRegister_s {
    uint16_t addr;                /**< Sensor register address */
    uint16_t value;               /**< Sensor register value */
}CamDeviceSensorRegister_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor size information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorSize_s {
	uint32_t boundsWidth;                 /**< Sensor bounds width */
	uint32_t boundsHeight;                /**< Sensor bounds height */
	uint32_t top;                         /**< Top position */
	uint32_t left;                        /**< Left position */
	uint32_t width;                       /**< Real image width */
	uint32_t height;                      /**< Real image height */
}CamDeviceSensorSize_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor compress information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorCompress_s {
	uint32_t enable;                  /**< Enable value */
	uint32_t xBit;                    /**< Expand curve input data bit width */
	uint32_t yBit;                    /**< Expand curve output data bit width */
}CamDeviceSensorCompress_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor mode information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorModeInfo_s {
    uint32_t index;                              /**< Sensor mode index value */
    CamDeviceSensorSize_t size;                  /**< Sensor size */
    CamDeviceSensorType_t sensorType;            /**< The sensor type is linear or HDR*/
    CamDeviceStitchingMode_t     stitchingMode;  /**< The sensor type is HDR stitching*/
    CamDeviceSensorNativeMode_t  nativeMode;     /**< The sensor type is HDR Native */
    uint32_t bitWidth;                           /**< Sensor bit width */
    CamDeviceSensorCompress_t compress;          /**< Sensor compression information */
    CamDeviceRawPattern_t bayerPattern;          /**< Sensor Bayer pattern type*/
    uint32_t maxFps;                             /**< Sensor maximum FPS value */
    CamDeviceSensorAutoFocusMode_t   afMode;     /**< Sensor auto focusing mode */
}CamDeviceSensorModeInfo_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor query information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorQuery_s {
    uint32_t number;                                              /**< Query sensor index number */
    CamDeviceSensorModeInfo_t sensorModeInfo[SENSOR_QUERY_NUM];   /**< Query sensor mode information */
}CamDeviceSensorQuery_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor range information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorRange_s {
    float32_t max;             /**< Maximum value */
    float32_t min;             /**< Minimum value */
    float32_t step;            /**< Step value */
}CamDeviceSensorRange_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor integer range information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorIntegerRange_s {
    uint32_t  max;         /**< Maximum value*/
    uint32_t  min;         /**< Minimum value*/
    uint32_t  step;        /**< Step value */
}CamDeviceSensorIntegerRange_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor motor position structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorFocusPos_s {
    CamDeviceSensorFocusPosMode_t  posMode;  /**< Sensor motor focusing postion mode*/
    uint32_t  position;                      /**< Sensor motor focusing postion, the ranging is from 0 to 1023. */
}CamDeviceSensorFocusPos_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor gain configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorGain_s {
    float32_t aGain[CAMDEV_SENSOR_EXP_NUM_MAX];        /**< Sensor analog gain */
    float32_t dGain[CAMDEV_SENSOR_EXP_NUM_MAX];        /**< Sensor digital gain */
}CamDeviceSensorGain_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor status information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorStatus_s {
    bool_t isConnected;          /**< Sensor connection status */
    CamDeviceSensorGain_t currGain; /**< Sensor current gain info */
    float32_t currIntTime[CAMDEV_SENSOR_EXP_NUM_MAX];              /**< Sensor current integration time info: the unit is microseconds  */
    CamDeviceSensorResolution_t  currRes;      /**< Sensor current resolution info */
    float32_t currFps;                             /**< Sensor current FPS info */
}CamDeviceSensorStatus_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor exposure time configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorExposureControl_s {
    float32_t integrationTime[CAMDEV_SENSOR_EXP_NUM_MAX];        /**< Sensor integration time value */
}CamDeviceSensorExposureControl_t;


/*****************************************************************************/
/**
 * @brief   CamDevice sensor exposure time configuration.
 *
 *****************************************************************************/
typedef union CamDeviceSensorMetadataAttr_s {
    struct {
        uint32_t support  : 1;   /**< bit 0: 0-disable 1-enable */
        uint32_t regInfo  : 1;   /**< bit 1: register information */
        uint32_t expTime  : 1;   /**< bit 2: exposure time */
        uint32_t again    : 1;   /**< bit 3: Analogue agin */
        uint32_t dgain    : 1;   /**< bit 4: Digital gain */
        uint32_t bls      : 1;   /**< bit 5: BLS */
        uint32_t hist     : 1;   /**< bit 6: Histogram */
        uint32_t meanLuma : 1;   /**< bit 7: Mean luminance */
        uint32_t reservedEnable : 24;/**< bit 8:31: Reserved bit */

    }subAttr; /**< Sub-attribute */
    uint32_t mainAttr;  /**< Main attribute */
}CamDeviceSensorMetadataAttr_t;


/*****************************************************************************/
/**
 * @brief   CamDevice sensor meatdata window information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorMetadataWin_s {
    uint8_t winNum;  /**< The number of windows */
    CamDeviceWindow_t metaWin[CAMDEV_SENSOR_METADATA_WIN_NUM_MAX];  /**< Metadata windows */
}CamDeviceSensorMetadataWin_t;


/*****************************************************************************/
/**
 * @brief   CamDevice sensor information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorInfo_s {

    char sensorName[CAMDEV_INPUT_DEV_NAME_LEN];  /**< Sensor name */
    uint32_t sensorRevId;                        /**< Sensor revision ID register */
    CamDeviceRawPattern_t bayerPattern;          /**< Sensor Bayer pattern type */
    CamDeviceSensorTestPattern_t testPattern;    /**< Sensor testPattern info*/

    CamDeviceSensorRange_t  aGainRange[CAMDEV_SENSOR_EXP_NUM_MAX];         /**< Sensor analog gain range*/
    CamDeviceSensorRange_t  dGainRange[CAMDEV_SENSOR_EXP_NUM_MAX];         /**< Sensor digital gain range*/
    CamDeviceSensorRange_t  intTimeRange[CAMDEV_SENSOR_EXP_NUM_MAX];      /**< Sensor integration time range: the unit is microseconds */

    CamDeviceSensorMetadataAttr_t metaSupp;   /**< Sensor metadata support information */
}CamDeviceSensorInfo_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor one time program information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorOtpModuleInfo_s {
    uint16_t hwVersion;                        /**< Module HW version */
    uint16_t SensorEEPROMRevision;             /**< EEPROM revision */
    uint16_t sensorRevision;                   /**< Image sensor revision */
    uint16_t tLensRevision;                    /**< Tlens revision */
    uint16_t ircfRevision;                     /**< Ircf revision */
    uint16_t lensRevision;                     /**< Lens revision */
    uint16_t caRevision;                       /**< Contact assembly revision */
    uint16_t moduleInteID;                     /**< Module integrator ID */
    uint16_t factoryID;                        /**< Factory ID */
    uint16_t mirrorFlip;                       /**< Image mirror and flip */
    uint16_t tLensSlaveID;                     /**< Tlens slave ID */
    uint16_t SensorEEPROMSlaveID;              /**< EEPROM slave ID */
    uint16_t sensorSlaveID;                    /**< Image sensor slave ID */
    uint8_t  sensorID[SENSOR_ID_LEN];          /**< Image sensor ID */
    uint16_t manuDateYear;                     /**< Manufacture Date (Year) */
    uint16_t manuDateMonth;                    /**< Manufacture Date (Month) */
    uint16_t manuDateDay;                      /**< Manufacture Date (Date) */
    uint8_t  barcodeModuleSN[MODULE_SN_LEN];   /**< Barcode-Module SN */
    uint16_t mapTotalSize;                     /**< Total size of EEPROM map */
}CamDeviceSensorOtpModuleInfo_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor driver mode information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorDrvModeInfo_s {
    uint32_t index;                              /**< Sensor mode index value */
    uint32_t width;                       /**< Real image width */
    uint32_t height;                      /**< Real image height */
    CamDeviceSensorType_t sensorType;            /**< The sensor type is linear or HDR*/
    CamDeviceStitchingMode_t     stitchingMode;  /**< The sensor type is HDR stitching*/
    CamDeviceSensorNativeMode_t  nativeMode;     /**< The sensor type is HDR Native */
    uint32_t bitWidth;                           /**< Sensor bit width */
    CamDeviceRawPattern_t bayerPattern;          /**< Sensor Bayer pattern type*/
    uint32_t maxFps;                             /**< Sensor maximum FPS value */
    CamDeviceSensorAutoFocusMode_t   afMode;     /**< Sensor auto focusing mode */
    CamDeviceSensorDataType_t dataType;     /**< Sensor data type */
    CamDeviceSensorItfType_t itfType;    /**< Sensor interface type */
}CamDeviceSensorDrvModeInfo_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor mode list information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorListInfo_s {
    uint32_t number;    /**< Sensor index number */
    char name[20];   /**< Sensor name */
    CamDeviceSensorDrvModeInfo_t sensorModeInfo[SENSOR_QUERY_NUM];   /**< Sensor mode information */
}CamDeviceSensorListInfo_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor connection port information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorConnectPortInfo_s {
    uint32_t chipId;  /**< Sensor chip ID */
    char name[20]; /**< Pointer to sensor name */
}CamDeviceSensorConnectPortInfo_t;

/****************************************************************************/
/**
 * @brief   Get the sensor information, e.g., sensor name, calibration file, etc.
 *
 * @param   hCamDevice          CamDevice driver handle
 * @param   pSensorInfo         Pointer to sensor information structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetInfo
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorInfo_t *pSensorInfo
);

/****************************************************************************/
/**
 * @brief   Open the sensor from the sensor driver.
 *
 * @param   hCamDevice          CamDevice driver handle
 * @param   modeIndex           sensor drv mode index
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorOpen
(
    struct vvcam_isp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    uint32_t          modeIndex
);

/****************************************************************************/
/**
 * @brief   Close the sensor from the sensor driver.
 *
 * @param   hCamDevice          CamDevice driver handle
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorClose
(
    struct vvcam_isp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   Sensor driver handle register.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   sensorDrvHandle     Sensor driver handle
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorDrvHandleRegister
(
    struct vvcam_isp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
	const CamDeviceSensorDrvCfg_t *pSensorDrv
);

/****************************************************************************/
/**
 * @brief   Sensor driver handle un-register.
 *
 * @param   hCamDevice          Cam Device driver handle
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorDrvHandleUnRegister
(
    struct vvcam_isp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice
);

/****************************************************************************/
/**
 * @brief   Changes the sensor output from a live image to a test pattern.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pTestPattern        Pointer to sensor test pattern structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetTestPattern
(
    struct vvcam_isp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    const CamDeviceSensorTestPattern_t *pTestPattern
);

/****************************************************************************/
/**
 * @brief   Mapping the sensor driver.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pSensorName         Pointer to sensor driver name
 * @param   pSensorDrvhandle    Sensor driver handle pointer
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 * @retval  RET_INVALID_PARM    Invalid parameter
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorMapping
(
    struct vvcam_isp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    const char *pSensorName,
    CamDeviceSensorDrvHandle_t *pSensorDrvhandle
);

/****************************************************************************/
/**
 * @brief   Application control set a value via I2C to the sensor registers.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pRegister           Pointer to sensor register structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetRegister
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorRegister_t *pRegister
);

/****************************************************************************/
/**
 * @brief   Application control gets a value via I2C from sensor registers.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pRegister           Pointer to sensor register structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetRegister
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorRegister_t *pRegister
);

/****************************************************************************/
/**
 * @brief   Set the sensor frame rate.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pFps                Pointer to sensor frame per second
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetFrameRate
(
    struct vvcam_isp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    /*float32_t **/uint32_t *pFps
);

/****************************************************************************/
/**
 * @brief   Get the current frame rate of the sensor.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pFps                Pointer to sensor frame per second
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetFrameRate
(
    CamDeviceHandle_t hCamDevice,
    float32_t *pFps
);

/****************************************************************************/
/**
 * @brief   Get the current working sensor mode, including the sensor working
            status, (HDR/Linear, image width/height, etc.).
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pMode               Pointer to sensor mode structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetModeInfo
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorModeInfo_t *pMode
);


/****************************************************************************/
/**
 * @brief   Get all supported modes for the sensor to the application, including
            sensor driver mode(width, height, bit width, etc).
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pQuery              Pointer to sensor query structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorQuery
(
    struct vvcam_isp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorQuery_t *pQuery
);

/****************************************************************************/
/**
 * @brief   Get the current sensor exposure time, gain value, etc.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pStatus             Pointer to sensor status structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetStatus
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorStatus_t *pStatus
);

/****************************************************************************/
/**
 * @brief   Application control set gain value to the sensor driver.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pGain               Pointer to sensor gain structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetGain
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorGain_t *pGain
);

/****************************************************************************/
/**
 * @brief   Application control get gain value from sensor driver.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pGain               Pointer to sensor gain structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported

 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetGain
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorGain_t *pGain
);

/****************************************************************************/
/**
 * @brief   Structure to set exposure time value to the sensor driver.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pExpCtrl            Pointer to sensor exposure structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetExposureControl
(
    CamDeviceHandle_t                  hCamDevice,
    CamDeviceSensorExposureControl_t  *pExpCtrl
);

/****************************************************************************/
/**
 * @brief   Structure to get exposure time value from the sensor driver.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pExpCtrl            Pointer to sensor exposure structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetExposureControl
(
    CamDeviceHandle_t                  hCamDevice,
    CamDeviceSensorExposureControl_t  *pExpCtrl

);

/****************************************************************************/
/**
 * @brief   This function sets sensor motor focusing position.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pFocusPos           Pointer to the focusing position
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetFocusPositon
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorFocusPos_t  *pFocusPos
);

/****************************************************************************/
/**
 * @brief   This function gets sensor motor focusing position.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pFocusPos           Pointer to the focusing position
 * @param   pRangeLimit         Pointer to the focusing position range
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetFocusPositon
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorFocusPos_t  *pFocusPos,
    CamDeviceSensorIntegerRange_t *pRangeLimit
);

/****************************************************************************/
/**
 * @brief   Application control set streaming on or off to the sensor driver.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   isEnable            Enable or Disable the streaming
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetStreaming
(
    CamDeviceHandle_t hCamDevice,
    bool_t isEnable
);

/****************************************************************************/
/**
 * @brief   Get the one-time program information from sensor driver.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pOtpModuleInfo      Pointer to sensor OTP module information structure
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetOtpInfo
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorOtpModuleInfo_t *pOtpModuleInfo
);

/****************************************************************************/
/**
 * @brief   Get sensor metadata support attribute.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pMetaAttr           Pointer to metadata attribute parameters
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetMetadataAttr
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorMetadataAttr_t *pMetaAttr
);

/****************************************************************************/
/**
 * @brief   Set sensor metadata enable attribute.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   metaAttr            Metadata attribute parameters
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorSetMetadataAttr
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorMetadataAttr_t metaAttr
);

/****************************************************************************/
/**
 * @brief   Get sensor metadata window information.
 *
 * @param   hCamDevice          Cam Device driver handle
 * @param   pMetaWin            Pointer to sensor metadata window info
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid handle
 * @retval  RET_NULL_POINTER    Null pointer
 * @retval  RET_WRONG_STATE     State machine in wrong state
 * @retval  RET_NOTSUPP         Feature not supported
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetMetadataWin
(
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorMetadataWin_t *pMetaWin
);

/****************************************************************************/
/**
 * @brief   Get the number of sensors.
 *
 * @param   pNumber     Pointer to sensor number
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetNumber
(
    CamDeviceHandle_t   hCamDevice,
    uint16_t            *pNumber
);

/****************************************************************************/
/**
 * @brief   Get all sensor mode information.
 *
 * @param   hCamDevice          CamDevice driver handle
 * @param   pSensorListInfo     Pointer to sensor mode list
 * @param   sensorNum           The number of sensors
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetListInfo
(
    CamDeviceHandle_t   hCamDevice,
    CamDeviceSensorListInfo_t *pSensorListInfo,
    const uint16_t sensorNum
);

/****************************************************************************/
/**
 * @brief   Get sensor name which is connect to FPGA i2c-8/9/10/11,
 * currently only support i2c-8/9 query.
 *
 * @param   hCamDevice          CamDevice driver handle
 * @param   portId              FPGA port index
 * @param   pPortInfo           Pointer to port information
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceSensorGetConnectPortInfo
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t   hCamDevice,
    CamDeviceMcmPortId_t  portId,
    CamDeviceSensorConnectPortInfo_t *pPortInfo
);

/* @} cam_device_sensor */

#ifdef __cplusplus
extern "C"
{
#endif

#endif    // __CAMDEV_SENSOR_API_H__

