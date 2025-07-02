/* SPDX-License-Identifier: MIT */
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

#ifndef _MBOX_API_H_
#define _MBOX_API_H_

#include <mbox_fifo.h>
/*
 *		Each core should be assigned an id number to tell
 *		the mbox who it is and give proper sender and receiver
 *		number of the message to clarify the source and
 *		destination of the incoming or outcoming message
 *		id number is defined based on MBOX_CORE_MAX
 *		count from 0 ~ MBOX_CORE_MAX-1
 */
/*
 *		The whole MBOX FIFO structure needs register size
 *		of mbox_fifo_block_size * MBOX_CORE_MAX *
 *						(MBOX_CORE_MAX -1)
 */

/**
 * @brief Enum structure of Mbox core id
 */
typedef enum mbox_core_id {
	MBOX_CORE_RPU0 = 0,
	MBOX_CORE_RPU1,
	MBOX_CORE_RPU2,
	MBOX_CORE_RPU3,
	MBOX_CORE_APU,
	MBOX_CORE_MAX
} mbox_core_id;

/**
 * @brief Structure of Mbox Control
 */
typedef struct mbox_fifo_ctrl {
	mbox_core_id core_id;
	mbox_core_id sender_id;
	mbox_core_id receiver_id;
	uint64_t buffer_address_phy;
	uint64_t buffer_address_virt;
	fifo_control *fifo;
} __attribute((aligned(8))) mbox_fifo_ctrl;

/**
 * @brief Structure of Mbox ack message
 */
typedef struct mbox_ack_msg {
	uint16_t meg_id;
	uint16_t result;
} __attribute((__packed__)) mbox_ack_msg;

/**
 * @brief write data callback
 * @param receiver_id id of receiver core
 */
typedef void (*mbox_driver_cb)(mbox_core_id receiver_id /*,int fd*/);

/**
 * @brief Initialize a Mbox and register the FIFO
 * @param core_id id of current core
 * @param shm_addr Customized shared memory physical address
 * @param shm_block_size Customized shared memory block size
 * @return Return result
 * @retval mbox_fifo_ctrl point for succeed, NULL for failure
 */
mbox_fifo_ctrl *visp_mbox_init(mbox_core_id core_id, u64 shm_addr,
			       u64 shm_addr_phy,
			       u64 shm_block_size);

/**
 * @brief Post meg to the designated receiver
 * @param mbox_fifo the mbox_fifo_ctrl pointer
 * @param msg message to post
 * @param receiver_id core_id of the receiver
 * @param mbox_driver_cb callback function of mbox driver
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int vpi_mbox_post(mbox_fifo_ctrl *mbox_fifo, mbox_post_msg *msg,
		  mbox_core_id receiver_id /*,int fd*/,
		  mbox_driver_cb mbox_driver_cb);

/**
 * @brief Boardcast meg to all other known cores
 * @param mbox_fifo the mbox_fifo_ctrl pointer
 * @param msg message to post
 * @param mbox_driver_cb callback function of mbox driver
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int vpi_mbox_broadcast(mbox_fifo_ctrl *mbox_fifo,
		       mbox_post_msg *msg /*,int fd*/,
		       mbox_driver_cb mbox_driver_cb);

/**
 * @brief Read meg from the designated sender
 * @param mbox_fifo the mbox_fifo_ctrl pointer
 * @param msg read message
 * @param sender_id core_id of the receiver
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int vpi_mbox_read(mbox_fifo_ctrl *mbox_fifo, mbox_post_msg *msg,
		  mbox_core_id sender_id /*,int fd*/);

/**
 * @brief Reset the designated FIFO with id to spectify
 * @param mbox_fifo the mbox_fifo_ctrl pointer
 * @param sender_id core_id of the sender
 * @param receiver_id core_id of the receiver
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int vpi_mbox_reset(mbox_fifo_ctrl *mbox_fifo, mbox_core_id sender_id,
		   mbox_core_id receiver_id /*,int fd*/);

/**
 * @brief Check designated FIFO stored number
 * @param mbox_fifo the mbox_fifo_ctrl pointer
 * @param sender_id core_id of the sender
 * @param receiver_id core_id of the receiver
 * @return Return result
 * @retval stored numbers, others for failure
 */
uint32_t vpi_mbox_get_stored(mbox_fifo_ctrl *mbox_fifo, mbox_core_id sender_id,
			     mbox_core_id receiver_id /*,int fd*/);

/**
 * @brief Check designated FIFO is full or not
 * @param mbox_fifo the mbox_fifo_ctrl pointer
 * @param sender_id core_id of the sender
 * @param receiver_id core_id of the receiver
 * @return Return result
 * @retval true for full, false for failure
 */
bool vpi_mbox_is_full(mbox_fifo_ctrl *mbox_fifo, mbox_core_id sender_id,
		      mbox_core_id receiver_id /*,int fd*/);

/**
 * @brief Check designated FIFO is empty or not
 * @param mbox_fifo the mbox_fifo_ctrl pointer
 * @param sender_id core_id of the sender
 * @param receiver_id core_id of the receiver
 * @return Return result
 * @retval ture for empty, false for failure
 */
bool vpi_mbox_is_empty(mbox_fifo_ctrl *mbox_fifo, mbox_core_id sender_id,
		       mbox_core_id receiver_id /*,int fd*/);

/**
 * @brief Free everything including FIFO
 * @param mbox_fifo the mbox_fifo_ctrl pointer
 */
// void vpi_mbox_destory(mbox_fifo_ctrl *mbox_fifo);

void vpi_mbox_destory(mbox_fifo_ctrl *mbox_fifo);

int mbox_core_id_check(mbox_fifo_ctrl *mbox_fifo, mbox_core_id sender_id,
		       mbox_core_id receiver_id);
int mbox_mem_map(mbox_fifo_ctrl *mbox_fifo, mbox_core_id core_id,
		 uint64_t shm_start_addr, uint64_t shm_start_addr_phy,
		 uint32_t shm_block_size);
#endif //_MBOX_API_H_
