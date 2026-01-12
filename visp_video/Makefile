TARGET := visp_video

obj-m := $(TARGET).o

$(TARGET)-objs += visp_video_driver.o
$(TARGET)-objs += visp_video_register.o

ccflags-y  += -I$(PWD)/common/
ccflags-y  += -g -DDEBUG

PWD := $(shell pwd)
