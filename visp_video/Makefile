TARGET := visp_video

obj-m := $(TARGET).o

$(TARGET)-objs += visp_video_driver.o
$(TARGET)-objs += visp_video_register.o

EXTRA_CFLAGS += -I$(PWD)/common/
EXTRA_CFLAGS += -g -DDEBUG

PWD := $(shell pwd)
