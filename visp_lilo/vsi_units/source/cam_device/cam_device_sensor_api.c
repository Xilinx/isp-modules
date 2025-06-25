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

#include "cam_device_sensor_api.h"
#include "cam_device_api.h"
#include "cam_device.h"
#include "visp_mbox_driver.h"
#include "sensor_cmd.h"
#include <linux/slab.h>
#include <linux/string.h>
#include "kmbox.h"
#include "visp_driver.h"
#include "isi_mixed.h"

#define SENSOR_NAME_LEN 20

RESULT vsi_cam_device_sensor_open(struct visp_dev *isp_dev,
				  cam_device_handle_t h_cam_device,
				  uint32_t mode_index)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return (RET_WRONG_HANDLE);
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
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, &mode_index, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) {
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
			packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_SENSOR_OPEN, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);

	return result;
}

RESULT vsi_cam_device_sensor_drv_handle_register(
	struct visp_dev *isp_dev, cam_device_handle_t h_cam_device,
	const cam_device_sensor_drv_cfg_t *p_sensor_drv)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;
	isi_cam_drv_config_mbox_t *pcamcfg = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (NULL == p_cam_dev_ctx || NULL == p_sensor_drv)
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

	pcamcfg =
	    (isi_cam_drv_config_mbox_t *)(p_sensor_drv->sensor_drv_handle);

	pcamcfg->instance_id = p_cam_dev_ctx->instance_id;
	p_data = packet->payload;

	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, p_sensor_drv->sensor_drv_handle,
	       sizeof(isi_cam_drv_config_mbox_t));
	p_data += sizeof(isi_cam_drv_config_mbox_t);
	packet->payload_size += sizeof(isi_cam_drv_config_mbox_t);

	memcpy(p_data, &p_sensor_drv->sensor_dev_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	if (packet->payload_size > MAX_ITEM) {
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
			packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_REG, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);
	return result;
}

RESULT
vsi_cam_device_sensor_drv_handle_un_register(struct visp_dev *isp_dev,
					     cam_device_handle_t h_cam_device)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return (RET_WRONG_HANDLE);
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
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) {
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
			packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_SENSOR_DRV_HANDLE_UNREG, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);
	return result;
}

RESULT vsi_cam_device_sensor_close(struct visp_dev *isp_dev,
				   cam_device_handle_t h_cam_device)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return (RET_WRONG_HANDLE);
	p_cam_dev_ctx->cookie++;

	packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}
	memset(packet, 0, sizeof(payload_packet));

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) {
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
			packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_SENSOR_CLOSE, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;
	kfree(packet);
	return result;
}
RESULT vsi_cam_device_sensor_mapping(
	struct visp_dev *isp_dev, cam_device_handle_t h_cam_device,
	const char *p_sensor_name,
	cam_device_sensor_drv_handle_t *p_sensor_drvhandle)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;
	isi_cam_drv_config_mbox_t camcfg; //= NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (NULL == p_sensor_name || NULL == p_sensor_drvhandle)
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
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, p_sensor_name, SENSOR_NAME_LEN);
	p_data += SENSOR_NAME_LEN;
	packet->payload_size += SENSOR_NAME_LEN;

	packet->payload_size += sizeof(isi_cam_drv_config_mbox_t);

	if (packet->payload_size > MAX_ITEM) {
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
			packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_MAPPING,
				packet,
				packet->payload_size + payload_extra_size,
				isp_dev->isp_rpu, MBOX_CORE_APU);

	*p_sensor_drvhandle =
	    kzalloc(sizeof(isi_cam_drv_config_mbox_t), GFP_KERNEL);

	if (*p_sensor_drvhandle == NULL) {
		kfree(packet);
		dev_err(isp_dev->dev,
			"APU Failed to allocate memory for sensor mapping.\n");
		return RET_OUTOFMEM;
	}
	memcpy(*p_sensor_drvhandle, p_data, sizeof(isi_cam_drv_config_mbox_t));

	kfree(packet);

	camcfg = **(isi_cam_drv_config_mbox_t **)p_sensor_drvhandle;

	return result;
}

RESULT vsi_cam_device_sensor_query(struct visp_dev *isp_dev,
				   cam_device_handle_t h_cam_device,
				   cam_device_sensor_query_t *p_query)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (p_query == NULL)
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

	memcpy(p_data, p_query, sizeof(cam_device_sensor_query_t));
	packet->payload_size += sizeof(cam_device_sensor_query_t);

	if (packet->payload_size > MAX_ITEM) {
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
			packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_SENSOR_QUERY, packet,
				packet->payload_size + payload_extra_size,
				isp_dev->isp_rpu, MBOX_CORE_APU);
	memcpy(p_query, p_data, sizeof(cam_device_sensor_query_t));

	kfree(packet);
	return result;
}

RESULT vsi_cam_device_sensor_set_test_pattern(
	struct visp_dev *isp_dev, cam_device_handle_t h_cam_device,
	const cam_device_sensor_test_pattern_t *p_test_pattern)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (p_test_pattern == NULL)
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

	memcpy(p_data, p_test_pattern,
	       sizeof(cam_device_sensor_test_pattern_t));
	packet->payload_size += sizeof(cam_device_sensor_test_pattern_t);

	if (packet->payload_size > MAX_ITEM) {
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
			packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_SENSOR_SET_TP, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);
	return result;
}

RESULT vsi_cam_device_sensor_set_frame_rate(struct visp_dev *isp_dev,
					    cam_device_handle_t h_cam_device,
					    uint32_t *p_fps)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (p_fps == NULL)
		return RET_NULL_POINTER;
	p_cam_dev_ctx->cookie++;

	packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	memset(packet, 0, sizeof(payload_packet));

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, p_fps, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) {
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
			packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_SENSOR_SET_FRAMERATE, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);

	return result;
}

RESULT vsi_cam_device_sensor_get_connect_port_info(
	struct visp_dev *isp_dev, cam_device_handle_t h_cam_device,
	cam_device_mcm_port_id_t port_id,
	cam_device_sensor_connect_port_info_t *p_port_info)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	payload_packet *packet = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;
	if (p_port_info == NULL)
		return RET_NULL_POINTER;
	p_cam_dev_ctx->cookie++;

	packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}
	memset(packet, 0, sizeof(payload_packet));

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, &port_id, sizeof(cam_device_mcm_port_id_t));
	p_data += sizeof(cam_device_mcm_port_id_t);
	packet->payload_size += sizeof(cam_device_mcm_port_id_t);

	memcpy(p_data, p_port_info,
	       sizeof(cam_device_sensor_connect_port_info_t));
	packet->payload_size += sizeof(cam_device_sensor_connect_port_info_t);

	if (packet->payload_size > MAX_ITEM) {
		dev_err(isp_dev->dev, "Payload size:%d is > MAX_ITEM:%d\n",
			packet->payload_size, MAX_ITEM);
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	xlnx_send_mbox_data_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_SENSOR_GET_ConnectPortInfo, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);

	memcpy(p_port_info, p_data,
	       sizeof(cam_device_sensor_connect_port_info_t));
	p_port_info->name[sizeof(p_port_info->name) - 1] = '\0';

	kfree(packet);

	return result;
}
