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

#include "cam_device.h"
#include "cam_device_api.h"
#include "sensor_cmd.h"
#include <linux/delay.h>
#include "visp_driver.h"
#include "visp_app.h"
#include "visp_mbox_driver.h"
#include "visp_common.h"
#include <linux/string.h>
uint8_t *cam_load_calib =
	NULL; //[160010] __attribute__((section(".cam_load_calib")));

RESULT vsi_cam_device_create(struct visp_dev *isp_dev,
				 cam_device_config_t *p_cam_config,
				 cam_device_handle_t *p_handle_cam_device)
{
	RESULT result = RET_SUCCESS;
	uint32_t hw_id = p_cam_config->isp_hw_id;
	uint32_t virtual_id = 0;
	uint32_t instance_id = 0;
	uint8_t *p_data = NULL;
	cam_device_handle_t h_cam_device = NULL;
	cam_device_context_t *p_cam_dev_ctx = NULL;
	payload_packet *packet = NULL;

	if (NULL == p_cam_config || NULL == p_handle_cam_device) {
		dev_err(isp_dev->dev, "DRV %s %d\n", __func__, __LINE__);
		return RET_NULL_POINTER;
	}

	/*
	 * AMD : Multi-Core Changes
	 * Verifying Multi-core
	 */

#ifdef DEBUG_FLAG
#else
	if (hw_id >= CAMDEV_HARDWARE_ID_MAX)
		return RET_OUTOFRANGE;
#endif
	cam_device_ispcore_init();
	/*Get Cam Device Handle*/
	result = cam_device_request_instance(hw_id, &h_cam_device, &virtual_id);
	if (RET_SUCCESS != result || NULL == h_cam_device)
		return RET_FAILURE;
	memset(h_cam_device, 0, sizeof(cam_device_context_t));

	/*Mapping instance id for a single device*/
	result = cam_device_instance_id_mapping(hw_id, virtual_id, &instance_id);
	if (result != RET_SUCCESS)
		return RET_UNSUPPORT_ID;

	p_cam_dev_ctx = (cam_device_context_t *)h_cam_device;
	p_cam_dev_ctx->isp_hw_id = hw_id;
	p_cam_dev_ctx->isp_vt_id = virtual_id;
	p_cam_dev_ctx->instance_id = instance_id;
	p_cam_dev_ctx->cookie = 0;
	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = 0;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &instance_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, p_cam_config, sizeof(cam_device_config_t));
	packet->payload_size += sizeof(cam_device_config_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	result = xlnx_send_mbox_acked_cmd(
		isp_dev, APU_2_RPU_MB_CMD_CREATE_INSTANCE, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
		MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	*p_handle_cam_device = h_cam_device;

	kfree(packet);
	return result;
}

EXPORT_SYMBOL(vsi_cam_device_create);

RESULT vsi_cam_device_destroy(struct visp_dev *isp_dev,
				  cam_device_handle_t h_cam_device)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;

	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result =
		xlnx_send_mbox_acked_cmd(isp_dev, APU_2_RPU_MB_CMD_DESTORY, packet,
					 packet->payload_size + payload_extra_size,
					 isp_dev->isp_rpu, MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);
	cam_device_free_instance(h_cam_device, p_cam_dev_ctx->isp_hw_id);
	return result;
}
EXPORT_SYMBOL(vsi_cam_device_destroy);

RESULT vsi_cam_device_set_out_format(struct visp_dev *isp_dev,
					 cam_device_handle_t h_cam_device,
					 cam_device_pipe_out_path_type_t path,
					 cam_device_pipe_out_fmt_t *p_fmt)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL || p_fmt == NULL) {
		dev_err(isp_dev->dev, "ISP_APP %s %d  \n", __func__, __LINE__);
		return 0;
	}
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (p_fmt == NULL)
		return RET_NULL_POINTER;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &path, sizeof(cam_device_pipe_out_path_type_t));
	packet->payload_size += sizeof(cam_device_pipe_out_path_type_t);
	p_data += sizeof(cam_device_pipe_out_path_type_t);

	memcpy(p_data, p_fmt, sizeof(cam_device_pipe_out_fmt_t));
	packet->payload_size += sizeof(cam_device_pipe_out_fmt_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	result = xlnx_send_mbox_acked_cmd(
		isp_dev, APU_2_RPU_MB_CMD_SET_OUT_FORMAT, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
		MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);
	return result;
}

RESULT vsi_cam_device_set_in_format(struct visp_dev *isp_dev,
					cam_device_handle_t h_cam_device,
					cam_device_pipe_in_path_type_t path,
					cam_device_pipe_in_fmt_t *p_fmt)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;

	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (p_fmt == NULL)
		return RET_NULL_POINTER;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &path, sizeof(cam_device_pipe_in_path_type_t));
	p_data += sizeof(cam_device_pipe_in_path_type_t);
	packet->payload_size += sizeof(cam_device_pipe_in_path_type_t);

	memcpy(p_data, p_fmt, sizeof(cam_device_pipe_in_fmt_t));
	packet->payload_size += sizeof(cam_device_pipe_in_fmt_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(
		isp_dev, APU_2_RPU_MB_CMD_SET_IN_FORMAT, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
		MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);

	return result;
}

RESULT vsi_cam_device_connect_camera(
	struct visp_dev *isp_dev, cam_device_handle_t h_cam_device,
	const cam_device_pipe_submodule_ctrl_u *p_sub_ctrl)
{
	RESULT result = RET_SUCCESS;

	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	if (p_cam_dev_ctx == NULL || p_sub_ctrl == NULL) {
		dev_err(isp_dev->dev, "ISP_APP NULL HANDLE %s %d\n", __func__,
			__LINE__);
		return RET_WRONG_HANDLE;
	}
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (p_sub_ctrl == NULL)
		return RET_NULL_POINTER;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;

	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, p_sub_ctrl, sizeof(cam_device_pipe_submodule_ctrl_u));
	packet->payload_size += sizeof(cam_device_pipe_submodule_ctrl_u);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	result = xlnx_send_mbox_acked_cmd(
		isp_dev, APU_2_RPU_MB_CMD_CONNECT_CAMERA, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
		MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);

	return result;
}

RESULT vsi_cam_device_disconnect_camera(struct visp_dev *isp_dev,
					cam_device_handle_t h_cam_device)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;

	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(
		isp_dev, APU_2_RPU_MB_CMD_DISCONNECT_CAMERA, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
		MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);
	return result;
}
RESULT vsi_cam_device_start_streaming(cam_device_config_t *p_cam_config,
					  cam_device_handle_t h_cam_device,
					  uint32_t frames // 0-continue
);

RESULT vsi_cam_device_start_streaming(cam_device_config_t *p_cam_config,
					  cam_device_handle_t h_cam_device,
					  uint32_t frames // 0-continue
)
{
	RESULT result = RET_SUCCESS;

	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;
	struct visp_dev *isp_dev = p_cam_config->isp_dev;

	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &frames, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	result = xlnx_send_mbox_acked_cmd(
		isp_dev, APU_2_RPU_MB_CMD_START_STREAMING, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
		MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);
	return result;
}

RESULT
vsi_cam_device_set_path_streaming(struct visp_dev *isp_dev,
				  cam_device_handle_t h_cam_device,
				  cam_device_path_streaming_cfg_t *p_config)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;
	int path_enable = -1;

	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (p_config == NULL)
		return RET_NULL_POINTER;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;

	*(packet->payload) = p_cam_dev_ctx->instance_id; //(int *)0x0;
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	path_enable = p_config->out_path_enable;
	*(p_data) = path_enable; //(int *)p_config->out_path_enable;//0x1;

	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	result = xlnx_send_mbox_acked_cmd(
		isp_dev, APU_2_RPU_MB_CMD_SET_PATH_STREAMING, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
		MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);
	return result;
}

RESULT
vsi_cam_device_get_path_streaming(struct visp_dev *isp_dev,
				  cam_device_handle_t h_cam_device,
				  cam_device_path_streaming_cfg_t *p_config)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;
	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (p_config == NULL)
		return RET_NULL_POINTER;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;

	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, p_config, sizeof(cam_device_path_streaming_cfg_t));
	packet->payload_size += sizeof(cam_device_path_streaming_cfg_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_GET_PATH_STREAMING,
				packet,
				packet->payload_size + payload_extra_size,
				isp_dev->isp_rpu, MBOX_CORE_APU);

	kfree(packet);
	return result;
}

RESULT vsi_cam_device_get_hardware_id(cam_device_handle_t h_cam_device,
					  uint32_t *p_hw_id)
{
	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (p_hw_id == NULL)
		return RET_NULL_POINTER;
	if (p_cam_dev_ctx->isp_hw_id > CAMDEV_HARDWARE_ID_MAX) {
		return RET_OUTOFRANGE;
	} else {
		*p_hw_id = p_cam_dev_ctx->isp_hw_id;
		return RET_SUCCESS;
	}
}

RESULT vsi_cam_device_alloc_res_memory(struct visp_dev *isp_dev,
					   cam_device_handle_t h_cam_device,
					   uint32_t byte_size,
					   uint32_t *p_base_address,
					   void **p_ipl_address)
{
	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &byte_size, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	p_data += sizeof(uint32_t);
	memcpy(p_data, p_base_address, sizeof(uint32_t));
	packet->payload_size += 2 * sizeof(uint32_t);
	dev_info(isp_dev->dev, "[ISP] %s %d %d %p\n", __func__, __LINE__,
		 byte_size, p_base_address);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_GET_PATH_STREAMING,
				packet,
				packet->payload_size + payload_extra_size,
				isp_dev->isp_rpu, MBOX_CORE_APU);

	memcpy(p_ipl_address, p_data, sizeof(uint32_t));

	kfree(packet);
	return result;
}

RESULT vsi_cam_device_free_res_memory(struct visp_dev *isp_dev,
					  cam_device_handle_t h_cam_device,
					  uint32_t base_address)
{
	cam_device_context_t *p_cam_dev_ctx =
		(cam_device_context_t *)h_cam_device;
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;

	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;

	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &base_address, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	result = xlnx_send_mbox_acked_cmd(
		isp_dev, APU_2_RPU_MB_CMD_FREE_RES_MEMORY, packet,
		packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
		MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);
	return RET_SUCCESS;
}
