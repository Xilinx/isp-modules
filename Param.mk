/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/
 
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
