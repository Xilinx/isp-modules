/******************************************************************************\
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

/**
 *   @file offline_trace.c
 *
 *  This file defines the implementation for the tracing facility of the
 *  embedded lib.
 *
 *****************************************************************************/

#include "ebase/offline_trace.h"

#include <dirent.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ebase/dct_assert.h"

#ifdef ISP_OFFLINE_TEST
#define FLIE_PATH_NAME_LEN_MAX 512
#define BUFFSIZE 1024

static uint32_t gframeId[MAX_SUPPORT_INSTANCE] = {0};
static char CasePrefix[FLIE_PATH_NAME_LEN_MAX] = "\0";

char glog_type[3][6] = {"INFO", "ERROR", "WARN"};
/*  For some stupid reason beyond my imagination, gccs stdio.h doesn't  */
/*  support vsnprintf() in strict c99 mode. Only happens in cygwin.     */
#if defined(__GNUC__) && defined(__CYGWIN__) && !defined(PICO)
int vsnprintf(char *, size_t, const char *, __VALIST);
#endif

void enableOffLineTrace(Offline_Tracer *t)
{
	DCT_ASSERT(t);
	t->enabled = 1;
}

void disableOffLineTrace(Offline_Tracer *t)
{
	DCT_ASSERT(t);
	t->enabled = 0;
}

void setOffLineTraceFile(Offline_Tracer *t, FILE *f)
{
	DCT_ASSERT(t);
	t->fp = f;
}

void setoffLineTraceFrameid(uint32_t ispId, uint32_t i)
{
	// DCT_ASSERT();
	gframeId[ispId] = i;
}

void setOffLineTraceCasePrefix(const char *pCasePrefix)
{
	DCT_ASSERT(pCasePrefix != NULL);
	sprintf(CasePrefix, "offline_test/%s", pCasePrefix);
	if (opendir((const char *)CasePrefix) == NULL)
	{
		DCT_ASSERT(mkdir((const char *)CasePrefix, 0777) == 0);
		chmod((const char *)CasePrefix, 0777);
	}
}

void OpenOffLineTrace(Offline_Tracer *off_Tracer, uint32_t instanceId,
					  const char *path_suffix)
{
	char file_path[FLIE_PATH_NAME_LEN_MAX] = "\0";
	char temp_dir[FLIE_PATH_NAME_LEN_MAX] = "\0";
	char temp_prefix[FLIE_PATH_NAME_LEN_MAX] = "\0";
	char temp_temp_prefix[FLIE_PATH_NAME_LEN_MAX] = "\0";

	FILE *fp = NULL;

	DCT_ASSERT(off_Tracer);
	DCT_ASSERT(CasePrefix[0]);
	DCT_ASSERT(path_suffix);

	off_Tracer[instanceId].enabled = 1;
	off_Tracer[instanceId].instanceId = instanceId;
	gframeId[instanceId] = 1;

	strncpy(temp_prefix, CasePrefix, sizeof(temp_prefix));
	temp_prefix[FLIE_PATH_NAME_LEN_MAX - 1] = '\0';
	for (uint16_t i = 0, j = 0; i < strlen(path_suffix); ++i)
	{
		if ('.' == path_suffix[i])
		{
			if (0 == i) continue;
			strncpy(temp_temp_prefix, temp_prefix, sizeof(temp_temp_prefix));
			temp_temp_prefix[FLIE_PATH_NAME_LEN_MAX - 1] = '\0';
			sprintf(temp_prefix, "%s/%s", temp_temp_prefix, temp_dir);
			if (opendir((const char *)temp_prefix) == NULL)
			{
				DCT_ASSERT(mkdir((const char *)temp_prefix, 0777) == 0);
				chmod((const char *)temp_prefix, 0777);
			}
			memset(temp_dir, 0, sizeof(temp_dir));
			j = 0;
			continue;
		}
		temp_dir[j++] = path_suffix[i];
	}
	sprintf(file_path, "%s/%sinstance%u.log", temp_prefix, path_suffix,
			instanceId);
	fprintf(stdout, "%s:%s\n", __func__, file_path);
	fp = fopen(file_path, "w+");
	DCT_ASSERT(fp != NULL);
	off_Tracer[instanceId].fp = fp;
	off_Tracer[instanceId].index = 0;
}

void CloseOffLineTrace(Offline_Tracer *t)
{
	DCT_ASSERT(t);
	if (t->fp != NULL && !fclose(t->fp))
	{
		t->fp = NULL;
		CasePrefix[0] = '\0';
	}
	return;
}

void offLinetrace(Offline_Tracer *off_Tracer, const char *log_type,
				  const char *func_name, const char *sFormat, ...)
{
	char buffer[BUFFSIZE];
	int length;
	va_list args;

	DCT_ASSERT(off_Tracer);

	if (off_Tracer->enabled != 0)
	{
		va_start(args, sFormat);
		length = vsnprintf(buffer, BUFFSIZE, sFormat, args);
		if (!((length > 0) && (length < BUFFSIZE)))
		{
			/* message was truncated */
			fprintf(stderr, "Warning: Trace output truncated !");
		}
		va_end(args);

		if (off_Tracer->fp == 0)
		{
			off_Tracer->fp = stdout;
		}

		fprintf(off_Tracer->fp, "%d: instance[%d]_frame[%d] [%s][%s]:%s:%s",
				off_Tracer->index, off_Tracer->instanceId,
				gframeId[off_Tracer->instanceId], off_Tracer->name, log_type,
				func_name, buffer);
		off_Tracer->index++;
		(void)fflush(off_Tracer->fp);
	}
}

#else

void setOffLineTraceCasePrefix(const char* pCasePrefix)
{
	(void)pCasePrefix;
}
#endif /* ISP_OFFLINE_TEST */
