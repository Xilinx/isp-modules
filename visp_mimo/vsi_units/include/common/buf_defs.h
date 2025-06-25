/* SPDX-License-Identifier: MIT*/
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

#ifndef __CAMERA_DEVICE_BUF_DEFS_H__
#define __CAMERA_DEVICE_BUF_DEFS_H__

#include <ebase/types.h>

#define UNIQUE_ENUM_NAME(u) assert_static__##u
#define GET_ENUM_NAME(x) UNIQUE_ENUM_NAME(x)
#define DCT_ASSERT_STATIC(e) _Static_assert((e), "Static assertion failed: " #e)

#define BUFF_POOL_MAX_INPUT_BUF_NUMBER 8
#define BUFF_POOL_MAX_OUTPUT_BUF_NUMBER 16

#define PIC_BUFFER_ALIGN (0x400)
#define PIC_WIDTH_ALIGN 16
#define ALIGN_16BYTES(width) (((width) + (0xFU)) & (~(0xFU)))

#define PIC_BUFFER_MEM_SIZE_MAX (512 * 1024 * 1024)

#define PIC_BUFFER_NUM_INPUT (1)
#define PIC_BUFFER_SIZE_INPUT (110 * 1024 * 1024) // 55MPixel@12bit => 110MByte

// The Software does not need too many large buffer, or will out of memory in
// cmodel test.
#ifdef SW_TEST
#define PIC_BUFFER_NUM_LARGE_IMAGE (4)
#else
#define PIC_BUFFER_NUM_LARGE_IMAGE (7)
#endif
#define PIC_BUFFER_SIZE_LARGE_IMAGE                                            \
	(42 * 1024 * 1024) // 21MPixel@12bit => 42MByte
#define PIC_BUFFER_NUM_SMALL_IMAGE (13)
#define PIC_BUFFER_SIZE_SMALL_IMAGE                                            \
	(8 * 1024 * 1024) // 2MPixel@4:4:4 10bit  => 8MByte

DCT_ASSERT_STATIC(((PIC_BUFFER_NUM_INPUT * PIC_BUFFER_SIZE_INPUT) +
		   (PIC_BUFFER_NUM_LARGE_IMAGE * PIC_BUFFER_SIZE_LARGE_IMAGE) +
		   (PIC_BUFFER_NUM_SMALL_IMAGE * PIC_BUFFER_SIZE_SMALL_IMAGE)) <
		  PIC_BUFFER_MEM_SIZE_MAX);

#ifdef ISP_SINGLE_DOM_OVER_MP
#define PIC_BUFFER_NUM_MAIN_SENSOR (10)
#define PIC_BUFFER_SIZE_MAIN_SENSOR                                            \
	(16 * 1024 * 1024) //  2MPixel@4:4:4  => 6MByte
#define PIC_BUFFER_NUM_SELF1_SENSOR (0)
#define PIC_BUFFER_SIZE_SELF1_SENSOR                                           \
	(6 * 1024 * 1024) // 2MPixel@4:4:4  => 6MByte, no buffer for sp
#define PIC_BUFFER_NUM_SELF2_SENSOR (0)
#define PIC_BUFFER_SIZE_SELF2_SENSOR                                           \
	(6 * 1024 * 1024) // 2MPixel@4:4:4  => 6MByte, no buffer for sp
#define PIC_BUFFER_NUM_MCMWR_SENSOR (6)
#define PIC_BUFFER_SIZE_MCMWR_SENSOR (8 * 1024 * 1024)

#define PIC_BUFFER_NUM_METADATA (10)
#define PIC_BUFFER_SIZE_METADATA (8 * 1024 * 1024) //  2MPixel@4:4:4  => 6MByte

#ifdef ISP_MI_HDR
#define PIC_BUFFER_NUM_HDR_SENSOR (6)
#define PIC_BUFFER_SIZE_HDR_SENSOR (8 * 1024 * 1024)
#endif
#define PIC_BUFFER_NUM_PP_PATH_RAW_SENSOR (0)
#define PIC_BUFFER_SIZE_PP_PATH_RAW_SENSOR                                     \
	(6 * 1024 * 1024) // 2MPixel@4:4:4  => 6MByte, no buffer for sp

#define PIC_BUFFER_NUM_MAIN_SENSOR_IMGSTAB (10)
#define PIC_BUFFER_SIZE_MAIN_SENSOR_IMGSTAB                                    \
	(6 * 1024 * 1024) // ISI_RES_4416_3312 => 14MPixel@12bit => 28MByte
#define PIC_BUFFER_NUM_SELF_SENSOR_IMGSTAB (10)
#define PIC_BUFFER_SIZE_SELF_SENSOR_IMGSTAB                                    \
	(6 * 1024 * 1024) // ISI_RES_4416_3312 => 14MPixel@12bit => 28MByte

DCT_ASSERT_STATIC(((PIC_BUFFER_NUM_MAIN_SENSOR * PIC_BUFFER_SIZE_MAIN_SENSOR) +
		   (PIC_BUFFER_NUM_SELF1_SENSOR *
		    PIC_BUFFER_SIZE_SELF1_SENSOR)) < PIC_BUFFER_MEM_SIZE_MAX);
#else // ISP_SINGLE_DOM_OVER_MP
// #define PIC_BUFFER_NUM_MAIN_SENSOR  (10)
// #define PIC_BUFFER_SIZE_MAIN_SENSOR (36 * 1024 * 1024)    //
// ISI_RES_4416_3312 => 14MPixel@12bit => 28MByte
#define PIC_BUFFER_NUM_MAIN_SENSOR (6)
#define PIC_BUFFER_SIZE_MAIN_SENSOR (64 * 1024 * 1024) // 8k
#define PIC_BUFFER_NUM_SELF1_SENSOR (8)
#define PIC_BUFFER_SIZE_SELF1_SENSOR                                           \
	(6 * 1024 * 1024) // 2MPixel@4:4:4  => 6MByte
#define PIC_BUFFER_NUM_SELF2_SENSOR (6)
#define PIC_BUFFER_SIZE_SELF2_SENSOR                                           \
	(6 * 1024 * 1024) // 2MPixel@4:4:4  => 6MByte
#define PIC_BUFFER_NUM_MCMWR_SENSOR (6)
#define PIC_BUFFER_SIZE_MCMWR_SENSOR (8 * 1024 * 1024)
#ifdef ISP_MI_HDR
#define PIC_BUFFER_NUM_HDR_SENSOR (6)
#define PIC_BUFFER_SIZE_HDR_SENSOR (8 * 1024 * 1024)
#endif
#define PIC_BUFFER_NUM_METADATA (10)
#define PIC_BUFFER_SIZE_METADATA (8 * 1024 * 1024) //  2MPixel@4:4:4  => 6MByte

#define PIC_BUFFER_NUM_PP_PATH_RAW_SENSOR (2)
#define PIC_BUFFER_SIZE_PP_PATH_RAW_SENSOR                                     \
	(6 * 1024 * 1024) // 2MPixel@4:4:4  => 6MByte

#define PIC_BUFFER_NUM_MAIN_SENSOR_IMGSTAB (10)
#define PIC_BUFFER_SIZE_MAIN_SENSOR_IMGSTAB                                    \
	(6 * 1024 * 1024) // ISI_RES_4416_3312 => 14MPixel@12bit => 28MByte
#define PIC_BUFFER_NUM_SELF_SENSOR_IMGSTAB (10)
#define PIC_BUFFER_SIZE_SELF_SENSOR_IMGSTAB                                    \
	(6 * 1024 * 1024) // ISI_RES_4416_3312 => 14MPixel@12bit => 28MByte

DCT_ASSERT_STATIC(((PIC_BUFFER_NUM_MAIN_SENSOR * PIC_BUFFER_SIZE_MAIN_SENSOR) +
		   (PIC_BUFFER_NUM_SELF1_SENSOR *
		    PIC_BUFFER_SIZE_SELF1_SENSOR)) < PIC_BUFFER_MEM_SIZE_MAX);
#endif			  // ISP_SINGLE_DOM_OVER_MP

typedef enum _BUFF_MODE_ {
	BUFF_MODE_INVALID = 0,
	BUFF_MODE_USRPTR = 1,
	BUFF_MODE_RESMEM = 2,
	BUFF_MODE_MAX,
	DUMMMY_BUFF_MODE = 0xDEADFEED,
} buff_mode;

#define MEDIA_BUF_ALIGN(buf, align) (((buf) + ((align)-1U)) & ~((align)-1U))

/*Typedef HAL handle*/
typedef void *hal_handle_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to specify the swap control.
 *
 * @note    This enumeration type is used to specify the swap control.
 *
 *****************************************************************************/
typedef enum pic_buf_mi_swap_e {
	PIC_BUF_MI_SWAP_INVALID =
	    -1, /**< lower border (only for an internal evaluation) */
	PIC_BUF_MI_NO_SWAP = 0,	   /**< The value of no swap. */
	PIC_BUF_MI_SWAP_BYTES = 1, /**< The value of swap bytes. */
	PIC_BUF_MI_SWAP_WORDS = 2, /**< The value of swap words. */
	PIC_BUF_MI_SWAP_DOUBLE_WORDS =
	    4,				/**< The value of swap double words. */
	PIC_BUF_MI_SWAP_FOUR_WORDS = 8, /**< The value of swap four words. */
	PIC_BUF_MI_SWAP_MAX, /**< upper border (only for an internal evaluation)
			      */
	DUMMY_PIC_BUF_MI_SWAP = 0xDEADFEED,
} pic_buf_mi_data_swap_t;

/*****************************************************************************/
/**
 * @brief   Union type to specify the raw and yuv swap control.
 *
 * @note    This union type is used to specify the raw and yuv swap control.
 *
 *****************************************************************************/
typedef union pic_buf_mi_swap_u {
	struct {
		pic_buf_mi_data_swap_t y;
		pic_buf_mi_data_swap_t u;
		pic_buf_mi_data_swap_t v;
	} yuv_swap;
	pic_buf_mi_data_swap_t raw_swap;
} pic_buf_mi_swap_t;

typedef struct _buf_identity_ {
	uint32_t buff_size; // common: biffer size
	int width;	    // common: image width
	int height;	    // common: image height
	int format;	    // common: image format enum
	int width_bytes;    // for yuvxxxP/yuvxxxSP , it means width_bytes of y
			    // channel;for yuvxxxI , it means width_bytes of yuv
			    // channel
	int cb_cr_width_bytes;	// yuvxxxSP: width bytes of uv line
	int cb_cr_height_pixel; // yuvxxxSP: num of uv line
	int cb_width_bytes;	// yuvxxxP: width bytes of u line
	int cb_height_pixel;	// yuvxxxP: num of u line
	int cr_width_bytes;	// yuvxxxP: width bytes of v line
	int cr_height_pixel;	// yuvxxxP: num of v line
	int yuv_order;		// RGB888: y,u,v order
	int align_mode;		// common: align mode enmu
	uint8_t bit_width;	// common: bit width enmu
	uint32_t buffer_instance;
	uint32_t buff_address;	// common: buffer physical address
	pic_buf_mi_swap_t swap; // common: swap info
} buf_identity;

typedef void *buf_mgmt_handle_t;

// Pic buffers

/*****************************************************************************/
/**
 *          pic_buf_type_t
 *
 * @brief   The type of image data a picture buffer holds.
 *
 * @note    MVDU_FXQuad requires PIC_BUF_TYPE_YCbCr422 in
 *PIC_BUF_LAYOUT_SEMIPLANAR mode.
 *
 *****************************************************************************/
typedef enum pic_buf_type_e {
	PIC_BUF_TYPE_INVALID = 0,
	PIC_BUF_TYPE_JPEG = 2,
	PIC_BUF_TYPE_YCbCr444 = 3,
	PIC_BUF_TYPE_YCbCr422 = 4,
	PIC_BUF_TYPE_YCbCr420 = 5,
	PIC_BUF_TYPE_YCbCr400 = 6,
	PIC_BUF_TYPE_RGB888 = 7,
	PIC_BUF_TYPE_RGB666 = 8, // r, g & b are LSBit aligned!
	PIC_BUF_TYPE_RGB565 =
	    9, // TODO: don't know the memory layout right now, investigate!
	PIC_BUF_TYPE_RAW8 = 10,
	PIC_BUF_TYPE_RAW12 =
	    11, // includes: 12bits, MSBit aligned, LSByte first!
	PIC_BUF_TYPE_RAW10 =
	    13, // includes: 10bits, MSBit aligned, LSByte first!
	PIC_BUF_TYPE_RAW14 =
	    14, // includes: 14bits, MSBit aligned, LSByte first!
	PIC_BUF_TYPE_RAW16 =
	    15, // includes: 9..16bits, MSBit aligned, LSByte first!
	PIC_BUF_TYPE_RAW20 = 16, // includes: 20bits, compressed to 16bit MSBit
				 // aligned, LSByte first!
	PIC_BUF_TYPE_RAW20_COMPRESS = 17, // includes: 20bits compressed to
					  // 16bit MSBit aligned, LSByte first!
	PIC_BUF_TYPE_RAW24 =
	    18, // includes: 24bits, MSBit aligned, LSByte first!
	PIC_BUF_TYPE_RAW24_COMPRESS = 19, // includes: 24bits, compressed to
					  // 16bit MSBit aligned, LSByte first!
	PIC_BUF_TYPE_META = 20,
	PIC_BUF_TYPE_DATA = 21, // just some sequential data
	PIC_BUF_TYPE_YCbCr32 = 22,
	// PIC_BUF_TYPE_YCbCr400 = 0x33, // "Black&White"
	PIC_BUF_TYPE_RGB32 = 23,
	PIC_BUF_TYPE_DPCC32 = 24,
	DUMMY_PIC_BUF_TYPE = 0xDEADFEED,
} pic_buf_type_t;

/*****************************************************************************/
/**
 * @brief   RDCE bit depth enum.
 *
 * @note    This is a enum of RDCE bit depth.
 *
 *****************************************************************************/
typedef enum pic_buf_rdce_type_e {
	PIC_BUF_RDCE_BIT_DEPTH_INVALID = -1,
	PIC_BUF_RDCE_BIT_DEPTH_RAW8,
	PIC_BUF_RDCE_BIT_DEPTH_RAW10,
	PIC_BUF_RDCE_BIT_DEPTH_RAW12,
	PIC_BUF_RDCE_BIT_DEPTH_RAW14,
	PIC_BUF_RDCE_BIT_DEPTH_RAW16,
	PIC_BUF_RDCE_BIT_DEPTH_MAX,
	DUMMY_PIC_BUF_RDCE_BIT_DEPTH = 0xDEADFEED,
} pic_buf_rdce_type_t;

/*****************************************************************************/
/**
 * @brief   RDCE bayer pattern enum.
 *
 * @note    This is a enum of RDCE bayer pattern.
 *
 *****************************************************************************/
typedef enum pic_buf_rdce_bayer_pat_e {
	PIC_BUF_RDCE_BPT_INVALID = -1,
	PIC_BUF_RDCE_BPT_RGGB, /** < Bayer pattern RGGB. */
	PIC_BUF_RDCE_BPT_GRBG, /** < Bayer pattern GRBG. */
	PIC_BUF_RDCE_BPT_GBRG, /** < Bayer pattern GBRG. */
	PIC_BUF_RDCE_BPT_BGGR, /** < Bayer pattern BGGR. */
	PIC_BUF_RDCE_BPT_MAX,
	DUMMY_PIC_BUF_RDCE_BPT = 0xDEADFEED,
} pic_buf_rdce_bayer_pat_t;

/*****************************************************************************/
/**
 *          pic_buf_layout_t
 *
 * @brief   The layout of the image data a picture buffer holds.
 *
 * @note    MVDU_FXQuad requires PIC_BUF_TYPE_YCbCr422 in
 *PIC_BUF_LAYOUT_SEMIPLANAR mode.
 *
 *****************************************************************************/
typedef enum pic_buf_layout_e {
	PIC_BUF_LAYOUT_INVALID = 0,

	PIC_BUF_LAYOUT_COMBINED =
	    0x10, // PIC_BUF_TYPE_DATA:      data: D0 D1 D2...
		  // PIC_BUF_TYPE_RAW8:      data: D0 D1 D2...
		  // PIC_BUF_TYPE_RAW16/10:  data: D0L D0H D1L D1H...
		  // PIC_BUF_TYPE_JPEG:      data: J0 J1 J2...
		  // PIC_BUF_TYPE_YCbCr444:  data: Y0 Cb0 Cr0 Y1 Cb1Cr1...
		  // PIC_BUF_TYPE_YCbCr422:  data: Y0 Cb0 Y1 Cr0 Y2 Cb1 Y3
		  // Cr1... PIC_BUF_TYPE_YCbCr32:   data: Cr0 Cb0 Y0 A0 Cr1 Cb1
		  // Y1 A1... PIC_BUF_TYPE_RGB888:    data: R0 G0 B0 R1 B2 G1...
		  // PIC_BUF_TYPE_RGB666:    data: {00,R0[5:0]} {00,G0[5:0]}
		  // {00,B0[5:0]} {00,R1[5:0]} {00,G2[5:0]} {00,B3[5:0]}...
		  // PIC_BUF_TYPE_RGB565:    data: {R0[4:0],G0[5:3]}
		  // {G0[2:0],B0[4:0]} {R1[4:0],G1[5:3]} {G1[2:0],B1[4:0]}...
		  // (is this correct?) PIC_BUF_TYPE_RGB32:     data: B0 G0 R0
		  // A0 B1 G1 R1 A1...
	PIC_BUF_LAYOUT_BAYER_RGRGGBGB =
	    0x11, // 1st line: RGRG... , 2nd line GBGB... , etc.
	PIC_BUF_LAYOUT_BAYER_GRGRBGBG =
	    0x12, // 1st line: GRGR... , 2nd line BGBG... , etc.
	PIC_BUF_LAYOUT_BAYER_GBGBRGRG =
	    0x13, // 1st line: GBGB... , 2nd line RGRG... , etc.
	PIC_BUF_LAYOUT_BAYER_BGBGGRGR =
	    0x14, // 1st line: BGBG... , 2nd line GRGR... , etc.

	PIC_BUF_LAYOUT_SEMIPLANAR =
	    0x20, // PIC_BUF_TYPE_YCbCr422:  Luma:  Y0 Y1 Y2 Y3... ; Chroma: Cb0
		  // Cr0 Cb1 Cr1... PIC_BUF_TYPE_YCbCr420:  Luma:  Y0 Y1 Y2
		  // Y3... ; Chroma: Cb0 Cr0 Cb1 Cr1... PIC_BUF_TYPE_YCbCr400:
		  // Luma:  Y0 Y1 Y2 Y3... ; Chroma: not used

	PIC_BUF_LAYOUT_PLANAR =
	    0x30, // PIC_BUF_TYPE_YCbCr444:  y: Y0 Y1 Y2 Y3...;  cb: Cb0 Cb1 Cb2
		  // Cb3...; cr: Cr0 Cr1 Cr2 Cr3... PIC_BUF_TYPE_YCbCr422:  y:
		  // Y0 Y1 Y2 Y3...;  cb: Cb0 Cb1 Cb2 Cb3...; cr: Cr0 Cr1 Cr2
		  // Cr3... PIC_BUF_TYPE_YCbCr420:  y: Y0 Y1 Y2 Y3...;  cb: Cb0
		  // Cb1 Cb2 Cb3...; cr: Cr0 Cr1 Cr2 Cr3...
		  // PIC_BUF_TYPE_YCbCr400:  y: Y0 Y1 Y2 Y3...;  cb: not used;
		  // cr: not used... PIC_BUF_TYPE_RGB888:    r: R0 R1 R2 R3...;
		  // g:  G0 G1 G2 G3...;     b:  B0 B1 B2 B3...
		  // PIC_BUF_TYPE_RGB666:    r: {00,R0[5:0]}...; g:
		  // {00,G0[5:0]}...;    b:  {00,B0[5:0]}...
	PIC_BUF_LAYOUT_META_DATA = 0x40,
	DUMMY_PIC_BUF_LAYOUT = 0xDEADFEED,
} pic_buf_layout_t;

/*****************************************************************************/
/**
 *          PixelDataAlignMode_t
 *
 * @brief   The align mode of the image data a picture buffer holds.
 *
 *
 *****************************************************************************/

typedef enum pic_buf_align_e {
	PIC_BUF_DATA_ALIGN_MODE_INVALID = -1,
	PIC_BUF_DATA_UNALIGN_MODE = 0,	    // pixel data not aligned.
	PIC_BUF_DATA_ALIGN_128BIT_MODE = 1, // pixel data  aligned with 128 bit.
	PIC_BUF_DATA_ALIGN_DOUBLE_WORD =
	    1,			     // pixel data  aligned with double word.
	PIC_BUF_DATA_ALIGN_WORD = 2, // pixel data  aligned with word.
	PIC_BUF_DATA_ALIGN_16BIT_MODE = 2, // pixel data  aligned with 16 bit.
	PIC_BUF_DATA_ALIGN_MODE_MAX,
	DUMMY_PIC_BUF_DATA_ALIGN_MODE = 0xDEADFEED,
} pic_buf_align_t;

typedef enum pic_buf_yuvbit_e {
	PIC_BUF_DATA_YUV_BIT_MAX_INVALID = -1,
	PIC_BUF_DATA_YUV_8_BIT = 0,  //  yuv pixel data 8 bit
	PIC_BUF_DATA_YUV_10_BIT = 1, // yuv pixel data 10 bit
	PIC_BUF_DATA_YUV_12_BIT = 2, // yuv pixel data 12 bit
	PIC_BUF_DATA_YUV_BIT_MAX,
	DUMMY_PIC_BUF_DATA_YUV_BIT = 0xDEADFEED,
} pic_buf_yuvbit_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to specify the order of YUV or rgb channel,
 *          Right now only surpport RGB888 format
 *
 *****************************************************************************/
typedef enum pic_buf_yuv_order_e {
	PIC_BUF_CHANNEL_ORDER_INVALID = -1,
	PIC_BUF_CHANNEL_ORDER_YUV = 0, /** 0: YUV or rgb */
	PIC_BUF_CHANNEL_ORDER_YVU = 1, /** 1: YVU or RBG */
	PIC_BUF_CHANNEL_ORDER_UYV = 2, /** 2: UYV or GRB */
	PIC_BUF_CHANNEL_ORDER_VYU = 3, /** 3: VYU or BRG */
	PIC_BUF_CHANNEL_ORDER_UVY = 4, /** 4: UVY or GBR */
	PIC_BUF_CHANNEL_ORDER_VUY = 5, /** 5: VUY or BGR */
	PIC_BUF_CHANNEL_ORDER_MAX = 6,
	DUMMY_PIC_BUF_CHANNEL_ORDER = 0xDEADFEED,
} pic_buf_yuv_order_t;

/*****************************************************************************/
/**
 * @brief   Picture buffer compress lossless/lossy  mode
 *
 *****************************************************************************/
typedef enum pic_buf_loss_mode_e {
	PIC_BUF_LOSS_MODE_INVALID = -1, /**< Invalid loss mode*/
	PIC_BUF_LOSS_LESS_MODE,		/**< lossless mode */
	PIC_BUF_LOSSY_MODE,		/**< lossy mode */
	PIC_BUF_LOSS_MODE_MAX,
	DUMMY_PIC_BUF_LOSS_MODE = 0xDEADFEED,
} pic_buf_loss_mode_t;

/*******************************************************************************
 * @brief Common SCMI buffer type
 *
 * This structure defines a SCMI buffer. In addition to an address pointer to
 * the actual buffer, this structure also contains the size of the buffer and
 * an optional (may be null) pointer to some buffer meta data. This meta data
 * is defined seperately for each specific buffer type. The buffer_flags
 * variable contains more information about the buffer in the form of
 * (up to 32) bit flags.
 * The bit flags and their meaning are defined by each SCMI system separately.
 * For a list of buffer bit flags, see the respective system's documentation.
 */

/*Be careful of machine int width bits(32/64)*/
typedef struct _scmi_buffer {
	uint32_t p_next;    /**< Pointer used by module to chain SCMI-buffers */
	uint32_t p_address; /**< Address of the actual buffer */
	uint32_t base_address; /**< HW address of the actual buffer */
	buf_identity buf_id;
	uint32_t size;	       /**< Size of the buffer in bytes */
	uint32_t buffer_flags; /**< Generic buffer flags */
	int64_t time_stamp;   /**< Time stamp of the buffer in ticks with 27 MHz
				 resolution */
	uint32_t p_meta_data; /**< Optional pointer to buffer meta data */
} scmi_buffer;

/*****************************************************************************/
/**
 *          pic_buf_plane_t
 *
 * @brief   Common information about a color component plane within an image
 *buffer.
 *
 *****************************************************************************/
typedef struct pic_buf_plane_s {
	uint32_t p_data;
	uint32_t base_address;
	uint32_t pic_width_pixel;
	uint32_t pic_width_bytes;
	uint32_t pic_height_pixel;
	uint8_t pixel_data_align_mode;
	uint8_t bit_width;
} pic_buf_plane_t;

#define PIC_EXP_NUM_MAX 4 /**< Maximum exposure number of image*/

/*****************************************************************************/
/**
 *          pic_buf_metadata_info_t
 *
 * @brief  image all metadata info.
 *
 *****************************************************************************/
typedef struct pic_buf_metadata_info_s {
	uint64_t frame_count;
	uint64_t timestamp_sof; // timestamp for start of the frame
	uint64_t timestamp_eof; // timestamp for end of the frame
	float sensor_gain[PIC_EXP_NUM_MAX];
	/**< In linear mode or native HDR mode:\n sensor_gain[0] is image gain\n
	 In stitch HDR mode:\n
	    sensor_gain[0]: L image gain\n
	    sensor_gain[1]: S image gain\n
	    sensor_gain[2]: VS image gain\n
	    sensor_gain[3]: ES image gain */
	float expo_info[PIC_EXP_NUM_MAX]; // us
	/**< In linear mode or native HDR mode:\n expo_info[0] is image
	 integration time\n In stitch HDR mode:\n expo_info[0]: L image
	 integration time\n expo_info[1]: S image integration time\n
	 expo_info[2]: VS image integration time\n
	 expo_info[3]: ES image integration time */
} pic_buf_metadata_info_t;

#define METADATA_MAX_NUM 3

/*****************************************************************************/
/**
 *          metadata_buf_info_t
 *
 * @brief  metadata windows buffer info.
 *
 *****************************************************************************/
typedef struct metadata_buf_info_s {
	uint8_t win_num;
	uint32_t p_buffer[METADATA_MAX_NUM];
	uint32_t address[METADATA_MAX_NUM];
	uint32_t buffer_size[METADATA_MAX_NUM];
} metadata_buf_info_t;

/*****************************************************************************/
/**
 *          RDC compress information
 *
 * @brief  compress information for buffer decompress.
 *
 *****************************************************************************/
typedef struct pic_buf_cmp_info_e {
	bool_t compressed;	       /**<buffer compressed*/
	pic_buf_loss_mode_t loss_mode; /**<loss mode*/
	pic_buf_rdce_type_t bitdepth;  /**<bit depth*/
	short target_size;	       /**<target size */
	short bit_thresh;
	pic_buf_rdce_bayer_pat_t bayer_pattern;
	uint32_t crc_value; /**<vi200 fusa crc */
} pic_buf_cmp_info_t;

/*****************************************************************************/
/**
 *          pic_buf_meta_data_t
 *
 * @brief   All the meta data one needs to know about an image buffer.
 *
 *****************************************************************************/
typedef struct pic_buf_meta_data_s {
	pic_buf_type_t type;	 // type of picture data
	pic_buf_layout_t layout; // kind of data layout
	uint32_t align; // min. alignment required for color component planes or
			// sub buffer base adresses for this picture buffer
	int64_t time_stamp_us; // timestamp in us
	uint32_t
	    p_next3_d; // Reference to PicBufMetaData of the subsequent buffer
		       // in a 3D descriptor chain, valid only in 3D mode; set
		       // to NULL if last in chain or for 2D mode. Note:
		       // depending on the 3D format in use, the primary buffer
		       // holds left image data while the secondary buffer holds
		       // right or depth information.
		       //       Certain 3D formats require further buffers, in
		       //       which case the 3D chain consists of more than
		       //       two descriptors.
	//  buf_identity             buf_id;
	pic_buf_cmp_info_t
	    compress_info; /**< comperss information for decompress*/
	pic_buf_metadata_info_t meta_info;
	pic_buf_yuv_order_t yuv_order;
	pic_buf_mi_swap_t swap; // MI output data swap
	union Data_u {		// the type and layout dependent meta data
		struct data_s { // DATA
			uint32_t p_data;
			uint32_t base_address;
			uint32_t data_size;
		} data;

		struct data_meta { // meta
			uint32_t p_data;
			uint32_t base_address;
			uint32_t data_size;

			pic_buf_plane_t
			    plane[METADATA_MAX_NUM]; // meta data windows number
						     // 3
			metadata_buf_info_t meta_buf_info;
		} meta;

		pic_buf_plane_t raw; // RAW8, RAW16

		struct jpeg_s { // JPEG
			uint32_t p_header;
			uint32_t header_size;
			uint32_t p_data;
			uint32_t base_address;
			uint32_t data_size;
		} jpeg;

		union YCbCr_u { // YCbCr444, YCbCr422, YCbCr420, YCbCr32
			pic_buf_plane_t combined;
			struct semiplanar_s {
				pic_buf_plane_t y;
				pic_buf_plane_t cb_cr;
				uint8_t bit_width;
			} semiplanar;
			struct planar_YUV_s {
				pic_buf_plane_t y;
				pic_buf_plane_t cb;
				pic_buf_plane_t cr;
			} planar;
		} y_cb_cr;

		union RGB_u { // RGB888, RGB32
			pic_buf_plane_t combined;
			struct planar_RGB_s {
				pic_buf_plane_t r;
				pic_buf_plane_t g;
				pic_buf_plane_t b;
			} planar;
		} rgb;
#ifdef ISP_MI_BP
		union BAYER_u { // rggb bggr
			struct planar_BAYER_s {
				pic_buf_plane_t r;
				pic_buf_plane_t Gr;
				pic_buf_plane_t Gb;
				pic_buf_plane_t b;
			} planar;
		} BAYER;
#endif
	} data;
} pic_buf_meta_data_t;

/**
 *
 * @brief The MediaBufferPool holds elements from type media_buffer_t.
 */
typedef struct media_buffer_s {
	uint32_t base_address; /**< HW address of system memory buffer. */
	uint32_t base_size;    /**< Base size of buffer. */
	uint32_t lock_count;   /**< Counting how many times buffer is used. 0
				  means buffer is free. */
	uint32_t p_owner;      /**< The buffer management context to which media
				  buffer belongs */
	bool_t is_full; /**< Flag set to TRUE when buffer is put in queue as a
			   full buffer */
	pic_buf_meta_data_t
	    *p_meta_data;	/**< Pointer to optional meta data structure. */
	uint8_t index;		/**< The index in buffer management context */
	buff_mode buf_mode;	/**< The memory type of this media buffer */
	uint32_t p_ipl_address; /**< The virtual address of this buffer in ISP
				   platform.*/
#ifdef WITH_FLEXA
	int fd;
	uint32_t mobj;
#endif
	scmi_buffer buf; /**< Common SCMI buffer type. not use.TODO delete */

} media_buffer_t;

#endif // __CAMERA_DEVICE_BUF_DEFS_H__
