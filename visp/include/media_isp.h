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

#ifndef __MEDIA_ISP_H__
#define __MEDIA_ISP_H__

#include "cam_device_api.h"
#include "cam_device_buffer_api.h"
#include "cam_device_common.h"
#include "cam_device_sensor_api.h"
#include "media_device.h"
#include "mc_isp_hal.h"

#define MEDIA_ISP_BUF_FRAME_MAX 32
#define MEDIA_ISP_CHAR_LENGTH_MAX 64
#define MEDIA_ISP_PATH_LENGTH_MAX 256

enum media_isp_port_pad_e {
	MEDIA_ISP_PORT_PAD_SINK = 0,
	MEDIA_ISP_PORT_PAD_SOURCE_MP,
	MEDIA_ISP_PORT_PAD_SOURCE_SP1,
	MEDIA_ISP_PORT_PAD_SOURCE_SP2,
	MEDIA_ISP_PORT_PAD_SOURCE_RAW,
	MEDIA_ISP_PORT_PAD_COUNT,
};

typedef enum media_isp_mcm_input_select_e {
	MEDIA_ISP_MCM_INPUT_SELECT_RPU =
	    0, /**< alloc mcm input buffer memory from rpu*/
	MEDIA_ISP_MCM_INPUT_SELECT_APU =
	    1, /**< alloc mcm input buffer memory from apu */
	MEDIA_ISP_MCM_INPUT_SELECT_MAX, //< Maximum mcm input buffer memory
					//   selection.
} media_isp_mcm_input_select;

#define MEDIA_ISP_PORT_MAX 4
#define MEDIA_ISP_CHN_MAX (MEDIA_ISP_PORT_PAD_COUNT - 1)
#define MEDIA_ISP_PAD_NR (MEDIA_ISP_PORT_MAX * MEDIA_ISP_PORT_PAD_COUNT)

int media_isp_event_create(media_entity_attr *media_entity);
int media_isp_event_destroy(media_entity_attr *media_entity);

typedef struct atm_info_s
{
   uint32_t ports_mask;
   uint32_t ref_count;
} atm_info;


typedef struct media_isp_chn_s {
	media_fmt format;
	uint8_t num_bufs;
	media_buf bufs[MEDIA_ISP_BUF_FRAME_MAX];
	void *cam_device_bufs[MEDIA_ISP_BUF_FRAME_MAX];
	/* Protects cam_device_bufs array access */
	struct mutex cam_device_bufs_lock;
} media_isp_chn_attr;

typedef struct media_isp_mcm_s {
	uint8_t num_bufs;
	media_isp_mcm_input_select input_select;
	media_buf bufs[MEDIA_ISP_BUF_FRAME_MAX];
} media_isp_mcm_attr;

typedef struct media_isp_sensor_info_s {
	uint8_t mode;
	char name[MEDIA_ISP_CHAR_LENGTH_MAX];
	char calib[MEDIA_ISP_PATH_LENGTH_MAX];
	char manu_json[MEDIA_ISP_PATH_LENGTH_MAX];
	char auto_json[MEDIA_ISP_PATH_LENGTH_MAX];
	char one_json[MEDIA_ISP_PATH_LENGTH_MAX];
	struct cam_device_sensor_mode_info_s mode_info;
	uint32_t frame_rate;
	uint8_t vc_id;
	uint32_t sensor_id;
	uint32_t i2c_id;
} media_isp_sensor_info;

#define BUF_MODE_SIZE 16
typedef struct media_isp_port_s {
	uint32_t ref_count;
	uint32_t cam_device_ref_mask;
	cam_device_handle_t cam_device_handle;
	media_isp_chn_attr isp_chns[MEDIA_ISP_CHN_MAX];
	media_sink_info sink_info;
	media_isp_sensor_info sensor_info;
	uint32_t path_out_type[MEDIA_ISP_CHN_MAX];
	uint32_t camera_connect_ref_cnt;
	media_isp_mcm_attr mcm_attr;
	bool_t one_json_mode;
	bool_t sensor_drv_registered;
	struct mutex main_lock;
	char bufmode[BUF_MODE_SIZE];
	bool_t hw_mcm;
	char fusa_json[MEDIA_ISP_PATH_LENGTH_MAX];
	bool_t load_json;
} media_isp_port_attr;

typedef struct media_isp_event_dev_s {
	int dev_id;
	uint32_t ports_mask;
	uint32_t ref_count;
	media_isp_port_attr isp_ports[MEDIA_ISP_PORT_MAX];
	mc_isp_hal_handle hal_handle;
	media_entity_attr *media_entity;
} media_isp_event_dev;

#endif
