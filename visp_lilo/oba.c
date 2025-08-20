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
#include "oba.h"

oba_inst_t *oba_LookupConfig(oba_isp_instance_t isp_no, oba_id_t oba_no);
int oba_cfg_initialize(oba_inst_t *instance_ptr, const oba_inst_t *ConfigPtr);
int oba_set_resolution(oba_inst_t *instance_ptr, u32 v_res, u32 h_res);
int oba_set_fps(oba_inst_t *instance_ptr, int fps);
int oba_set_virtual_channel(oba_inst_t *instance_ptr, int virtual_channel);
int oba_set_fifo_write_mode(oba_inst_t *instance_ptr, int fifomode);
int oba_init(oba_map_t *instance_ptr);

oba_inst_t oba_config_table[XPAR_ISP_INSTANCE][MAX_OBA_PER_ISP] = {
	{ // isp
		{//Tile-0, ISP-0, OBA0 configuration
			.base_address[0] = ISP0_OBA0_BASEADDR,
	  .device_id = OBA_0_MP,
	  .path_info = MAIN_PATH,
	  .pixle_mode = QUAD_PIXEL_MODE,
	  .data_format = RGB888_FORMAT,
	  .data_type = YUV_420_8_BIT,
	  .tile_id = TILE_0,
	  .oba_is_enabled = OBA_DISABLED,
	  .isp_instance = OBA_ISP0},
		{// Tile-0, ISP-0, oba1 configuration
	  .base_address[0] = ISP0_OBA1_BASEADDR,
	  .device_id = OBA_1_SP,
	  .path_info = SELF_PATH,
	  .pixle_mode = QUAD_PIXEL_MODE,
	  .data_format = RGB888_FORMAT,
	  .data_type = YUV_420_8_BIT,
	  .tile_id = TILE_0,
	  .oba_is_enabled = OBA_DISABLED,
	  .isp_instance = OBA_ISP0}

	},
	{ // ISP1
		{// Tile-0, ISP-1, oba0 configuration
	  .base_address[0] = ISP1_OBA0_BASEADDR,
	  .device_id = OBA_2_MP,
	  .path_info = MAIN_PATH,
	  .pixle_mode = QUAD_PIXEL_MODE,
	  .data_format = RGB888_FORMAT,
	  .data_type = YUV_420_8_BIT,
	  .tile_id = TILE_0,
	  .oba_is_enabled = OBA_DISABLED,
	  .isp_instance = OBA_ISP1},
		{// Tile-0, ISP-1, oba1 configuration
	  .base_address[0] = ISP1_OBA0_BASEADDR,
	  .device_id = OBA_3_SP,
	  .path_info = SELF_PATH,
	  .pixle_mode = QUAD_PIXEL_MODE,
	  .data_format = RGB888_FORMAT,
	  .data_type = YUV_420_8_BIT,
	  .tile_id = TILE_0,
	  .oba_is_enabled = OBA_DISABLED,
	  .isp_instance = OBA_ISP1}}

};

oba_inst_t *oba_lookup_config(oba_isp_instance_t isp_no, oba_id_t oba_no)
{
	oba_inst_t *config_ptr = NULL;

	config_ptr = &oba_config_table[isp_no][oba_no];
	return config_ptr;
}

int oba_cfg_initialize(oba_inst_t *instance_ptr, const oba_inst_t *config_ptr)
{
	*instance_ptr = *config_ptr; // Copy structure content
	return 0;			// XST_SUCCESS
}

RESULT oba_init_send_command(struct visp_dev *isp_dev,
				 cam_device_handle_t h_cam_device, u8 path)
{
	RESULT result = RET_SUCCESS;
	oba_map_t oba_map;
	int oba_no = 0;
	u8 *p_data; // = packet->payload;

	memset(&oba_map, 0, sizeof(oba_map));

	//OBA initialization needed only for non-MCM streaming cases.

	// Ref:Bufpath is MP/SP1
	// caseCtx->instanceCfgCtx[instance_id].instancePath[bufPath].path =
	// bufPath;

	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;

	oba_no = path;

	int isp_no = isp_dev->id;

	oba_inst_t *config_ptr = oba_lookup_config(isp_no % 2, oba_no);

	if (config_ptr->oba_is_enabled == OBA_DISABLED)
		config_ptr->oba_is_enabled = OBA_ENABLED;

	config_ptr->isp_instance = isp_no;
	config_ptr->path_info = oba_no;
	config_ptr->tile_id = isp_no / 2;
	config_ptr->hcamdev_instanceid =
		p_cam_dev_ctx->instance_id;

	/* path value counts only output pads, corresponding pad in media-graph
	 * can be realized by incrementing path by 1.
	 */
	u8 pad_no = path + 1;

	switch (isp_dev->pad_data[pad_no].format.code) {
	case MEDIA_BUS_FMT_RBG888_1X24:
		dev_info(isp_dev->dev,
			 "Configure OBA for path:%d of ISP :%d format:RGB888\n",
			 path, isp_dev->id);
		config_ptr->data_format = RGB888_FORMAT;
		config_ptr->data_type = RGB888;
		break;
	case MEDIA_BUS_FMT_UYVY8_1X16:
		dev_info(isp_dev->dev,
			 "Configure OBA for path:%d of ISP :%d format:YUV_422 bits:%d\n",
			 path, isp_dev->id, isp_dev->oba[pad_no].bpp);
		config_ptr->data_format = YUV422_SP_FORMAT;
		if (isp_dev->oba[pad_no].bpp == 8)
			config_ptr->data_type = YUV_422_8_BIT;
		if (isp_dev->oba[pad_no].bpp == 10)
			config_ptr->data_type = YUV_422_10_BIT;
		break;
	case MEDIA_BUS_FMT_VYYUYY8_1X24:
		dev_info(isp_dev->dev,
			 "Configure OBA for path:%d of ISP :%d format:YUV_420 bits:%d\n",
			 path, isp_dev->id, isp_dev->oba[pad_no].bpp);
		config_ptr->data_format = YUV420_SP_FORMAT;
		if (isp_dev->oba[pad_no].bpp == 8)
			config_ptr->data_type = YUV_420_8_BIT;
		if (isp_dev->oba[pad_no].bpp == 10)
			config_ptr->data_type = YUV_420_10_BIT;
		break;
	default:
		dev_err(isp_dev->dev,
			"Unsupported format to Configure OBA for path:%d of ISP :%d\n",
			path, isp_dev->id);
		return -1;
	}

	payload_packet * packet;

	packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet)
		return -ENOMEM;

	p_data = packet->payload;
	packet->cookie = 0x11;
	packet->type = CMD;
	packet->payload_size = 0;

	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, config_ptr, sizeof(oba_inst_t));
	packet->payload_size += sizeof(oba_inst_t);
	// xil_printf("packet.payload_size %d\n",packet.payload_size);

	if (packet->payload_size > MAX_ITEM) {
		dev_err(isp_dev->dev,
			"%s: Payload size (%d) exceeds maximum allowed (%d)\n",
			__func__, packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_OBA_INIT, packet,
				 packet->payload_size + payload_extra_size,
				 isp_dev->isp_rpu, MBOX_CORE_APU);
	kfree(packet);
	return result;
}
