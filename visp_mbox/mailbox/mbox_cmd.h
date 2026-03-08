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

#ifndef _MBOX_CMD_H_
#define _MBOX_CMD_H_

// #include "vsi_command.h"
// #include <linux/mbox_api.h>
#include "mbox_api.h"
#include "mbox_error_code.h"
// #include <linux/mbox_fifo.h>
#include "mbox_fifo.h"
#include "mbox_hardware.h"
// #include <linux/sensor_cmd.h>
#include "sensor_cmd.h"
#include "visp_mbox_driver.h"

#define MAX_MSGS_PER_BOX 30
#define MBOX_FIFO_BLOCK_SIZE                                                   \
	(sizeof(fifo_control) + sizeof(mbox_post_msg) * MAX_MSGS_PER_BOX)
// #define MBOX_FIFO_START_ADDR 0x4ed08000
#define MBOX_SECTION_SIZE 0x100000
#define ONE_MB 0x100000

/*
 *#define MAX_MSGS_PER_BOX 4
 *#define MBOX_FIFO_BLOCK_SIZE (sizeof(fifo_control) +
 *sizeof(mbox_post_msg)*MAX_MSGS_PER_BOX) #define MBOX_FIFO_START_ADDR 0x4ed08000
 *#define MBOX_SECTION_SIZE  0x100000
 *#define ONE_MB 0x100000
 */

/************************* Test Configuration ********************************/

#define IPI_DEVICE_ID_0 XPAR_XIPIPSU_0_DEVICE_ID
// #define IPI_DEVICE_ID_1	XPAR_XIPIPSU_1_DEVICE_ID
// #define IPI_DEVICE_ID_2	XPAR_XIPIPSU_2_DEVICE_ID

#define IPI_CH0_BIT_MASK XPAR_XIPIPSU_0_BIT_MASK
// #define IPI_CH1_BIT_MASK XPAR_XIPIPSU_1_BIT_MASK
// #define IPI_CH2_BIT_MASK XPAR_XIPIPSU_2_BIT_MASK

/* Test message length in words. Max is 8 words (32 bytes) */
#define TEST_MSG_LEN 8
/* Interrupt Controller device id */
#define INTC_DEVICE_ID XPAR_SCUGIC_0_DEVICE_ID
/* Time out parameter while polling for response */
#define TIMEOUT_COUNT 100000

/*****************************************************************************/

void visp_mbox_mailbox_init(struct rpu_dev *rpu, u32 cpu, uint64_t MBOX_FIFO_START_ADDR,
			    uint64_t mbox_fifo_start_addr_phy);
uint32_t write_mboxcmd(uint32_t cmd_id, void *struct_msg, uint16_t size,
		       uint32_t flag, mbox_core_id receiver_id,
		       mbox_core_id core_id);

int visp_mbox_send_command(mb_cmd_id_e cmd, void *data, uint32_t size,
			   uint32_t flag, uint8_t dest_cpu, uint8_t src_cpu);

struct response_user_packet {
	/*Define your data fields*/
	u32 cmdid;
	void *data;
	payload_packet res_payload_pkt;
};

int visp_mbox_apu_read(struct rpu_dev *rpu);
uint32_t parse_command(mb_cmd_id_e cmd, void *data, uint32_t size, mbox_core_id id,
		       mbox_core_id id1);
void mailbox_close(struct rpu_dev *rpu);
int send_response(mb_cmd_id_e res, payload_packet *data, uint32_t size,
		  uint8_t dest_cpu, uint8_t src_cpu);

#endif
