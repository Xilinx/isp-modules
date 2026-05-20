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
#include "visp_mbox_driver.h"
#include "sensor_cmd.h"
#include "visp_app.h"
#include "visp_driver.h"
extern uint32_t cookie;

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/time.h>
#include <linux/timekeeping.h>
#include <linux/workqueue.h>

#define ENQ_TIMEOUT_RETRY_MAX 2
#define ENQ_TIMEOUT_RETRY_DELAY_US 2000

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
	    isp_dev, APU_2_RPU_MB_CMD_RELEASE_BUF_MGMT, packet,
	    packet->payload_size + payload_extra_size, isp_dev->isp_rpu,
	    MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;

	kfree(packet);

	return result;
}

int drv_dq_cnt;
int drv_enq_cnt;

#define ENQ_CUSTOM_PAYLOAD_SIZE 48
typedef struct payload_template_enq {
	payload_type type;
	uint32_t cookie;
	uint32_t payload_size;
	uint32_t reserved[1]; //to meet the 8 byte alignment requirement
	response_field_t resp_field;
	uint8_t payload[ENQ_CUSTOM_PAYLOAD_SIZE];
} payload_packet_enq;

struct visp_enq_work_ctx {
	struct work_struct work;
	struct visp_dev *isp_dev;
	cam_device_context_t *cam_ctx;
	cam_device_buf_chain_id_t buf_id;
	output_buffer_t *p_media_buf;
};

static struct workqueue_struct *visp_get_enq_wq(struct visp_dev *isp_dev,
					    int port,
					    cam_device_buf_chain_id_t buf_id)
{
	struct workqueue_struct *wq = NULL;

	if (!isp_dev || port < 0 || port >= MAX_PORTS)
		return NULL;

	/* Prefer per-chain queues: chain 0 = MP, chain 1 = SP1 */
	if (buf_id < ENQ_WQ_CHAIN_MAX)
		wq = isp_dev->enq_wq_chain[port][buf_id];

	/* Fallback: use MP queue when chain-specific queue is unavailable */
	if (!wq)
		wq = isp_dev->enq_wq_chain[port][CAMDEV_BUFCHAIN_MP];

	return wq;
}

static bool visp_enq_stream_on(struct visp_dev *isp_dev, int port,
			      cam_device_buf_chain_id_t buf_id)
{
	int path_idx;
	int pad_index;

	if (!isp_dev || port < 0 || port >= MAX_PORTS)
		return false;

	path_idx = buf_id;
	if (path_idx < 0 || path_idx >= MEDIA_ISP_CHN_MAX)
		return true;

	pad_index = (port * MEDIA_ISP_PORT_PAD_COUNT) + path_idx + 1;
	if (pad_index < 0 || pad_index >= (VISP_PORT_PAD_NR * MAX_PORTS))
		return false;

	return isp_dev->streamon[pad_index] != 0;
}

static int visp_resolve_enq_port(struct visp_dev *isp_dev,
				 cam_device_handle_t h_cam_device,
				 cam_device_context_t *p_cam_dev_ctx)
{
	int port;

	if (!isp_dev)
		return 0;

	for (port = 0; port < MAX_PORTS; port++) {
		if (isp_dev->isp_ports[port].cam_device_handle == h_cam_device)
			return port;
	}

	/* Fallback for legacy behavior if handle is not discoverable. */
	if (p_cam_dev_ctx && p_cam_dev_ctx->isp_vt_id < MAX_PORTS) {
		dev_warn_ratelimited(isp_dev->dev,
			"ENQ port fallback: handle=%p not matched, using vt_id=%u\n",
			h_cam_device, p_cam_dev_ctx->isp_vt_id);
		return p_cam_dev_ctx->isp_vt_id;
	}

	return 0;
}

static int visp_send_enq_cmd(struct visp_dev *isp_dev,
			      cam_device_context_t *p_cam_dev_ctx,
			      cam_device_buf_chain_id_t buf_id,
			      output_buffer_t *p_media_buf)
{
	RESULT result = RET_SUCCESS;
	uint8_t *p_data = NULL;
	int retry;
	int port;
	payload_packet_enq packet = {0};
	media_isp_chn_attr *isp_chns;

	if (!isp_dev || !p_cam_dev_ctx || !p_media_buf)
		return RET_NULL_POINTER;

	port = visp_resolve_enq_port(isp_dev,
				    (cam_device_handle_t)p_cam_dev_ctx,
				    p_cam_dev_ctx);
	if (port < 0 || port >= MAX_PORTS) {
		dev_warn(isp_dev->dev,
			 "ENQ clamp resolved_port=%d to port=0 (max=%d, vt_id=%d)\n",
			 port, MAX_PORTS, p_cam_dev_ctx->isp_vt_id);
		port = 0;
	}

	/* Stream is already stopped: drop stale late ENQ requests quietly. */
	if (!visp_enq_stream_on(isp_dev, port, buf_id))
		return RET_SUCCESS;

	drv_enq_cnt++;
	p_cam_dev_ctx->cookie++;

	packet.cookie = p_cam_dev_ctx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	p_data = packet.payload;
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &buf_id, sizeof(cam_device_buf_chain_id_t));
	packet.payload_size += sizeof(cam_device_buf_chain_id_t);
	p_data += sizeof(cam_device_buf_chain_id_t);
	memcpy(p_data, &(p_media_buf->index), sizeof(uint8_t));
	packet.payload_size += sizeof(uint8_t);
	p_data += sizeof(uint8_t);

	/* Lock to read p_owner - protects against concurrent p_owner writes from dequeue path */
	isp_chns = &isp_dev->isp_ports[port].isp_chns[buf_id];
	mutex_lock(&isp_chns->cam_device_bufs_lock);
	memcpy(p_data, &((p_media_buf)->p_owner), sizeof(uint32_t));
	mutex_unlock(&isp_chns->cam_device_bufs_lock);
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	if (packet.payload_size > MAX_ITEM)
		result = RET_OUTOFRANGE;

	for (retry = 0; retry <= ENQ_TIMEOUT_RETRY_MAX; retry++) {
		result = xlnx_send_mbox_acked_cmd(isp_dev,
		    APU_2_RPU_MB_CMD_ENQUE_BUFFER, &packet,
		    packet.payload_size + payload_extra_size, isp_dev->isp_rpu,
		    MBOX_CORE_APU);

		if (result == RET_SUCCESS)
			break;

		if (result == -ESHUTDOWN)
			return RET_SUCCESS;

		/* Retry only on ACK timeout; keep other failures unchanged. */
		if (result != -ETIMEDOUT || retry == ENQ_TIMEOUT_RETRY_MAX)
			break;

		/* Stream can turn off between retries; stop retrying stale ENQ. */
		if (!visp_enq_stream_on(isp_dev, port, buf_id))
			return RET_SUCCESS;

		udelay(ENQ_TIMEOUT_RETRY_DELAY_US);
	}

	if (result != RET_SUCCESS)
		result = RET_FAILURE;

	return result;
}

static void visp_enq_work_handler(struct work_struct *work)
{
	struct visp_enq_work_ctx *ctx =
	    container_of(work, struct visp_enq_work_ctx, work);
	int port;
	int ret;

	if (ctx->isp_dev && ctx->cam_ctx) {
		port = visp_resolve_enq_port(ctx->isp_dev,
					    (cam_device_handle_t)ctx->cam_ctx,
					    ctx->cam_ctx);
		if (port < 0 || port >= MAX_PORTS)
			port = 0;

		if (!visp_enq_stream_on(ctx->isp_dev, port, ctx->buf_id)) {
			/* Stream already off: silently drop late queued ENQ work. */
			kfree(ctx);
			return;
		}
	}

	ret = visp_send_enq_cmd(ctx->isp_dev, ctx->cam_ctx,
			       ctx->buf_id, ctx->p_media_buf);

	kfree(ctx);
}
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

RESULT vsi_cam_device_en_que_buffer(struct visp_dev *isp_dev,
				    cam_device_handle_t h_cam_device,
				    cam_device_buf_chain_id_t buf_id,
				    output_buffer_t *p_media_buf)
{
	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;
	int port;
	int path_idx;
	struct visp_enq_work_ctx *ctx;

	if (p_cam_dev_ctx == NULL) {
		dev_err(isp_dev->dev, "Null Pcamdevctx\n");
		return RET_NULL_POINTER;
	}
	if (p_media_buf == NULL) {
		dev_err(isp_dev->dev, "Null p_media_buf\n");
		return RET_NULL_POINTER;
	}

	port = visp_resolve_enq_port(isp_dev, h_cam_device, p_cam_dev_ctx);
	if (port < 0 || port >= MAX_PORTS) {
		dev_warn(isp_dev->dev,
			 "ENQ clamp resolved_port=%d to port=0 (max=%d, vt_id=%d)\n",
			 port, MAX_PORTS, p_cam_dev_ctx->isp_vt_id);
		port = 0;
	}

	if (!visp_enq_stream_on(isp_dev, port, buf_id)) {
		dev_warn_ratelimited(isp_dev->dev,
			"ENQ dropped: stream off isp=%d port=%d path=%d vt_id=%d instance_id=%u idx=%u\n",
			isp_dev->id, port, (int)buf_id, p_cam_dev_ctx->isp_vt_id,
			p_cam_dev_ctx->instance_id, p_media_buf->index);
		int path_idx_check = buf_id;
		int pad_index_check;

		/* Normal stream-off drops are expected; log only inconsistent gate decisions. */
		pad_index_check = (port * MEDIA_ISP_PORT_PAD_COUNT) + path_idx_check + 1;
		if (path_idx_check >= 0 && path_idx_check < MEDIA_ISP_CHN_MAX &&
		    pad_index_check >= 0 &&
		    pad_index_check < (VISP_PORT_PAD_NR * MAX_PORTS) &&
		    isp_dev->streamon[pad_index_check] != 0) {
			dev_err_ratelimited(isp_dev->dev,
				"ENQ gate inconsistency (isp=%d port=%d path=%d"
				" idx=%u vt_id=%d streamon=1)\n",
				isp_dev->id, port, (int)buf_id, p_media_buf->index,
				p_cam_dev_ctx->isp_vt_id);
		}
		return RET_SUCCESS;
	}

	path_idx = buf_id;

	/* Pick per-chain queue if available; else fall back */
	{
		struct workqueue_struct *wq =
			visp_get_enq_wq(isp_dev, port, buf_id);

		if (!wq) {
			dev_warn(isp_dev->dev,
				 "ENQ async fallback: vt_id=%d port=%d buf_id=%d wq=NULL\n",
				 p_cam_dev_ctx->isp_vt_id, port, buf_id);
			return visp_send_enq_cmd(isp_dev, p_cam_dev_ctx, buf_id,
					p_media_buf);
		}

		ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
		if (!ctx)
			return RET_OUTOFMEM;

		ctx->isp_dev = isp_dev;
		ctx->cam_ctx = p_cam_dev_ctx;
		ctx->buf_id = buf_id;
		ctx->p_media_buf = p_media_buf;
		INIT_WORK(&ctx->work, visp_enq_work_handler);

		if (!queue_work(wq, &ctx->work)) {
			kfree(ctx);
			return RET_FAILURE;
		}
	}

	return RET_SUCCESS;
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
	dev_info(isp_dev->dev, "RDMA p_data=%d==%d\n", *p_data, buf_id);
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
