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
 *   @file dct_assert.h
 *
 *  This file defines the API for the assertion facility of the embedded lib.
 *
 *****************************************************************************/
/*****************************************************************************/
/**
 * @defgroup module_assert Assert macros
 *
 * @brief The assertion system used by Dream Chip.
 *
 * Example use of the assert system:
 *
 *
 * - In your source file just use the macro
 *
 * @code
 * void foo( uint8_t* pData, size_t size)
 * {
 *     DCT_ASSERT(pData != NULL);
 *     DCT_ASSERT(size > 0);
 * }
 * @endcode
 *
 * @{
 *
 *****************************************************************************/
#ifndef ASSERT_H_
#define ASSERT_H_

#define RET_FAILURE 1 //!< general failure

/**
 * @brief   The type of the assert handler. @see assert_handler
 *
 *****************************************************************************/
typedef void (*ASSERT_HANDLER)(void) __attribute__((noreturn));

/**
 *          The assert handler is a function that is called in case an
 *          assertion failed. If no handler is registered, which is the
 *          default, exit() is called.
 *
 *****************************************************************************/
extern ASSERT_HANDLER assert_handler;

#if defined(ENABLE_ASSERT) || !defined(NDEBUG)
/**
 *              Dump information on stderr and exit.
 *
 *  @param      file  Filename where assertion occured.
 *  @param      line  Linenumber where assertion occured.
 *
 *****************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
	void
	exit_(const char *file, int line) __attribute__((noreturn));

/**
 *              The assert macro.
 *
 *  @param      exp Expression which assumed to be true.
 *
 *****************************************************************************/
#define DCT_ASSERT(exp)                        \
	do                                         \
	{                                          \
		if (!(exp))                            \
		{                                      \
			static CHAR filename[] = __FILE__; \
			exit_(&filename[0], __LINE__);     \
		}                                      \
	} while (0)
#else
#define DCT_ASSERT(exp) \
	do                  \
	{                   \
		if ((exp))      \
		{               \
		}               \
		else            \
		{               \
		}               \
	} while (0)
#endif

/* @} module_tracer*/

#endif /*ASSERT_H_*/
