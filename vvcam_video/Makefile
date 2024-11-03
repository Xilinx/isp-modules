ifeq ($(VVCAM_PATH),)
	VVCAM_PATH:=$(shell cd $(shell pwd)/../; pwd)
	export VVCAM_PATH
endif

include $(PWD)/Param.mk

TARGET := vvcam_video

obj-m := $(TARGET).o

$(TARGET)-objs += vvcam_video_driver.o
$(TARGET)-objs += vvcam_video_register.o
$(TARGET)-objs += vvcam_video_event.o

EXTRA_CFLAGS += -I$(PWD)/
EXTRA_CFLAGS += -I$(PWD)/common/
EXTRA_CFLAGS += -g -DDEBUG
