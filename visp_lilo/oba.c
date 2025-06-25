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

oba_inst_t *oba_LookupConfig(oba_isp_instance_t isp_no, oba_id_t oba_no)
{

	oba_inst_t *ConfigPtr = NULL;

	ConfigPtr = &oba_config_table[isp_no][oba_no];
	return ConfigPtr;
}

int oba_cfg_initialize(oba_inst_t *instance_ptr, const oba_inst_t *ConfigPtr)
{
	// Xil_AssertNonvoid(instance_ptr != NULL);
	// Xil_AssertNonvoid(ConfigPtr != NULL);
	*instance_ptr = *ConfigPtr; // Copy structure content
	return 0;			// XST_SUCCESS
}

RESULT oba_init_send_command(struct visp_dev *isp_dev,
				 cam_device_handle_t h_cam_device)
{
	RESULT result = RET_SUCCESS;
	cam_device_buf_chain_id_t path = 0;
	oba_map_t oba_map;
	int oba_no = 0;
	uint8_t *p_data; // = packet->payload;

	memset(&oba_map, 0, sizeof(oba_map));

	//OBA initialization needed only for non-MCM streaming cases.

	// Ref:Bufpath is MP/SP1
	// caseCtx->instanceCfgCtx[instance_id].instancePath[bufPath].path =
	// bufPath;

	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;

	for (path = CAMDEV_BUFCHAIN_MP; path <= CAMDEV_BUFCHAIN_SP1; path++) {
		oba_no = path;

		int isp_no = isp_dev->id;

		// l_o_g_i("AMD-Debug search for isp no. in tile %d,oba %d, path
		// %d \n",isp_no%2,oba_no,path );

		oba_inst_t *ConfigPtr = oba_LookupConfig(isp_no % 2, oba_no);

		if (ConfigPtr->oba_is_enabled == OBA_DISABLED) {
			ConfigPtr->oba_is_enabled = OBA_ENABLED;
			ConfigPtr->isp_instance = isp_no;
			ConfigPtr->path_info = oba_no;
			ConfigPtr->tile_id = isp_no / 2;
			ConfigPtr->hcamdev_instanceid =
				p_cam_dev_ctx->instance_id;

			if (strcmp(isp_dev->oba[path].data_format, "RGB888") ==
				0) {
				ConfigPtr->data_format = RGB888_FORMAT;
				ConfigPtr->data_type = RGB888;
			} else if (strcmp(isp_dev->oba[path].data_format,
					  "YUV422") == 0) {
				ConfigPtr->data_format = YUV422_SP_FORMAT;
				if (isp_dev->oba[MEDIA_ISP_PORT_PAD_SOURCE_MP]
					.bpp == 8)
					ConfigPtr->data_type = YUV_422_8_BIT;

				if (isp_dev->oba[MEDIA_ISP_PORT_PAD_SOURCE_MP]
					.bpp == 10)
					ConfigPtr->data_type = YUV_422_10_BIT;
			} else if (strcmp(isp_dev->oba[path].data_format,
					  "YUV420") == 0) {
				ConfigPtr->data_format = YUV420_SP_FORMAT;
				if (isp_dev->oba[MEDIA_ISP_PORT_PAD_SOURCE_MP]
					.bpp == 8)
					ConfigPtr->data_type = YUV_420_8_BIT;

				if (isp_dev->oba[MEDIA_ISP_PORT_PAD_SOURCE_MP]
					.bpp == 10)
					ConfigPtr->data_type = YUV_420_10_BIT;
			} else if (strcmp(isp_dev->oba[path].data_format,
					  "YUV400") == 0) {
				ConfigPtr->data_format = Y_FORMAT;
				if (isp_dev->oba[MEDIA_ISP_PORT_PAD_SOURCE_MP]
					.bpp == 8)
					ConfigPtr->data_type = YUV_400_8_BIT;

				if (isp_dev->oba[MEDIA_ISP_PORT_PAD_SOURCE_MP]
					.bpp == 10)
					ConfigPtr->data_type = YUV_400_10_BIT;
			} else {
				dev_err(
					isp_dev->dev,
					"Unsupported format configured to OBA.\n");
				return -1;
			}
			ConfigPtr->data_format = RGB888_FORMAT;
			ConfigPtr->data_type = RGB888;
		}

		// oba_cfg_initialize(&instance_ptr->oba_inst[isp_no/2][isp_no][oba_no],
		// ConfigPtr); dev_info("%s %d  devieId:%d base_address:%x
		// tile_no:%d isp_no:%d   oba_no:%d path_info:%d isp_inst:%d
		// instance_id:%d\n",
		//		__func__,__LINE__,ConfigPtr->device_id,
		//		ConfigPtr->base_address[0],isp_no/2, isp_no%2,
		//oba_no, ConfigPtr->path_info, ConfigPtr->isp_instance,
		//ConfigPtr->hcamdev_instanceid);

		payload_packet * packet;

		packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
		if (!packet) {
			dev_err(isp_dev->dev,
				"%s: Failed to allocate memory for packet\n",
				__func__);
			return -ENOMEM;
		}
		p_data = packet->payload;
		packet->cookie = 0x11;
		packet->type = CMD;
		packet->payload_size = 0;

		memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
		p_data += sizeof(uint32_t);
		packet->payload_size += sizeof(uint32_t);

		memcpy(p_data, ConfigPtr, sizeof(oba_inst_t));
		packet->payload_size += sizeof(oba_inst_t);
		// xil_printf("packet.payload_size %d\n",packet.payload_size);

		if (packet->payload_size > MAX_ITEM) {
			dev_err(isp_dev->dev,
				"%s: Payload size (%d) exceeds maximum allowed "
				"(%d)\n",
				__func__, packet->payload_size, MAX_ITEM);
			kfree(packet);
			return RET_OUTOFRANGE;
		}

		xlnx_send_mbox_acked_cmd(
			isp_dev, APU_2_RPU_MB_CMD_OBA_INIT, packet,
			packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
			MBOX_CORE_APU);
		kfree(packet);
	}
	return result;
}
