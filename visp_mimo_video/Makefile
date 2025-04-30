ifeq ($(VISP_PATH),)
	VISP_PATH:=$(shell cd $(shell pwd)/../; pwd)
	export VISP_PATH
endif

TARGET := visp_video_mimo
obj-m := $(TARGET).o

#  $(CC) $(CFLAGS) -c $< -o $@
$(TARGET)-objs += visp_mimo_driver.o
$(TARGET)-objs += visp_mimo_driver_utils.o
$(TARGET)-objs += visp_video_event.o

EXTRA_CFLAGS += -I$(PWD)/common/
EXTRA_CFLAGS := -I$(PWD)/visp_mimo_video
EXTRA_CFLAGS += -I$(PWD)/visp_mimo/
EXTRA_CFLAGS += -I$(PWD)/visp_mimo/include/
EXTRA_CFLAGS += -I$(PWD)/common/
EXTRA_CFLAGS += -I$(PWD)/visp_mbox_mimo/
EXTRA_CFLAGS += -I$(PWD)/visp_mbox_mimo/mailbox/
EXTRA_CFLAGS += -I$(PWD)/visp_mimo/vsi_units/include/
EXTRA_CFLAGS += -I$(PWD)/visp_mimo/vsi_units/include/cam_device/
EXTRA_CFLAGS += -I$(PWD)/include/
EXTRA_CFLAGS += -I$(PWD)/isp_ctrl/sensor
EXTRA_CFLAGS += -I$(PWD)/visp_mbox_mimo/mailbox/
EXTRA_CFLAGS += -I$(PWD)/visp_mbox_mimo/
USER_INC += $(PWD)/visp_mimo/isp_ctrl/sensor/*.h
USER_INC += $(PWD)/visp_mbox_mimo/mailbox/*.h
EXTRA_CFLAGS += -g -DDEBUG

PWD := $(shell pwd)
