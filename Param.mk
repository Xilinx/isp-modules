# Example Makefile

ISP_VERSION := ISP8200_V2311

#INSTALL_DIR:= $(VVCAM_PATH)/install/
include $(PWD)/isp_version/ISP8200_V2311.mk
ifeq ($(ISP_VERSION), ISP8200_V2311)
#include isp_version/ISP8200_V2311.mk
endif
