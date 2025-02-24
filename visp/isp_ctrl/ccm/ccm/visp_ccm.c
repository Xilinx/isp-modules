/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 VeriSilicon Holdings Co., Ltd.
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
 * Copyright (c) 2023-2024 VeriSilicon Holdings Co., Ltd.
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

#include "visp_ccm.h"

#include <media/v4l2-ioctl.h>

#include "visp_ctrl.h"
#include "visp_driver.h"
#include "visp_event.h"

static int visp_ccm_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_CCM_ENABLE:
		case VISP_CID_CCM_RESET:
		case VISP_CID_CCM_MODE:
		case VISP_CID_CCM_MATRIX:
		case VISP_CID_CCM_OFFSET:
		case VISP_CID_CCM_ALL_CONFIG:
			ret = visp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static int visp_ccm_g_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	switch (ctrl->id)
	{
		case VISP_CID_CCM_ENABLE:
		case VISP_CID_CCM_RESET:
		case VISP_CID_CCM_MODE:
		case VISP_CID_CCM_MATRIX:
		case VISP_CID_CCM_OFFSET:
		case VISP_CID_CCM_STAT_MATRIX:
		case VISP_CID_CCM_STAT_OFFSET:
		case VISP_CID_CCM_ALL_CONFIG:
		case VISP_CID_CCM_ALL_STATUS:
			ret = visp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
			break;

		default:
			dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
			return -EACCES;
	}

	return ret;
}

static const struct v4l2_ctrl_ops visp_ccm_ctrl_ops = {
	.s_ctrl = visp_ccm_s_ctrl,
	.g_volatile_ctrl = visp_ccm_g_ctrl,
};

const struct v4l2_ctrl_config visp_ccm_ctrls[] = {
	{
		.ops = &visp_ccm_ctrl_ops,
		.id = VISP_CID_CCM_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_ccm_enable",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_ccm_ctrl_ops,
		.id = VISP_CID_CCM_RESET,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_ccm_reset",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		/* manual/auto */
		.ops = &visp_ccm_ctrl_ops,
		.id = VISP_CID_CCM_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_ccm_mode",
		.step = 1,
		.min = 0,
		.max = 1,
	},
	{
		.ops = &visp_ccm_ctrl_ops,
		.id = VISP_CID_CCM_MATRIX,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_ccm_matrix",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {3, 3, 0, 0},
	},
	{
		/* float 3x array -2047~2047 */
		.ops = &visp_ccm_ctrl_ops,
		.id = VISP_CID_CCM_OFFSET,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_ccm_offset",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {3},
	},
	{
		.ops = &visp_ccm_ctrl_ops,
		.id = VISP_CID_CCM_STAT_MATRIX,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_ccm_stat_matrix",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {3, 3, 0, 0},
	},
	{
		/* float 3x array -2047~2047 */
		.ops = &visp_ccm_ctrl_ops,
		.id = VISP_CID_CCM_STAT_OFFSET,
		.type = V4L2_CTRL_TYPE_U32,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_ccm_stat_offset",
		.step = 1,
		.min = 0,
		.max = 0xFFFFFFFF,
		.dims = {3},
	},
	{
		/* uint8_t data of CamDeviceCcmConfig_t */
		.ops = &visp_ccm_ctrl_ops,
		.id = VISP_CID_CCM_ALL_CONFIG,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_ccm_all_config",
		.step = 1,
		.min = 0,
		.max = 0xFF,
		.dims = {52},
	},
	{
		/* uint8_t data of CamDeviceCcmStatus_t */
		.ops = &visp_ccm_ctrl_ops,
		.id = VISP_CID_CCM_ALL_STATUS,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_ccm_all_status",
		.step = 1,
		.min = 0,
		.max = 0xFF,
		.dims = {56},
	},
};

int visp_ccm_ctrl_count(void)
{
	return ARRAY_SIZE(visp_ccm_ctrls);
}

int visp_ccm_ctrl_create(struct visp_dev *isp_dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(visp_ccm_ctrls); i++)
	{
		v4l2_ctrl_new_custom(&isp_dev->ctrl_handler, &visp_ccm_ctrls[i], NULL);
		if (isp_dev->ctrl_handler.error)
		{
			dev_err(isp_dev->dev, "reigster isp ccm ctrl %s failed %d.\n",
					visp_ccm_ctrls[i].name, isp_dev->ctrl_handler.error);
		}
	}

	return 0;
}