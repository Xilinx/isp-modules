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

#ifndef __VISP_MBOX_DRIVER_H__
#define __VISP_MBOX_DRIVER_H__

#include <linux/completion.h>
#include <linux/kfifo.h>
#include <linux/workqueue.h>
#include "mailbox/mbox_fifo.h"
#include "sensor_cmd.h"
#include "visp_driver.h"

#define CHAR_DEV_NAME "mailbox_dev"
#define SUCCESS 0

#define VISP_MBOX_RPU6 6
#define VISP_MBOX_RPU7 7
#define VISP_MBOX_RPU8 8
#define VISP_MBOX_RPU9 9

#define VISP_MBOX_RPU6_0 0
#define VISP_MBOX_RPU7_1 1
#define VISP_MBOX_RPU8_2 2
#define VISP_MBOX_RPU9_3 3

/*
 * ISP instance and path definitions:
 *   MAX_ISP_INSTANCES: 6 ISP devices (0-5)
 *   INSTANCES_PER_ISP: 16 instances per ISP (0-15 for each ISP)
 *   MAX_INSTANCE_ID: 95 (6 ISPs × 16 instances - 1)
 *   Path 0-3: Valid paths for video/sensor pipelines
 *   Path >=4: Invalid, redirected to path 1 in some contexts
 */
#define MAX_ISP_INSTANCES 6
#define INSTANCES_PER_ISP 16
#define MAX_INSTANCE_ID 95
#define MAX_PATH_ID 3
#define VISP_MBOX_MAX_RPU_ID 9

struct rpu_dev *visp_mbox_get_rpu_dev(int rpu_id);
uint8_t xlnx_mbox_apu_wait_for_ack(struct visp_dev *isp_dev, uint32_t instance_id,
				   uint32_t path, uint32_t buffer_index, mb_cmd_id_e cmd);
int xlnx_send_mbox_data_cmd(struct visp_dev *isp_dev, mb_cmd_id_e cmd,
			    void *data, uint32_t size, uint8_t dest_cpu,
			    uint8_t src_cpu);
int xlnx_send_mbox_acked_cmd(struct visp_dev *isp_dev, mb_cmd_id_e cmd,
			     void *data, uint32_t size, uint8_t dest_cpu,
			     uint8_t src_cpu);

int xlnx_send_mbox_without_ack_cmd(struct visp_dev *isp_dev, mb_cmd_id_e cmd,
				   void *data, uint32_t size, uint8_t dest_cpu,
				   uint8_t src_cpu);

void mailbox_init(uint32_t cpu, uint64_t MBOX_FIFO_START_ADDR,
		  uint64_t mbox_fifo_start_addr_phy);
uint8_t xlnx_mbox_apu_wait_for_data(struct visp_dev *isp_dev, void *data);
// extern int handle_frameout_buffer(void *Enque_Buff_L, struct visp_dev
// *isp_dev); int (* exported_func1)(void *,struct visp_dev *isp_dev);
typedef struct payload_user_template {
	payload_type type;
	mb_cmd_id_e cmd_id;
	__u32 cookie;
	__u32 payload_size;
	__u32 reserved[1];
	response_field_t resp_field;
	uint8_t payload[MAX_ITEM];

} payload_user_packet;
struct reserved_memory {
	phys_addr_t phys_addr;
	void __iomem *virt_addr;
};

/*
 * Increased from 8 to 32 to handle burst traffic from 6 ISPs at frame start.
 * Old: 8 slots shared across all ISPs → overflow with 6+ simultaneous sends.
 * New: 32 slots provides 5× headroom for burst absorption.
 */
#define RPU_CMD_KFIFO_SIZE 32

/*
 * Pre-allocated message buffer pool size.
 * 32 buffers per direction (TX/RX) with free-list management.
 */
#define MSG_BUFFER_POOL_SIZE 48

/*
 * Message buffer node - wraps mbox_post_msg with free-list linkage.
 * Embedded in pre-allocated pool, managed via free-list for O(1) alloc/free.
 */
struct msg_buf_node {
	struct list_head list;        /* Free-list linkage */
	mbox_post_msg msg;            /* Actual message buffer */
};

/* Structures to hold the rpu_device specific information */
struct rpu_dev {
	struct device *dev;
	int rpu_id;
	int core_id;
	dev_t devno;
	struct cdev cdev;
	struct mutex read_lock;
	struct mutex rpu_lock;
	struct mutex write_lock;
	struct mutex userapp_lock;
	/* Protects app_fifo from concurrent access */
	struct mutex app_fifo_lock;
	/* Protects data_fifo from concurrent access */
	struct mutex data_fifo_lock;
	mbox_fifo_ctrl *apu_rx_ctrl;
	mbox_fifo_ctrl *apu_tx_ctrl;
	/* Pre-allocated buffer pools with free-list management */
	struct msg_buf_node tx_bufs[MSG_BUFFER_POOL_SIZE];
	struct msg_buf_node rx_bufs[MSG_BUFFER_POOL_SIZE];
	struct list_head tx_free_list;  /* Free TX buffers */
	struct list_head rx_free_list;  /* Free RX buffers */
	spinlock_t tx_free_lock;        /* Protects TX free-list */
	spinlock_t rx_free_lock;        /* Protects RX free-list */
	struct list_head node;
	struct kref refcount;
	struct visp_dev *isp_dev[MAX_NO_ISP];
	struct class *rpu_class[4];
	struct mbox_client tx_mc;
	struct mbox_client rx_mc;
	struct mbox_chan *tx_chan;
	struct mbox_chan *rx_chan;
	bool removing;
	struct work_struct mbox_work;
	struct workqueue_struct *rpu_wq;
	struct sk_buff_head tx_mc_skbs;
	/* Wait queue for poll() and read() blocking */
	wait_queue_head_t read_wait;
	DECLARE_KFIFO(app_fifo, struct mbox_post_msg *, RPU_CMD_KFIFO_SIZE);
	DECLARE_KFIFO(data_fifo, struct mbox_post_msg *, RPU_CMD_KFIFO_SIZE);
	/* Remoteproc for automatic firmware loading */
	struct rproc *rproc;
	struct platform_device *rproc_pdev;
	struct completion init_fw_done;
	int init_fw_status;
};

/*
 * Buffer pool helper functions - inline for performance.
 * Free-list implementation: O(1) alloc/free with spinlock protection.
 * ~40-60ns per operation - much faster than kmem_cache (~300ns).
 */

/*
 * Get a TX buffer from the free-list.
 * Returns pointer to buffer, or NULL if pool exhausted (should never happen).
 * Thread-safe via spinlock.
 */
static inline mbox_post_msg *visp_get_tx_buffer(struct rpu_dev *rpu)
{
	struct msg_buf_node *node;
	unsigned long flags;

	spin_lock_irqsave(&rpu->tx_free_lock, flags);

	if (list_empty(&rpu->tx_free_list)) {
		spin_unlock_irqrestore(&rpu->tx_free_lock, flags);
		pr_err("%s: TX buffer pool exhausted!\n", __func__);
		return NULL;
	}

	node = list_first_entry(&rpu->tx_free_list, struct msg_buf_node, list);
	list_del(&node->list);

	spin_unlock_irqrestore(&rpu->tx_free_lock, flags);

	/* Caller must initialize all fields - no memset for performance (~1.1μs saving) */
	return &node->msg;
}

/*
 * Return a TX buffer to the free-list.
 * Thread-safe via spinlock.
 */
static inline void visp_free_tx_buffer(struct rpu_dev *rpu, mbox_post_msg *msg)
{
	struct msg_buf_node *node;
	unsigned long flags;

	if (!msg)
		return;

	/* Get the containing node structure */
	node = container_of(msg, struct msg_buf_node, msg);

	/* Optional: Zero buffer on free (moves ~1.1μs out of alloc path) */
	/* memset(&node->msg, 0, sizeof(mbox_post_msg)); */

	spin_lock_irqsave(&rpu->tx_free_lock, flags);
	list_add(&node->list, &rpu->tx_free_list);
	spin_unlock_irqrestore(&rpu->tx_free_lock, flags);
}

/*
 * Get an RX buffer from the free-list.
 * Returns pointer to buffer, or NULL if pool exhausted (should never happen).
 * Thread-safe via spinlock.
 */
static inline mbox_post_msg *visp_get_rx_buffer(struct rpu_dev *rpu)
{
	struct msg_buf_node *node;
	unsigned long flags;

	spin_lock_irqsave(&rpu->rx_free_lock, flags);

	if (list_empty(&rpu->rx_free_list)) {
		spin_unlock_irqrestore(&rpu->rx_free_lock, flags);
		pr_err("%s: RX buffer pool exhausted!\n", __func__);
		return NULL;
	}

	node = list_first_entry(&rpu->rx_free_list, struct msg_buf_node, list);
	list_del(&node->list);

	spin_unlock_irqrestore(&rpu->rx_free_lock, flags);

	/* Caller must initialize all fields - no memset for performance (~1.1μs saving) */
	return &node->msg;
}

/*
 * Return an RX buffer to the free-list.
 * Thread-safe via spinlock.
 */
static inline void visp_free_rx_buffer(struct rpu_dev *rpu, mbox_post_msg *msg)
{
	struct msg_buf_node *node;
	unsigned long flags;

	if (!msg)
		return;

	/* Get the containing node structure */
	node = container_of(msg, struct msg_buf_node, msg);

	/* Optional: Zero buffer on free (moves ~1.1μs out of alloc path) */
	/* memset(&node->msg, 0, sizeof(mbox_post_msg)); */

	spin_lock_irqsave(&rpu->rx_free_lock, flags);
	list_add(&node->list, &rpu->rx_free_list);
	spin_unlock_irqrestore(&rpu->rx_free_lock, flags);
}

#endif
