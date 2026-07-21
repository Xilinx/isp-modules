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

#include "mbox_crc.h"

#define MBOX_CRC16_POLYNOM 0x8005U
#define MBOX_CRC16_MASK 0xFFFFU

static u16 visp_mbox_crc16_update_byte(u16 crc, u8 data)
{
	u16 value = crc ^ ((u16)data << 8);
	int bit;

	for (bit = 0; bit < 8; bit++) {
		if (value & 0x8000U)
			value = (value << 1) ^ MBOX_CRC16_POLYNOM;
		else
			value <<= 1;
	}

	return value & MBOX_CRC16_MASK;
}

u16 visp_mbox_crc16_update(const void *data, size_t len, u16 crc)
{
	const u8 *bytes = data;
	size_t index;

	for (index = 0; index < len; index++)
		crc = visp_mbox_crc16_update_byte(crc, bytes[index]);

	return crc;
}

u16 visp_mbox_crc16(const void *data, size_t len)
{
	return visp_mbox_crc16_update(data, len, MBOX_CRC16_INIT);
}

size_t visp_mbox_checksum_size(const mbox_post_msg *msg)
{
	return visp_mbox_message_used_size(msg);
}

u16 visp_mbox_calculate_checksum(mbox_post_msg *msg)
{
	u32 checksum = msg->checksum;
	size_t crc_len;
	u16 crc;

	if (!msg || msg->size > MAX_PAYLOAD_SIZE)
		return 0;

	crc_len = visp_mbox_checksum_size(msg);
	msg->checksum = 0;
	crc = visp_mbox_crc16(msg, crc_len);
	msg->checksum = checksum;

	return crc;
}
