ifeq ($(VISP_PATH),)
	VISP_PATH:=$(shell cd $(shell pwd)/; pwd)
	export VISP_PATH
endif

include $(VISP_PATH)/isp_version/ISP8200_V2311.mk

SRC := $(shell pwd)

obj-m += visp_mbox/
obj-m += visp/
obj-m += visp_mimo/
obj-m += visp_video/

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install
clean:
	find . -name "*.ko" -type f -delete
	find . -name "*.o" -type f -delete
	find . -name "*.order" -type f -delete
	find . -name "*.cmd" -type f -delete
	find . -name "*.mod*" -type f -delete
	rm -f Module.symvers
