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

#include "visp_sensor.h"

#include <media/v4l2-ioctl.h>

#include "visp_ctrl.h"
#include "visp_driver.h"
#include "visp_event.h"

static int visp_sensor_get_sensor_number(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	uint16_t sensor_num = 0;

	ret = vsi_cam_device_sensor_get_number(isp_dev, &sensor_num);
	if (ret) {
		dev_err(isp_dev->dev,
			"CamDevice get support sensor number failed, ret is %d\n",
			ret);
		return ret;
	}

	*ctrl->p_new.p_s32 = (int32_t)sensor_num;

	return ret;
}

static int visp_sensor_get_sensor_list(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	uint16_t sensor_num = 0;
	int ctrl_size = ctrl->elem_size * ctrl->elems;
	cam_device_sensor_list_info_t *p_sensor_list = NULL;

	ret = vsi_cam_device_sensor_get_number(isp_dev, &sensor_num);
	if (ret) {
		dev_err(isp_dev->dev,
			"CamDevice get support sensor number failed, ret is %d\n",
			ret);
		return ret;
	}

	/* if sensor number too large */
	if (ctrl_size < sizeof(cam_device_sensor_list_info_t) * sensor_num)
		sensor_num = ctrl_size / sizeof(cam_device_sensor_list_info_t);

	p_sensor_list = (cam_device_sensor_list_info_t *)ctrl->p_new.p_u8;
	memset(p_sensor_list, 0, sizeof(cam_device_sensor_list_info_t) * sensor_num);

	ret = vsi_cam_device_sensor_get_list_info(isp_dev, p_sensor_list, sensor_num);
	if (ret) {
		dev_err(isp_dev->dev,
			"CamDevice get support sensor list failed, ret is %d\n",
			ret);
		return ret;
	}

	return ret;
}

static int visp_sensor_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	int32_t mode = -1;
	int port = isp_dev->ctrl_pad / MEDIA_ISP_PORT_PAD_COUNT;

	switch (ctrl->id) {
	case VISP_CID_SENSOR_NAME:
	{
		size_t ctrl_size = ctrl->elem_size * ctrl->elems;
		size_t name_len = min_t(size_t, ctrl_size,
				 sizeof(isp_dev->isp_ports[port].sensor_info.name) - 1);

		memset(isp_dev->isp_ports[port].sensor_info.name, 0,
		       sizeof(isp_dev->isp_ports[port].sensor_info.name));
		memcpy(isp_dev->isp_ports[port].sensor_info.name,
		       ctrl->p_new.p_u8, name_len);
		break;
	}
		break;

	case VISP_CID_SENSOR_MODE:
		memcpy(&mode, ctrl->p_new.p_s32, sizeof(int32_t));
		isp_dev->isp_ports[port].sensor_info.mode = (uint8_t)mode;
		break;

	case VISP_CID_SENSOR_REG_ADDR:
		ret = visp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
		break;

	case VISP_CID_SENSOR_REG_VAL:
		ret = visp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
		break;

	default:
		dev_err(isp_dev->dev, "unknown v4l2 ctrl id %d\n", ctrl->id);
		return -EACCES;
	}

	return ret;
}

static int visp_sensor_g_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;
	struct visp_dev *isp_dev =
		container_of(ctrl->handler, struct visp_dev, ctrl_handler);

	int32_t *p_mode = NULL;
	int port = isp_dev->ctrl_pad / MEDIA_ISP_PORT_PAD_COUNT;

	switch (ctrl->id) {
	case VISP_CID_SENSOR_NAME:
	{
		size_t ctrl_size = ctrl->elem_size * ctrl->elems;
		size_t name_len = min_t(size_t, ctrl_size,
				 sizeof(isp_dev->isp_ports[port].sensor_info.name));

		memset(ctrl->p_new.p_u8, 0, ctrl_size);
		memcpy(ctrl->p_new.p_u8,
		       isp_dev->isp_ports[port].sensor_info.name,
		       name_len);
		break;
	}
		break;

	case VISP_CID_SENSOR_MODE:
		p_mode = (int32_t *)ctrl->p_new.p_s32;
		*p_mode = (int32_t)isp_dev->isp_ports[port].sensor_info.mode;
		break;

	case VISP_CID_SENSOR_NUMBER:
		ret = visp_sensor_get_sensor_number(ctrl);
		break;

	case VISP_CID_SENSOR_LIST:
		ret = visp_sensor_get_sensor_list(ctrl);
		break;

	case VISP_CID_SENSOR_REG_ADDR:
		ret = visp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
		break;

	case VISP_CID_SENSOR_REG_VAL:
		ret = visp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
		break;

	default:
		dev_err(isp_dev->dev, "unknown v4l2 ctrl id %d\n", ctrl->id);
		return -EACCES;
	}

	return ret;
}

static const struct v4l2_ctrl_ops visp_sensor_ctrl_ops = {
	.s_ctrl = visp_sensor_s_ctrl,
	.g_volatile_ctrl = visp_sensor_g_ctrl,
};

const struct v4l2_ctrl_config visp_sensor_ctrls[] = {
	{
		.ops = &visp_sensor_ctrl_ops,
		.id = VISP_CID_SENSOR_NAME,
		.type = V4L2_CTRL_TYPE_STRING,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_sensor_name",
		.step = 1,
		.min = 0,
		.max = 19,
	},
	{
		.ops = &visp_sensor_ctrl_ops,
		.id = VISP_CID_SENSOR_MODE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_sensor_mode",
		.step = 1,
		.min = 0,
		.max = 0xF,
	},
	{
		.ops = &visp_sensor_ctrl_ops,
		.id = VISP_CID_SENSOR_XML,
		.type = V4L2_CTRL_TYPE_STRING,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_sensor_xml",
		.step = 1,
		.min = 0,
		.max = 63,
	},
	{
		.ops = &visp_sensor_ctrl_ops,
		.id = VISP_CID_SENSOR_MANU_JSON,
		.type = V4L2_CTRL_TYPE_STRING,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_sensor_manu_json",
		.step = 1,
		.min = 0,
		.max = 127,
	},
	{
		.ops = &visp_sensor_ctrl_ops,
		.id = VISP_CID_SENSOR_AUTO_JSON,
		.type = V4L2_CTRL_TYPE_STRING,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_sensor_auto_json",
		.step = 1,
		.min = 0,
		.max = 127,
	},
	{
		.ops = &visp_sensor_ctrl_ops,
		.id = VISP_CID_SENSOR_ONE_JSON,
		.type = V4L2_CTRL_TYPE_STRING,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_sensor_one_json",
		.step = 1,
		.min = 0,
		.max = 127,
	},
	{
	/* SensorNumber force only list 10 */
		.ops  = &visp_sensor_ctrl_ops,
		.id   = VISP_CID_SENSOR_NUMBER,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_sensor_number",
		.step = 1,
		.min  = 0,
		.max  = 10,
	},
	{
		.ops = &visp_sensor_ctrl_ops,
		.id = VISP_CID_SENSOR_LIST,
		.type = V4L2_CTRL_TYPE_U8,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_sensor_list",
		.step = 1,
		.min = 0,
		.max = 0xFF,
		.dims = {0x8b8 * 10},	/* the size dependent on number */
	},
	{
		.ops = &visp_sensor_ctrl_ops,
		.id = VISP_CID_SENSOR_REG_ADDR,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_sensor_reg_addr",
		.step = 1,
		.min = 0,
		.max = 0xFFFF,
		.dims = {1},
	},
	{
		.ops = &visp_sensor_ctrl_ops,
		.id = VISP_CID_SENSOR_REG_VAL,
		.type = V4L2_CTRL_TYPE_U16,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
		.name = "isp_sensor_reg_val",
		.step = 1,
		.min = 0,
		.max = 0xFFFF,
		.dims = {1},
	}
};

int visp_sensor_ctrl_count(void)
{
	return ARRAY_SIZE(visp_sensor_ctrls);
}

int visp_sensor_ctrl_create(struct visp_dev *isp_dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(visp_sensor_ctrls); i++)
	{
		v4l2_ctrl_new_custom(&isp_dev->ctrl_handler, &visp_sensor_ctrls[i],
							 NULL);
		if (isp_dev->ctrl_handler.error)
		{
			dev_err(isp_dev->dev, "reigster isp sensor ctrl %s failed %d.\n",
					visp_sensor_ctrls[i].name, isp_dev->ctrl_handler.error);
		}
	}

	return 0;
}
