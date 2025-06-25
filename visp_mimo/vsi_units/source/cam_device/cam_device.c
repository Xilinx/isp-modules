// SPDX-License-Identifier: MIT
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

#include "cam_device.h"

cam_device_ispcore_t g_cam_dev_ispcore;

void cam_device_ispcore_init(void)
{
	static bool_t init = BOOL_FALSE;

	if (init == BOOL_FALSE) {
		uint32_t hw_id = 0;

		for (; hw_id < CAMDEV_HARDWARE_ID_MAX; hw_id++) {
			uint32_t vt_id = 0;

			for (; vt_id < CAMDEV_VIRTUAL_ID_MAX; vt_id++) {
				g_cam_dev_ispcore.h_cam_dev_set[hw_id][vt_id] =
				    NULL;
			}
		}
		init = BOOL_TRUE;
	}
}

RESULT cam_device_request_instance(uint32_t hw_id,
				   cam_device_handle_t *p_cam_devhandle,
				   uint32_t *pvt_id)
{
	uint32_t index = 0;

	if (hw_id > CAMDEV_HARDWARE_ID_MAX)
		return RET_OUTOFRANGE;
	for (; index < CAMDEV_VIRTUAL_ID_MAX; index++) {
		if (g_cam_dev_ispcore.h_cam_dev_set[hw_id][index] == NULL)
			break;
	}
	if (index < CAMDEV_VIRTUAL_ID_MAX) {
		g_cam_dev_ispcore.h_cam_dev_set[hw_id][index] =
		    /*malloc(sizeof(cam_device_context_t));*/ kzalloc(
			sizeof(cam_device_context_t), GFP_KERNEL);
		if (g_cam_dev_ispcore.h_cam_dev_set[hw_id][index] == NULL)
			return RET_OUTOFMEM;
		*p_cam_devhandle =
		    g_cam_dev_ispcore.h_cam_dev_set[hw_id][index];
		*pvt_id = index;
		return RET_SUCCESS;
	} else {
		return RET_NOTAVAILABLE;
	}
}

RESULT cam_device_free_instance(cam_device_handle_t cam_devhandle,
				uint32_t hw_id)
{
	uint32_t index = 0;

	if (hw_id > CAMDEV_HARDWARE_ID_MAX)
		return RET_OUTOFRANGE;
	for (; index < CAMDEV_VIRTUAL_ID_MAX; index++) {
		if (cam_devhandle ==
		    g_cam_dev_ispcore.h_cam_dev_set[hw_id][index])
			break;
	}
	if (index < CAMDEV_VIRTUAL_ID_MAX) {
		kfree(cam_devhandle);
		g_cam_dev_ispcore.h_cam_dev_set[hw_id][index] = NULL;
		return RET_SUCCESS;
	} else {
		return RET_NOTAVAILABLE;
	}
}

RESULT cam_device_instance_id_mapping(uint32_t hw_id, uint32_t vt_id,
				      uint32_t *p_instance_id)
{
	RESULT ret = RET_SUCCESS;

	/* Hardware Pipeline id / Virtual Device id Mapping Policy */
	/* The mapping can be modified according to system configurations */
	/*----------------------------------------------------------------------------
	 */
	/*    id                        |   HW              |   VT */
	/*----------------------------------------------------------------------------
	 */
	/*    0                         |   0               |   0 */
	/*    1                         |   0               |   1 */
	/*    2                         |   0               |   2 */
	/*    ..                        |   ..              |   .. */
	/*    CAMDEV_VIRTUAL_ID_MAX-1   |   0               |
	 * CAMDEV_VIRTUAL_ID_MAX -1      */
	/*----------------------------------------------------------------------------
	 */
	/*    CAMDEV_VIRTUAL_ID_MAX     |   1               |   0 */
	/*    CAMDEV_VIRTUAL_ID_MAX+1   |   1               |   1 */
	/*    ..                        |   ..              |   .. */
	/*    CAMDEV_VIRTUAL_ID_MAX*2-1 |   1               |
	 * CAMDEV_VIRTUAL_ID_MAX -1      */
	/*-------------------------------------------------------------------------
	 */
	if (p_instance_id == NULL)
		return RET_NULL_POINTER;

	if (hw_id >= CAMDEV_HARDWARE_ID_MAX)
		return RET_UNSUPPORT_ID;

	if (vt_id >= CAMDEV_VIRTUAL_ID_MAX)
		return RET_UNSUPPORT_ID;

	*p_instance_id = hw_id * CAMDEV_VIRTUAL_ID_MAX + vt_id;
	return ret;
}
