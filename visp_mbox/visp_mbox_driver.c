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

#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/remoteproc.h>
#include "visp_mbox_driver.h"
#include "mbox_cmd.h"
#include "mbox_api.h"
#include <linux/delay.h>

/*
 * Enable sanity checks for stale completion detection.
 * Disable in production to eliminate minimal overhead.
 */
//#define DEBUG_MBOX_SANITY_CHECKS

/*
 * Process up to 32 messages per work execution.
 *
 * FIFO: 30 messages → drained in a single work cycle (<=32 msgs)
 * Per RPU: 360 msgs/sec ≈ 2.78ms per message
 *
 * Strategy: Larger batch clears the FIFO in one dispatch, avoiding
 * head-of-line blocking when RPU6+RPU7 workqueues land on the same CPU,
 * keeping FPS stable with minimal drops.
 */
#define MAX_MESSAGES_PER_WORK 32

struct class *mailbox_class;
static DEFINE_MUTEX(rpu_list_lock);
static LIST_HEAD(rpu_devices);
static atomic_t ipi5_pair_flag = ATOMIC_INIT(0);
static atomic_t ipi6_pair_flag = ATOMIC_INIT(0);

static atomic_t *visp_mbox_get_pair_flag(int rpu_id)
{
	switch (rpu_id) {
	case VISP_MBOX_RPU6:
	case VISP_MBOX_RPU7:
		return &ipi5_pair_flag;
	case VISP_MBOX_RPU8:
	case VISP_MBOX_RPU9:
		return &ipi6_pair_flag;
	default:
		return NULL;
	}
}

static void visp_mbox_mark_init_firmware_success(struct rpu_dev *rpu)
{
	atomic_t *pair_flag;

	if (!rpu || rpu->init_fw_synced)
		return;

	pair_flag = visp_mbox_get_pair_flag(rpu->rpu_id);
	if (pair_flag)
		atomic_inc(pair_flag);

	rpu->init_fw_synced = true;
}

static void visp_mbox_clear_init_firmware_success(struct rpu_dev *rpu)
{
	atomic_t *pair_flag;

	if (!rpu || !rpu->init_fw_synced)
		return;

	pair_flag = visp_mbox_get_pair_flag(rpu->rpu_id);
	if (pair_flag)
		atomic_dec(pair_flag);

	rpu->init_fw_synced = false;
}

static bool visp_mbox_pair_peer_active(struct rpu_dev *rpu)
{
	atomic_t *pair_flag;
	int active_count;

	if (!rpu)
		return false;

	pair_flag = visp_mbox_get_pair_flag(rpu->rpu_id);
	if (!pair_flag)
		return false;

	active_count = atomic_read(pair_flag);
	if (rpu->init_fw_synced)
		active_count--;

	return active_count > 0;
}

/*
 * Per-RPU workqueues are now created in visp_mbox_setup() to eliminate
 * contention when multiple RPUs share the same IPI channel.
 */

static void visp_mbox_event_notified(struct work_struct *work)
{
	struct rpu_dev *rpu = container_of(work, struct rpu_dev, mbox_work);
	int ret;
	int msg_count = 0;

	if (READ_ONCE(rpu->removing))
		return;

	while (!vpi_mbox_is_empty(rpu->apu_rx_ctrl, rpu->core_id, MBOX_CORE_APU) &&
	       msg_count < MAX_MESSAGES_PER_WORK) {
		ret = visp_mbox_apu_read(rpu);
		if (ret != 0)
			dev_err(rpu->dev, "Failed to read mailbox command:%d\n",
				rpu->rpu_id);
		msg_count++;
	}
}

static void visp_mbox_tx_done(struct mbox_client *cl, void *msg, int r)
{
}

static void visp_mbox_rx_cb(struct mbox_client *cl, void *msg)
{
	/* Get RPU id from the received message */
	struct rpu_dev *rpu = dev_get_drvdata(cl->dev);
	if (rpu) {
		/* Skip scheduling when shared IPI fired with empty FIFO */
		if (!vpi_mbox_is_empty(rpu->apu_rx_ctrl, rpu->core_id, MBOX_CORE_APU))
			queue_work(rpu->rpu_wq, &rpu->mbox_work);
		/* Always ACK the IPI so firmware does not stall */
		(void)mbox_send_message(rpu->rx_chan, NULL);
	} else {
		pr_info("%s: Invalid RPU Device structure\n", __func__);
	}
}

static int visp_mbox_setup(struct rpu_dev *rpu, struct device_node *node)
{
	struct mbox_client *mclient;
	char wq_name[32];
	int attempt;

	if (!rpu || !rpu->dev) {
		dev_err(rpu->dev,
			"%s: Invalid RPU device structure.\n", __func__);
		return -EINVAL;
	}

	/* Setup TX mailbox channel client */
	mclient = &rpu->tx_mc;
	mclient->tx_block = false;
	mclient->knows_txdone = false;
	mclient->tx_done = visp_mbox_tx_done; // Set the TX done callback
	mclient->dev = rpu->dev;

	/* Setup RX mailbox channel client */
	mclient = &rpu->rx_mc;
	mclient->dev = rpu->dev;
	mclient->rx_callback = visp_mbox_rx_cb; // Set the RX callback

	/* Initialize work item for RX handling */
	INIT_WORK(&rpu->mbox_work, visp_mbox_event_notified);

	/* Dedicated per-RPU workqueue to avoid cross-RPU contention */
	snprintf(wq_name, sizeof(wq_name), "visp-mbox-rpu%d", rpu->rpu_id);
	/* High-priority unbound worker to reduce preemption under load */
	rpu->rpu_wq = alloc_workqueue(wq_name,
				      WQ_UNBOUND | WQ_HIGHPRI |
				      WQ_CPU_INTENSIVE, 1);
	if (!rpu->rpu_wq) {
		dev_err(rpu->dev, "Failed to create per-RPU workqueue\n");
		return -ENOMEM;
	}

	dev_dbg(rpu->dev, "Per-RPU workqueue '%s' created.\n", wq_name);

	/* Request TX channel with retry to handle reload timing */
	for (attempt = 0; attempt < 5; attempt++) {
		rpu->tx_chan = mbox_request_channel_byname(&rpu->tx_mc, "tx");
		if (!IS_ERR(rpu->tx_chan))
			break;
		rpu->tx_chan = NULL;
		msleep(100);
	}
	if (!rpu->tx_chan) {
		dev_err(rpu->dev, "Failed to request TX mailbox channel.\n");
		return -EPROBE_DEFER;
	}

	/* Request RX channel with retry to handle reload timing */
	for (attempt = 0; attempt < 5; attempt++) {
		rpu->rx_chan = mbox_request_channel_byname(&rpu->rx_mc, "rx");
		if (!IS_ERR(rpu->rx_chan))
			break;
		rpu->rx_chan = NULL;
		msleep(100);
	}
	if (!rpu->rx_chan) {
		dev_err(rpu->dev, "Failed to request RX mailbox channel.\n");
		return -EPROBE_DEFER;
	}

	INIT_KFIFO(rpu->data_fifo);
	INIT_KFIFO(rpu->app_fifo);

	/* Initialize mutexes for thread-safe kfifo access in workqueue context */
	mutex_init(&rpu->app_fifo_lock);
	mutex_init(&rpu->data_fifo_lock);

	/* Initialize TX mailbox client SKB queue */
	skb_queue_head_init(&rpu->tx_mc_skbs);

	init_completion(&rpu->init_fw_done);
	rpu->init_fw_status = 0;

	return 0;
}

static int visp_mbox_rpudev_open(struct inode *inode, struct file *file)
{
	struct rpu_dev *rpu = NULL;

	/* Retrieve the RPU device structure using container_of */
	rpu = container_of(inode->i_cdev, struct rpu_dev, cdev);
	if (!rpu) {
		pr_err("%s: Failed to retrieve RPU device\n", __func__);
		return -ENODEV; // Return error if RPU is not found
	}

	/* Set private_data for the file structure */
	file->private_data = rpu;

	return 0; // Indicate successful open
}

static int visp_mbox_rpudev_release(struct inode *inode, struct file *file)
{
	struct rpu_dev *rpu = file->private_data;

	/* Perform any necessary cleanup or validation */
	if (!rpu) {
		pr_err("%s: Release called with invalid RPU device\n",
		       __func__);
		return -ENODEV; // Return error if RPU is invalid
	}

	return 0; // Indicate successful release
}

static ssize_t visp_mbox_rpudev_write(struct file *file, const char __user *buf,
				      size_t lbuf, loff_t *offset)
{
	struct rpu_dev *rpu = file->private_data;
	payload_packet *packet = NULL;
	payload_user_packet *user_packet = NULL;
	uint32_t instance_id = 0x99;
	uint8_t *p_data;
	uint32_t flag = 1;
	int ret = 0;

	/* Validate input buffer size */
	if (lbuf < sizeof(payload_user_packet)) {
		pr_err("%s: Insufficient input buffer size\n", __func__);
		return -EINVAL;
	}

	/* Validate RPU instance */
	if (!rpu) {
		pr_err("%s: RPU instance is NULL\n", __func__);
		return -EINVAL;
	}

	/* Allocate memory for user_packet */
	user_packet = kzalloc(sizeof(payload_user_packet), GFP_KERNEL);
	if (!user_packet) {
		pr_err("%s: Failed to allocate memory for user_packet\n",
		       __func__);
		return -ENOMEM;
	}

	/* Allocate memory for packet */
	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		pr_err("%s: Failed to allocate memory for packet\n", __func__);
		kfree(user_packet);
		return -ENOMEM;
	}

	/* Copy data from user space to kernel space */
	if (copy_from_user(user_packet, buf, sizeof(payload_user_packet))) {
		pr_err("%s: copy_from_user failed\n", __func__);
		kfree(packet);
		kfree(user_packet);
		return -EFAULT;
	}

	/* Populate packet with data from user_packet */
	packet->type = user_packet->type;
	packet->cookie = user_packet->cookie;
	packet->payload_size = user_packet->payload_size;
	if (packet->type == RESP) {
		packet->resp_field.processed_cmdid =
			user_packet->resp_field.processed_cmdid;
		packet->resp_field.error_subcode_t =
			user_packet->resp_field.error_subcode_t;
	}
	memcpy(packet->payload, user_packet->payload,
	       sizeof(user_packet->payload));

	/* Extract instance_id from payload */
	p_data = packet->payload;
	memcpy(&instance_id, p_data, sizeof(uint32_t));

	/* write_lock now held internally by write_mboxcmd() with reduced scope */
	ret = visp_mbox_send_command(user_packet->cmd_id, packet,
				     sizeof(payload_packet), flag, rpu->core_id,
				     MBOX_CORE_APU);
	if (ret < 0) {
		pr_err("%s: send_command failed with error: %d\n", __func__,
		       ret);
		kfree(packet);
		kfree(user_packet);
		return -EIO;
	}

	/* IPI trigger - mailbox framework handles synchronization */
	ret = mbox_send_message(rpu->tx_chan, NULL);

	if (ret < 0) {
		pr_err("%s: mbox_send_message() failed: %d\n", __func__, ret);
		kfree(packet);
		kfree(user_packet);
		return -EIO;
	}
	kfree(packet);
	kfree(user_packet);
	return lbuf; // Return the number of bytes written
}

static ssize_t visp_mbox_rpudev_read(struct file *file, char __user *buf,
				     size_t count, loff_t *offset)
{
	struct rpu_dev *rpu = file->private_data;
	ssize_t bytes_copied = sizeof(struct response_user_packet);
	mbox_post_msg *msg;
	int result;

	if (!rpu) {
		pr_err("%s: Invalid RPU device pointer\n", __func__);
		return -EINVAL;
	}

	/* Wait for data to be available in the FIFO */
	result = wait_event_timeout(rpu->read_wait,
				    !kfifo_is_empty(&rpu->app_fifo),
				    msecs_to_jiffies(5000));
	if (result == 0) {
		pr_err("%s: Wait timed out\n", __func__);
		bytes_copied = -ETIMEDOUT;
		goto ERROR;
	}

	mutex_lock(&rpu->app_fifo_lock);
	if (!kfifo_out(&rpu->app_fifo, &msg, 1)) {
		mutex_unlock(&rpu->app_fifo_lock);
		pr_err("Failed to dequeue from app_fifo\n");
		bytes_copied = -ENOMSG;
		goto ERROR;
	}
	mutex_unlock(&rpu->app_fifo_lock);

	/* Validate message size before processing */
	if (msg->size > sizeof(payload_packet)) {
		pr_err("%s: Message size %u exceeds maximum %zu\n",
		       __func__, msg->size, sizeof(payload_packet));
		bytes_copied = -EOVERFLOW;
		visp_free_rx_buffer(rpu, msg);
		goto ERROR;
	}

	/* Use heap allocation for large packet structure */
	struct response_user_packet *user_pkt = kmalloc(sizeof(*user_pkt), GFP_KERNEL);

	if (!user_pkt) {
		bytes_copied = -ENOMEM;
		visp_free_rx_buffer(rpu, msg);
		goto ERROR;
	}

	user_pkt->cmdid = msg->msg_id;
	user_pkt->data = NULL;

	/* Copy payload data with size validation */
	size_t copy_size = min_t(size_t, msg->size, sizeof(payload_packet));

	memcpy(&user_pkt->res_payload_pkt, msg->payload, copy_size);

	/* Copy data to user space */
	if (copy_to_user(buf, user_pkt, bytes_copied)) {
		pr_err("%s: Failed to copy data to user space\n", __func__);
		bytes_copied = -EFAULT;
		kfree(user_pkt);
		visp_free_rx_buffer(rpu, msg);
		goto ERROR;
	}
	kfree(user_pkt);
	visp_free_rx_buffer(rpu, msg);
ERROR:
	return bytes_copied; // Return the size of data copied on success
}

/**
 * visp_mbox_rpudev_poll - poll file operation for mailbox device
 * @file: File structure
 * @wait: Poll table structure
 *
 * Returns POLLIN|POLLRDNORM when data is available in app_fifo
 * Returns 0 when no data is available
 * Returns POLLERR on error conditions
 */
static __poll_t visp_mbox_rpudev_poll(struct file *file, poll_table *wait)
{
	struct rpu_dev *rpu = file->private_data;
	__poll_t mask = 0;

	if (!rpu) {
		pr_err("%s: Invalid RPU device pointer\n", __func__);
		return POLLERR;
	}

	/* Add our wait queue to the poll table */
	poll_wait(file, &rpu->read_wait, wait);

	/* Check if data is available in the FIFO */
	mutex_lock(&rpu->app_fifo_lock);
	if (!kfifo_is_empty(&rpu->app_fifo)) {
		/* Data is available for reading */
		mask |= (POLLIN | POLLRDNORM);
	}
	mutex_unlock(&rpu->app_fifo_lock);

	/* Note: POLLOUT/POLLWRNORM could be added for write readiness if needed */

	return mask;
}

static const struct file_operations visp_mbox_rpudev_fops = {
	.owner		= THIS_MODULE,
	.read		= visp_mbox_rpudev_read,
	.write		= visp_mbox_rpudev_write,
	.poll		= visp_mbox_rpudev_poll,
	.open		= visp_mbox_rpudev_open,
	.release	= visp_mbox_rpudev_release
};
struct reserved_memory reserved_memory;
/* Function to find an existing rpu_dev by rpu_id */

struct rpu_dev *visp_mbox_get_rpu_dev(int rpu_id)
{
	struct rpu_dev *rpu;

	/* Acquire lock to protect the RPU list */
	mutex_lock(&rpu_list_lock);

	/* Check if the RPU devices list is empty */
	if (list_empty(&rpu_devices)) {
		pr_warn("%s: RPU devices list is empty.\n", __func__);
		mutex_unlock(&rpu_list_lock);
		return NULL;
	}

	/* Search for the RPU with the given id */
	list_for_each_entry(rpu, &rpu_devices, node) {
		if (rpu->rpu_id == rpu_id) {
			mutex_unlock(&rpu_list_lock);
			return rpu;
		}
	}

	/* Unlock the mutex before returning */
	mutex_unlock(&rpu_list_lock);

	/* Return NULL as the device was not found */
	return NULL;
}
EXPORT_SYMBOL(visp_mbox_get_rpu_dev);

// TODO make this as static and keep appropriate name
static int visp_mbox_reserved_memory_init(const char *node_name)
{
	struct device_node *node;
	int ret;

	if (!node_name) {
		pr_err("%s: Invalid node name\n", __func__);
		return -EINVAL;
	}

	/* Find the reserved memory node */
	node = of_find_node_by_name(NULL, node_name);
	if (!node) {
		pr_err("%s: Reserved memory node '%s' not found.\n", __func__,
		       node_name);
		return -ENODEV;
	}

	/* Get the physical address */
	ret = of_property_read_u64(node, "reg", &reserved_memory.phys_addr);
	if (ret) {
		pr_err("%s:No reg property for node %s Error: %d\n", __func__,
		       node_name, ret);
		return -EINVAL;
	}

	/* Map to virtual address */
	reserved_memory.virt_addr = of_iomap(node, 0);
	if (!reserved_memory.virt_addr) {
		pr_err("%s: Failed to map virtual address%s\n", __func__,
		       node_name);
		return -ENOMEM;
	}

	pr_debug("%s:Reserved memory initialized node %s\n", __func__,
		 node_name);
	return 0;
}

static void visp_mbox_reserved_memory_exit(void)
{
	if (reserved_memory.virt_addr) {
		iounmap(reserved_memory.virt_addr);
		pr_info("%s: Unmapped reserved memory virtual address\n",
			__func__);
		reserved_memory.virt_addr = NULL;
	} else {
		pr_warn("%s: No reserved memory to unmap.\n", __func__);
	}
}

uint8_t xlnx_mbox_apu_wait_for_data(struct visp_dev *isp_dev, void *data)
{
	struct rpu_dev *rpu = NULL;
	mbox_post_msg *msg;
	long result = 0;
	int ret;

	if (!isp_dev) {
		pr_err("xlnx_mbox_apu_wait_for_data: Invalid ISP device (NULL "
		       "pointer).\n");
		ret = -EINVAL;
		goto ERROR;
	}

	rpu = isp_dev->rpu;
	if (!rpu) {
		dev_err(isp_dev->dev, "RPU device is NULL in %s.\n", __func__);
		ret = -ENOMEM;
		goto ERROR;
	}

	if (!data) {
		dev_err(isp_dev->dev, "Invalid data pointer in %s.\n",
			__func__);
		ret = -EINVAL;
		goto ERROR;
	}

	/* 3-minute timeout for data commands */
	long timeout_jiffies = msecs_to_jiffies(1000 * 60 * 3);
	result = wait_for_completion_timeout(
	    &isp_dev->apu_wait_for_data, timeout_jiffies);
	if (result == 0) {
		dev_err(rpu->dev,
			"Timeout occurred while waiting for APU ACK\n");
		ret = -ETIMEDOUT;
		goto ERROR;
	}

	mutex_lock(&rpu->data_fifo_lock);
	if (!kfifo_out(&rpu->data_fifo, &msg, 1)) {
		mutex_unlock(&rpu->data_fifo_lock);
		pr_err("Failed to dequeue from data_fifo\n");
		ret = -ENOMEM;
		goto ERROR;
	}

	/* Directly extract data from message payload */
	payload_packet *pkt = (payload_packet *)msg->payload;

	memcpy(data, pkt->payload, sizeof(pkt->payload));
	ret = pkt->resp_field.error_subcode_t;

	mutex_unlock(&rpu->data_fifo_lock);
	visp_free_rx_buffer(rpu, msg);
ERROR:
	/* Return the error code from the interrupt's response payload */
	return ret;
}
EXPORT_SYMBOL(xlnx_mbox_apu_wait_for_data);

int xlnx_send_mbox_data_cmd(struct visp_dev *isp_dev, mb_cmd_id_e cmd,
			    void *data, uint32_t size, uint8_t dest_cpu,
			    uint8_t src_cpu)
{
	int result;
	struct rpu_dev *rpu;
	uint32_t flag = 0;

	if (!isp_dev || !isp_dev->rpu || !data) {
		pr_err("%s: Invalid input parameters\n", __func__);
		return -EINVAL;
	}

	rpu = isp_dev->rpu;

	result = visp_mbox_send_command(cmd, data, size, flag, rpu->core_id,
					src_cpu);
	if (result != 0) {
		dev_err(rpu->dev,
			"%s: Mailbox Send message failed at line %d\n",
			__func__, __LINE__);
		goto unlock_and_exit;
	}

	result = mbox_send_message(rpu->tx_chan, NULL);
	if (result < 0) {
		dev_err(rpu->dev, "%s: mbox_send_message failed at line %d\n",
			__func__, __LINE__);
		goto unlock_and_exit;
	}

	payload_packet *pkt = (payload_packet *)data;

	if (!pkt) {
		dev_err(rpu->dev, "%s: Payload data is NULL\n", __func__);
		result = -EINVAL;
		goto unlock_and_exit;
	}

	result = xlnx_mbox_apu_wait_for_data(isp_dev, pkt->payload);
	if (result) {
		dev_err(rpu->dev, "%s: Failed to get buffer data\n", __func__);
		goto unlock_and_exit;
	}

	return result;
unlock_and_exit:
	return result;
}
EXPORT_SYMBOL(xlnx_send_mbox_data_cmd);

int xlnx_send_mbox_without_ack_cmd(struct visp_dev *isp_dev, mb_cmd_id_e cmd,
				   void *data, uint32_t size, uint8_t dest_cpu,
				   uint8_t src_cpu)
{
	int result;
	struct rpu_dev *rpu;
	uint32_t flag = 0;

	if (!isp_dev || !isp_dev->rpu) {
		pr_err("%s: Invalid ISP device\n", __func__);
		return -EINVAL;
	}
	rpu = isp_dev->rpu;

	/* write_lock now held internally by write_mboxcmd() with reduced scope */
	result = visp_mbox_send_command(cmd, data, size, flag, rpu->core_id,
					src_cpu);
	if (result != 0) {
		dev_err(rpu->dev,
			"%s: Mailbox Send message failed at line %d\n",
			__func__, __LINE__);
		return result;
	}

	/* IPI trigger - mailbox framework handles synchronization */
	result = mbox_send_message(rpu->tx_chan, NULL);
	if (result < 0) {
		dev_err(rpu->dev, "%s: mbox_send_message failed at line %d\n",
			__func__, __LINE__);
		return result;
	}
	return 0; // Success
}
EXPORT_SYMBOL(xlnx_send_mbox_without_ack_cmd);

int xlnx_send_mbox_acked_cmd(struct visp_dev *isp_dev, mb_cmd_id_e cmd,
			     void *data, uint32_t size, uint8_t dest_cpu,
			     uint8_t src_cpu)
{
	int result;
	struct rpu_dev *rpu;
	uint32_t flag = 0;
	uint32_t instance_id, port;
	uint32_t path = 0;
	uint32_t path_idx = 0;
	uint32_t buffer_index = 0;
	uint8_t *p_data;

	if (!isp_dev || !isp_dev->rpu) {
		pr_err("%s: Invalid ISP device\n", __func__);
		return -EINVAL;
	}

	if (!data) {
		pr_err("%s: Invalid data pointer\n", __func__);
		return -EINVAL;
	}

	/* Extract instance_id from payload (common to all commands) */
	p_data = ((payload_packet *)data)->payload;
	memcpy(&instance_id, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	/* Map instance_id to port: 4 instances per port (common to all commands) */
	port = instance_id % MAX_PORTS;

	if (port >= 4) {
		pr_err("%s: Invalid port %u (max 3)\n", __func__, port);
		return -EINVAL;
	}

	rpu = isp_dev->rpu;

	if (cmd == APU_2_RPU_MB_CMD_ENQUE_BUFFER) {
		/* Extract path and buffer_index from payload */
		memcpy(&path, p_data, sizeof(uint32_t));
		p_data += sizeof(uint32_t);
		memcpy(&buffer_index, p_data, sizeof(uint8_t));

		/* Initialize path_idx for MIMO mapping */
		path_idx = path;

		/* MIMO path mapping: firmware reports path=6, map to path=3 */
		if (path == 6 && isp_dev->ss_mode_i0 &&
		    strcmp(isp_dev->ss_mode_i0, "mimo") == 0)
			path_idx = 3;

		/* Validate path for array bounds (only 0-3 supported) */
		if (path_idx >= 4) {
			pr_err("%s: Invalid path %u for ENQUE_BUFFER "
			       "(expected 0-3)\n", __func__, path);
			return -EINVAL;
		}

		/* Validate buffer_index for array bounds */
		if (buffer_index >= 32) {
			pr_err("%s: Invalid buffer_index %u for ENQUE_BUFFER "
			       "(max 31)\n", __func__, buffer_index);
			return -EINVAL;
		}

#ifdef DEBUG_MBOX_SANITY_CHECKS
		/* Sanity check: Should never happen if buffer management is correct */
		if (try_wait_for_completion(&isp_dev->apu_wait_for_enq_ack
				    [port][path_idx][buffer_index])) {
			dev_err(rpu->dev,
				"BUG: Stale ENQUE_BUFFER completion detected! "
				"Buffer %u already in use (instance=%u path=%u)\n",
				buffer_index, instance_id, path_idx);
		}
#endif
		reinit_completion(&isp_dev->apu_wait_for_enq_ack[port]
				  [path_idx][buffer_index]);
	} else {
#ifdef DEBUG_MBOX_SANITY_CHECKS
		/* Sanity check: Drain stale completion and message */
		if (try_wait_for_completion(&isp_dev->apu_wait_for_cmd_ack
		    [port])) {
			/* Also drain any stale message from FIFO */
			mbox_post_msg *stale_msg = NULL;

			mutex_lock(&isp_dev->cmd_ack_fifo_lock[port]);
			if (kfifo_out(&isp_dev->cmd_ack_fifo[port], &stale_msg, 1)) {
				visp_free_rx_buffer(rpu, stale_msg);
				dev_err(rpu->dev,
					"BUG: Stale cmd ACK detected (port=%u)\n",
					port);
			}
			mutex_unlock(&isp_dev->cmd_ack_fifo_lock[port]);
		}
#endif
		reinit_completion(&isp_dev->apu_wait_for_cmd_ack[port]);
	}

	/* write_lock now held internally by write_mboxcmd() with reduced scope */
	result = visp_mbox_send_command(cmd, data, size, flag, rpu->core_id,
					src_cpu);
	if (result != 0) {
		dev_err(rpu->dev,
			"%s: Mailbox Send message failed at line %d\n",
			__func__, __LINE__);
		return result;
	}

	/* IPI trigger - mailbox framework handles synchronization */
	result = mbox_send_message(rpu->tx_chan, NULL);
	if (result < 0) {
		dev_err(rpu->dev, "%s: mbox_send_message failed at line %d\n",
			__func__, __LINE__);
		return result;
	}

	/* RPU lock released - TX mailbox operation complete */

	/* Wait for ACK outside the lock */
	result = xlnx_mbox_apu_wait_for_ack(isp_dev, instance_id, path_idx, buffer_index, cmd);
	if (result < 0) {
		if (result == -ENODEV) {
			dev_warn(rpu->dev,
				"%s: Mailbox not ready for cmd=%d (instance_id=%u)\n",
				__func__, cmd, instance_id);
			return -EPIPE;
		}
		dev_err(rpu->dev, "%s: Failed to get ack\n", __func__);
		return result;
	}

	return result;
}
EXPORT_SYMBOL(xlnx_send_mbox_acked_cmd);

uint8_t xlnx_mbox_apu_wait_for_ack(struct visp_dev *isp_dev,
				   uint32_t instance_id, uint32_t path,
				   uint32_t buffer_index, mb_cmd_id_e cmd)
{
	struct rpu_dev *rpu = NULL;
	long result;
	mbox_post_msg *msg;
	int ret;
	uint32_t port;
	uint32_t path_idx = path;

	if (!isp_dev) {
		pr_err("%s : Invalid ISP device (NULL pointer).\n", __func__);
		ret = -EINVAL;
		goto ERROR;
	}

	/* Map instance_id to port: 4 instances per port (16 instances / 4 ports) */
	port = instance_id % MAX_PORTS;

	if (port >= MAX_PORTS) {
		pr_err("%s: Invalid instance_id %u -> port %u (max port %u)\n",
		       __func__, instance_id, port, MAX_PORTS - 1);
		ret = -EINVAL;
		goto ERROR;
	}

	if (port >= isp_dev->num_streams) {
		dev_err(isp_dev->dev,
			"%s: instance_id %u maps to inactive port %u (num_streams=%u)\n",
			__func__, instance_id, port, isp_dev->num_streams);
		ret = -EINVAL;
		goto ERROR;
	}

	rpu = isp_dev->rpu;
	if (!rpu) {
		dev_err(isp_dev->dev, "RPU device is NULL in %s.\n", __func__);
		ret = -EINVAL;
		goto ERROR;
	}

	if (!rpu->tx_chan || !rpu->rx_chan) {
		dev_err(isp_dev->dev,
			"Mailbox channels not initialized in %s (tx=%p, rx=%p).\n",
			__func__, rpu->tx_chan, rpu->rx_chan);
		ret = -ENODEV;
		goto ERROR;
	}

	if (cmd == APU_2_RPU_MB_CMD_ENQUE_BUFFER) {
		/* ENQUE_BUFFER: 20s timeout for fast-path */
		result = wait_for_completion_timeout(
		    &isp_dev->apu_wait_for_enq_ack[port][path_idx]
		    [buffer_index], msecs_to_jiffies(20000));

		if (result == 0) {
			/* Timeout */
			dev_err_ratelimited(rpu->dev,
					    "ENQUEUE TIMEOUT: No ACK in 20s "
					    "(instance_id=%u->port=%u, path=%u, "
					    "buf=%u) - RPU overwhelmed or RX "
					    "workqueue stalled. Check 'Bdgt "
					    "exhausted' logs.\n",
				instance_id, port, path, buffer_index);
			ret = -ETIMEDOUT;
			goto ERROR;
		}

		/* Success: read error code directly from storage */
		ret = isp_dev->enq_ack_error[port][path_idx][buffer_index];

	} else {
		/* Regular commands: 120s timeout for pipeline operations */
		result = wait_for_completion_timeout(
		    &isp_dev->apu_wait_for_cmd_ack[port],
		    msecs_to_jiffies(120000));

		if (result == 0) {
			/* Timeout - drain FIFO to prevent leak */
			dev_err(rpu->dev, "Timeout waiting for APU ACK "
				"(cmd=%d, instance_id=%u->port=%u)\n",
				cmd, instance_id, port);

			mbox_post_msg *leaked_msg = NULL;

			mutex_lock(&isp_dev->cmd_ack_fifo_lock[port]);
			if (kfifo_out(&isp_dev->cmd_ack_fifo[port],
				      &leaked_msg, 1)) {
				visp_free_rx_buffer(rpu, leaked_msg);
				dev_warn(rpu->dev,
					 "Drained late ACK cmd_fifo[%u] "
					 "after timeout\n", port);
			}
			mutex_unlock(&isp_dev->cmd_ack_fifo_lock[port]);
			ret = -ETIMEDOUT;
			goto ERROR;
		}

		/* Success: dequeue message and extract error code */
		mutex_lock(&isp_dev->cmd_ack_fifo_lock[port]);
		if (!kfifo_out(&isp_dev->cmd_ack_fifo[port], &msg, 1)) {
			mutex_unlock(&isp_dev->cmd_ack_fifo_lock[port]);
			pr_err("Failed to dequeue from cmd_ack_fifo[%u]\n",
			       port);
			ret = -ENOMSG;
			goto ERROR;
		}
		mutex_unlock(&isp_dev->cmd_ack_fifo_lock[port]);

		if (!msg) {
			pr_err("Received invalid message\n");
			ret = -EINVAL;
			goto ERROR;
		}

		payload_packet *pkt = (payload_packet *)msg->payload;

		ret = pkt->resp_field.error_subcode_t;
		visp_free_rx_buffer(rpu, msg);
	}

	return ret;
ERROR:
	return ret;

}
EXPORT_SYMBOL(xlnx_mbox_apu_wait_for_ack);

/* Helper function to find rproc device among children */
static int find_rproc_child(struct device *dev, void *data)
{
	struct rproc **rp = (struct rproc **)data;

	/* Try to get rproc from this device's driver data */
	*rp = dev_get_drvdata(dev);
	if (*rp)
		return 1; /* Found it, stop iteration */
	return 0; /* Continue searching */
}

static int visp_mbox_send_init_firmware(struct rpu_dev *rpu)
{
	payload_packet *pkt = NULL;
	int ret;

	if (!rpu || !rpu->tx_chan)
		return -EINVAL;

	if (!rpu->rproc || rpu->rproc->state != RPROC_RUNNING)
		return 0;

	pkt = kzalloc(sizeof(*pkt), GFP_KERNEL);
	if (!pkt)
		return -ENOMEM;

	pkt->type = CMD;
	pkt->cookie = 0;
	uint32_t instance_id = 0;

	memcpy(pkt->payload, &instance_id, sizeof(instance_id));
	pkt->payload_size = sizeof(instance_id);

	for (int attempt = 0; attempt < 5; attempt++) {
		reinit_completion(&rpu->init_fw_done);
		rpu->init_fw_status = 0;

		ret = visp_mbox_send_command(
			APU_2_RPU_MB_CMB_INIT_FIRMWARE, pkt,
			payload_extra_size, 0,
			rpu->core_id, MBOX_CORE_APU);
		if (ret < 0) {
			dev_err(rpu->dev,
				"Failed to send INIT_FIRMWARE cmd: %d\n", ret);
			kfree(pkt);
			return ret;
		}

		ret = mbox_send_message(rpu->tx_chan, NULL);
		if (ret < 0) {
			dev_err(rpu->dev,
				"Failed to trigger IPI. Error: %d\n", ret);
			kfree(pkt);
			return ret;
		}

		if (wait_for_completion_timeout(
				&rpu->init_fw_done,
				msecs_to_jiffies(200))) {
			if (rpu->init_fw_status != 0) {
				kfree(pkt);
				return -EIO;
			}
			visp_mbox_mark_init_firmware_success(rpu);
			kfree(pkt);
			return 0;
		}
		dev_warn_ratelimited(rpu->dev,
			"INIT_FIRMWARE ack timeout, retrying\n");
	}

	kfree(pkt);
	return -ETIMEDOUT;
}

/**
 * visp_mbox_firmware_load - Load and boot firmware via remoteproc
 * @rpu: pointer to the RPU device structure
 *
 * Discovers the remoteproc instance via device tree phandle, sets the
 * firmware name (isp-r52-<rpu_id>-firmware.elf), boots the remote
 * processor, and triggers an IPI sync after a settling delay.
 *
 * Return: 0 on success or if remoteproc is unavailable/already running,
 *         negative errno on IPI failure.
 */
static int visp_mbox_firmware_load(struct rpu_dev *rpu)
{
	struct device_node *node, *rproc_node;
	struct device *dev;
	int rpu_id;
	int ret;
	char fw_name[64];

	node = rpu->dev->of_node;
	dev = rpu->dev;
	rpu_id = rpu->rpu_id;

	/* Get remoteproc node using phandle */
	rproc_node = of_parse_phandle(node, "rproc", 0);
	if (!rproc_node) {
		dev_warn(dev,
			 "No rproc phandle for RPU %d, skipping remoteproc\n",
			 rpu_id);
		return -ENODEV;
	}

	dev_info(dev, "Found remoteproc node: %s\n", rproc_node->full_name);

	/* Find the remoteproc platform device */
	rpu->rproc_pdev = of_find_device_by_node(rproc_node);
	of_node_put(rproc_node);

	if (!rpu->rproc_pdev) {
		dev_err(dev,
			"Remoteproc platform device not found for RPU %d\n",
			rpu_id);
		return -ENODEV;
	}

	/* Search for the remoteproc device by looking
	 * at the platform device's children
	 */
	rpu->rproc = NULL;
	device_for_each_child(&rpu->rproc_pdev->dev, &rpu->rproc,
			      find_rproc_child);

	if (!rpu->rproc) {
		dev_err(dev, "Remoteproc not initialized yet for RPU %d\n",
			rpu_id);
		put_device(&rpu->rproc_pdev->dev);
		return -ENODEV;
	}

	dev_info(dev, "Found remoteproc instance for RPU %d\n", rpu_id);

	/* Check if remoteproc is already running */
	if (rpu->rproc->state == RPROC_RUNNING) {
		dev_info(dev,
			 "Remoteproc already running for RPU %d, skipping boot\n",
			rpu_id);
		return 0;
	}

	/* Generate and set firmware name */
	snprintf(fw_name, sizeof(fw_name), "isp-r52-%d-firmware.elf",
		 rpu_id);
	dev_info(dev, "Using firmware: %s for RPU %d\n", fw_name, rpu_id);

	ret = rproc_set_firmware(rpu->rproc, fw_name);
	if (ret) {
		dev_err(dev, "Failed to set firmware name: %d\n", ret);
		rpu->rproc = NULL;
		put_device(&rpu->rproc_pdev->dev);
		return ret;
	}

	/* Automatically boot the remoteproc (firmware loading) */
	dev_info(dev, "Booting remoteproc for RPU %d\n", rpu_id);
	ret = rproc_boot(rpu->rproc);
	if (ret) {
		dev_err(dev, "Failed to boot remoteproc for RPU %d: %d\n",
			rpu_id, ret);
		dev_err(dev, "Ensure firmware exists in /lib/firmware/\n");
		/*
		 * Keep mailbox resources intact so users can load custom firmware
		 * and retry boot without re-probing the device.
		 */
		rpu->rproc = NULL;
		put_device(&rpu->rproc_pdev->dev);
		return ret;
	}

	dev_info(dev, "Successfully booted remoteproc for RPU %d\n", rpu_id);

	return 0;
}

static int visp_mbox_rpu_remove(struct rpu_dev *rpu);

static int visp_mbox_mailbox_initialization(struct rpu_dev *rpu)
{
	int ret;

	if (!rpu) {
		pr_err("%s: Invalid RPU device (NULL pointer).\n", __func__);
		return -EINVAL;
	}

	/* Verify reserved memory is initialized */
	if (!reserved_memory.virt_addr) {
		dev_err(rpu->dev, "%s: Reserved memory not initialized (virt_addr is NULL).\n",
			__func__);
		return -ENOMEM;
	}

	if (!reserved_memory.phys_addr) {
		dev_err(rpu->dev, "%s: Reserved memory not initialized (phys_addr is NULL).\n",
			__func__);
		return -ENOMEM;
	}

	/* Initialize mailbox with reserved memory */
	ret = visp_mbox_mailbox_init(rpu, MBOX_CORE_APU,
				     (uintptr_t)reserved_memory.virt_addr,
				     (uintptr_t)reserved_memory.phys_addr);
	if (ret < 0) {
		dev_err(rpu->dev, "Failed to initialize mailbox structures. Error: %d\n", ret);
		return ret;
	}

	/* Load firmware via remoteproc and trigger IPI */
	ret = visp_mbox_firmware_load(rpu);
	if (ret < 0)
		return ret;

	return 0;
}

static void visp_mbox_rpu_cleanup(struct kref *ref)
{
	//   struct rpu_dev *rpu = container_of(ref, struct rpu_dev, refcount);
	//        devm_kfree(&rpu->pdev->dev, rpu);
}

static struct rpu_dev *visp_mbox_get_or_create_rpu(struct platform_device *pdev,
						   int rpu_id)
{
	struct rpu_dev *rpu;
	struct device_node *node;
	struct device *mboxdev = NULL;
	char dev_name[20];
	int ret;
	bool devno_allocated = false;
	bool cdev_added = false;
	bool drvdata_set = false;

	node = pdev->dev.of_node;
	mutex_lock(&rpu_list_lock);

	/* Look for an existing RPU with the given rpu_id */
	list_for_each_entry(rpu, &rpu_devices, node) {
		if (rpu->rpu_id == rpu_id) {
			kref_get(&rpu->refcount);
			mutex_unlock(&rpu_list_lock);
			return rpu;
		}
	}

	/* Create a new RPU */
	rpu = devm_kzalloc(&pdev->dev, sizeof(*rpu), GFP_KERNEL);
	if (!rpu) {
		dev_err(&pdev->dev, "Failed to allocate memory for RPU\n");
		goto cleanup;
	}

	snprintf(dev_name, sizeof(dev_name), "%s_%d", CHAR_DEV_NAME, rpu_id);

	/*
	 * Initialize pre-allocated buffer pools with free-list management.
	 * Each pool has 32 buffers statically embedded in rpu_dev structure.
	 * Free-list provides O(1) alloc/free with spinlock protection.
	 * Benefits:
	 *   - No dynamic allocation overhead (~40-60ns vs kmem_cache ~300ns)
	 *   - Deterministic behavior (pre-allocated at init)
	 *   - Thread-safe with proper ownership tracking
	 *   - No buffer exhaustion from memory pressure
	 */
	int i;

	/* Initialize free-list heads */
	INIT_LIST_HEAD(&rpu->tx_free_list);
	INIT_LIST_HEAD(&rpu->rx_free_list);

	/* Initialize spinlocks */
	spin_lock_init(&rpu->tx_free_lock);
	spin_lock_init(&rpu->rx_free_lock);

	/* Add all TX buffers to free list */
	for (i = 0; i < MSG_BUFFER_POOL_SIZE; i++) {
		INIT_LIST_HEAD(&rpu->tx_bufs[i].list);
		list_add(&rpu->tx_bufs[i].list, &rpu->tx_free_list);
	}

	/* Add all RX buffers to free list */
	for (i = 0; i < MSG_BUFFER_POOL_SIZE; i++) {
		INIT_LIST_HEAD(&rpu->rx_bufs[i].list);
		list_add(&rpu->rx_bufs[i].list, &rpu->rx_free_list);
	}

	mutex_init(&rpu->rpu_lock);
	mutex_init(&rpu->read_lock);
	mutex_init(&rpu->write_lock);
	cdev_init(&rpu->cdev, &visp_mbox_rpudev_fops);
	rpu->cdev.owner = THIS_MODULE;

	/* Allocate a device number */
	ret = alloc_chrdev_region(&rpu->devno, rpu_id, 1, dev_name);
	if (ret) {
		dev_err(&pdev->dev,
			"Failed to allocate device number (error %d)\n", ret);
		goto cleanup;
	}
	devno_allocated = true;

	/* Add the character device */
	ret = cdev_add(&rpu->cdev, rpu->devno, 1);
	if (ret) {
		dev_err(&pdev->dev, "Failed to add cdev (error %d)\n", ret);
		goto cleanup;
	}
	cdev_added = true;

	rpu->dev = &pdev->dev;
	rpu->rpu_id = rpu_id;
	rpu->core_id = rpu_id - VISP_MBOX_RPU6;

	/* Setup mailbox if required */
	if (of_property_read_bool(node, "mboxes")) {
		dev_dbg(&pdev->dev, "Setting up mailboxes for RPU with id %d\n",
			rpu_id);
		ret = visp_mbox_setup(rpu, node);
		if (ret) {
			dev_err(&pdev->dev,
				"Failed to set up mailboxes (error %d)\n", ret);
			goto cleanup;
		}
	} else {
		goto cleanup;
	}

	/* Store the RPU pointer in the platform device's driver data */
	platform_set_drvdata(pdev, rpu);
	drvdata_set = true;

	/* Initialize the mailbox */
	ret = visp_mbox_mailbox_initialization(rpu);
	if (ret) {
		dev_err(&pdev->dev, "Failed to initialize mailbox for RPU id: %d\n",
			rpu_id);
		/* Clear driver data on failure */
		platform_set_drvdata(pdev, NULL);
		goto cleanup;
	}

	/* Create a device node only after successful mailbox initialization. */
	mboxdev = device_create(mailbox_class, NULL, rpu->devno, NULL,
				dev_name);
	if (!mboxdev) {
		dev_err(&pdev->dev, "Failed to create device node\n");
		goto cleanup;
	}

	kref_init(&rpu->refcount);

	/*
	 * Add the new RPU to the list only after successful setup and
	 * kref_init
	 */
	list_add_tail(&rpu->node, &rpu_devices);

	/* Initialize wait queue for poll() and read() blocking */
	init_waitqueue_head(&rpu->read_wait);

	mutex_unlock(&rpu_list_lock);
	return rpu;

cleanup:
	if (rpu) {
		if (drvdata_set)
			platform_set_drvdata(pdev, NULL);
		if (rpu->tx_chan)
			mbox_free_channel(rpu->tx_chan);
		if (rpu->rx_chan)
			mbox_free_channel(rpu->rx_chan);
		if (rpu->rpu_wq) {
			cancel_work_sync(&rpu->mbox_work);
			destroy_workqueue(rpu->rpu_wq);
			rpu->rpu_wq = NULL;
		}
		/* Buffer pools are embedded in rpu_dev - no cleanup needed */
		if (cdev_added)
			cdev_del(&rpu->cdev);
		if (devno_allocated)
			unregister_chrdev_region(rpu->devno, 1);
		if (mboxdev)
			device_destroy(mailbox_class, rpu->devno);

		devm_kfree(&pdev->dev, rpu);
	}
	mutex_unlock(&rpu_list_lock);
	return NULL;
}

static int visp_mbox_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rpu_dev *rpu = NULL;
	int ret;
	struct device_node *node;
	int rpu_id = -1;

	node = pdev->dev.of_node;

	#ifdef ISP_VERSION_MACRO
	    dev_info(dev, "|****************************************|\n");
	    dev_info(dev, "|isp modules : %s|\n", ISP_VERSION_MACRO);
	    dev_info(dev, "|****************************************|\n");
	#endif

	/* Read the rpu_id property */
	ret = of_property_read_u32(node, "rpu_id", &rpu_id);
	if (ret) {
		dev_err(dev, " Failed to read rpu_id from device tree\n");
		return -EINVAL;
	}


	/* Validate rpu_id if there is a known range */
	if (rpu_id < 6 || rpu_id > VISP_MBOX_MAX_RPU_ID) {
		dev_err(dev, "Invalid rpu_id: %d\n", rpu_id);
		return -EINVAL;
	}

	/* Find or create a new RPU with the given rpu_id */
	rpu = visp_mbox_get_or_create_rpu(pdev, rpu_id);
	if (!rpu) {
		dev_err(dev, "Failed to find or create RPU with id: %d\n",
			rpu_id);
		return -ENOMEM;
	}

	ret = visp_mbox_send_init_firmware(rpu);
	if (ret) {
		dev_err(dev, "INIT_FIRMWARE sync failed for RPU id: %d\n",
			rpu_id);
		platform_set_drvdata(pdev, NULL);
		if (atomic_read(&rpu->refcount.refcount.refs) > 1)
			kref_put(&rpu->refcount, visp_mbox_rpu_cleanup);
		else
			visp_mbox_rpu_remove(rpu);
		return ret;
	}

	dev_info(dev, "Mailbox successfully initialized for RPU id: %d\n",
		 rpu->rpu_id);
	return 0;
}

static int visp_mbox_rpu_remove(struct rpu_dev *rpu)
{
	int ref_count = 0;

	if (!rpu) {
		/* Handle NULL RPU device (shouldn't normally happen) */
		pr_err("Encountered a NULL RPU device in the list skipping.\n");
		return -ENODEV;
	}

	/* Debug log the current RPU's reference count */
	ref_count = atomic_read(&rpu->refcount.refcount.refs);

	dev_dbg(rpu->dev, "RPU id %d - refcount = %d\n", rpu->rpu_id, ref_count);

	/* Check if the device is safe to remove */
	if (ref_count == 1) {
		WRITE_ONCE(rpu->removing, true);

		if (rpu->rpu_wq) {
			cancel_work_sync(&rpu->mbox_work);
			destroy_workqueue(rpu->rpu_wq);
			rpu->rpu_wq = NULL;
		}

		rpu->rx_mc.rx_callback = NULL;

		if (rpu->tx_chan) {
			if (visp_mbox_pair_peer_active(rpu)) {
				dev_info(rpu->dev,
					 "Skipping mailbox TX channel free for paired active peer (tx=%p, rpu_id=%d).\n",
					 rpu->tx_chan, rpu->rpu_id);
			} else {
				dev_info(rpu->dev,
					 "Freeing mailbox TX channel (tx=%p).\n",
					 rpu->tx_chan);
				mbox_free_channel(rpu->tx_chan);
			}
			rpu->tx_chan = NULL;
		}

		if (rpu->rx_chan) {
			if (visp_mbox_pair_peer_active(rpu)) {
				dev_info(rpu->dev,
					 "Skipping mailbox RX channel free for paired active peer (rx=%p, rpu_id=%d).\n",
					 rpu->rx_chan, rpu->rpu_id);
			} else {
				dev_info(rpu->dev,
					 "Freeing mailbox RX channel (rx=%p).\n",
					 rpu->rx_chan);
				mbox_free_channel(rpu->rx_chan);
			}
			rpu->rx_chan = NULL;
		}

		visp_mbox_clear_init_firmware_success(rpu);

		/* Shutdown and release remoteproc if it was booted and running */
		if (rpu->rproc) {
			if (rpu->init_fw_synced && rpu->rproc->state == RPROC_RUNNING) {
				dev_info(rpu->dev,
					 "Shutting down remoteproc for RPU %d\n",
					 rpu->rpu_id);
				rproc_shutdown(rpu->rproc);
			} else if (rpu->rproc->state == RPROC_RUNNING) {
				dev_info(rpu->dev,
					 "Skipping remoteproc shutdown for unsynchronized RPU %d remove\n",
					 rpu->rpu_id);
			}
			rpu->rproc = NULL;
		}

		/* Release platform device reference */
		if (rpu->rproc_pdev) {
			put_device(&rpu->rproc_pdev->dev);
			rpu->rproc_pdev = NULL;
		}

		/* Perform cleanup and removal */
		/* Buffer pools are embedded in rpu_dev - freed with structure */

		device_destroy(mailbox_class, rpu->devno);
		cdev_del(&rpu->cdev);
		unregister_chrdev_region(rpu->devno, 1);

		/* Remove the RPU from the list */
		mutex_lock(&rpu_list_lock);
		list_del(&rpu->node);
		mutex_unlock(&rpu_list_lock);

		/* Free resources using the kref_put mechanism */
		kref_put(&rpu->refcount, visp_mbox_rpu_cleanup);
		/* Log successful removal */
		dev_info(rpu->dev,
			 "RPU with id %d removed successfully.\n", rpu->rpu_id);
	} else {
		/* If still in use, decrement reference and log */
		kref_put(&rpu->refcount, visp_mbox_rpu_cleanup);
		dev_warn(rpu->dev,
			 "Cannot remove RPU%d still in use (refcount=%d)",
			 rpu->rpu_id, ref_count);
		return -EBUSY;
	}

		return SUCCESS;
}

static void visp_mbox_shutdown(struct platform_device *pdev)
{
	struct rpu_dev *rpu;
	int ret;

	/* Retrieve the RPU device associated with this platform device */
	rpu = platform_get_drvdata(pdev);
	if (!rpu) {
		dev_err(&pdev->dev,
			"Failed to retrieve RPU device during shutdown.\n");
		return;
	}

	/* Call rpu_remove to handle RPU-specific cleanup */
	ret = visp_mbox_rpu_remove(rpu);
	if (ret != 0)
		pr_err("Failed to do rpu resource clean up.\n");
}

static void visp_mbox_remove(struct platform_device *pdev)
{
	struct rpu_dev *rpu;
	int ret;

	/* Retrieve the RPU device associated with this platform device */
	rpu = platform_get_drvdata(pdev);
	if (!rpu) {
		dev_err(&pdev->dev,
			"Failed to retrieve RPU device during remove.\n");
		return;
	}

	/* Call rpu_remove to handle RPU-specific cleanup */
	ret = visp_mbox_rpu_remove(rpu);
	if (ret != 0)
		pr_err("Failed to do rpu resource clean up.\n");

	return;
}

static const struct of_device_id visp_mbox_of_match[] = {
	{.compatible = "xlnx,mimo-mbox"},
	{.compatible = "xlnx,mbox"},
	{.compatible = "xlnx,isp-mbox"},
	{/* sentinel */},
};

MODULE_DEVICE_TABLE(of, visp_mbox_of_match);

static struct platform_driver visp_mbox_driver = {
	.probe = visp_mbox_probe,
	.remove = visp_mbox_remove,
	.driver = {
		.name = "visp_mbox_driver",
		.owner = THIS_MODULE,
		.of_match_table = visp_mbox_of_match,
		},
	.shutdown = visp_mbox_shutdown
};

static int __init visp_mbox_init_module(void)
{
	int ret;

	pr_info("Initializing AMD MBox driver.\n");

	/* Create the mailbox class */
	// mailbox_class = class_create(THIS_MODULE, "mailbox-class");
	mailbox_class = class_create("mailbox-class");
	if (IS_ERR(mailbox_class)) {
		pr_err("Failed to create mailbox class.\n");
		ret = PTR_ERR(mailbox_class);
		return ret;
	}

	/* Initialize reserved memory */
	ret = visp_mbox_reserved_memory_init("isp_mbox_buffer");
	if (ret) {
		pr_err("Failed to initialize reserved memory. Error: %d\n",
		       ret);
		goto err_cleanup_class;
	}

	/* Register the platform driver */
	ret = platform_driver_register(&visp_mbox_driver);
	if (ret) {
		pr_err("Failed to register AMD MBox driver. Error: %d\n", ret);
		goto err_cleanup_reserved_mem;
	}

	pr_info("AMD MBox driver registered successfully.\n");
	return 0;

err_cleanup_reserved_mem:
	visp_mbox_reserved_memory_exit();
err_cleanup_class:
	class_destroy(mailbox_class);
	mailbox_class = NULL;
	return ret;
}

static void __exit visp_mbox_exit_module(void)
{
	pr_info("Exiting AMD MBox driver.\n");

	/* Unregister the platform driver */
	platform_driver_unregister(&visp_mbox_driver);
	pr_info("AMD MBox driver unregistered successfully.\n");

	/* Clean up reserved memory structure */
	visp_mbox_reserved_memory_exit();
	pr_info("Reserved memory cleaned up.\n");

	/* Destroy the mailbox class */
	if (mailbox_class) {
		class_destroy(mailbox_class);
		pr_info("Mailbox class destroyed successfully.\n");
	}
}

module_init(visp_mbox_init_module);
module_exit(visp_mbox_exit_module);

MODULE_DESCRIPTION("AMD MBOX driver");
MODULE_AUTHOR("Thippeswamy Havalige thippeswamy.havalige@amd.com");
MODULE_AUTHOR("Ajay Kumar Nandam anandam@amd.com");
MODULE_AUTHOR("Anil Mamidala amamidal@amd.com");
MODULE_LICENSE("GPL");
