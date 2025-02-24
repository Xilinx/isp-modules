/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2022 Vivante Corporation
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
 ****************************************************************************/

#ifndef __CAMDEV_COMMON_H__
#define __CAMDEV_COMMON_H__

#include <common/return_codes.h>
#include <ebase/types.h>
//#include "mailbox.h"

/**
 * @defgroup cam_device_common Cam Device Common Definitions
 * @{
 *
 *
 */

#define CAMDEV_ISO_STRENGTH_NUM 20 /**< ISO strength number */

#define CAMDEV_VIRTUAL_ID_MAX \
	16 /**< Platform supported multiple input devices number of one ISP*/
#define CAMDEV_HARDWARE_ID_MAX 5 /**< Platform supported ISP hardware number*/

#define CAMDEV_INPUT_DEV_NAME_LEN 20  /**< Input device name length */
#define CAM_DEVICE_ROI_WINDOWS_MAX 25 /**< Maximum ROI windows */

#define CAMDEV_HIST_BINS_NUM_MAX 256 /**< Maximum histogram  bins */
#define CAMDEV_BETCH_MODE_NUM_MAX \
	32 /**< Maximum betch mode support frame number */

#ifndef REFSET
#define REFSET(_DST_REF_, _VAL_) memset(&_DST_REF_, _VAL_, sizeof(_DST_REF_))
#endif

#ifndef REFCPY
#define REFCPY(_DST_REF_, _SRC_) memcpy(&_DST_REF_, _SRC_, sizeof(_DST_REF_))
#endif

/*****************************************************************************/
/**
 * @brief   Cam Device software work status.
 *
 *****************************************************************************/
typedef enum CamDeviceStatus_e
{
	CAMDEV_INVALID = 0, /**< Un-intialized */
	CAMDEV_INIT,		/**< Intialized */
	CAMDEV_IDLE,		/**< Idle */
	CAMDEV_RUNNING,		/**< Running */
	DUMMY_CAMDEV_0004 = 0xdeadfeed
} CamDeviceStatus_t;

/*****************************************************************************/
/**
 * @brief   Cam Device submodules parameters configure mode.
 *
 *****************************************************************************/
typedef enum CamDeviceConfigMode_e
{
	CAMDEV_CFG_MODE_MANUAL, /**< Manual mode */
	CAMDEV_CFG_MODE_AUTO,	/**< Auto mode */
	CAMDEV_CFG_MODE_MAX,
	DUMMY_CAMDEV_0005 = 0xdeadfeed
} CamDeviceConfigMode_t;

/*****************************************************************************/
/**
 * @brief   Cam Device snapshot image type.
 *
 *****************************************************************************/
typedef enum CamDeviceSnapshotType_s
{
	CAMDEV_SNAPSHOT_RGB = 0, /**< RGB */
	CAMDEV_SNAPSHOT_RAW8,	 /**< RAW8 */
	CAMDEV_SNAPSHOT_RAW12,	 /**< RAW12 */
							 //  CAMDEV_SNAPSHOT_JPEG,
	CAMDEV_SNAPSHOT_RAW10,	 /**< RAW10 */
	DUMMY_CAMDEV_0006 = 0xdeadfeed
} CamDeviceSnapshotType_t;

/*****************************************************************************/
/**
 * @brief Cam Device lock types for the auto algorithms.
 *        Can be OR combined.
 *
 *****************************************************************************/
typedef enum CamDeviceLockType_e
{
	CAMDEV_LOCK_NO = 0x00U,	 /**< Unlock */
	CAMDEV_LOCK_AF = 0x01U,	 /**< Auto-focus lock*/
	CAMDEV_LOCK_AEC = 0x02U, /**< Auto-exposure-control lock */
	CAMDEV_LOCK_AWB = 0x04U, /**< Auto-white-balance lock */
	CAMDEV_LOCK_ALL = 0x07U, /**< AF&AWB&AE lock 0x00| 0x01 | 0x02 | 0x04*/
	DUMMY_CAMDEV_0007 = 0xdeadfeed
} CamDeviceLockType_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the work mode of one input device.
 *
 *****************************************************************************/
typedef enum CamDeviceWorkMode_e
{
	CAMDEV_WORK_MODE_INVALID = 0, /**< Invalid work mode*/
	CAMDEV_WORK_MODE_STREAM, /**< Stream mode: input device connects ISP pipeline directly*/
	CAMDEV_WORK_MODE_MCM,	/**< MCM mode: input device connects MCM. */
	CAMDEV_WORK_MODE_RDMA,	/**< RDMA mode: input from DMA buffer */
	CAMDEV_WORK_MODE_BATCH, /**< Batch mode: process a group of continuous frame data at one time, suitable for high frame rate scenes. */
	CAMDEV_WORK_MODE_MAX,	/**< Maximum work mode*/
	DUMMY_CAMDEV_0008 = 0xdeadfeed
} CamDeviceWorkMode_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the input type of ISP pipeline.
 *
 *****************************************************************************/
typedef enum CamDeviceInputType_e
{
	CAMDEV_INPUT_TYPE_INVALID = 0, /**< Invalid input type. */
	CAMDEV_INPUT_TYPE_SENSOR,	   /**< Sensor input */
	CAMDEV_INPUT_TYPE_IMAGE,	   /**< Image buffer input */
	CAMDEV_INPUT_TYPE_TPG,		   /**< ISP TPG buffer input */
	CAMDEV_INPUT_TYPE_MAX,		   /**< Maximum input type. */
	DUMMY_CAMDEV_0009 = 0xdeadfeed
} CamDeviceInputType_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the output type of ISP pipeline.
 *
 *****************************************************************************/
typedef enum CamDeviceOutputType_e
{
	CAMDEV_OUTPUT_TYPE_INVALID = 0, /**< Invalid output type. */
	CAMDEV_OUTPUT_TYPE_MEMORY, /**< ISP hardware output image into extra memory. */
	CAMDEV_OUTPUT_TYPE_ONLINE, /**< ISP hardware output image into next process module directly.*/
	CAMDEV_OUTPUT_TYPE_BOTH, /**< ISP hardware bpth output image into memory and next process module. */
	CAMDEV_OUTPUT_TYPE_MAX, /**< Maximum output type. */
	DUMMY_CAMDEV_0010 = 0xdeadfeed
} CamDeviceOutputType_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to specify the swap control.
 *
 *****************************************************************************/
typedef enum CamDeviceMiSwapType_e
{
	CAMDEV_MI_SWAP_INVALID =
		-1,				   /**< (For internal evaluation only) Lower border  */
	CAMDEV_MI_NO_SWAP = 0, /**< No swapping. */
	CAMDEV_MI_SWAP_BYTES = 1,		 /**< Byte swapping. */
	CAMDEV_MI_SWAP_WORDS = 2,		 /**< Words swapping. */
	CAMDEV_MI_SWAP_DOUBLE_WORDS = 4, /**< Double-word swapping. */
	CAMDEV_MI_SWAP_FOUR_WORDS = 8,	 /**< Four-word swapping. */
	CAMDEV_MI_SWAP_MAX, /**< (For internal evaluation only) Upper border */
	DUMMY_CAMDEV_0011 = 0xdeadfeed
} CamDeviceMiSwapType_t;

/*****************************************************************************/
/**
 * @brief   Union type to specify the RAW and YUV swap mode.
 *
 *****************************************************************************/
typedef union CamDeviceMiSwap_s
{
	struct
	{
		CamDeviceMiSwapType_t y;   /**< Swap type of Y channel. */
		CamDeviceMiSwapType_t u;   /**< Swap type of U channel. */
		CamDeviceMiSwapType_t v;   /**< Swap type of V channel. */
	} yuvSwap;					   /**< The swap type of YUV format. */
	CamDeviceMiSwapType_t rawSwap; /**< Swap type of RAW format. */
} CamDeviceMiSwap_u;

/*****************************************************************************/
/**
 * @brief   Enumeration type to specify the order of YUV or RGB channel
 *          Right now only surpport RGB888 format
 *
 *****************************************************************************/
typedef enum CamDeviceMiYuvOrder_e
{
	CAM_DEVICE_MI_PIC_CHANNEL_ORDER_INVALID = -1,
	CAM_DEVICE_MI_PIC_CHANNEL_ORDER_YUV = 0, /** 0: YUV or RGB */
	CAM_DEVICE_MI_PIC_CHANNEL_ORDER_YVU = 1, /** 1: YVU or RBG */
	CAM_DEVICE_MI_PIC_CHANNEL_ORDER_UYV = 2, /** 2: UYV or GRB */
	CAM_DEVICE_MI_PIC_CHANNEL_ORDER_VYU = 3, /** 3: VYU or BRG */
	CAM_DEVICE_MI_PIC_CHANNEL_ORDER_UVY = 4, /** 4: UVY or GBR */
	CAM_DEVICE_MI_PIC_CHANNEL_ORDER_VUY = 5, /** 5: VUY or BGR */
	CAM_DEVICE_MI_PIC_CHANNEL_ORDER_MAX = 6,
	DUMMY_CAMDEV_0012 = 0xdeadfeed
} CamDeviceMiYuvOrder_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the tile operation mode for tile mode.
 *
 *****************************************************************************/
typedef enum CamDeviceTileOperation_e
{
	CAMDEV_TILE_OP_GENERAL, /**< Not tile mode.*/
	CAMDEV_TILE_OP_SW,		/**< SW split image operation.*/
	CAMDEV_TILE_OP_HW, /**< HW split image operation(need HW mp_ctrl support).*/
	CAMDEV_TILE_OP_MAX, /**< Maximum tile operation mode.*/
	DUMMY_CAMDEV_0013 = 0xdeadfeed
} CamDeviceTileOperation_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the tile joint mode.
 *
 *****************************************************************************/
typedef enum CamDeviceTileJoint_e
{
	CAMDEV_TILE_JOINT_INVALID = 0, /**< Invalid tile number.*/
	CAMDEV_TILE_JOINT_2X1,		   /**< Tile number 2x1.*/
	CAMDEV_TILE_JOINT_4X3,		   /**< Tile number 4X3.*/
	CAMDEV_TILE_JOINT_USER, /**< User defined joint in x axis and y axis*/
	CAMDEV_TILE_JOINT_MAX,	/**< Maximum tile number.*/
	DUMMY_CAMDEV_0014 = 0xdeadfeed
} CamDeviceTileJoint_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the tile x axis when joint mode is user-defined(CAMDEV_TILE_JOINT_USER).
 *
 *****************************************************************************/
typedef enum CamDeviceTileXAxis_e
{
	CAMDEV_TILE_X_INVALID = 0, /**< Invalid x axis.*/
	CAMDEV_TILE_X_AXIS_1,	   /**< Tile x axis 1.*/
	CAMDEV_TILE_X_AXIS_2,	   /**< Tile x axis 2.*/
	CAMDEV_TILE_X_AXIS_3,	   /**< Tile x axis 3.*/
	CAMDEV_TILE_X_AXIS_4,	   /**< Tile x axis 4.*/
	DUMMY_CAMDEV_0015 = 0xdeadfeed
} CamDeviceTileXAxis_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the tile y axis when joint mode is user-defined(CAMDEV_TILE_JOINT_USER).
 *
 *****************************************************************************/
typedef enum CamDeviceTileYAxis_e
{
	CAMDEV_TILE_Y_INVALID = 0, /**< Invalid y axis.*/
	CAMDEV_TILE_Y_AXIS_1,	   /**< Tile y axis 1.*/
	CAMDEV_TILE_Y_AXIS_2,	   /**< Tile y axis 2.*/
	CAMDEV_TILE_Y_AXIS_3,	   /**< Tile y axis 3.*/
	DUMMY_CAMDEV_0016 = 0xdeadfeed
} CamDeviceTileYAxis_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the MCM port index and the VI200 adapter port index.
 *
 *****************************************************************************/
typedef enum CamDeviceMcmPortId_e
{

	CAMDEV_MCM_PORT_INVALID = 0, /**< Invalid video in MCM invalid port index.*/
	CAMDEV_MCM_PORT_0,			 /**< Video input MCM port index 0.*/
	CAMDEV_MCM_PORT_1,			 /**< Video input MCM port index 1.*/
	CAMDEV_MCM_PORT_2,			 /**< Video input MCM port index 2.*/
	CAMDEV_MCM_PORT_3,			 /**< Video input MCM port index 3.*/
	CAMDEV_MCM_PORT_MAX,		 /**< Video input MCM max port index.*/
	CAMDEV_VI200_ADAPT_PORT_INVALID =
		0,						 /**< Invalid VI200 adapter port index.*/
	CAMDEV_VI200_ADAPT_PORT_0,	 /**< VI200 adapter port index 0.*/
	CAMDEV_VI200_ADAPT_PORT_1,	 /**< VI200 adapter port index 1.*/
	CAMDEV_VI200_ADAPT_PORT_2,	 /**< VI200 adapter port index 2.*/
	CAMDEV_VI200_ADAPT_PORT_3,	 /**< VI200 adapter port index 3.*/
	CAMDEV_VI200_ADAPT_PORT_4,	 /**< VI200 adapter port index 4.*/
	CAMDEV_VI200_ADAPT_PORT_5,	 /**< VI200 adapter port index 5.*/
	CAMDEV_VI200_ADAPT_PORT_6,	 /**< VI200 adapter port index 6.*/
	CAMDEV_VI200_ADAPT_PORT_7,	 /**< VI200 adapter port index 7.*/
	CAMDEV_VI200_ADAPT_PORT_8,	 /**< VI200 adapter port index 8.*/
	CAMDEV_VI200_ADAPT_PORT_9,	 /**< VI200 adapter port index 9.*/
	CAMDEV_VI200_ADAPT_PORT_10,	 /**< VI200 adapter port index 10.*/
	CAMDEV_VI200_ADAPT_PORT_11,	 /**< VI200 adapter port index 11.*/
	CAMDEV_VI200_ADAPT_PORT_12,	 /**< VI200 adapter port index 12.*/
	CAMDEV_VI200_ADAPT_PORT_13,	 /**< VI200 adapter port index 13.*/
	CAMDEV_VI200_ADAPT_PORT_14,	 /**< VI200 adapter port index 14.*/
	CAMDEV_VI200_ADAPT_PORT_15,	 /**< VI200 adapter port index 15.*/
	CAMDEV_VI200_ADAPT_PORT_MAX, /**< VI200 adapter max port index.*/
	DUMMY_CAMDEV_0017 = 0xdeadfeed
} CamDeviceMcmPortId_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the MCM port index for stream mode.
 *
 *****************************************************************************/
typedef CamDeviceMcmPortId_t CamDeviceStreamPortId_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the MCM operation mode for MCM mode.
 *
 *****************************************************************************/
typedef enum CamDeviceMcmOperation_e
{
	CAMDEV_MCM_OP_INVALID = 0, /**< Invalid MCM operation mode.*/
	CAMDEV_MCM_OP_SW,		   /**< Software MCM operation mode.*/
	CAMDEV_MCM_OP_HW,		   /**< Hardware MCM operation mode.*/
	CAMDEV_MCM_OP_MAX,		   /**< Maximum MCM operation mode.*/
	DUMMY_CAMDEV_0018 = 0xdeadfeed
} CamDeviceMcmOperation_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the MCM selection for MCM mode.
 *
 *****************************************************************************/
typedef enum CamDeviceMcmSelection_e
{
	CAMDEV_MCM_SEL_INVALID = -1, /**< Invalid MCM selection.*/
	CAMDEV_MCM_SEL_RDMA,		 /**< MCM RDMA.*/
	CAMDEV_MCM_SEL_RDCD,		 /**< MCM RDCD.*/
	CAMDEV_MCM_SEL_MAX,			 /**< Maximum MCM selection.*/
	DUMMY_CAMDEV_0019 = 0xdeadfeed
} CamDeviceMcmSelection_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the sequence mode of switch control.
 *
 *****************************************************************************/
typedef enum CamDeviceSwitchSeqMode_e
{
	CAMDEV_SEQ_MODE_INVALID = 0, /**< Sequence invalid mode.*/
	CAMDEV_SEQ_MODE_PRIORITY, /**< Sequence priority mode. The switch control layer will schedule
								   each camera device according to the sequence priority.*/
	CAMDEV_SEQ_MODE_BATCH, /**< Sequence batch mode. The switch control layer will schedule
								each camera device according to the batch sequence.*/
	CAMDEV_SEQ_MODE_MAX, /**< Sequence max mode.*/
	DUMMY_CAMDEV_0020 = 0xdeadfeed
} CamDeviceSwitchSeqMode_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the sequence priority in switch control.
 *
 *****************************************************************************/
typedef enum CamDeviceSwitchSeqPriority_e
{
	CAMDEV_SEQ_PRI_0,	/**< Sequence priority 0. Highest priority.*/
	CAMDEV_SEQ_PRI_1,	/**< Sequence priority 1.*/
	CAMDEV_SEQ_PRI_2,	/**< Sequence priority 2.*/
	CAMDEV_SEQ_PRI_3,	/**< Sequence priority 3.*/
	CAMDEV_SEQ_PRI_4,	/**< Sequence priority 4.*/
	CAMDEV_SEQ_PRI_5,	/**< Sequence priority 5.*/
	CAMDEV_SEQ_PRI_6,	/**< Sequence priority 6.*/
	CAMDEV_SEQ_PRI_7,	/**< Sequence priority 7.*/
	CAMDEV_SEQ_PRI_8,	/**< Sequence priority 8.*/
	CAMDEV_SEQ_PRI_9,	/**< Sequence priority 9.*/
	CAMDEV_SEQ_PRI_10,	/**< Sequence priority 10.*/
	CAMDEV_SEQ_PRI_11,	/**< Sequence priority 11.*/
	CAMDEV_SEQ_PRI_12,	/**< Sequence priority 12.*/
	CAMDEV_SEQ_PRI_13,	/**< Sequence priority 13.*/
	CAMDEV_SEQ_PRI_14,	/**< Sequence priority 14.*/
	CAMDEV_SEQ_PRI_15,	/**< Sequence priority 15. Lowest priority.*/
	CAMDEV_SEQ_PRI_MAX, /**< Sequence priority max.*/
	DUMMY_CAMDEV_0021 = 0xdeadfeed
} CamDeviceSwitchSeqPriority_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure the output path channel.
 *
 *****************************************************************************/
typedef enum CamDevicePipeOutPathType_e
{
	CAMDEV_PIPE_OUTPATH_INVALID = -1,
	CAMDEV_PIPE_OUTPATH_MP = 0U,  /**< Main path of ISP hardware pipeline */
	CAMDEV_PIPE_OUTPATH_SP1 = 1U, /**< Self path 1 of ISP hardware pipeline */
	CAMDEV_PIPE_OUTPATH_SP2 = 2U, /**< Self path 2 of ISP hardware pipeline */
	CAMDEV_PIPE_OUTPATH_RAW = 3U, /**< Main path for RAW image output */
	CAMDEV_PIPE_OUTPATH_HDR_RAW =
		4U,					 /**< Retiming HDR path for RAW image output */
	CAMDEV_PIPE_OUTPATH_MAX, /**< Total number of output paths */
	DUMMY_CAMDEV_0022 = 0xdeadfeed
} CamDevicePipeOutPathType_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure ISP output path enable mask.
 *
 *****************************************************************************/
#define CAMDEV_OUT_MP_PATH_MASK 0x1U  /**< The output mask for main path */
#define CAMDEV_OUT_SP1_PATH_MASK 0x2U /**< The output mask for self path 1 */
#define CAMDEV_OUT_SP2_PATH_MASK 0x4U /**< The output mask for self path 2 */
#define CAMDEV_OUT_RAW_PATH_MASK 0x8U /**< The output mask for raw path */
#define CAMDEV_OUT_HDR_RAW_PATH_MASK \
	0x10U /**< The output mask for HDR raw path */

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure input path channel.
 *
 *****************************************************************************/
typedef enum CamDevicePipeInPathType_e
{
	CAMDEV_PIPE_INPATH_RDMA = 0,	 /**< ISP input read DMA path */
	CAMDEV_PIPE_INPATH_RETIMING = 1, /**< ISP input HDR retiming path */
	CAMDEV_PIPE_INPATH_TPG = 2,		 /**< ISP input TPG path */
	CAMDEV_PIPE_INPATH_MAX,			 /**< Total number of input paths */
	DUMMY_CAMDEV_0023 = 0xdeadfeed
} CamDevicePipeInPathType_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure bit depth of input pixel.
 *
 *****************************************************************************/
typedef enum CamDeviceBitDepth_e
{
	CAMDEV_BIT_DEPTH_8BIT = 0,	/**< 8-bit */
	CAMDEV_BIT_DEPTH_10BIT = 1, /**< 10-bit */
	CAMDEV_BIT_DEPTH_12BIT = 2, /**< 12-bit */
	CAMDEV_BIT_DEPTH_BIT_MAX,	/**< The maximum input bit depth */
	DUMMY_CAMDEV_0024 = 0xdeadfeed
} CamDeviceBitDepth_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure format of output pixel.
 *
 *****************************************************************************/
typedef enum CamDevicePipePixOutFmt_e
{
	CAMDEV_PIX_FMT_YUV422SP = 0, /**< ISP output format: YUV422 Semi-Planar */
	CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE0, /**< ISP output format: YUV422 Semi-Planar in align mode 0*/
	CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE1, /**< ISP output format: YUV422 Semi-Planar in align mode 1*/
	CAMDEV_PIX_FMT_YUV422I, /**< ISP output format: YUV422 Interleaved */
	CAMDEV_PIX_FMT_YUV422I_ALIGNED_MODE1, /**< ISP output format: YUV422 Interleaved in align mode 1*/
	CAMDEV_PIX_FMT_YUV420SP, /**< ISP output format: YUV420 Semi-Planar */
	CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE0, /**< ISP output format: YUV420 Semi-Planar in align mode 0*/
	CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE1, /**< ISP output format: YUV420 Semi-Planar in align mode 1*/
	CAMDEV_PIX_FMT_YUV444P, /**< ISP output format: YUV444 Planar */
	CAMDEV_PIX_FMT_YUV444I, /**< ISP output format: YUV444 Interleaved */
	CAMDEV_PIX_FMT_YUV444I_ALIGNED_MODE0, /**< ISP output format: YUV444 Interleaved in align mode 0*/
	CAMDEV_PIX_FMT_YUV400, /**< ISP output format: YUV400 Y only format */
	CAMDEV_PIX_FMT_YUV400_ALIGNED_MODE0, /**< ISP output format: YUV400 Y only format in align mode 0*/
	CAMDEV_PIX_FMT_YUV400_ALIGNED_MODE1, /**< ISP output format: YUV400 Y only format in align mode 1*/
	CAMDEV_PIX_FMT_RGB888, /**< ISP output format: RGB888 Raster Scan*/
	CAMDEV_PIX_FMT_RGB888_ALIGNED_MODE0, /**< ISP output format: RGB888 Raster Scan in align mode 0*/
	CAMDEV_PIX_FMT_RGB888P, /**< ISP output format: RGB888 Planar */
	CAMDEV_PIX_FMT_RAW8,	/**< ISP output format: Raw 8-bit */
	CAMDEV_PIX_FMT_RAW10,	/**< ISP output format: Raw 10-bit */
	CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE0, /**< ISP output format: Raw 10-bit in align mode 0 */
	CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE1, /**< ISP output format: Raw 10-bit in align mode 1 */
	CAMDEV_PIX_FMT_RAW12,				/**< ISP output format: Raw 12-bit */
	CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE0, /**< ISP output format: Raw 12-bit in align mode 0 */
	CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE1, /**< ISP output format: Raw 12-bit in align mode 1 */
	CAMDEV_PIX_FMT_RAW14,				/**< ISP output format: Raw 14-bit */
	CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE0, /**< ISP output format: Raw 14-bit in align mode 0 */
	CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE1, /**< ISP output format: Raw 14-bit in align mode 1 */
	CAMDEV_PIX_FMT_RAW16,				/**< ISP output format: Raw 16-bit */
	CAMDEV_PIX_FMT_RAW24,				/**< ISP output format: Raw 24-bit */
	CAMDEV_PIX_FMT_MAX,					/**< Total number of output formats */
	DUMMY_CAMDEV_0025 = 0xdeadfeed
} CamDevicePipePixOutFmt_t;

/*****************************************************************************/
/**
 * @brief   Cam Device input RAW format.
 *
 *****************************************************************************/
typedef enum CamDeviceInputRawFmt_e
{
	CAMDEV_INPUT_FMT_RAW8 = 0,			 /**< RAW8 */
	CAMDEV_INPUT_FMT_RAW10 = 1,			 /**< RAW10 unaligned*/
	CAMDEV_INPUT_FMT_RAW10_ALIGNED0 = 2, /**< RAW10 align double word */
	CAMDEV_INPUT_FMT_RAW10_ALIGNED1 = 3, /**< RAW10 align 16-bit */
	CAMDEV_INPUT_FMT_RAW12 = 4,			 /**< RAW12 unaligned */
	CAMDEV_INPUT_FMT_RAW12_ALIGNED0 = 5, /**< RAW12 align double word */
	CAMDEV_INPUT_FMT_RAW12_ALIGNED1 = 6, /**< RAW12 align 16-bit */
	CAMDEV_INPUT_FMT_RAW14 = 7,			 /**< RAW14 unaligned*/
	CAMDEV_INPUT_FMT_RAW14_ALIGNED0 = 8, /**< RAW14 align double word */
	CAMDEV_INPUT_FMT_RAW14_ALIGNED1 = 9, /**< RAW14 align 16-bit */
	CAMDEV_INPUT_FMT_RAW16 = 10,		 /**< RAW16 */
	CAMDEV_INPUT_FMT_RAW20_COMPRESS =
		11, /**< RAW20 compressed, only for 8200/8000 HDR mode. */
	CAMDEV_INPUT_FMT_RAW24 = 12, /**< RAW24, only for 8200 HDR mode. */
	CAMDEV_INPUT_FMT_RAW24_COMPRESS =
		13, /**< RAW24 compressed, only for 8200 HDR mode. */
	CAMDEV_INPUT_FMT_2DOL = 14, /**< HDR 2DOL image raw12 */
	CAMDEV_INPUT_FMT_3DOL = 15, /**< HDR 3DOL image raw12 */
	CAMDEV_INPUT_FMT_4DOL = 16, /**< HDR 4DOL image raw12 */
	CAMDEV_INPUT_FMT_MAX,
	DUMMY_CAMDEV_0026 = 0xdeadfeed
} CamDeviceInputRawFmt_t;

/*****************************************************************************/
/**
 * @brief   Cam Device HDR stitching mode.
 *
 *****************************************************************************/
typedef enum CamDeviceStitchingMode_e
{
	CAMDEV_STITCHING_DUAL_DCG = 0, /**< Dual DCG<SUP>TM</SUP> mode 3x12-bit */
	CAMDEV_STITCHING_3DOL = 1,	   /**< DOL3 frame 3x12-bit */
	CAMDEV_STITCHING_LINEBYLINE =
		2, /**< 3x12-bit line by line without waiting */
	CAMDEV_STITCHING_16BIT_COMPRESS =
		3, /**< 16-bit compressed data + 12-bit RAW */
	CAMDEV_STITCHING_DUAL_DCG_NOWAIT =
		4,						  /**< 2x12-bit dual DCG without waiting */
	CAMDEV_STITCHING_2DOL = 5,	  /**< DOL2 frame or 1 CG+VS sx12-bit RAW */
	CAMDEV_STITCHING_L_AND_S = 6, /**< L+S 2x12-bit RAW */
	CAMDEV_STITCHING_4DOL = 7,	  /**< DOL4 frame 3x12-bit */
	CAMDEV_STITCHING_MAX,
	DUMMY_CAMDEV_0027 = 0xdeadfeed
} CamDeviceStitchingMode_t;

/*****************************************************************************/
/**
 * @brief   Cam Device HDR path index.
 *
 *****************************************************************************/
typedef enum CamDeviceStitchingPath_e
{
	CAMDEV_STITCHING_PATH_L = 0,  /**< HDR stitch path L  */
	CAMDEV_STITCHING_PATH_S = 1,  /**< HDR stitch path S  */
	CAMDEV_STITCHING_PATH_VS = 2, /**< HDR stitch path VS */
	CAMDEV_STITCHING_PATH_XS = 3, /**< HDR stitch path XS */
	CAMDEV_STITCHING_PATH_MAX,
	DUMMY_CAMDEV_0028 = 0xdeadfeed
} CamDeviceStitchingPath_t;

/*****************************************************************************/
/**
 * @brief   Cam Device RAW Bayer pattern.
 *
 *****************************************************************************/
typedef enum CamDeviceRawPattern_e
{
	CAMDEV_RAW_RGB_PAT_RGGB = 0,	/**< RGGB */
	CAMDEV_RAW_RGB_PAT_GRBG,		/**< GRBG */
	CAMDEV_RAW_RGB_PAT_GBRG,		/**< GBRG */
	CAMDEV_RAW_RGB_PAT_BGGR,		/**< BGGR */
	CAMDEV_RAW_RGB_PAT_MAX,			/**< Maximum RGB pattern */
	CAMDEV_RAW_RGBIR_PAT_BGGIR = 4, /**< BGGIR */
	CAMDEV_RAW_RGBIR_PAT_GRIRG,		/**< GRIRG */
	CAMDEV_RAW_RGBIR_PAT_RGGIR,		/**< RGGIR */
	CAMDEV_RAW_RGBIR_PAT_GBIRG,		/**< GBIRG */
	CAMDEV_RAW_RGBIR_PAT_GIRRG,		/**< GIRRG */
	CAMDEV_RAW_RGBIR_PAT_IRGGB,		/**< IRGGB */
	CAMDEV_RAW_RGBIR_PAT_GIRBG,		/**< GIRBG */
	CAMDEV_RAW_RGBIR_PAT_IRGGR,		/**< IRGGR */
	CAMDEV_RAW_RGBIR_PAT_RGIRB,		/**< RGIRB */
	CAMDEV_RAW_RGBIR_PAT_GRBIR,		/**< GRBIR */
	CAMDEV_RAW_RGBIR_PAT_IRBRG,		/**< GRBIR */
	CAMDEV_RAW_RGBIR_PAT_BIRGR,		/**< BIRGR */
	CAMDEV_RAW_RGBIR_PAT_BGIRR,		/**< BGIRR */
	CAMDEV_RAW_RGBIR_PAT_GBRIR,		/**< GBRIR */
	CAMDEV_RAW_RGBIR_PAT_IRRBG,		/**< IRRBG */
	CAMDEV_RAW_RGBIR_PAT_RIRGB,		/**< RIRGB */
	CAMDEV_RAW_RGBIR_PAT_RCCC,		/**< RCCC */
	CAMDEV_RAW_RGBIR_PAT_RCCB,		/**< RCCB */
	CAMDEV_RAW_RGBIR_PAT_RYYCY,		/**< RYYCY */
	CAMDEV_RAW_PAT_MAX,				/**< Maximum RGBIR pattern */
	DUMMY_CAMDEV_0029 = 0xdeadfeed
} CamDeviceRawPattern_t;

/*****************************************************************************/
/**
 * @brief   Cam Device RAW image color channel.
 */
/*****************************************************************************/
typedef enum CamDeviceRawColorCh_e
{
	CAMDEV_RAW_CHANNEL_RED = 0,	   /**< Red channel */
	CAMDEV_RAW_CHANNEL_GREENR = 1, /**< GreenR channel */
	CAMDEV_RAW_CHANNEL_GREENB = 2, /**< GreenB channel */
	CAMDEV_RAW_CHANNEL_BLUE = 3,   /**< Blue channel */
	CAMDEV_RAW_CHANNEL_NUM,
	DUMMY_CAMDEV_0030 = 0xdeadfeed
} CamDeviceRawColorCh_t;

/*****************************************************************************/
/**
 * @brief   Cam Device RGBIR image color channel.
 */
/*****************************************************************************/
typedef enum CamDeviceRgbirColorCh_e
{
	CAMDEV_RGBIR_CHANNEL_R = 0,	 /**< Red channel */
	CAMDEV_RGBIR_CHANNEL_G = 1,	 /**< Green channel */
	CAMDEV_RGBIR_CHANNEL_B = 2,	 /**< Blue channel */
	CAMDEV_RGBIR_CHANNEL_IR = 3, /**< IR channel */
	CAMDEV_RGBIR_CHANNEL_NUM,
	DUMMY_CAMDEV_0031 = 0xdeadfeed
} CamDeviceRgbirColorCh_t;

/*****************************************************************************/
/**
 * @brief   Cam Device exposure frame index.
 *
 *****************************************************************************/
typedef enum CamDeviceExposureFrameIndex_e
{

	CAMDEV_EXPOSURE_LINEAR_FRAME = 0,  /**< Linear frame exposure */
	CAMDEV_EXPOSURE_LONG_FRAME = 0,	   /**< Long frame exposure */
	CAMDEV_EXPOSURE_SHORT_FRAME,	   /**< Short frame exposure */
	CAMDEV_EXPOSURE_VERY_SHORT_FRAME,  /**< Very short frame exposure */
	CAMDEV_EXPOSURE_EXTRA_SHORT_FRAME, /**< Extremely short frame exposure */
	CAMDEV_EXPOSURE_FIFTH_SHORT_FRAME, /**< Fifth short frame exposure */
	CAMDEV_EXPOSURE_COMBINED_FRAME,	   /**< Combined frame exposure */
	CAMDEV_EXPOSURE_FRAME_MAX,
	DUMMY_CAMDEV_0032 = 0xdeadfeed
} CamDeviceExposureFrameIndex_t;

/******************************************************************************/
/**
 * @brief   Cam Device ae mode select.
 *
 *****************************************************************************/
typedef enum CamDeviceAeWorkMode_e
{
	CAMDEV_AE_WORK_MODE_VSI_CTRL = 0,	   /**< Choose VSI interal ae control*/
	CAMDEV_AE_WORK_MODE_METADATA_CTRL = 1, /**< Choose metadata ae control*/
	CAMDEV_AE_WORK_MODE_MAX,
	DUMMY_CAMDEV_0033 = 0xdeadfeed
} CamDeviceAeWorkMode_t;

/******************************************************************************/
/**
 * @brief   Cam Device awb mode select.
 *
 *****************************************************************************/
typedef enum CamDeviceAwbWorkMode_e
{
	CAMDEV_AWB_WORK_MODE_VSI_CTRL = 0, /**< Choose VSI interal awb control*/
	CAMDEV_AWB_WORK_MODE_METADATA_CTRL = 1, /**< Choose metadata awb control*/
	CAMDEV_AWB_WORK_MODE_MAX,
	DUMMY_CAMDEV_0034 = 0xdeadfeed
} CamDeviceAwbWorkMode_t;

/*****************************************************************************/
/**
 * @brief   Cam Device histogram bins.
 */
/*****************************************************************************/
typedef struct CamDeviceHistBins_s
{
	uint32_t binNum; /**< The number of bins */
	uint32_t bins
		[CAMDEV_HIST_BINS_NUM_MAX]; /**< The value that define the upper end points of bins */
} CamDeviceHistBins_t;

/*****************************************************************************/
/**
 * @brief   Cam Device sensor resolution structure.
 *
 *****************************************************************************/
typedef struct CamDeviceResolution_s
{
	uint16_t width;	 /**< Width */
	uint16_t height; /**< Height */
} CamDeviceResolution_t;

/*****************************************************************************/
/**
 * @brief   Cam Device ISP window structure.
 *
 *****************************************************************************/
typedef struct CamDeviceWindow_s
{
	uint16_t hOffset; /**< Horizontal start offset */
	uint16_t vOffset; /**< Vertical start offset */
	uint16_t width;	  /**< Width */
	uint16_t height;  /**< Height */
} CamDeviceWindow_t;

/*****************************************************************************/
/**
 * @brief   Cam Device ISP window structure.
 *
 *****************************************************************************/
typedef struct CamDeviceRoiWindow_s
{
	CamDeviceWindow_t window; /**< ROT window */
	float32_t weight;		  /**< Weight */
} CamDeviceRoiWindow_t;

/******************************************************************************/
/**
 * @brief   Cam Device region of interest configuration.
 *
 *****************************************************************************/
typedef struct CamDeviceRoi_s
{
	uint8_t roiNum;		 /**< Number of ROI window */
	float32_t roiWeight; /**< The weight of ROI; */
	CamDeviceRoiWindow_t
		roiWindow[CAM_DEVICE_ROI_WINDOWS_MAX]; /**< ROI windows */
} CamDeviceRoi_t;

/*****************************************************************************/
/**
 * @brief   CamDevice integer range information structure.
 *
 *****************************************************************************/
typedef struct CamDeviceIntegerRange_s
{
	uint32_t max;  /**< Maximum value*/
	uint32_t min;  /**< Minimum value*/
	uint32_t step; /**< Step value */
} CamDeviceIntegrationTimeRange_t;

typedef void* CamDeviceHandle_t;

/* @} cam_device_common */

#endif // __ISP_CAMDEV_COMMON_H__
