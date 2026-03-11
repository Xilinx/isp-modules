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

#include "visp_driver.h"
#include "sensor_cmd.h"
#include <linux/kfifo.h>
#include <linux/workqueue.h>
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
	struct kmem_cache *tx_msg_cache;
	struct kmem_cache *rx_msg_cache;
	struct list_head node;
	struct kref refcount;
	struct visp_dev *isp_dev[MAX_NO_ISP];
	struct class *rpu_class[4];
	struct mbox_client tx_mc;
	struct mbox_client rx_mc;
	struct mbox_chan *tx_chan;
	struct mbox_chan *rx_chan;
	struct work_struct mbox_work;
	struct workqueue_struct *rpu_wq;
	struct sk_buff_head tx_mc_skbs;
	struct completion mailbox_completion;
	DECLARE_KFIFO(app_fifo, struct mbox_post_msg *, RPU_CMD_KFIFO_SIZE);
	DECLARE_KFIFO(data_fifo, struct mbox_post_msg *, RPU_CMD_KFIFO_SIZE);
	/* Remoteproc for automatic firmware loading */
	struct rproc *rproc;
	struct platform_device *rproc_pdev;
};

#endif
