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

#include "visp_lsc.h"

#include <media/v4l2-ioctl.h>

#include "visp_ctrl.h"
#include "visp_driver.h"
#include "visp_event.h"

static int visp_lsc_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_LSC_ENABLE:
		case VISP_CID_LSC_RESET:
		case VISP_CID_LSC_MODE:
		case VISP_CID_LSC_AUTO_LEVEL:
		case VISP_CID_LSC_AUTO_DAMPING:
		case VISP_CID_LSC_AUTO_GAIN:
		case VISP_CID_LSC_AUTO_STRENGTH:
		case VISP_CID_LSC_AUTO_INTER_MODE:
		case VISP_CID_LSC_MANU_X_SIZE_TBL:
		case VISP_CID_LSC_MANU_Y_SIZE_TBL:
		case VISP_CID_LSC_MANU_DATA_TBL:
			ret = visp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static int visp_lsc_g_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_LSC_ENABLE:
		case VISP_CID_LSC_RESET:
		case VISP_CID_LSC_MODE:
		case VISP_CID_LSC_MANU_X_SIZE_TBL:
		case VISP_CID_LSC_MANU_Y_SIZE_TBL:
		case VISP_CID_LSC_MANU_DATA_TBL:
		case VISP_CID_LSC_AUTO_LEVEL:
		case VISP_CID_LSC_AUTO_DAMPING:
		case VISP_CID_LSC_AUTO_GAIN:
		case VISP_CID_LSC_AUTO_STRENGTH:
		case VISP_CID_LSC_AUTO_INTER_MODE:
		case VISP_CID_LSC_STAT_X_SIZE_TBL:
		case VISP_CID_LSC_STAT_Y_SIZE_TBL:
		case VISP_CID_LSC_STAT_DATA_TBL:
			ret = visp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static const struct v4l2_ctrl_ops visp_lsc_ctrl_ops = {
	.s_ctrl = visp_lsc_s_ctrl,
	.g_volatile_ctrl = visp_lsc_g_ctrl,
};

const struct v4l2_ctrl_config visp_lsc_ctrls[] = {
	{
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_RESET,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_reset",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		/* manual/auto */
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_mode",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_MANU_X_SIZE_TBL,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_manu_x_size_tbl",
		.step = 1,
		.min = 1,
		.max = 0xFFFF,
		.def = 1,
		.dims = {32, 0, 0, 0},
	},
	{
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_MANU_Y_SIZE_TBL,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_manu_y_size_tbl",
		.step = 1,
		.min = 1,
		.max = 0xFFFF,
		.def = 1,
		.dims = {32, 0, 0, 0},
	},
	{
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_MANU_DATA_TBL,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_manu_data_tbl",
		.step = 1,
		.min = 1024,
		.max = 16383,
		.def = 1024,
		.dims = {4, 33, 33, 0},
	},
	{
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_AUTO_LEVEL,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_auto_level",
		.step = 1,
		.min = 1,
		.max = 20,
		.def = 1,
		.dims = {1},
	},
	{
		/* float 0~1 */
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_AUTO_DAMPING,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_auto_damping",
		.step = 1,
		.min = 0,
		.max = 100,
	},
	{
		/* float 20x */
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_AUTO_GAIN,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_auto_gain",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {20, 0, 0, 0},
	},
	{
		/* float 20x 0~1 */
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_AUTO_STRENGTH,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_auto_strength",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {20, 0, 0, 0},
	},
	{
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_AUTO_INTER_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_auto_inter_mode",
		.step = 1,
		.min = 0,
		.max = 2,
	},
	{
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_STAT_X_SIZE_TBL,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_stat_x_size_tbl",
		.step = 1,
		.min = 10,
		.max = 0xFFFF,
		.def = 10,
		.dims = {32, 0, 0, 0},
	},
	{
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_STAT_Y_SIZE_TBL,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_stat_y_size_tbl",
		.step = 1,
		.min = 8,
		.max = 0xFFFF,
		.def = 8,
		.dims = {32, 0, 0, 0},
	},
	{
		.ops = &visp_lsc_ctrl_ops,
		.id = VISP_CID_LSC_STAT_DATA_TBL,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_lsc_stat_data_tbl",
		.step = 1,
		.min = 1024,
		.max = 16383,
		.def = 1024,
		.dims = {4, 33, 33, 0},
	},
};

int visp_lsc_ctrl_count(void)
{
	return ARRAY_SIZE(visp_lsc_ctrls);
}

int visp_lsc_ctrl_create(struct visp_dev *isp_dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(visp_lsc_ctrls); i++)
	{
		v4l2_ctrl_new_custom(&isp_dev->ctrl_handler, &visp_lsc_ctrls[i], NULL);
		if (isp_dev->ctrl_handler.error)
		{
			dev_err(isp_dev->dev, "reigster isp lsc ctrl %s failed %d.\n",
					visp_lsc_ctrls[i].name, isp_dev->ctrl_handler.error);
		}
	}

	return 0;
}
