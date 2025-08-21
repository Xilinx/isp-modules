include $(VISP_PATH)/isp_version/ISP8200_V2311.mk
TARGET := visp
#  $(CC) $(LDFLAGS) -o $@ $^

obj-m := $(TARGET).o
#  $(CC) $(CFLAGS) -c $< -o $@

$(TARGET)-objs-y += -Wconversion -Wsign-conversion -Wextra -Wall

$(TARGET)-objs += visp_driver.o
$(TARGET)-objs += visp_event.o
$(TARGET)-objs += visp_procfs.o
$(TARGET)-objs += isp_ctrl/visp_ctrl.o
$(TARGET)-objs += visp_app.o
$(TARGET)-objs += iba.o


$(TARGET)-objs += vsi_units/source/cam_device/cam_device_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_sensor_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_buffer_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_ae_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_awb_api.o

ccflags-y := -I$(PWD)/visp/
ccflags-y += -I$(PWD)/common/
ccflags-y += -I$(PWD)/visp_mbox/mailbox/
ccflags-y += -I$(PWD)/visp_mbox/
ccflags-y += -I$(PWD)/visp/isp_ctrl/
ccflags-y += -I$(PWD)/visp/isp_ctrl/ae/aev2/
ccflags-y += -I$(PWD)/visp/vsi_units/include/
ccflags-y += -I$(PWD)/visp/vsi_units/include/cam_device/
ccflags-y += -I$(PWD)/visp/include/
ccflags-y += -I$(PWD)/visp/vsi_units/include_priv/cam_device/
ccflags-y += -I$(PWD)/visp/vsi_units/source/cam_device/
ccflags-y += -I$(PWD)/visp/vsi_units/isi/include/
ccflags-y += -I$(PWD)/visp/vsi_units/
ccflags-y += -I$(PWD)/visp/vsi_units/isi/


USER_INC:=$(PWD)/visp/isp_ctrl/visp_ctrl.h
USER_INC += $(PWD)/visp_mbox/mailbox/*.h



ifeq ($(ISP_AE_V3), 1)
$(TARGET)-objs += isp_ctrl/ae/ae_v3/visp_ae.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ae/ae_v3/
USER_INC += $(PWD)/visp/isp_ctrl/ae/ae_v3/*.h
endif

ifeq ($(ISP_AE_V4), 1)
$(TARGET)-objs += isp_ctrl/ae/ae_v4/visp_ae.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ae/ae_v4/
USER_INC += $(PWD)/visp/isp_ctrl/ae/ae_v4/*.h
endif

ifeq ($(ISP_AE_V4_1), 1)
$(TARGET)-objs += isp_ctrl/ae/ae_v4_1/visp_ae.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ae/ae_v4_1/
USER_INC += $(PWD)/visp/isp_ctrl/ae/ae_v4_1/*.h
endif

ifeq ($(ISP_AWB_V3), 1)
$(TARGET)-objs += isp_ctrl/awb/awb_v3/visp_awb.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/awb/awb_v3/
USER_INC += $(PWD)/visp/isp_ctrl/awb/awb_v3/*.h
endif

ifeq ($(ISP_AWB_V4), 1)
$(TARGET)-objs += isp_ctrl/awb/awb_v4/visp_awb.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/awb/awb_v4/
USER_INC += $(PWD)/visp/isp_ctrl/awb/awb_v4/*.h
endif

ifeq ($(ISP_AWB_V4_1), 1)
$(TARGET)-objs += isp_ctrl/awb/awb_v4_1/visp_awb.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/awb/awb_v4_1/
USER_INC += $(PWD)/visp/isp_ctrl/awb/awb_v4_1/*.h
endif

ifeq ($(ISP_GC_V1), 1)
$(TARGET)-objs += isp_ctrl/gc/gc_v1/visp_gc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/gc/gc_v1/
USER_INC += $(PWD)/visp/isp_ctrl/gc/gc_v1/*.h
endif

ifeq ($(ISP_GC_V2), 1)
$(TARGET)-objs += isp_ctrl/gc/gc_v2/visp_gc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/gc/gc_v2/
USER_INC += $(PWD)/visp/isp_ctrl/gc/gc_v2/*.h
endif

ifeq ($(ISP_2DNR_V3), 1)
$(TARGET)-objs += isp_ctrl/2dnr/2dnr_v3/visp_2dnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/2dnr/2dnr_v3/
USER_INC += $(PWD)/visp/isp_ctrl/2dnr/2dnr_v3/*.h
endif

ifeq ($(ISP_2DNR_V5_2), 1)
$(TARGET)-objs += isp_ctrl/2dnr/2dnr_v5_2/visp_2dnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/2dnr/2dnr_v5_2/
USER_INC += $(PWD)/visp/isp_ctrl/2dnr/2dnr_v5_2/*.h
endif

ifeq ($(ISP_2DNR_V6), 1)
$(TARGET)-objs += isp_ctrl/2dnr/2dnr_v6/visp_2dnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/2dnr/2dnr_v6/
USER_INC += $(PWD)/visp/isp_ctrl/2dnr/2dnr_v6/*.h
endif

ifeq ($(ISP_EE_V1), 1)
$(TARGET)-objs += isp_ctrl/ee/ee_v1/visp_ee.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ee/ee_v1/
USER_INC += $(PWD)/visp/isp_ctrl/ee/ee_v1/*.h
endif

ifeq ($(ISP_EE_V2), 1)
$(TARGET)-objs += isp_ctrl/ee/ee_v2/visp_ee.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ee/ee_v2/
USER_INC += $(PWD)/visp/isp_ctrl/ee/ee_v2/*.h
endif

ifeq ($(ISP_EE_V3), 1)
$(TARGET)-objs += isp_ctrl/ee/ee_v3/visp_ee.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ee/ee_v3/
USER_INC += $(PWD)/visp/isp_ctrl/ee/ee_v3/*.h
endif

ifeq ($(ISP_CPROC), 1)
$(TARGET)-objs += isp_ctrl/cproc/cproc/visp_cproc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/cproc/cproc/
USER_INC += $(PWD)/visp/isp_ctrl/cproc/cproc/*.h
endif

ifeq ($(ISP_WDR_V3), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v3/visp_wdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wdr/wdr_v3/
USER_INC += $(PWD)/visp/isp_ctrl/wdr/wdr_v3/*.h
endif

ifeq ($(ISP_WDR_V4), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v4/visp_wdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wdr/wdr_v4/
USER_INC += $(PWD)/visp/isp_ctrl/wdr/wdr_v4/*.h
endif

ifeq ($(ISP_WDR_V5), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5/visp_wdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wdr/wdr_v5/
USER_INC += $(PWD)/visp/isp_ctrl/wdr/wdr_v5/*.h
endif

ifeq ($(ISP_WDR_V5_1), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5_1/visp_wdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wdr/wdr_v5_1/
USER_INC += $(PWD)/visp/isp_ctrl/wdr/wdr_v5_1/*.h
endif

ifeq ($(ISP_WDR_V5_2), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5_2/visp_wdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wdr/wdr_v5_2/
USER_INC += $(PWD)/visp/isp_ctrl/wdr/wdr_v5_2/*.h
endif

ifeq ($(ISP_WDR_V5_2_1), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5_2_1/visp_wdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wdr/wdr_v5_2_1/
USER_INC += $(PWD)/visp/isp_ctrl/wdr/wdr_v5_2_1/*.h
endif

ifeq ($(ISP_WDR_V5_3), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5_3/visp_wdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wdr/wdr_v5_3/
USER_INC += $(PWD)/visp/isp_ctrl/wdr/wdr_v5_3/*.h
endif

ifeq ($(ISP_BLS), 1)
$(TARGET)-objs += isp_ctrl/bls/bls/visp_bls.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/bls/bls/
USER_INC += $(PWD)/visp/isp_ctrl/bls/bls/*.h
endif

ifeq ($(ISP_CCM), 1)
$(TARGET)-objs += isp_ctrl/ccm/ccm/visp_ccm.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ccm/ccm/
USER_INC += $(PWD)/visp/isp_ctrl/ccm/ccm/*.h
endif

ifeq ($(ISP_DPCC), 1)
$(TARGET)-objs += isp_ctrl/dpcc/dpcc/visp_dpcc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/dpcc/dpcc/
USER_INC += $(PWD)/visp/isp_ctrl/dpcc/dpcc/*.h
endif

ifeq ($(ISP_GE), 1)
$(TARGET)-objs += isp_ctrl/ge/ge/visp_ge.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ge/ge/
USER_INC += $(PWD)/visp/isp_ctrl/ge/ge/*.h
endif

ifeq ($(ISP_LSC_V1), 1)
$(TARGET)-objs += isp_ctrl/lsc/lsc_v1/visp_lsc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/lsc/lsc_v1/
endif

ifeq ($(ISP_LSC_V2), 1)
$(TARGET)-objs += isp_ctrl/lsc/lsc_v2/visp_lsc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/lsc/lsc_v2/
USER_INC += $(PWD)/visp/isp_ctrl/lsc/lsc_v2/*.h
endif

ifeq ($(ISP_LSC_V3), 1)
$(TARGET)-objs += isp_ctrl/lsc/lsc_v3/visp_lsc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/lsc/lsc_v3/
USER_INC += $(PWD)/visp/isp_ctrl/lsc/lsc_v3/*.h
endif

ifeq ($(ISP_LSC_V4), 1)
$(TARGET)-objs += isp_ctrl/lsc/lsc_v4/visp_lsc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/lsc/lsc_v4/
USER_INC += $(PWD)/visp/isp_ctrl/lsc/lsc_v4/*.h
endif

ifeq ($(ISP_WB_V1_1), 1)
$(TARGET)-objs += isp_ctrl/wb/wb_v1_1/visp_wb.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wb/wb_v1_1/
USER_INC += $(PWD)/visp/isp_ctrl/wb/wb_v1_1/*.h
endif

ifeq ($(ISP_WB_V1), 1)
$(TARGET)-objs += isp_ctrl/wb/wb_v1/visp_wb.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wb/wb_v1/
USER_INC += $(PWD)/visp/isp_ctrl/wb/wb_v1/*.h
endif

ifeq ($(ISP_3DNR_V1_2), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v1_2/visp_3dnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/3dnr/3dnr_v1_2
USER_INC += $(PWD)/visp/isp_ctrl/3dnr/3dnr_v1_2/*.h
endif

ifeq ($(ISP_3DNR_V2_1), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v2_1/visp_3dnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/3dnr/3dnr_v2_1
USER_INC += $(PWD)/visp/isp_ctrl/3dnr/3dnr_v2_1/*.h
endif

ifeq ($(ISP_3DNR_V2_4), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v2_4/visp_3dnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/3dnr/3dnr_v2_4
USER_INC += $(PWD)/visp/isp_ctrl/3dnr/3dnr_v2_4/*.h
endif

ifeq ($(ISP_3DNR_V3), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v3/visp_3dnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/3dnr/3dnr_v3
USER_INC += $(PWD)/visp/isp_ctrl/3dnr/3dnr_v3/*.h
endif

ifeq ($(ISP_3DNR_V3_1), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v3_1/visp_3dnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/3dnr/3dnr_v3_1
USER_INC += $(PWD)/visp/isp_ctrl/3dnr/3dnr_v3_1/*.h
endif

ifeq ($(ISP_3DNR_V4), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v4/visp_3dnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/3dnr/3dnr_v4
USER_INC += $(PWD)/visp/isp_ctrl/3dnr/3dnr_v4/*.h
endif

ifeq ($(ISP_AF_V3), 1)
$(TARGET)-objs += isp_ctrl/af/af_v3/visp_af.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/af/af_v3
USER_INC += $(PWD)/visp/isp_ctrl/af/af_v3/*.h
endif

ifeq ($(ISP_AF_V4), 1)
$(TARGET)-objs += isp_ctrl/af/af_v4/visp_af.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/af/af_v4
USER_INC += $(PWD)/visp/isp_ctrl/af/af_v4/*.h
endif

ifeq ($(ISP_AF_V4_1_1), 1)
$(TARGET)-objs += isp_ctrl/af/af_v4_1_1/visp_af.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/af/af_v4_1_1
USER_INC += $(PWD)/visp/isp_ctrl/af/af_v4_1_1/*.h
endif

ifeq ($(ISP_AF_V4_3), 1)
$(TARGET)-objs += isp_ctrl/af/af_v4_3/visp_af.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/af/af_v4_3
USER_INC += $(PWD)/visp/isp_ctrl/af/af_v4_3/*.h
endif

ifeq ($(ISP_PDAF_V1), 1)
$(TARGET)-objs += isp_ctrl/pdaf/pdaf_v1/visp_pdaf.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/pdaf/pdaf_v1
USER_INC += $(PWD)/visp/isp_ctrl/pdaf/pdaf_v1/*.h
endif

ifeq ($(ISP_AFM_V1), 1)
$(TARGET)-objs += isp_ctrl/afm/afm_v1/visp_afm.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/afm/afm_v1/
USER_INC += $(PWD)/visp/isp_ctrl/afm/afm_v1/*.h
endif

ifeq ($(ISP_AFM_V1_1), 1)
$(TARGET)-objs += isp_ctrl/afm/afm_v1_1/visp_afm.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/afm/afm_v1_1/
USER_INC += $(PWD)/visp/isp_ctrl/afm/afm_v1_1/*.h
endif

ifeq ($(ISP_AFM_V3), 1)
$(TARGET)-objs += isp_ctrl/afm/afm_v3/visp_afm.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/afm/afm_v3/
USER_INC += $(PWD)/visp/isp_ctrl/afm/afm_v3/*.h
endif

ifeq ($(ISP_DMSC_V1), 1)
$(TARGET)-objs += isp_ctrl/dmsc/dmsc_v1/visp_dmsc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/dmsc/dmsc_v1/
USER_INC += $(PWD)/visp/isp_ctrl/dmsc/dmsc_v1/*.h
endif

ifeq ($(ISP_DMSC_V2), 1)
$(TARGET)-objs += isp_ctrl/dmsc/dmsc_v2/visp_dmsc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/dmsc/dmsc_v2/
USER_INC += $(PWD)/visp/isp_ctrl/dmsc/dmsc_v2/*.h
endif

ifeq ($(ISP_DMSC_V3), 1)
$(TARGET)-objs += isp_ctrl/dmsc/dmsc_v3/visp_dmsc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/dmsc/dmsc_v3/
USER_INC += $(PWD)/visp/isp_ctrl/dmsc/dmsc_v3/*.h
endif

ifeq ($(ISP_EXP_V2), 1)
$(TARGET)-objs += isp_ctrl/exp/exp_v2/visp_exp.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/exp/exp_v2/
USER_INC += $(PWD)/visp/isp_ctrl/exp/exp_v2/*.h
endif

ifeq ($(ISP_EXP_V3), 1)
$(TARGET)-objs += isp_ctrl/exp/exp_v3/visp_exp_v3.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/exp/exp_v3/
USER_INC += $(PWD)/visp/isp_ctrl/exp/exp_v3/*.h
endif

ifeq ($(ISP_GTM_V1), 1)
$(TARGET)-objs += isp_ctrl/gtm/gtm_v1/visp_gtm.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/gtm/gtm_v1
USER_INC += $(PWD)/visp/isp_ctrl/gtm/gtm_v1/*.h
endif

ifeq ($(ISP_HDR_V1_3), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v1_3/visp_hdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/hdr/hdr_v1_3
USER_INC += $(PWD)/visp/isp_ctrl/hdr/hdr_v1_3/*.h
endif

ifeq ($(ISP_HDR_V2), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v2/visp_hdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/hdr/hdr_v2
USER_INC += $(PWD)/visp/isp_ctrl/hdr/hdr_v2/*.h
endif

ifeq ($(ISP_HDR_V2_1), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v2_1/visp_hdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/hdr/hdr_v2_1
USER_INC += $(PWD)/visp/isp_ctrl/hdr/hdr_v2_1/*.h
endif

ifeq ($(ISP_HDR_V2_1_2DOL), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v2_1_2dol/visp_hdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/hdr/hdr_v2_1_2dol
USER_INC += $(PWD)/visp/isp_ctrl/hdr/hdr_v2_1_2dol/*.h
endif

ifeq ($(ISP_HDR_V3), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v3/visp_hdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/hdr/hdr_v3
USER_INC += $(PWD)/visp/isp_ctrl/hdr/hdr_v3/*.h
endif

ifeq ($(ISP_HDR_V3_2DOL), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v3_2dol/visp_hdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/hdr/hdr_v3_2dol
USER_INC += $(PWD)/visp/isp_ctrl/hdr/hdr_v3_2dol/*.h
endif

ifeq ($(ISP_HDR_V3_1), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v3_1/visp_hdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/hdr/hdr_v3_1
USER_INC += $(PWD)/visp/isp_ctrl/hdr/hdr_v3_1/*.h
endif

ifeq ($(ISP_RGBIR_V1), 1)
$(TARGET)-objs += isp_ctrl/rgbir/rgbir_v1/visp_rgbir.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/rgbir/rgbir_v1
USER_INC += $(PWD)/visp/isp_ctrl/rgbir/rgbir_v1/*.h
endif

ifeq ($(ISP_RGBIR_V1_1), 1)
$(TARGET)-objs += isp_ctrl/rgbir/rgbir_v1_1/visp_rgbir.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/rgbir/rgbir_v1_1
USER_INC += $(PWD)/visp/isp_ctrl/rgbir/rgbir_v1_1/*.h
endif

ifeq ($(ISP_RGBIR_V2), 1)
$(TARGET)-objs += isp_ctrl/rgbir/rgbir_v2/visp_rgbir.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/rgbir/rgbir_v2
USER_INC += $(PWD)/visp/isp_ctrl/rgbir/rgbir_v2/*.h
endif

ifeq ($(ISP_RGBIR_V2_1), 1)
$(TARGET)-objs += isp_ctrl/rgbir/rgbir_v2_1/visp_rgbir.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/rgbir/rgbir_v2_1
USER_INC += $(PWD)/visp/isp_ctrl/rgbir/rgbir_v2_1/*.h
endif

ifeq ($(ISP_DG_V1), 1)
$(TARGET)-objs += isp_ctrl/dg/dg_v1/visp_dg.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/dg/dg_v1
USER_INC += $(PWD)/visp/isp_ctrl/dg/dg_v1/*.h
endif

ifeq ($(ISP_CMPD_20BIT), 1)
$(TARGET)-objs += isp_ctrl/cpd/cpd_v1/visp_cpd.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/cpd/cpd_v1
USER_INC += $(PWD)/visp/isp_ctrl/cpd/cpd_v1/*.h
endif

ifeq ($(ISP_CMPD_24BIT), 1)
$(TARGET)-objs += isp_ctrl/cpd/cpd_v1_1/visp_cpd.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/cpd/cpd_v1_1
USER_INC += $(PWD)/visp/isp_ctrl/cpd/cpd_v1_1/*.h
endif

ifeq ($(ISP_DPF_V1), 1)
$(TARGET)-objs += isp_ctrl/dpf/dpf_v1/visp_dpf.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/dpf/dpf_v1
USER_INC += $(PWD)/visp/isp_ctrl/dpf/dpf_v1/*.h
endif

ifeq ($(ISP_CNR_V1), 1)
$(TARGET)-objs += isp_ctrl/cnr/cnr_v1/visp_cnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/cnr/cnr_v1
USER_INC += $(PWD)/visp/isp_ctrl/cnr/cnr_v1/*.h
endif

ifeq ($(ISP_CNR_V2_1), 1)
$(TARGET)-objs += isp_ctrl/cnr/cnr_v2_1/visp_cnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/cnr/cnr_v2_1
USER_INC += $(PWD)/visp/isp_ctrl/cnr/cnr_v2_1/*.h
endif

ifeq ($(ISP_CNR_V2_2), 1)
$(TARGET)-objs += isp_ctrl/cnr/cnr_v2_2/visp_cnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/cnr/cnr_v2_2
USER_INC += $(PWD)/visp/isp_ctrl/cnr/cnr_v2_2/*.h
endif

ifeq ($(ISP_GWDR_V1), 1)
$(TARGET)-objs += isp_ctrl/gwdr/gwdr_v1/visp_gwdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/gwdr/gwdr_v1
USER_INC += $(PWD)/visp/isp_ctrl/gwdr/gwdr_v1/*.h
endif

ifeq ($(ISP_YNR_V1), 1)
$(TARGET)-objs += isp_ctrl/ynr/ynr_v1/visp_ynr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ynr/ynr_v1
USER_INC += $(PWD)/visp/isp_ctrl/ynr/ynr_v1/*.h
endif

ifeq ($(ISP_LUT3D_V1), 1)
$(TARGET)-objs += isp_ctrl/lut3d/lut3d_v1/visp_lut3d.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/lut3d/lut3d_v1
USER_INC += $(PWD)/visp/isp_ctrl/lut3d/lut3d_v1/*.h
endif

$(TARGET)-objs += isp_ctrl/sensor/visp_sensor.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/sensor
USER_INC += $(PWD)/visp/isp_ctrl/sensor/*.h

EXTRA_CFLAGS += -DDEBUG -O2 

#EXTRA_CFLAGS += -DDEBUG -O2 -g3 -fno-inline



PWD := $(shell pwd)

