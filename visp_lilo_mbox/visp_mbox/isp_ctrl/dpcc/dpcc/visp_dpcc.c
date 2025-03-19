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

#include "visp_dpcc.h"

#include <media/v4l2-ioctl.h>

#include "visp_ctrl.h"
#include "visp_driver.h"
#include "visp_event.h"

static int visp_dpcc_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_DPCC_ENABLE:
		case VISP_CID_DPCC_RESET:
		case VISP_CID_DPCC_MODE:
		case VISP_CID_DPCC_AUTO_LEVEL:
		case VISP_CID_DPCC_AUTO_GAIN:
		case VISP_CID_DPCC_AUTO_LINE_MAD_FACTOR:
		case VISP_CID_DPCC_AUTO_LINE_THRESHOLD:
		case VISP_CID_DPCC_AUTO_METHODS_SET:
		case VISP_CID_DPCC_AUTO_OUT_MODE:
		case VISP_CID_DPCC_AUTO_PG_FACTOR:
		case VISP_CID_DPCC_AUTO_RG_FACTOR:
		case VISP_CID_DPCC_AUTO_RND_OFFSETS:
		case VISP_CID_DPCC_AUTO_RND_THRESHOLD:
		case VISP_CID_DPCC_AUTO_RO_LIMITS:
		case VISP_CID_DPCC_AUTO_SET_USE:
		case VISP_CID_DPCC_MANU_BPT_ENABLE:
		case VISP_CID_DPCC_MANU_BPT_NUM:
		case VISP_CID_DPCC_MANU_BPT_OUT_MODE:
		case VISP_CID_DPCC_MANU_OUT_MODE:
		case VISP_CID_DPCC_MANU_SET_USE:
		case VISP_CID_DPCC_MANU_BPT_POS_X:
		case VISP_CID_DPCC_MANU_BPT_POS_Y:
		case VISP_CID_DPCC_MANU_METHODS_SET:
		case VISP_CID_DPCC_MANU_LINE_MAD_FACTOR:
		case VISP_CID_DPCC_MANU_LINE_THRESHOLD:
		case VISP_CID_DPCC_MANU_PG_FACTOR:
		case VISP_CID_DPCC_MANU_RG_FACTOR:
		case VISP_CID_DPCC_MANU_RND_OFFSETS:
		case VISP_CID_DPCC_MANU_RND_THRESHOLD:
		case VISP_CID_DPCC_MANU_RO_LIMITS:
		case VISP_CID_DPCC_ALL_CONFIG:
			ret = visp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static int visp_dpcc_g_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_DPCC_ENABLE:
		case VISP_CID_DPCC_RESET:
		case VISP_CID_DPCC_MODE:
		case VISP_CID_DPCC_AUTO_LEVEL:
		case VISP_CID_DPCC_AUTO_GAIN:
		case VISP_CID_DPCC_AUTO_LINE_MAD_FACTOR:
		case VISP_CID_DPCC_AUTO_LINE_THRESHOLD:
		case VISP_CID_DPCC_AUTO_METHODS_SET:
		case VISP_CID_DPCC_AUTO_OUT_MODE:
		case VISP_CID_DPCC_AUTO_PG_FACTOR:
		case VISP_CID_DPCC_AUTO_RG_FACTOR:
		case VISP_CID_DPCC_AUTO_RND_OFFSETS:
		case VISP_CID_DPCC_AUTO_RND_THRESHOLD:
		case VISP_CID_DPCC_AUTO_RO_LIMITS:
		case VISP_CID_DPCC_AUTO_SET_USE:
		case VISP_CID_DPCC_MANU_BPT_ENABLE:
		case VISP_CID_DPCC_MANU_BPT_NUM:
		case VISP_CID_DPCC_MANU_BPT_OUT_MODE:
		case VISP_CID_DPCC_MANU_OUT_MODE:
		case VISP_CID_DPCC_MANU_SET_USE:
		case VISP_CID_DPCC_MANU_BPT_POS_X:
		case VISP_CID_DPCC_MANU_BPT_POS_Y:
		case VISP_CID_DPCC_MANU_METHODS_SET:
		case VISP_CID_DPCC_MANU_LINE_MAD_FACTOR:
		case VISP_CID_DPCC_MANU_LINE_THRESHOLD:
		case VISP_CID_DPCC_MANU_PG_FACTOR:
		case VISP_CID_DPCC_MANU_RG_FACTOR:
		case VISP_CID_DPCC_MANU_RND_OFFSETS:
		case VISP_CID_DPCC_MANU_RND_THRESHOLD:
		case VISP_CID_DPCC_MANU_RO_LIMITS:
		case VISP_CID_DPCC_STAT_BPT_ENABLE:
		case VISP_CID_DPCC_STAT_BPT_NUM:
		case VISP_CID_DPCC_STAT_BPT_OUT_MODE:
		case VISP_CID_DPCC_STAT_OUT_MODE:
		case VISP_CID_DPCC_STAT_SET_USE:
		case VISP_CID_DPCC_STAT_BPT_POS_X:
		case VISP_CID_DPCC_STAT_BPT_POS_Y:
		case VISP_CID_DPCC_STAT_METHODS_SET:
		case VISP_CID_DPCC_STAT_LINE_MAD_FACTOR:
		case VISP_CID_DPCC_STAT_LINE_THRESHOLD:
		case VISP_CID_DPCC_STAT_PG_FACTOR:
		case VISP_CID_DPCC_STAT_RG_FACTOR:
		case VISP_CID_DPCC_STAT_RND_OFFSETS:
		case VISP_CID_DPCC_STAT_RND_THRESHOLD:
		case VISP_CID_DPCC_STAT_RO_LIMITS:
		case VISP_CID_DPCC_ALL_CONFIG:
		case VISP_CID_DPCC_ALL_STATUS:
			ret = visp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static const struct v4l2_ctrl_ops visp_dpcc_ctrl_ops = {
	.s_ctrl = visp_dpcc_s_ctrl,
	.g_volatile_ctrl = visp_dpcc_g_ctrl,
};

const struct v4l2_ctrl_config visp_dpcc_ctrls[] = {
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_RESET,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_reset",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		/* 0: Manual 1: Auto */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_mode",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_LEVEL,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_level",
		.step = 1,
		.min = 1,
		.max = 20,
		.def = 1,
		.dims = {1},
	},
	{
		/* float 20x array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_GAIN,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_gain",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {20, 0, 0, 0},
	},
	{
		/* uint8_t 20x2x3 array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_LINE_MAD_FACTOR,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_line_mad_factor",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {20, 2, 3, 0},
	},
	{
		/* uint8_t 20x2x3 array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_LINE_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_line_threshold",
		.step = 1,
		.min = 0,
		.max = 255,
		.dims = {20, 2, 3, 0},
	},
	{
		/* uint16_t 20x3 array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_METHODS_SET,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_methods_set",
		.step = 1,
		.min = 0,
		.max = 8191,
		.dims = {20, 3, 0, 0},
	},
	{
		/* uint8_t 20x array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_OUT_MODE,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_out_mode",
		.step = 1,
		.min = 0,
		.max = 15,
		.dims = {20, 0, 0, 0},
	},
	{
		/* uint8_t 20x2x3 array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_PG_FACTOR,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_pg_factor",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {20, 2, 3, 0},
	},
	{
		/* uint8_t 20x2x3 array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_RG_FACTOR,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_rg_factor",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {20, 2, 3, 0},
	},
	{
		/* uint8_t 20x2x3 array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_RND_OFFSETS,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_rnd_offsets",
		.step = 1,
		.min = 0,
		.max = 3,
		.dims = {20, 2, 3, 0},
	},
	{
		/* uint8_t 20x2x3 array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_RND_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_rnd_threshold",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {20, 2, 3, 0},
	},
	{
		/* uint8_t 20x2x3 array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_RO_LIMITS,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_ro_limits",
		.step = 1,
		.min = 0,
		.max = 3,
		.dims = {20, 2, 3, 0},
	},
	{
		/* uint8_t 20x array */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_AUTO_SET_USE,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_auto_set_use",
		.step = 1,
		.min = 0,
		.max = 15,
		.dims = {20, 0, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_BPT_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_bpt_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_BPT_NUM,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_bpt_num",
		.step = 1,
		.min = 0,
		.max = 2048,
		.dims = {1},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_BPT_OUT_MODE,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_bpt_out_mode",
		.step = 1,
		.min = 0,
		.max = 14,
		.dims = {1},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_OUT_MODE,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_out_mode",
		.step = 1,
		.min = 0,
		.max = 15,
		.dims = {1},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_SET_USE,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_set_use",
		.step = 1,
		.min = 0,
		.max = 15,
		.dims = {1},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_BPT_POS_X,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_bpt_pos_x",
		.step = 1,
		.min = 0,
		.max = 65535,
		.dims = {2048},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_BPT_POS_Y,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_bpt_pos_y",
		.step = 1,
		.min = 0,
		.max = 65535,
		.dims = {2048},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_METHODS_SET,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_methods_set",
		.step = 1,
		.min = 0,
		.max = 8191,
		.dims = {3},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_LINE_MAD_FACTOR,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_line_mad_factor",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_LINE_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_line_threshold",
		.step = 1,
		.min = 0,
		.max = 255,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_PG_FACTOR,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_pg_factor",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_RG_FACTOR,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_rg_factor",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_RND_OFFSETS,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_rnd_offsets",
		.step = 1,
		.min = 0,
		.max = 3,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_RND_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_rnd_threshold",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_MANU_RO_LIMITS,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_manu_ro_limits",
		.step = 1,
		.min = 0,
		.max = 3,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_BPT_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_bpt_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_BPT_NUM,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_bpt_num",
		.step = 1,
		.min = 0,
		.max = 2048,
		.dims = {1},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_BPT_OUT_MODE,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_bpt_out_mode",
		.step = 1,
		.min = 0,
		.max = 14,
		.dims = {1},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_OUT_MODE,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_out_mode",
		.step = 1,
		.min = 0,
		.max = 15,
		.dims = {1},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_SET_USE,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_set_use",
		.step = 1,
		.min = 0,
		.max = 15,
		.dims = {1},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_BPT_POS_X,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_bpt_pos_x",
		.step = 1,
		.min = 0,
		.max = 65535,
		.dims = {2048},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_BPT_POS_Y,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_bpt_pos_y",
		.step = 1,
		.min = 0,
		.max = 65535,
		.dims = {2048},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_METHODS_SET,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_methods_set",
		.step = 1,
		.min = 0,
		.max = 8191,
		.dims = {3},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_LINE_MAD_FACTOR,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_line_mad_factor",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_LINE_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_line_threshold",
		.step = 1,
		.min = 0,
		.max = 255,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_PG_FACTOR,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_pg_factor",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_RG_FACTOR,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_rg_factor",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_RND_OFFSETS,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_rnd_offsets",
		.step = 1,
		.min = 0,
		.max = 3,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_RND_THRESHOLD,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_rnd_threshold",
		.step = 1,
		.min = 0,
		.max = 63,
		.dims = {2, 3, 0, 0},
	},
	{
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_STAT_RO_LIMITS,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_stat_ro_limits",
		.step = 1,
		.min = 0,
		.max = 3,
		.dims = {2, 3, 0, 0},
	},
	{
		/* uint8_t data of CamDeviceDpccConfig_t */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_ALL_CONFIG,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_all_config",
		.step = 1,
		.min = 0,
		.max = 0xFF,
		.dims = {9340},
	},
	{
		/* uint8_t data of CamDeviceDpccStatus_t */
		.ops = &visp_dpcc_ctrl_ops,
		.id = VISP_CID_DPCC_ALL_STATUS,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_dpcc_all_status",
		.step = 1,
		.min = 0,
		.max = 0xFF,
		.dims = {8260},
	},
};

int visp_dpcc_ctrl_count(void)
{
	return ARRAY_SIZE(visp_dpcc_ctrls);
}

int visp_dpcc_ctrl_create(struct visp_dev *isp_dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(visp_dpcc_ctrls); i++)
	{
		v4l2_ctrl_new_custom(&isp_dev->ctrl_handler, &visp_dpcc_ctrls[i], NULL);
		if (isp_dev->ctrl_handler.error)
		{
			dev_err(isp_dev->dev, "reigster isp dpcc ctrl %s failed %d.\n",
					visp_dpcc_ctrls[i].name, isp_dev->ctrl_handler.error);
		}
	}

	return 0;
}
