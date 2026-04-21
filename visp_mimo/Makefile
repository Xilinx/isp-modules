ifeq ($(VISP_PATH),)
	VISP_PATH:=$(shell cd $(shell pwd)/../; pwd)
	export VISP_PATH
endif


TARGET := visp_mimo
#  $(CC) $(LDFLAGS) -o $@ $^

obj-m := $(TARGET).o
#  $(CC) $(CFLAGS) -c $< -o $@

$(TARGET)-objs-y += -Wconversion -Wsign-conversion -Wextra -Wall

$(TARGET)-objs += visp_driver.o
$(TARGET)-objs += visp_procfs.o
$(TARGET)-objs += visp_app.o

$(TARGET)-objs += vsi_units/source/cam_device/cam_device_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_sensor_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_buffer_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_ae_api.o
$(TARGET)-objs += vsi_units/source/cam_device/cam_device_awb_api.o

ccflags-y := -I$(PWD)/visp_mimo/
ccflags-y += -I$(PWD)/common/
ccflags-y += -I$(PWD)/visp_mbox/mailbox/
ccflags-y += -I$(PWD)/visp_mbox/
ccflags-y += -I$(PWD)/visp_mimo/vsi_units/include/
ccflags-y += -I$(PWD)/visp_mimo/vsi_units/include/cam_device/
ccflags-y += -I$(PWD)/visp_mimo/include/
ccflags-y += -I$(PWD)/visp_mimo/vsi_units/include_priv/cam_device/
ccflags-y += -I$(PWD)/visp_mimo/vsi_units/source/cam_device/
ccflags-y += -I$(PWD)/visp_mimo/vsi_units/isi/include/
ccflags-y += -I$(PWD)/visp_mimo/vsi_units/
ccflags-y += -I$(PWD)/visp_mimo/vsi_units/isi/


USER_INC += $(PWD)/visp_mbox/mailbox/*.h


ccflags-y += -DDEBUG -O2

PWD := $(shell pwd)
