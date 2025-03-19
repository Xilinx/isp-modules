/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
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
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
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

#include "visp_rgbir.h"

#include <media/v4l2-ioctl.h>

#include "visp_ctrl.h"
#include "visp_driver.h"
#include "visp_event.h"

static int visp_rgbir_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_RGBIR_ENABLE:
		case VISP_CID_RGBIR_RCCC_ENABLE:
		case VISP_CID_RGBIR_IR_RAW_OUT_ENABLE:
		case VISP_CID_RGBIR_RESET:
		case VISP_CID_RGBIR_CC_MATRIX:
		case VISP_CID_RGBIR_DPCC_MIDDLE_THRESHOLD:
		case VISP_CID_RGBIR_DPCC_THRESHOLD:
		case VISP_CID_RGBIR_IR_THRESHOLD:
		case VISP_CID_RGBIR_L_THRESHOLD:
		case VISP_CID_RGBIR_OUT_PATTERN:
			ret = visp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static int visp_rgbir_g_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_RGBIR_ENABLE:
		case VISP_CID_RGBIR_RCCC_ENABLE:
		case VISP_CID_RGBIR_IR_RAW_OUT_ENABLE:
		case VISP_CID_RGBIR_RESET:
		case VISP_CID_RGBIR_CC_MATRIX:
		case VISP_CID_RGBIR_DPCC_MIDDLE_THRESHOLD:
		case VISP_CID_RGBIR_DPCC_THRESHOLD:
		case VISP_CID_RGBIR_IR_THRESHOLD:
		case VISP_CID_RGBIR_L_THRESHOLD:
		case VISP_CID_RGBIR_OUT_PATTERN:
		case VISP_CID_RGBIR_STAT_CC_MATRIX:
		case VISP_CID_RGBIR_STAT_DPCC_MIDDLE_THRESHOLD:
		case VISP_CID_RGBIR_STAT_DPCC_THRESHOLD:
		case VISP_CID_RGBIR_STAT_IR_THRESHOLD:
		case VISP_CID_RGBIR_STAT_L_THRESHOLD:
			ret = visp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static const struct v4l2_ctrl_ops visp_rgbir_ctrl_ops = {
	.s_ctrl = visp_rgbir_s_ctrl,
	.g_volatile_ctrl = visp_rgbir_g_ctrl,
};

const struct v4l2_ctrl_config visp_rgbir_ctrls[] = {
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_RCCC_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_rccc_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_IR_RAW_OUT_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_ir_raw_out_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_RESET,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_reset",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		/* float array 12x (-4,4)*/
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_CC_MATRIX,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_cc_matrix",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {12, 0, 0, 0},
	},
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_DPCC_MIDDLE_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_dpcc_middle_threshold",
		.step = 1,
		.min = 0,
		.max = 65535,
		.dims = {4, 0, 0, 0},
	},
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_DPCC_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_dpcc_threshold",
		.step = 1,
		.min = 0,
		.max = 65535,
		.dims = {4, 0, 0, 0},
	},
	{
		/* 12bit: 4095 24bit: 16777215*/
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_IR_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_ir_threshold",
		.step = 1,
		.min = 0,
		.max = 16777215,
		.dims = {1, 0, 0, 0},
	},
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_L_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_l_threshold",
		.step = 1,
		.min = 0,
		.max = 16777215,
		.dims = {1, 0, 0, 0},
	},
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_OUT_PATTERN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_out_pattern",
		.step = 1,
		.min = 0,
		.max = 3,
	},
	{
		/* float array 12x (-4,4)*/
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_STAT_CC_MATRIX,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_stat_cc_matrix",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {12, 0, 0, 0},
	},
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_STAT_DPCC_MIDDLE_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_stat_dpcc_middle_threshold",
		.step = 1,
		.min = 0,
		.max = 65535,
		.dims = {4, 0, 0, 0},
	},
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_STAT_DPCC_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_stat_dpcc_threshold",
		.step = 1,
		.min = 0,
		.max = 65535,
		.dims = {4, 0, 0, 0},
	},
	{
		/* 12bit: 4095 24bit: 16777215*/
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_STAT_IR_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_stat_ir_threshold",
		.step = 1,
		.min = 0,
		.max = 16777215,
		.dims = {1, 0, 0, 0},
	},
	{
		.ops = &visp_rgbir_ctrl_ops,
		.id = VISP_CID_RGBIR_STAT_L_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_rgbir_stat_l_threshold",
		.step = 1,
		.min = 0,
		.max = 16777215,
		.dims = {1, 0, 0, 0},
	},
};

int visp_rgbir_ctrl_count(void)
{
	return ARRAY_SIZE(visp_rgbir_ctrls);
}

int visp_rgbir_ctrl_create(struct visp_dev *isp_dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(visp_rgbir_ctrls); i++)
	{
		v4l2_ctrl_new_custom(&isp_dev->ctrl_handler, &visp_rgbir_ctrls[i],
							 NULL);
		if (isp_dev->ctrl_handler.error)
		{
			dev_err(isp_dev->dev, "reigster isp rgbir ctrl %s failed %d.\n",
					visp_rgbir_ctrls[i].name, isp_dev->ctrl_handler.error);
		}
	}

	return 0;
}
