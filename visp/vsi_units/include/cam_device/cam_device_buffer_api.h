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

#ifndef __CAMDEV_BUFFER_API_H__
#define __CAMDEV_BUFFER_API_H__

//#include "media_buffer_pool.h"

#include <common/buf_defs.h>
#include <common/return_codes.h>
#include <ebase/types.h>

#include "cam_device_api.h"
#include "visp_app.h"

/**
 * @defgroup cam_device_buffer Cam Device Buffer Definitions
 * @{
 *
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMDEV_BUFFER_RDMA_WAIT_TIME_OUT 4000

	/******************************************************************************/
	/**
	 * @brief   The buffer chain index enumeration of each output path.
	 *
	 *****************************************************************************/
	typedef enum CamDeviceBufChainId_e
	{
		CAMDEV_BUFCHAIN_MP = 0,	 /**< ISP output main path buffer chain index*/
		CAMDEV_BUFCHAIN_SP1 = 1, /**< ISP output self1 path buffer chain index*/
		CAMDEV_BUFCHAIN_SP2 = 2, /**< ISP output self2 path buffer chain index*/
		CAMDEV_BUFCHAIN_RAW = 3, /**< ISP output RAW path buffer chain index*/
		CAMDEV_BUFCHAIN_HDR_RAW =
			4, /**< ISP output Retiming HDR RAW path buffer chain index*/
		CAMDEV_BUFCHAIN_METADATA = 5, /**< Vi200 metadata buffer chain index*/
		CAMDEV_BUFCHAIN_RDMA =
			6, /**< ISP input read DMA path buffer chain index*/
		CAMDEV_BUFCHAIN_RETIMING =
			7, /**< ISP input retiming DMA path buffer chain index*/
		CAMDEV_BUFCHAIN_MAX,
		DUMMY_CAMDEV_BUFCHAIN =
			0xDEADFEED /**< Maximum path buffer chain index */
	} CamDeviceBufChainId_t;

	/******************************************************************************/
	/**
	 * @brief   The buffer mode enumeration which can be configured by user application.
	 *
	 *****************************************************************************/
	typedef enum CamDeviceBufMode_e
	{
		CAMDEV_BUFMODE_INVALID = 0, /**< Invalid buffer mode */
		CAMDEV_BUFMODE_USERPTR, /**< The buffer alloc from user application*/
		CAMDEV_BUFMODE_RESMEM, /**< The buffer alloc from internal reserved memory */
		//CAMDEV_BUFMODE_FLEXA,                  /**< The FLEXA share memory mode*/
		CAMDEV_BUFMODE_MAX, /**< Maximum buffer mode */
		DUMMY_CAMDEV_BUFMODE = 0xDEADFEED
	} CamDeviceBufMode_t;

	/******************************************************************************/
	/**
	 * @brief   The buffer queue blocking type enumeration when user read/write buffer queue.
	 *
	 *****************************************************************************/
	typedef enum CamDeviceBufQueBlockType_e
	{
		CAMDEV_BUFQUE_NONBLOCK_TYPE = 0, /**< Non-blocking type */
		CAMDEV_BUFQUE_TIMEOUT_TYPE,		 /**< Time-blocking type*/
		CAMDEV_BUFQUE_BLOCK_TYPE,		 /**< Blocking type */
		CAMDEV_BUFQUE_BLOCK_TYPE_MAX,	 /**< Blocking type max */
		DUMMY_CAMDEV_BUFQUE_BLOCK_TYPE = 0xDEADFEED
	} CamDeviceBufQueBlockType_t;

	/******************************************************************************/
	/**
	 * @brief   The buffer pool configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceBufPoolConfig_s
	{
		CamDeviceBufMode_t bufMode; /**< Buffer memory mode */
		uint32_t bufNum;			/**< The number of buffer pool */
		uint32_t bufSize;			/**< The buffer size*/
		uint32_t *
			pBaseAddrList; /**< The physical base address list. If the bufMode is set to USERPTR,
						 this member must be set. The base address should be aligned with 1024. */
		bool_t
			is_mapped; /*new member: whether to map ISP platform virtual address.*/
		void **pIplAddrList;
	} CamDeviceBufPoolConfig_t;

	/******************************************************************************/
	/**
	 * @brief   The buffer queue operation configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceBufQueOpConfig_s
	{
		CamDeviceBufQueBlockType_t
			blockType; /**< The buffer queue block type */
		uint32_t
			waitTime; /**< When the blockType is time-blocking, should set the wait time. unit:msec*/
	} CamDeviceBufQueOpConfig_t;

	/******************************************************************************/
	/**
	 * @brief   The buffer chain configuration.
	 *
	 *****************************************************************************/
	typedef struct CamDeviceBufChainConfig_s
	{
		uint32_t
			skipInterval; /**< Interval of skip frame, only enable for ISP output path */
		uint32_t bufQueLength; /**< The empty and full buffer queue Length*/
		CamDeviceBufQueOpConfig_t emptyQueOp;
		CamDeviceBufQueOpConfig_t fullQueOp;
	} CamDeviceBufChainConfig_t;

	/*****************************************************************************/
	/**
	 * @brief   This function initializes buffer chain.
	 *
	 * @param   hCamDevice                 Handle to the CamDevice instance
	 * @param   bufId                      Buffer chain index
	 * @param   pConfig                    Buffer chain configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceInitBufChain(struct visp_dev *isp_dev,
									CamDeviceHandle_t hCamDevice,
									CamDeviceBufChainId_t bufId,
									CamDeviceBufChainConfig_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function deinitializes buffer chain.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance
	 * @param   bufId                   Buffer chain index
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDeInitBufChain(struct visp_dev *isp_dev,
									  CamDeviceHandle_t hCamDevice,
									  CamDeviceBufChainId_t bufId);

	/*****************************************************************************/
	/**
	 * @brief   This function creates buffer pool.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   bufId               Buffer chain index
	 * @param   pConfig             The pointer of buffer pool configuration
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceCreateBufPool(struct visp_dev *isp_dev,
									 CamDeviceHandle_t hCamDevice,
									 CamDeviceBufChainId_t bufId,
									 CamDeviceBufPoolConfig_t *pConfig);

	/*****************************************************************************/
	/**
	 * @brief   This function destroys buffer pool
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   bufId               Buffer chain index
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDestroyBufPool(struct visp_dev *isp_dev,
									  CamDeviceHandle_t hCamDevice,
									  CamDeviceBufChainId_t bufId);

	/*****************************************************************************/
	/**
	 * @brief  This function sets up buffer management.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   bufId               Buffer channel index
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceSetupBufMgmt(struct visp_dev *isp_dev,
									CamDeviceHandle_t hCamDevice,
									CamDeviceBufChainId_t bufId);

	/*****************************************************************************/
	/**
	 * @brief   This function releases buffer management.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   bufId               Buffer channel index
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceReleaseBufMgmt(struct visp_dev *isp_dev,
									  CamDeviceHandle_t hCamDevice,
									  CamDeviceBufChainId_t bufId);

	/*****************************************************************************/
	/**
	 * @brief   This function dequeues a buffer from full buffer queue.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   bufId               Buffer chain index
	 * @param   pMediaBuf           The pointer of a media buffer
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceDeQueBuffer(struct visp_dev *isp_dev,
								   CamDeviceHandle_t hCamDevice,
								   CamDeviceBufChainId_t bufId,
								   MediaBuffer_t **pMediaBuf);

	/*****************************************************************************/
	/**
	 * @brief   This function returns a buffer to empty buffer queue.
	 *
	 * @param   hCamDevice          Handle to the CamDevice instance
	 * @param   bufId               Buffer index
	 * @param   pMediaBuf           The pointer of a media buffer
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceEnQueBuffer(struct visp_dev *isp_dev,
								   CamDeviceHandle_t hCamDevice,
								   CamDeviceBufChainId_t bufId,
								   OutputBuffer_t *pMediaBuf);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the size of buffer.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   bufId                   Buffer chain index
	 * @param   pBufSize                Buffer size pointer
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         Operation failed
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGetBufferSize(struct visp_dev *isp_dev,
									 CamDeviceHandle_t hCamDevice,
									 CamDeviceBufChainId_t bufId,
									 uint32_t *pBufSize);

	/*****************************************************************************/
	/**
	 * @brief   This function gets the buffer handle.
	 *
	 * @param   hCamDevice              Handle to the CamDevice instance.
	 * @param   bufId                   Buffer chain index
	 * @param   bufHandle               Buffer handle pointer
	 *
	 * @retval  RET_SUCCESS         Operation succeeded
	 * @retval  RET_FAILURE         Operation failed
	 *
	 *****************************************************************************/
	RESULT VsiCamDeviceGetBufMgmt(CamDeviceHandle_t hCamDevice,
								  CamDeviceBufChainId_t bufId,
								  BufMgmtHandle_t *bufHandle);

	/* @} cam_device_buffer */

#ifdef __cplusplus
}
#endif

#endif // __CAMDEV_BUFFER_API_H__
