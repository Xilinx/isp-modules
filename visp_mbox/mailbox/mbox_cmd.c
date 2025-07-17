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

//#include <linux/mbox_api.h>
#include "mbox_cmd.h"

#include "mbox_api.h"
#include "mbox_error_code.h"
//#include <linux/sensor_cmd.h>
#include <linux/arm-smccc.h>
#include <linux/gfp.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "sensor_cmd.h"
//#include "buf_defs.h"
#include <linux/kernel.h>
#include <linux/ktime.h>

#include "visp_driver.h"

struct response_user_packet data_from_interrupt;
EXPORT_SYMBOL_GPL(data_from_interrupt);

//extern int Handle_Frameout_Buffer(void * Enque_Buff_L);
//int (* exported_func)(void *);

typedef void (*kmbox_parse_process_t)(uint16_t cmd, void *data, uint32_t size);
static kmbox_parse_process_t kmbox_parse_process = NULL;

static MboxFifoCtrl *rpu0_fifo_ctrl = NULL;
static MboxFifoCtrl *rpu1_fifo_ctrl = NULL;
static MboxFifoCtrl *rpu2_fifo_ctrl = NULL;
static MboxFifoCtrl *rpu3_fifo_ctrl = NULL;
static MboxFifoCtrl *apu_fifo_ctrl = NULL;
//static MboxPostMsg *rmsg_rpu0 = NULL;
//static MboxPostMsg *rmsg_rpu1 = NULL;
static MboxPostMsg *rmsg_apu = NULL;

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

uint32_t flag = 0;
#define DEBUG_ENABLE_LOG

struct response_packet
{
	u32 cmdid;
	u32 cookie;
	u32 error_subcode;
};

void apu_postmsg(MboxCoreId receiver_id)
{
	if (MBOX_CORE_RPU0 == receiver_id)
	{
		//Status = trigger_ipi(&IpiInst_apu_rpu,MBOX_CORE_RPU0);
	}

	if (MBOX_CORE_RPU1 == receiver_id)
	{
		//		Status = trigger_ipi(&IpiInst_apu_rpu,MBOX_CORE_RPU1);
	}
}

uint32_t write_mboxcmd(uint32_t cmdId, void *struct_msg, uint16_t size,
					   MboxCoreId receiver_id, MboxCoreId core_id)
{
	int ret;
	MboxPostMsg *msg = NULL;
	msg = (MboxPostMsg *)kzalloc(sizeof(MboxPostMsg), GFP_KERNEL);
	if (msg == NULL)
	{
		pr_err("%s %d Failed to allocate memory\n", __func__, __LINE__);
		return VPI_ERR_NOMEM;
	}

	if (size == 0)
	{
		msg->msg_id = cmdId;
	}
	else
	{
		msg->msg_id = cmdId;
		//msg->size = size;
		// memcpy(msg->payload, struct_msg, size);
		msg->size = sizeof(payload_packet) - MAX_ITEM +
					((payload_packet *)struct_msg)->payload_size;
		memcpy(msg->payload, struct_msg, ALIGN(msg->size, 8));
	}

	if (core_id != 4)
	{
		core_id = 4;
	}
	if (MBOX_CORE_APU == core_id)
	{
		ret = vpi_mbox_post(apu_fifo_ctrl, msg, receiver_id, apu_postmsg);
	}

	kfree(msg);

	if (ret)
	{
		return ret;
	}

	return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(write_mboxcmd);

uint32_t kmbox_write(int32_t cmdId, void *msg, uint16_t size,
					 MboxCoreId receiver_id, MboxCoreId core_id)
{
	unsigned long a0, a1, a2, a3;
	struct arm_smccc_res res;
	uint32_t ret;

	ret = write_mboxcmd(cmdId, msg, size, receiver_id, core_id);

	a0 = SMC_IPI_MAILBOX_NOTIFY;
	a1 = 5; //Local Id
	a2 = 3; // Remote ID
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

int apu_mailbox_read(uint32_t IpiSrcMask, uint32_t *isp_id)
{
	if (IpiSrcMask == 0 /*RPU0 id*/)
	{ //sdvsv
		if (vpi_mbox_is_empty(
				apu_fifo_ctrl, MBOX_CORE_RPU0,
				MBOX_CORE_APU)) //rpu0 check  the msg from MBOX_CORE_APU
		{
			pr_err("there is no msg in APU Share memory!\n");
		}
		else
		{
			vpi_mbox_read(apu_fifo_ctrl, rmsg_apu,
						  MBOX_CORE_RPU0); //rpu0 rece MBOX_CORE_APU's msg
			if (kmbox_parse_process)
			{
				kmbox_parse_process(rmsg_apu->msg_id, rmsg_apu->payload,
									rmsg_apu->size);
			}
			else
			{
				ParseCommand(rmsg_apu->msg_id, rmsg_apu->payload,
							 rmsg_apu->size, MBOX_CORE_APU, MBOX_CORE_RPU0);
			}
		}
	}
	else if (IpiSrcMask == 1 /*RPU1 id*/)
	{
		if (vpi_mbox_is_empty(
				apu_fifo_ctrl, MBOX_CORE_RPU1,
				MBOX_CORE_APU)) //rpu0 check  the msg from MBOX_CORE_APU
		{
			pr_err("there is no msg in Share memory!\n");
		}
		else
		{
			vpi_mbox_read(apu_fifo_ctrl, rmsg_apu,
						  MBOX_CORE_RPU1); //rpu0 received MBOX_CORE_APU's msg
			if (kmbox_parse_process)
			{
				kmbox_parse_process(rmsg_apu->msg_id, rmsg_apu->payload,
									rmsg_apu->size);
			}
			else
			{
				ParseCommand(rmsg_apu->msg_id, rmsg_apu->payload,
							 rmsg_apu->size, MBOX_CORE_APU, MBOX_CORE_RPU1);
			}
		}
	}
	else
	{
		pr_err("worng  aruguement %s %d\n", __func__, __LINE__);
		return -1;
	}
	memcpy(isp_id, ((payload_packet *)rmsg_apu->payload)->payload, 4);
	//	pr_err("%s %d instance_id 0x%x, payload[0] 0x%x\n",__func__,__LINE__,isp_id,((payload_packet *)rmsg_apu->payload)->payload[0]);
	*isp_id = *isp_id / 15;
	return rmsg_apu->msg_id;
}
EXPORT_SYMBOL_GPL(apu_mailbox_read);

//void mailbox_init(uint8_t cpu)
void mailbox_init(uint32_t cpu, uint64_t MBOX_FIFO_START_ADDR,
				  uint64_t MBOX_FIFO_START_ADDR_PHY)
{
	//init mbox and mem_map
	//	rmsg_rpu0 = (MboxPostMsg *)kmalloc(sizeof(MboxPostMsg),GFP_KERNEL);
	//	rmsg_rpu1 = (MboxPostMsg *)kmalloc(sizeof(MboxPostMsg),GFP_KERNEL);
	rmsg_apu = (MboxPostMsg *)kmalloc(sizeof(MboxPostMsg), GFP_KERNEL);

	if (MBOX_CORE_APU == cpu)
	{
		apu_fifo_ctrl =
			vpi_mbox_init(MBOX_CORE_APU, MBOX_FIFO_START_ADDR,
						  MBOX_FIFO_START_ADDR_PHY, MBOX_FIFO_BLOCK_SIZE);
	}
	else if (MBOX_CORE_RPU0 == cpu)
	{
		rpu0_fifo_ctrl =
			vpi_mbox_init(MBOX_CORE_RPU0, MBOX_FIFO_START_ADDR,
						  MBOX_FIFO_START_ADDR_PHY, MBOX_FIFO_BLOCK_SIZE);
	}
	else if (MBOX_CORE_RPU1 == cpu)
	{
		rpu1_fifo_ctrl =
			vpi_mbox_init(MBOX_CORE_RPU1, MBOX_FIFO_START_ADDR,
						  MBOX_FIFO_START_ADDR_PHY, MBOX_FIFO_BLOCK_SIZE);
	}
	else if (MBOX_CORE_RPU2 == cpu)
	{
		rpu2_fifo_ctrl =
			vpi_mbox_init(MBOX_CORE_RPU2, MBOX_FIFO_START_ADDR,
						  MBOX_FIFO_START_ADDR_PHY, MBOX_FIFO_BLOCK_SIZE);
	}
	else if (MBOX_CORE_RPU3 == cpu)
	{
		rpu3_fifo_ctrl =
			vpi_mbox_init(MBOX_CORE_RPU3, MBOX_FIFO_START_ADDR,
						  MBOX_FIFO_START_ADDR_PHY, MBOX_FIFO_BLOCK_SIZE);
	}
	/*
		if(MBOX_CORE_APU == cpu){
			apu_fifo_ctrl = vpi_mbox_init(MBOX_CORE_APU, MBOX_FIFO_START_ADDR,MBOX_FIFO_START_ADDR_PHY, MBOX_FIFO_BLOCK_SIZE);
		}else if(MBOX_CORE_RPU0 == cpu){
			rpu0_fifo_ctrl = vpi_mbox_init(MBOX_CORE_RPU0, MBOX_FIFO_START_ADDR,MBOX_FIFO_START_ADDR_PHY, MBOX_FIFO_BLOCK_SIZE);
		}else if(MBOX_CORE_RPU1 == cpu){
			rpu1_fifo_ctrl = vpi_mbox_init(MBOX_CORE_RPU1, MBOX_FIFO_START_ADDR,MBOX_FIFO_START_ADDR_PHY, MBOX_FIFO_BLOCK_SIZE);
		}
	*/
}
EXPORT_SYMBOL_GPL(mailbox_init);

void mailbox_close(void)
{
	kfree(apu_fifo_ctrl);
	//	kfree(rpu0_fifo_ctrl);
	//	kfree(rpu1_fifo_ctrl);

	//kfree(rmsg_rpu0);
	//kfree(rmsg_rpu1);
	kfree(rmsg_apu);
}

int Send_Command(MBCmdId_E cmd, void *data, uint32_t size, uint8_t dest_cpu,
				 uint8_t src_cpu)
{
	int ret = 0;
	ret = write_mboxcmd(cmd, data, size, dest_cpu, src_cpu);
	return ret;
}
EXPORT_SYMBOL_GPL(Send_Command);

int Send_Response(MBCmdId_E res, payload_packet *data, uint32_t size,
				  uint8_t dest_cpu, uint8_t src_cpu)
{
	int ret = 0;
	struct arm_smccc_res ress;
	unsigned long a0, a1, a2, a3;

	ret = write_mboxcmd(res, data, size, dest_cpu, src_cpu);
	a0 = SMC_IPI_MAILBOX_NOTIFY;
	a1 = 5; //Local Id
	a2 = 3; // Remote ID
	a3 = 0;
	arm_smccc_smc(a0, a1, a2, a3, 0, 0, 0, 0, &ress);

	return ret;
}
EXPORT_SYMBOL_GPL(Send_Response);

volatile void *Enque_Buff_G;
volatile int DQ_BUF_AVAILABLE = 0;

EXPORT_SYMBOL_GPL(Enque_Buff_G);
EXPORT_SYMBOL_GPL(DQ_BUF_AVAILABLE);

struct Chn_info_l
{
	int HwId;
	int Mode;
	int VtId;
	int path;
};

int Read_DQ_Bufinfo_l(void *data, MediaBuffer_t *Enque_Buff_L,
					  struct Chn_info_l *info);

int Read_DQ_Bufinfo_l(void *data, MediaBuffer_t *Enque_Buff_L,
					  struct Chn_info_l *info)
{
	MediaBuffer_t Display_Mediabuff_G;
	payload_packet *packet = data;
	uint8_t *p_data; // = packet->payload;
	if (!packet)
	{
		kfree(packet);
		return -ENOMEM;
	}

	p_data = packet->payload;

	//struct Chn_info info;
	memcpy(info, p_data, sizeof(struct Chn_info_l));
	p_data += sizeof(struct Chn_info_l);

	memcpy(Display_Mediabuff_G.pMetaData, p_data, sizeof(PicBufMetaData_t));
	p_data += sizeof(PicBufMetaData_t);

	memcpy(&(Display_Mediabuff_G.baseAddress), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(Display_Mediabuff_G.baseSize), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(Display_Mediabuff_G.lockCount), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(Display_Mediabuff_G.isFull), p_data, sizeof(bool_t));
	p_data += sizeof(bool_t);

	memcpy(&(Display_Mediabuff_G.index), p_data, sizeof(uint8_t));
	p_data += sizeof(uint8_t);

	memcpy(&(Display_Mediabuff_G.bufMode), p_data, sizeof(BUFF_MODE));
	p_data += sizeof(BUFF_MODE);

	memcpy(&(Display_Mediabuff_G.pIplAddress), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(Display_Mediabuff_G.pOwner), p_data, sizeof(uint32_t));

	Enque_Buff_L->baseAddress = Display_Mediabuff_G.baseAddress;
	Enque_Buff_L->baseSize = Display_Mediabuff_G.baseSize;
	Enque_Buff_L->buf = Display_Mediabuff_G.buf;
	Enque_Buff_L->bufMode = Display_Mediabuff_G.bufMode;
	Enque_Buff_L->index = Display_Mediabuff_G.index;
	Enque_Buff_L->isFull = Display_Mediabuff_G.isFull;
	Enque_Buff_L->lockCount = Display_Mediabuff_G.lockCount;
	Enque_Buff_L->pIplAddress = Display_Mediabuff_G.pIplAddress;
	Enque_Buff_L->pMetaData = Display_Mediabuff_G.pMetaData;
	Enque_Buff_L->pOwner = Display_Mediabuff_G.pOwner;

	return 0;
}
//
int Disp_cnt = 0;
uint32_t ParseCommand(MBCmdId_E cmd, void *data, uint32_t size,
					  MboxCoreId core_id, MboxCoreId src_cpu)
{
	int res = 0;
	switch (cmd)
	{
			/*These cases handle the commands sent by the RPU's*/
		case MB_CMD_RES_SUCCESS: //xilinx flow
			data_from_interrupt.cmdid = MB_CMD_RES_SUCCESS;
			data_from_interrupt.data = data;
			memcpy(&data_from_interrupt.res_payload_pkt, rmsg_apu->payload,
				   ALIGN(size, 8) /*sizeof(payload_packet)*/);
			break;

		case MB_CMD_GET_SUCCESS: //xilinx flow
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
		case MB_CMD_RES_ERR: //xilinx flow
		case MB_CMD_RES_TIMEOUT: //xilinx flow
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
			pr_err("RPU to APU cmd: %d \n", cmd);
			break;

		default:
			pr_err("DRIVER::In default\n");
			break;
	}
	//TODO:Send response tacket
	return res;
}
EXPORT_SYMBOL_GPL(ParseCommand);

void apu_mailbox_read_data(uint32_t IpiSrcMask, void *pdst)
{
	struct response_user_packet *dst = pdst;
	if (IpiSrcMask == 0 /*RPU0 id*/)
	{ //sdvsv
		if (vpi_mbox_is_empty(
				apu_fifo_ctrl, MBOX_CORE_RPU0,
				MBOX_CORE_APU)) //rpu0 check  the msg from MBOX_CORE_APU
		{
			pr_err("there is no msg in Share memory!\n");
		}
		else
		{
			vpi_mbox_read(apu_fifo_ctrl, rmsg_apu,
						  MBOX_CORE_RPU0); //rpu0 rece MBOX_CORE_APU's msg
			dst->cmdid = rmsg_apu->msg_id;
			memcpy(&dst->res_payload_pkt, rmsg_apu->payload,
				   ALIGN(rmsg_apu->size, 8));
		}
	}
	else if (IpiSrcMask == 1 /*RPU1 id*/)
	{
		if (vpi_mbox_is_empty(
				apu_fifo_ctrl, MBOX_CORE_RPU1,
				MBOX_CORE_APU)) //rpu0 check  the msg from MBOX_CORE_APU
		{
			pr_err("there is no msg in Share memory!\n");
		}
		else
		{
			vpi_mbox_read(apu_fifo_ctrl, rmsg_apu,
						  MBOX_CORE_RPU1); //rpu0 received MBOX_CORE_APU's msg
			dst->cmdid = rmsg_apu->msg_id;
			memcpy(&dst->res_payload_pkt, rmsg_apu->payload,
				   ALIGN(rmsg_apu->size, 8));
		}
	}
	else
	{
		pr_err("worng  aruguement %s %d\n", __func__, __LINE__);
	}
}
EXPORT_SYMBOL_GPL(apu_mailbox_read_data);

MODULE_AUTHOR("anandam");
MODULE_DESCRIPTION("MBOX_CMD");
MODULE_LICENSE("GPL");
