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

#include "mbox_cmd.h"
#include "mbox_api.h"
#include "mbox_error_code.h"
#include <linux/gfp.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "sensor_cmd.h"
#include <linux/kernel.h>
#include <linux/ktime.h>

#include "visp_driver.h"
#include "visp_mbox_driver.h"

#define XPAR_PS_0_PSPMC_0_PSV_IPI_0_BIT_MASK 0x00000004U
#define XPAR_PS_0_PSPMC_0_PSV_IPI_1_BIT_MASK 0x00000008U

/* Default IPI SMC function IDs */
#define SMC_IPI_MAILBOX_OPEN 0x82001000U
#define SMC_IPI_MAILBOX_RELEASE 0x82001001U
#define SMC_IPI_MAILBOX_STATUS_ENQUIRY 0x82001002U
#define SMC_IPI_MAILBOX_NOTIFY 0x82001003U
#define SMC_IPI_MAILBOX_ACK 0x82001004U
#define SMC_IPI_MAILBOX_ENABLE_IRQ 0x82001005U
#define SMC_IPI_MAILBOX_DISABLE_IRQ 0x82001006U

#define DEBUG_ENABLE_LOG

struct response_packet {
	u32 cmdid;
	u32 cookie;
	u32 error_subcode;
};

uint32_t write_mboxcmd(uint32_t cmd_id, void *struct_msg, uint16_t size,
		       uint32_t flag, mbox_core_id receiver_id,
		       mbox_core_id core_id)
{
	int ret;
	mbox_post_msg *msg = NULL;
	struct rpu_dev *rpu;
	rpu = visp_mbox_get_rpu_dev(receiver_id + VISP_MBOX_RPU6);
	if (!rpu)
		return -EINVAL;

	if (!rpu->tx_msg_cache)
		return -EINVAL;

	msg = kmem_cache_zalloc(rpu->tx_msg_cache, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;
	if (size == 0) {
		msg->msg_id = cmd_id;
	} else {
		msg->msg_id = cmd_id;
		msg->media_server_flags = flag;
		msg->size = sizeof(payload_packet) - MAX_ITEM +
			    ((payload_packet *)struct_msg)->payload_size;
		memcpy(msg->payload, struct_msg, ALIGN(msg->size, 8));
	}
	if (core_id != MBOX_CORE_APU)
		core_id = MBOX_CORE_APU;

	ret = vpi_mbox_post(rpu->apu_tx_ctrl, msg, receiver_id, NULL);

	kmem_cache_free(rpu->tx_msg_cache, msg);

	if (ret)
		return ret;

	return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(write_mboxcmd);

int visp_mbox_apu_read(struct rpu_dev *rpu)
{
	struct visp_dev *isp_dev;
	uint32_t cmd, isp_id;
	int ret = 0;
	mbox_post_msg *msg = NULL;
	bool msg_enqueued = false;

	if (!rpu || !rpu->rx_msg_cache)
		return -EINVAL;

	if (vpi_mbox_is_empty(rpu->apu_rx_ctrl, rpu->core_id, MBOX_CORE_APU)) {
		if (mutex_is_locked(&rpu->write_lock)) {
			mutex_unlock(&rpu->write_lock);
		}
		(void)mbox_send_message(rpu->rx_chan, NULL);
		pr_err("No message in shared memory for rpu_id %d!\n", rpu->rpu_id);
		return -ENOMSG;
	}

	msg = kmem_cache_zalloc(rpu->rx_msg_cache, GFP_ATOMIC);
	if (!msg)
		return -ENOMEM;

	mutex_lock(&rpu->read_lock);
	vpi_mbox_read(rpu->apu_rx_ctrl, msg, rpu->core_id);
	mutex_unlock(&rpu->read_lock);

	memcpy(&isp_id, ((payload_packet *)msg->payload)->payload, sizeof(uint32_t));

	/* Normalize ISP instance ID; clarify why divided by 15 */
	isp_id = isp_id / 15;
	if (isp_id < 0 || isp_id >= MAX_ISP_INSTANCES) {
		dev_err(rpu->dev, "%s: Invalid isp_id %u\n", __func__,
			isp_id);
		ret = -EINVAL;
		goto ERROR;
	}

	isp_dev = rpu->isp_dev[isp_id];
	if (!isp_dev) {
		pr_err("%s: Invalid or uninitialized isp_dev for isp_id = %d\n", __func__, isp_id);
		ret = -EINVAL;
		goto ERROR;
	}

	cmd = msg->msg_id;
	/* Handle specific commands */
	if (msg->media_server_flags == 1) {
		if (cmd == MB_CMD_RES_SUCCESS) {
			if (!kfifo_in(&rpu->app_fifo, &msg, 1)) {
				pr_err("Failed to queue into kfifo\n");
				ret = -ENOMEM;
				goto ERROR;
			}
			msg_enqueued = true;
			complete(&rpu->mailbox_completion);
			goto DONE;
		} else {
			dev_err(rpu->dev, "%s: Unexpected command id %d received\n",
				__func__, cmd);
			ret = -EINVAL;
			goto ERROR;
		}
	} else {
		if (cmd == MB_CMD_RES_SUCCESS) {
			if (!kfifo_in(&rpu->ack_fifo, &msg, 1)) {
				pr_err("Failed to queue into apu ack kfifo\n");
				ret = -ENOMEM;
				goto ERROR;
			}
			msg_enqueued = true;
			complete(&isp_dev->apu_wait_for_ack);
			goto DONE;
		} else if (cmd == MB_CMD_GET_SUCCESS) {
			if (!kfifo_in(&rpu->data_fifo, &msg, 1)) {
				pr_err("Failed to queue into apu data kfifo\n");
				ret = -ENOMEM;
				goto ERROR;
			}
			msg_enqueued = true;
			complete(&isp_dev->apu_wait_for_data);
			goto DONE;
		} else if (cmd == RPU_2_APU_CMD_DISPLAY_BUFFER) {
			if (isp_dev->frameout_cb) {
				if (!kfifo_in(&isp_dev->display_fifo, &msg, 1)) {
					pr_err("Failed to queue into display kfifo\n");
					ret = -ENOMEM;
					goto ERROR;
				}
				msg_enqueued = true;
			isp_dev->frameout_cb(isp_dev);
			goto DISP_DONE;
			} else {
				pr_err("%s %d CALLBACK IS NULL\n", __func__, __LINE__);
				ret = -EINVAL;
				goto ERROR;
			}
		} else {
			dev_err(rpu->dev, "%s: Unexpected command id %d received\n",
				__func__, cmd);
			ret = -EINVAL;
			goto ERROR;
		}
	}
	/* Send a message to the RX mailbox channel */
ERROR:
DONE:
	if (!msg_enqueued && msg)
		kmem_cache_free(rpu->rx_msg_cache, msg);
DISP_DONE:
	return ret;
}
EXPORT_SYMBOL_GPL(visp_mbox_apu_read);

int visp_mbox_mailbox_init(struct rpu_dev *rpu, u32 cpu, uint64_t MBOX_FIFO_START_ADDR,
			   uint64_t mbox_fifo_start_addr_phy)
{
	if (!rpu) {
		pr_err("%s: Invalid RPU device (NULL pointer).\n", __func__);
		return -EINVAL;
	}

	if (MBOX_FIFO_START_ADDR == 0) {
		pr_err("%s: Invalid virtual address (NULL).\n", __func__);
		return -EINVAL;
	}

	if (mbox_fifo_start_addr_phy == 0) {
		pr_err("%s: Invalid physical address (NULL).\n", __func__);
		return -EINVAL;
	}

	rpu->apu_rx_ctrl = visp_mbox_init(rpu->core_id,
					  MBOX_CORE_APU,
					  MBOX_FIFO_START_ADDR,
					  mbox_fifo_start_addr_phy,
					  MBOX_FIFO_BLOCK_SIZE);
	if (!rpu->apu_rx_ctrl) {
		pr_err("%s: Failed to initialize APU RX control.\n", __func__);
		return -ENOMEM;
	}

	rpu->apu_tx_ctrl = visp_mbox_init(MBOX_CORE_APU, rpu->core_id,
					  MBOX_FIFO_START_ADDR,
					  mbox_fifo_start_addr_phy,
					  MBOX_FIFO_BLOCK_SIZE);
	if (!rpu->apu_tx_ctrl) {
		pr_err("%s: Failed to initialize APU TX control.\n", __func__);
		kfree(rpu->apu_rx_ctrl);
		rpu->apu_rx_ctrl = NULL;
		return -ENOMEM;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(visp_mbox_mailbox_init);

void mailbox_close(struct rpu_dev *rpu)
{
	kfree(rpu->apu_tx_ctrl);
	kfree(rpu->apu_rx_ctrl);
}

int visp_mbox_send_command(mb_cmd_id_e cmd, void *data, uint32_t size,
			   uint32_t flag, uint8_t dest_cpu, uint8_t src_cpu)
{
	int ret = 0;

	ret = write_mboxcmd(cmd, data, size, flag, dest_cpu, src_cpu);
	return ret;
}
EXPORT_SYMBOL_GPL(visp_mbox_send_command);

MODULE_AUTHOR("anandam");
MODULE_DESCRIPTION("MBOX_CMD");
MODULE_LICENSE("GPL");
