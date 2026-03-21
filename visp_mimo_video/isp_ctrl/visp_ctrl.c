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

#include "visp_ctrl.h"

#include <media/v4l2-ioctl.h>

#include "visp_driver.h"

#if defined(ISP_AE_V4)
#include "visp_ae.h"
#endif

#if defined(ISP_AWB_V4)
#include "visp_awb.h"
#endif

#if defined(ISP_GC_V2)
#include "visp_gc.h"
#endif

#if defined(ISP_2DNR_V5_2)
#include "visp_2dnr.h"
#endif

#if defined(ISP_EE_V2)
#include "visp_ee.h"
#endif

#if defined(ISP_CPROC)
#include "visp_cproc.h"
#endif

#if defined(ISP_WDR_V5_2)
#include "visp_wdr.h"
#endif

#if defined(ISP_BLS)
#include "visp_bls.h"
#endif

#if defined(ISP_CCM_V1_1)
#include "visp_ccm.h"
#endif

#if defined(ISP_DPCC)
#include "visp_dpcc.h"
#endif

#if defined(ISP_GE)
#include "visp_ge.h"
#endif

#if defined(ISP_LSC_V3)
#include "visp_lsc.h"
#endif

#if defined(ISP_WB_V1_1)
#include "visp_wb.h"
#endif

#if defined(ISP_DMSC_V2)
#include "visp_dmsc.h"
#endif

#if defined(ISP_EXP_V2)
#include "visp_exp.h"
#endif

#if defined(ISP_GTM_V1)
#include "visp_gtm.h"
#endif

#if defined(ISP_RGBIR_V2_1)
#include "visp_rgbir.h"
#endif

#if defined(ISP_DG_V1)
#include "visp_dg.h"
#endif

#if defined(ISP_CPD_V1_1)
#include "visp_cpd.h"
#endif

#if defined(ISP_HIST64)
#include "visp_hist64.h"
#endif

#if defined(ISP_HIST256)
#include "visp_hist256.h"
#endif

#include "visp_base.h"

int visp_ctrl_init(struct visp_dev *isp_dev)
{
	uint32_t ctrl_count = 0;

#if defined(ISP_AE_V4)
	pr_err("#### %s %d \n", __func__, __LINE__);
	ctrl_count += visp_ae_ctrl_count();
#endif

#if defined(ISP_AWB_V4)
	ctrl_count += visp_awb_ctrl_count();
#endif

#if defined(ISP_GC_V2)
	ctrl_count += visp_gc_ctrl_count();
#endif

#if defined(ISP_2DNR_V5_2)
	ctrl_count += visp_2dnr_ctrl_count();
#endif

#if defined(ISP_EE_V2)
	ctrl_count += visp_ee_ctrl_count();
#endif

#if defined(ISP_CPROC)
	ctrl_count += visp_cproc_ctrl_count();
#endif

#if defined(ISP_WDR_V5_2)
	ctrl_count += visp_wdr_ctrl_count();
#endif

#if defined(ISP_BLS)
	ctrl_count += visp_bls_ctrl_count();
#endif

#if defined(ISP_CCM_V1_1)
	ctrl_count += visp_ccm_ctrl_count();
#endif

#if defined(ISP_DPCC)
	ctrl_count += visp_dpcc_ctrl_count();
#endif

#if defined(ISP_GE)
	ctrl_count += visp_ge_ctrl_count();
#endif

#if defined(ISP_LSC_V3)
	ctrl_count += visp_lsc_ctrl_count();
#endif

#if defined(ISP_WB_V1_1)
	ctrl_count += visp_wb_ctrl_count();
#endif

#if defined(ISP_DMSC_V2)
	ctrl_count += visp_dmsc_ctrl_count();
#endif

#if defined(ISP_EXP_V2)
	ctrl_count += visp_exp_ctrl_count();
#endif

#if defined(ISP_GTM_V1)
	ctrl_count += visp_gtm_ctrl_count();
#endif

#if defined(ISP_RGBIR_V2_1)
	ctrl_count += visp_rgbir_ctrl_count();
#endif

#if defined(ISP_DG_V1)
	ctrl_count += visp_dg_ctrl_count();
#endif

#if defined(ISP_CPD_V1_1)
	ctrl_count += visp_cpd_ctrl_count();
#endif

#if defined(ISP_HIST64)
	ctrl_count += visp_hist64_ctrl_count();
#endif

#if defined(ISP_HIST256)
	ctrl_count += visp_hist256_ctrl_count();
#endif

	ctrl_count += visp_base_ctrl_count();

	pr_err("#### %s %d \n", __func__, __LINE__);
	v4l2_ctrl_handler_init(&isp_dev->ctrl_handler, ctrl_count);

#if defined(ISP_AE_V4)
	visp_ae_ctrl_create(isp_dev);
#endif

#if defined(ISP_AWB_V4)
	visp_awb_ctrl_create(isp_dev);
#endif

#if defined(ISP_GC_V2)
	visp_gc_ctrl_create(isp_dev);
#endif

#if defined(ISP_2DNR_V5_2)
	visp_2dnr_ctrl_create(isp_dev);
#endif

#if defined(ISP_EE_V2)
	visp_ee_ctrl_create(isp_dev);
#endif

#if defined(ISP_CPROC)
	visp_cproc_ctrl_create(isp_dev);
#endif

#if defined(ISP_WDR_V5_2)
	visp_wdr_ctrl_create(isp_dev);
#endif

#if defined(ISP_BLS)
	visp_bls_ctrl_create(isp_dev);
#endif

#if defined(ISP_CCM_V1_1)
	visp_ccm_ctrl_create(isp_dev);
#endif

#if defined(ISP_DPCC)
	visp_dpcc_ctrl_create(isp_dev);
#endif

#if defined(ISP_GE)
	visp_ge_ctrl_create(isp_dev);
#endif

#if defined(ISP_LSC_V3)
	visp_lsc_ctrl_create(isp_dev);
#endif

#if defined(ISP_WB_V1_1)
	visp_wb_ctrl_create(isp_dev);
#endif

#if defined(ISP_DMSC_V2)
	visp_dmsc_ctrl_create(isp_dev);
#endif

#if defined(ISP_EXP_V2)
	visp_exp_ctrl_create(isp_dev);
#endif

#if defined(ISP_GTM_V1)
	visp_gtm_ctrl_create(isp_dev);
#endif

#if defined(ISP_RGBIR_V2_1)
	visp_rgbir_ctrl_create(isp_dev);
#endif

#if defined(ISP_DG_V1)
	visp_dg_ctrl_create(isp_dev);
#endif

#if defined(ISP_CPD_V1_1)
	visp_cpd_ctrl_create(isp_dev);
#endif

#if defined(ISP_HIST64)
	visp_hist64_ctrl_create(isp_dev);
#endif

#if defined(ISP_HIST256)
	visp_hist256_ctrl_create(isp_dev);
#endif

	visp_base_ctrl_create(isp_dev);

	isp_dev->sd.ctrl_handler = &isp_dev->ctrl_handler;

	pr_err("#### %s %d \n", __func__, __LINE__);
	return 0;
}

int visp_ctrl_destroy(struct visp_dev *isp_dev)
{
	v4l2_ctrl_handler_free(&isp_dev->ctrl_handler);

	return 0;
}
