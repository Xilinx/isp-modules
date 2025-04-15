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
 *   @file trace.c
 *
 *	This file defines the implementation for the tracing facility of the
 *	embedded lib.
 *
 *****************************************************************************/

#include "ebase/trace.h"

#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "ebase/dct_assert.h"

#ifndef NDEBUG

static int glb_level = MAX_LEVEL;

static Tracer* tracerListHead = NULL;

// use macro instead of variable, or build error as "variable length array folded to constant array as an extension"
#define BUFFSIZE 1024

/*	For some stupid reason beyond my imagination, gccs stdio.h doesnt	*/
/*	support vsnprintf() in strict c99 mode. Only happens in cygwin.		*/
#if defined(__GNUC__) && defined(__CYGWIN__) && !defined(PICO)
int vsnprintf(char*, size_t, const char*, __VALIST);
#endif

int getTraceLevel(void)
{
	return glb_level;
}
void setTraceLevel(int new_level)
{
	if (TRACE_OFF != new_level)
	{
		new_level = (~(((uint32_t)(new_level)) - 1u)) & MAX_LEVEL;
	}
	glb_level = new_level;
}

void enableTracer(Tracer* t)
{
	DCT_ASSERT(t);
	t->enabled = 1;
}

void disableTracer(Tracer* t)
{
	DCT_ASSERT(t);
	t->enabled = 0;
}

void setTracerFile(Tracer* t, FILE* f)
{
	DCT_ASSERT(t);
	t->fp = f;
}

void flushTracer(const Tracer* t)
{
	if (t->fp)
	{
		(void)fflush(t->fp);
	}
}

static void addToList(Tracer* tracer)
{
	if (tracerListHead)
	{
		tracer->next = tracerListHead;
	}
	tracer->linked = 1;
	tracerListHead = tracer;
}

Tracer* getTracerList(void)
{
	return tracerListHead;
}

void trace(Tracer* tracer, const CHAR* sFormat, ...)
{
	char buffer[BUFFSIZE];
	int length;
	va_list args;
	float32_t timeStamp;

	char* TRACE_LOG_LEVEL = getenv("TRACE_LOG_LEVEL");
	if (TRACE_LOG_LEVEL != NULL)
	{
		int log_level = atoi(TRACE_LOG_LEVEL);
		if (log_level >= 0 && log_level <= MAX_LEVEL) glb_level = log_level;
	}

	DCT_ASSERT(tracer);

	if (!tracer->linked)
	{
		addToList(tracer);
	}
	if ((tracer->level & glb_level) && (tracer->enabled != 0))
	{
		va_start(args, sFormat);
		length = vsnprintf(buffer, BUFFSIZE, sFormat, args);
		if (!((length > 0) && (length < BUFFSIZE)))
		{
			/* message was truncated */
			fprintf(stderr, "Warning: Trace output truncated !");
		}
		va_end(args);

		if (tracer->fp == 0)
		{
			tracer->fp = stdout;
		}

#ifdef SW_TEST
		(void)timeStamp;
		fprintf(tracer->fp, "%s%s", tracer->prefix, buffer);
#else
		osTimeStampNs(&timeStamp);
		fprintf(tracer->fp, "[%.6lf]%s%s", timeStamp, tracer->prefix, buffer);
#endif
		(void)fflush(tracer->fp);
	}
}

#endif /* NDEBUG */
