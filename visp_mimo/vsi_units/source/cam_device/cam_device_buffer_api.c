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

/*Copy from cam_device.cpp*/

#include "cam_device_buffer_api.h"
#include "cam_device.h"
#include "cam_device_api.h"
#include "sensor_cmd.h"
#include "visp_driver.h"
#include "visp_mbox_driver.h"
#include "visp_app.h"
extern uint32_t cookie;

#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/time.h>
#include <linux/timekeeping.h>

RESULT vsi_cam_device_init_buf_chain(struct visp_dev *isp_dev,
				     cam_device_handle_t h_cam_device,
				     cam_device_buf_chain_id_t buf_id,
				     cam_device_buf_chain_config_t *p_config)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (NULL == p_cam_dev_ctx || NULL == p_config)
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

	memcpy(p_data, &buf_id, sizeof(cam_device_buf_chain_id_t));
	packet->payload_size += sizeof(cam_device_buf_chain_id_t);
	p_data += sizeof(cam_device_buf_chain_id_t);

	memcpy(p_data, p_config, sizeof(cam_device_buf_chain_config_t));
	packet->payload_size += sizeof(cam_device_buf_chain_config_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}

	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_INIT_BUF_CHAIN, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;
	kfree(packet);
	return result;
}

RESULT vsi_cam_device_de_init_buf_chain(struct visp_dev *isp_dev,
					cam_device_handle_t h_cam_device,
					cam_device_buf_chain_id_t buf_id)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
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

	memcpy(p_data, &buf_id, sizeof(cam_device_buf_chain_id_t));
	packet->payload_size += sizeof(cam_device_buf_chain_id_t);

	if (packet->payload_size > MAX_ITEM) {
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_DEINIT_BUF_CHAIN, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;
	kfree(packet);

	return result;
}

RESULT vsi_cam_device_create_buf_pool(
	struct visp_dev *isp_dev, cam_device_handle_t h_cam_device,
	cam_device_buf_chain_id_t buf_id,
	cam_device_buf_pool_config_t *h_buffer_pool_cfg)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;

	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	if (NULL == p_cam_dev_ctx || NULL == h_buffer_pool_cfg)
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

	memcpy(p_data, &buf_id, sizeof(cam_device_buf_chain_id_t));
	packet->payload_size += sizeof(cam_device_buf_chain_id_t);
	p_data += sizeof(cam_device_buf_chain_id_t);

	memcpy(p_data, &(h_buffer_pool_cfg->buf_mode),
	       sizeof(cam_device_buf_mode_t));
	packet->payload_size += sizeof(cam_device_buf_mode_t);
	p_data += sizeof(cam_device_buf_mode_t);

	memcpy(p_data, &(h_buffer_pool_cfg->buf_num), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(h_buffer_pool_cfg->buf_size), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	// 6 physical address
	memcpy(p_data, h_buffer_pool_cfg->p_base_addr_list,
	       h_buffer_pool_cfg->buf_num * sizeof(uint32_t));
	packet->payload_size += h_buffer_pool_cfg->buf_num * sizeof(uint32_t);
	p_data += h_buffer_pool_cfg->buf_num * sizeof(uint32_t);

	memcpy(p_data, &(h_buffer_pool_cfg->is_mapped), sizeof(bool_t));
	packet->payload_size += sizeof(bool_t);
	p_data += sizeof(bool_t);

	memcpy(p_data, h_buffer_pool_cfg->p_ipl_addr_list,
	       h_buffer_pool_cfg->buf_num * sizeof(uint32_t));
	packet->payload_size += h_buffer_pool_cfg->buf_num * sizeof(uint32_t);
	p_data += h_buffer_pool_cfg->buf_num * sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM) {
		mutex_unlock(&isp_dev->mlock);
		kfree(packet);
		return RET_OUTOFRANGE;
	}
	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_CREATE_BUFFER_POOL, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;
	kfree(packet);

	return result;
}

RESULT vsi_cam_device_destroy_buf_pool(struct visp_dev *isp_dev,
				       cam_device_handle_t h_cam_device,
				       cam_device_buf_chain_id_t buf_id)
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
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &buf_id, sizeof(cam_device_buf_chain_id_t));
	packet->payload_size += sizeof(cam_device_buf_chain_id_t);

	if (packet->payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_DESTORY_BUFFER_POOL, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);

	return result;
}

RESULT vsi_cam_device_setup_buf_mgmt(struct visp_dev *isp_dev,
				     cam_device_handle_t h_cam_device,
				     cam_device_buf_chain_id_t buf_id)
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
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &buf_id, sizeof(cam_device_buf_chain_id_t));
	packet->payload_size += sizeof(cam_device_buf_chain_id_t);
	p_data += sizeof(cam_device_buf_chain_id_t);

	if (packet->payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;
	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_SETUP_BUF_MGMT, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);

	return result;
}

/*****************************************************************************/
/**
 * @brief   This function releases buffer management.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_release_buf_mgmt(struct visp_dev *isp_dev,
				       cam_device_handle_t h_cam_device,
				       cam_device_buf_chain_id_t buf_id)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;
	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
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
	memcpy(p_data, &buf_id, sizeof(cam_device_buf_chain_id_t));
	packet->payload_size += sizeof(cam_device_buf_chain_id_t);

	if (packet->payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;
	result = xlnx_send_mbox_acked_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_RELEASE_BUF_MGMT, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);

	return result;
}

int drv_dq_cnt;
RESULT vsi_cam_device_de_que_buffer(struct visp_dev *isp_dev,
				    cam_device_handle_t h_cam_device,
				    cam_device_buf_chain_id_t buf_id,
				    media_buffer_t **p_media_buf)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	uint8_t *p_data = NULL;
	cam_device_context_t *p_cam_dev_ctx = NULL;

	drv_dq_cnt++;

	*p_media_buf = kzalloc(sizeof(media_buffer_t), GFP_KERNEL);
	if (!p_media_buf) {
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}
	p_cam_dev_ctx = (cam_device_context_t *)h_cam_device;
	if (NULL == p_cam_dev_ctx || NULL == *p_media_buf)
		return RET_NULL_POINTER;
	p_cam_dev_ctx->cookie++;

	packet = kzalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp_dev->dev, "FAILED TO KZALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	(*p_media_buf)->p_meta_data =
	    kzalloc(sizeof(pic_buf_meta_data_t), GFP_KERNEL);

	packet->cookie = p_cam_dev_ctx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	p_data = packet->payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &buf_id, sizeof(cam_device_buf_chain_id_t));
	packet->payload_size += sizeof(cam_device_buf_chain_id_t);
	p_data += sizeof(cam_device_buf_chain_id_t);

	memcpy(p_data, (*p_media_buf)->p_meta_data,
	       sizeof(pic_buf_meta_data_t));
	packet->payload_size += sizeof(pic_buf_meta_data_t);
	p_data += sizeof(pic_buf_meta_data_t);

	memcpy(p_data, &((*p_media_buf)->base_address), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &((*p_media_buf)->base_size), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &((*p_media_buf)->lock_count), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &((*p_media_buf)->is_full), sizeof(bool_t));
	packet->payload_size += sizeof(bool_t);
	p_data += sizeof(bool_t);

	memcpy(p_data, &((*p_media_buf)->index), sizeof(uint8_t));
	packet->payload_size += sizeof(uint8_t);
	p_data += sizeof(uint8_t);

	memcpy(p_data, &((*p_media_buf)->buf_mode), sizeof(buff_mode));
	packet->payload_size += sizeof(buff_mode);
	p_data += sizeof(buff_mode);

	memcpy(p_data, &((*p_media_buf)->p_ipl_address), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &((*p_media_buf)->p_owner), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;
	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_DEQUE_BUFFER, packet,
				packet->payload_size + payload_extra_size,
				isp_dev->isp_rpu, MBOX_CORE_APU);

	p_data = packet->payload;
	p_data += (sizeof(uint32_t) + sizeof(cam_device_buf_chain_id_t));

	memcpy((*p_media_buf)->p_meta_data, p_data,
	       sizeof(pic_buf_meta_data_t));
	p_data += sizeof(pic_buf_meta_data_t);

	memcpy(&((*p_media_buf)->base_address), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&((*p_media_buf)->base_size), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&((*p_media_buf)->lock_count), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&((*p_media_buf)->is_full), p_data, sizeof(bool_t));
	p_data += sizeof(bool_t);

	memcpy(&((*p_media_buf)->index), p_data, sizeof(uint8_t));
	p_data += sizeof(uint8_t);

	memcpy(&((*p_media_buf)->buf_mode), p_data, sizeof(buff_mode));
	p_data += sizeof(buff_mode);

	memcpy(&((*p_media_buf)->p_ipl_address), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&((*p_media_buf)->p_owner), p_data, sizeof(uint32_t));

	kfree(packet);

	return result;
}

int drv_enq_cnt;
RESULT vsi_cam_device_en_que_buffer(struct visp_dev *isp_dev,
				    cam_device_handle_t h_cam_device,
				    cam_device_buf_chain_id_t buf_id,
				    media_buffer_t *p_media_buf)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;

	payload_packet *packet = NULL;

	drv_enq_cnt++;

	if (p_cam_dev_ctx == NULL) {
		dev_err(isp_dev->dev, "Null Pcamdevctx\n");
		return RET_NULL_POINTER;
	}
	if (p_media_buf == NULL) {
		dev_err(isp_dev->dev, "Null p_media_buf\n");
		return RET_NULL_POINTER;
	}

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
	memcpy(p_data, &buf_id, sizeof(cam_device_buf_chain_id_t));
	packet->payload_size += sizeof(cam_device_buf_chain_id_t);
	p_data += sizeof(cam_device_buf_chain_id_t);

	memcpy(p_data, &(p_media_buf->base_address), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(p_media_buf->base_size), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(p_media_buf->lock_count), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &(p_media_buf->is_full), sizeof(bool_t));
	packet->payload_size += sizeof(bool_t);
	p_data += sizeof(bool_t);

	memcpy(p_data, &(p_media_buf->index), sizeof(uint8_t));
	packet->payload_size += sizeof(uint8_t);
	p_data += sizeof(uint8_t);

	memcpy(p_data, &(p_media_buf->buf_mode), sizeof(buff_mode));
	packet->payload_size += sizeof(buff_mode);
	p_data += sizeof(buff_mode);

	// dev_err(isp_dev->dev , "The value of p_owner is 0x%x
	// p_media_buf->p_owner 0x%x\n",
	// *((uint32_t*)p_data),(p_media_buf)->p_owner);
	memcpy(p_data, &((p_media_buf)->p_ipl_address), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	memcpy(p_data, &((p_media_buf)->p_owner), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	result = xlnx_send_mbox_without_ack_cmd(
	    isp_dev, APU_2_RPU_MB_CMD_ENQUE_BUFFER, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;
	kfree(packet);
	return result;
}

RESULT vsi_cam_device_get_buffer_size(struct visp_dev *isp_dev,
				      cam_device_handle_t h_cam_device,
				      cam_device_buf_chain_id_t buf_id,
				      uint32_t *p_buf_size)
{
	RESULT result = RET_SUCCESS;
	payload_packet *packet = NULL;
	cam_device_context_t *p_cam_dev_ctx = NULL;
	uint8_t *p_data = NULL;

	if (NULL == h_cam_device || NULL == p_buf_size)
		return RET_NULL_POINTER;

	p_cam_dev_ctx = (cam_device_context_t *)h_cam_device;
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

	memcpy(p_data, &buf_id, sizeof(cam_device_buf_chain_id_t));
	packet->payload_size += sizeof(cam_device_buf_chain_id_t);
	p_data += sizeof(cam_device_buf_chain_id_t);

	memcpy(p_data, p_buf_size, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	if (packet->payload_size > MAX_ITEM)
		return RET_OUTOFRANGE;

	xlnx_send_mbox_data_cmd(isp_dev, APU_2_RPU_MB_CMD_GET_BUFFER_SIZE,
				packet,
				packet->payload_size + payload_extra_size,
				isp_dev->isp_rpu, MBOX_CORE_APU);

	memcpy(p_buf_size, p_data, sizeof(uint32_t));
	if ((*p_buf_size) == 0) {
		dev_err(isp_dev->dev, "INVALID BUF SIZE = 0 %d\n", -EINVAL);
		return -EINVAL;
	}

	kfree(packet);
	return result;
}
