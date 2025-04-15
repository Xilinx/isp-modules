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

#if defined(ISP_AE_V3) || defined(ISP_AE_V4) || defined(ISP_AE_V4_1)
#include "visp_ae.h"
#endif

#if defined(ISP_AWB_V3) || defined(ISP_AWB_V4) || defined(ISP_AWB_V4_1)
#include "visp_awb.h"
#endif

#if defined(ISP_GC_V1) || defined(ISP_GC_V2)
#include "visp_gc.h"
#endif

#if defined(ISP_2DNR_V3) || defined(ISP_2DNR_V5_2) || defined(ISP_2DNR_V6)
#include "visp_2dnr.h"
#endif

#if defined(ISP_EE_V1) || defined(ISP_EE_V2) || defined(ISP_EE_V3)
#include "visp_ee.h"
#endif

#if defined(ISP_CPROC)
#include "visp_cproc.h"
#endif

#if defined(ISP_WDR_V3) || defined(ISP_WDR_V4) || defined(ISP_WDR_V5) || \
	defined(ISP_WDR_V5_1) || defined(ISP_WDR_V5_2) ||                    \
	defined(ISP_WDR_V5_2_1) || defined(ISP_WDR_V5_3)
#include "visp_wdr.h"
#endif

#if defined(ISP_BLS)
#include "visp_bls.h"
#endif

#if defined(ISP_CCM)
#include "visp_ccm.h"
#endif

#if defined(ISP_DPCC)
#include "visp_dpcc.h"
#endif

#if defined(ISP_GE)
#include "visp_ge.h"
#endif

#if defined(ISP_LSC_V1) || defined(ISP_LSC_V2) || defined(ISP_LSC_V3) || \
	defined(ISP_LSC_V4)
#include "visp_lsc.h"
#endif

#if defined(ISP_WB_V1) || defined(ISP_WB_V1_1)
#include "visp_wb.h"
#endif

#if defined(ISP_3DNR_V1_2) || defined(ISP_3DNR_V2_1) || \
	defined(ISP_3DNR_V2_4) || defined(ISP_3DNR_V3) ||   \
	defined(ISP_3DNR_V3_1) || defined(ISP_3DNR_V4)
#include "visp_3dnr.h"
#endif

#if defined(ISP_AF_V3) || defined(ISP_AF_V4) || defined(ISP_AF_V4_1_1) || \
	defined(ISP_AF_V4_3)
#include "visp_af.h"
#endif

#if defined(ISP_PDAF_V1)
#include "visp_pdaf.h"
#endif

#if defined(ISP_AFM_V1) || defined(ISP_AFM_V1_1) || defined(ISP_AFM_V3)
#include "visp_afm.h"
#endif

#if defined(ISP_DMSC_V1) || defined(ISP_DMSC_V2) || defined(ISP_DMSC_V3)
#include "visp_dmsc.h"
#endif

#if defined(ISP_EXP_V2)
#include "visp_exp.h"
#endif

#if defined(ISP_EXP_V3)
#include "visp_exp_v3.h"
#endif

#if defined(ISP_GTM_V1)
#include "visp_gtm.h"
#endif

#if defined(ISP_HDR_V1_3) || defined(ISP_HDR_V2) || defined(ISP_HDR_V2_1) || \
	defined(ISP_HDR_V2_1_2DOL) || defined(ISP_HDR_V3) ||                     \
	defined(ISP_HDR_V3_2DOL) || defined(ISP_HDR_V3_1)
#include "visp_hdr.h"
#endif

#if defined(ISP_RGBIR_V1) || defined(ISP_RGBIR_V1_1) || \
	defined(ISP_RGBIR_V2) || defined(ISP_RGBIR_V2_1)
#include "visp_rgbir.h"
#endif

#if defined(ISP_DG_V1)
#include "visp_dg.h"
#endif

#if defined(ISP_CMPD_20BIT) || defined(ISP_CMPD_24BIT)
#include "visp_cpd.h"
#endif

#if defined(ISP_DPF_V1)
#include "visp_dpf.h"
#endif

#if defined(ISP_CNR_V1) || defined(ISP_CNR_V2_1) || defined(ISP_CNR_V2_2)
#include "visp_cnr.h"
#endif

#if defined(ISP_GWDR_V1)
#include "visp_gwdr.h"
#endif

#if defined(ISP_YNR_V1)
#include "visp_ynr.h"
#endif

#if defined(ISP_LUT3D_V1)
#include "visp_lut3d.h"
#endif

#include "visp_sensor.h"

int visp_ctrl_init(struct visp_dev *isp_dev)
{
	uint32_t ctrl_count = 0;

#if defined(ISP_AE_V3) || defined(ISP_AE_V4) || defined(ISP_AE_V4_1)
	ctrl_count += visp_ae_ctrl_count();
#endif

#if defined(ISP_AWB_V3) || defined(ISP_AWB_V4) || defined(ISP_AWB_V4_1)
	ctrl_count += visp_awb_ctrl_count();
#endif

#if defined(ISP_GC_V1) || defined(ISP_GC_V2)
	ctrl_count += visp_gc_ctrl_count();
#endif

#if defined(ISP_2DNR_V3) || defined(ISP_2DNR_V5_2) || defined(ISP_2DNR_V6)
	ctrl_count += visp_2dnr_ctrl_count();
#endif

#if defined(ISP_EE_V1) || defined(ISP_EE_V2) || defined(ISP_EE_V3)
	ctrl_count += visp_ee_ctrl_count();
#endif

#if defined(ISP_CPROC)
	ctrl_count += visp_cproc_ctrl_count();
#endif

#if defined(ISP_WDR_V3) || defined(ISP_WDR_V4) || defined(ISP_WDR_V5) || \
	defined(ISP_WDR_V5_1) || defined(ISP_WDR_V5_2) ||                    \
	defined(ISP_WDR_V5_2_1) || defined(ISP_WDR_V5_3)
	ctrl_count += visp_wdr_ctrl_count();
#endif

#if defined(ISP_BLS)
	ctrl_count += visp_bls_ctrl_count();
#endif

#if defined(ISP_CCM)
	ctrl_count += visp_ccm_ctrl_count();
#endif

#if defined(ISP_DPCC)
	ctrl_count += visp_dpcc_ctrl_count();
#endif

#if defined(ISP_GE)
	ctrl_count += visp_ge_ctrl_count();
#endif

#if defined(ISP_LSC_V1) || defined(ISP_LSC_V2) || defined(ISP_LSC_V3) || \
	defined(ISP_LSC_V4)
	ctrl_count += visp_lsc_ctrl_count();
#endif

#if defined(ISP_WB_V1) || defined(ISP_WB_V1_1)
	ctrl_count += visp_wb_ctrl_count();
#endif

#if defined(ISP_3DNR_V1_2) || defined(ISP_3DNR_V2_1) || \
	defined(ISP_3DNR_V2_4) || defined(ISP_3DNR_V3) ||   \
	defined(ISP_3DNR_V3_1) || defined(ISP_3DNR_V4)
	ctrl_count += visp_3dnr_ctrl_count();
#endif

#if defined(ISP_AF_V3) || defined(ISP_AF_V4) || defined(ISP_AF_V4_1_1) || \
	defined(ISP_AF_V4_3)
	ctrl_count += visp_af_ctrl_count();
#endif

#if defined(ISP_PDAF_V1)
	ctrl_count += visp_pdaf_ctrl_count();
#endif

#if defined(ISP_AFM_V1) || defined(ISP_AFM_V1_1) || defined(ISP_AFM_V3)
	ctrl_count += visp_afm_ctrl_count();
#endif

#if defined(ISP_DMSC_V1) || defined(ISP_DMSC_V2) || defined(ISP_DMSC_V3)
	ctrl_count += visp_dmsc_ctrl_count();
#endif

#if defined(ISP_EXP_V2)
	ctrl_count += visp_exp_ctrl_count();
#endif

#if defined(ISP_EXP_V3)
	ctrl_count += visp_exp_v3_ctrl_count();
#endif

#if defined(ISP_GTM_V1)
	ctrl_count += visp_gtm_ctrl_count();
#endif

#if defined(ISP_HDR_V1_3) || defined(ISP_HDR_V2) || defined(ISP_HDR_V2_1) || \
	defined(ISP_HDR_V2_1_2DOL) || defined(ISP_HDR_V3) ||                     \
	defined(ISP_HDR_V3_2DOL) || defined(ISP_HDR_V3_1)
	ctrl_count += visp_hdr_ctrl_count();
#endif

#if defined(ISP_RGBIR_V1) || defined(ISP_RGBIR_V1_1) || \
	defined(ISP_RGBIR_V2) || defined(ISP_RGBIR_V2_1)
	ctrl_count += visp_rgbir_ctrl_count();
#endif

#if defined(ISP_DG_V1)
	ctrl_count += visp_dg_ctrl_count();
#endif

#if defined(ISP_CMPD_20BIT) || defined(ISP_CMPD_24BIT)
	ctrl_count += visp_cpd_ctrl_count();
#endif

#if defined(ISP_DPF_V1)
	ctrl_count += visp_dpf_ctrl_count();
#endif

#if defined(ISP_CNR_V1) || defined(ISP_CNR_V2_1) || defined(ISP_CNR_V2_2)
	ctrl_count += visp_cnr_ctrl_count();
#endif

#if defined(ISP_GWDR_V1)
	ctrl_count += visp_gwdr_ctrl_count();
#endif

#if defined(ISP_YNR_V1)
	ctrl_count += visp_ynr_ctrl_count();
#endif

#if defined(ISP_LUT3D_V1)
	ctrl_count += visp_lut3d_ctrl_count();
#endif

	ctrl_count += visp_sensor_ctrl_count();

	v4l2_ctrl_handler_init(&isp_dev->ctrl_handler, ctrl_count);

#if defined(ISP_AE_V3) || defined(ISP_AE_V4) || defined(ISP_AE_V4_1)
	visp_ae_ctrl_create(isp_dev);
#endif

#if defined(ISP_AWB_V3) || defined(ISP_AWB_V4) || defined(ISP_AWB_V4_1)
	visp_awb_ctrl_create(isp_dev);
#endif

#if defined(ISP_GC_V1) || defined(ISP_GC_V2)
	visp_gc_ctrl_create(isp_dev);
#endif

#if defined(ISP_2DNR_V3) || defined(ISP_2DNR_V5_2) || defined(ISP_2DNR_V6)
	visp_2dnr_ctrl_create(isp_dev);
#endif

#if defined(ISP_EE_V1) || defined(ISP_EE_V2) || defined(ISP_EE_V3)
	visp_ee_ctrl_create(isp_dev);
#endif

#if defined(ISP_CPROC)
	visp_cproc_ctrl_create(isp_dev);
#endif

#if defined(ISP_WDR_V3) || defined(ISP_WDR_V4) || defined(ISP_WDR_V5) || \
	defined(ISP_WDR_V5_1) || defined(ISP_WDR_V5_2) ||                    \
	defined(ISP_WDR_V5_2_1) || defined(ISP_WDR_V5_3)
	visp_wdr_ctrl_create(isp_dev);
#endif

#if defined(ISP_BLS)
	visp_bls_ctrl_create(isp_dev);
#endif

#if defined(ISP_CCM)
	visp_ccm_ctrl_create(isp_dev);
#endif

#if defined(ISP_DPCC)
	visp_dpcc_ctrl_create(isp_dev);
#endif

#if defined(ISP_GE)
	visp_ge_ctrl_create(isp_dev);
#endif

#if defined(ISP_LSC_V1) || defined(ISP_LSC_V2) || defined(ISP_LSC_V3) || \
	defined(ISP_LSC_V4)
	visp_lsc_ctrl_create(isp_dev);
#endif

#if defined(ISP_WB_V1) || defined(ISP_WB_V1_1)
	visp_wb_ctrl_create(isp_dev);
#endif

#if defined(ISP_3DNR_V1_2) || defined(ISP_3DNR_V2_1) || \
	defined(ISP_3DNR_V2_4) || defined(ISP_3DNR_V3) ||   \
	defined(ISP_3DNR_V3_1) || defined(ISP_3DNR_V4)
	visp_3dnr_ctrl_create(isp_dev);
#endif

#if defined(ISP_AF_V3) || defined(ISP_AF_V4) || defined(ISP_AF_V4_1_1) || \
	defined(ISP_AF_V4_3)
	visp_af_ctrl_create(isp_dev);
#endif

#if defined(ISP_PDAF_V1)
	visp_pdaf_ctrl_create(isp_dev);
#endif

#if defined(ISP_AFM_V1) || defined(ISP_AFM_V1_1) || defined(ISP_AFM_V3)
	visp_afm_ctrl_create(isp_dev);
#endif

#if defined(ISP_DMSC_V1) || defined(ISP_DMSC_V2) || defined(ISP_DMSC_V3)
	visp_dmsc_ctrl_create(isp_dev);
#endif

#if defined(ISP_EXP_V2)
	visp_exp_ctrl_create(isp_dev);
#endif

#if defined(ISP_EXP_V3)
	visp_exp_v3_ctrl_create(isp_dev);
#endif

#if defined(ISP_GTM_V1)
	visp_gtm_ctrl_create(isp_dev);
#endif

#if defined(ISP_HDR_V1_3) || defined(ISP_HDR_V2) || defined(ISP_HDR_V2_1) || \
	defined(ISP_HDR_V2_1_2DOL) || defined(ISP_HDR_V3) ||                     \
	defined(ISP_HDR_V3_2DOL) || defined(ISP_HDR_V3_1)
	visp_hdr_ctrl_create(isp_dev);
#endif

#if defined(ISP_RGBIR_V1) || defined(ISP_RGBIR_V1_1) || \
	defined(ISP_RGBIR_V2) || defined(ISP_RGBIR_V2_1)
	visp_rgbir_ctrl_create(isp_dev);
#endif

#if defined(ISP_DG_V1)
	visp_dg_ctrl_create(isp_dev);
#endif

#if defined(ISP_CMPD_20BIT) || defined(ISP_CMPD_24BIT)
	visp_cpd_ctrl_create(isp_dev);
#endif

#if defined(ISP_DPF_V1)
	visp_dpf_ctrl_create(isp_dev);
#endif

#if defined(ISP_CNR_V1) || defined(ISP_CNR_V2_1) || defined(ISP_CNR_V2_2)
	visp_cnr_ctrl_create(isp_dev);
#endif

#if defined(ISP_GWDR_V1)
	visp_gwdr_ctrl_create(isp_dev);
#endif

#if defined(ISP_YNR_V1)
	visp_ynr_ctrl_create(isp_dev);
#endif

#if defined(ISP_LUT3D_V1)
	visp_lut3d_ctrl_create(isp_dev);
#endif

	visp_sensor_ctrl_create(isp_dev);

	isp_dev->sd.ctrl_handler = &isp_dev->ctrl_handler;

	return 0;
}

int visp_ctrl_destroy(struct visp_dev *isp_dev)
{
	v4l2_ctrl_handler_free(&isp_dev->ctrl_handler);

	return 0;
}
