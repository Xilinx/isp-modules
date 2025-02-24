TARGET := visp_video

obj-m := $(TARGET).o

$(TARGET)-objs += visp_video_driver.o
$(TARGET)-objs += visp_video_register.o
$(TARGET)-objs += visp_video_event.o

EXTRA_CFLAGS += -I$(PWD)/common/
EXTRA_CFLAGS += -g -DDEBUG

PWD := $(shell pwd)
