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

#ifndef __CAMDEV_API_H__
#define __CAMDEV_API_H__

#include "cam_device_calibration.h"
#include "cam_device_common.h"
//#include "visp_driver.h"
//#include <ebase/offline_trace.h>

#ifdef __cplusplus
extern "C"
{
#endif

	//typedef void* CamDeviceHandle_t;

	/**
	 * @defgroup cam_device_pipeline Cam Device Pipeline Definitions
	 * @{
	 *
	 *
	 */

	//typedef struct CamDeviceStreamModeConfig_s {
	//    uint8_t nop;
	//}CamDeviceStreamModeConfig_t;
	//
	//typedef struct CamDeviceRdmaModeConfig_s {
	//    uint8_t nop;
	//}CamDeviceRdmaModeConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure MCM mode parameters.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceMcmModeConfig_s
	{
		CamDeviceMcmPortId_t
			portId; /**< Port index of MCM which indicates the sensor hardware connect position*/
		CamDeviceMcmOperation_t mcmOp;	/**< MCM operation mode */
		CamDeviceMcmSelection_t mcmSel; /**< MCM selection mode */
	} CamDeviceMcmModeConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure stream mode parameters.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceStreamModeConfig_s
	{
		CamDeviceStreamPortId_t
			portId; /**< Port index of MCM which indicates the sensor hardware connect position*/
	} CamDeviceStreamModeConfig_t;

	/*****************************************************************************/
	/**
	 *  * @brief   Structure to configure betch mode parameters.
	 *   *
	 *    *****************************************************************************/
	typedef struct CamDeviceBatchModeConfig_s
	{
		CamDeviceStreamPortId_t
			portId; /**< Port index of MCM which indicates the sensor hardware connect position*/
		uint8_t batchNumber; /**< Buffer number of one batch*/
	} CamDeviceBatchModeConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure tile mode parameters. Reserved.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceTileModeConfig_s
	{
		CamDeviceTileOperation_t tileOp; /**< Tile operation mode */
		CamDeviceTileJoint_t tileNum;	 /**< Tile number */
		CamDeviceTileXAxis_t
			xPices; /**< Tile joint mode set user, x direction tile number */
		CamDeviceTileYAxis_t
			yPices; /**< Tile joint mode set user, y direction tile number */
	} CamDeviceTileModeConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure ISP work mode.
	 *
	 *****************************************************************************/
	typedef union CamDeviceModeConfig_s
	{
		CamDeviceStreamModeConfig_t
			stream; /**< Stream mode configuration parameters */
					//    CamDeviceRdmaModeConfig_t   rdma;     /**< Reseverd */
		CamDeviceMcmModeConfig_t mcm; /**< MCM mode configuration parameters */
		CamDeviceBatchModeConfig_t
			batch; /**< Batch mode configuration parameters */
	} CamDeviceModeConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure ISP input.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceInputConfig_s
	{
		CamDeviceInputType_t inputType; /**< ISP input type */
		char inputDevName
			[CAMDEV_INPUT_DEV_NAME_LEN]; /**< ISP input device name */
	} CamDeviceInputConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure ISP output.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceOutputConfig_s
	{
		CamDeviceOutputType_t outputType; /**< ISP output type */
	} CamDeviceOutputConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure work parameters for camera device.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceWorkConfig_s
	{
		CamDeviceWorkMode_t workMode;  /**< ISP software work mode */
		CamDeviceModeConfig_t modeCfg; /**< Work mode configurations */
		CamDeviceTileModeConfig_t
			tileCfg; /**< Tile mode configurations. It can be used together with other modes*/
	} CamDeviceWorkConfig_t;

/*****************************************************************************/
/**
 * @brief    Structure to create camera device instance.
 *
 *****************************************************************************/
#if 1
	typedef struct CamDeviceConfig_s
	{
		uint32_t ispHwId; /**< Hardware Pipeline ID. */
		CamDeviceInputConfig_t
			inputCfg; /**< Input device configuration parameters */
		CamDeviceWorkConfig_t workCfg; /**< ISP Work configuration parameters */
		CamDeviceOutputConfig_t
			outputCfg; /**< ISP output configuration parameters */
		CamDeviceSwitchSeqPriority_t
			priority;	/**< Input device priority in switch control */
		void *hCascade; /**< Cascade ctx handle */
		struct visp_dev *isp_dev;
	} CamDeviceConfig_t;
#endif

	/*****************************************************************************/
	/**
	 * @brief   Structure to set ISP pipeline output format.
	 *
	 *****************************************************************************/
	typedef struct CamDevicePipeOutFmt_s
	{
		uint32_t outWidth;					/**< Width of output image */
		uint32_t outHeight;					/**< Height of output image */
		uint32_t pathOutType;				/**< path out type */
		CamDevicePipePixOutFmt_t outFormat; /**< Format of output pixel */
		uint32_t dataBits; /**< Data width of each color component [8~14] */
		uint8_t
			alpha; /**< Alpha value of ARGB format , range 0~255 for ARGB/yuv8bit, range 0~3 for yuv10bit */
		CamDeviceMiYuvOrder_t
			yuvOrder; /**< The difference order of the three yuv channal save in ddr */
		CamDeviceMiSwap_u swap; /**< Output format swap control information */
	} CamDevicePipeOutFmt_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to set ISP output streaming.
	 *
	 *****************************************************************************/
	typedef struct CamDevicePathStreamingCfg_s
	{
		uint8_t outPathEnable; /**< Path streaming configuration */
	} CamDevicePathStreamingCfg_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to set ISP DMA read image input format.
	 *
	 *****************************************************************************/
	typedef struct CamDevicePipeInFmt_s
	{
		uint32_t inWidth;				 /**< Width of input image */
		uint32_t inHeight;				 /**< Height of input image */
		CamDeviceInputRawFmt_t inFormat; /**< Format of input pixel */
		CamDeviceBitDepth_t inBit;		 /**< Bit depth of input device: tpg */
		CamDeviceRawPattern_t
			inPattern; /**< Bayer pattern of input RAW RGB/RGBIR image */
		CamDeviceStitchingMode_t
			stitchMode; /**< This member should be configured when loading HDR image*/
	} CamDevicePipeInFmt_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure windows in ISP pipeline.
	 *
	 *****************************************************************************/
	typedef struct CamDevicePipeIspWindow_s
	{
		CamDeviceWindow_t cropWindow; /**< Crop window for output path */
	} CamDevicePipeIspWindow_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure submodules of pipeline when connecting camera.
	 *
	 *****************************************************************************/
	typedef union CamDevicePipeSubmoduleCtrl_s
	{
		struct
		{
			uint32_t aeEnable : 1;		 /**< bit 0: 0-disable 1-enable */
			uint32_t afEnable : 1;		 /**< bit 1 */
			uint32_t hdrEnable : 1;		 /**< bit 2 */
			uint32_t awbEnable : 1;		 /**< bit 3 */
			uint32_t ccmEnable : 1;		 /**< bit 4 */
			uint32_t compressEnable : 1; /**< bit 5 */
			uint32_t expandEnable : 1;	 /**< bit 6 */
			uint32_t cnrEnable : 1;		 /**< bit 7 */
			uint32_t ynrEnable : 1;		 /**< bit 8 */
			uint32_t cprocEnable : 1;	 /**< bit 9 */
			uint32_t dciEnable : 1;		 /**< bit 10 */
			uint32_t demosaicEnable : 1; /**< bit 11 */
			uint32_t dgEnable : 1;		 /**< bit 12 */
			uint32_t dpccEnable : 1;	 /**< bit 13 */
			uint32_t dpfEnable : 1;		 /**< bit 14 */
			uint32_t eeEnable : 1;		 /**< bit 15 */
			uint32_t gcEnable : 1;		 /**< bit 16 */
			uint32_t geEnable : 1;		 /**< bit 17 */
			uint32_t gtmEnable : 1;		 /**< bit 18 */
			uint32_t lscEnable : 1;		 /**< bit 19 */
			uint32_t lut3dEnable : 1;	 /**< bit 20 */
			uint32_t pdafEnable : 1;	 /**< bit 21 */
			uint32_t rgbirEnable : 1;	 /**< bit 22 */
			uint32_t wbEnable : 1;		 /**< bit 23 */
			uint32_t wdrEnable : 1;		 /**< bit 24 */
			uint32_t dnr3Enable : 1;	 /**< bit 25 */
			uint32_t dnr2Enable : 1;	 /**< bit 26 */
			uint32_t reservedEnable : 5; /**< bit 27:31 */
		} subCtrl;
		uint32_t allCtrl;
	} CamDevicePipeSubmoduleCtrl_u;

#define CAMDEV_IMAGE_EXP_NUM_MAX 4 /**< Maximum exposure number of image*/

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure image exposure control information.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceImageExposureControl_s
	{
		uint32_t frameIndex;
		float32_t imageGain[CAMDEV_IMAGE_EXP_NUM_MAX];
		/**< In linear mode or native HDR mode:\n imageGain[0] is image gain\n
			 In stitch HDR mode:\n
			 imageGain[0]: L image gain\n
			 imageGain[1]: S image gain\n
			 imageGain[2]: VS image gain\n
			 imageGain[3]: ES image gain */
		float32_t imageIntTime[CAMDEV_IMAGE_EXP_NUM_MAX];
		/**< In linear mode or native HDR mode:\n imageIntTime[0] is image integration time\n
		 In stitch HDR mode:\n
		 imageIntTime[0]: L image integration time\n
		 imageIntTime[1]: S image integration time\n
		 imageIntTime[2]: VS image integration time\n
		 imageIntTime[3]: ES image integration time */
	} CamDeviceImageExposureControl_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to histogram metadata  information.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceMetadataHist_s
	{
		uint8_t histWinNum;
		CamDeviceWindow_t histRoiWin[CAM_DEVICE_ROI_WINDOWS_MAX];
		CamDeviceHistBins_t redChannelBins[CAM_DEVICE_ROI_WINDOWS_MAX];
		CamDeviceHistBins_t grChannelBins[CAM_DEVICE_ROI_WINDOWS_MAX];
		CamDeviceHistBins_t gbChannelBins[CAM_DEVICE_ROI_WINDOWS_MAX];
		CamDeviceHistBins_t blueChannelBins[CAM_DEVICE_ROI_WINDOWS_MAX];
		CamDeviceHistBins_t totalBins[CAM_DEVICE_ROI_WINDOWS_MAX];
	} CamDeviceMetadataHist_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to mean luminance metadata  information.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceMetadataMeanLuma_s
	{
		uint8_t meanWinNum;
		CamDeviceWindow_t meanRoiWin[CAM_DEVICE_ROI_WINDOWS_MAX];
		uint32_t redChannelMean[CAM_DEVICE_ROI_WINDOWS_MAX];
		uint32_t grChannelMean[CAM_DEVICE_ROI_WINDOWS_MAX];
		uint32_t gbChannelMean[CAM_DEVICE_ROI_WINDOWS_MAX];
		uint32_t blueChannelMean[CAM_DEVICE_ROI_WINDOWS_MAX];
		uint32_t totalMean[CAM_DEVICE_ROI_WINDOWS_MAX];

	} CamDeviceMetadataMeanLuma_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to metadata exposure information.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceMetadataInfo_s
	{
		uint32_t chipId;
		uint64_t frameCount;

		float32_t junctionTemperature;
		uint32_t blackLevelPedestal;

		uint8_t exposureNum; /**< The number of exposures */
		uint32_t integrationTime
			[CAMDEV_EXPOSURE_FRAME_MAX]; /**< Exposure time(unit: us) */
		CamDeviceIntegrationTimeRange_t integrationTimeRange
			[CAMDEV_EXPOSURE_FRAME_MAX]; /**< Exposure time(unit: us) */

		float32_t analogGain[CAMDEV_EXPOSURE_FRAME_MAX];
		float32_t digitalGain[CAMDEV_EXPOSURE_FRAME_MAX];
		float32_t wbGain[CAMDEV_EXPOSURE_FRAME_MAX][CAMDEV_RAW_CHANNEL_NUM];
		float32_t dualConvGain[CAMDEV_EXPOSURE_FRAME_MAX];

		CamDeviceMetadataHist_t histBins[CAMDEV_EXPOSURE_FRAME_MAX]; //reserved
		CamDeviceMetadataMeanLuma_t
			meanLuma[CAMDEV_EXPOSURE_FRAME_MAX]; //reserved

	} CamDeviceMetadataInfo_t;

	/*****************************************************************************/
	/**
	 * @brief   Structure to configure metadata information.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceMetadataConfig_s
	{
		CamDeviceAwbWorkMode_t modeAwb;	  /**< Choose awb mode */
		CamDeviceAeWorkMode_t modeAe;	  /**< Choose ae mode */
		CamDeviceMetadataInfo_t metaInfo; /**< Metadata information */
	} CamDeviceMetadataConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   This function creates and initializes a CamDevice instance.
	 *
	 * @param   pCamConfig          Instance configuration.
	 * @param   hCamDevice          Handle to the CamDevice instance.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_NULL_POINTER    The/one/all parameter(s) is a(are) NULL pointer(s)
	 * @retval  RET_INVALID_PARM    Invalid configuration
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceCreate(struct visp_dev *isp_dev,
							  CamDeviceConfig_t *pCamConfig,
							  CamDeviceHandle_t *hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function destroys a CamDevice instance.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_NULL_POINTER    The/one/all parameter(s) is a(are) NULL pointer(s)
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDestroy(struct visp_dev *isp_dev,
							   CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function loads the calibration parameters into the database of
	 *          software stack.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance.
	 * @param   defCalibIllum       Select the default illumination when pipeline is
	 *                              connected.
	 * @param   pCalibCfg           Configuration structure for calibration parameter.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceLoadCalibration(CamDeviceHandle_t hCamDevice,
									   CamDeviceCalibIllumType_t defCalibIllum,
									   CamDeviceCalibCfg_t *pCalibCfg);

	/*****************************************************************************/
	/**
	 * @brief   This function sets the output format of main or self path.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   path                    Select the output path ID
	 * @param   pFmt                    Configuration structure for output format
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceSetOutFormat(struct visp_dev *isp_dev,
									CamDeviceHandle_t hCamDevice,
									CamDevicePipeOutPathType_t path,
									CamDevicePipeOutFmt_t *pFmt);

	/*****************************************************************************/
	/**
	 * @brief   This function sets the Input format.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   path                    Select the input path ID
	 * @param   pFmt                    Configuration structure for input format
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceSetInFormat(struct visp_dev *isp_dev,
								   CamDeviceHandle_t hCamDevice,
								   CamDevicePipeInPathType_t path,
								   CamDevicePipeInFmt_t *pFmt);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the output format of main or self path.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   path                    Select the output path ID
	 * @param   pFmt                    Configuration structure for output format
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGetOutFormat(CamDeviceConfig_t *pCamConfig,
									CamDeviceHandle_t hCamDevice,
									CamDevicePipeOutPathType_t path,
									CamDevicePipeOutFmt_t *pFmt);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the input format of main or self path.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   path                    Select the input path ID
	 * @param   pFmt                    Configuration structure for input format
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGetInFormat(CamDeviceConfig_t *pCamConfig,
								   CamDeviceHandle_t hCamDevice,
								   CamDevicePipeInPathType_t path,
								   CamDevicePipeInFmt_t *pFmt);

	/*****************************************************************************/
	/**
	 * @brief   This function sets windows in the ISP pipeline.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   path                    Config the output path ID
	 * @param   pIspWindow              Configuration structure for windows in pipeline
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceSetIspWindow(CamDeviceConfig_t *pCamConfig,
									CamDeviceHandle_t hCamDevice,
									CamDevicePipeOutPathType_t path,
									const CamDevicePipeIspWindow_t *pIspWindow);

	/*****************************************************************************/
	/**
	 * @brief   This function gets windows in the ISP pipeline.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   path                    Config the output path ID
	 * @param   pIspWindow              Pointer to ISP windows in pipeline
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGetIspWindow(CamDeviceConfig_t *pCamConfig,
									CamDeviceHandle_t hCamDevice,
									CamDevicePipeOutPathType_t path,
									CamDevicePipeIspWindow_t *pIspWindow);

	/*****************************************************************************/
	/**
	 * @brief   This function sets up the whole camera system. First, initialize the
	 *          CamEngine and set the pipeline path. Then, connect the input
	 *          (sensor or image) with the CamEngine to process the image data.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   pSubCtrl                Control the submodules when setting up the
	 *                                  camera system.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceConnectCamera(
		struct visp_dev *isp_dev, CamDeviceHandle_t hCamDevice,
		const CamDevicePipeSubmoduleCtrl_u *pSubCtrl);

	/*****************************************************************************/
	/**
	 * @brief   This function disconnects the whole camera system.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDisconnectCamera(struct visp_dev *isp_dev,
										CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the version ID.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   pVersionId              Version ID which is related to the ISP
	 *                                  hardware version.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGetSoftwareVersion(CamDeviceHandle_t hCamDevice,
										  char *pVersionId);

	/*****************************************************************************/
	/**
	 * @brief   This function sets output path streaming.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   pConfig                 Pointer to the output path streaming configuration
	 *
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceSetPathStreaming(struct visp_dev *isp_dev,
										CamDeviceHandle_t hCamDevice,
										CamDevicePathStreamingCfg_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function gets output path streaming status.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   path                    Pointer to the output path streaming configuration
	 *
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGetPathStreaming(struct visp_dev *isp_dev,
										CamDeviceHandle_t hCamDevice,
										CamDevicePathStreamingCfg_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function starts pipeline path.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance.
	 * @param   path                Pipeline output path.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceStartPath(CamDeviceConfig_t *pCamConfig,
								 CamDeviceHandle_t hCamDevice,
								 CamDevicePipeOutPathType_t path);

	/*****************************************************************************/
	/**
	 * @brief   This function stops pipeline path.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance.
	 * @param   path                Pipeline output path.
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         General failure
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceStopPath(CamDeviceConfig_t *pCamConfig,
								CamDeviceHandle_t hCamDevice,
								CamDevicePipeOutPathType_t path);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the instance ID.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance.
	 * @param   pHwId               Pointer to hardware pipeline ID
	 *
	 * @return                      Return the instance ID.
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGetHardwareId(CamDeviceHandle_t hCamDevice,
									 uint32_t *pHwId);

	//uint32_t CamDeviceGetVirtualId
	//(
	//    uint32_t instanceId
	//);

	/*****************************************************************************/
	/**
	 * @brief   This function allocates the memory from internal reserved memory.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   byteSize            The total byte size of allocate memory
	 * @param   pBaseAddress        The pointer of physical base address of allocated memory
	 * @param   pIplAddress         The pointer of the mapped virtual base address
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceAllocResMemory(struct visp_dev *isp_dev,
									  CamDeviceHandle_t hCamDevice,
									  uint32_t byteSize, uint32_t *pBaseAddress,
									  void **pIplAddress);

	/*****************************************************************************/
	/**
	 * @brief   This function frees the memory of internal reserved memory.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   baseAddress         The physical base address of free memory
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceFreeResMemory(struct visp_dev *isp_dev,
									 CamDeviceHandle_t hCamDevice,
									 uint32_t baseAddress);

#ifdef ISP_OFFLINE_TEST
	/*****************************************************************************/
	/**
	 * @brief   This function sets the name of offline case.
	 *
	 * @param   pCaseName               The name of offline case
	 * @param   len                     The string length for pCaseName
	 *
	 * @return                     void.
	 *
	 *****************************************************************************/
	void VsiCamDeviceSetCaseName(char *pCaseName, uint32_t len);
#endif

	/*****************************************************************************/
	/**
	 * @brief   This function writes data to the specified register.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   address             Data address
	 * @param   value               Data value
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceWriteRegister(CamDeviceHandle_t hCamDevice,
									 uint32_t address, uint32_t value);

	/*****************************************************************************/
	/**
	 * @brief   This function reads data from the specified register.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   address             Data address
	 * @param   pValue              Pointer to data value
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceReadRegister(CamDeviceHandle_t hCamDevice,
									uint32_t address, uint32_t *pValue);

	/*****************************************************************************/
	/**
	 * @brief   This function sets image metadata configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pMetadata           Pointer to metadata value
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceConfigMetadata(CamDeviceHandle_t hCamDevice,
									  CamDeviceMetadataConfig_t *pMetadata);

	/*****************************************************************************/
	/**
	 * @brief   This function enables noise removal relocation.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceNrRelocEnable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables noise removal relocation.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceNrRelocDisable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function does fast ISP software reset.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceSwFastStop(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function does hardware system reset.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHwSystemReset(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function does fast ISP software reset.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_WRONG_HANDLE    Invalid instance handle
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceSwFastStart(CamDeviceHandle_t hCamDevice);

	/* @} cam_device_pipeline */
RESULT VsiCamDeviceStartStreaming(CamDeviceConfig_t *pCamConfig,
								  CamDeviceHandle_t hCamDevice,
								  uint32_t frames //0-continue
);

#ifdef __cplusplus
}
#endif

#endif // __CAMDEV_API_H__
