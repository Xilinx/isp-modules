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

/**
 * @brief Structure of fifo control
 */
typedef struct Fifo_Control {
  uint64_t *buffer_phy;
  uint64_t *buffer_virt;
  uint32_t item_size;
  uint32_t item_total;
  uint32_t buffer_size;
  uint32_t item_stored;
  uint32_t read_offset;
  uint32_t write_offset;
} FifoControl;

void fifo_info(FifoControl *);
/**
 * @brief Structure of init fifo data
 */
typedef struct FifoInitData {
  uint64_t buffer_addr_phy;
  uint64_t buffer_addr_virt;
  uint32_t item_size;
  uint32_t item_total;
  uint32_t buffer_size;
} FifoInitData;

/**
 * @brief Structure of Mbox post message
 */
typedef struct MboxPostMsg {
  uint8_t group_id;
  uint8_t ack : 1;
  uint8_t flags : 7;
  uint16_t msg_id;
  uint32_t size;
  uint8_t payload[16420];
} __attribute((aligned(8))) MboxPostMsg;

/**
 * @brief Enum structure of Mailbox Interrupt id
 */
typedef enum MboxIntId {
  MAILBOX_INTERRUPT_0,
  MAILBOX_INTERRUPT_1,
  MAILBOX_INTERRUPT_2,
  MAILBOX_INTERRUPT_3
} MboxIntId;

/**
 * @brief Initialize a Fifo with given input
 * @param fifo FifoControl pointer
 * @param fifodata Fifo init data for construct the fifo
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int fifo_init(FifoControl *fifo, FifoInitData *fifodata /*,int fd*/);

/**
 * @brief Write a given meg to the fifo
 * @param msg write message
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int fifo_write(MboxPostMsg *msg, FifoControl *fifo /*,int fd*/);

/**
 * @brief Read a given meg from the fifo
 * @param msg read message
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int fifo_read(MboxPostMsg *msg, FifoControl *fifo /*,int fd*/);

/**
 * @brief Reset the FIFO
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int fifo_reset(FifoControl *fifo /*,int fd*/);

/**
 * @brief Check current FIFO stored number
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval stored numbers, others for failure
 */
uint32_t fifo_get_stored(FifoControl *fifo /*,int fd*/);

/**
 * @brief Check current FIFO is full or not
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval true for full, false for failure
 */
bool fifo_is_full(FifoControl *fifo /*,int fd*/);

/**
 * @brief Check current FIFO is empty or not
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval ture for empty, false for failure
 */
bool fifo_is_empty(FifoControl *fifo /*,int fd*/);

/**
 * @brief Free FifoControl buffer data, only used if the buffer is assigned by
 * @brief malloc() or similar function
 * @param fifo FifoControl info pointer
 */
int fifo_buffer_free(FifoControl *fifo);

#endif //_MBOX_FIFO_H_
