ifeq ($(VVCAM_PATH),)
	VVCAM_PATH:=$(shell cd $(shell pwd)/../; pwd)
	export VVCAM_PATH
endif

include $(PWD)/Param.mk
TARGET := vvcam_isp

obj-m := $(TARGET).o

$(TARGET)-objs += vvcam_isp_driver.o
$(TARGET)-objs += vvcam_isp_event.o
$(TARGET)-objs += vvcam_isp_procfs.o
$(TARGET)-objs += isp_ctrl/vvcam_isp_ctrl.o
$(TARGET)-objs += vvcam_isp_app.o
$(TARGET)-objs += mailbox/mbox_cmd.o
$(TARGET)-objs += mailbox/mbox_api.o
$(TARGET)-objs += mailbox/mbox_fifo.o


$(TARGET)-objs += vsi_units/source/cam_device/cam_device_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_sensor_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_buffer_api.o

EXTRA_CFLAGS := -I$(PWD)/vvcam_isp/
EXTRA_CFLAGS += -I$(PWD)/common/
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/mailbox/
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/ae/aev2/
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/vsi_units/include/
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/vsi_units/include/cam_device/
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/include/
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/vsi_units/include_priv/cam_device/

USER_INC:=$(PWD)/vvcam_isp/isp_ctrl/vvcam_isp_ctrl.h
USER_INC += $(PWD)/vvcam_isp/mailbox/*.h

ifeq ($(ISP_AE_V3), 1)
$(TARGET)-objs += isp_ctrl/ae/ae_v3/vvcam_isp_ae.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/ae/ae_v3/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/ae/ae_v3/*.h
endif

ifeq ($(ISP_AWB_V3), 1)
$(TARGET)-objs += isp_ctrl/awb/awb_v3/vvcam_isp_awb.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/awb/awb_v3/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/awb/awb_v3/*.h
endif

ifeq ($(ISP_GC_V2), 1)
$(TARGET)-objs += isp_ctrl/gc/gc_v2/vvcam_isp_gc.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/gc/gc_v2/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/gc/gc_v2/*.h
endif

ifeq ($(ISP_2DNR_V5_2), 1)
$(TARGET)-objs += isp_ctrl/2dnr/2dnr_v5_2/vvcam_isp_2dnr.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/2dnr/2dnr_v5_2/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/2dnr/2dnr_v5_2/*.h
endif

ifeq ($(ISP_EE_V2), 1)
$(TARGET)-objs += isp_ctrl/ee/ee_v2/vvcam_isp_ee.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/ee/ee_v2/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/ee/ee_v2/*.h
endif

ifeq ($(ISP_CPROC), 1)
$(TARGET)-objs += isp_ctrl/cproc/cproc/vvcam_isp_cproc.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/cproc/cproc/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/cproc/cproc/*.h
endif

ifeq ($(ISP_WDR_V5_2), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5_2/vvcam_isp_wdr.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/wdr/wdr_v5_2/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/wdr/wdr_v5_2/*.h
endif

ifeq ($(ISP_BLS), 1)
$(TARGET)-objs += isp_ctrl/bls/bls/vvcam_isp_bls.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/bls/bls/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/bls/bls/*.h
endif

ifeq ($(ISP_CCM), 1)
$(TARGET)-objs += isp_ctrl/ccm/ccm/vvcam_isp_ccm.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/ccm/ccm/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/ccm/ccm/*.h
endif

ifeq ($(ISP_DPCC), 1)
$(TARGET)-objs += isp_ctrl/dpcc/dpcc/vvcam_isp_dpcc.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/dpcc/dpcc/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/dpcc/dpcc/*.h
endif

ifeq ($(ISP_GE), 1)
$(TARGET)-objs += isp_ctrl/ge/ge/vvcam_isp_ge.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/ge/ge/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/ge/ge/*.h
endif

ifeq ($(ISP_LSC_V3), 1)
$(TARGET)-objs += isp_ctrl/lsc/lsc_v3/vvcam_isp_lsc.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/lsc/lsc_v3/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/lsc/lsc_v3/*.h
endif

ifeq ($(ISP_WB_V1_1), 1)
$(TARGET)-objs += isp_ctrl/wb/wb_v1_1/vvcam_isp_wb.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/wb/wb_v1_1/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/wb/wb_v1_1/*.h
endif

ifeq ($(ISP_3DNR_V2_1), 1)
$(TARGET)-objs += isp_ctrl/3dnr/3dnr_v2_1/vvcam_isp_3dnr.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/3dnr/3dnr_v2_1
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/3dnr/3dnr_v2_1/*.h
endif

ifeq ($(ISP_DMSC_V2), 1)
$(TARGET)-objs += isp_ctrl/dmsc/dmsc_v2/vvcam_isp_dmsc.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/dmsc/dmsc_v2/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/dmsc/dmsc_v2/*.h
endif

ifeq ($(ISP_EXP_V2), 1)
$(TARGET)-objs += isp_ctrl/exp/exp_v2/vvcam_isp_exp.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/exp/exp_v2/
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/exp/exp_v2/*.h
endif

ifeq ($(ISP_GTM_V1), 1)
$(TARGET)-objs += isp_ctrl/gtm/gtm_v1/vvcam_isp_gtm.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/gtm/gtm_v1
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/gtm/gtm_v1/*.h
endif

ifeq ($(ISP_HDR_V3_1), 1)
$(TARGET)-objs += isp_ctrl/hdr/hdr_v3_1/vvcam_isp_hdr.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/hdr/hdr_v3_1
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/hdr/hdr_v3_1/*.h
endif

ifeq ($(ISP_RGBIR_V2_1), 1)
$(TARGET)-objs += isp_ctrl/rgbir/rgbir_v2_1/vvcam_isp_rgbir.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/rgbir/rgbir_v2_1
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/rgbir/rgbir_v2_1/*.h
endif

ifeq ($(ISP_DG_V1), 1)
$(TARGET)-objs += isp_ctrl/dg/dg_v1/vvcam_isp_dg.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/dg/dg_v1
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/dg/dg_v1/*.h
endif

ifeq ($(ISP_CPD), 1)
$(TARGET)-objs += isp_ctrl/cpd/cpd/vvcam_isp_cpd.o
EXTRA_CFLAGS += -I$(PWD)/vvcam_isp/isp_ctrl/cpd/cpd
USER_INC += $(PWD)/vvcam_isp/isp_ctrl/cpd/cpd/*.h
endif
