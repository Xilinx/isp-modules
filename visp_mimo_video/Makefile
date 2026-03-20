ifeq ($(VISP_PATH),)
	VISP_PATH:=$(shell cd $(shell pwd)/../; pwd)
	export VISP_PATH
endif

TARGET := visp_mimo_video
obj-m := $(TARGET).o

#  $(CC) $(CFLAGS) -c $< -o $@
$(TARGET)-objs += visp_mimo_driver.o
$(TARGET)-objs += visp_mimo_driver_utils.o
$(TARGET)-objs += visp_video_event.o

ccflags-y += -I$(PWD)/common/
ccflags-y := -I$(PWD)/visp_mimo_video
ccflags-y += -I$(PWD)/visp_mimo/
ccflags-y += -I$(PWD)/visp_mimo/include/
ccflags-y += -I$(PWD)/common/
ccflags-y += -I$(PWD)/visp_mbox/
ccflags-y += -I$(PWD)/visp_mbox/mailbox/
ccflags-y += -I$(PWD)/visp_mimo/vsi_units/include/
ccflags-y += -I$(PWD)/visp_mimo/vsi_units/include/cam_device/
ccflags-y += -I$(M)/visp_mimo/vsi_units/include_priv/
ccflags-y += -I$(M)/visp_mimo/vsi_units/include_priv/cam_device/
ccflags-y += -I$(PWD)/include/
ccflags-y += -I$(PWD)/isp_ctrl/sensor
ccflags-y += -I$(PWD)/visp_mbox/mailbox/
ccflags-y += -I$(PWD)/visp_mbox/
USER_INC += $(PWD)/visp_mimo/isp_ctrl/sensor/*.h
USER_INC += $(PWD)/visp_mbox/mailbox/*.h
USER_INC += $(PWD)/visp_mimo/*.h
ccflags-y += -g -DDEBUG

PWD := $(shell pwd)
