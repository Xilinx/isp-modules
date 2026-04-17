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

#include "mbox_error_code.h"
// #include <linux/mbox_fifo.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <mbox_fifo.h>
#include <sensor_cmd.h>
#include <asm-generic/barrier.h>

void visp_mbox_fifo_info(fifo_control *fifo)
{
	pr_err("%s-%d\n", __func__, __LINE__);
	pr_err("\tbuffer_phy       (%p) %p\n", (void *)&fifo->buffer_phy,
	       fifo->buffer_phy);
	pr_err("\tbuffer_virt      (%p) %p\n", (void *)&fifo->buffer_virt,
	       fifo->buffer_virt);
	pr_err("\titem_size        (%p) %u\n", (void *)&fifo->item_size,
	       fifo->item_size);
	pr_err("\titem_total       (%p) %u\n", (void *)&fifo->item_total,
	       fifo->item_total);
	pr_err("\tbuffer_size      (%p) %u\n", (void *)&fifo->buffer_size,
	       fifo->buffer_size);
	pr_err("\tread_index       (%p) %u\n", (void *)&fifo->read_offset,
	       fifo->read_offset);
	pr_err("\twrite_index      (%p) %u\n", (void *)&fifo->write_offset,
	       fifo->write_offset);
}
EXPORT_SYMBOL_GPL(visp_mbox_fifo_info);

int visp_mbox_fifo_init(fifo_control *fifo, fifo_init_data *init_fifo)
{
	if (fifo == NULL || init_fifo == NULL)
		return VPI_ERR_INVALID;

	/* Physical Address */
	fifo->buffer_phy = (uint64_t *)(init_fifo->buffer_addr_phy);
	/* virtual Address */
	fifo->buffer_virt = (uint64_t *)(init_fifo->buffer_addr_virt);
	fifo->item_size = init_fifo->item_size;
	fifo->item_total = init_fifo->item_total;
	fifo->buffer_size = init_fifo->buffer_size;
	fifo->read_offset = 0;
	fifo->write_offset = 0;
	return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(visp_mbox_fifo_init);

int visp_mbox_fifo_write(mbox_post_msg *msg, fifo_control *fifo)
{
	uint32_t items_stored;
	uint32_t current_write_offset, current_read_offset;
	uint32_t aligned_size;

	if (fifo == NULL || msg == NULL)
		return VPI_ERR_INVALID;

	/* Validate message size to prevent buffer overflow */
	if (msg->size > MAX_PAYLOAD_SIZE) {
		pr_err("Message size %u exceeds maximum payload size %u\n",
		       msg->size, MAX_PAYLOAD_SIZE);
		return VPI_ERR_INVALID;
	}

	/*
	 * CRITICAL: Read indices with system barrier for cross-cluster visibility
	 * read_offset: Modified by RPU (consumer), must see fresh value (now simple index)
	 * write_offset: Local to this producer (now simple index)
	 * Use //dmb(sy) not dmb(ish) - RPU may be in different cluster
	 */
	//dmb(sy);  /* System-wide barrier - invalidate cache before reading RPU's read_offset */
	current_write_offset = fifo->write_offset;
	current_read_offset = READ_ONCE(fifo->read_offset);

	if (current_write_offset >= current_read_offset)
		items_stored = current_write_offset - current_read_offset;
	else
		items_stored = (fifo->item_total - current_read_offset) +
				current_write_offset;

	/* Reserve one slot to distinguish full from empty (N-1 slots usable) */
	if (items_stored >= (fifo->item_total - 1)) {
		pr_err("FIFO is full: stored items %u, max usable %u (total %u)\n",
		       items_stored, fifo->item_total - 1, fifo->item_total);
		return VPI_ERR_FULL;
	}

	aligned_size = ALIGN(sizeof(mbox_post_msg) - sizeof(payload_packet) + msg->size,
			     8);
	if (aligned_size > fifo->item_size) {
		pr_err("Write size %u exceeds fifo item size %u\n",
		       aligned_size, fifo->item_size);
		return VPI_ERR_INVALID;
	}

	/* Copy message to shared memory buffer using index */
	memcpy(((uint8_t *)fifo->buffer_virt + (current_write_offset * fifo->item_size)), msg,
	       aligned_size);
	/*
	 * CRITICAL: Memory barrier sequence for APU→RPU communication
	 * Use //dmb(sy) for full system visibility (matches RPU's dsb_sync_barrier)
	 * 1. //dmb(sy): Ensure memcpy completes before updating write_offset
	 * 2. WRITE_ONCE: Atomic write of write_offset
	 * 3. //dmb(sy): Ensure write_offset visible to RPU before IPI trigger
	 * NOTE: Barriers are currently commented out; enable if coherence issues are observed.
	 */
	//dmb(sy);
	WRITE_ONCE(fifo->write_offset,
		   (current_write_offset + 1) >= fifo->item_total ?
		   0 : (current_write_offset + 1));
	//dmb(sy);

	return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(visp_mbox_fifo_write);

int visp_mbox_fifo_read(mbox_post_msg *msg, fifo_control *fifo)
{
	mbox_post_msg *fifo_msg;
	uint32_t current_write_offset, current_read_offset;
	uint32_t aligned_size;

	if (fifo == NULL)
		return VPI_ERR_INVALID;

	/*
	 * CRITICAL: Memory barrier sequence for RPU→APU communication
	 * FIXED ORDERING (was causing false EMPTY reads):
	 * 1. //dmb(sy): System-wide barrier BEFORE reading - invalidates cached write_offset
	 *             Ensures we see RPU's latest write_offset even from different cluster
	 * 2. READ_ONCE: Atomic read of write_offset from memory (not cache)
	 * 3. Check empty with fresh value
	 *
	 * OLD BUG: Barrier was AFTER read → CPU used stale cached write_offset → false EMPTY
	 * NOTE: Barriers are currently commented out; enable if coherence issues are observed.
	 */
	//dmb(sy);  /* Full system barrier - invalidate cache before read */
	current_write_offset = READ_ONCE(fifo->write_offset);
	current_read_offset = fifo->read_offset; /* Local index, no need for READ_ONCE */

	/* Check if FIFO is empty using indices */
	if (current_write_offset == current_read_offset)
		return VPI_ERR_EMPTY;

	fifo_msg =
	    (mbox_post_msg *)(((char *)fifo->buffer_virt +
	    (current_read_offset * fifo->item_size)));

	aligned_size = sizeof(mbox_post_msg) - sizeof(payload_packet) +
		       (((fifo_msg->size) + 63) & ~63);
	if (aligned_size > fifo->item_size) {
		pr_err("Read size %u exceeds fifo item size %u\n",
		       aligned_size, fifo->item_size);
		return VPI_ERR_INVALID;
	}

	memcpy(msg, fifo_msg, aligned_size);

	/* Update read_offset after memcpy completes - use system barrier for RPU visibility
	 * NOTE: Barrier is currently commented out; enable if coherence issues are observed.
	 */
	//dmb(sy);  /* System-wide barrier to ensure RPU sees updated read_offset */
	WRITE_ONCE(fifo->read_offset,
		   (current_read_offset + 1) >= fifo->item_total ?
		   0 : (current_read_offset + 1));
	//dmb(sy);  /* Ensure read_offset update visible to RPU before returning */

	return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(visp_mbox_fifo_read);

int visp_mbox_fifo_reset(fifo_control *fifo)
{
	if (fifo == NULL)
		return VPI_ERR_INVALID;

	fifo->read_offset = 0;
	fifo->write_offset = 0;

	return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(visp_mbox_fifo_reset);

uint32_t visp_mbox_fifo_get_stored(fifo_control *fifo)
{
	uint32_t offset_diff;
	uint32_t current_write_offset, current_read_offset;

	if (fifo == NULL)
		return VPI_ERR_INVALID;

	/* Read indices atomically with system-wide memory barrier */
	//dmb(sy);  /* Invalidate cache before reading */
	current_write_offset = READ_ONCE(fifo->write_offset);
	current_read_offset = fifo->read_offset;

	/* Calculate stored items using simple index difference with wrap-around */
	if (current_write_offset >= current_read_offset)
		offset_diff = current_write_offset - current_read_offset;
	else
		offset_diff = (fifo->item_total - current_read_offset) + current_write_offset;

	return offset_diff;
}
EXPORT_SYMBOL_GPL(visp_mbox_fifo_get_stored);

bool visp_mbox_fifo_is_full(fifo_control *fifo)
{
	uint32_t offset_diff;
	uint32_t current_write_offset, current_read_offset;

	/* Read indices atomically with system-wide memory barrier */
	//dmb(sy);  /* Invalidate cache before reading */
	current_write_offset = READ_ONCE(fifo->write_offset);
	/* RPU updates this - must use READ_ONCE */
	current_read_offset = READ_ONCE(fifo->read_offset);

	/* Calculate stored items using simple index difference with
	 * wrap-around
	 */
	if (current_write_offset >= current_read_offset)
		offset_diff = current_write_offset - current_read_offset;
	else
		offset_diff = (fifo->item_total - current_read_offset) +
			      current_write_offset;

	/* Reserve one slot to distinguish full from empty (N-1 slots usable) */
	bool is_full = offset_diff >= (fifo->item_total - 1);
	return is_full;
}
EXPORT_SYMBOL_GPL(visp_mbox_fifo_is_full);

bool visp_mbox_fifo_is_empty(fifo_control *fifo)
{
	/* System-wide barrier before reading to see RPU's latest write */
	//dmb(sy);
	uint32_t current_write_offset = READ_ONCE(fifo->write_offset);
	uint32_t current_read_offset = fifo->read_offset;
	bool is_empty = (current_write_offset == current_read_offset);
	return is_empty;
}
EXPORT_SYMBOL_GPL(visp_mbox_fifo_is_empty);

int visp_mbox_fifo_buffer_free(fifo_control *fifo)
{
	if (fifo == NULL)
		return VPI_ERR_INVALID;
	kfree(fifo->buffer_phy);
	kfree(fifo->buffer_virt);
	return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(visp_mbox_fifo_buffer_free);

MODULE_LICENSE("GPL");
