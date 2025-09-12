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

ccflags-y := -DISP_VERSION_MACRO=\"Version:M15\ Release:25.2.0\"

EXTRA_CFLAGS := -I$(PWD)
EXTRA_CFLAGS += -I$(PWD)/visp/
EXTRA_CFLAGS += -I$(PWD)/visp/include/
EXTRA_CFLAGS += -I$(PWD)/common/
EXTRA_CFLAGS += -I$(PWD)/visp_mbox/
EXTRA_CFLAGS += -I$(PWD)/visp_mbox/mailbox/
#EXTRA_CFLAGS += -I$(PWD)/../visp/mailbox/
#EXTRA_CFLAGS += -I$(PWD)/../visp/isp_ctrl/
#EXTRA_CFLAGS += -I$(PWD)/../vvcma_isp/isp_ctrl/ae/aev2/


EXTRA_CFLAGS += -I$(PWD)/visp/vsi_units/include/
EXTRA_CFLAGS += -I$(PWD)/visp/vsi_units/include/cam_device/
EXTRA_CFLAGS += -I$(PWD)/include/
#EXTRA_CFLAGS += -I$(PWD)/../visp/user_lib/source/vsi_units/include_priv/cam_device/
#EXTRA_CFLAGS += -I$(PWD)/../visp/user_lib/source/vsi_units/source/cam_device/


USER_INC:=$(PWD)/isp_ctrl/visp_ctrl.h


#EXTRA_CFLAGS += -I$(USER_INC)

EXTRA_CFLAGS += -DDEBUG -O2 
#EXTRA_CFLAGS += -DDEBUG -O2 -g3 -fno-inline

PWD := $(shell pwd)
