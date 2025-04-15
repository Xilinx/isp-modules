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

#ifndef __VSI_ERRNO_H__
#define __VSI_ERRNO_H__

#define VSI_SUCCESS 0
#define VSI_FAILURE (-1)
#define VSI_NULL  ((void *) 0)

typedef enum ErrCode_E
{
    VSI_ERR_INVALID_DEVID     = 1,
    VSI_ERR_INVALID_PORTID    = 2,
    VSI_ERR_INVALID_CHNID     = 3,
    VSI_ERR_ILLEGAL_PARAM     = 4,
    VSI_ERR_EXIST             = 5,
    VSI_ERR_UNEXIST           = 6,
    VSI_ERR_NULL_PTR          = 7,
    VSI_ERR_NOT_CONFIG        = 8,
    VSI_ERR_NOT_SUPPORT       = 9,
    VSI_ERR_NOT_PERM          = 10,
    VSI_ERR_NOMEM             = 11,
    VSI_ERR_NOBUF             = 12,
    VSI_ERR_BUF_EMPTY         = 13,
    VSI_ERR_BUF_FULL          = 14,
    VSI_ERR_NOTREADY          = 15,
    VSI_ERR_BADADDR           = 17,
    VSI_ERR_BUSY              = 18,
    VSI_ERR_TIMEOUT           = 19,
    VSI_ERR_BUTT              = 256
} VSIErrCode_E;

#endif
