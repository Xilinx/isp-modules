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

// #include <linux/mbox_api.h>
#include "mbox_cmd.h"

#include "mbox_api.h"
#include "mbox_error_code.h"
// #include <linux/sensor_cmd.h>
#include <linux/arm-smccc.h>
#include <linux/gfp.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "sensor_cmd.h"
// #include "buf_defs.h"
#include <linux/kernel.h>
#include <linux/ktime.h>

#include "visp_driver.h"

struct response_user_packet data_from_interrupt;
EXPORT_SYMBOL_GPL(data_from_interrupt);

// extern int handle_frameout_buffer(void * Enque_Buff_L);
// int (* exported_func)(void *);

typedef void (*kmbox_parse_process_t)(uint16_t cmd, void *data, uint32_t size);
static kmbox_parse_process_t kmbox_parse_process;

static mbox_fifo_ctrl *rpu0_fifo_ctrl;
static mbox_fifo_ctrl *rpu1_fifo_ctrl;
static mbox_fifo_ctrl *rpu2_fifo_ctrl;
static mbox_fifo_ctrl *rpu3_fifo_ctrl;
static mbox_fifo_ctrl *apu_fifo_ctrl;
// static mbox_post_msg *rmsg_rpu0 = NULL;
// static mbox_post_msg *rmsg_rpu1 = NULL;
static mbox_post_msg *rmsg_apu;

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

	msg = (mbox_post_msg *)kzalloc(sizeof(mbox_post_msg), GFP_KERNEL);
	if (msg == NULL) {
		pr_err("%s %d Failed to allocate memory\n", __func__, __LINE__);
		return VPI_ERR_NOMEM;
	}

	if (size == 0) {
		msg->msg_id = cmd_id;
	} else {
		msg->msg_id = cmd_id;
		// msg->size = size;
		//  memcpy(msg->payload, struct_msg, size);
		msg->size = sizeof(payload_packet) - MAX_ITEM +
			    ((payload_packet *)struct_msg)->payload_size;
		memcpy(msg->payload, struct_msg, ALIGN(msg->size, 8));
	}

	if (core_id != 4)
		core_id = 4;
	if (core_id == MBOX_CORE_APU) {
		ret =
		    vpi_mbox_post(apu_fifo_ctrl, msg, receiver_id, apu_postmsg);
	}

	kfree(msg);

	if (ret)
		return ret;

	return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(write_mboxcmd);

uint32_t kmbox_write(int32_t cmd_id, void *msg, uint16_t size,
		     mbox_core_id receiver_id, mbox_core_id core_id)
{
	unsigned long a0, a1, a2, a3;
	struct arm_smccc_res res;
	uint32_t ret;

	ret = write_mboxcmd(cmd_id, msg, size, receiver_id, core_id);

	a0 = SMC_IPI_MAILBOX_NOTIFY;
	a1 = 5; // Local id
	a2 = 3; // Remote id
	a3 = 0;

	arm_smccc_smc(a0, a1, a2, a3, 0, 0, 0, 0, &res);

	return ret;
}
EXPORT_SYMBOL_GPL(kmbox_write);

int32_t kmbox_register_parse(kmbox_parse_process_t parse_process);
int32_t kmbox_register_parse(kmbox_parse_process_t parse_process)
{
	kmbox_parse_process = parse_process;

	return 0;
}
EXPORT_SYMBOL_GPL(kmbox_register_parse);

int32_t kmbox_unregister_parse(void)
{
	kmbox_parse_process = NULL;

	return 0;
}
EXPORT_SYMBOL_GPL(kmbox_unregister_parse);

int apu_mailbox_read(uint32_t ipi_src_mask, uint32_t *isp_id)
{
	int core_id;

	switch (ipi_src_mask) {
	case VISP_MBOX_RPU6_0:
		core_id = MBOX_CORE_RPU0;
		break;
	case VISP_MBOX_RPU7_1:
		core_id = MBOX_CORE_RPU1;
		break;
	case VISP_MBOX_RPU8_2:
		core_id = MBOX_CORE_RPU2;
		break;
	case VISP_MBOX_RPU9_3:
		core_id = MBOX_CORE_RPU3;
		break;
	default:
		pr_err("Invalid ipi_src_mask %d\n", ipi_src_mask);
		return -EINVAL;
	}

	if (vpi_mbox_is_empty(apu_fifo_ctrl, core_id, MBOX_CORE_APU)) {
		pr_err("No message in shared memory for core %d!\n", core_id);
	}

	vpi_mbox_read(apu_fifo_ctrl, rmsg_apu, core_id);
	if (kmbox_parse_process) {
		kmbox_parse_process(rmsg_apu->msg_id, rmsg_apu->payload,
				    rmsg_apu->size);
	} else {
		parse_command(rmsg_apu->msg_id, rmsg_apu->payload,
			      rmsg_apu->size, MBOX_CORE_APU, core_id);
	}

	memcpy(isp_id, ((payload_packet *)rmsg_apu->payload)->payload, sizeof(uint32_t));

	/* Normalize ISP instance ID; clarify why divided by 15 */
	*isp_id = *isp_id / 15;

	return rmsg_apu->msg_id;
}
EXPORT_SYMBOL_GPL(apu_mailbox_read);

// void mailbox_init(uint8_t cpu)
void mailbox_init(uint32_t cpu, uint64_t MBOX_FIFO_START_ADDR,
		  uint64_t mbox_fifo_start_addr_phy)
{
	rmsg_apu = (mbox_post_msg *)kmalloc(sizeof(mbox_post_msg), GFP_KERNEL);

	if (cpu == MBOX_CORE_APU) {
		apu_fifo_ctrl = vpi_mbox_init(
		    MBOX_CORE_APU, MBOX_FIFO_START_ADDR,
		    mbox_fifo_start_addr_phy, MBOX_FIFO_BLOCK_SIZE);
	} else if (cpu == MBOX_CORE_RPU0) {
		rpu0_fifo_ctrl = vpi_mbox_init(
		    MBOX_CORE_RPU0, MBOX_FIFO_START_ADDR,
		    mbox_fifo_start_addr_phy, MBOX_FIFO_BLOCK_SIZE);
	} else if (cpu == MBOX_CORE_RPU1) {
		rpu1_fifo_ctrl = vpi_mbox_init(
		    MBOX_CORE_RPU1, MBOX_FIFO_START_ADDR,
		    mbox_fifo_start_addr_phy, MBOX_FIFO_BLOCK_SIZE);
	} else if (cpu == MBOX_CORE_RPU2) {
		rpu2_fifo_ctrl = vpi_mbox_init(
		    MBOX_CORE_RPU2, MBOX_FIFO_START_ADDR,
		    mbox_fifo_start_addr_phy, MBOX_FIFO_BLOCK_SIZE);
	} else if (cpu == MBOX_CORE_RPU3) {
		rpu3_fifo_ctrl = vpi_mbox_init(
		    MBOX_CORE_RPU3, MBOX_FIFO_START_ADDR,
		    mbox_fifo_start_addr_phy, MBOX_FIFO_BLOCK_SIZE);
	}
}
EXPORT_SYMBOL_GPL(mailbox_init);

void mailbox_close(void)
{
	kfree(apu_fifo_ctrl);
	//	kfree(rpu0_fifo_ctrl);
	//	kfree(rpu1_fifo_ctrl);

	// kfree(rmsg_rpu0);
	// kfree(rmsg_rpu1);
	kfree(rmsg_apu);
}

int send_command(mb_cmd_id_e cmd, void *data, uint32_t size, uint8_t dest_cpu,
		 uint8_t src_cpu)
{
	int ret = 0;

	ret = write_mboxcmd(cmd, data, size, dest_cpu, src_cpu);
	return ret;
}
EXPORT_SYMBOL_GPL(send_command);

int send_response(mb_cmd_id_e res, payload_packet *data, uint32_t size,
		  uint8_t dest_cpu, uint8_t src_cpu)
{
	int ret = 0;
	struct arm_smccc_res ress;
	unsigned long a0, a1, a2, a3;

	ret = write_mboxcmd(res, data, size, dest_cpu, src_cpu);
	a0 = SMC_IPI_MAILBOX_NOTIFY;
	a1 = 5; // Local id
	a2 = 3; // Remote id
	a3 = 0;
	arm_smccc_smc(a0, a1, a2, a3, 0, 0, 0, 0, &ress);

	return ret;
}
EXPORT_SYMBOL_GPL(send_response);

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
//
int disp_cnt;
uint32_t parse_command(mb_cmd_id_e cmd, void *data, uint32_t size,
		       mbox_core_id core_id, mbox_core_id src_cpu)
{
	int res = 0;

	switch (cmd) {
		/*These cases handle the commands sent by the RPU's*/
	case MB_CMD_RES_SUCCESS: // xilinx flow
		data_from_interrupt.cmdid = MB_CMD_RES_SUCCESS;
		data_from_interrupt.data = data;
		memcpy(&data_from_interrupt.res_payload_pkt, rmsg_apu->payload,
		       ALIGN(size, 8) /*sizeof(payload_packet)*/);
		break;

	case MB_CMD_GET_SUCCESS: // xilinx flow
		data_from_interrupt.cmdid = MB_CMD_GET_SUCCESS;
		data_from_interrupt.data = rmsg_apu->payload;
		memcpy(&data_from_interrupt.res_payload_pkt, rmsg_apu->payload,
		       ALIGN(size, 8) /*sizeof(payload_packet)*/);
		break;

	case RPU_2_APU_CMD_DISPLAY_BUFFER /*RPU_2_APU_MB_CMD_FULL_BUFFER_INFORM*/:
		data_from_interrupt.data = data;
		break;

	case RPU_2_APU_MB_CMD_REPORT_INTERNAL_FAILURE:

	case MB_CMD_END:
	case APU_2_RPU_MB_CMB_INIT_FIRMWARE:
	case MB_CMD_RES_ERR:	 // xilinx flow
	case MB_CMD_RES_TIMEOUT: // xilinx flow
	case MB_CMD_BUF_RET:
	case RPU_2_APU_MB_CMD_ISP_ERR_REPORT:
	case RPU_2_APU_MB_CMD_FUSA_EVENT_CB:
	case RPU_2_APU_MB_CMD_IsiCreateIss:
	case RPU_2_APU_MB_CMD_IsiReleaseIss:
	case RPU_2_APU_MB_CMD_IsiEnumModeIss:
	case RPU_2_APU_MB_CMD_IsiCheckConnectionIss:
	case RPU_2_APU_MB_CMD_IsiOpenIss:
	case RPU_2_APU_MB_CMD_IsiCloseIss:
	case RPU_2_APU_MB_CMD_IsiGetModeIss:
	case RPU_2_APU_MB_CMD_IsiGetCapsIss:
	case RPU_2_APU_MB_CMD_IsiGetRevisionIss:
	case RPU_2_APU_MB_CMD_IsiSetStreamingIss:
	case RPU_2_APU_MB_CMD_IsiGetAeBaseInfoIss:
	case RPU_2_APU_MB_CMD_IsiGetAGainIss:
	case RPU_2_APU_MB_CMD_IsiGetDGainIss:
	case RPU_2_APU_MB_CMD_IsiSetAGainIss:
	case RPU_2_APU_MB_CMD_IsiSetDGainIss:
	case RPU_2_APU_MB_CMD_IsiGetIntTimeIss:
	case RPU_2_APU_MB_CMD_IsiSetIntTimeIss:
	case RPU_2_APU_MB_CMD_IsiGetFpsIss:
	case RPU_2_APU_MB_CMD_IsiSetFpsIss:
	case RPU_2_APU_MB_CMD_IsiGetIspStatusIss:
	case RPU_2_APU_MB_CMD_IsiSetWBIss:
	case RPU_2_APU_MB_CMD_IsiGetWBIss:
	case RPU_2_APU_MB_CMD_IsiSetBlcIss:
	case RPU_2_APU_MB_CMD_IsiGetBlcIss:
	case RPU_2_APU_MB_CMD_IsiSetTpgIss:
	case RPU_2_APU_MB_CMD_IsiGetTpgIss:
	case RPU_2_APU_MB_CMD_IsiGetExpandCurveIss:
	case RPU_2_APU_MB_CMD_IsiWriteRegIss:
	case RPU_2_APU_MB_CMD_IsiReadRegIss:
		data_from_interrupt.cmdid = cmd;
		data_from_interrupt.data = rmsg_apu->payload;
		memcpy(&data_from_interrupt.res_payload_pkt, rmsg_apu->payload,
		       ALIGN(size, 8) /*sizeof(payload_packet)*/);
		pr_err("RPU to APU cmd: %d\n", cmd);
		break;

	default:
		pr_err("DRIVER::In default\n");
		break;
	}
	// TODO:Send response tacket
	return res;
}
EXPORT_SYMBOL_GPL(parse_command);

void apu_mailbox_read_data(uint32_t ipi_src_mask, void *pdst)
{
	struct response_user_packet *dst = pdst;

	if (ipi_src_mask == 0) { // sdvsv
		if (vpi_mbox_is_empty(apu_fifo_ctrl, MBOX_CORE_RPU0,
				      MBOX_CORE_APU)) // rpu0 check  the msg
						      // from MBOX_CORE_APU
		{
			pr_err("there is no msg in Share memory!\n");
		} else {
			vpi_mbox_read(
			    apu_fifo_ctrl, rmsg_apu,
			    MBOX_CORE_RPU0); // rpu0 rece MBOX_CORE_APU's msg
			dst->cmdid = rmsg_apu->msg_id;
			memcpy(&dst->res_payload_pkt, rmsg_apu->payload,
			       ALIGN(rmsg_apu->size, 8));
		}
	} else if (ipi_src_mask == 1 /*RPU1 id*/) {
		if (vpi_mbox_is_empty(apu_fifo_ctrl, MBOX_CORE_RPU1,
				      MBOX_CORE_APU)) // rpu0 check  the msg
						      // from MBOX_CORE_APU
		{
			pr_err("there is no msg in Share memory!\n");
		} else {
			vpi_mbox_read(apu_fifo_ctrl, rmsg_apu,
				      MBOX_CORE_RPU1); // rpu0 received
						       // MBOX_CORE_APU's msg
			dst->cmdid = rmsg_apu->msg_id;
			memcpy(&dst->res_payload_pkt, rmsg_apu->payload,
			       ALIGN(rmsg_apu->size, 8));
		}
	} else {
		pr_err("worng  aruguement %s %d\n", __func__, __LINE__);
	}
}
EXPORT_SYMBOL_GPL(apu_mailbox_read_data);

MODULE_AUTHOR("anandam");
MODULE_DESCRIPTION("MBOX_CMD");
MODULE_LICENSE("GPL");
