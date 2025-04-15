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

#ifndef _MBOX_HARDWARE_H_
#define _MBOX_HARDWARE_H_

//#include <linux/mbox_fifo.h>
#include <mbox_fifo.h>

typedef enum MboxIOCFlag
{
	MAILBOXIOC_WRITE_REG = 0x100,
	MAILBOXIOC_READ_REG,
	MAILBOXIOC_TRI_INT,
	MAILBOXIOC_WRITE_MSG_REG,
	MAILBOXIOC_READ_MSG_REG
} MboxIOCFlag;

typedef enum MboxFifoFlag
{
	MAILBOXFIFO_BUFFER,
	MAILBOXFIFO_ITEM_SIZE,
	MAILBOXFIFO_ITEM_TOTAL,
	MAILBOXFIFO_BUFFER_SIZE,
	MAILBOXFIFO_ITEM_STORED,
	MAILBOXFIFO_READ_OFFSET,
	MAILBOXFIFO_WRITE_OFFSET
} MboxFifoFlag;

typedef enum MboxMsgFlag
{
	MAILBOXMSG_GROUP_ID = 0x10,
	MAILBOXMSG_MSG_ID,
	MAILBOXMSG_ACK,
	MAILBOXMSG_FLAGS,
	MAILBOXMSG_SIZE,
	MAILBOXMSG_PAYLOAD //MAILBOXMSG_PAYLOAD must be the last one
} MboxMsgFlag;

typedef struct mailbox_reg_s
{
	unsigned long val;
	FifoControl *fifo;
	MboxFifoFlag fifoflag;
} mailbox_reg_t;

typedef struct mailbox_reg_msg
{
	unsigned long val;
	FifoControl *fifo;
	MboxMsgFlag msgflag;
} mailbox_reg_msg_t;

int mailbox_write_msg(MboxPostMsg *msg, FifoControl *fifo, int fd);
int mailbox_resd_msg(MboxPostMsg *msg, FifoControl *fifo, int fd);

#endif
