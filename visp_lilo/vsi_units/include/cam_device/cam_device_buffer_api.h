/* SPDX-License-Identifier: MIT*/
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

#ifndef __CAMDEV_BUFFER_API_H__
#define __CAMDEV_BUFFER_API_H__

// #include "media_buffer_pool.h"

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
extern "C" {
#endif

#define CAMDEV_BUFFER_RDMA_WAIT_TIME_OUT 4000

/******************************************************************************/
/**
 * @brief   The buffer chain index enumeration of each output path.
 *
 *****************************************************************************/
typedef enum cam_device_buf_chain_id_e {
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
} cam_device_buf_chain_id_t;

/******************************************************************************/
/**
 * @brief   The buffer mode enumeration which can be configured by user
 *application.
 *
 *****************************************************************************/
typedef enum cam_device_buf_mode_e {
	CAMDEV_BUFMODE_INVALID = 0, /**< Invalid buffer mode */
	CAMDEV_BUFMODE_USERPTR, /**< The buffer alloc from user application*/
	CAMDEV_BUFMODE_RESMEM,	/**< The buffer alloc from internal reserved
				   memory */
	// CAMDEV_BUFMODE_FLEXA,                  /**< The FLEXA share memory
	// mode*/
	CAMDEV_BUFMODE_MAX, /**< Maximum buffer mode */
} cam_device_buf_mode_t;

/******************************************************************************/
/**
 * @brief   The buffer queue blocking type enumeration when user read/write
 *buffer queue.
 *
 *****************************************************************************/
typedef enum cam_device_buf_que_block_type_e {
	CAMDEV_BUFQUE_NONBLOCK_TYPE = 0, /**< Non-blocking type */
	CAMDEV_BUFQUE_TIMEOUT_TYPE,	 /**< Time-blocking type*/
	CAMDEV_BUFQUE_BLOCK_TYPE,	 /**< Blocking type */
	CAMDEV_BUFQUE_BLOCK_TYPE_MAX,	 /**< Blocking type max */
} cam_device_buf_que_block_type_t;

/******************************************************************************/
/**
 * @brief   The buffer pool configuration.
 *
 *****************************************************************************/
typedef struct cam_device_buf_pool_config_s {
	cam_device_buf_mode_t buf_mode; /**< Buffer memory mode */
	uint32_t buf_num;		/**< The number of buffer pool */
	uint32_t buf_size;		/**< The buffer size*/
	uint32_t *p_base_addr_list; /**< The physical base address list. If the
				       buf_mode is set to USERPTR, this member
				       must be set. The base address should be
				       aligned with 1024. */
	bool_t is_mapped; /*new member: whether to map ISP platform virtual
			     address.*/
	void **p_ipl_addr_list;
} cam_device_buf_pool_config_t;

/******************************************************************************/
/**
 * @brief   The buffer queue operation configuration.
 *
 *****************************************************************************/
typedef struct cam_device_buf_que_op_config_s {
	cam_device_buf_que_block_type_t
	    block_type;	    /**< The buffer queue block type */
	uint32_t wait_time; /**< When the block_type is time-blocking, should
			       set the wait time. unit:msec*/
} cam_device_buf_que_op_config_t;

/******************************************************************************/
/**
 * @brief   The buffer chain configuration.
 *
 *****************************************************************************/
typedef struct cam_device_buf_chain_config_s {
	uint32_t skip_interval; /**< Interval of skip frame, only enable for ISP
				   output path */
	uint32_t buf_que_length; /**< The empty and full buffer queue Length*/
	cam_device_buf_que_op_config_t empty_que_op;
	cam_device_buf_que_op_config_t full_que_op;
} cam_device_buf_chain_config_t;

/*****************************************************************************/
/**
 * @brief   This function initializes buffer chain.
 *
 * @param   h_cam_device                 Handle to the CamDevice instance
 * @param   buf_id                      Buffer chain index
 * @param   p_config                    Buffer chain configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_init_buf_chain(struct visp_dev *isp_dev,
				     cam_device_handle_t h_cam_device,
				     cam_device_buf_chain_id_t buf_id,
				     cam_device_buf_chain_config_t *p_config);

/*****************************************************************************/
/**
 * @brief   This function deinitializes buffer chain.
 *
 * @param   h_cam_device              Handle to the CamDevice instance
 * @param   buf_id                   Buffer chain index
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_de_init_buf_chain(struct visp_dev *isp_dev,
					cam_device_handle_t h_cam_device,
					cam_device_buf_chain_id_t buf_id);

/*****************************************************************************/
/**
 * @brief   This function creates buffer pool.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   buf_id               Buffer chain index
 * @param   p_config             The pointer of buffer pool configuration
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_create_buf_pool(struct visp_dev *isp_dev,
				      cam_device_handle_t h_cam_device,
				      cam_device_buf_chain_id_t buf_id,
				      cam_device_buf_pool_config_t *p_config);

/*****************************************************************************/
/**
 * @brief   This function destroys buffer pool
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   buf_id               Buffer chain index
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_destroy_buf_pool(struct visp_dev *isp_dev,
				       cam_device_handle_t h_cam_device,
				       cam_device_buf_chain_id_t buf_id);

/*****************************************************************************/
/**
 * @brief  This function sets up buffer management.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   buf_id               Buffer channel index
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_setup_buf_mgmt(struct visp_dev *isp_dev,
				     cam_device_handle_t h_cam_device,
				     cam_device_buf_chain_id_t buf_id);

/*****************************************************************************/
/**
 * @brief   This function releases buffer management.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   buf_id               Buffer channel index
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_release_buf_mgmt(struct visp_dev *isp_dev,
				       cam_device_handle_t h_cam_device,
				       cam_device_buf_chain_id_t buf_id);

/*****************************************************************************/
/**
 * @brief   This function dequeues a buffer from full buffer queue.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   buf_id               Buffer chain index
 * @param   p_media_buf           The pointer of a media buffer
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_de_que_buffer(struct visp_dev *isp_dev,
				    cam_device_handle_t h_cam_device,
				    cam_device_buf_chain_id_t buf_id,
				    media_buffer_t **p_media_buf);

/*****************************************************************************/
/**
 * @brief   This function returns a buffer to empty buffer queue.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   buf_id               Buffer index
 * @param   p_media_buf           The pointer of a media buffer
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_en_que_buffer(struct visp_dev *isp_dev,
				    cam_device_handle_t h_cam_device,
				    cam_device_buf_chain_id_t buf_id,
				    output_buffer_t *p_media_buf);

/*****************************************************************************/
/**
 * @brief   This function gets the size of buffer.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   buf_id                   Buffer chain index
 * @param   p_buf_size                Buffer size pointer
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT vsi_cam_device_get_buffer_size(struct visp_dev *isp_dev,
				      cam_device_handle_t h_cam_device,
				      cam_device_buf_chain_id_t buf_id,
				      uint32_t *p_buf_size);

/*****************************************************************************/
/**
 * @brief   This function gets the buffer handle.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   buf_id                   Buffer chain index
 * @param   buf_handle               Buffer handle pointer
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         Operation failed
 *
 *****************************************************************************/
RESULT vsi_cam_device_get_buf_mgmt(cam_device_handle_t h_cam_device,
				   cam_device_buf_chain_id_t buf_id,
				   buf_mgmt_handle_t *buf_handle);

/* @} cam_device_buffer */

#ifdef __cplusplus
}
#endif

#endif // __CAMDEV_BUFFER_API_H__
