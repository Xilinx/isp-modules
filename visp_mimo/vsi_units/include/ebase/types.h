/* SPDX-License-Identifier: MIT*/
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

/* VeriSilicon 2020 */

/**
 *   @file types.h
 *
 *  This file defines some basic type names like the int types.
 *
 *****************************************************************************/
#ifndef TYPES_H_
#define TYPES_H_

// #include "linux_compat.h"
// #include <limits.h>
#if 0
#ifndef __cplusplus
    /* Only C99 compilers know stdbool */
#if (__STDC_VERSION__ >= 199901L) || defined(CONFIG_HAVE_STDBOOL)
#include <stdbool.h>
#else
//#define bool  unsigned  int
//#define true    (1)
//#define false   (0)
#endif
#endif
#endif

#if defined(__GNUC__)
#if !defined(INLINE)
#define INLINE (static inline)
#endif
#endif

// #include <stddef.h>
#if 1
#if 0
#if defined(__cplusplus) || ((__STDC_VERSION__ >= 199901L))
   //#include <stdint.h>
#else
#endif
#endif
// #include <stdint.h>
/* We only check for __int8_t_defined */
/* as this is all that gcc defines. */
#if !defined(__int8_t_defined)

/* In the rare cases that a system does not define
 * __int8_t_defined (Android p.e.) we check for the
 * include guard of the stdint header */
#if !defined(_STDINT_H)

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;

#if !defined(_MSC_VER)
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#else
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif /* #if !defined(_MSC_VER) */

typedef unsigned int uint_least32_t;
typedef int int_least32_t;
typedef unsigned int uint_least8_t;
typedef unsigned int uint;
typedef unsigned char uchar;

#endif /* #if !defined(_STDINT_H) */
#endif /* #if !defined(__int8_t_defined) */
#endif /* #if defined(__cplusplus) || ((__STDC_VERSION__ >= 199901L)) */

#ifndef NULL
#define NULL ((void *)0)
#endif

/* make lint happy: */
typedef char CHAR;
typedef char char_t; /* like suggested in  Misra 6.3 (P. 29) */
typedef float float32_t;
typedef double float64_t;
typedef long double float128_t;
typedef enum {
	BOOL_FALSE = 0,
	BOOL_TRUE = (!BOOL_FALSE),
	DUMMY_BOOL = 0xDEADFEED
} bool_t;

#define UNUSED_PARAM(unref_param) ((void)(unref_param));
#define CAST_POINTER_TO_UINT32(pointer) ((uint32_t)(pointer))
#define CAST_POINTER_TO_INT32(pointer) ((int32_t)(pointer))
#define CAST_UINT32_TO_POINTER(pointerType, value) ((pointerType)(value))
#define CAST_INT32_TO_POINTER(value) ((int32_t *)(value))
#define N_ELEMENTS(s) (sizeof(s) / sizeof((s)[0]))
#define ABS(a) ((a) > 0 ? (a) : -(a))

#endif /*TYPES_H_*/
