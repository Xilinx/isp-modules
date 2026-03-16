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

#ifndef _MBOX_FIFO_H_
#define _MBOX_FIFO_H_
#include <linux/types.h>

/* Maximum payload size in mbox_post_msg structure */
#define MAX_PAYLOAD_SIZE 16424

/**
 * @brief Structure of fifo control
 * NOTE: This structure is shared with RPU firmware - do not modify layout
 * without updating firmware!
 *
 * read_offset and write_offset are SIMPLE INDICES (0, 1, 2, 3...) not byte offsets.
 * These indices are mapped to actual message slot addresses by the RPU firmware
 * using the message slot address list from mbox_phy.c
 *
 * read_offset and write_offset MUST be volatile because:
 * 1. APU reads write_offset written by RPU (RX direction)
 * 2. RPU reads write_offset written by APU (TX direction)
 * 3. Compiler must not cache these in registers across function calls
 */
typedef struct fifo_control {
	uint64_t *buffer_phy;
	void *buffer_virt;
	uint32_t item_size;
	uint32_t item_total;
	uint32_t buffer_size;
	uint32_t item_stored;
	uint32_t read_offset;
	uint32_t write_offset;
} fifo_control;

void visp_mbox_fifo_info(fifo_control *fifo);
/**
 * @brief Structure of init fifo data
 */
typedef struct fifo_init_data {
	uint64_t buffer_addr_phy;
	uint64_t buffer_addr_virt;
	uint32_t item_size;
	uint32_t item_total;
	uint32_t buffer_size;
} fifo_init_data;

/**
 * @brief Structure of Mbox post message
 */
typedef struct mbox_post_msg {
	uint32_t media_server_flags;
	/* to meet the 8 byte alignment requirement */
	uint32_t reserved[3];
	uint32_t msg_id;
	uint32_t size;
	/* Cast to payload_packet* when needed */
	uint8_t payload[MAX_PAYLOAD_SIZE];
} __attribute((aligned(8))) mbox_post_msg;

/**
 * @brief Enum structure of Mailbox Interrupt id
 */
typedef enum mbox_int_id {
	MAILBOX_INTERRUPT_0,
	MAILBOX_INTERRUPT_1,
	MAILBOX_INTERRUPT_2,
	MAILBOX_INTERRUPT_3
} mbox_int_id;

/**
 * @brief Initialize a Fifo with given input
 * @param fifo fifo_control pointer
 * @param fifodata Fifo init data for construct the fifo
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int visp_mbox_fifo_init(fifo_control *fifo, fifo_init_data *fifodata);

/**
 * @brief Write a given meg to the fifo
 * @param msg write message
 * @param fifo fifo_control info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int visp_mbox_fifo_write(mbox_post_msg *msg, fifo_control *fifo /*,int fd*/);

/**
 * @brief Read a given meg from the fifo
 * @param msg read message
 * @param fifo fifo_control info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int visp_mbox_fifo_read(mbox_post_msg *msg, fifo_control *fifo /*,int fd*/);

/**
 * @brief Reset the FIFO
 * @param fifo fifo_control info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int visp_mbox_fifo_reset(fifo_control *fifo /*,int fd*/);

/**
 * @brief Check current FIFO stored number
 * @param fifo fifo_control info pointer
 * @return Return result
 * @retval stored numbers, others for failure
 */
uint32_t visp_mbox_fifo_get_stored(fifo_control *fifo /*,int fd*/);

/**
 * @brief Check current FIFO is full or not
 * @param fifo fifo_control info pointer
 * @return Return result
 * @retval true for full, false for failure
 */
bool visp_mbox_fifo_is_full(fifo_control *fifo /*,int fd*/);

/**
 * @brief Check current FIFO is empty or not
 * @param fifo fifo_control info pointer
 * @return Return result
 * @retval ture for empty, false for failure
 */
bool visp_mbox_fifo_is_empty(fifo_control *fifo /*,int fd*/);

/**
 * @brief Free fifo_control buffer data, only used if the buffer is assigned by
 * @brief malloc() or similar function
 * @param fifo fifo_control info pointer
 */
int visp_mbox_fifo_buffer_free(fifo_control *fifo);

#endif //_MBOX_FIFO_H_
