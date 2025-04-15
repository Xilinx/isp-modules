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

#include "visp_gc.h"

#include <media/v4l2-ioctl.h>

#include "visp_ctrl.h"
#include "visp_driver.h"
#include "visp_event.h"

static int visp_gc_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_GC_ENABLE:
		case VISP_CID_GC_RESET:
		case VISP_CID_GC_MANU_USE_STD:
		case VISP_CID_GC_MANU_STD_VAL:
		case VISP_CID_GC_MANU_CURVE:
		case VISP_CID_GC_MANU_CURVE_MODE:
			ret = visp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static int visp_gc_g_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_GC_ENABLE:
		case VISP_CID_GC_RESET:
		case VISP_CID_GC_MANU_USE_STD:
		case VISP_CID_GC_MANU_STD_VAL:
		case VISP_CID_GC_MANU_CURVE:
		case VISP_CID_GC_MANU_CURVE_MODE:
		case VISP_CID_GC_STAT_USE_STD:
		case VISP_CID_GC_STAT_STD_VAL:
		case VISP_CID_GC_STAT_CURVE:
		case VISP_CID_GC_STAT_CURVE_MODE:
			ret = visp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static const struct v4l2_ctrl_ops visp_gc_ctrl_ops = {
	.s_ctrl = visp_gc_s_ctrl,
	.g_volatile_ctrl = visp_gc_g_ctrl,
};

const struct v4l2_ctrl_config visp_gc_ctrls[] = {
	{
		.ops = &visp_gc_ctrl_ops,
		.id = VISP_CID_GC_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_gc_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_gc_ctrl_ops,
		.id = VISP_CID_GC_RESET,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_gc_reset",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_gc_ctrl_ops,
		.id = VISP_CID_GC_MANU_USE_STD,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_gc_manu_use_std",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		/* float 1.0 ~ 4.0 */
		.ops = &visp_gc_ctrl_ops,
		.id = VISP_CID_GC_MANU_STD_VAL,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_gc_manu_std_val",
		.step = 1,
		.min = 100,
		.max = 400,
		.def = 220,
	},
	{
		/* uint16_t 17*16bit */
		.ops = &visp_gc_ctrl_ops,
		.id = VISP_CID_GC_MANU_CURVE,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_gc_manu_curve",
		.step = 1,
		.min = 0,
		.max = 1023,
		.dims = {17, 0, 0, 0},
	},
	{
		/* uint32_t array 64*32bit */
		.ops = &visp_gc_ctrl_ops,
		.id = VISP_CID_GC_MANU_CURVE_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_gc_manu_curve_mode",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_gc_ctrl_ops,
		.id = VISP_CID_GC_STAT_USE_STD,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_gc_stat_use_std",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		/* float 1.0 ~ 4.0 */
		.ops = &visp_gc_ctrl_ops,
		.id = VISP_CID_GC_STAT_STD_VAL,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_gc_stat_std_val",
		.step = 1,
		.min = 100,
		.max = 400,
		.def = 220,
	},
	{
		/* uint16_t 17*16bit */
		.ops = &visp_gc_ctrl_ops,
		.id = VISP_CID_GC_STAT_CURVE,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_gc_stat_curve",
		.step = 1,
		.min = 0,
		.max = 1023,
		.dims = {17, 0, 0, 0},
	},
	{
		/* uint32_t array 64*32bit */
		.ops = &visp_gc_ctrl_ops,
		.id = VISP_CID_GC_STAT_CURVE_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_gc_stat_curve_mode",
		.step = 1,
		.min = 0,
		.max = 1,
	},
};

int visp_gc_ctrl_count(void)
{
	return ARRAY_SIZE(visp_gc_ctrls);
}

int visp_gc_ctrl_create(struct visp_dev *isp_dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(visp_gc_ctrls); i++)
	{
		v4l2_ctrl_new_custom(&isp_dev->ctrl_handler, &visp_gc_ctrls[i], NULL);
		if (isp_dev->ctrl_handler.error)
		{
			dev_err(isp_dev->dev, "reigster isp gc ctrl %s failed %d.\n",
					visp_gc_ctrls[i].name, isp_dev->ctrl_handler.error);
		}
	}

	return 0;
}
