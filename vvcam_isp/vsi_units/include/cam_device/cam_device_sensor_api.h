/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2022 Vivante Corporation
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
 ****************************************************************************/

#ifndef __CAMDEV_SENSOR_API_H__
#define __CAMDEV_SENSOR_API_H__

#include "cam_device_api.h"
#include <linux/types.h>
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

#define SENSOR_NAME_LEN 20    /**< Sensor name length */

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
    DUMMY_0063 = 0xDEADFEED
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
    DUMMY_0064 = 0xDEADFEED
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
    DUMMY_0065 = 0xDEADFEED
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
    DUMMY_0066 = 0xDEADFEED
} CamDeviceSensorFocusPosMode_t;

/*****************************************************************************/
/**
 * @brief   Cam Device isi sensor driver configuration.
 *
 *****************************************************************************/
typedef void* CamDeviceSensorDrvHandle_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor compand configuration.
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
 * @brief   Cam Device sensor test pattern configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorTestPattern_s {
    uint32_t enable;                        /**< Test pattern enable value */
    uint32_t pattern;                       /**< Sensor pattern type */
}CamDeviceSensorTestPattern_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor resolution structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorResolution_s {
    uint32_t width;                   /**< Sensor resoulution width */
    uint32_t height;                  /**< Sensor resoulution height */
}CamDeviceSensorResolution_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor resolution collection information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorResolutionCollection_s {
    int index;                                    /**< Sensor mode index */
    CamDeviceSensorResolution_t resolution;       /**< Sensor resolution */
}CamDeviceSensorResolutionCollection_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor register configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorRegister_s {
    uint16_t addr;                /**< Sensor register address */
    uint16_t value;               /**< Sensor register value */
}CamDeviceSensorRegister_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor size information structure.
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
 * @brief   Cam Device sensor compress information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorCompress_s {
	uint32_t enable;                  /**< Enable value */
	uint32_t xBit;                    /**< Expand curve input data bit width */
	uint32_t yBit;                    /**< Expand curve output data bit width */
}CamDeviceSensorCompress_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor mode information structure.
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
 * @brief   Cam Device sensor query information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorQuery_s {
    uint32_t number;                                              /**< Query sensor index number */
    CamDeviceSensorModeInfo_t sensorModeInfo[SENSOR_QUERY_NUM];   /**< Query sensor mode information */
}CamDeviceSensorQuery_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor range information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorRange_s {
    float32_t max;             /**< Maximum value */
    float32_t min;             /**< Minimum value */
    float32_t step;            /**< Step value */
}CamDeviceSensorRange_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor integer range information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorIntegerRange_s {
    uint32_t  max;         /**< Maximum value*/
    uint32_t  min;         /**< Minimum value*/
    uint32_t  step;        /**< Step value */
}CamDeviceSensorIntegerRange_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor motor position structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorFocusPos_s {
    CamDeviceSensorFocusPosMode_t  posMode;  /**< Sensor motor focusing postion mode*/
    uint32_t  position;                      /**< Sensor motor focusing postion, the ranging is from 0 to 1023. */
}CamDeviceSensorFocusPos_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor gain configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorGain_s {
    float32_t aGain[CAMDEV_SENSOR_EXP_NUM_MAX];        /**< Sensor analog gain */
    float32_t dGain[CAMDEV_SENSOR_EXP_NUM_MAX];        /**< Sensor digital gain */
}CamDeviceSensorGain_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor status information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorStatus_s {
    bool isConnected;          /**< Sensor connection status */
    CamDeviceSensorGain_t currGain; /**< Sensor current gain info */
    float32_t currIntTime[CAMDEV_SENSOR_EXP_NUM_MAX];              /**< Sensor current integration time info */
    CamDeviceSensorResolution_t  currRes;      /**< Sensor current resolution info */
    float32_t currFps;                             /**< Sensor current FPS info */
}CamDeviceSensorStatus_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor exposure time configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorExposureControl_s {
    float32_t integrationTime[CAMDEV_SENSOR_EXP_NUM_MAX];        /**< Sensor integration time value */
}CamDeviceSensorExposureControl_t;


/*****************************************************************************/
/**
 * @brief   Cam Device sensor exposure time configuration.
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
 * @brief   Cam Device sensor meatdata window information.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorMetadataWin_s {
    uint8_t winNum;  /**< The number of windows */
    CamDeviceWindow_t metaWin[CAMDEV_SENSOR_METADATA_WIN_NUM_MAX];  /**< Metadata windows */
}CamDeviceSensorMetadataWin_t;


/*****************************************************************************/
/**
 * @brief   Cam Device sensor information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceSensorInfo_s {

    char sensorName[CAMDEV_INPUT_DEV_NAME_LEN];  /**< Sensor name */
    uint32_t sensorRevId;                        /**< Sensor revision ID register */
    CamDeviceRawPattern_t bayerPattern;          /**< Sensor Bayer pattern type */
    CamDeviceSensorTestPattern_t testPattern;    /**< Sensor testPattern info*/

    CamDeviceSensorRange_t  aGainRange[CAMDEV_SENSOR_EXP_NUM_MAX];         /**< Sensor analog gain range*/
    CamDeviceSensorRange_t  dGainRange[CAMDEV_SENSOR_EXP_NUM_MAX];         /**< Sensor digital gain range*/
    CamDeviceSensorRange_t  intTimeRange[CAMDEV_SENSOR_EXP_NUM_MAX];      /**< Sensor integration time range*/

    CamDeviceSensorMetadataAttr_t metaSupp;   /**< Sensor metadata support information */
}CamDeviceSensorInfo_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor one time program information structure.
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

/****************************************************************************/
/**
 * @brief   Get the sensor information, e.g., sensor name, calibration file, etc.
 *
 * @param   hCamDevice          Cam Device driver handle
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
 * @param   hCamDevice          Cam Device driver handle
 * @param   modeIndex           sensor drv mode index
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
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
 * @param   hCamDevice          Cam Device driver handle
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
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
    CamDeviceSensorDrvHandle_t sensorDrvHandle
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
    CamDeviceHandle_t hCamDevice,
    CamDeviceSensorTestPattern_t *pTestPattern
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
    char *pSensorName,
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
    CamDeviceHandle_t hCamDevice,
    float32_t *pFps
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

/* @} cam_device_sensor */

#ifdef __cplusplus
extern "C"
{
#endif

#endif    // __CAMDEV_SENSOR_API_H__

