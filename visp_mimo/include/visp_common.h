/* SPDX-License-Identifier: MIT*/
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

#ifndef __VISP_COMMON_H__
#define __VISP_COMMON_H__

#include "visp_app.h"

int visp_buf_done(struct v4l2_subdev *sd, void *arg);
int visp_set_frame_interval_public(struct visp_dev *isp_dev,
				   struct v4l2_subdev_frame_interval *fi);
int visp_set_fmt_public(struct visp_dev *isp_dev,
			struct v4l2_subdev_format *format);
int visp_get_format_stride_public(struct visp_dev *isp_dev, uint32_t fourcc,
				  uint32_t width, uint32_t height,
				  uint32_t *pstride);
int media_isp_device_set_frame_rate(struct visp_dev *isp_dev, uint8_t port,
				    uint32_t *frame_rate);
int media_isp_device_stream_on(struct visp_dev *isp_dev, uint8_t port,
			       uint8_t chn);
int media_isp_device_stream_on_out(struct visp_dev *isp_dev, uint8_t port,
				   uint8_t chn);
int isp_device_destroy(struct visp_dev *isp_dev, uint8_t port, uint8_t chn);
int isp_destroy_cam_device(struct visp_dev *isp_dev, uint8_t port, uint8_t chn);
int isp_destroy_pipeline(struct visp_dev *isp_dev, uint8_t port, uint8_t chn);
int media_isp_stream_off(struct visp_dev *isp_dev, uint8_t port, uint8_t chn);
int media_isp_device_qbuf(struct visp_dev *isp_dev, uint8_t port, uint8_t chn,
			  media_buf *buf);
int media_isp_q_buf(struct visp_dev *isp_dev, int pad_index, media_buf *buf);
int media_isp_hal_mbus_fmt_to_media_fmt(uint32_t *code, uint32_t *pixel_format,
					uint32_t fourcc);
int media_isp_device_set_format(struct visp_dev *isp_dev, uint8_t port,
				uint8_t chn);
int media_isp_calib_get_mode_info(struct visp_dev *isp_dev, uint8_t port,
				  cam_device_sensor_mode_info_t *mode_info);
int media_isp_calib_get_sensor_name(struct visp_dev *isp_dev, uint8_t port,
				    char *sensor_name);
int media_isp_calib_get_sensor_mode(struct visp_dev *isp_dev, uint8_t port,
				    uint8_t *sensor_mode);
int media_isp_device_sensor_open(struct visp_dev *isp_dev, uint8_t port);
int media_isp_device_camera_connect(struct visp_dev *isp_dev, uint8_t index);

int media_isp_device_dqbuf(struct visp_dev *isp_dev, struct Chn_info *info,
			   media_buf *buf, void *enque_buff_g,
			   media_buffer_t *p_media_buffer);
int media_isp_calib_query_sensor(struct visp_dev *isp_dev, uint8_t port);
int media_isp_calib_load_isp_config(struct visp_dev *isp_dev, uint8_t port);
int media_isp_hal_set_fmt(struct visp_dev *isp_dev, int pad, media_fmt *format);
int media_isp_set_format(struct visp_dev *isp_dev, uint32_t pad_index,
			 media_fmt format_t);
int media_isp_set_frame_rate(struct visp_dev *isp_dev, int pad,
			     uint32_t *frame_rate);
int media_isp_hal_buf_done(struct v4l2_subdev *sd, int pad,
			   const media_buf *buf);
int isp_device_create(struct visp_dev *isp_dev, uint8_t port);
int media_isp_device_dq_buf_out(struct visp_dev *isp_dev, struct Chn_info *info,
				void *packet_from_rpu,
				media_buffer_t *p_media_buffer);

RESULT vsi_cam_device_awb_disable(struct visp_dev *isp_dev,
				  cam_device_handle_t h_cam_device);
RESULT vsi_cam_device_un_register_awb_lib(struct visp_dev *isp_dev,
					  cam_device_handle_t h_cam_device);
RESULT vsi_cam_device_ae_disable(struct visp_dev *isp_dev,
				 cam_device_handle_t h_cam_device);
RESULT vsi_cam_device_un_register_ae_lib(struct visp_dev *isp_dev,
					 cam_device_handle_t h_cam_device);

#define VSI_SUCCESS 0
#define VSI_FAILURE (-1)
#define VSI_NULL ((void *)0)
#define VSI_RPU_NOT_SUPPORT (0xFFFF)
#define APU_META_DATA_SIZE 1024

typedef enum ErrCode_E {
	VSI_ERR_INVALID_DEVID = 1,
	VSI_ERR_INVALID_PORTID = 2,
	VSI_ERR_INVALID_CHNID = 3,
	VSI_ERR_ILLEGAL_PARAM = 4,
	VSI_ERR_EXIST = 5,
	VSI_ERR_UNEXIST = 6,
	VSI_ERR_NULL_PTR = 7,
	VSI_ERR_NOT_CONFIG = 8,
	VSI_ERR_NOT_SUPPORT = 9,
	VSI_ERR_NOT_PERM = 10,
	VSI_ERR_NOMEM = 11,
	VSI_ERR_NOBUF = 12,
	VSI_ERR_BUF_EMPTY = 13,
	VSI_ERR_BUF_FULL = 14,
	VSI_ERR_NOTREADY = 15,
	VSI_ERR_BADADDR = 17,
	VSI_ERR_BUSY = 18,
	VSI_ERR_TIMEOUT = 19,
	VSI_ERR_BUTT = 256
} vsi_err_code_e;

#endif
