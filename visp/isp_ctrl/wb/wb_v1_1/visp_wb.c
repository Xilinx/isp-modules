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

#include "visp_wb.h"

#include <media/v4l2-ioctl.h>

#include "visp_ctrl.h"
#include "visp_driver.h"
#include "visp_event.h"

static int visp_wb_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_WB_ENABLE:
		case VISP_CID_WB_RESET:
		case VISP_CID_WB_R_GAIN:
		case VISP_CID_WB_GR_GAIN:
		case VISP_CID_WB_GB_GAIN:
		case VISP_CID_WB_B_GAIN:
		case VISP_CID_WB_ALL_CONFIG:
			ret = visp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static int visp_wb_g_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_WB_ENABLE:
		case VISP_CID_WB_R_GAIN:
		case VISP_CID_WB_GR_GAIN:
		case VISP_CID_WB_GB_GAIN:
		case VISP_CID_WB_B_GAIN:
		case VISP_CID_WB_STAT_R_GAIN:
		case VISP_CID_WB_STAT_GR_GAIN:
		case VISP_CID_WB_STAT_GB_GAIN:
		case VISP_CID_WB_STAT_B_GAIN:
		case VISP_CID_WB_ALL_CONFIG:
		case VISP_CID_WB_ALL_STATUS:
			ret = visp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static const struct v4l2_ctrl_ops visp_wb_ctrl_ops = {
	.s_ctrl = visp_wb_s_ctrl,
	.g_volatile_ctrl = visp_wb_g_ctrl,
};

const struct v4l2_ctrl_config visp_wb_ctrls[] = {
	{
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_RESET,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_reset",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		/* float (0, 15.999], ratio 1024:1 */
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_R_GAIN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_r_gain",
		.step = 1,
		.min = 1,
		.max = 16382,
		.def = 1,
	},
	{
		/* float (0, 15.999], ratio 1024:1 */
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_GR_GAIN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_gr_gain",
		.step = 1,
		.min = 1,
		.max = 16382,
		.def = 1,
	},
	{
		/* float (0, 15.999], ratio 1024:1 */
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_GB_GAIN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_gb_gain",
		.step = 1,
		.min = 1,
		.max = 16382,
		.def = 1,
	},
	{
		/* float (0, 15.999], ratio 1024:1 */
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_B_GAIN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_b_gain",
		.step = 1,
		.min = 1,
		.max = 16382,
		.def = 1,
	},
	{
		/* float (0, 15.999], ratio 1024:1 */
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_STAT_R_GAIN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_stat_r_gain",
		.step = 1,
		.min = 1,
		.max = 16382,
		.def = 1,
	},
	{
		/* float (0, 15.999], ratio 1024:1 */
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_STAT_GR_GAIN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_stat_gr_gain",
		.step = 1,
		.min = 1,
		.max = 16382,
		.def = 1,
	},
	{
		/* float (0, 15.999], ratio 1024:1 */
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_STAT_GB_GAIN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_stat_gb_gain",
		.step = 1,
		.min = 1,
		.max = 16382,
		.def = 1,
	},
	{
		/* float (0, 15.999], ratio 1024:1 */
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_STAT_B_GAIN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_stat_b_gain",
		.step = 1,
		.min = 1,
		.max = 16382,
		.def = 1,
	},
	{
		/* uint8_t data of CamDeviceWbConfig_t */
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_ALL_CONFIG,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_all_config",
		.step = 1,
		.min = 0,
		.max = 0xFF,
		.dims = {0x10},
	},
	{
		/* uint8_t data of CamDeviceWbStatus_t */
		.ops = &visp_wb_ctrl_ops,
		.id = VISP_CID_WB_ALL_STATUS,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_wb_all_status",
		.step = 1,
		.min = 0,
		.max = 0xFF,
		.dims = {0x18},
	},
};

int visp_wb_ctrl_count(void)
{
	return ARRAY_SIZE(visp_wb_ctrls);
}

int visp_wb_ctrl_create(struct visp_dev *isp_dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(visp_wb_ctrls); i++)
	{
		v4l2_ctrl_new_custom(&isp_dev->ctrl_handler, &visp_wb_ctrls[i], NULL);
		if (isp_dev->ctrl_handler.error)
		{
			dev_err(isp_dev->dev, "reigster isp wb ctrl %s failed %d.\n",
					visp_wb_ctrls[i].name, isp_dev->ctrl_handler.error);
		}
	}

	return 0;
}
