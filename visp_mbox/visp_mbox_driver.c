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
#include <linux/kernel.h>
#include "visp_mbox_driver.h"
#include "mbox_cmd.h"
#include "mbox_api.h"

struct class *mailbox_class;
static DEFINE_MUTEX(rpu_list_lock);
static LIST_HEAD(rpu_devices);

typedef int (*frameout_cb_t)(struct visp_dev *dev);

static void visp_mbox_event_notified(struct work_struct *work)
{
	struct rpu_dev *rpu;
	int ret;

	/* Get the visp_dev structure from the work struct */
	rpu = container_of(work, struct rpu_dev, mbox_work);
	if (!rpu) {
		dev_err(NULL, "Unable to find rpud_dev from mbox_work\n");
		return;
	}

	/* Read command from mailbox */
	ret = visp_mbox_apu_read(rpu);
	if (ret != 0) {
		dev_err(rpu->dev, "%s:Failed to read mailbox command:%d\n",
			__func__, rpu->rpu_id);
		return;
	}
}

static void visp_mbox_tx_done(struct mbox_client *cl, void *msg, int r)
{
}

static void visp_mbox_rx_cb(struct mbox_client *cl, void *msg)
{
	/* Get RPU id from the received message */
	struct rpu_dev *rpu = dev_get_drvdata(cl->dev);
	if (rpu != NULL) {
		/* Schedule work to handle the received event */
		// if ( queue_work_on(1, system_wq, &rpu->mbox_work) )
		if (work_pending(&rpu->mbox_work))
			pr_err("Work already pending for RPU id %d\n",
			       rpu->rpu_id);
		else
			queue_work(rpu->visp_mbox_evt_wq, &rpu->mbox_work);
	} else {
		dev_err(rpu->dev, "%s:Failed to find rpu_dev for rpu_id:%d\n",
			__func__, rpu->rpu_id);
	}
}

static int visp_mbox_setup(struct rpu_dev *rpu, struct device_node *node)
{
	struct mbox_client *mclient;

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

	/* Alloc Work_Queue */
	rpu->visp_mbox_evt_wq = alloc_workqueue("visp_mbox_evt_wq",
						WQ_UNBOUND, 0);
	if (!rpu->visp_mbox_evt_wq) {
		dev_err(rpu->dev, "Failed to create visp_mbox_evt_wq\n");
		return -ENOMEM;
	}

	/* Initialize work */
	INIT_WORK(&rpu->mbox_work, visp_mbox_event_notified);
	dev_dbg(rpu->dev, "Mailbox work handler initialized.\n");

	/* Request TX channel */
	rpu->tx_chan = mbox_request_channel_byname(&rpu->tx_mc, "tx");
	if (IS_ERR(rpu->tx_chan)) {
		dev_err(rpu->dev, "Failed to request TX mailbox channel.\n");
		rpu->tx_chan = NULL;
		return PTR_ERR(rpu->tx_chan);
	}

	/* Request RX channel */
	rpu->rx_chan = mbox_request_channel_byname(&rpu->rx_mc, "rx");
	if (IS_ERR(rpu->rx_chan)) {
		dev_err(rpu->dev, "Failed to request RX mailbox channel.\n");
		rpu->rx_chan = NULL;
		return PTR_ERR(rpu->rx_chan);
	}

	INIT_KFIFO(rpu->ack_fifo);
	INIT_KFIFO(rpu->data_fifo);
	INIT_KFIFO(rpu->app_fifo);

	/* Initialize TX mailbox client SKB queue */
	skb_queue_head_init(&rpu->tx_mc_skbs);

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
	int ret = 0;
	/* Validate input buffer size */
	if (lbuf < sizeof(payload_user_packet)) {
		pr_err("%s: Insufficient input buffer size\n", __func__);
		return -EINVAL;
	}
	/* Validate RPU instance */
	if (!rpu) {
		pr_err("%s: RPU instance is NULL\n", __func__);
		kfree(packet);
		kfree(user_packet);
		return -EINVAL;
	}

	/* Allocate memory for user_packet */
	user_packet = kmalloc(sizeof(payload_user_packet), GFP_KERNEL);
	if (!user_packet) {
		pr_err("%s: Failed to allocate memory for user_packet\n",
		       __func__);
		return -ENOMEM;
	}
	/* Allocate memory for packet */
	packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		pr_err("%s: Failed to allocate memory for packet\n", __func__);
		kfree(packet);
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
	memcpy(packet->payload, user_packet->payload,
	       sizeof(user_packet->payload));

	/* Extract instance_id from payload */
	p_data = packet->payload;
	memcpy(&instance_id, p_data, sizeof(uint32_t));

	mutex_lock(&rpu->write_lock); // Replaced spinlock with mutex
	/* Send command and message */
	ret = visp_mbox_send_command(user_packet->cmd_id, packet,
				     sizeof(payload_packet), rpu->core_id,
				     MBOX_CORE_APU);

	if (ret < 0) {
		pr_err("%s: send_command failed with error: %d\n", __func__,
		       ret);
		kfree(packet);
		kfree(user_packet);
		return -EIO;
	}

	ret = mbox_send_message(rpu->tx_chan, NULL); // Trigger irq
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
	size_t size;
	int result;

	if (!rpu) {
		pr_err("%s: Invalid RPU device pointer\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&rpu->ack_lock);

	rpu->app_wait_flag = 1;
	/* Acquire lock to protect RPU structure */
	/* Wait for completion interruptibly */
	result = wait_for_completion_interruptible(&rpu->mailbox_completion);
	if (result != 0) {
		pr_err("%s: Completion wait interrupted, error code: %d\n",
		       __func__, result);
		bytes_copied = -ERESTARTSYS; // Return appropriate error for
				     // interruption
		goto ERROR;
	}

	if (!kfifo_out(&rpu->app_fifo, &msg, 1)) {
		pr_err("Failed to queue into kfifo\n");
		bytes_copied = -ENOMSG;
		goto ERROR;
	}

	rpu->visp_mbox_app_data->cmdid = msg->msg_id;
	rpu->visp_mbox_app_data->data = msg->payload;
	size = msg->size;
	memcpy(&rpu->visp_mbox_app_data->res_payload_pkt, msg->payload,
	       ALIGN(size, 8));

	/* Copy data to user space */
	if (copy_to_user(buf, rpu->visp_mbox_app_data, bytes_copied)) {
		pr_err("%s: Failed to copy data to user space\n", __func__);
		bytes_copied = -EFAULT; // Handle the copy_to_user error
		goto ERROR;
	}
ERROR:
	mutex_unlock(&rpu->ack_lock);
	mutex_unlock(&rpu->write_lock);
	return bytes_copied; // Return the size of data copied on success
}

static const struct file_operations visp_mbox_rpudev_fops = {
	.owner		= THIS_MODULE,
	.read		= visp_mbox_rpudev_read,
	.write		= visp_mbox_rpudev_write,
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
		dev_warn(rpu->dev, "%s: RPU devices list is empty.\n",
			 __func__);
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

static struct rpu_dev *visp_mbox_get_or_create_rpu(struct platform_device *pdev,
						   int rpu_id)
{

	struct rpu_dev *rpu;
	struct device_node *node;
	char dev_name[20];
	int ret;

	node = pdev->dev.of_node;
	mutex_lock(&rpu_list_lock);

	/* Look for an existing RPU with the given rpu_id */
	list_for_each_entry(rpu, &rpu_devices, node) {
		if (rpu->rpu_id == rpu_id) {
			kref_get(&rpu->refcount);
			//      dev_dbg(&pdev->dev, "Found existing RPU with id
			//      %d\n", rpu_id);
			mutex_unlock(&rpu_list_lock);
			return rpu;
		}
	}

	/* Create a new RPU */
	rpu = devm_kzalloc(&pdev->dev, sizeof(*rpu), GFP_KERNEL);
	if (!rpu) {
		dev_err(&pdev->dev, "Failed to allocate memory for RPU\n");
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(-ENOMEM);
	}

	snprintf(dev_name, sizeof(dev_name), "%s_%d", CHAR_DEV_NAME, rpu_id);

	rpu->dev = &pdev->dev;
	rpu->rpu_id = rpu_id;
	rpu->core_id = rpu_id - VISP_MBOX_RPU6;
	/* Initialize mutexes, cdev, and reference count */

	mutex_init(&rpu->rpu_lock);
	mutex_init(&rpu->ack_lock);
	mutex_init(&rpu->read_lock);
	mutex_init(&rpu->write_lock);
	cdev_init(&rpu->cdev, &visp_mbox_rpudev_fops);
	rpu->cdev.owner = THIS_MODULE;

	/* Allocate a device number */
	ret = alloc_chrdev_region(&rpu->devno, rpu_id, 1, dev_name);
	if (ret) {
		dev_err(&pdev->dev,
			"Failed to allocate device number (error %d)\n", ret);
		devm_kfree(&pdev->dev, rpu);
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(ret);
	}

	rpu->visp_mbox_intr_data = kzalloc(sizeof(struct response_user_packet), GFP_KERNEL);
	if (!rpu->visp_mbox_intr_data) {
		pr_err("Failed to allocate memory\n");
		devm_kfree(&pdev->dev, rpu);
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(-ENOMEM);
	}
	rpu->msg = kzalloc(sizeof(mbox_post_msg), GFP_KERNEL);
	if (!rpu->msg) {
		pr_err("Failed to allocate memory\n");
		devm_kfree(&pdev->dev, rpu);
		kfree(rpu->visp_mbox_intr_data);
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(-ENOMEM);
	}
	rpu->visp_mbox_app_data = kzalloc(sizeof(struct response_user_packet), GFP_KERNEL);
	if (!rpu->visp_mbox_app_data) {
		pr_err("Failed to allocate memory\n");
		devm_kfree(&pdev->dev, rpu);
		kfree(rpu->visp_mbox_intr_data);
		kfree(rpu->msg);
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(-ENOMEM);
	}
	rpu->visp_mbox_apu_data = kzalloc(sizeof(struct response_user_packet), GFP_KERNEL);
	if (!rpu->visp_mbox_apu_data) {
		pr_err("Failed to allocate memory\n");
		devm_kfree(&pdev->dev, rpu);
		kfree(rpu->visp_mbox_intr_data);
		kfree(rpu->msg);
		kfree(rpu->visp_mbox_app_data);
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(-ENOMEM);
	}

	/* Add the character device */
	ret = cdev_add(&rpu->cdev, rpu->devno, 1);
	if (ret) {
		dev_err(&pdev->dev, "Failed to add cdev (error %d)\n", ret);
		unregister_chrdev_region(rpu->devno, 1);
		devm_kfree(&pdev->dev, rpu);
		kfree(rpu->visp_mbox_intr_data);
		kfree(rpu->msg);
		kfree(rpu->visp_mbox_app_data);
		kfree(rpu->visp_mbox_apu_data);
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(ret);
	}

	/* Create a device node */
	if (!device_create(mailbox_class, NULL, rpu->devno, NULL, dev_name)) {
		dev_err(&pdev->dev, "Failed to create device node\n");
		cdev_del(&rpu->cdev);
		unregister_chrdev_region(rpu->devno, 1);
		devm_kfree(&pdev->dev, rpu);
		kfree(rpu->visp_mbox_intr_data);
		kfree(rpu->msg);
		kfree(rpu->visp_mbox_app_data);
		kfree(rpu->visp_mbox_apu_data);
		mutex_unlock(&rpu_list_lock);
		return ERR_PTR(-ENOMEM);
	}

	kref_init(&rpu->refcount);

	/* Add the new RPU to the list */
	list_add_tail(&rpu->node, &rpu_devices);

	/* Setup mailbox if required */
	if (of_property_read_bool(node, "mboxes")) {
		dev_dbg(&pdev->dev, "Setting up mailboxes for RPU with id %d\n",
			rpu_id);
		ret = visp_mbox_setup(rpu, node);
		if (ret) {
			dev_err(&pdev->dev,
				"Failed to set up mailboxes (error %d)\n", ret);
			device_destroy(mailbox_class, rpu->devno);
			cdev_del(&rpu->cdev);
			unregister_chrdev_region(rpu->devno, 1);
			devm_kfree(&pdev->dev, rpu);
			kfree(rpu->visp_mbox_intr_data);
			kfree(rpu->msg);
			kfree(rpu->visp_mbox_app_data);
			kfree(rpu->visp_mbox_apu_data);
			mutex_unlock(&rpu_list_lock);
			return ERR_PTR(ret);
		}
	}

	mutex_unlock(&rpu_list_lock);
	return rpu;
}

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
	//struct response_user_packet *visp_mbox_intr_data;
	struct rpu_dev *rpu = NULL;
	mbox_post_msg *msg;
	size_t size;
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

	long timeout_jiffies =
	    msecs_to_jiffies(1000 * 60 * 5); // Timeout of 5000ms (5 seconds)
	result = wait_for_completion_interruptible_timeout(
	    &isp_dev->apu_wait_for_data, timeout_jiffies);
	if (result == 0) {
		dev_err(rpu->dev,
			"Timeout occurred while waiting for APU ACK\n");
		ret = -ETIMEDOUT;
		goto ERROR;
	} else if (result < 0) {
		dev_err(rpu->dev, "Wait interrupted\n");
		ret = -EINVAL;
		goto ERROR;
	}

	if (!kfifo_out(&rpu->data_fifo, &msg, 1)) {
		pr_err("Failed to queue into kfifo\n");
		ret = -ENOMEM;
		goto ERROR;
	}

	size = msg->size;
	rpu->visp_mbox_apu_data->cmdid = msg->msg_id;
	rpu->visp_mbox_apu_data->data = msg->payload;
	memcpy(&rpu->visp_mbox_apu_data->res_payload_pkt, msg->payload,
	       ALIGN(size, 8) /*sizeof(payload_packet)*/);
	/* Copy the received data */
	memcpy(data, &rpu->visp_mbox_apu_data->res_payload_pkt.payload,
	       sizeof(rpu->visp_mbox_apu_data->res_payload_pkt.payload));

	ret = rpu->visp_mbox_apu_data->res_payload_pkt.resp_field.error_subcode_t;
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

	if (!isp_dev || !isp_dev->rpu || !data) {
		pr_err("%s: Invalid input parameters\n", __func__);
		return -EINVAL;
	}

	rpu = isp_dev->rpu;

	mutex_lock(&rpu->write_lock);

	result = visp_mbox_send_command(cmd, data, size, rpu->core_id,
					src_cpu);
	if (result != 0) {
		dev_err(rpu->dev,
			"%s: Mailbox Send message failed at line %d\n",
			__func__, __LINE__);
		goto unlock_and_exit;
	}

	/* Set the data flag */
	isp_dev->k_apu_data_flag = 1;

	result = mbox_send_message(isp_dev->tx_chan, NULL);
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

unlock_and_exit:
	mutex_unlock(&rpu->write_lock);
	return result;
}
EXPORT_SYMBOL(xlnx_send_mbox_data_cmd);

int xlnx_send_mbox_without_ack_cmd(struct visp_dev *isp_dev, mb_cmd_id_e cmd,
				   void *data, uint32_t size, uint8_t dest_cpu,
				   uint8_t src_cpu)
{
	int result;
	struct rpu_dev *rpu;

	if (!isp_dev || !isp_dev->rpu) {
		pr_err("%s: Invalid ISP device\n", __func__);
		return -EINVAL;
	}
	rpu = isp_dev->rpu;
	result = visp_mbox_send_command(cmd, data, size, rpu->core_id,
					src_cpu);
	if (result != 0) {
		dev_err(rpu->dev,
			"%s: Mailbox Send message failed at line %d\n",
			__func__, __LINE__);
		mutex_unlock(&rpu->write_lock);
		return result;
	}

	// result = mbox_send_message(isp_dev->tx_chan, NULL);
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

	if (!isp_dev || !isp_dev->rpu) {
		pr_err("%s: Invalid ISP device\n", __func__);
		return -EINVAL;
	}

	rpu = isp_dev->rpu;

	mutex_lock(&rpu->write_lock);

	result = visp_mbox_send_command(cmd, data, size, rpu->core_id,
					src_cpu);
	if (result != 0) {
		dev_err(rpu->dev,
			"%s: Mailbox Send message failed at line %d\n",
			__func__, __LINE__);
		mutex_unlock(&rpu->write_lock);
		return result;
	}
	// Prepare for ACK
	reinit_completion(&isp_dev->apu_wait_for_ack);

	/* Set acknowledgment flag */
	isp_dev->k_apu_ack_flag = 1;

	// result = mbox_send_message(isp_dev->tx_chan, NULL);
	result = mbox_send_message(rpu->tx_chan, NULL);
	if (result < 0) {
		dev_err(rpu->dev, "%s: mbox_send_message failed at line %d\n",
			__func__, __LINE__);
		mutex_unlock(&rpu->write_lock);
		return result;
	}

	result = xlnx_mbox_apu_wait_for_ack(isp_dev);
	if (result < 0) {
		dev_err(rpu->dev, "%s: Failed to get ack\n", __func__);
		mutex_unlock(&rpu->write_lock);
		goto unlock_and_exit;
	}

unlock_and_exit:
	mutex_unlock(&rpu->write_lock);
	return result; // Success
}
EXPORT_SYMBOL(xlnx_send_mbox_acked_cmd);

uint8_t xlnx_mbox_apu_wait_for_ack(struct visp_dev *isp_dev)
{
	struct rpu_dev *rpu = NULL;
	long result;
	mbox_post_msg *msg;
	size_t size;
	int ret;

	if (!isp_dev) {
		pr_err("xlnx_mbox_apu_wait_for_ack: Invalid ISP device (NULL "
		       "pointer).\n");
		ret = -EINVAL;
		goto ERROR;
	}
	rpu = isp_dev->rpu;
	if (!rpu) {
		dev_err(isp_dev->dev, "RPU device is NULL in %s.\n", __func__);
		ret = -EINVAL;
		goto ERROR;
	}

	/* Wait for acknowledgment completion */
	long timeout_jiffies =
	    msecs_to_jiffies(1000 * 60 * 5); // Timeout of 5000ms (5 seconds)
	result = wait_for_completion_interruptible_timeout(
	    &isp_dev->apu_wait_for_ack, timeout_jiffies);
	if (result == 0) {
		dev_err(rpu->dev,
			"Timeout occurred while waiting for APU ACK\n");
		ret = -ETIMEDOUT;
		goto ERROR;
	} else if (result < 0) {
		dev_err(rpu->dev, "Wait interrupted\n");
		ret = -1;
		goto ERROR;
	}

	if (!kfifo_out(&rpu->ack_fifo, &msg, 1)) {
		pr_err("Failed to queue into kfifo\n");
		ret = -ENOMSG;
	}

	if (!msg) {
		pr_err("Received invalid message or payload\n");
		ret = -EINVAL;
		goto ERROR;
	}

	rpu->visp_mbox_intr_data->cmdid = msg->msg_id;
	rpu->visp_mbox_intr_data->data = msg->payload;
	size = msg->size;
	memcpy(&rpu->visp_mbox_intr_data->res_payload_pkt, msg->payload,
	       ALIGN(size, 8));

	return rpu->visp_mbox_intr_data->res_payload_pkt.resp_field.error_subcode_t;
ERROR:
	return ret;

}
EXPORT_SYMBOL(xlnx_mbox_apu_wait_for_ack);

static int visp_mbox_mailbox_initialization(struct rpu_dev *rpu)
{
	int ret;

	if (!rpu) {
		pr_err("%s: Invalid RPU device NULLpointer).\n", __func__);
		return -EINVAL;
	}

	/* Initialize reserved memory */
	ret = visp_mbox_reserved_memory_init("isp_mbox_buffer");
	if (ret) {
		dev_err(rpu->dev,
			"Failed to initialize reserved memory. Error: %d\n",
			ret);
		return ret;
	}

	/* Initialize mailbox with reserved memory */
	visp_mbox_mailbox_init(rpu, MBOX_CORE_APU,
			       (uintptr_t)reserved_memory.virt_addr,
			       (uintptr_t)reserved_memory.phys_addr);

	/* Trigger an inter-processor interrupt (IPI) */
	ret = mbox_send_message(rpu->tx_chan, NULL);
	if (ret < 0) {
		dev_err(rpu->dev, "Failed to trigger IPI. Error: %d\n", ret);
		return ret;
	}

	return 0;
}

static void visp_mbox_rpu_cleanup(struct kref *ref)
{
	//   struct rpu_dev *rpu = container_of(ref, struct rpu_dev, refcount);
	//        devm_kfree(&rpu->pdev->dev, rpu);
}
static int visp_mbox_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rpu_dev *rpu = NULL;
	int ret;
	struct device_node *node;
	int rpu_id = -1;

	node = pdev->dev.of_node;

	/* Read the rpu_id property */
	ret = of_property_read_u32(node, "rpu_id", &rpu_id);
	if (ret) {
		dev_err(dev, " Failed to read rpu_id from device tree\n");
		return -EINVAL;
	}

	/* Log rpu_id for debug purposes */
	dev_dbg(dev, "rpu_id read from device tree: %u\n", rpu_id);

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

	init_completion(&rpu->mailbox_completion);

	/* Store the RPU pointer in the platform device's driver data */
	platform_set_drvdata(pdev, rpu);

	/* Initialize the mailbox */
	ret = visp_mbox_mailbox_initialization(rpu);
	if (ret) {
		dev_err(dev, "Failed to initialize mailbox for RPU id: %d\n",
			rpu_id);
		 /* Clear driver data on failure */
		platform_set_drvdata(pdev, NULL);
		return ret;
	}

	dev_info(dev, "Mailbox successfully initialized for RPU id: %d\n",
		 rpu_id);
	return 0;
}

static void visp_mbox_rpu_remove(void)
{
	struct rpu_dev *rpu, *tmp;

	/* Lock the RPU list to prevent concurrent access */
	mutex_lock(&rpu_list_lock);

	/* Iterate through the list of RPU devices */
	list_for_each_entry_safe(rpu, tmp, &rpu_devices, node) {
		if (!rpu) {
			/* Handle NULL RPU device (shouldn't normally happen) */
			pr_err("Encountered a NULL RPU device in the list, "
			       "skipping.\n");
			continue;
		}

		/* Debug log the current RPU's reference count */
		dev_dbg(rpu->dev, "RPU id %d - refcount = %d\n", rpu->rpu_id,
			atomic_read(&rpu->refcount.refcount.refs));

		/* Check if the device is safe to remove */
		if (atomic_read(&rpu->refcount.refcount.refs) == 1) {
			/* Perform cleanup and removal */
			kfree(rpu->visp_mbox_intr_data);
			kfree(rpu->msg);
			kfree(rpu->visp_mbox_app_data);
			kfree(rpu->visp_mbox_apu_data);

			device_destroy(mailbox_class, rpu->devno);
			cdev_del(&rpu->cdev);
			unregister_chrdev_region(rpu->devno, 1);

			/* Free resources using the kref_put mechanism */
			kref_put(&rpu->refcount, visp_mbox_rpu_cleanup);

			/* Remove the RPU from the list */
			list_del(&rpu->node);

			/* Log successful removal */
			dev_info(rpu->dev,
				 "RPU with id %d removed successfully.\n",
				 rpu->rpu_id);
		} else {
			/* If still in use, decrement reference and log */
			kref_put(&rpu->refcount, visp_mbox_rpu_cleanup);
			dev_warn(rpu->dev,
				 "Cannot remove RPU with id %d - still in use "
				 "(refcount = "
				 "%d).\n",
				 rpu->rpu_id,
				 atomic_read(&rpu->refcount.refcount.refs));
		}
	}

	/* Unlock the RPU list */
	mutex_unlock(&rpu_list_lock);
}

static void visp_mbox_remove(struct platform_device *pdev)
{
	struct rpu_dev *rpu;

	/* Retrieve the RPU device associated with this platform device */
	rpu = platform_get_drvdata(pdev);
	if (!rpu) {
		dev_err(&pdev->dev,
			"Failed to retrieve RPU device during remove.\n");
		return;
	}

	/* Check and free mailbox channels if defined in the device tree */
	if (of_property_read_bool(rpu->dev->of_node, "mboxes")) {
		dev_info(rpu->dev, "Freeing mailbox channels.\n");
		if (rpu->tx_chan) {
			mbox_free_channel(rpu->tx_chan);
			rpu->tx_chan = NULL; // Avoid dangling pointers
		}
		if (rpu->rx_chan) {
			mbox_free_channel(rpu->rx_chan);
			rpu->rx_chan = NULL; // Avoid dangling pointers
		}
	} else {
		dev_dbg(rpu->dev, "No mailbox channels to free.\n");
	}

	/* Clean up reserved memory structure */
	visp_mbox_reserved_memory_exit();
	dev_dbg(&pdev->dev, "Reserved memory cleaned up.\n");

	/* Call rpu_remove to handle RPU-specific cleanup */
	visp_mbox_rpu_remove();

	return;
}

static const struct of_device_id visp_mbox_of_match[] = {
	{.compatible = "xlnx,mimo-mbox"},
	{.compatible = "xlnx,mbox"},
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
		}};

static int __init visp_mbox_init_module(void)
{
	int ret;

	pr_info("Initializing AMD MBox driver.\n");

	/* Create the mailbox class */
	// mailbox_class = class_create(THIS_MODULE, "mailbox-class");
	mailbox_class = class_create("mailbox-class");
	if (IS_ERR(mailbox_class)) {
		pr_err("Failed to create mailbox class.\n");
		return PTR_ERR(mailbox_class);
	}

	/* Register the platform driver */
	ret = platform_driver_register(&visp_mbox_driver);
	if (ret) {
		pr_err("Failed to register AMD MBox driver. Error: %d\n", ret);
		class_destroy(mailbox_class);
		return ret;
	}

	pr_info("AMD MBox driver registered successfully.\n");
	return 0;
}

static void __exit visp_mbox_exit_module(void)
{
	pr_info("Exiting AMD MBox driver.\n");

	/* Cleanup RPU devices */
	visp_mbox_rpu_remove();

	/* Unregister the platform driver */
	platform_driver_unregister(&visp_mbox_driver);
	pr_info("AMD MBox driver unregistered successfully.\n");

	/* Destroy the mailbox class */
	if (mailbox_class) {
		class_destroy(mailbox_class);
		pr_info("Mailbox class destroyed successfully.\n");
	}
}

module_init(visp_mbox_init_module);
module_exit(visp_mbox_exit_module);

MODULE_DESCRIPTION("AMD MBOX driver");
MODULE_AUTHOR("Ajay Kumar Nandam anandam@amd.com");
MODULE_AUTHOR("Anil Mamidala amamidal@amd.com");
MODULE_LICENSE("GPL");
