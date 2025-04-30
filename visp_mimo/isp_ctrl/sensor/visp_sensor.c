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

#include <media/v4l2-ioctl.h>
#include "visp_driver.h"
#include "visp_ctrl.h"
#include "visp_sensor.h"
#include "visp_event.h"

static int visp_sensor_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct visp_dev *isp_dev =
        container_of(ctrl->handler, struct visp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VISP_CID_SENSOR_NAME:
        case VISP_CID_SENSOR_MODE:
        case VISP_CID_SENSOR_XML:
        case VISP_CID_SENSOR_MANU_JSON:
        case VISP_CID_SENSOR_AUTO_JSON:
        case VISP_CID_SENSOR_ONE_JSON:
            ret = visp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int visp_sensor_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct visp_dev *isp_dev =
        container_of(ctrl->handler, struct visp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VISP_CID_SENSOR_NAME:
        case VISP_CID_SENSOR_MODE:
        case VISP_CID_SENSOR_XML:
        case VISP_CID_SENSOR_MANU_JSON:
        case VISP_CID_SENSOR_AUTO_JSON:
        case VISP_CID_SENSOR_ONE_JSON:
        case VISP_CID_SENSOR_LIST:
        case VISP_CID_SENSOR_CONNECT:
            ret = visp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
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
        .ops  = &visp_sensor_ctrl_ops,
        .id   = VISP_CID_SENSOR_NAME,
        .type = V4L2_CTRL_TYPE_STRING,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_sensor_name",
        .step = 1,
        .min  = 0,
        .max  = 19,
    },
    {
        .ops  = &visp_sensor_ctrl_ops,
        .id   = VISP_CID_SENSOR_MODE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_sensor_mode",
        .step = 1,
        .min  = 0,
        .max  = 0xFF,
        .dims = {1},
    },
    {
        .ops  = &visp_sensor_ctrl_ops,
        .id   = VISP_CID_SENSOR_XML,
        .type = V4L2_CTRL_TYPE_STRING,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_sensor_xml",
        .step = 1,
        .min  = 0,
        .max  = 63,
    },
    {
        .ops  = &visp_sensor_ctrl_ops,
        .id   = VISP_CID_SENSOR_MANU_JSON,
        .type = V4L2_CTRL_TYPE_STRING,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_sensor_manu_json",
        .step = 1,
        .min  = 0,
        .max  = 127,
    },
    {
        .ops  = &visp_sensor_ctrl_ops,
        .id   = VISP_CID_SENSOR_AUTO_JSON,
        .type = V4L2_CTRL_TYPE_STRING,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_sensor_auto_json",
        .step = 1,
        .min  = 0,
        .max  = 127,
    },
    {
        .ops  = &visp_sensor_ctrl_ops,
        .id   = VISP_CID_SENSOR_ONE_JSON,
        .type = V4L2_CTRL_TYPE_STRING,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_sensor_one_json",
        .step = 1,
        .min  = 0,
        .max  = 127,
    },
    {
        .ops  = &visp_sensor_ctrl_ops,
        .id   = VISP_CID_SENSOR_LIST,
        .type = V4L2_CTRL_TYPE_STRING,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_sensor_list",
        .step = 1,
        .min  = 0,
        .max  = 255,
    },
    {
        .ops  = &visp_sensor_ctrl_ops,
        .id   = VISP_CID_SENSOR_CONNECT,
        .type = V4L2_CTRL_TYPE_STRING,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_sensor_connect",
        .step = 1,
        .min  = 0,
        .max  = 19,
    },
};

int visp_sensor_ctrl_count(void)
{
    return ARRAY_SIZE(visp_sensor_ctrls);
}

int visp_sensor_ctrl_create(struct visp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(visp_sensor_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &visp_sensor_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp sensor ctrl %s failed %d.\n",
                visp_sensor_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;
}
