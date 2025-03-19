include $(VISP_PATH)/isp_version/ISP8200_V2311.mk
TARGET := visp_lilo
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
$(TARGET)-objs += oba.o
#$(TARGET)-objs += mailbox/mbox_cmd.o
#$(TARGET)-objs += mailbox/mbox_api.o
#$(TARGET)-objs += mailbox/mbox_fifo.o


$(TARGET)-objs += vsi_units/source/cam_device/cam_device_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_sensor_api.o
#$(TARGET)-objs += user_lib/source/media_isp_device.o 
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_buffer_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_ae_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_awb_api.o
#$(TARGET)-objs += mailbox/mbox_cmd.o

EXTRA_CFLAGS := -I$(PWD)/visp_lilo/
EXTRA_CFLAGS += -I$(PWD)/common/
#EXTRA_CFLAGS += -I$(PWD)/mbox/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo_mbox/mailbox/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo_mbox/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/isp_ctrl/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/isp_ctrl/ae/aev2/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/vsi_units/include/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/vsi_units/include/cam_device/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/include/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/vsi_units/include_priv/cam_device/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/vsi_units/source/cam_device/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/vsi_units/isi/include/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/vsi_units/
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/vsi_units/isi/


USER_INC:=$(PWD)/visp_lilo/isp_ctrl/visp_ctrl.h
#USER_INC+=$(PWD)/isp_ctrl/visp_app.h
#USER_INC+=$(PWD)/user_lib/source/vsi_units/include/cam_device/*h
USER_INC += $(PWD)/visp_lilo_mbox/mailbox/*.h



ifeq ($(ISP_AE_V3), 1)
$(TARGET)-objs += isp_ctrl/ae/ae_v3/visp_ae.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/ae/ae_v3/
USER_INC += $(PWD)/isp_ctrl/ae/ae_v3/*.h
endif

ifeq ($(ISP_AE_V4), 1)
$(TARGET)-objs += isp_ctrl/ae/ae_v4/visp_ae.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/ae/ae_v4/
USER_INC += $(PWD)/isp_ctrl/ae/ae_v4/*.h
endif

ifeq ($(ISP_AE_V4_1), 1)
$(TARGET)-objs += isp_ctrl/ae/ae_v4_1/visp_ae.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/ae/ae_v4_1/
USER_INC += $(PWD)/isp_ctrl/ae/ae_v4_1/*.h
endif

ifeq ($(ISP_AWB_V3), 1)
$(TARGET)-objs += isp_ctrl/awb/awb_v3/visp_awb.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/awb/awb_v3/
USER_INC += $(PWD)/isp_ctrl/awb/awb_v3/*.h
endif

ifeq ($(ISP_AWB_V4), 1)
$(TARGET)-objs += isp_ctrl/awb/awb_v4/visp_awb.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/awb/awb_v4/
USER_INC += $(PWD)/isp_ctrl/awb/awb_v4/*.h
endif

ifeq ($(ISP_AWB_V4_1), 1)
$(TARGET)-objs += isp_ctrl/awb/awb_v4_1/visp_awb.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/awb/awb_v4_1/
USER_INC += $(PWD)/isp_ctrl/awb/awb_v4_1/*.h
endif

ifeq ($(ISP_GC_V1), 1)
$(TARGET)-objs += isp_ctrl/gc/gc_v1/visp_gc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/gc/gc_v1/
USER_INC += $(PWD)/isp_ctrl/gc/gc_v1/*.h
endif

ifeq ($(ISP_GC_V2), 1)
$(TARGET)-objs += isp_ctrl/gc/gc_v2/visp_gc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/gc/gc_v2/
USER_INC += $(PWD)/isp_ctrl/gc/gc_v2/*.h
endif

ifeq ($(ISP_2DNR_V3), 1)
$(TARGET)-objs += isp_ctrl/2dnr/2dnr_v3/visp_2dnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/2dnr/2dnr_v3/
USER_INC += $(PWD)/isp_ctrl/2dnr/2dnr_v3/*.h
endif

ifeq ($(ISP_2DNR_V5_2), 1)
$(TARGET)-objs += isp_ctrl/2dnr/2dnr_v5_2/visp_2dnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/2dnr/2dnr_v5_2/
USER_INC += $(PWD)/isp_ctrl/2dnr/2dnr_v5_2/*.h
endif

ifeq ($(ISP_2DNR_V6), 1)
$(TARGET)-objs += isp_ctrl/2dnr/2dnr_v6/visp_2dnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/2dnr/2dnr_v6/
USER_INC += $(PWD)/isp_ctrl/2dnr/2dnr_v6/*.h
endif

ifeq ($(ISP_EE_V1), 1)
$(TARGET)-objs += isp_ctrl/ee/ee_v1/visp_ee.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/ee/ee_v1/
USER_INC += $(PWD)/isp_ctrl/ee/ee_v1/*.h
endif

ifeq ($(ISP_EE_V2), 1)
$(TARGET)-objs += isp_ctrl/ee/ee_v2/visp_ee.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/ee/ee_v2/
USER_INC += $(PWD)/isp_ctrl/ee/ee_v2/*.h
endif

ifeq ($(ISP_EE_V3), 1)
$(TARGET)-objs += isp_ctrl/ee/ee_v3/visp_ee.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/ee/ee_v3/
USER_INC += $(PWD)/isp_ctrl/ee/ee_v3/*.h
endif

ifeq ($(ISP_CPROC), 1)
$(TARGET)-objs += isp_ctrl/cproc/cproc/visp_cproc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/cproc/cproc/
USER_INC += $(PWD)/isp_ctrl/cproc/cproc/*.h
endif

ifeq ($(ISP_WDR_V3), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v3/visp_wdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/wdr/wdr_v3/
USER_INC += $(PWD)/isp_ctrl/wdr/wdr_v3/*.h
endif

ifeq ($(ISP_WDR_V4), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v4/visp_wdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/wdr/wdr_v4/
USER_INC += $(PWD)/isp_ctrl/wdr/wdr_v4/*.h
endif

ifeq ($(ISP_WDR_V5), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5/visp_wdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/wdr/wdr_v5/
USER_INC += $(PWD)/isp_ctrl/wdr/wdr_v5/*.h
endif

ifeq ($(ISP_WDR_V5_1), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5_1/visp_wdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/wdr/wdr_v5_1/
USER_INC += $(PWD)/isp_ctrl/wdr/wdr_v5_1/*.h
endif

ifeq ($(ISP_WDR_V5_2), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5_2/visp_wdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/wdr/wdr_v5_2/
USER_INC += $(PWD)/isp_ctrl/wdr/wdr_v5_2/*.h
endif

ifeq ($(ISP_WDR_V5_2_1), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5_2_1/visp_wdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/wdr/wdr_v5_2_1/
USER_INC += $(PWD)/isp_ctrl/wdr/wdr_v5_2_1/*.h
endif

ifeq ($(ISP_WDR_V5_3), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5_3/visp_wdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/wdr/wdr_v5_3/
USER_INC += $(PWD)/isp_ctrl/wdr/wdr_v5_3/*.h
endif

ifeq ($(ISP_BLS), 1)
$(TARGET)-objs += isp_ctrl/bls/bls/visp_bls.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/bls/bls/
USER_INC += $(PWD)/isp_ctrl/bls/bls/*.h
endif

ifeq ($(ISP_CCM), 1)
$(TARGET)-objs += isp_ctrl/ccm/ccm/visp_ccm.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/ccm/ccm/
USER_INC += $(PWD)/isp_ctrl/ccm/ccm/*.h
endif

ifeq ($(ISP_DPCC), 1)
$(TARGET)-objs += isp_ctrl/dpcc/dpcc/visp_dpcc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/dpcc/dpcc/
USER_INC += $(PWD)/isp_ctrl/dpcc/dpcc/*.h
endif

ifeq ($(ISP_GE), 1)
$(TARGET)-objs += isp_ctrl/ge/ge/visp_ge.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/ge/ge/
USER_INC += $(PWD)/isp_ctrl/ge/ge/*.h
endif

ifeq ($(ISP_LSC_V1), 1)
$(TARGET)-objs += isp_ctrl/lsc/lsc_v1/visp_lsc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/lsc/lsc_v1/
endif

ifeq ($(ISP_LSC_V2), 1)
$(TARGET)-objs += isp_ctrl/lsc/lsc_v2/visp_lsc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/lsc/lsc_v2/
USER_INC += $(PWD)/isp_ctrl/lsc/lsc_v2/*.h
endif

ifeq ($(ISP_LSC_V3), 1)
$(TARGET)-objs += isp_ctrl/lsc/lsc_v3/visp_lsc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/lsc/lsc_v3/
USER_INC += $(PWD)/isp_ctrl/lsc/lsc_v3/*.h
endif

ifeq ($(ISP_LSC_V4), 1)
$(TARGET)-objs += isp_ctrl/lsc/lsc_v4/visp_lsc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/lsc/lsc_v4/
USER_INC += $(PWD)/isp_ctrl/lsc/lsc_v4/*.h
endif

ifeq ($(ISP_WB_V1_1), 1)
$(TARGET)-objs += isp_ctrl/wb/wb_v1_1/visp_wb.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/wb/wb_v1_1/
USER_INC += $(PWD)/isp_ctrl/wb/wb_v1_1/*.h
endif

ifeq ($(ISP_WB_V1), 1)
$(TARGET)-objs += isp_ctrl/wb/wb_v1/visp_wb.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/wb/wb_v1/
USER_INC += $(PWD)/isp_ctrl/wb/wb_v1/*.h
endif

ifeq ($(ISP_3DNR_V1_2), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v1_2/visp_3dnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/3dnr/3dnr_v1_2
USER_INC += $(PWD)/isp_ctrl/3dnr/3dnr_v1_2/*.h
endif

ifeq ($(ISP_3DNR_V2_1), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v2_1/visp_3dnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/3dnr/3dnr_v2_1
USER_INC += $(PWD)/isp_ctrl/3dnr/3dnr_v2_1/*.h
endif

ifeq ($(ISP_3DNR_V2_4), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v2_4/visp_3dnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/3dnr/3dnr_v2_4
USER_INC += $(PWD)/isp_ctrl/3dnr/3dnr_v2_4/*.h
endif

ifeq ($(ISP_3DNR_V3), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v3/visp_3dnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/3dnr/3dnr_v3
USER_INC += $(PWD)/isp_ctrl/3dnr/3dnr_v3/*.h
endif

ifeq ($(ISP_3DNR_V3_1), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v3_1/visp_3dnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/3dnr/3dnr_v3_1
USER_INC += $(PWD)/isp_ctrl/3dnr/3dnr_v3_1/*.h
endif

ifeq ($(ISP_3DNR_V4), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v4/visp_3dnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/3dnr/3dnr_v4
USER_INC += $(PWD)/isp_ctrl/3dnr/3dnr_v4/*.h
endif

ifeq ($(ISP_AF_V3), 1)
$(TARGET)-objs += isp_ctrl/af/af_v3/visp_af.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/af/af_v3
USER_INC += $(PWD)/isp_ctrl/af/af_v3/*.h
endif

ifeq ($(ISP_AF_V4), 1)
$(TARGET)-objs += isp_ctrl/af/af_v4/visp_af.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/af/af_v4
USER_INC += $(PWD)/isp_ctrl/af/af_v4/*.h
endif

ifeq ($(ISP_AF_V4_1_1), 1)
$(TARGET)-objs += isp_ctrl/af/af_v4_1_1/visp_af.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/af/af_v4_1_1
USER_INC += $(PWD)/isp_ctrl/af/af_v4_1_1/*.h
endif

ifeq ($(ISP_AF_V4_3), 1)
$(TARGET)-objs += isp_ctrl/af/af_v4_3/visp_af.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/af/af_v4_3
USER_INC += $(PWD)/isp_ctrl/af/af_v4_3/*.h
endif

ifeq ($(ISP_PDAF_V1), 1)
$(TARGET)-objs += isp_ctrl/pdaf/pdaf_v1/visp_pdaf.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/pdaf/pdaf_v1
USER_INC += $(PWD)/isp_ctrl/pdaf/pdaf_v1/*.h
endif

ifeq ($(ISP_AFM_V1), 1)
$(TARGET)-objs += isp_ctrl/afm/afm_v1/visp_afm.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/afm/afm_v1/
USER_INC += $(PWD)/isp_ctrl/afm/afm_v1/*.h
endif

ifeq ($(ISP_AFM_V1_1), 1)
$(TARGET)-objs += isp_ctrl/afm/afm_v1_1/visp_afm.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/afm/afm_v1_1/
USER_INC += $(PWD)/isp_ctrl/afm/afm_v1_1/*.h
endif

ifeq ($(ISP_AFM_V3), 1)
$(TARGET)-objs += isp_ctrl/afm/afm_v3/visp_afm.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/afm/afm_v3/
USER_INC += $(PWD)/isp_ctrl/afm/afm_v3/*.h
endif

ifeq ($(ISP_DMSC_V1), 1)
$(TARGET)-objs += isp_ctrl/dmsc/dmsc_v1/visp_dmsc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/dmsc/dmsc_v1/
USER_INC += $(PWD)/isp_ctrl/dmsc/dmsc_v1/*.h
endif

ifeq ($(ISP_DMSC_V2), 1)
$(TARGET)-objs += isp_ctrl/dmsc/dmsc_v2/visp_dmsc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/dmsc/dmsc_v2/
USER_INC += $(PWD)/isp_ctrl/dmsc/dmsc_v2/*.h
endif

ifeq ($(ISP_DMSC_V3), 1)
$(TARGET)-objs += isp_ctrl/dmsc/dmsc_v3/visp_dmsc.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/dmsc/dmsc_v3/
USER_INC += $(PWD)/isp_ctrl/dmsc/dmsc_v3/*.h
endif

ifeq ($(ISP_EXP_V2), 1)
$(TARGET)-objs += isp_ctrl/exp/exp_v2/visp_exp.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/exp/exp_v2/
USER_INC += $(PWD)/isp_ctrl/exp/exp_v2/*.h
endif

ifeq ($(ISP_EXP_V3), 1)
$(TARGET)-objs += isp_ctrl/exp/exp_v3/visp_exp_v3.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/exp/exp_v3/
USER_INC += $(PWD)/isp_ctrl/exp/exp_v3/*.h
endif

ifeq ($(ISP_GTM_V1), 1)
$(TARGET)-objs += isp_ctrl/gtm/gtm_v1/visp_gtm.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/gtm/gtm_v1
USER_INC += $(PWD)/isp_ctrl/gtm/gtm_v1/*.h
endif

ifeq ($(ISP_HDR_V1_3), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v1_3/visp_hdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/hdr/hdr_v1_3
USER_INC += $(PWD)/isp_ctrl/hdr/hdr_v1_3/*.h
endif

ifeq ($(ISP_HDR_V2), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v2/visp_hdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/hdr/hdr_v2
USER_INC += $(PWD)/isp_ctrl/hdr/hdr_v2/*.h
endif

ifeq ($(ISP_HDR_V2_1), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v2_1/visp_hdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/hdr/hdr_v2_1
USER_INC += $(PWD)/isp_ctrl/hdr/hdr_v2_1/*.h
endif

ifeq ($(ISP_HDR_V2_1_2DOL), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v2_1_2dol/visp_hdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/hdr/hdr_v2_1_2dol
USER_INC += $(PWD)/isp_ctrl/hdr/hdr_v2_1_2dol/*.h
endif

ifeq ($(ISP_HDR_V3), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v3/visp_hdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/hdr/hdr_v3
USER_INC += $(PWD)/isp_ctrl/hdr/hdr_v3/*.h
endif

ifeq ($(ISP_HDR_V3_2DOL), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v3_2dol/visp_hdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/hdr/hdr_v3_2dol
USER_INC += $(PWD)/isp_ctrl/hdr/hdr_v3_2dol/*.h
endif

ifeq ($(ISP_HDR_V3_1), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v3_1/visp_hdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/hdr/hdr_v3_1
USER_INC += $(PWD)/isp_ctrl/hdr/hdr_v3_1/*.h
endif

ifeq ($(ISP_RGBIR_V1), 1)
$(TARGET)-objs += isp_ctrl/rgbir/rgbir_v1/visp_rgbir.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/rgbir/rgbir_v1
USER_INC += $(PWD)/isp_ctrl/rgbir/rgbir_v1/*.h
endif

ifeq ($(ISP_RGBIR_V1_1), 1)
$(TARGET)-objs += isp_ctrl/rgbir/rgbir_v1_1/visp_rgbir.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/rgbir/rgbir_v1_1
USER_INC += $(PWD)/isp_ctrl/rgbir/rgbir_v1_1/*.h
endif

ifeq ($(ISP_RGBIR_V2), 1)
$(TARGET)-objs += isp_ctrl/rgbir/rgbir_v2/visp_rgbir.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/rgbir/rgbir_v2
USER_INC += $(PWD)/isp_ctrl/rgbir/rgbir_v2/*.h
endif

ifeq ($(ISP_RGBIR_V2_1), 1)
$(TARGET)-objs += isp_ctrl/rgbir/rgbir_v2_1/visp_rgbir.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/rgbir/rgbir_v2_1
USER_INC += $(PWD)/isp_ctrl/rgbir/rgbir_v2_1/*.h
endif

ifeq ($(ISP_DG_V1), 1)
$(TARGET)-objs += isp_ctrl/dg/dg_v1/visp_dg.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/dg/dg_v1
USER_INC += $(PWD)/isp_ctrl/dg/dg_v1/*.h
endif

ifeq ($(ISP_CMPD_20BIT), 1)
$(TARGET)-objs += isp_ctrl/cpd/cpd_v1/visp_cpd.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/cpd/cpd_v1
USER_INC += $(PWD)/isp_ctrl/cpd/cpd_v1/*.h
endif

ifeq ($(ISP_CMPD_24BIT), 1)
$(TARGET)-objs += isp_ctrl/cpd/cpd_v1_1/visp_cpd.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/cpd/cpd_v1_1
USER_INC += $(PWD)/isp_ctrl/cpd/cpd_v1_1/*.h
endif

ifeq ($(ISP_DPF_V1), 1)
$(TARGET)-objs += isp_ctrl/dpf/dpf_v1/visp_dpf.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/dpf/dpf_v1
USER_INC += $(PWD)/isp_ctrl/dpf/dpf_v1/*.h
endif

ifeq ($(ISP_CNR_V1), 1)
$(TARGET)-objs += isp_ctrl/cnr/cnr_v1/visp_cnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/cnr/cnr_v1
USER_INC += $(PWD)/isp_ctrl/cnr/cnr_v1/*.h
endif

ifeq ($(ISP_CNR_V2_1), 1)
$(TARGET)-objs += isp_ctrl/cnr/cnr_v2_1/visp_cnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/cnr/cnr_v2_1
USER_INC += $(PWD)/isp_ctrl/cnr/cnr_v2_1/*.h
endif

ifeq ($(ISP_CNR_V2_2), 1)
$(TARGET)-objs += isp_ctrl/cnr/cnr_v2_2/visp_cnr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/cnr/cnr_v2_2
USER_INC += $(PWD)/isp_ctrl/cnr/cnr_v2_2/*.h
endif

ifeq ($(ISP_GWDR_V1), 1)
$(TARGET)-objs += isp_ctrl/gwdr/gwdr_v1/visp_gwdr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/gwdr/gwdr_v1
USER_INC += $(PWD)/isp_ctrl/gwdr/gwdr_v1/*.h
endif

ifeq ($(ISP_YNR_V1), 1)
$(TARGET)-objs += isp_ctrl/ynr/ynr_v1/visp_ynr.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/ynr/ynr_v1
USER_INC += $(PWD)/isp_ctrl/ynr/ynr_v1/*.h
endif

ifeq ($(ISP_LUT3D_V1), 1)
$(TARGET)-objs += isp_ctrl/lut3d/lut3d_v1/visp_lut3d.o
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/lut3d/lut3d_v1
USER_INC += $(PWD)/isp_ctrl/lut3d/lut3d_v1/*.h
endif

$(TARGET)-objs += isp_ctrl/sensor/visp_sensor.o
EXTRA_CFLAGS += -I$(PWD)/visp_lilo/isp_ctrl/sensor
USER_INC += $(PWD)/visp_lilo/isp_ctrl/sensor/*.h

EXTRA_CFLAGS += -DDEBUG -O2 

#EXTRA_CFLAGS += -DDEBUG -O2 -g3 -fno-inline



PWD := $(shell pwd)

