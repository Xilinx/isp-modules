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

/**
 * @cond HIST64
 *
 * @defgroup cam_device_hist64 Cam Device HIST64 Definitions
 * @{
 *
 */

#ifndef __CAMDEV_HIST64_API_H__
#define __CAMDEV_HIST64_API_H__

#include "cam_device_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_HIST64_GRID_ITEMS 25 /**< The number of grid sub windows */
#define CAMDEV_HIST64_NUM_BINS 32	/**< number of bins */

	typedef uint32_t CamDeviceHist64Bins_t[CAMDEV_HIST64_NUM_BINS];

	/******************************************************************************/
	/**
	 * @brief   Cam Device HIST mode.
	 *
	 *****************************************************************************/
	typedef enum CamDeviceHist64Mode_e
	{
		CAMDEV_HIST64_MODE_ONE_FROM_YRGB =
			0, /**< Y/R/G/B histogram controlled via coefficients coeff_r/g/b */
		CAMDEV_HIST64_MODE_R = 1,  /**< R histogram */
		CAMDEV_HIST64_MODE_GR = 2, /**< Gr histogram */
		CAMDEV_HIST64_MODE_B = 3,  /**< B histogram */
		CAMDEV_HIST64_MODE_GB = 4, /**< GB histogram */
		CAMDEV_HIST64_MODE_MAX, /**< Upper border (only for an internal evaluation) */
		DUMMY_0057 = 0xDEADFEED
	} CamDeviceHist64Mode_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device HIST channel.
	 *
	 *****************************************************************************/
	typedef enum CamDeviceHist64Channel_e
	{
		CAMDEV_HIST64_CHANNEL_0 = 0, /**< Disable channel, no measurements  */
		CAMDEV_HIST64_CHANNEL_1 = 1, /**< HIST channel 1 */
		CAMDEV_HIST64_CHANNEL_2 = 2, /**< HIST channel 2 */
		CAMDEV_HIST64_CHANNEL_3 = 3, /**< HIST channel 3 */
		CAMDEV_HIST64_CHANNEL_4 = 4, /**< HIST channel 4 */
		CAMDEV_HIST64_CHANNEL_5 = 5, /**< HIST channel 5 */
		CAMDEV_HIST64_CHANNEL_6 = 6, /**< HIST channel 6 */
		CAMDEV_HIST64_CHANNEL_7 = 7, /**< HIST channel 7 */
		CAMDEV_HIST64_CHANNEL_MAX, /**< upper border (only for an internal evaluation) */
		DUMMY_0058 = 0xDEADFEED
	} CamDeviceHist64Channel_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device HIST64 sub-sample range configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceHist64SubSampleRange_s
	{
		uint8_t shift;	 /**< HIST shift */
		uint16_t offset; /**< HIST offset */
	} CamDeviceHist64SubSampleRange_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device HIST64 coefficient configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceHist64Coeff_s
	{
		float32_t rCoeff; /**< Red channel coefficient */
		float32_t gCoeff; /**< Green channel coefficient */
		float32_t bCoeff; /**< Blue channel coefficient */
	} CamDeviceHist64Coeff_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device HIST configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceHist64Config_s
	{
		CamDeviceHist64Mode_t mode;					  /**< HIST mode */
		uint8_t gridWeight[CAMDEV_HIST64_GRID_ITEMS]; /**< HIST weight */
		CamDeviceHist64SubSampleRange_t sampleRange;  /**< Sample range */
		CamDeviceHist64Coeff_t
			coeff; /**< HIST coefficient of red/green/blue channel */
		CamDeviceHist64Channel_t channel; /**< HIST channel */
	} CamDeviceHist64Config_t;

	/******************************************************************************/
	/**
	 * @brief   Cam Device HIST status structure.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceHist64Status_s
	{
		bool_t enable; /**< HIST enable status*/
	} CamDeviceHist64Status_t;

	/*****************************************************************************/
	/**
	 * @brief   This function sets HIST configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pConfig             Pointer to HIST configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64SetConfig(CamDeviceHandle_t hCamDevice,
									   CamDeviceHist64Config_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function gets HIST configuration parameters.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pConfig             Pointer to HIST configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64GetConfig(CamDeviceHandle_t hCamDevice,
									   CamDeviceHist64Config_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function enables HIST.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64Enable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function disables HIST.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64Disable(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function sets HIST64 measure window.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pWindow             Pointer to HIST64 measure window
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64SetMeasureWindow(CamDeviceHandle_t hCamDevice,
											  CamDeviceWindow_t *pWindow);

	/*****************************************************************************/
	/**
	 * @brief   This function gets HIST64 measure window.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pWindow             Pointer to HIST64 measure window
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64GetMeasureWindow(CamDeviceHandle_t hCamDevice,
											  CamDeviceWindow_t *pWindow);

	/*****************************************************************************/
	/**
	 * @brief   This function gets HIST64 bins.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pHistBin            Pointer to HIST64 bins
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64GetBins(CamDeviceHandle_t hCamDevice,
									 CamDeviceHist64Bins_t *pHistBin);

	/*****************************************************************************/
	/**
	 * @brief   This function forces update HIST64.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64ForceUpdate(CamDeviceHandle_t hCamDevice);

	/*****************************************************************************/
	/**
	 * @brief   This function gets HIST status.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pStatus             Pointer to HIST status
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64GetStatus(CamDeviceHandle_t hCamDevice,
									   CamDeviceHist64Status_t *pStatus);

	/*****************************************************************************/
	/**
	 * @brief   This function gets HIST version.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   pVersion            Pointer to HIST version
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64GetVersion(CamDeviceHandle_t hCamDevice,
										uint32_t *pVersion);

	/*****************************************************************************/
	/**
	 * @brief   This function resets HIST64.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceHist64Reset(CamDeviceHandle_t hCamDevice);

	/* @} cam_device_hist64 */
	/* @endcond */

#ifdef __cplusplus
}
#endif

#endif /* __CAMDEV_HIST64_API_H__ */
