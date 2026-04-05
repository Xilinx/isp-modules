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

#include <visp_mimo_driver.h>
#include "visp_common.h"
int xlnx_link_mbox(struct visp_dev *isp_dev)
{
	/* Find or create a new RPU with the given rpu_id */
	isp_dev->rpu = visp_mbox_get_rpu_dev(isp_dev->isp_rpu);
	if (!isp_dev->rpu) {
		dev_err(isp_dev->dev, "Failed to find or create RPU: %d\n",
			isp_dev->isp_rpu);
		return -ENOMEM;
	}
	/* Initialise completions/FIFOs used for ack/data handling */
	for (int inst = 0; inst < isp_dev->num_streams; inst++) {
		for (int path = 0; path < 4; path++)
			for (int buf = 0; buf < 32; buf++)
				init_completion(&isp_dev->apu_wait_for_enq_ack[inst][path][buf]);
		init_completion(&isp_dev->apu_wait_for_cmd_ack[inst]);
		mutex_init(&isp_dev->cmd_ack_fifo_lock[inst]);
		if (kfifo_alloc(&isp_dev->cmd_ack_fifo[inst], 128, GFP_KERNEL)) {
			dev_err(isp_dev->dev,
				"Failed to allocate cmd_ack_fifo[%d]\n", inst);
			return -ENOMEM;
		}
	}
	init_completion(&isp_dev->apu_wait_for_data);

	isp_dev->apu_wait_for_isp_frame_done = 0;
	init_waitqueue_head(&isp_dev->wq_frame_done_finished);

	if (!isp_dev->rpu->tx_chan || !isp_dev->rpu->rx_chan) {
		dev_err(isp_dev->dev, "No TX or RX Channel found on RPU: %d\n",
			isp_dev->isp_rpu);
		return -ENOMEM;
	}

	isp_dev->tx_chan = isp_dev->rpu->tx_chan;
	isp_dev->rx_chan = isp_dev->rpu->rx_chan;
	// Assigning isp_dev structure value to isp_dev present in rpu_dev
	// struct
	isp_dev->rpu->isp_dev[isp_dev->id] = isp_dev;
	return 0;
}

int isp_device_create_m_i_m_o(struct visp_dev *isp_dev, uint8_t port)
{
	int ret_val = VSI_SUCCESS;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];
	cam_device_config_t CamConfig;

	if (!isp_dev)
		pr_err("%s %d NULL_ISP_DEV\n", __func__, __LINE__);

	// mutex_lock(&isp_dev->port_lock[port]);

	memset(&CamConfig, 0, sizeof(CamConfig));

	CamConfig.isp_hw_id = isp_dev->id;
	CamConfig.input_cfg.input_type = CAMDEV_INPUT_TYPE_IMAGE;
	if (isp_port->cam_device_handle)
		return VSI_SUCCESS;

	CamConfig.work_cfg.work_mode = CAMDEV_WORK_MODE_RDMA;
	CamConfig.work_cfg.mode_cfg.stream.port_id = 0;

	CamConfig.output_cfg.output_type = CAMDEV_OUTPUT_TYPE_MEMORY;
	CamConfig.priority = CAMDEV_SEQ_PRI_0;

	/****CamDeviceCreate*****/
	ret_val = vsi_cam_device_create(isp_dev, &CamConfig,
				       &isp_port->cam_device_handle);
	if (ret_val != VSI_SUCCESS) {
		dev_err(isp_dev->dev,
			"CamDevice Creat Isp Device Handle Failed, ret is %d\n",
			ret_val);
		ret_val = VSI_ERR_TIMEOUT;
		return ret_val;
	}

	return ret_val;
}

int isp_device_distroy_m_i_m_o(struct visp_dev *isp_dev, uint8_t port)
{
	int ret_val = VSI_SUCCESS;
	media_isp_port_attr *isp_port = &isp_dev->isp_ports[port];

	/*Enter port Level Critical Section */

	ret_val = vsi_cam_device_destroy(isp_dev, &isp_port->cam_device_handle);
	if (ret_val != VSI_SUCCESS) {
		dev_err(
		    isp_dev->dev,
		    "CamDevice Distroy Isp Device Handle Failed, ret is %d\n",
		    ret_val);
		ret_val = VSI_ERR_TIMEOUT;
		return ret_val;
	}
	isp_port->cam_device_handle = VSI_NULL;

	return ret_val;
}
