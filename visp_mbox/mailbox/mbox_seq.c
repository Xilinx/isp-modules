// SPDX-License-Identifier: MIT
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

#include "mbox_seq.h"

#include <linux/kernel.h>

#define MBOX_SEQ_MAX INT_MAX

u32 visp_mbox_seq_increment(u32 seq)
{
	return (seq >= MBOX_SEQ_MAX) ? 0 : (seq + 1);
}

void visp_mbox_commit_outbound_seq(struct rpu_dev *rpu)
{
	rpu->outbound_seq = visp_mbox_seq_increment(rpu->outbound_seq);
}

int visp_mbox_validate_seq(struct rpu_dev *rpu, mbox_post_msg *msg)
{
	unsigned long flags;
	u32 expected_seq;
	u32 received_seq;

	received_seq = ((payload_packet *)msg->payload)->seq_counter;
	if (received_seq > MBOX_SEQ_MAX) {
		dev_err(rpu->dev,
			"Mailbox sequence out of range from RPU%d: seq=%u msg_id=%u\n",
			rpu->rpu_id, received_seq, msg->msg_id);
		return VPI_ERR_SEQ_MISMATCH;
	}

	spin_lock_irqsave(&rpu->seq_lock, flags);
	if (rpu->seq_resync_pending) {
		rpu->seq_resync_pending = false;
		rpu->inbound_seq = visp_mbox_seq_increment(received_seq);
		spin_unlock_irqrestore(&rpu->seq_lock, flags);
		return VPI_SUCCESS;
	}
	expected_seq = rpu->inbound_seq;
	if (received_seq != expected_seq) {
		spin_unlock_irqrestore(&rpu->seq_lock, flags);
		dev_err(rpu->dev,
			"Mailbox sequence mismatch from RPU%d: expected %u got %u for msg_id=%u\n",
			rpu->rpu_id, expected_seq, received_seq, msg->msg_id);
		return VPI_ERR_SEQ_MISMATCH;
	}
	rpu->inbound_seq = visp_mbox_seq_increment(received_seq);
	spin_unlock_irqrestore(&rpu->seq_lock, flags);

	return VPI_SUCCESS;
}

void visp_mbox_mark_seq_resync(struct rpu_dev *rpu)
{
	unsigned long flags;

	spin_lock_irqsave(&rpu->seq_lock, flags);
	rpu->seq_resync_pending = true;
	spin_unlock_irqrestore(&rpu->seq_lock, flags);
}
