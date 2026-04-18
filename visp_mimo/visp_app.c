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

#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <linux/delay.h>
#include "visp_event.h"
#include "visp_driver.h"
#include "media_isp.h"
#include <linux/string.h>
#include <linux/slab.h>
#include <cam_device.h>
#include "cam_device_api.h"
#include "cam_device_buffer_api.h"
#include <linux/delay.h>
#include "sensor_cmd.h"
#include "visp_app.h"
#include "visp_v4l2_std_exts.h"
#include "cam_device_sensor_api.h"
#include "iba.h"
#include "visp_common.h"
#include "visp_mbox_driver.h"

#define MEDIA_ISP_MCM_BUF_COUNT 4
#define MEDIA_ISP_EMPTY_BUF_WAIT_TIME 10
#define MEDIA_ISP_FULL_BUF_WAIT_TIME 120
int media_isp_device_mcm_set_format(struct visp_dev *isp_dev, uint8_t port);
int visp_set_fmt_public(struct visp_dev *isp_dev,
			struct v4l2_subdev_format *format);
int media_isp_device_camera_connect(struct visp_dev *isp_dev, uint8_t index);
int media_isp_device_set_format(struct visp_dev *isp_dev, uint8_t port,
				uint8_t chn);
#if 0
int media_isp_device_set_frame_rate(struct visp_dev *isp_dev, uint8_t port,
					uint32_t *frame_rate)
{
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	int ret_val = VSI_SUCCESS;

	ret_val = vsi_cam_device_sensor_set_frame_rate(
		isp_dev, isp_port->cam_device_handle, frame_rate);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"CamDevice set framerate failed, ret is %d", ret_val);
		ret_val = VSI_ERR_ILLEGAL_PARAM;
	}

	return ret_val;
}
#endif
int visp_buffer_alloc_public(struct visp_dev *isp_dev,
				 struct visp_ext_buf_info *ext_buf_info);

int visp_buffer_free_public(struct visp_dev *isp_dev,
				struct visp_ext_buf_info *ext_buf_info);

int media_isp_hal_alloc_buf(struct visp_dev *isp_dev, int port,
				media_buf *BufInfo);
int media_isp_hal_alloc_buf(struct visp_dev *isp_dev, int port,
				media_buf *BufInfo)
{
	struct visp_ext_buf_info ExtBufInfo;
	int ret_val = VSI_SUCCESS;

	ExtBufInfo.port = port;
	ExtBufInfo.plane.size = BufInfo->planes[0].dma_size;
	ret_val = visp_buffer_alloc_public(isp_dev, &ExtBufInfo);
	if (ret_val) {
		dev_err(isp_dev->dev, "port %d alloc dma buffer failed:\n",
			port);
		return VSI_ERR_ILLEGAL_PARAM;
	}

	BufInfo->planes[0].dma_addr = ExtBufInfo.plane.dma_addr;

	return ret_val;
}

int visp_buffer_free_public_wrapper(struct visp_dev *isp_dev, void *arg);

int media_isp_hal_free_buf(struct visp_dev *isp_dev, int port,
			   media_buf *BufInfo);
int media_isp_hal_free_buf(struct visp_dev *isp_dev, int port,
			   media_buf *BufInfo)
{
	struct visp_ext_buf_info ExtBufInfo;

	ExtBufInfo.port = port;
	ExtBufInfo.plane.dma_addr = BufInfo->planes[0].dma_addr;
	visp_buffer_free_public_wrapper(isp_dev, &ExtBufInfo);
	dev_info(isp_dev->dev, " port %d buffer free: Dma 0x%llx Size %d", port,
		 BufInfo->planes[0].dma_addr, BufInfo->planes[0].dma_size);

	return VSI_SUCCESS;
}

int media_isp_device_destroy_buf_pool(struct visp_dev *isp_dev,
					       uint8_t port, uint8_t chn)
{
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	int ret_val = VSI_SUCCESS;

	vsi_cam_device_release_buf_mgmt(isp_dev, isp_port->cam_device_handle,
					chn);
	vsi_cam_device_destroy_buf_pool(isp_dev, isp_port->cam_device_handle,
					chn);

	if (chn == CAMDEV_BUFCHAIN_RDMA) {
		if (isp_port->mcm_attr.input_select ==
			MEDIA_ISP_MCM_INPUT_SELECT_RPU) {
			int i = 0;

			for (i = 0; i < isp_port->mcm_attr.num_bufs; i++) {
				vsi_cam_device_free_res_memory(
					isp_dev, isp_port->cam_device_handle,
					isp_port->mcm_attr.bufs[i]
					.planes[0]
					.dma_addr);
				isp_port->mcm_attr.bufs[i].planes[0].dma_addr =
					0;
			}
		} else if (isp_port->mcm_attr.input_select ==
			   MEDIA_ISP_MCM_INPUT_SELECT_APU) {
			int i = 0;

			for (i = 0; i < isp_port->mcm_attr.num_bufs; i++) {
				media_buf *BufInfo =
					&isp_port->mcm_attr.bufs[i];
				media_isp_hal_free_buf(isp_dev, port, BufInfo);
				BufInfo->planes[0].dma_addr = 0;
			}
		}
	}
	vsi_cam_device_de_init_buf_chain(isp_dev, isp_port->cam_device_handle,
					 chn);

	return ret_val;
}
EXPORT_SYMBOL_GPL(media_isp_device_destroy_buf_pool);

int media_isp_device_stream_off(struct visp_dev *isp_dev, uint8_t port,
				uint8_t chn);
int media_isp_device_stream_off(struct visp_dev *isp_dev, uint8_t port,
				uint8_t chn)
{
	int ret_val = VSI_SUCCESS;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];

	cam_device_path_streaming_cfg_t PathStatus;

	vsi_cam_device_get_path_streaming(isp_dev, isp_port->cam_device_handle,
					  &PathStatus);
	PathStatus.out_path_enable &= ~(1 << chn);

	PathStatus.out_path_enable = 0;
	ret_val = vsi_cam_device_set_path_streaming(
		isp_dev, isp_port->cam_device_handle, &PathStatus);

	media_isp_device_destroy_buf_pool(isp_dev, port, CAMDEV_BUFCHAIN_RDMA);
	media_isp_device_destroy_buf_pool(isp_dev, port, CAMDEV_BUFCHAIN_MP);

	return ret_val;
}
EXPORT_SYMBOL_GPL(media_isp_device_stream_off);

RESULT vsi_cam_device_un_register_ae_lib(struct visp_dev *isp_dev,
					 cam_device_handle_t h_cam_device);
RESULT vsi_cam_device_ae_disable(struct visp_dev *isp_dev,
				 cam_device_handle_t h_cam_device);
RESULT vsi_cam_device_awb_disable(struct visp_dev *isp_dev,
				  cam_device_handle_t h_cam_device);
RESULT vsi_cam_device_un_register_awb_lib(struct visp_dev *isp_dev,
					  cam_device_handle_t h_cam_device);

static int media_isp_device_un_register3a_lib(struct visp_dev *isp_dev,
						  uint8_t port, uint8_t chn)
{
#ifdef LOAD_CALIB_ENABLE

	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];

	vsi_cam_device_ae_disable(isp_dev, isp_port->cam_device_handle);
	vsi_cam_device_un_register_ae_lib(isp_dev, isp_port->cam_device_handle);
	vsi_cam_device_awb_disable(isp_dev, isp_port->cam_device_handle);
	vsi_cam_device_un_register_awb_lib(isp_dev,
					   isp_port->cam_device_handle);
	//
#endif

	return VSI_SUCCESS;
}

static int media_isp_device_mcm_terminate(struct visp_dev *isp_dev,
					  uint8_t port)
{
	int ret_val = VSI_SUCCESS;
	uint8_t chn = CAMDEV_BUFCHAIN_RDMA;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];

	media_isp_device_destroy_buf_pool(isp_dev, 0, chn);
	isp_port->mcm_attr.num_bufs = 0;

	return ret_val;
}

int media_isp_device_camera_dis_connect(struct visp_dev *isp_dev, uint8_t port,
					uint8_t chn)
{
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];

	dev_info(isp_dev->dev, "[VVCAM-CLEANUP]->%s %d\n", __func__, __LINE__);

	if (isp_port->camera_connect_ref_cnt > 0)
		isp_port->camera_connect_ref_cnt--;

	dev_info(isp_dev->dev, "[VVCAM-CLEANUP]->%s %d Camcnt=%d\n", __func__,
		 __LINE__, isp_port->camera_connect_ref_cnt);

	if (isp_port->camera_connect_ref_cnt == 0) {
		dev_info(isp_dev->dev, "[VVCAM-CLEANUP]->%s %d\n", __func__,
			 __LINE__);
		media_isp_device_un_register3a_lib(isp_dev, port, chn);
		vsi_cam_device_disconnect_camera(isp_dev,
						 isp_port->cam_device_handle);
		media_isp_device_mcm_terminate(isp_dev, port);
	}

	dev_info(isp_dev->dev, "[VVCAM-CLEANUP]->%s %d\n", __func__, __LINE__);

	return VSI_SUCCESS;
}

static int isp_send_atm_prop_to_rpu(struct visp_dev *isp,
				    cam_device_handle_t h_cam_device)
{
	int result = 0;
	payload_packet *packet;
	uint8_t *p_data;
	cam_device_context_t *p_cam_dev_ctx =
	    (cam_device_context_t *)h_cam_device;

	if (p_cam_dev_ctx == NULL)
		return RET_WRONG_HANDLE;

	packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
	if (!packet) {
		dev_err(isp->dev, "%s: Failed to allocate memory for packet\n",
			__func__);
		return -ENOMEM;
	}

	p_data = packet->payload;
	packet->cookie = 0x99;
	packet->type = CMD;
	packet->payload_size = 0;

	// Copy instance id
	memcpy(p_data, &p_cam_dev_ctx->instance_id, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	// Copy ATM properties (high_mem, is_64bit, node_name)
	memcpy(p_data, &isp->atm.high_mem_addr, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	packet->payload_size += sizeof(uint32_t);

	memcpy(p_data, &isp->atm.is_64bit, sizeof(int));
	p_data += sizeof(int);
	packet->payload_size += sizeof(int);

	result =
	    xlnx_send_mbox_acked_cmd(isp, APU_2_RPU_MB_CMD_SET_ATM, packet,
				     packet->payload_size + payload_extra_size,
				     isp->isp_rpu, MBOX_CORE_APU);
	if (result != RET_SUCCESS)
		return RET_FAILURE;
	kfree(packet);

	return result;
}

int media_isp_device_create_buf_pool(struct visp_dev *isp_dev, uint8_t port,
					 uint8_t chn);
int media_isp_device_create_buf_pool(struct visp_dev *isp_dev, uint8_t port,
					 uint8_t chn)
{
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	int ret_val = VSI_SUCCESS;
	int i = 0;
	uint32_t num_bufs = 0;
	cam_device_buf_pool_config_t BufPoolCfg;
	uint32_t buf_size = 0;

	cam_device_buf_chain_config_t BufferChain;

	if (chn < MEDIA_ISP_CHN_MAX)
		num_bufs = isp_port->isp_chns[chn].num_bufs;
	else
		num_bufs = isp_port->mcm_attr.num_bufs;
	if (chn == CAMDEV_BUFCHAIN_RDMA)
		num_bufs = 2;
	else
		num_bufs = 4;

	/**** STEP 1.INIT BUF CHAIN **********/
	memset(&BufferChain, 0, sizeof(cam_device_buf_chain_config_t));
	BufferChain.skip_interval = 0;
	BufferChain.buf_que_length = num_bufs;
	BufferChain.empty_que_op.block_type = CAMDEV_BUFQUE_TIMEOUT_TYPE;
	BufferChain.empty_que_op.wait_time =
		5000; // MEDIA_ISP_EMPTY_BUF_WAIT_TIME;
	BufferChain.full_que_op.block_type = CAMDEV_BUFQUE_TIMEOUT_TYPE;
	BufferChain.full_que_op.wait_time =
		5000; // MEDIA_ISP_FULL_BUF_WAIT_TIME;

	ret_val = vsi_cam_device_init_buf_chain(
		isp_dev, isp_port->cam_device_handle, chn, &BufferChain);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"%s: CamDevice init buf chain %d failed, ret is %d",
			__func__, chn, ret_val);
		ret_val = VSI_ERR_ILLEGAL_PARAM;
		return ret_val;
	}

	/****** STEP 2. RESERVE BUFFERS *******/
	memset(&BufPoolCfg, 0, sizeof(BufPoolCfg));
	BufPoolCfg.buf_num = num_bufs;
	BufPoolCfg.buf_mode = CAMDEV_BUFMODE_USERPTR;
	BufPoolCfg.is_mapped = 0;
	BufPoolCfg.p_base_addr_list =
		kmalloc(num_bufs * sizeof(uint32_t), GFP_KERNEL);
	BufPoolCfg.p_ipl_addr_list =
		kmalloc(num_bufs * sizeof(void *), GFP_KERNEL);

	if (chn < MEDIA_ISP_CHN_MAX) {
		// create isp buf pool by user allocated dma memory
		BufPoolCfg.buf_size = isp_dev->cap_sizeimage;

		for (i = 0; i < num_bufs; i++) {
			BufPoolCfg.p_base_addr_list[i] = isp_dev->op_a[i];
				dev_info(isp_dev->dev,"%s: isp:%d o/p buff:%d address : %llx ",
				__func__, isp_dev->id, i, isp_dev->op_a[i]);
			BufPoolCfg.p_ipl_addr_list[i] = VSI_NULL;
		}
	} else if (chn == CAMDEV_BUFCHAIN_RDMA) {
		// create mcm buf pool by camdevice reserved memory
		// uint32_t phy_addr = 0;
		uint32_t *pIpl_addr = VSI_NULL;

		ret_val = vsi_cam_device_get_buffer_size(
			isp_dev, isp_port->cam_device_handle, chn, &buf_size);
		if (ret_val != VSI_SUCCESS) {
			dev_err(isp_dev->dev,
				"%s: CamDevice get chain %d buf size failed, "
				"ret is %d",
				__func__, chn, ret_val);
			ret_val = VSI_ERR_ILLEGAL_PARAM;
			goto ERR_TO_DEINIT_CHAIN;
		}
		BufPoolCfg.buf_size = buf_size;
		BufPoolCfg.buf_size = isp_dev->out_sizeimage; //
		isp_port->mcm_attr.input_select =
			MEDIA_ISP_MCM_INPUT_SELECT_APU;

		if (isp_port->mcm_attr.input_select ==
			MEDIA_ISP_MCM_INPUT_SELECT_RPU) {
		} else if (isp_port->mcm_attr.input_select ==
			   MEDIA_ISP_MCM_INPUT_SELECT_APU) {

			for (i = 0; i < num_bufs; i++) {
				media_buf *BufInfo =
					&isp_port->mcm_attr.bufs[i];
				BufInfo->planes[0].dma_size = buf_size;

				BufPoolCfg.p_base_addr_list[i] =
					isp_dev->ip_a[i];
				BufPoolCfg.p_ipl_addr_list[i] =
					(void *)pIpl_addr;
				dev_info(isp_dev->dev,"%s: ISP:%d input buff:%d  address : %llx ",
				__func__, isp_dev->id, i, isp_dev->ip_a[i]);

			}
			uint32_t high_mem = isp_dev->ip_a[i]>>32U;

			if(high_mem)
			{
				isp_dev->atm.high_mem_addr = high_mem;
				isp_dev->atm.is_64bit = true;
			}
			isp_send_atm_prop_to_rpu(isp_dev, isp_port->cam_device_handle);
		}
	}

	else {
		dev_err(isp_dev->dev,
			"%s: current not support chn %d to create buf pool",
			__func__, chn);
		ret_val = VSI_ERR_NOT_SUPPORT;
		goto ERR_TO_DEINIT_CHAIN;
	}

	/************ STEP 3. CREATE BUFFER POOL ****************/
	ret_val = vsi_cam_device_create_buf_pool(
		isp_dev, isp_port->cam_device_handle, chn, &BufPoolCfg);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"CamDevice chn %d create buf pool failed, ret is %d",
			chn, ret_val);
		ret_val = VSI_ERR_ILLEGAL_PARAM;
		goto ERR_TO_RELEASE_MEM;
	}
	/************ STEP 4. SETUP BUF MGMT *******************/
	ret_val = vsi_cam_device_setup_buf_mgmt(
		isp_dev, isp_port->cam_device_handle, chn);
	if (ret_val != VSI_SUCCESS) {
		dev_err(
			isp_dev->dev,
			"CamDevice chn %d setup buf managementfailed, ret is %d",
			chn, ret_val);
		ret_val = VSI_ERR_ILLEGAL_PARAM;
		goto ERR_TO_DESTROY_BUFPOOL;
	}

	kfree(BufPoolCfg.p_ipl_addr_list);
	kfree(BufPoolCfg.p_base_addr_list);
	return ret_val;

ERR_TO_DESTROY_BUFPOOL:
	vsi_cam_device_destroy_buf_pool(isp_dev, isp_port->cam_device_handle,
					chn);
ERR_TO_RELEASE_MEM:
	if (chn == CAMDEV_BUFCHAIN_RDMA) {
		if (isp_port->mcm_attr.input_select ==
			MEDIA_ISP_MCM_INPUT_SELECT_RPU) {
			for (--i; i >= 0; --i) {
				vsi_cam_device_free_res_memory(
					isp_dev, isp_port->cam_device_handle,
					BufPoolCfg.p_base_addr_list[i]);
				isp_port->mcm_attr.bufs[i].planes[0].dma_addr =
					0;
			}
		} else if (isp_port->mcm_attr.input_select ==
			   MEDIA_ISP_MCM_INPUT_SELECT_APU) {
			for (--i; i >= 0; --i) {
				media_buf *BufInfo =
					&isp_port->mcm_attr.bufs[i];
				media_isp_hal_free_buf(isp_dev, port, BufInfo);
				BufInfo->planes[0].dma_addr = 0;
			}
		}
	}
ERR_TO_DEINIT_CHAIN:
	kfree(BufPoolCfg.p_ipl_addr_list);
	kfree(BufPoolCfg.p_base_addr_list);
	vsi_cam_device_de_init_buf_chain(isp_dev, isp_port->cam_device_handle,
					 chn);

	return ret_val;
}

int media_isp_device_stream_on(struct visp_dev *isp_dev, uint8_t port,
				   uint8_t chn)
{
	int ret_val = VSI_SUCCESS;

	media_isp_device_mcm_set_format(isp_dev, port);

	/*Create Buffer pool for input*/
	ret_val = media_isp_device_create_buf_pool(isp_dev, port, 6);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"%s: port %d chn %d create buf pool failed, ret is %d",
			__func__, port, chn, ret_val);
		return ret_val;
	}
	/* Try connecting the sensor - not required for MIMO*/
	media_isp_device_camera_connect(isp_dev, 0);
	return 0;
}

int media_isp_device_stream_on_out(struct visp_dev *isp_dev, uint8_t port,
				   uint8_t chn)
{
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	int ret_val = VSI_SUCCESS;
	int pad_index = -1;
	cam_device_path_streaming_cfg_t PathStatus;

	/* Set Input format */
	media_isp_device_set_format(isp_dev, 0, 0);

	chn = CAMDEV_BUFCHAIN_MP;
	/*Create Buffer pool for output*/
	ret_val = media_isp_device_create_buf_pool(isp_dev, 0, chn);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"%s: port %d chn %d create buf pool failed, ret is %d",
			__func__, port, chn, ret_val);
		return ret_val;
	}

	PathStatus.out_path_enable |= (1 << chn);

	/*Set streaming state*/
	ret_val = vsi_cam_device_set_path_streaming(
		isp_dev, isp_port->cam_device_handle, &PathStatus);
	if (ret_val != VSI_SUCCESS) {
		dev_err(
			isp_dev->dev,
			"port %d chn %d CamDevice start stream failed, ret is %d",
			port, chn, ret_val);
		ret_val = VSI_ERR_NOTREADY;
		goto ERR_TO_DESTROY_BUFPOOL;
	}

	pad_index = (port * MEDIA_ISP_PORT_PAD_COUNT) + chn + 1;
	isp_dev->streamon[pad_index] = 1;
	return ret_val;

ERR_TO_DESTROY_BUFPOOL:
	media_isp_device_destroy_buf_pool(isp_dev, port, chn);

	return ret_val;
}
EXPORT_SYMBOL(media_isp_device_stream_on);
EXPORT_SYMBOL(media_isp_device_stream_on_out);

int media_isp_device_deque(struct visp_dev *isp_dev, uint8_t port);
int media_isp_device_deque(struct visp_dev *isp_dev, uint8_t port)
{
	int ret_val = VSI_SUCCESS;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	media_buffer_t *p_media_buf;

	if (isp_port->cam_device_handle) {

		ret_val = vsi_cam_device_de_que_buffer(
			isp_dev, isp_port->cam_device_handle, CAMDEV_BUFCHAIN_RDMA,
			&p_media_buf);
		if (ret_val != VSI_SUCCESS) {
			dev_err(isp_dev->dev,
				"VsiCamDeviceDeQueBuffer failed %d", ret_val);
			ret_val = VSI_ERR_TIMEOUT;
			return ret_val;
		}

		ret_val = vsi_cam_device_en_que_buffer(
			isp_dev, isp_port->cam_device_handle, CAMDEV_BUFCHAIN_RDMA,
			p_media_buf);
		if (ret_val != VSI_SUCCESS) {
			dev_err(isp_dev->dev,
				"VsiCamDeviceEnQueBuffer failed %d", ret_val);
			ret_val = VSI_ERR_TIMEOUT;
			return ret_val;
		}
	}
	return ret_val;
}
EXPORT_SYMBOL(media_isp_device_deque);

int isp_device_destroy(struct visp_dev *isp_dev, uint8_t port, uint8_t chn)
{
	int ret_val = VSI_SUCCESS;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];

	if (isp_port->cam_device_handle) {
		//		ret_val =
		//		vsi_cam_device_sensor_drv_handle_un_register(isp_dev,
		//		isp_port->cam_device_handle);
		if (ret_val != VSI_SUCCESS) {
			dev_err(isp_dev->dev,
				"CamDevice unregister sensor driver Failed, "
				"ret is %d",
				ret_val);
			ret_val = VSI_ERR_TIMEOUT;
			return ret_val;
		}

		ret_val = vsi_cam_device_destroy(isp_dev,
						isp_port->cam_device_handle);
		if (ret_val != VSI_SUCCESS) {
			dev_err(isp_dev->dev,
				"CamDevice Isp Device handle destroy Failed, "
				"ret is %d",
				ret_val);
			ret_val = VSI_ERR_TIMEOUT;
			return ret_val;
		}
		isp_port->cam_device_handle = VSI_NULL;
	}
	return ret_val;
}

int isp_destroy_cam_device(struct visp_dev *isp_dev, uint8_t port,
			   uint8_t chn)
{

	if (isp_dev->isp_ports[port].ref_count)
		isp_dev->isp_ports[port].ref_count--;

	if (!isp_dev->isp_ports[port].ref_count)
		isp_device_destroy(isp_dev, port, chn);

	return VSI_SUCCESS;
}

int isp_destroy_pipeline(struct visp_dev *isp_dev, uint8_t port, uint8_t chn)
{
	isp_destroy_cam_device(isp_dev, port, chn);

	return VSI_SUCCESS;
}
EXPORT_SYMBOL_GPL(isp_destroy_pipeline);

int media_isp_stream_off(struct visp_dev *isp_dev, uint8_t port, uint8_t chn)
{
	int pad_index = (port * MEDIA_ISP_PORT_PAD_COUNT) + chn + 1;

	media_isp_device_stream_off(isp_dev, port, chn);
	if (strlen(isp_dev->isp_ports[port].fusa_json))
		visp_stop_fusa_event(isp_dev, pad_index);

	isp_destroy_pipeline(isp_dev, port, chn);

	return VSI_SUCCESS;
}
EXPORT_SYMBOL(media_isp_stream_off);

int media_isp_device_qbuf(struct visp_dev *isp_dev, uint8_t port, uint8_t chn,
			  media_buf *buf)
{
	int ret_val = VSI_SUCCESS;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	media_isp_chn_attr *IspChn = &isp_port->isp_chns[chn];
	media_buffer_t *p_media_buffer = VSI_NULL;

	p_media_buffer = IspChn->cam_device_bufs[buf->index];

	if (p_media_buffer == VSI_NULL) {
		dev_err(isp_dev->dev, "CamDevice queue buf is null");
		ret_val = VSI_ERR_NULL_PTR;
		return ret_val;
	}
	ret_val = vsi_cam_device_en_que_buffer(
		isp_dev, isp_port->cam_device_handle, chn, p_media_buffer);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "CamDevice queue buf failed, ret is %d",
			ret_val);
		ret_val = VSI_ERR_TIMEOUT;
		return ret_val;
	}

	return ret_val;
}

int media_isp_q_buf(struct visp_dev *isp_dev, int pad_index, media_buf *buf)
{
	int ret_val = VSI_SUCCESS;
	int port = pad_index / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad_index % MEDIA_ISP_PORT_PAD_COUNT) - 1;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	media_isp_chn_attr *IspChn = &isp_port->isp_chns[chn];
	//  dev_info(isp_dev->dev," %d %d %x %x\n",buf->index,buf->num_planes,
	//  buf->planes[0].dma_addr, buf->planes[0].dma_size );

	if (buf == NULL) {
		dev_err(isp_dev->dev, "got NULL BUFFER\n");
		return -1;
	}

	if (isp_dev->streamon[pad_index] == 0) {
		memcpy(&IspChn->bufs[buf->index], buf, sizeof(media_buf));
	} else {
		ret_val = media_isp_device_qbuf(isp_dev, port, chn, buf);
		if (ret_val != VSI_SUCCESS) {
			dev_err(isp_dev->dev,
				"port %d chn %d Qbuf failed, ret is %d", port,
				chn, ret_val);
		}
	}
	return ret_val;
}

static int media_isp_hal_media_fmt_to_m_bus_fmt(uint32_t *code,
						uint32_t *pixel_format)
{
	switch (*code) {
	case MEDIA_PIX_FMT_NV16:
		*pixel_format = MEDIA_BUS_FMT_YUYV8_2X8;
		break;
	case MEDIA_PIX_FMT_NV12:
		*pixel_format = MEDIA_BUS_FMT_YUYV8_1_5X8;
		break;
	case MEDIA_PIX_FMT_YUYV:
		*pixel_format = MEDIA_BUS_FMT_YUYV8_1X16;
		break;
	case MEDIA_PIX_FMT_GREY:
		*pixel_format = MEDIA_BUS_FMT_Y8_1X8;
		break;
	case MEDIA_PIX_FMT_Y10BPACK:
		*pixel_format = MEDIA_BUS_FMT_Y10_1X10;
		break;
	case MEDIA_PIX_FMT_Y10DWA:
		*pixel_format = MEDIA_BUS_FMT_Y10_1X10;
		break;
	case MEDIA_PIX_FMT_Y10:
		*pixel_format = MEDIA_BUS_FMT_Y10_1X10;
		break;
	case MEDIA_PIX_FMT_P00BPACK:
		*pixel_format = MEDIA_BUS_FMT_YUYV10_2X10;
		break;
	case MEDIA_PIX_FMT_P00DWA:
		*pixel_format = MEDIA_BUS_FMT_YUYV10_2X10;
		break;
	case MEDIA_PIX_FMT_P010:
		*pixel_format = MEDIA_BUS_FMT_YUYV10_2X10;
		break;
	case MEDIA_PIX_FMT_P02BPACK:
		*pixel_format = MEDIA_BUS_FMT_YUYV12_2X12;
		break;
	case MEDIA_PIX_FMT_P20BPACK:
		*pixel_format = MEDIA_BUS_FMT_YUYV10_2X10;
		break;
	case MEDIA_PIX_FMT_P20DWA:
		*pixel_format = MEDIA_BUS_FMT_YUYV10_2X10;
		break;
	case MEDIA_PIX_FMT_P210:
		*pixel_format = MEDIA_BUS_FMT_YUYV10_2X10;
		break;
	case MEDIA_PIX_FMT_P22BPACK:
		*pixel_format = MEDIA_BUS_FMT_YUYV12_2X12;
		break;
	case MEDIA_PIX_FMT_I20BPACK:
		*pixel_format = MEDIA_BUS_FMT_YUYV10_2X10;
		break;
	case MEDIA_PIX_FMT_I210:
		*pixel_format = MEDIA_BUS_FMT_YUYV10_2X10;
		break;
	case MEDIA_PIX_FMT_M48BPACK:
		*pixel_format = MEDIA_BUS_FMT_YUV8_1X24;
		break;
	case MEDIA_PIX_FMT_I48BPACK:
		*pixel_format = MEDIA_BUS_FMT_YUV8_1X24;
		break;
	case MEDIA_PIX_FMT_I48DWA:
		*pixel_format = MEDIA_BUS_FMT_YUV8_1X24;
		break;
	case MEDIA_PIX_FMT_I40DWA:
		*pixel_format = MEDIA_BUS_FMT_YUV8_1X24;
		break;
	case MEDIA_PIX_FMT_RGB24:
		*pixel_format = MEDIA_BUS_FMT_RGB888_1X24;
		break;
	case MEDIA_PIX_FMT_RGB24DWA:
		*pixel_format = MEDIA_BUS_FMT_RGB888_1X24;
		break;
	case MEDIA_PIX_FMT_RGB24P:
		*pixel_format = MEDIA_BUS_FMT_RGB888_3X8;
		break;
		;
	case MEDIA_PIX_FMT_SBGGR8:
		*pixel_format = MEDIA_BUS_FMT_SBGGR8_1X8;
		break;
	case MEDIA_PIX_FMT_SGBRG8:
		*pixel_format = MEDIA_BUS_FMT_SGBRG8_1X8;
		break;
	case MEDIA_PIX_FMT_SGRBG8:
		*pixel_format = MEDIA_BUS_FMT_SGRBG8_1X8;
		break;
	case MEDIA_PIX_FMT_SRGGB8:
		*pixel_format = MEDIA_BUS_FMT_SRGGB8_1X8;
		break;
	case MEDIA_PIX_FMT_SBGGR10:
		*pixel_format = MEDIA_BUS_FMT_SBGGR10_1X10;
		break;
	case MEDIA_PIX_FMT_SGBRG10:
		*pixel_format = MEDIA_BUS_FMT_SGBRG10_1X10;
		break;
	case MEDIA_PIX_FMT_SGRBG10:
		*pixel_format = MEDIA_BUS_FMT_SGRBG10_1X10;
		break;
	case MEDIA_PIX_FMT_SRGGB10:
		*pixel_format = MEDIA_BUS_FMT_SRGGB10_1X10;
		break;
	case MEDIA_PIX_FMT_SBGGR10BPACK:
		*pixel_format = MEDIA_BUS_FMT_SBGGR10_1X10;
		break;
	case MEDIA_PIX_FMT_SGBRG10BPACK:
		*pixel_format = MEDIA_BUS_FMT_SGBRG10_1X10;
		break;
	case MEDIA_PIX_FMT_SGRBG10BPACK:
		*pixel_format = MEDIA_BUS_FMT_SGRBG10_1X10;
		break;
	case MEDIA_PIX_FMT_SRGGB10BPACK:
		*pixel_format = MEDIA_BUS_FMT_SRGGB10_1X10;
		break;
	case MEDIA_PIX_FMT_SBGGR10DWA:
		*pixel_format = MEDIA_BUS_FMT_SBGGR10_1X10;
		break;
	case MEDIA_PIX_FMT_SGBRG10DWA:
		*pixel_format = MEDIA_BUS_FMT_SGBRG10_1X10;
		break;
	case MEDIA_PIX_FMT_SGRBG10DWA:
		*pixel_format = MEDIA_BUS_FMT_SGRBG10_1X10;
		break;
	case MEDIA_PIX_FMT_SRGGB10DWA:
		*pixel_format = MEDIA_BUS_FMT_SRGGB10_1X10;
		break;
	case MEDIA_PIX_FMT_SBGGR12:
		*pixel_format = MEDIA_BUS_FMT_SBGGR12_1X12;
		break;
	case MEDIA_PIX_FMT_SGBRG12:
		*pixel_format = MEDIA_BUS_FMT_SGBRG12_1X12;
		break;
	case MEDIA_PIX_FMT_SGRBG12:
		*pixel_format = MEDIA_BUS_FMT_SGRBG12_1X12;
		break;
	case MEDIA_PIX_FMT_SRGGB12:
		*pixel_format = MEDIA_BUS_FMT_SRGGB12_1X12;
		break;
	case MEDIA_PIX_FMT_SBGGR12BPACK:
		*pixel_format = MEDIA_BUS_FMT_SBGGR12_1X12;
		break;
	case MEDIA_PIX_FMT_SGBRG12BPACK:
		*pixel_format = MEDIA_BUS_FMT_SGBRG12_1X12;
		break;
	case MEDIA_PIX_FMT_SGRBG12BPACK:
		*pixel_format = MEDIA_BUS_FMT_SGRBG12_1X12;
		break;
	case MEDIA_PIX_FMT_SRGGB12BPACK:
		*pixel_format = MEDIA_BUS_FMT_SRGGB12_1X12;
		break;
	case MEDIA_PIX_FMT_SBGGR12DWA:
		*pixel_format = MEDIA_BUS_FMT_SBGGR12_1X12;
		break;
	case MEDIA_PIX_FMT_SGBRG12DWA:
		*pixel_format = MEDIA_BUS_FMT_SGBRG12_1X12;
		break;
	case MEDIA_PIX_FMT_SGRBG12DWA:
		*pixel_format = MEDIA_BUS_FMT_SGRBG12_1X12;
		break;
	case MEDIA_PIX_FMT_SRGGB12DWA:
		*pixel_format = MEDIA_BUS_FMT_SRGGB12_1X12;
		break;
	case MEDIA_PIX_FMT_SBGGR14BPACK:
		*pixel_format = MEDIA_BUS_FMT_SBGGR14_1X14;
		break;
	case MEDIA_PIX_FMT_SGBRG14BPACK:
		*pixel_format = MEDIA_BUS_FMT_SGBRG14_1X14;
		break;
	case MEDIA_PIX_FMT_SGRBG14BPACK:
		*pixel_format = MEDIA_BUS_FMT_SGRBG14_1X14;
		break;
	case MEDIA_PIX_FMT_SRGGB14BPACK:
		*pixel_format = MEDIA_BUS_FMT_SRGGB14_1X14;
		break;
	case MEDIA_PIX_FMT_SBGGR14DWA:
		*pixel_format = MEDIA_BUS_FMT_SBGGR14_1X14;
		break;
	case MEDIA_PIX_FMT_SGBRG14DWA:
		*pixel_format = MEDIA_BUS_FMT_SGBRG14_1X14;
		break;
	case MEDIA_PIX_FMT_SGRBG14DWA:
		*pixel_format = MEDIA_BUS_FMT_SGRBG14_1X14;
		break;
	case MEDIA_PIX_FMT_SRGGB14DWA:
		*pixel_format = MEDIA_BUS_FMT_SRGGB14_1X14;
		break;
	case MEDIA_PIX_FMT_SBGGR14:
		*pixel_format = MEDIA_BUS_FMT_SBGGR14_1X14;
		break;
	case MEDIA_PIX_FMT_SGBRG14:
		*pixel_format = MEDIA_BUS_FMT_SGBRG14_1X14;
		break;
	case MEDIA_PIX_FMT_SGRBG14:
		*pixel_format = MEDIA_BUS_FMT_SGRBG14_1X14;
		break;
	case MEDIA_PIX_FMT_SRGGB14:
		*pixel_format = MEDIA_BUS_FMT_SRGGB14_1X14;
		break;
	case MEDIA_PIX_FMT_SBGGR16:
		*pixel_format = MEDIA_BUS_FMT_SBGGR16_1X16;
		break;
	case MEDIA_PIX_FMT_SGBRG16:
		*pixel_format = MEDIA_BUS_FMT_SGBRG16_1X16;
		break;
	case MEDIA_PIX_FMT_SGRBG16:
		*pixel_format = MEDIA_BUS_FMT_SGRBG16_1X16;
		break;
	case MEDIA_PIX_FMT_SRGGB16:
		*pixel_format = MEDIA_BUS_FMT_SRGGB16_1X16;
		break;
	case MEDIA_PIX_FMT_SBGGR24:
		*pixel_format = MEDIA_BUS_FMT_SBGGR24_1X24;
		break;
	case MEDIA_PIX_FMT_SGBRG24:
		*pixel_format = MEDIA_BUS_FMT_SGBRG24_1X24;
		break;
	case MEDIA_PIX_FMT_SGRBG24:
		*pixel_format = MEDIA_BUS_FMT_SGRBG24_1X24;
		break;
	case MEDIA_PIX_FMT_SRGGB24:
		*pixel_format = MEDIA_BUS_FMT_SRGGB24_1X24;
		break;
	default:
		return VSI_ERR_ILLEGAL_PARAM;
	}

	return VSI_SUCCESS;
}

int media_isp_hal_mbus_fmt_to_media_fmt(uint32_t *code, uint32_t *pixel_format,
					uint32_t fourcc)
{
	uint32_t new_code = 0;

	switch (*code) {
	case MEDIA_BUS_FMT_YUYV8_2X8:
		*pixel_format = MEDIA_PIX_FMT_NV16;
		break;
	case MEDIA_BUS_FMT_YUYV8_1_5X8:
		*pixel_format = MEDIA_PIX_FMT_NV12;
		break;
	case MEDIA_BUS_FMT_YUYV8_1X16:
		*pixel_format = MEDIA_PIX_FMT_YUYV;
		break;
	case MEDIA_BUS_FMT_Y8_1X8:
		*pixel_format = MEDIA_PIX_FMT_GREY;
		break;
	case MEDIA_BUS_FMT_RGB888_3X8:
		*pixel_format = MEDIA_PIX_FMT_RGB24P;
		break;
	case MEDIA_BUS_FMT_SBGGR8_1X8:
		*pixel_format = MEDIA_PIX_FMT_SBGGR8;
		break;
	case MEDIA_BUS_FMT_SGBRG8_1X8:
		*pixel_format = MEDIA_PIX_FMT_SGBRG8;
		break;
	case MEDIA_BUS_FMT_SGRBG8_1X8:
		*pixel_format = MEDIA_PIX_FMT_SGRBG8;
		break;
	case MEDIA_BUS_FMT_SRGGB8_1X8:
		*pixel_format = MEDIA_PIX_FMT_SRGGB8;
		break;
	case MEDIA_BUS_FMT_Y10_1X10:
	case MEDIA_BUS_FMT_YUYV10_2X10:
	case MEDIA_BUS_FMT_YUYV12_2X12:
	case MEDIA_BUS_FMT_YUV8_1X24:
	case MEDIA_BUS_FMT_RGB888_1X24:
	case MEDIA_BUS_FMT_SBGGR10_1X10:
	case MEDIA_BUS_FMT_SGBRG10_1X10:
	case MEDIA_BUS_FMT_SGRBG10_1X10:
	case MEDIA_BUS_FMT_SRGGB10_1X10:
	case MEDIA_BUS_FMT_SBGGR12_1X12:
	case MEDIA_BUS_FMT_SGBRG12_1X12:
	case MEDIA_BUS_FMT_SGRBG12_1X12:
	case MEDIA_BUS_FMT_SRGGB12_1X12:
	case MEDIA_BUS_FMT_SBGGR14_1X14:
	case MEDIA_BUS_FMT_SGBRG14_1X14:
	case MEDIA_BUS_FMT_SGRBG14_1X14:
	case MEDIA_BUS_FMT_SRGGB14_1X14:
	case MEDIA_BUS_FMT_SBGGR16_1X16:
	case MEDIA_BUS_FMT_SGBRG16_1X16:
	case MEDIA_BUS_FMT_SGRBG16_1X16:
	case MEDIA_BUS_FMT_SRGGB16_1X16:
	case MEDIA_BUS_FMT_SBGGR24_1X24:
	case MEDIA_BUS_FMT_SGBRG24_1X24:
	case MEDIA_BUS_FMT_SGRBG24_1X24:
	case MEDIA_BUS_FMT_SRGGB24_1X24:
		*pixel_format = fourcc;
		break;
	default:
		return VSI_ERR_ILLEGAL_PARAM;
	}

	/* double check fourcc and media bus format*/
	media_isp_hal_media_fmt_to_m_bus_fmt(&new_code, pixel_format);
	if (new_code != *code)
		return VSI_ERR_ILLEGAL_PARAM;

	return VSI_SUCCESS;
}

int media_isp_hal_set_fmt(struct visp_dev *isp_dev, int pad,
			  media_fmt *format)
{
	struct v4l2_subdev_format *SdFmt;
	int ret_val = 0;

	SdFmt = kmalloc(sizeof(SdFmt), GFP_KERNEL);
	if (!SdFmt) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}
	memset(SdFmt, 0, sizeof(struct v4l2_subdev_format));

	SdFmt->pad = pad, SdFmt->which = V4L2_SUBDEV_FORMAT_ACTIVE;
	SdFmt->format.width = format->width;
	SdFmt->format.height = format->height;
	SdFmt->format.colorspace = format->color_space;
	SdFmt->format.quantization = format->quantization;

	if (sizeof(SdFmt->format.reserved) == (sizeof(uint16_t) * 10)) {
		memcpy(SdFmt->format.reserved, &format->pixel_format,
			   sizeof(format->pixel_format));
	} else {
		memcpy(&SdFmt->format.reserved[1], &format->pixel_format,
			   sizeof(format->pixel_format));
	}

	ret_val = media_isp_hal_media_fmt_to_m_bus_fmt(&format->pixel_format,
							  &SdFmt->format.code);
	if (ret_val != 0) {
		kfree(SdFmt);
		dev_err(isp_dev->dev, "%s: media_isp_hal_set_fmt failed %d",
			__func__, ret_val);
		return ret_val;
	}

	ret_val = visp_set_fmt_public(isp_dev, SdFmt);
	if (ret_val != 0) {
		kfree(SdFmt);
		dev_err(isp_dev->dev, " %s: visp_set_fmt_public failed %d",
			__func__, ret_val);
		return ret_val;
	}
	// TODO - FREE SdFMt check if it is not neded after this
	kfree(SdFmt);
	return ret_val;
}
EXPORT_SYMBOL(media_isp_hal_set_fmt);

static int media_fmt_to_isp_fmt(uint32_t *media_fmt,
				cam_device_pipe_out_fmt_t *IspFmt)
{
	switch (*media_fmt) {
	case MEDIA_PIX_FMT_YUYV:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV422I;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_NV16:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV422SP;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_NV12:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV420SP;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_GREY:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV400;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_Y10BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV400;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_Y10DWA:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV400_ALIGNED_MODE0;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_Y10:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV400_ALIGNED_MODE1;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_P00BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV420SP;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_P00DWA:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE0;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_P010:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE1;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_P02BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV420SP;
		IspFmt->data_bits = 12;
		break;
	case MEDIA_PIX_FMT_P20BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV422SP;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_P20DWA:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE0;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_P210:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE1;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_P22BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV422SP;
		IspFmt->data_bits = 12;
		break;
	case MEDIA_PIX_FMT_I20BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV422I;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_I210:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV422I_ALIGNED_MODE1;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_M48BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV444P;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_I48BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV444I;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_I48DWA:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV444I_ALIGNED_MODE0;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_I40DWA:
		IspFmt->out_format = CAMDEV_PIX_FMT_YUV444I_ALIGNED_MODE0;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_RGB24:
		IspFmt->out_format = CAMDEV_PIX_FMT_RGB888;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_RGB24DWA:
		IspFmt->out_format = CAMDEV_PIX_FMT_RGB888_ALIGNED_MODE0;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_RGB24P:
		IspFmt->out_format = CAMDEV_PIX_FMT_RGB888P;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_SBGGR8:
	case MEDIA_PIX_FMT_SGBRG8:
	case MEDIA_PIX_FMT_SGRBG8:
	case MEDIA_PIX_FMT_SRGGB8:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW8;
		IspFmt->data_bits = 8;
		break;
	case MEDIA_PIX_FMT_SBGGR10:
	case MEDIA_PIX_FMT_SGBRG10:
	case MEDIA_PIX_FMT_SGRBG10:
	case MEDIA_PIX_FMT_SRGGB10:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE1;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_SBGGR10BPACK:
	case MEDIA_PIX_FMT_SGBRG10BPACK:
	case MEDIA_PIX_FMT_SGRBG10BPACK:
	case MEDIA_PIX_FMT_SRGGB10BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW10;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_SBGGR10DWA:
	case MEDIA_PIX_FMT_SGBRG10DWA:
	case MEDIA_PIX_FMT_SGRBG10DWA:
	case MEDIA_PIX_FMT_SRGGB10DWA:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE0;
		IspFmt->data_bits = 10;
		break;
	case MEDIA_PIX_FMT_SBGGR12:
	case MEDIA_PIX_FMT_SGBRG12:
	case MEDIA_PIX_FMT_SGRBG12:
	case MEDIA_PIX_FMT_SRGGB12:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE1;
		IspFmt->data_bits = 12;
		break;
	case MEDIA_PIX_FMT_SBGGR12BPACK:
	case MEDIA_PIX_FMT_SGBRG12BPACK:
	case MEDIA_PIX_FMT_SGRBG12BPACK:
	case MEDIA_PIX_FMT_SRGGB12BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW12;
		IspFmt->data_bits = 12;
		break;
	case MEDIA_PIX_FMT_SBGGR12DWA:
	case MEDIA_PIX_FMT_SGBRG12DWA:
	case MEDIA_PIX_FMT_SGRBG12DWA:
	case MEDIA_PIX_FMT_SRGGB12DWA:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE0;
		IspFmt->data_bits = 12;
		break;
	case MEDIA_PIX_FMT_SBGGR14:
	case MEDIA_PIX_FMT_SGBRG14:
	case MEDIA_PIX_FMT_SGRBG14:
	case MEDIA_PIX_FMT_SRGGB14:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE1;
		IspFmt->data_bits = 14;
		break;
	case MEDIA_PIX_FMT_SBGGR14BPACK:
	case MEDIA_PIX_FMT_SGBRG14BPACK:
	case MEDIA_PIX_FMT_SGRBG14BPACK:
	case MEDIA_PIX_FMT_SRGGB14BPACK:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW14;
		IspFmt->data_bits = 14;
		break;
	case MEDIA_PIX_FMT_SBGGR14DWA:
	case MEDIA_PIX_FMT_SGBRG14DWA:
	case MEDIA_PIX_FMT_SGRBG14DWA:
	case MEDIA_PIX_FMT_SRGGB14DWA:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE0;
		IspFmt->data_bits = 14;
		break;
	case MEDIA_PIX_FMT_SBGGR16:
	case MEDIA_PIX_FMT_SGBRG16:
	case MEDIA_PIX_FMT_SGRBG16:
	case MEDIA_PIX_FMT_SRGGB16:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW16;
		IspFmt->data_bits = 16;
		break;
	case MEDIA_PIX_FMT_SBGGR24:
	case MEDIA_PIX_FMT_SGBRG24:
	case MEDIA_PIX_FMT_SGRBG24:
	case MEDIA_PIX_FMT_SRGGB24:
		IspFmt->out_format = CAMDEV_PIX_FMT_RAW24;
		IspFmt->data_bits = 24;
		break;
	default:
		printk(KERN_ERR "Not support format %s", (char *)media_fmt);
		return VSI_ERR_NOT_SUPPORT;
	}
	return VSI_SUCCESS;
}

//

int media_isp_device_set_format(struct visp_dev *isp_dev, uint8_t port,
				uint8_t chn)
{
	/************ STEP 2 Streamon --> SetOutFormat******************/
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port]; //
	int ret_val = 0;

	cam_device_pipe_out_fmt_t IspFormat;

	memset(&IspFormat, 0, sizeof(IspFormat));
	IspFormat.out_width = isp_dev->cap_w;
	IspFormat.out_height = isp_dev->cap_h;
	IspFormat.path_out_type = 0;

	ret_val = media_fmt_to_isp_fmt(&isp_dev->cap_fmt, &IspFormat);

	ret_val = vsi_cam_device_set_out_format(
		isp_dev, isp_port->cam_device_handle, chn, &IspFormat);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "CamDevice set format failed, ret is %d",
			ret_val);
		dev_err(isp_dev->dev,
			"port %d chn %d set format failed, ret is %d", port,
			chn, ret_val);
		ret_val = VSI_ERR_TIMEOUT;
		goto ERR_TO_CAMERA_DISCONNECT;
	}

	return ret_val;
ERR_TO_CAMERA_DISCONNECT:
	// RANJITH ADD the cleanup later
	return ret_val;
}
EXPORT_SYMBOL(media_isp_device_set_format);

int media_isp_calib_get_mode_info(struct visp_dev *isp_dev, uint8_t port,
				  cam_device_sensor_mode_info_t *mode_info)
{
	int ret_val = VSI_SUCCESS;
	media_isp_port_attr *isp_port = VSI_NULL;

	if (!mode_info) {
		dev_err(isp_dev->dev, "%s: null pointer of mode_info",
			__func__);
		return VSI_ERR_NULL_PTR;
	}

	isp_port = &isp_dev->isp_ports[port];
	memcpy(mode_info, &isp_port->sensor_info.mode_info,
		   sizeof(isp_port->sensor_info.mode_info));

	return ret_val;
}

static int media_isp_device_sub_module_init(
	struct visp_dev *isp_dev, uint8_t port,
	cam_device_pipe_submodule_ctrl_u *sub_module_init)
{
	int ret_val = VSI_SUCCESS;
	cam_device_sensor_mode_info_t mode_info;

	memset(&mode_info, 0, sizeof(mode_info));

	sub_module_init->all_ctrl = 0xFFFFFFFF;
	sub_module_init->sub_ctrl.rgbir_enable = 0;
	sub_module_init->sub_ctrl.hdr_enable = 0;
	sub_module_init->sub_ctrl.pdaf_enable = 0;
	sub_module_init->sub_ctrl.dpf_enable = 0;
	sub_module_init->sub_ctrl.compress_enable = 0;
	sub_module_init->sub_ctrl.expand_enable = 0;
	sub_module_init->sub_ctrl.ee_enable = 0;
	sub_module_init->sub_ctrl.ynr_enable = 0;
	sub_module_init->sub_ctrl.cnr_enable = 0;
	sub_module_init->sub_ctrl.lut3d_enable = 0;
	sub_module_init->sub_ctrl.dnr2_enable = 0;
	sub_module_init->sub_ctrl.dnr3_enable = 0;
	sub_module_init->sub_ctrl.gtm_enable = 0;
	sub_module_init->sub_ctrl.wdr_enable = 0;
	sub_module_init->sub_ctrl.lsc_enable = 0;

	ret_val = media_isp_calib_get_mode_info(isp_dev, port, &mode_info);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "%s: get sensor mode failed, ret is %d",
			__func__, ret_val);
		return ret_val;
	}

	if (mode_info.sensor_type == CAMDEV_SENSOR_TYPE_STITCHING_HDR)
		sub_module_init->sub_ctrl.hdr_enable = 1;

	return ret_val;
}

int media_isp_calib_get_sensor_name(struct visp_dev *isp_dev, uint8_t port,
					char *sensor_name)
{
	media_isp_port_attr *isp_port = VSI_NULL;
	int ret_val = VSI_SUCCESS;

	if (!sensor_name) {
		dev_err(isp_dev->dev, "%s: null pointer of sensor_name",
			__func__);
		return VSI_ERR_NULL_PTR;
	}

	isp_port = &isp_dev->isp_ports[port];

	dev_info(isp_dev->dev, "%s: isp : %d", __func__, isp_dev->id);
	dev_info(isp_dev->dev, "%s: port: %d", __func__, port);
	dev_info(isp_dev->dev, "%s: name: %s", __func__,
		 isp_port->sensor_info.name);
	dev_info(isp_dev->dev, "%s: mode: %d", __func__,
		 isp_port->sensor_info.mode);
	dev_info(isp_dev->dev, "%s: xml : %s", __func__,
		 isp_port->sensor_info.calib);
	dev_info(isp_dev->dev, "%s: manu_json: %s", __func__,
		 isp_port->sensor_info.manu_json);
	dev_info(isp_dev->dev, "%s: auto_json: %s", __func__,
		 isp_port->sensor_info.auto_json);

	if (strlen(isp_port->sensor_info.name)) {
		strscpy(sensor_name, isp_port->sensor_info.name,
			sizeof(sensor_name));
	} else {
		dev_err(isp_dev->dev, "%s: get null string of sensor name",
			__func__);
		ret_val = VSI_ERR_ILLEGAL_PARAM;
	}

	return ret_val;
}

int media_isp_calib_get_sensor_mode(struct visp_dev *isp_dev, uint8_t port,
					uint8_t *sensor_mode)
{
	int ret_val = VSI_SUCCESS;
	media_isp_port_attr *isp_port = VSI_NULL;

	if (!sensor_mode) {
		dev_err(isp_dev->dev, "%s: null pointer of sensor_mode",
			__func__);
		return VSI_ERR_NULL_PTR;
	}

	isp_port = &isp_dev->isp_ports[port];
	*sensor_mode = isp_port->sensor_info.mode;

	return ret_val;
}

int media_isp_device_sensor_open(struct visp_dev *isp_dev, uint8_t port)
{
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	int ret_val = VSI_SUCCESS;
	uint8_t mode_index = 0;
	char sensor_name[MEDIA_ISP_CHAR_LENGTH_MAX];

	memset(sensor_name, 0, sizeof(sensor_name));

	ret_val = media_isp_calib_get_sensor_mode(isp_dev, port, &mode_index);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "%s: get sensor mode failed, ret is %d",
			__func__, ret_val);
	}
	ret_val = vsi_cam_device_sensor_open(
		isp_dev, isp_port->cam_device_handle, (uint32_t)mode_index);
	if (ret_val != VSI_SUCCESS) {
		dev_err(
			isp_dev->dev,
			"CamDevice open sensor %s mode %d driver Failed, ret is %d",
			sensor_name, mode_index, ret_val);
	}

	return ret_val;
}

int media_isp_device_mcm_set_format(struct visp_dev *isp_dev, uint8_t port)
{
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	int ret_val = VSI_SUCCESS;
	cam_device_pipe_in_fmt_t InFormat;
	cam_device_sensor_mode_info_t mode_info;
	cam_device_pipe_in_path_type_t InPath = CAMDEV_PIPE_INPATH_RDMA;

	memset(&InFormat, 0, sizeof(cam_device_pipe_in_fmt_t));
	memset(&mode_info, 0, sizeof(cam_device_sensor_mode_info_t));

	ret_val = media_isp_calib_get_mode_info(isp_dev, port, &mode_info);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"%s: port %d get sensor mode info failed, ret is %d",
			__func__, port, ret_val);
		return ret_val;
	}
	InFormat.in_width = isp_dev->out_w;
	InFormat.in_height = isp_dev->out_h;
	InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_RGGB;
	InFormat.stitch_mode =
		(cam_device_stitching_mode_t)CAMDEV_SENSOR_TYPE_LINEAR;

	if (mode_info.sensor_type == CAMDEV_SENSOR_TYPE_STITCHING_HDR) {
		// hardware limit, may cause accuracy loss for bitwidth
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW16;
	} else {
		switch (mode_info.bit_width) {
		case 8:
			InFormat.in_format = CAMDEV_INPUT_FMT_RAW8;
			break;
		case 10:
			InFormat.in_format = CAMDEV_INPUT_FMT_RAW10;
			break;
		case 12:
			InFormat.in_format = CAMDEV_INPUT_FMT_RAW12;
			break;
		case 14:
			InFormat.in_format = CAMDEV_INPUT_FMT_RAW14;
			break;
		case 16:
			InFormat.in_format = CAMDEV_INPUT_FMT_RAW16;
			break;
		default:
			InFormat.in_format = CAMDEV_INPUT_FMT_RAW16;
			break;
		}
	}

	if (isp_dev->out_fmt == V4L2_PIX_FMT_SRGGB10)
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
	else if (isp_dev->out_fmt == V4L2_PIX_FMT_SRGGB12)
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
	else if (isp_dev->out_fmt == V4L2_PIX_FMT_SRGGB8)
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW8;
	else
		InFormat.in_format = isp_dev->out_fmt;

	switch (isp_dev->out_fmt) {
	case V4L2_PIX_FMT_SRGGB8:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_RGGB;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW8;
		break;
	case V4L2_PIX_FMT_SRGGB10:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_RGGB;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SRGGB12:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_RGGB;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SRGGB14:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_RGGB;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW14_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SRGGB16:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_RGGB;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW16;
		break;
	case V4L2_PIX_FMT_SBGGR8:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_BGGR;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW8;
		break;
	case V4L2_PIX_FMT_SBGGR10:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_BGGR;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SBGGR12:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_BGGR;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SBGGR14:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_BGGR;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW14_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SBGGR16:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_BGGR;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW16;
		break;
	case V4L2_PIX_FMT_SGBRG8:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_GBRG;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW8;
		break;
	case V4L2_PIX_FMT_SGBRG10:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_GBRG;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SGBRG12:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_GBRG;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SGBRG14:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_GBRG;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW14_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SGBRG16:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_GBRG;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW16;
		break;
	case V4L2_PIX_FMT_SGRBG8:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_GRBG;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW8;
		break;
	case V4L2_PIX_FMT_SGRBG10:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_GRBG;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SGRBG12:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_GRBG;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SGRBG14:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_GRBG;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW14_ALIGNED1;
		break;
	case V4L2_PIX_FMT_SGRBG16:
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_GRBG;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW16;
		break;
	default:
		dev_err(isp_dev->dev, "unsupported in format");
		InFormat.in_pattern = CAMDEV_RAW_RGB_PAT_RGGB;
		InFormat.in_format = CAMDEV_INPUT_FMT_RAW12;
		break;
	}

	ret_val = vsi_cam_device_set_in_format(
		isp_dev, isp_port->cam_device_handle, InPath, &InFormat);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"CamDevice set input path %d format failed, ret is %d",
			InPath, ret_val);
		ret_val = VSI_ERR_ILLEGAL_PARAM;
		;
	}

	return ret_val;
}
EXPORT_SYMBOL(media_isp_device_mcm_set_format);

int media_isp_device_camera_connect(struct visp_dev *isp_dev, uint8_t index)
{
	int port = index / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (index % MEDIA_ISP_PORT_PAD_COUNT) - 1;

	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	int ret_val = VSI_SUCCESS;
	cam_device_pipe_submodule_ctrl_u sub_module_init;

	memset(&sub_module_init, 0, sizeof(sub_module_init));

	ret_val =
		media_isp_device_sub_module_init(isp_dev, port, &sub_module_init);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			" port %d chn %d init submodule failed, ret is %d",
			port, chn, ret_val);
		goto exit;
	}

	ret_val = vsi_cam_device_connect_camera(
		isp_dev, isp_port->cam_device_handle, &sub_module_init);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"CamDevice camera connect failed, ret is %d", ret_val);
		ret_val = VSI_ERR_ILLEGAL_PARAM;
	}
exit:
	return ret_val;
}

int visp_buf_done(struct v4l2_subdev *sd, void *arg);

int media_isp_hal_buf_done(struct v4l2_subdev *sd, int pad,
			   const media_buf *buf)
{
	struct visp_dev *isp_dev = v4l2_get_subdevdata(sd);
	struct visp_buf KernelBuf;
	uint32_t i;
	int ret_val = 0;

	if (!buf || !isp_dev) {
		dev_err(isp_dev->dev, "Got a NULL BUFFER BUFFER\n");
		return -EINVAL;
	}
	KernelBuf.index = buf->index;
	KernelBuf.num_planes = buf->num_planes;
	for (i = 0; i < KernelBuf.num_planes; i++) {
		KernelBuf.planes[i].dma_addr = buf->planes[i].dma_addr;
		KernelBuf.planes[i].size = buf->planes[i].dma_size;
	}
	KernelBuf.pad = pad;

	ret_val = visp_buf_done(sd, &KernelBuf);
	if (ret_val != 0)
		return ret_val;
	return VSI_SUCCESS;
}

static int read_dq_buf_info(void *data, media_buffer_t *p_media_buffer,
			struct Chn_info *info, int *instance_id)
{
	uint8_t *p_data = NULL;
	uint32_t hw_id_t = 100;
	payload_packet *packet = data;

	if (!packet) {
		loge("NO data in DQ Payload%s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	p_data = packet->payload;

	memcpy(&(hw_id_t), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	*instance_id = hw_id_t;
	memcpy(info, p_data, sizeof(struct Chn_info));
	p_data += sizeof(struct Chn_info);
	p_media_buffer->p_meta_data =
		kzalloc(APU_META_DATA_SIZE, GFP_KERNEL);
	if (!(p_media_buffer->p_meta_data)) {
		loge("FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}

	/* memcpy(p_media_buffer->p_meta_data, p_data,
	 * sizeof(pic_buf_meta_data_t));
	 * p_data += sizeof(pic_buf_meta_data_t);
	 */

	memcpy(p_media_buffer->p_meta_data, p_data, APU_META_DATA_SIZE);
	p_data += APU_META_DATA_SIZE;

	memcpy(&(p_media_buffer->base_address), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(p_media_buffer->base_size), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(p_media_buffer->lock_count), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(p_media_buffer->is_full), p_data, sizeof(bool_t));
	p_data += sizeof(bool_t);

	memcpy(&(p_media_buffer->index), p_data, sizeof(uint8_t));
	p_data += sizeof(uint8_t);

	memcpy(&(p_media_buffer->buf_mode), p_data, sizeof(buff_mode));
	p_data += sizeof(buff_mode);

	memcpy(&(p_media_buffer->p_ipl_address), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&(p_media_buffer->p_owner), p_data, sizeof(uint32_t));
	return 0;
}

int media_isp_device_dq_buf_out(struct visp_dev *isp_dev, struct Chn_info *info,
				void *packet_from_rpu,
				media_buffer_t *p_media_buffer)
{
	int ret_val = VSI_SUCCESS;
	int instance_id;

	if (!packet_from_rpu) {
		dev_err(isp_dev->dev, "Received Null data %s %d\n", __func__,
			__LINE__);
		return -ENOMEM;
	}

	ret_val = read_dq_buf_info(packet_from_rpu, p_media_buffer, info, &instance_id);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "failed to read dq_buf info %d", ret_val);
		return ret_val;
	}

	isp_dev->isp_dq_out_index = p_media_buffer->index;

	cam_device_context_t p_cam_dev_ctx;

	p_cam_dev_ctx.isp_hw_id = isp_dev->id;
	p_cam_dev_ctx.isp_vt_id = 0;
	p_cam_dev_ctx.instance_id = instance_id;
	p_cam_dev_ctx.cookie = 99;

	ret_val = vsi_cam_device_en_que_buffer(
		isp_dev, &p_cam_dev_ctx, CAMDEV_BUFCHAIN_MP, p_media_buffer);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "VsiCamDeviceEnQueBuffer failed %d",
			ret_val);
		ret_val = VSI_ERR_TIMEOUT;
		return ret_val;
	}

	return ret_val;
}
EXPORT_SYMBOL(media_isp_device_dq_buf_out);

int media_isp_set_format(struct visp_dev *isp_dev, uint32_t pad_index,
			 media_fmt format_t)
{
	int port = pad_index / MEDIA_ISP_PORT_PAD_COUNT;
	int chn = (pad_index % MEDIA_ISP_PORT_PAD_COUNT) - 1;

	memcpy(&isp_dev->isp_ports[port].isp_chns[chn].format, &format_t,
		   sizeof(media_fmt));

	return VSI_SUCCESS;
}

static int media_isp_device_get_port_sink_info(struct visp_dev *isp_dev,
						   uint8_t port,
						   media_sink_info *sink_info)
{
	int ret_val = VSI_SUCCESS;
	cam_device_sensor_mode_info_t ModeInfo_s = {0};
	cam_device_sensor_mode_info_t *mode_info = &ModeInfo_s;

	ret_val = media_isp_calib_get_mode_info(isp_dev, port, mode_info);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"%s: get sensor mode info failed, ret is %d", __func__,
			ret_val);
		return ret_val;
	}

	sink_info->rect.top = mode_info->size.top;
	sink_info->rect.left = mode_info->size.left;
	sink_info->rect.width = mode_info->size.width;
	sink_info->rect.height = mode_info->size.height;
	sink_info->frmival_min.numerator = 1;
	sink_info->frmival_min.denominator = 1;
	sink_info->frmival_max.numerator = 30;
	sink_info->frmival_max.denominator = 1;

	switch (mode_info->bayer_pattern) {
	case CAMDEV_RAW_RGBIR_PAT_RGGIR:
	case CAMDEV_RAW_RGBIR_PAT_RGIRB:
	case CAMDEV_RAW_RGBIR_PAT_IRGGB:
	case CAMDEV_RAW_RGBIR_PAT_RIRGB:
	case CAMDEV_RAW_RGB_PAT_RGGB:
		if (mode_info->bit_width == 8)
			sink_info->fourcc = MEDIA_PIX_FMT_SRGGB8;
		else if (mode_info->bit_width == 10)
			sink_info->fourcc = MEDIA_PIX_FMT_SRGGB10;
		else if (mode_info->bit_width == 12)
			sink_info->fourcc = MEDIA_PIX_FMT_SRGGB12;
		else if (mode_info->bit_width == 14)
			sink_info->fourcc = MEDIA_PIX_FMT_SRGGB14;
		else
			sink_info->fourcc = MEDIA_PIX_FMT_SRGGB16;
		break;
	case CAMDEV_RAW_RGBIR_PAT_GRIRG:
	case CAMDEV_RAW_RGBIR_PAT_GIRBG:
	case CAMDEV_RAW_RGBIR_PAT_GRBIR:
	case CAMDEV_RAW_RGBIR_PAT_IRRBG:
	case CAMDEV_RAW_RGB_PAT_GRBG:
		if (mode_info->bit_width == 8)
			sink_info->fourcc = MEDIA_PIX_FMT_SGRBG8;
		else if (mode_info->bit_width == 10)
			sink_info->fourcc = MEDIA_PIX_FMT_SGRBG10;
		else if (mode_info->bit_width == 12)
			sink_info->fourcc = MEDIA_PIX_FMT_SGRBG12;
		else if (mode_info->bit_width == 14)
			sink_info->fourcc = MEDIA_PIX_FMT_SGRBG14;
		else
			sink_info->fourcc = MEDIA_PIX_FMT_SGRBG16;
		break;
	case CAMDEV_RAW_RGBIR_PAT_GBIRG:
	case CAMDEV_RAW_RGBIR_PAT_GIRRG:
	case CAMDEV_RAW_RGBIR_PAT_IRBRG:
	case CAMDEV_RAW_RGBIR_PAT_GBRIR:
	case CAMDEV_RAW_RGB_PAT_GBRG:
		if (mode_info->bit_width == 8)
			sink_info->fourcc = MEDIA_PIX_FMT_SGBRG8;
		else if (mode_info->bit_width == 10)
			sink_info->fourcc = MEDIA_PIX_FMT_SGBRG10;
		else if (mode_info->bit_width == 12)
			sink_info->fourcc = MEDIA_PIX_FMT_SGBRG12;
		else if (mode_info->bit_width == 14)
			sink_info->fourcc = MEDIA_PIX_FMT_SGBRG14;
		else
			sink_info->fourcc = MEDIA_PIX_FMT_SGBRG16;
		break;
	case CAMDEV_RAW_RGBIR_PAT_BGGIR:
	case CAMDEV_RAW_RGBIR_PAT_IRGGR:
	case CAMDEV_RAW_RGBIR_PAT_BIRGR:
	case CAMDEV_RAW_RGBIR_PAT_BGIRR:
	case CAMDEV_RAW_RGB_PAT_BGGR:
		if (mode_info->bit_width == 8)
			sink_info->fourcc = MEDIA_PIX_FMT_SBGGR8;
		else if (mode_info->bit_width == 10)
			sink_info->fourcc = MEDIA_PIX_FMT_SBGGR10;
		else if (mode_info->bit_width == 12)
			sink_info->fourcc = MEDIA_PIX_FMT_SBGGR12;
		else if (mode_info->bit_width == 14)
			sink_info->fourcc = MEDIA_PIX_FMT_SBGGR14;
		else
			sink_info->fourcc = MEDIA_PIX_FMT_SBGGR16;
		break;
	default:
		dev_err(isp_dev->dev, "Not support pattern %d bitwidth %d\n",
			mode_info->bayer_pattern, mode_info->bit_width);
		sink_info->fourcc = MEDIA_PIX_FMT_SRGGB12;
		break;
	}

	return ret_val;
}

int media_isp_calib_query_sensor(struct visp_dev *isp_dev, uint8_t port)
{
	int ret_val = VSI_SUCCESS;
	uint8_t sensor_mode = 0;
	cam_device_sensor_query_t QueryInfo_s = {0};
	cam_device_sensor_query_t *QueryInfo = &QueryInfo_s;
	media_isp_port_attr *isp_port = VSI_NULL;

	isp_port = &isp_dev->isp_ports[port];

	ret_val = media_isp_calib_get_sensor_mode(isp_dev, port, &sensor_mode);
	if (ret_val != VSI_SUCCESS) {
		kfree(QueryInfo);
		dev_err(isp_dev->dev, "%s: port %d get sensor mode failed",
			__func__, port);
		return ret_val;
	}

	ret_val = vsi_cam_device_sensor_query(
		isp_dev, isp_port->cam_device_handle, QueryInfo);
	if (ret_val != VSI_SUCCESS) {
		kfree(QueryInfo);
		dev_err(isp_dev->dev,
			"%s: port %d CamDevice query sensor info failed %d",
			__func__, port, ret_val);
		ret_val = VSI_ERR_NOTREADY;
		return ret_val;
	}

	if (sensor_mode >= QueryInfo->number) {
		kfree(QueryInfo);
		ret_val = VSI_ERR_ILLEGAL_PARAM;
		dev_err(isp_dev->dev,
			"%s: port %d sensor mode %d out of range [0, %d]",
			__func__, port, sensor_mode, QueryInfo->number - 1);
		return ret_val;
	}

	memcpy(&isp_port->sensor_info.mode_info,
		   &QueryInfo->sensor_mode_info[sensor_mode],
		   sizeof(QueryInfo->sensor_mode_info[sensor_mode]));

	isp_port->sensor_info.frame_rate =
		isp_port->sensor_info.mode_info.max_fps;

	return ret_val;
}

int media_isp_calib_load_isp_config(struct visp_dev *isp_dev, uint8_t port)
{
	int ret_val = VSI_SUCCESS;
	media_isp_port_attr *isp_port = VSI_NULL;
	char dev_name[MEDIA_ISP_CHAR_LENGTH_MAX];

	isp_port = &isp_dev->isp_ports[port];

	snprintf(dev_name, sizeof(dev_name), "/proc/vsi/isp_subdev%d",
		 isp_dev->id);
	dev_err(isp_dev->dev, "%s: parse %s info:", __func__, dev_name);
	dev_err(isp_dev->dev, "%s: isp : %d", __func__, isp_dev->id);
	dev_err(isp_dev->dev, "%s: port: %u", __func__, port);
	dev_err(isp_dev->dev, "%s: name: %s ", __func__,
		isp_port->sensor_info.name);
	dev_err(isp_dev->dev, "SenNAme : %s\n", isp_port->sensor_info.name);

	dev_err(isp_dev->dev, "%s: mode: %u", __func__,
		isp_port->sensor_info.mode);
	dev_err(isp_dev->dev, "%s: xml : %s", __func__,
		isp_port->sensor_info.calib);
	dev_err(isp_dev->dev, "%s: manu_json: %s", __func__,
		isp_port->sensor_info.manu_json);
	dev_err(isp_dev->dev, "%s: auto_json: %s", __func__,
		isp_port->sensor_info.auto_json);
	dev_err(isp_dev->dev, "%zu %zu %zu\n",
		strlen(isp_port->sensor_info.calib),
		strlen(isp_port->sensor_info.manu_json),
		strlen(isp_port->sensor_info.auto_json));

	dev_err(isp_dev->dev, "%s: sensor_id: %u", __func__,
		isp_port->sensor_info.sensor_id);
	dev_err(isp_dev->dev, "%s: port: %u", __func__, port);
	dev_err(isp_dev->dev, "%s: buf_mode: %s", __func__, isp_port->bufmode);
	dev_err(isp_dev->dev, "%s: port: %u", __func__, port);

	return ret_val;
}

int media_isp_set_frame_rate(struct visp_dev *isp_dev, int pad,
				 /*float * */ uint32_t *frame_rate)
{
	int port = pad / MEDIA_ISP_PORT_PAD_COUNT;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];

	if (*(uint32_t *)frame_rate < /*1.0f*/ 1 ||
		*(uint32_t *)frame_rate > isp_port->sensor_info.mode_info.max_fps) {
		dev_info(isp_dev->dev,
			 " port %d set frame_rate %d Out of range (1 ~ %d)",
			 port, *frame_rate,
			 isp_port->sensor_info.mode_info.max_fps);
		*frame_rate = (*(uint32_t *)frame_rate < 1)
				  ? /*1.0f*/ 1
				  : isp_port->sensor_info.mode_info.max_fps;
	}
	isp_port->sensor_info.frame_rate = *frame_rate;

	return 0;
}

int visp_set_frame_interval_public(struct visp_dev *isp_dev,
				   struct v4l2_subdev_frame_interval *fi);
int media_isp_hal_set_frame_rate(struct visp_dev *isp_dev, int pad,
				 /*float **/ uint32_t *frame_rate);
int media_isp_hal_set_frame_rate(struct visp_dev *isp_dev, int pad,
				 /*float **/ uint32_t *frame_rate)
{
	struct v4l2_subdev_frame_interval SdFi;

	memset(&SdFi, 0, sizeof(SdFi));
	SdFi.pad = pad;
	SdFi.interval.denominator = (uint32_t)*frame_rate;
	SdFi.interval.numerator = 1;
	visp_set_frame_interval_public(isp_dev, &SdFi);

	return VSI_SUCCESS;
}

int isp_device_create(struct visp_dev *isp_dev, uint8_t port)
{
	return 0;

	int ret_val = VSI_SUCCESS;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	cam_device_config_t CamConfig;
	media_fmt *format = NULL;
	cam_device_sensor_drv_handle_t SensorDrvHandle = NULL;
	cam_device_sensor_module_map_cfg_t module_info;
	char sensor_name[MEDIA_ISP_CHAR_LENGTH_MAX];

	/*Enter port Level Critical Section */

	memset(&CamConfig, 0, sizeof(CamConfig)); //

	CamConfig.isp_hw_id = 0; // isp_dev->id;
	CamConfig.input_cfg.input_type =
		CAMDEV_INPUT_TYPE_IMAGE; // CAMDEV_INPUT_TYPE_SENSOR;
	if (isp_port->cam_device_handle)
		return VSI_SUCCESS;

	if (isp_dev->ports_mask != 0x01) {
		CamConfig.work_cfg.work_mode = CAMDEV_WORK_MODE_RDMA;
		CamConfig.work_cfg.mode_cfg.mcm.port_id =
			port + 1; //"1:CAMDEV_MCM_PORT_0, 2:CAMDEV_MCM_PORT_1, ..."
		CamConfig.work_cfg.mode_cfg.mcm.mcm_op =
			1; //"1:CAMDEV_MCM_OP_SW, 2:CAMDEV_MCM_OP_HW"
	} else {
		CamConfig.work_cfg.work_mode = CAMDEV_WORK_MODE_RDMA;
		CamConfig.work_cfg.mode_cfg.stream.port_id =
			port + 1; //"1:CAMDEV_MCM_PORT_0, 2:CAMDEV_MCM_PORT_1, ..."
		dev_err(isp_dev->dev, "%s %d port = %d %d\n", __func__,
			__LINE__, port,
			CamConfig.work_cfg.mode_cfg.stream.port_id);
	}

	CamConfig.output_cfg.output_type = CAMDEV_OUTPUT_TYPE_MEMORY;
	CamConfig.priority = CAMDEV_SEQ_PRI_0;

	/****CamDeviceCreate*****/
	ret_val = vsi_cam_device_create(isp_dev, &CamConfig,
					   &isp_port->cam_device_handle);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"CamDevice Creat Isp Device Handle Failed, ret is %d",
			ret_val);
		ret_val = VSI_ERR_TIMEOUT;
		return ret_val;
	}

	/*****Map the sensor*****/
	memset(&module_info, 0, sizeof(module_info));
	strscpy(module_info.module_name, sensor_name,
		sizeof(module_info.module_name));
	module_info.sensor_dev_id = isp_port->sensor_info.sensor_id;
	ret_val =
		vsi_cam_device_sensor_mapping(isp_dev, isp_port->cam_device_handle,
				  &module_info, &SensorDrvHandle);
	if (ret_val != VSI_SUCCESS || SensorDrvHandle == VSI_NULL) {
		dev_err(isp_dev->dev,
			"CamDevice sensor mapping %s Failed ret is %d",
			sensor_name, ret_val);
		ret_val = VSI_ERR_TIMEOUT;
		goto ERR_TO_DESTROY_CAMDEVICE_HANDLE;
	}

	/***** FMC Config *****/
	ret_val = vsi_cam_device_sensor_drv_handle_register(
		isp_dev, isp_port->cam_device_handle,
		SensorDrvHandle);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"CamDevice register sensor %s driver Failed, ret is %d",
			sensor_name, ret_val);
		ret_val = VSI_ERR_TIMEOUT;
		goto ERR_TO_DESTROY_CAMDEVICE_HANDLE;
	}

	kfree(SensorDrvHandle);

	/***** Query Sensor *****/
	ret_val = media_isp_calib_query_sensor(isp_dev, port);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "%s: query sensor failed %d", __func__,
			ret_val);
		goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
	}

	/***** Configure sink port *****/
	ret_val = media_isp_device_get_port_sink_info(isp_dev, port,
							 &isp_port->sink_info);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "%s: get port sink info failed %d",
			__func__, ret_val);
		goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
	}

	format = kmalloc(sizeof(media_fmt), GFP_KERNEL);
	if (!format) {
		dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__,
			__LINE__);
		ret_val = -ENOMEM;
		goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
	}

	memset(format, 0, sizeof(media_fmt));
	format->width = isp_port->sink_info.rect.width;
	format->height = isp_port->sink_info.rect.height;
	format->pixel_format = isp_port->sink_info.fourcc;

	ret_val = media_isp_hal_set_fmt(isp_dev, port * MEDIA_ISP_PORT_PAD_COUNT,
					   format);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "%s: media_isp_hal_set_fmt failed %d",
			__func__, ret_val);
		goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
	}

	/***** Set Frame Rate *****/
	ret_val = media_isp_hal_set_frame_rate(
		isp_dev, port * MEDIA_ISP_PORT_PAD_COUNT,
		&isp_port->sensor_info.frame_rate);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "%s: MediaIspHalSetFramerate failed %d",
			__func__, ret_val);
		goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
	}

	kfree(format);
	/*Exit port Level Critical Section */
	iba_init_send_command(isp_dev, isp_port->cam_device_handle);

	return ret_val;

ERR_TO_UNREGISTER_SENSOR_HANDLE:
	kfree(format);
	vsi_cam_device_sensor_drv_handle_un_register(
		isp_dev, isp_port->cam_device_handle);
ERR_TO_DESTROY_CAMDEVICE_HANDLE:
	vsi_cam_device_destroy(isp_dev, isp_port->cam_device_handle);
	isp_port->cam_device_handle = VSI_NULL;
	return ret_val;
}
