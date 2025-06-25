/* SPDX-License-Identifier: MIT */
/*
 * Copyright 2025 Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __IBA_H__
#define __IBA_H__

#include "cam_device_calibration.h"
#include "cam_device_common.h"
#include "iba.h"
#include "visp_driver.h"
#define MAX_IBA_PER_TILE 5UL

#define VISP_PLATFORM
typedef enum iba_id {
	IBA_0,
	IBA_1,
	IBA_2,
	IBA_3,
	IBA_4,
	MAX_IBA,
	DUMMY_IBA_ID = 0XDEADFEED

} iba_id_t;

typedef enum isp_input_mcm_port_id {
	ISP0_MCM_PORT_0,
	ISP0_MCM_PORT_1,
	ISP0_MCM_PORT_2,
	ISP0_MCM_PORT_3,
	ISP1_MCM_PORT_0,
	ISP1_MCM_PORT_1,
	DUMMY_ISP_INPUT_MCM_PORT = 0XDEADFEED
} iba_out_port_t;

typedef enum iba_input_pixel_width {
	SINGLE_PIXEL_WIDTH = 1,
	DUAL_PIXEL_WIDTH = 2,
	QUAD_PIXEL_WIDTH = 4,
	DUMMY_IBA_INPUT_PIXEL_WIDTH = 0XDEADFEED
} iba_input_pixel_width_t;

typedef enum iba_virtual_channel {
	VC0 = 0,
	VC1,
	VC2,
	VC3,
	DUMMY_VC = 0XDEADFEED
} iba_virtual_channel_id_t;

typedef enum iba_fifo_write_mode {
	SINGLE_FIFO_WRITE = 0,
	DUAL_FIFO_WRITE,
	QUAD_FIFO_WRITE,
	DUMMY_IBA_FIFO_WRITE_MODE = 0XDEADFEED
} iba_fifo_write_mode_t;

typedef enum tile_id {
	TILE_0 = 0,
	MAX_TILE,
	DUMMY_TILE_ID = 0XDEADFEED
} tile_id_t;

typedef enum iba_isp_instance {
	ISP0 = 0,
	ISP1 = 1,
	ISP2 = 2,
	ISP3 = 3,
	ISP4 = 4,
	ISP5 = 5,
	MAX_ISP,
	DUMMY_IBA_ISP = 0XDEADFEED
} iba_isp_instance_t;

typedef enum iba_mipi_number {
	MIPI_0 = 0,
	MIPI_1,
	MIPI_2,
	MIPI_3,
	MIPI_4,
	MIPI_5,
	MIPI_6,
	MIPI_7,
	MAX_MIPI_PER_TILE,
	DUMMY_MIPI_NUMBER = 0XDEADFEED
} iba_mipi_id_t;

typedef enum iba_status {
	IBA_DISABLED = 0,
	IBA_ENABLED,
	DUMMY_IBA_STATUS = 0XDEADFEED
} iba_status_t;

typedef struct iba_isp_combination {
} iba_isp_split_t;

typedef struct iba_config_params {
	cam_device_work_mode_t work_mode;
	uint32_t isp_hw_id;
	cam_device_mcm_port_id_t port_id;
} iba_config_params_t;

typedef struct iba_instance {
	u32 base_address;
	u32 iba_id;
	u32 hres;
	u32 vres;
	u32 data_format;
	iba_status_t iba_is_enabled;
	u32 hblank_prog;
	u32 vblank_prog;
	u32 virtual_channel_id;		 //Virtual channel of MIPI to which IBA
					 //   connected
	u32 input_pixel_width;		 // From Axi video stream
	iba_isp_instance_t isp_instance; // To which ISP is this IBA connected
	u32 frame_rate;
} __attribute((__packed__)) __attribute((aligned(8))) iba_inst_t;

typedef struct iba_map {
	iba_inst_t iba_inst[MAX_ISP][MAX_IBA_PER_ISP];
	uint32_t active_iba_count[MAX_ISP];
	int is_ready;
} __attribute((__packed__)) __attribute((aligned(8))) iba_map_t;

int iba_init_send_command(struct visp_dev *isp_dev,
			  cam_device_handle_t h_cam_device);

#define iba_write_reg(base_address, RegOffset, data)                            \
	do {                                                                       \
		Xil_Out32((UINTPTR)(((u8 *)base_address) + (RegOffset)), (u32)(data)); \
		xil_printf("RPU:IBA reg add 0x%x ,val 0x%x\n",                         \
		(UINTPTR)(((u8 *)base_address) + (RegOffset)), data);       \
		} while (0)
#define iba_read_reg(base_address, RegOffset)                                   \
	Xil_In32((UINTPTR)(((u8 *)base_address) + (RegOffset)))

#define XPAR_IBA_INSTANCES 4
#define XPAR_ISP_INSTANCE 2
#define XPAR_TILE_INSTANCES 1

/*
 * The configuration table for devices
 */
#define IBA_PIXEL_WIDTH_OFFSET (0x0)
#define IBA_HORIZONTAL_RES_OFFSET (0x4)
#define IBA_VERTICAL_RES_OFFSET (0x8)
#define IBA_VRES_LINE_CNT_OFFSET (0xc)
#define IBA_FRAME_CNT_OFFSET (0x10)
#define IBA_CFG_IA_FIFO_OFFSET (0x14)
#define IBA_VIRT_CHANNEL_OFFSET (0x24)

#define TILE_0_BASE_ADDR (0x88500000)
#define TILE_1_BASE_ADDR (0x88600000)
#define TILE_2_BASE_ADDR (0x88700000)

#define ISP0_BASE_ADDR (TILE_0_BASE_ADDR + 0x00000)
#define ISP1_BASE_ADDR (TILE_0_BASE_ADDR + 0x10000)
#define ISP2_BASE_ADDR (TILE_1_BASE_ADDR + 0x00000)
#define ISP3_BASE_ADDR (TILE_1_BASE_ADDR + 0x10000)
#define ISP4_BASE_ADDR (TILE_2_BASE_ADDR + 0x00000)
#define ISP5_BASE_ADDR (TILE_2_BASE_ADDR + 0x10000)

#define TILE0_SLCR_BASE_ADDR (TILE_0_BASE_ADDR + 0x80000)
#define TILE1_SLCR_BASE_ADDR (TILE_1_BASE_ADDR + 0x80000)
#define TILE2_SLCR_BASE_ADDR (TILE_2_BASE_ADDR + 0x80000)
#define ISP_SLCR_VIDIF3_SEL__OFFSET_ (0x2030)

#define XPAR_TILE0_IBA0_BASEADDR                                               \
	(TILE_0_BASE_ADDR + 0x20000) // IBA0 base address
#define XPAR_TILE0_IBA1_BASEADDR                                               \
	(TILE_0_BASE_ADDR + 0x21000) // IBA0 base address
#define XPAR_TILE0_IBA2_BASEADDR                                               \
	(TILE_0_BASE_ADDR + 0x22000) // IBA0 base address
#define XPAR_TILE0_IBA3_BASEADDR                                               \
	(TILE_0_BASE_ADDR + 0x23000) // IBA0 base address
#define XPAR_TILE0_IBA4_BASEADDR                                               \
	(TILE_0_BASE_ADDR + 0x24000) // ISP1 IBA0 base address

#define XPAR_TILE1_IBA0_BASEADDR                                               \
	(TILE_1_BASE_ADDR + 0x20000) // IBA0 base address
#define XPAR_TILE1_IBA1_BASEADDR                                               \
	(TILE_1_BASE_ADDR + 0x21000) // IBA0 base address
#define XPAR_TILE1_IBA2_BASEADDR                                               \
	(TILE_1_BASE_ADDR + 0x22000) // IBA0 base address
#define XPAR_TILE1_IBA3_BASEADDR                                               \
	(TILE_1_BASE_ADDR + 0x23000) // IBA0 base address
#define XPAR_TILE1_IBA4_BASEADDR                                               \
	(TILE_1_BASE_ADDR + 0x24000) // ISP1 IBA0 base address

#define XPAR_TILE2_IBA0_BASEADDR                                               \
	(TILE_2_BASE_ADDR + 0x20000) // IBA0 base address
#define XPAR_TILE2_IBA1_BASEADDR                                               \
	(TILE_2_BASE_ADDR + 0x21000) // IBA0 base address
#define XPAR_TILE2_IBA2_BASEADDR                                               \
	(TILE_2_BASE_ADDR + 0x22000) // IBA0 base address
#define XPAR_TILE2_IBA3_BASEADDR                                               \
	(TILE_2_BASE_ADDR + 0x23000) // IBA0 base address
#define XPAR_TILE2_IBA4_BASEADDR                                               \
	(TILE_2_BASE_ADDR + 0x24000) // ISP1 IBA0 base address

#define ISP_MCM_CTRL_OFFSET 0x1200

#endif
