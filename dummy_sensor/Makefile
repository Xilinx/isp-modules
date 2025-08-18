TARGET := visp_dummy_sensor_drv

obj-m := $(TARGET).o

$(TARGET)-objs += dummy_sensor_drv.o 

EXTRA_CFLAGS += -I$(PWD)/../common/
EXTRA_CFLAGS += -g -DDEBUG

PWD := $(shell pwd)
