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

#ifndef __OBA_H__
#define __OBA_H__

#include "iba.h"

#define MAX_OBA_PER_TILE    4UL

#define MAX_OBA_PER_ISP     2UL
#define XPAR_OBA_INSTANCES  2UL


struct oba_info {
   uint32_t ppc;
   uint32_t bpp;
   const char * data_format;
};

typedef enum oba_id{
	OBA_0_MP,
	OBA_1_SP,
	OBA_2_MP,
	OBA_3_SP,
	MAX_OBA,
	DUMMY_OBA_ID = 0XDEADFEED
} oba_id_t;


typedef enum oba_path{
	MAIN_PATH=0,
	SELF_PATH,
	DUMMY_OBA_PATH_ID = 0XDEADFEED
} oba_path_t;


typedef enum oba_output_data_mode {
	RGB888_FORMAT,
	YUV444_FORMAT,
	YUV422_SP_FORMAT,
	YUV420_SP_FORMAT,
	Y_FORMAT,
	DUMMY_OBA_OUTPUT_DATA_MODE = 0XDEADFEED
}oba_data_mode_t;


typedef enum oba_data_type {
	YUV_420_8_BIT  = 0x18,
	YUV_420_10_BIT = 0x19,
	YUV_422_8_BIT  = 0x1E,
	YUV_422_10_BIT = 0x1F,
	RGB888         = 0x24,
	YUV_400_8_BIT = 0x20,
	YUV_400_10_BIT = 0x21,
	DUMMY_OBA_DATA_TYPE = 0XDEADFEED
}oba_data_type_t;


typedef enum oba_pixel_mode {
	SINGLE_PIXEL_MODE = 0,
	DUAL_PIXEL_MODE,
	QUAD_PIXEL_MODE,
	DUMMY_OBA_PIXEL_MODE = 0XDEADFEED
}oba_pixel_mode_t;

/*typedef enum tile_id{
	TILE_0 = 0,
	MAX_TILE,
	DUMMY_TILE_ID = 0XDEADFEED
} tile_id_t;
*/
typedef enum oba_isp_instance{
	OBA_ISP0 = 0,
	OBA_ISP1    ,
	MAX_ISP_PER_TILE,
	DUMMY_OBA_ISP = 0XDEADFEED
} oba_isp_instance_t;

typedef enum oba_status{
	OBA_DISABLED=0,
	OBA_ENABLED,
	DUMMY_OBA_STATUS = 0XDEADFEED
} oba_status_t;

typedef struct oba_isp_combination {

} oba_isp_split_t;

typedef struct oba_instance {

	/*
	 * Hi-addr, Low-addr , padding of 4bytes is required on RPU
	 * as the baseaddress will be aligned to 8 bytes as APU is 64 bit
	 */
	u32               		  BaseAddress[2];
	u32						  hcamdev_instanceid;
	oba_id_t        		  DeviceId;
	oba_path_t                path_info;
	oba_pixel_mode_t  		  pixle_mode;
	oba_data_mode_t 		  data_format;
	oba_data_type_t   		  data_type;
	tile_id_t                 tile_id;
	oba_status_t              oba_is_enabled;
	oba_isp_instance_t        isp_instance;           /*To which ISP is this oba connected*/

}__attribute((__packed__)) __attribute((aligned(8))) oba_inst_t ;

typedef struct oba_map {
	oba_inst_t      oba_inst[MAX_TILE][MAX_ISP_PER_TILE][MAX_OBA_PER_ISP];
	int IsReady;
}  __attribute((__packed__)) __attribute((aligned(8))) oba_map_t ;

#define oba_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32( (UINTPTR)(((u8 *)BaseAddress) + (RegOffset)), (u32)(Data)) ; TRACE(OBA_RPU_INFO,"RPU:oba reg add 0x%x ,val 0x%x \n",(UINTPTR)(((u8 *)BaseAddress) + (RegOffset)),Data);
#define oba_ReadReg(BaseAddress, RegOffset) \
    Xil_In32( (UINTPTR) (((u8 *)BaseAddress) + (RegOffset)))


/*
* The configuration table for devices
*/
//#define XPAR_ISP0_OBA_PIX_MODE_BASEADDR (0xe8540000) //oba0 base address  xE8540004
//#define XPAR_ISP1_OBA_PIX_MODE_BASEADDR (0xe8541000)

#define ISP0_OBA0_BASEADDR \
	(ISP0_BASE_ADDR + 0x40000) //TILE_0 ISP_0 OBA base address
#define ISP0_OBA1_BASEADDR \
	(ISP0_BASE_ADDR + 0x41000) //TILE_0 ISP_1 OBA base address

#define ISP1_OBA0_BASEADDR \
	(ISP1_BASE_ADDR + 0x40000) //TILE_1 ISP_0 OBA base address
#define ISP1_OBA1_BASEADDR \
	(ISP1_BASE_ADDR + 0x41000) //TILE_1 ISP_1 OBA base address


#define ISP2_OBA0_BASEADDR \
	(ISP2_BASE_ADDR + 0x40000) //TILE_2 ISP_0 OBA base address
#define ISP2_OBA1_BASEADDR \
	(ISP2_BASE_ADDR + 0x41000) //TILE_2 ISP_1 OBA base address

#define ISP3_OBA0_BASEADDR \
	(ISP3_BASE_ADDR + 0x40000) //TILE_3 ISP_0 OBA base address
#define ISP3_OBA1_BASEADDR \
	(ISP3_BASE_ADDR + 0x41000) //TILE_3 ISP_1 OBA base address

#define ISP4_OBA0_BASEADDR \
	(ISP4_BASE_ADDR + 0x40000) //TILE_4 ISP_0 OBA base address
#define ISP4_OBA1_BASEADDR \
	(ISP4_BASE_ADDR + 0x41000) //TILE_4 ISP_1 OBA base address

#define ISP5_OBA0_BASEADDR \
	(ISP5_BASE_ADDR + 0x40000) //TILE_5 ISP_0 OBA base address
#define ISP5_OBA1_BASEADDR \
	(ISP5_BASE_ADDR + 0x41000) //TILE_5 ISP_1 OBA base address


RESULT OBA_init_send_command
(
        struct visp_dev *isp_dev,
	    CamDeviceHandle_t  hCamDevice
);


#endif
