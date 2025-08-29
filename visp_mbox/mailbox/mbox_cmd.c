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

uint32_t flag;
#define DEBUG_ENABLE_LOG

struct response_packet {
	u32 cmdid;
	u32 cookie;
	u32 error_subcode;
};

void apu_postmsg(mbox_core_id receiver_id)
{
/*
 * if (MBOX_CORE_RPU0 == receiver_id) {
 *		// Status = trigger_ipi(&IpiInst_apu_rpu,MBOX_CORE_RPU0);
 *	}
 *
 *	if (MBOX_CORE_RPU1 == receiver_id) {
 *		//		Status =
 *		// trigger_ipi(&IpiInst_apu_rpu,MBOX_CORE_RPU1);
 *	}
 */
}

uint32_t write_mboxcmd(uint32_t cmd_id, void *struct_msg, uint16_t size,
		       mbox_core_id receiver_id, mbox_core_id core_id)
{
	int ret;
	mbox_post_msg *msg = NULL;
	struct rpu_dev *rpu;

	msg = (mbox_post_msg *)kzalloc(sizeof(mbox_post_msg), GFP_KERNEL);
	if (msg == NULL) {
		pr_err("%s %d Failed to allocate memory\n", __func__, __LINE__);
		return VPI_ERR_NOMEM;
	}

	if (size == 0) {
		msg->msg_id = cmd_id;
	} else {
		msg->msg_id = cmd_id;
		msg->size = sizeof(payload_packet) - MAX_ITEM +
			    ((payload_packet *)struct_msg)->payload_size;
		memcpy(msg->payload, struct_msg, ALIGN(msg->size, 8));
	}

	rpu = visp_mbox_get_rpu_dev(receiver_id + VISP_MBOX_RPU6);
	if (!rpu)
		return -EINVAL;

	if (core_id != MBOX_CORE_APU)
		core_id = MBOX_CORE_APU;

	ret = vpi_mbox_post(rpu->apu_tx_ctrl, msg, receiver_id, apu_postmsg);

	kfree(msg);

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

	if (vpi_mbox_is_empty(rpu->apu_rx_ctrl, rpu->core_id, MBOX_CORE_APU)) {
		if (mutex_is_locked(&rpu->write_lock)) {
			mutex_unlock(&rpu->write_lock);
		}
		(void)mbox_send_message(rpu->rx_chan, NULL);
		pr_err("No message in shared memory for rpu_id %d!\n", rpu->rpu_id);
		return -ENOMSG;
	}

	mutex_lock(&rpu->read_lock);
	vpi_mbox_read(rpu->apu_rx_ctrl, rpu->msg, rpu->core_id);

	memcpy(&isp_id, ((payload_packet *)rpu->msg->payload)->payload, sizeof(uint32_t));

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

	cmd = rpu->msg->msg_id;
	/* Handle specific commands */
	if (isp_dev->k_apu_ack_flag && cmd == MB_CMD_RES_SUCCESS) {
		if (!kfifo_in(&rpu->ack_fifo, &rpu->msg, 1)) {
			pr_err("Failed to queue into apu ack kfifo\n");
			ret = -ENOMEM;
			goto ERROR;
		}
		isp_dev->k_apu_ack_flag = 0;
		complete(&isp_dev->apu_wait_for_ack);
		goto DONE;
	} else if (isp_dev->k_apu_data_flag && cmd == MB_CMD_GET_SUCCESS) {
		if (!kfifo_in(&rpu->data_fifo, &rpu->msg, 1)) {
			pr_err("Failed to queue into apu data kfifo\n");
			ret = -ENOMEM;
			goto ERROR;
		}
		isp_dev->k_apu_data_flag = 0;
		complete(&isp_dev->apu_wait_for_data);
		goto DONE;
	} else if ((rpu->app_wait_flag && cmd == MB_CMD_RES_SUCCESS) ||
		   (rpu->app_wait_flag && cmd == MB_CMD_GET_SUCCESS)) {
		if (!kfifo_in(&rpu->app_fifo, &rpu->msg, 1)) {
			pr_err("Failed to queue into kfifo\n");
			ret = -ENOMEM;
			goto ERROR;
		}
		rpu->app_wait_flag = 0;
		complete(&rpu->mailbox_completion);
		goto DONE;
	} else if (cmd == RPU_2_APU_CMD_DISPLAY_BUFFER) {
		if (isp_dev->frameout_cb) {
			if (!kfifo_in(&isp_dev->display_fifo, &rpu->msg, 1)) {
				pr_err("Failed to queue into display kfifo\n");
				ret = -ENOMEM;
				goto ERROR;
			}
			(void)mbox_send_message(rpu->rx_chan, NULL);
			mutex_unlock(&rpu->read_lock);
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

	/* Send a message to the RX mailbox channel */
ERROR:
DONE:
	(void)mbox_send_message(rpu->rx_chan, NULL);
	mutex_unlock(&rpu->read_lock);
DISP_DONE:
	return ret;
}
EXPORT_SYMBOL_GPL(visp_mbox_apu_read);

void visp_mbox_mailbox_init(struct rpu_dev *rpu, u32 cpu, uint64_t MBOX_FIFO_START_ADDR,
			    uint64_t mbox_fifo_start_addr_phy)
{
		rpu->apu_rx_ctrl = visp_mbox_init(rpu->core_id,
						  MBOX_CORE_APU,
						  MBOX_FIFO_START_ADDR,
						  mbox_fifo_start_addr_phy,
						  MBOX_FIFO_BLOCK_SIZE);

		rpu->apu_tx_ctrl = visp_mbox_init(MBOX_CORE_APU, rpu->core_id,
						  MBOX_FIFO_START_ADDR,
						  mbox_fifo_start_addr_phy,
						  MBOX_FIFO_BLOCK_SIZE);
}
EXPORT_SYMBOL_GPL(visp_mbox_mailbox_init);

void mailbox_close(struct rpu_dev *rpu)
{
	kfree(rpu->apu_tx_ctrl);
	kfree(rpu->apu_tx_ctrl);
}

int visp_mbox_send_command(mb_cmd_id_e cmd, void *data, uint32_t size,
			   uint8_t dest_cpu, uint8_t src_cpu)
{
	int ret = 0;

	ret = write_mboxcmd(cmd, data, size, dest_cpu, src_cpu);
	return ret;
}
EXPORT_SYMBOL_GPL(visp_mbox_send_command);

volatile void *enque_buff_g;
volatile int dq_buf_available;

EXPORT_SYMBOL_GPL(enque_buff_g);
EXPORT_SYMBOL_GPL(dq_buf_available);

struct Chn_info_l {
	int hw_id;
	int mode;
	int vt_id;
	int path;
};

int read_dq_buf_info_l(void *data, media_buffer_t *Enque_Buff_L,
		      struct Chn_info_l *info);

int read_dq_buf_info_l(void *data, media_buffer_t *Enque_Buff_L,
		      struct Chn_info_l *info)
{
	media_buffer_t Display_Mediabuff_G;
	payload_packet *packet = data;
	uint8_t *p_data; // = packet->payload;

	if (!packet) {
		kfree(packet);
		return -ENOMEM;
	}

	p_data = packet->payload;

	// struct Chn_info info;
	memcpy(info, p_data, sizeof(struct Chn_info_l));
	p_data += sizeof(struct Chn_info_l);

	memcpy(Display_Mediabuff_G.p_meta_data, p_data,
	       sizeof(pic_buf_meta_data_t));
	p_data += sizeof(pic_buf_meta_data_t);

	memcpy(&(Display_Mediabuff_G.base_address), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(Display_Mediabuff_G.base_size), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(Display_Mediabuff_G.lock_count), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(Display_Mediabuff_G.is_full), p_data, sizeof(bool_t));
	p_data += sizeof(bool_t);

	memcpy(&(Display_Mediabuff_G.index), p_data, sizeof(uint8_t));
	p_data += sizeof(uint8_t);

	memcpy(&(Display_Mediabuff_G.buf_mode), p_data, sizeof(buff_mode));
	p_data += sizeof(buff_mode);

	memcpy(&(Display_Mediabuff_G.p_ipl_address), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(Display_Mediabuff_G.p_owner), p_data, sizeof(uint32_t));

	Enque_Buff_L->base_address = Display_Mediabuff_G.base_address;
	Enque_Buff_L->base_size = Display_Mediabuff_G.base_size;
	Enque_Buff_L->buf = Display_Mediabuff_G.buf;
	Enque_Buff_L->buf_mode = Display_Mediabuff_G.buf_mode;
	Enque_Buff_L->index = Display_Mediabuff_G.index;
	Enque_Buff_L->is_full = Display_Mediabuff_G.is_full;
	Enque_Buff_L->lock_count = Display_Mediabuff_G.lock_count;
	Enque_Buff_L->p_ipl_address = Display_Mediabuff_G.p_ipl_address;
	Enque_Buff_L->p_meta_data = Display_Mediabuff_G.p_meta_data;
	Enque_Buff_L->p_owner = Display_Mediabuff_G.p_owner;

	return 0;
}

MODULE_AUTHOR("anandam");
MODULE_DESCRIPTION("MBOX_CMD");
MODULE_LICENSE("GPL");
