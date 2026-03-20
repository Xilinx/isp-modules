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

#ifndef __CAMDEV_API_H__
#define __CAMDEV_API_H__

#include "cam_device_calibration.h"
#include "cam_device_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CAMDEV_CC_MATRIX_SIZE       9U      /**< Color correction matrix size 3x3*/
#define CAMDEV_CC_COLOR_CHANNEL_NUM 3U      /**< Color channel number 3*/

/*****************************************************************************/
/**
 * @brief   Structure to configure MCM mode parameters.
 *
 *****************************************************************************/
typedef struct cam_device_mcm_mode_config_s {
	cam_device_mcm_port_id_t
	    port_id; /**< port index of MCM which indicates the sensor hardware
			connect position*/
	cam_device_mcm_operation_t mcm_op;  /**< MCM operation mode */
	cam_device_mcm_selection_t mcm_sel; /**< MCM selection mode */
	cam_device_mcm_reg_cfg_mode_t mcm_reg_cfg;/**< MCM register configuration mode */
} cam_device_mcm_mode_config_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure stream mode parameters.
 *
 *****************************************************************************/
typedef struct cam_device_stream_mode_config_s {
	cam_device_stream_port_id_t
	    port_id; /**< port index of MCM which indicates the sensor hardware
			connect position*/
} cam_device_stream_mode_config_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure tile mode parameters. Reserved.
 *
 *****************************************************************************/
typedef struct cam_device_tile_mode_config_s {
	cam_device_tile_operation_t tile_op; /**< Tile operation mode */
	cam_device_tile_joint_t tile_num;    /**< Tile number */
	cam_device_tile_x_axis_t
	    x_pices; /**< Tile joint mode set user, x direction tile number */
	cam_device_tile_y_axis_t
	    y_pices; /**< Tile joint mode set user, y direction tile number */
} cam_device_tile_mode_config_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure ISP work mode.
 *
 *****************************************************************************/
typedef union cam_device_mode_config_s {
	cam_device_stream_mode_config_t
	    stream; /**< Stream mode configuration parameters */
		    //    CamDeviceRdmaModeConfig_t   rdma;     /**< Reserved */
	cam_device_mcm_mode_config_t
	    mcm; /**< MCM mode configuration parameters */
} cam_device_mode_config_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure ISP input.
 *
 *****************************************************************************/
typedef struct cam_device_input_config_s {
	cam_device_input_type_t input_type;		/**< ISP input type */
	char input_dev_name[CAMDEV_INPUT_DEV_NAME_LEN]; /**< ISP input device
							   name */
} cam_device_input_config_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure ISP output.
 *
 *****************************************************************************/
typedef struct cam_device_output_config_s {
	cam_device_output_type_t output_type; /**< ISP output type */
} cam_device_output_config_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure work parameters for camera device.
 *
 *****************************************************************************/
typedef struct cam_device_work_config_s {
	cam_device_work_mode_t work_mode;  /**< ISP software work mode */
	cam_device_mode_config_t mode_cfg; /**< Work mode configurations */
	cam_device_tile_mode_config_t
	    tile_cfg; /**< Tile mode configurations. It can be used together
			 with other modes*/
} cam_device_work_config_t;

/*****************************************************************************/
/**
 * @brief    Structure to create camera device instance.
 *
 *****************************************************************************/
#if 1
typedef struct cam_device_config_s {
	uint32_t isp_hw_id; /**< Hardware Pipeline id. */
	cam_device_input_config_t
	    input_cfg; /**< Input device configuration parameters */
	cam_device_work_config_t
	    work_cfg; /**< ISP Work configuration parameters */
	cam_device_output_config_t
	    output_cfg; /**< ISP output configuration parameters */
	cam_device_switch_seq_priority_t
	    priority;	 /**< Input device priority in switch control */
	void *h_cascade; /**< Cascade ctx handle */
	struct visp_dev *isp_dev;
} cam_device_config_t;
#endif

/*****************************************************************************/
/**
 * @brief   Structure to set ISP pipeline output format.
 *
 *****************************************************************************/
typedef struct cam_device_pipe_out_fmt_s {
	uint32_t out_width;			  /**< width of output image */
	uint32_t out_height;			  /**< height of output image */
	uint32_t path_out_type;			  /**< path out type */
	cam_device_pipe_pix_out_fmt_t out_format; /**< format of output pixel */
	uint32_t data_bits; /**< data width of each color component [8~14] */
	uint8_t alpha; /* < Alpha value of ARGB format, range 0~255 for
			* ARGB/yuv8bit, range 0~3 for yuv10bit
			*/
	cam_device_mi_yuv_order_t yuv_order; /*< The difference order of the
					      * three yuv channel save in ddr
					      */
	cam_device_mi_swap_u
	    swap; /**< Output format swap control information */
	cam_device_mi_buf_offset_t buf_offset; /**< Output format buffer offset */
	cam_device_mi_buf_stride_t buf_stride; /**< Output format buffer stride */
} cam_device_pipe_out_fmt_t;

/*****************************************************************************/
/**
 * @brief   Structure to set ISP output streaming.
 *
 *****************************************************************************/
typedef struct cam_device_path_streaming_cfg_s {
	uint8_t out_path_enable; /**< Path streaming configuration */
} cam_device_path_streaming_cfg_t;

/*****************************************************************************/
/**
 * @brief   Structure to set ISP DMA read image input format.
 *
 *****************************************************************************/
typedef struct cam_device_pipe_in_fmt_s {
	uint32_t in_width;		      /**< width of input image */
	uint32_t in_height;		      /**< height of input image */
	cam_device_input_raw_fmt_t in_format; /**< format of input pixel */
	cam_device_bit_depth_t in_bit; /**< Bit depth of input device: tpg */
	cam_device_raw_pattern_t
	    in_pattern; /**< Bayer pattern of input RAW rgb/RGBIR image */
	cam_device_stitching_mode_t
	    stitch_mode; /* This member should be configured when loading HDR
			  * image
			  */
} cam_device_pipe_in_fmt_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure windows in ISP pipeline.
 *
 *****************************************************************************/
typedef struct cam_device_pipe_isp_window_s {
	cam_device_window_t crop_window; /**< Crop window for output path */
} cam_device_pipe_isp_window_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure submodules of pipeline when connecting
 *camera.
 *
 *****************************************************************************/
typedef union cam_device_pipe_submodule_ctrl_s {
	struct {
		uint32_t ae_enable : 1;	      /**< bit 0: 0-disable 1-enable */
		uint32_t af_enable : 1;	      /**< bit 1 */
		uint32_t hdr_enable : 1;      /**< bit 2 */
		uint32_t awb_enable : 1;      /**< bit 3 */
		uint32_t ccm_enable : 1;      /**< bit 4 */
		uint32_t compress_enable : 1; /**< bit 5 */
		uint32_t expand_enable : 1;   /**< bit 6 */
		uint32_t cnr_enable : 1;      /**< bit 7 */
		uint32_t ynr_enable : 1;      /**< bit 8 */
		uint32_t cproc_enable : 1;    /**< bit 9 */
		uint32_t dci_enable : 1;      /**< bit 10 */
		uint32_t demosaic_enable : 1; /**< bit 11 */
		uint32_t dg_enable : 1;	      /**< bit 12 */
		uint32_t dpcc_enable : 1;     /**< bit 13 */
		uint32_t dpf_enable : 1;      /**< bit 14 */
		uint32_t ee_enable : 1;	      /**< bit 15 */
		uint32_t gc_enable : 1;	      /**< bit 16 */
		uint32_t ge_enable : 1;	      /**< bit 17 */
		uint32_t gtm_enable : 1;      /**< bit 18 */
		uint32_t lsc_enable : 1;      /**< bit 19 */
		uint32_t lut3d_enable : 1;    /**< bit 20 */
		uint32_t pdaf_enable : 1;     /**< bit 21 */
		uint32_t rgbir_enable : 1;    /**< bit 22 */
		uint32_t wb_enable : 1;	      /**< bit 23 */
		uint32_t wdr_enable : 1;      /**< bit 24 */
		uint32_t dnr3_enable : 1;     /**< bit 25 */
		uint32_t dnr2_enable : 1;     /**< bit 26 */
		uint32_t reserved_enable : 5; /**< bit 27:31 */
	} sub_ctrl;
	uint32_t all_ctrl;
} cam_device_pipe_submodule_ctrl_u;

#define CAMDEV_IMAGE_EXP_NUM_MAX 4 /**< Maximum exposure number of image*/

/*****************************************************************************/
/**
 * @brief   Structure to configure image exposure control information.
 *
 *****************************************************************************/
typedef struct cam_device_image_exposure_control_s {
	uint32_t frame_index;
	float32_t image_gain[CAMDEV_IMAGE_EXP_NUM_MAX];
	/**< In linear mode or native HDR mode:\n image_gain[0] is image gain\n
		 In stitch HDR mode:\n
		 image_gain[0]: L image gain\n
		 image_gain[1]: S image gain\n
		 image_gain[2]: VS image gain\n
		 image_gain[3]: ES image gain */
	float32_t image_int_time[CAMDEV_IMAGE_EXP_NUM_MAX];
	/**< In linear mode or native HDR mode:\n image_int_time[0] is image
	 integration time\n In stitch HDR mode:\n image_int_time[0]: L image
	 integration time\n image_int_time[1]: S image integration time\n
	 image_int_time[2]: VS image integration time\n
	 image_int_time[3]: ES image integration time */
} cam_device_image_exposure_control_t;

/*****************************************************************************/
/**
 * @brief   Structure to histogram metadata  information.
 *
 *****************************************************************************/
typedef struct cam_device_metadata_hist_s {
	uint8_t hist_win_num;
	cam_device_window_t hist_roi_win[CAM_DEVICE_ROI_WINDOWS_MAX];
	cam_device_hist_bins_t red_channel_bins[CAM_DEVICE_ROI_WINDOWS_MAX];
	cam_device_hist_bins_t gr_channel_bins[CAM_DEVICE_ROI_WINDOWS_MAX];
	cam_device_hist_bins_t gb_channel_bins[CAM_DEVICE_ROI_WINDOWS_MAX];
	cam_device_hist_bins_t blue_channel_bins[CAM_DEVICE_ROI_WINDOWS_MAX];
	cam_device_hist_bins_t total_bins[CAM_DEVICE_ROI_WINDOWS_MAX];
} cam_device_metadata_hist_t;

/*****************************************************************************/
/**
 * @brief   Structure to mean luminance metadata  information.
 *
 *****************************************************************************/
typedef struct cam_device_metadata_mean_luma_s {
	uint8_t mean_win_num;
	cam_device_window_t mean_roi_win[CAM_DEVICE_ROI_WINDOWS_MAX];
	uint32_t red_channel_mean[CAM_DEVICE_ROI_WINDOWS_MAX];
	uint32_t gr_channel_mean[CAM_DEVICE_ROI_WINDOWS_MAX];
	uint32_t gb_channel_mean[CAM_DEVICE_ROI_WINDOWS_MAX];
	uint32_t blue_channel_mean[CAM_DEVICE_ROI_WINDOWS_MAX];
	uint32_t total_mean[CAM_DEVICE_ROI_WINDOWS_MAX];

} cam_device_metadata_mean_luma_t;

typedef struct cam_device_raw_channel_float_s {
	float32_t red_channel;
	float32_t gr_rhannel;
	float32_t gb_channel;
	float32_t blue_channel;
} cam_device_raw_channel_float_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice CCM manual configuration.
 *
 *****************************************************************************/
typedef struct cam_device_ccm_manual_config_s {
	float32_t cc_matrix[CAMDEV_CC_MATRIX_SIZE]; /**< Color correction matrix coeff */
	float32_t cc_offset[CAMDEV_CC_COLOR_CHANNEL_NUM];    /**< Color offset coefficient*/
} cam_device_ccm_manual_config_t;

/******************************************************************************/
/**
 * @brief   VsCamDevice CCM status structure.
 *
 *****************************************************************************/
typedef struct cam_device_ccm_status_s {
	bool_t enable;              /**< CCM enable status*/
	cam_device_config_mode_t current_mode;       /**< The run mode: 0--manual, 1--auto */
	cam_device_ccm_manual_config_t current_cfg;   /**< CCM current configuration*/
} cam_device_ccm_status_t;

/*****************************************************************************/
/**
 * @brief   Structure to metadata exposure information.
 *
 *****************************************************************************/
typedef struct cam_device_metadata_info_s {
	uint32_t chip_id;
	uint64_t frame_count;
	uint32_t aperture_size;  /**< Aperture size */
	uint32_t iso_strength;   /**< ISO strength */

	float32_t junction_temperature;
	uint32_t black_level_pedestal;

	uint8_t exposure_num; /**< The number of exposures */
	uint32_t
	    integration_time[CAMDEV_EXPOSURE_FRAME_MAX]; /*< Exposure
							  * time(unit: us)
							  */
	cam_device_integration_time_range_t integration_time_range
	    [CAMDEV_EXPOSURE_FRAME_MAX]; /**< Exposure time(unit: us) */

	float32_t analog_gain[CAMDEV_EXPOSURE_FRAME_MAX];
	cam_device_float_range_t analog_gain_range[CAMDEV_EXPOSURE_FRAME_MAX];
	float32_t digital_gain[CAMDEV_EXPOSURE_FRAME_MAX];
	cam_device_float_range_t digital_gain_range[CAMDEV_EXPOSURE_FRAME_MAX];
	cam_device_raw_channel_float_t wb_gain[CAMDEV_EXPOSURE_FRAME_MAX];
	float32_t dual_conv_gain[CAMDEV_EXPOSURE_FRAME_MAX];

	cam_device_raw_channel_float_t isp_dgain;
	cam_device_raw_channel_float_t isp_wb_gain;

	float32_t lux_index;
	float32_t total_gain;
	cam_device_ccm_status_t ccmConfig;

	cam_device_metadata_hist_t
	    hist_bins[CAMDEV_EXPOSURE_FRAME_MAX]; // reserved
	cam_device_metadata_mean_luma_t
	    mean_luma[CAMDEV_EXPOSURE_FRAME_MAX]; // reserved

} cam_device_metadata_info_t;

/*****************************************************************************/
/**
 * @brief   Structure to configure metadata information.
 *
 *****************************************************************************/
typedef struct cam_device_metadata_config_s {
	cam_device_awb_work_mode_t mode_awb;  /**< Choose awb mode */
	cam_device_ae_work_mode_t mode_ae;    /**< Choose ae mode */
	cam_device_metadata_info_t meta_info; /**< Metadata information */
} cam_device_metadata_config_t;

/*****************************************************************************/
/**
 * @brief   This function creates and initializes a CamDevice instance.
 *
 * @param   p_cam_config          Instance configuration.
 * @param   h_cam_device          Handle to the CamDevice instance.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    The/one/all parameter(s) is a(are) NULL
 *pointer(s)
 * @retval  RET_INVALID_PARM    Invalid configuration
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT vsi_cam_device_create(struct visp_dev *isp_dev,
			     cam_device_config_t *p_cam_config,
			     cam_device_handle_t *h_cam_device);

/*****************************************************************************/
/**
 * @brief   This function destroys a CamDevice instance.
 *
 * @param   h_cam_device          Handle to the CamDevice instance.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_NULL_POINTER    The/one/all parameter(s) is a(are) NULL
 *pointer(s)
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT vsi_cam_device_destroy(struct visp_dev *isp_dev,
			      cam_device_handle_t h_cam_device);

/*****************************************************************************/
/**
 * @brief   This function sets the output format of main or self path.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   path                    Select the output path id
 * @param   p_fmt                    Configuration structure for output format
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT vsi_cam_device_set_out_format(struct visp_dev *isp_dev,
				     cam_device_handle_t h_cam_device,
				     cam_device_pipe_out_path_type_t path,
				     cam_device_pipe_out_fmt_t *p_fmt);

/*****************************************************************************/
/**
 * @brief   This function sets the Input format.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   path                    Select the input path id
 * @param   p_fmt                    Configuration structure for input format
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT vsi_cam_device_set_in_format(struct visp_dev *isp_dev,
				    cam_device_handle_t h_cam_device,
				    cam_device_pipe_in_path_type_t path,
				    cam_device_pipe_in_fmt_t *p_fmt);

/*****************************************************************************/
/**
 * @brief   This function gets the output format of main or self path.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   path                    Select the output path id
 * @param   p_fmt                    Configuration structure for output format
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT vsi_cam_device_get_out_format(cam_device_config_t *p_cam_config,
				     cam_device_handle_t h_cam_device,
				     cam_device_pipe_out_path_type_t path,
				     cam_device_pipe_out_fmt_t *p_fmt);

/*****************************************************************************/
/**
 * @brief   This function gets the input format of main or self path.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   path                    Select the input path id
 * @param   p_fmt                    Configuration structure for input format
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT vsi_cam_device_get_in_format(cam_device_config_t *p_cam_config,
				    cam_device_handle_t h_cam_device,
				    cam_device_pipe_in_path_type_t path,
				    cam_device_pipe_in_fmt_t *p_fmt);

/*****************************************************************************/
/**
 * @brief   This function sets windows in the ISP pipeline.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   path                    Config the output path id
 * @param   p_isp_window              Configuration structure for windows in
 *pipeline
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT
vsi_cam_device_set_isp_window(cam_device_config_t *p_cam_config,
			      cam_device_handle_t h_cam_device,
			      cam_device_pipe_out_path_type_t path,
			      const cam_device_pipe_isp_window_t *p_isp_window);

/*****************************************************************************/
/**
 * @brief   This function gets windows in the ISP pipeline.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   path                    Config the output path id
 * @param   p_isp_window              Pointer to ISP windows in pipeline
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT
vsi_cam_device_get_isp_window(cam_device_config_t *p_cam_config,
			      cam_device_handle_t h_cam_device,
			      cam_device_pipe_out_path_type_t path,
			      cam_device_pipe_isp_window_t *p_isp_window);

/*****************************************************************************/
/**
 * @brief   This function sets up the whole camera system. First, initialize the
 *          CamEngine and set the pipeline path. Then, connect the input
 *          (sensor or image) with the CamEngine to process the image data.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   p_sub_ctrl                Control the submodules when setting up the
 *                                  camera system.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT
vsi_cam_device_connect_camera(struct visp_dev *isp_dev,
			      cam_device_handle_t h_cam_device,
			      const cam_device_pipe_submodule_ctrl_u *p_sub_ctrl);

/*****************************************************************************/
/**
 * @brief   This function disconnects the whole camera system.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT vsi_cam_device_disconnect_camera(struct visp_dev *isp_dev,
					cam_device_handle_t h_cam_device);

/*****************************************************************************/
/**
 * @brief   This function gets the version id.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   p_version_id              Version id which is related to the ISP
 *                                  hardware version.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT vsi_cam_device_get_software_version(cam_device_handle_t h_cam_device,
					   char *p_version_id);

/*****************************************************************************/
/**
 * @brief   This function sets output path streaming.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   p_config                 Pointer to the output path streaming
 *configuration
 *
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT
vsi_cam_device_set_path_streaming(struct visp_dev *isp_dev,
				  cam_device_handle_t h_cam_device,
				  cam_device_path_streaming_cfg_t *p_config);

/*****************************************************************************/
/**
 * @brief   This function gets output path streaming status.
 *
 * @param   h_cam_device              Handle to the CamDevice instance.
 * @param   path                    Pointer to the output path streaming
 *configuration
 *
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT
vsi_cam_device_get_path_streaming(struct visp_dev *isp_dev,
				  cam_device_handle_t h_cam_device,
				  cam_device_path_streaming_cfg_t *p_config);

/*****************************************************************************/
/**
 * @brief   This function starts pipeline path.
 *
 * @param   h_cam_device          Handle to the CamDevice instance.
 * @param   path                Pipeline output path.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT vsi_cam_device_start_path(cam_device_config_t *p_cam_config,
				 cam_device_handle_t h_cam_device,
				 cam_device_pipe_out_path_type_t path);

/*****************************************************************************/
/**
 * @brief   This function stops pipeline path.
 *
 * @param   h_cam_device          Handle to the CamDevice instance.
 * @param   path                Pipeline output path.
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_FAILURE         General failure
 *
 *****************************************************************************/
RESULT vsi_cam_device_stop_path(cam_device_config_t *p_cam_config,
				cam_device_handle_t h_cam_device,
				cam_device_pipe_out_path_type_t path);

/*****************************************************************************/
/**
 * @brief   This function gets the instance id.
 *
 * @param   h_cam_device          Handle to the CamDevice instance.
 * @param   p_hw_id               Pointer to hardware pipeline id
 *
 * @return                      Return the instance id.
 *
 *****************************************************************************/
RESULT vsi_cam_device_get_hardware_id(cam_device_handle_t h_cam_device,
				      uint32_t *p_hw_id);

// uint32_t cam_device_get_virtual_id
//(
//     uint32_t instance_id
//);

/*****************************************************************************/
/**
 * @brief   This function allocates the memory from internal reserved memory.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   byte_size            The total byte size of allocate memory
 * @param   p_base_address        The pointer of physical base address of
 *allocated memory
 * @param   p_ipl_address         The pointer of the mapped virtual base address
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_alloc_res_memory(struct visp_dev *isp_dev,
				       cam_device_handle_t h_cam_device,
				       uint32_t byte_size,
				       uint32_t *p_base_address,
				       void **p_ipl_address);

/*****************************************************************************/
/**
 * @brief   This function frees the memory of internal reserved memory.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   base_address         The physical base address of free memory
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_free_res_memory(struct visp_dev *isp_dev,
				      cam_device_handle_t h_cam_device,
				      uint32_t base_address);

#ifdef ISP_OFFLINE_TEST
/*****************************************************************************/
/**
 * @brief   This function sets the name of offline case.
 *
 * @param   p_case_name               The name of offline case
 * @param   len                     The string length for p_case_name
 *
 * @return                     void.
 *
 *****************************************************************************/
void vsi_cam_device_set_case_name(char *p_case_name, uint32_t len);
#endif

/*****************************************************************************/
/**
 * @brief   This function writes data to the specified register.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   address             data address
 * @param   value               data value
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT vsi_cam_device_write_register(cam_device_handle_t h_cam_device,
				     uint32_t address, uint32_t value);

/*****************************************************************************/
/**
 * @brief   This function reads data from the specified register.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   address             data address
 * @param   p_value              Pointer to data value
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT vsi_cam_device_read_register(cam_device_handle_t h_cam_device,
				    uint32_t address, uint32_t *p_value);

/*****************************************************************************/
/**
 * @brief   This function sets image metadata configuration parameters.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 * @param   p_metadata           Pointer to metadata value
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT vsi_cam_device_config_metadata(cam_device_handle_t h_cam_device,
				      cam_device_metadata_config_t *p_metadata);

/*****************************************************************************/
/**
 * @brief   This function enables noise removal relocation.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT vsi_cam_device_nr_reloc_enable(cam_device_handle_t h_cam_device);

/*****************************************************************************/
/**
 * @brief   This function disables noise removal relocation.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT vsi_cam_device_nr_reloc_disable(cam_device_handle_t h_cam_device);

/*****************************************************************************/
/**
 * @brief   This function does fast ISP software reset.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT vsi_cam_device_sw_fast_stop(cam_device_handle_t h_cam_device);

/*****************************************************************************/
/**
 * @brief   This function does hardware system reset.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT vsi_cam_device_hw_system_reset(cam_device_handle_t h_cam_device);

/*****************************************************************************/
/**
 * @brief   This function does fast ISP software reset.
 *
 * @param   h_cam_device          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 * @retval  RET_WRONG_HANDLE    Invalid instance handle
 *
 *****************************************************************************/
RESULT vsi_cam_device_sw_fast_start(cam_device_handle_t h_cam_device);

/* @} cam_device_pipeline */
RESULT vsi_cam_device_start_streaming(cam_device_config_t *p_cam_config,
				      cam_device_handle_t h_cam_device,
				      uint32_t frames // 0-continue
);

#ifdef __cplusplus
}
#endif

#endif // __CAMDEV_API_H__
