#KERNEL_SRC="/wrk/paeg1/users/katravul/Telluride/25_1/linux_driver/6_12_Kernel/linux-xlnx/"

ISP_VERSION := ISP8200_V2311

INSTALL_DIR:= $(VISP_PATH)/install/$(ISP_VERSION)

ifeq ($(ISP_VERSION), ISP8200_V2311)
#include $(VISP_PATH)/isp_version/ISP8200_V2311.mk
endif
