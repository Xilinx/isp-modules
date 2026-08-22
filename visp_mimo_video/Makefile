ifeq ($(VISP_PATH),)
	VISP_PATH:=$(shell cd $(shell pwd)/../; pwd)
	export VISP_PATH
endif

include $(VISP_PATH)/isp_version/ISP8200_V2311.mk
TARGET := visp_mimo_video
obj-m := $(TARGET).o

#  $(CC) $(CFLAGS) -c $< -o $@
$(TARGET)-objs += visp_mimo_driver.o
$(TARGET)-objs += visp_mimo_driver_utils.o
$(TARGET)-objs += visp_video_event.o

ccflags-y += -I$(PWD)/common/
ccflags-y += -I$(PWD)/visp_mimo_video
ccflags-y += -I$(PWD)/visp_mimo/
ccflags-y += -I$(PWD)/common/
ccflags-y += -I$(PWD)/visp_mbox/
ccflags-y += -I$(PWD)/visp_mbox/mailbox/
ccflags-y += -I$(PWD)/visp_common/isp_ctrl/
ccflags-y += -I$(PWD)/visp_common/include/
ccflags-y += -I$(PWD)/visp_common/vsi_units/include/
ccflags-y += -I$(PWD)/visp_common/vsi_units/include/cam_device/
ccflags-y += -I$(PWD)/visp_common/vsi_units/include_priv/
ccflags-y += -I$(PWD)/visp_common/vsi_units/include_priv/cam_device/
ccflags-y += -I$(PWD)/include/




USER_INC:=$(PWD)/visp_common/isp_ctrl/visp_ctrl.h
USER_INC += $(PWD)/visp_mbox/mailbox/*.h
USER_INC += $(PWD)/visp_mimo_video/*.h
ccflags-y += -g -DDEBUG -DVISP_VARIANT_MIMO

PWD := $(shell pwd)
