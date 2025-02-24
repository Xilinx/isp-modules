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

#include "visp_af.h"

#include <media/v4l2-ioctl.h>

#include "visp_ctrl.h"
#include "visp_driver.h"
#include "visp_event.h"

static int visp_af_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_AF_ENABLE:
		case VISP_CID_AF_RESET:
		case VISP_CID_AF_STATE:
		case VISP_CID_AF_MODE:
		case VISP_CID_CDAF_WINDOW_WEIGHT:
		case VISP_CID_CDAF_STABLE_TOLERENCE:
		case VISP_CID_CDAF_POINTS_OF_CURVE:
		case VISP_CID_CDAF_FOCAL_FILTER:
		case VISP_CID_CDAF_SHAPE_FILTER:
		case VISP_CID_CDAF_MAX_FOCAL:
		case VISP_CID_CDAF_MIN_FOCAL:
		case VISP_CID_CDAF_MOTION_THRESHOLD:
		case VISP_CID_CDAF_STABLE_THRESHOLD:
		case VISP_CID_PDAF_MSE_TOLERENCE:
		case VISP_CID_PDAF_PD_CONF_THRESHOLD:
		case VISP_CID_PDAF_PD_FOCAL:
		case VISP_CID_PDAF_PD_DISTANCE:
		case VISP_CID_PDAF_PD_SHIFT_THRESHOLD:
		case VISP_CID_PDAF_PD_STABLE_CNT_MAX:
		case VISP_CID_PDAF_ROI_INDEX:
			ret = visp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static int visp_af_g_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_AF_ENABLE:
		case VISP_CID_AF_RESET:
		case VISP_CID_AF_STATE:
		case VISP_CID_AF_MODE:
		case VISP_CID_CDAF_WINDOW_WEIGHT:
		case VISP_CID_CDAF_STABLE_TOLERENCE:
		case VISP_CID_CDAF_POINTS_OF_CURVE:
		case VISP_CID_CDAF_FOCAL_FILTER:
		case VISP_CID_CDAF_SHAPE_FILTER:
		case VISP_CID_CDAF_MAX_FOCAL:
		case VISP_CID_CDAF_MIN_FOCAL:
		case VISP_CID_CDAF_MOTION_THRESHOLD:
		case VISP_CID_CDAF_STABLE_THRESHOLD:
		case VISP_CID_PDAF_MSE_TOLERENCE:
		case VISP_CID_PDAF_PD_CONF_THRESHOLD:
		case VISP_CID_PDAF_PD_FOCAL:
		case VISP_CID_PDAF_PD_DISTANCE:
		case VISP_CID_PDAF_PD_SHIFT_THRESHOLD:
		case VISP_CID_PDAF_PD_STABLE_CNT_MAX:
		case VISP_CID_PDAF_ROI_INDEX:
			ret = visp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static const struct v4l2_ctrl_ops visp_af_ctrl_ops = {
	.s_ctrl = visp_af_s_ctrl,
	.g_volatile_ctrl = visp_af_g_ctrl,
};

const struct v4l2_ctrl_config visp_af_ctrls[] = {
	{
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_AF_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_af_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_AF_RESET,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_af_reset",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_AF_STATE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_af_state",
		.step = 1,
		.min = 0,
		.max = 5,
	},
	{
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_AF_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_af_mode",
		.step = 1,
		.min = 0,
		.max = 0,
	},
	{
		/* float array 3x */
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_CDAF_WINDOW_WEIGHT,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_cdaf_window_weight",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {3, 0, 0, 0},
	},
	{
		/* float (0,1) */
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_CDAF_STABLE_TOLERENCE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_cdaf_stable_tolerance",
		.step = 1,
		.min = 1,
		.max = 99,
		.def = 1,
	},
	{
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_CDAF_POINTS_OF_CURVE,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_cdaf_points_of_curve",
		.step = 1,
		.min = 3,
		.max = 20,
		.def = 3,
		.dims = {1},
	},
	{
		/* float 5x array 0~1023 */
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_CDAF_FOCAL_FILTER,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_cdaf_focal_filter",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {5, 0, 0, 0},
	},
	{
		/* float 5x array 0~1023 */
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_CDAF_SHAPE_FILTER,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_cdaf_shape_filter",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {5, 0, 0, 0},
	},
	{
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_CDAF_MAX_FOCAL,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_cdaf_max_focal",
		.step = 1,
		.min = 0,
		.max = 1023,
		.dims = {1},
	},
	{
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_CDAF_MIN_FOCAL,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_cdaf_min_focal",
		.step = 1,
		.min = 0,
		.max = 1023,
		.dims = {1},
	},
	{
		/* float (0,1) */
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_CDAF_MOTION_THRESHOLD,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_cdaf_motion_threshold",
		.step = 1,
		.min = 1,
		.max = 999,
		.def = 1,
	},
	{
		/* float (0,1) */
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_CDAF_STABLE_THRESHOLD,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_cdaf_stable_threshold",
		.step = 1,
		.min = 1,
		.max = 999,
		.def = 1,
	},
	{
		/* float (0,1) */
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_PDAF_MSE_TOLERENCE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_pdaf_mse_tolerence",
		.step = 1,
		.min = 1,
		.max = 99,
		.def = 1,
	},
	{
		/* float (0,10230) */
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_PDAF_PD_CONF_THRESHOLD,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_pdaf_pd_conf_threshold",
		.step = 1,
		.min = 1,
		.max = 10229,
		.def = 1,
	},
	{
		/* int 48x array (-255,255)*/
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_PDAF_PD_FOCAL,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_pdaf_pd_focal",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {48, 0, 0, 0},
	},
	{
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_PDAF_PD_DISTANCE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_pdaf_pd_distance",
		.step = 1,
		.min = 1,
		.max = 254,
		.def = 1,
	},
	{
		/* float (0,1) */
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_PDAF_PD_SHIFT_THRESHOLD,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_pdaf_pd_shift_threshold",
		.step = 1,
		.min = 1,
		.max = 999,
		.def = 1,
	},
	{
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_PDAF_PD_STABLE_CNT_MAX,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_pdaf_pd_stable_cnt_max",
		.step = 1,
		.min = 1,
		.max = 10,
		.def = 1,
		.dims = {1},
	},
	{
		.ops = &visp_af_ctrl_ops,
		.id = VISP_CID_PDAF_ROI_INDEX,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_pdaf_roi_index",
		.step = 1,
		.min = 0,
		.max = 48,
	},
};

int visp_af_ctrl_count(void)
{
	return ARRAY_SIZE(visp_af_ctrls);
}

int visp_af_ctrl_create(struct visp_dev *isp_dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(visp_af_ctrls); i++)
	{
		v4l2_ctrl_new_custom(&isp_dev->ctrl_handler, &visp_af_ctrls[i], NULL);
		if (isp_dev->ctrl_handler.error)
		{
			dev_err(isp_dev->dev, "reigster isp af ctrl %s failed %d.\n",
					visp_af_ctrls[i].name, isp_dev->ctrl_handler.error);
		}
	}

	return 0;
}
