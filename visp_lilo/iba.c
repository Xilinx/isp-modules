// SPDX-License-Identifier: MIT
/*
 * Copyright 2025 Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "sensor_cmd.h"
#include "visp_mbox_driver.h"
#include "cam_device.h"
#include "cam_device_api.h"
#include "iba.h"

static void print_iba_info(const iba_info_t *iba_info)
{
	if (!iba_info) {
		pr_err("IBA info is NULL\n");
		return;
	}
#ifdef ENABLE_IBA_LOG
	pr_err("IBA Info:\n");
	pr_err("  Base Address: 0x%x\n", iba_info->baseaddress);
	pr_err("  IBA id: %u\n", iba_info->iba_id);
	pr_err("  Max width: %u\n", iba_info->max_width);
	pr_err("  Max height: %u\n", iba_info->max_height);
	pr_err("  data format: %u\n", iba_info->data_format);
	pr_err("  IBA Enabled: %u\n",
	       iba_info->is_iba_enabled); // Assuming iba_status_t is u32
	pr_err("  H-Blank Prog: %u\n", iba_info->hblank_prog);
	pr_err("  V-Blank Prog: %u\n", iba_info->vblank_prog);
	pr_err("  VCID: %u\n", iba_info->vcid);
	pr_err("  PPC: %u\n", iba_info->ppc);
	pr_err("  ISP id: %u\n", iba_info->isp_id);
	pr_err("  Frame Rate: %u\n", iba_info->frame_rate);
#endif
}

int iba_init_send_command(struct visp_dev *isp_dev,
			  cam_device_handle_t h_cam_device)
{
	int result = 0;

	payload_packet
	    *packet;	 // = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	uint8_t *p_data; // = packet->payload;
	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	iba_info_t *iba_info;
	uint32_t iba_id;
	int isp_id = isp_dev->id;
	int num_streams = isp_dev->num_streams;
	int i = 0;

	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	// num_streams
	for (i = 0; i < isp_dev->num_streams; i++) {
		p_cam_dev_ctx->cookie++;
		if ((isp_id % 2) == 0)
			iba_id = i;
		else if ((isp_id % 2) == 1)
			iba_id = (num_streams == 1) ? IBA_4 : IBA_3 + i;

		iba_info = &isp_dev->iba[iba_id];
		iba_info->baseaddress = 0xE8520000 + (0x1000 * iba_id);
		iba_info->iba_id = iba_id;
		iba_info->max_width = 1920;
		iba_info->max_height = 1080;
		iba_info->is_iba_enabled = IBA_ENABLED;
		iba_info->isp_id = isp_dev->id;
		print_iba_info(iba_info);
		packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
		if (!packet) {
			dev_err(isp_dev->dev,
				"%s: Failed to allocate memory for packet\n",
				__func__);
			return -ENOMEM;
		}
		p_data = packet->payload;
		packet->cookie = 0x99;
		packet->type = CMD;
		packet->payload_size = 0;

		memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
		p_data += sizeof(uint32_t);
		packet->payload_size += sizeof(uint32_t);

		memcpy(p_data, iba_info, sizeof(iba_info_t));

		packet->payload_size += sizeof(iba_info_t);
		if (packet->payload_size > MAX_ITEM) {
			dev_err(isp_dev->dev,
				"%s: Payload size (%d) exceeds maximum allowed "
				"(%d)\n",
				__func__, packet->payload_size, MAX_ITEM);
			kfree(packet);
			return RET_OUTOFRANGE;
		}

		xlnx_send_mbox_acked_cmd(
		    isp_dev, APU_2_RPU_MB_CMD_IBA_INIT, packet,
		    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
		    MBOX_CORE_APU);
		kfree(packet);
	}
	return result;
}
