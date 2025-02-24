ifeq ($(VVCAM_PATH),)
	VVCAM_PATH:=$(shell cd $(shell pwd)/../; pwd)
	export VVCAM_PATH
endif

#include $(PWD)/../Param.mk
TARGET := visp_mbox_mimo

obj-m := $(TARGET).o

$(TARGET)-objs += amd_mbox_driver.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_cmd.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_api.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_fifo.o


EXTRA_CFLAGS := -I$(PWD)
EXTRA_CFLAGS += -I$(PWD)/visp_mimo/
EXTRA_CFLAGS += -I$(PWD)/visp_mimo/include/
EXTRA_CFLAGS += -I$(PWD)/common/
EXTRA_CFLAGS += -I$(PWD)/visp_mbox_mimo/mbox/
EXTRA_CFLAGS += -I$(PWD)/visp_mbox_mimo/mailbox/
EXTRA_CFLAGS += -I$(PWD)/visp_mimo/vsi_units/include/
EXTRA_CFLAGS += -I$(PWD)/visp_mimo/vsi_units/include/cam_device/
EXTRA_CFLAGS += -I$(PWD)/include/

USER_INC:=$(PWD)/visp_mimo/isp_ctrl/vvcam_isp_ctrl.h

EXTRA_CFLAGS += -DDEBUG -O2 

PWD := $(shell pwd)
