#KERNEL_SRC="/everest/sivxhd_apps_bkup/users/chandu/Modular_designs/2024.2/MIPI/mipi_isp_hdmi_2.0/linux_source_build/linux-xlnx_versal_vck190_reva_x_ebm_01_reva-standard-build"

#KERNEL_SRC="/everest/sivxhd_apps_bkup/users/chandu/Modular_designs/2024.2/MIPI/mipi_isp_hdmi_2.0/kernel_source/linux-xlnx_versal_vck190_reva_x_ebm_01_reva-standard-build"
#KERNEL_SRC="/everest/sivxhd_apps_bkup/users/chandu/Modular_designs/2024.2/MIPI/mipi_isp_hdmi_2.0/kernel_source-for-visp/linux-xlnx_versal_vck190_reva_x_ebm_01_reva-standard-build"

#KERNEL_SRC="/everest/sivxhd_apps_bkup/users/chandu/Modular_designs/2024.2/MIPI/mipi_isp_hdmi_2.0/final/KERNEL_SRC_050124"

#KERNEL_SRC="/everest/sivxhd_apps_bkup/users/chandu/Modular_designs/2024.2/MIPI/mipi_isp_hdmi_2.0/final/KERNEL_SRC_050124/linux-xlnx_versal_vck190_reva_x_ebm_01_reva-standard-build"

#KERNEL_SRC="/wrk/paeg/users/amamidal/proj/audio/vcu/telluride/isp_driver_dev/vnc/linux/linux-xlnx"
#KERNEL_SRC="/wrk/paeg/users/amamidal/proj/audio/vcu/telluride/isp_driver_dev/vnc/linux/linux-xlnx"
#KERNEL_SRC="/wrk/paeg/users/amamidal/proj/audio/vcu/telluride/isp_driver_dev/vnc/linux/linux_build/output/linux-xlnx-arm64"
KERNEL_SRC="/wrk/paeg1/users/katravul/Telluride/25_1/linux_driver/6_12_Kernel/linux-xlnx/"
#KERNEL_SRC="/wrk/paeg/users/amamidal/proj/audio/vcu/telluride/isp_driver_dev/vnc/linux/linux-xlnx"
#KERNLE_SRC="/everest/sivxhd_apps_bkup/users/chandu/Modular_designs/2024.2/MIPI/mipi_isp_hdmi_2.0/linux_sources/linux-xlnx_versal_vck190_reva_x_ebm_01_reva-standard-build"


#KERNEL_SRC="/scratch/Telluride/M11_git_petalinux_build/Working_M11_VSI_NOV2024/build/tmp/work/xilinx_vck190-xilinx-linux/linux-xlnx/5.15.19+gitAUTOINC+b0c1be301e-r0/linux-xilinx_vck190-standard-build/"

#KERNEL_SRC=/everest/ssw_pset2_bkup/ranjith/linux/M13-VSI-V4l2/v4l2/petalinux/VSI_LINUX_MBOX_M5/build/tmp/work/xilinx_vck190-xilinx-linux/linux-xlnx/5.15.19+gitAUTOINC+b0c1be301e-r0/linux-xilinx_vck190-standard-build

ISP_VERSION := ISP8200_V2311

INSTALL_DIR:= $(VISP_PATH)/install/$(ISP_VERSION)

ifeq ($(ISP_VERSION), ISP8200_V2311)
#include $(VISP_PATH)/isp_version/ISP8200_V2311.mk
endif
