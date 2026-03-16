TARGET := visp_mbox

obj-m := $(TARGET).o

$(TARGET)-objs += visp_mbox_driver.o
#$(TARGET)-objs += visp_event.o
#$(TARGET)-objs += visp_procfs.o
#$(TARGET)-objs += isp_ctrl/visp_ctrl.o
#$(TARGET)-objs += visp_app.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_cmd.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_api.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_fifo.o

$(info PWD_PATH is $(PWD))

ccflags-y := -DISP_VERSION_MACRO=\"Version:M17\ Release:26.1.0\"
ccflags-y += -I$(PWD)
ccflags-y += -I$(PWD)/visp/
ccflags-y += -I$(PWD)/visp/include/
ccflags-y += -I$(PWD)/common/
ccflags-y += -I$(PWD)/visp_mbox/
ccflags-y += -I$(PWD)/visp_mbox/mailbox/
#ccflags-y += -I$(PWD)/../visp/mailbox/
#ccflags-y += -I$(PWD)/../visp/isp_ctrl/
#ccflags-y += -I$(PWD)/../vvcma_isp/isp_ctrl/ae/aev2/

ccflags-y += -I$(PWD)/visp/vsi_units/include/
ccflags-y += -I$(PWD)/visp/vsi_units/include/cam_device/
ccflags-y += -I$(PWD)/include/
#ccflags-y += -I$(PWD)/../visp/user_lib/source/vsi_units/include_priv/cam_device/
#ccflags-y += -I$(PWD)/../visp/user_lib/source/vsi_units/source/cam_device/

USER_INC:=$(PWD)/isp_ctrl/visp_ctrl.h

ccflags-y += -DDEBUG -O2

PWD := $(shell pwd)
