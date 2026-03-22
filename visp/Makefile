include $(VISP_PATH)/isp_version/ISP8200_V2311.mk
TARGET := visp

obj-m := $(TARGET).o

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

ccflags-y += -I$(PWD)/visp/
ccflags-y += -I$(PWD)/common/
ccflags-y += -I$(PWD)/visp_mbox/mailbox/
ccflags-y += -I$(PWD)/visp_mbox/
ccflags-y += -I$(PWD)/visp/isp_ctrl/
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

ifeq ($(ISP_GC_V2), 1)
$(TARGET)-objs += isp_ctrl/gc/gc_v2/visp_gc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/gc/gc_v2/
USER_INC += $(PWD)/visp/isp_ctrl/gc/gc_v2/*.h
endif

ifeq ($(ISP_2DNR_V5_2), 1)
$(TARGET)-objs += isp_ctrl/2dnr/2dnr_v5_2/visp_2dnr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/2dnr/2dnr_v5_2/
USER_INC += $(PWD)/visp/isp_ctrl/2dnr/2dnr_v5_2/*.h
endif

ifeq ($(ISP_EE_V2), 1)
$(TARGET)-objs += isp_ctrl/ee/ee_v2/visp_ee.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ee/ee_v2/
USER_INC += $(PWD)/visp/isp_ctrl/ee/ee_v2/*.h
endif

ifeq ($(ISP_CPROC), 1)
$(TARGET)-objs += isp_ctrl/cproc/cproc/visp_cproc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/cproc/cproc/
USER_INC += $(PWD)/visp/isp_ctrl/cproc/cproc/*.h
endif

ifeq ($(ISP_WDR_V5_2), 1)
$(TARGET)-objs += isp_ctrl/wdr/wdr_v5_2/visp_wdr.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wdr/wdr_v5_2/
USER_INC += $(PWD)/visp/isp_ctrl/wdr/wdr_v5_2/*.h
endif

ifeq ($(ISP_BLS), 1)
$(TARGET)-objs += isp_ctrl/bls/bls/visp_bls.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/bls/bls/
USER_INC += $(PWD)/visp/isp_ctrl/bls/bls/*.h
endif

ifeq ($(ISP_CCM_V1_1), 1)
$(TARGET)-objs += isp_ctrl/ccm/ccm_v1_1/visp_ccm.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/ccm/ccm_v1_1/
USER_INC += $(PWD)/visp/isp_ctrl/ccm/ccm_v1_1/*.h
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

ifeq ($(ISP_LSC_V3), 1)
$(TARGET)-objs += isp_ctrl/lsc/lsc_v3/visp_lsc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/lsc/lsc_v3/
USER_INC += $(PWD)/visp/isp_ctrl/lsc/lsc_v3/*.h
endif

ifeq ($(ISP_WB_V1_1), 1)
$(TARGET)-objs += isp_ctrl/wb/wb_v1_1/visp_wb.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/wb/wb_v1_1/
USER_INC += $(PWD)/visp/isp_ctrl/wb/wb_v1_1/*.h
endif

ifeq ($(ISP_DMSC_V2), 1)
$(TARGET)-objs += isp_ctrl/dmsc/dmsc_v2/visp_dmsc.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/dmsc/dmsc_v2/
USER_INC += $(PWD)/visp/isp_ctrl/dmsc/dmsc_v2/*.h
endif


ifeq ($(ISP_EXP_V2), 1)
$(TARGET)-objs += isp_ctrl/exp/exp_v2/visp_exp.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/exp/exp_v2/
USER_INC += $(PWD)/visp/isp_ctrl/exp/exp_v2/*.h
endif

ifeq ($(ISP_GTM_V1), 1)
$(TARGET)-objs += isp_ctrl/gtm/gtm_v1/visp_gtm.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/gtm/gtm_v1
USER_INC += $(PWD)/visp/isp_ctrl/gtm/gtm_v1/*.h
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

ifeq ($(ISP_CPD_V1_1), 1)
$(TARGET)-objs += isp_ctrl/cpd/cpd_v1_1/visp_cpd.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/cpd/cpd_v1_1
USER_INC += $(PWD)/visp/isp_ctrl/cpd/cpd_v1_1/*.h
endif

ifeq ($(ISP_HIST64), 1)
$(TARGET)-objs += isp_ctrl/hist/hist64/visp_hist64.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/hist/hist64/
USER_INC += $(PWD)/visp/isp_ctrl/hist/hist64/*.h
endif

ifeq ($(ISP_HIST256), 1)
$(TARGET)-objs += isp_ctrl/hist/hist256/visp_hist256.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/hist/hist256/
USER_INC += $(PWD)/visp/isp_ctrl/hist/hist256/*.h
endif

$(TARGET)-objs += isp_ctrl/sensor/visp_sensor.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/sensor
USER_INC += $(PWD)/visp/isp_ctrl/sensor/*.h

$(TARGET)-objs += isp_ctrl/base/visp_base.o
ccflags-y += -I$(PWD)/visp/isp_ctrl/base
USER_INC += $(PWD)/visp/isp_ctrl/base/*.h

EXTRA_CFLAGS += -DDEBUG -O2

PWD := $(shell pwd)

