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

#ifndef CAMDEV_SENSOR_API_H
#define CAMDEV_SENSOR_API_H

#include "cam_device_api.h"

/**
 * @defgroup cam_device_sensor Cam Device Sensor Definitions
 * @{
 *
 *
 */

#define SENSOR_QUERY_NUM 12		/**< Sensor query number */
#define FILE_NAME_LEN 30		/**< Sensor file name length */
#define COMPAND_CURVE_LEN 65		/**< Sensor compand curve length */
#define SENSOR_ID_LEN 11		/**< Image sensor id length */
#define MODULE_SN_LEN 12		/**< Barcode-Module SN length */
#define CAMDEV_SENSOR_EXP_NUM_MAX 4 /**< Maximum exposure number of sensor*/
#define CAMDEV_SENSOR_METADATA_WIN_NUM_MAX									 \
	3 /**< Maximum sensor metedata window number */

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor type.
 *
 *****************************************************************************/
typedef enum cam_device_sensor_type_e {
	CAMDEV_SENSOR_TYPE_LINEAR = 0,	  /**< Sensor type linear*/
	CAMDEV_SENSOR_TYPE_STITCHING_HDR, /**< Sensor type stitching HDR */
	CAMDEV_SENSOR_TYPE_NATIVE_HDR,	  /**< Sensor type native HDR */
	DUMMY_CAMDEV_0035 = 0xdeadfeed
} cam_device_sensor_type_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor native HDR mode.
 *
 *****************************************************************************/
typedef enum cam_device_sensor_native_mode_e {
	CAMDEV_SENSOR_NATIVE_MODE_DCG =
		0, /**< HCG and LCG combined in sensor */
	CAMDEV_SENSOR_NATIVE_MODE_L_AND_S,	/**< L+S combined in sensor */
	CAMDEV_SENSOR_NATIVE_MODE_3DOL,		  /**< 3DOL combined in sensor */
	CAMDEV_SENSOR_NATIVE_MODE_4DOL,		  /**< 4DOL combined in sensor*/
	CAMDEV_SENSOR_NATIVE_MODE_DCG_SPD_VS, /**< 4DOL combined in sensor*/
	CAMDEV_SENSOR_NATIVE_MODE_MAX,
	DUMMY_CAMDEV_0036 = 0xdeadfeed
} cam_device_sensor_native_mode_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor AF mode.
 *
 *****************************************************************************/
typedef enum cam_device_sensor_auto_focus_mode_s {
	CAMDEV_SENSOR_AF_NOTSUPP,   /**< AF not support */
	CAMDEV_SENSOR_AF_MODE_CDAF, /**< CDAF mode */
	CAMDEV_SENSOR_AF_MODE_PDAF, /**< PDAF mode */
	DUMMY_CAMDEV_0037 = 0xdeadfeed
} cam_device_sensor_auto_focus_mode_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type of sensor focusing position mode.
 *
 *****************************************************************************/
typedef enum cam_device_sensor_focus_pos_mode_e {
	CAMDEV_SENSOR_FOCUS_POS_ABSOLUTE = 0, /**< Absolute focusing mode */
	CAMDEV_SENSOR_FOCUS_POS_RELATIVE,	 /**< Relative focusing mode */
	CAMDEV_SENSOR_FOCUS_POS_MAX,
	DUMMY_CAMDEV_0038 = 0xdeadfeed
} cam_device_sensor_focus_pos_mode_t;

/******************************************************************************/
/**
 * @brief   Cam Device sensor output data type.
 *
 *****************************************************************************/
typedef enum cam_device_sensor_data_type_e {
	CAMDEV_SENSOR_DATA_TYPE_BAYER = 0, /**< Bayer data type */
	CAMDEV_SENSOR_DATA_TYPE_YUV = 1,   /**< YUV data type */
	CAMDEV_SENSOR_DATA_TYPE_MAX,
	DUMMY_CAMDEV_0039 = 0xdeadfeed
} cam_device_sensor_data_type_t;

/******************************************************************************/
/**
 * @brief   Cam Device sensor interface type.
 *
 *****************************************************************************/
typedef enum cam_device_sensor_itf_type_e {
	CAMDEV_SENSOR_ITF_TYPE_MIPI_1LANES = 0, /**< 1 lane mipi mode */
	CAMDEV_SENSOR_ITF_TYPE_MIPI_2LANES = 1, /**< 2 lane mipi mode */
	CAMDEV_SENSOR_ITF_TYPE_MIPI_4LANES = 2, /**< 4 lane mipi mode */
	CAMDEV_SENSOR_ITF_TYPE_BT601 = 3,	/**< BT.601 type */
	CAMDEV_SENSOR_ITF_TYPE_MAX,
	DUMMY_CAMDEV_0040 = 0xdeadfeed
} cam_device_sensor_itf_type_t;

/*****************************************************************************/
/**
 * @brief   CamDevice isi sensor driver configuration.
 *
 *****************************************************************************/
typedef void *cam_device_sensor_drv_handle_t;

/*****************************************************************************/
/**
 * @brief   CamDevice isi sensor driver configuration.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_drv_cfg_s {
	cam_device_sensor_drv_handle_t sensor_drv_handle;
	uint32_t sensor_dev_id;
} cam_device_sensor_drv_cfg_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor compand configuration.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_compand_curve_s {
	bool_t enable;	 /**< Enable value */
	uint8_t in_bit;	 /**< Compand curve input bit value */
	uint8_t out_bit; /**< Compand curve output bit value */
	uint8_t px[COMPAND_CURVE_LEN - 1]; /**< X axis interval */
	uint32_t
		x_data[COMPAND_CURVE_LEN]; /**< Compand curve X axis - 65 points */
	uint32_t
		y_data[COMPAND_CURVE_LEN]; /**< Compand curve y axis - 65 points */
} cam_device_sensor_compand_curve_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor test pattern configuration.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_test_pattern_s {
	uint32_t enable;  /**< Test pattern enable value */
	uint32_t pattern; /**< Sensor pattern type */
} cam_device_sensor_test_pattern_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor resolution structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_resolution_s {
	uint32_t width;	 /**< Sensor resoulution width */
	uint32_t height; /**< Sensor resoulution height */
} cam_device_sensor_resolution_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor resolution collection information structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_resolution_collection_s {
	int index;				   /**< Sensor mode index */
	cam_device_sensor_resolution_t resolution; /**< Sensor resolution */
} cam_device_sensor_resolution_collection_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor register configuration.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_register_s {
	uint16_t addr;	/**< Sensor register address */
	uint16_t value; /**< Sensor register value */
} cam_device_sensor_register_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor size information structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_size_s {
	uint32_t bounds_width;	/**< Sensor bounds width */
	uint32_t bounds_height; /**< Sensor bounds height */
	uint32_t top;		/**< top position */
	uint32_t left;		/**< left position */
	uint32_t width;		/**< Real image width */
	uint32_t height;	/**< Real image height */
} cam_device_sensor_size_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor compress information structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_compress_s {
	uint32_t enable; /**< Enable value */
	uint32_t x_bit;	 /**< Expand curve input data bit width */
	uint32_t y_bit;	 /**< Expand curve output data bit width */
} cam_device_sensor_compress_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor mode information structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_mode_info_s {
	uint32_t index;			   /**< Sensor mode index value */
	cam_device_sensor_size_t size; /**< Sensor size */
	cam_device_sensor_type_t
		sensor_type; /**< The sensor type is linear or HDR*/
	cam_device_stitching_mode_t
		stitching_mode; /**< The sensor type is HDR stitching*/
	cam_device_sensor_native_mode_t
		native_mode;	/**< The sensor type is HDR Native */
	uint32_t bit_width; /**< Sensor bit width */
	cam_device_sensor_compress_t
		compress; /**< Sensor compression information */
	cam_device_raw_pattern_t bayer_pattern; /**< Sensor Bayer pattern type*/
	uint32_t max_fps;			/**< Sensor maximum FPS value */
	cam_device_sensor_auto_focus_mode_t
		af_mode; /**< Sensor auto focusing mode */
} cam_device_sensor_mode_info_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor query information structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_query_s {
	uint32_t number; /**< Query sensor index number */
	cam_device_sensor_mode_info_t
		sensor_mode_info[SENSOR_QUERY_NUM]; /**< Query sensor mode
						   information */
} cam_device_sensor_query_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor range information structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_range_s {
	float32_t max;	/**< Maximum value */
	float32_t min;	/**< Minimum value */
	float32_t step; /**< Step value */
} cam_device_sensor_range_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor integer range information structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_integer_range_s {
	uint32_t max;  /**< Maximum value*/
	uint32_t min;  /**< Minimum value*/
	uint32_t step; /**< Step value */
} cam_device_sensor_integer_range_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor motor position structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_focus_pos_s {
	cam_device_sensor_focus_pos_mode_t
		pos_mode;	   /**< Sensor motor focusing postion mode*/
	uint32_t position; /**< Sensor motor focusing postion, the ranging is
				  from 0 to 1023. */
} cam_device_sensor_focus_pos_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor gain configuration.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_gain_s {
	float32_t a_gain[CAMDEV_SENSOR_EXP_NUM_MAX]; /**< Sensor analog gain */
	float32_t d_gain[CAMDEV_SENSOR_EXP_NUM_MAX]; /**< Sensor digital gain */
} cam_device_sensor_gain_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor status information structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_status_s {
	bool_t is_connected;			/**< Sensor connection status */
	cam_device_sensor_gain_t curr_gain; /**< Sensor current gain info */
	float32_t curr_int_time
		[CAMDEV_SENSOR_EXP_NUM_MAX]; /**< Sensor current integration time
						info: the unit is microseconds  */
	cam_device_sensor_resolution_t
		curr_res;		/**< Sensor current resolution info */
	float32_t curr_fps; /**< Sensor current FPS info */
} cam_device_sensor_status_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor exposure time configuration.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_exposure_control_s {
	float32_t
		integration_time[CAMDEV_SENSOR_EXP_NUM_MAX]; /**< Sensor integration
								time value */
} cam_device_sensor_exposure_control_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor exposure time configuration.
 *
 *****************************************************************************/
typedef union cam_device_sensor_metadata_attr_s {
	struct {
		uint32_t support : 1;	/**< bit 0: 0-disable 1-enable */
		uint32_t reg_info : 1;	/**< bit 1: register information */
		uint32_t exp_time : 1;	/**< bit 2: exposure time */
		uint32_t again : 1;	/**< bit 3: Analogue agin */
		uint32_t dgain : 1;	/**< bit 4: Digital gain */
		uint32_t bls : 1;	/**< bit 5: BLS */
		uint32_t hist : 1;	/**< bit 6: Histogram */
		uint32_t mean_luma : 1; /**< bit 7: Mean luminance */
		uint32_t reserved_enable : 24; /**< bit 8:31: Reserved bit */

	} sub_attr;		/**< Sub-attribute */
	uint32_t main_attr; /**< Main attribute */
} cam_device_sensor_metadata_attr_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor meatdata window information.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_metadata_win_s {
	uint8_t win_num; /**< The number of windows */
	cam_device_window_t
		meta_win[CAMDEV_SENSOR_METADATA_WIN_NUM_MAX]; /**< Metadata windows
							   */
} cam_device_sensor_metadata_win_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor information structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_info_s {

	char sensor_name[CAMDEV_INPUT_DEV_NAME_LEN]; /**< Sensor name */
	uint32_t sensor_rev_id; /**< Sensor revision id register */
	cam_device_raw_pattern_t
		bayer_pattern; /**< Sensor Bayer pattern type */
	cam_device_sensor_test_pattern_t
		test_pattern; /**< Sensor test_pattern info*/

	cam_device_sensor_range_t
		a_gain_range[CAMDEV_SENSOR_EXP_NUM_MAX]; /**< Sensor analog gain
							range*/
	cam_device_sensor_range_t
		d_gain_range[CAMDEV_SENSOR_EXP_NUM_MAX]; /**< Sensor digital gain
							range*/
	cam_device_sensor_range_t
		int_time_range[CAMDEV_SENSOR_EXP_NUM_MAX]; /**< Sensor integration
							  time range: the unit
							  is microseconds */

	cam_device_sensor_metadata_attr_t
		meta_supp; /**< Sensor metadata support information */
} cam_device_sensor_info_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor one time program information structure.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_otp_module_info_s {
	uint16_t hw_version;		  /**< Module HW version */
	uint16_t sensor_eeprom_revision;  /**< EEPROM revision */
	uint16_t sensor_revision;	  /**< Image sensor revision */
	uint16_t t_lens_revision;	  /**< Tlens revision */
	uint16_t ircf_revision;		  /**< Ircf revision */
	uint16_t lens_revision;		  /**< Lens revision */
	uint16_t ca_revision;		  /**< Contact assembly revision */
	uint16_t module_inte_id;	  /**< Module integrator id */
	uint16_t factory_id;		  /**< Factory id */
	uint16_t mirror_flip;		  /**< Image mirror and flip */
	uint16_t t_lens_slave_id;	  /**< Tlens slave id */
	uint16_t sensor_eeprom_slave_id;  /**< EEPROM slave id */
	uint16_t sensor_slave_id;	  /**< Image sensor slave id */
	uint8_t sensor_id[SENSOR_ID_LEN]; /**< Image sensor id */
	uint16_t manu_date_year;	  /**< Manufacture date (Year) */
	uint16_t manu_date_month;	  /**< Manufacture date (Month) */
	uint16_t manu_date_day;		  /**< Manufacture date (Date) */
	uint8_t barcode_module_sn[MODULE_SN_LEN]; /**< Barcode-Module SN */
	uint16_t map_total_size; /**< Total size of EEPROM map */
} cam_device_sensor_otp_module_info_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor driver mode information.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_drv_mode_info_s {
	uint32_t index;	 /**< Sensor mode index value */
	uint32_t width;	 /**< Real image width */
	uint32_t height; /**< Real image height */
	cam_device_sensor_type_t
		sensor_type; /**< The sensor type is linear or HDR*/
	cam_device_stitching_mode_t
		stitching_mode; /**< The sensor type is HDR stitching*/
	cam_device_sensor_native_mode_t
		native_mode;	/**< The sensor type is HDR Native */
	uint32_t bit_width; /**< Sensor bit width */
	cam_device_raw_pattern_t bayer_pattern; /**< Sensor Bayer pattern type*/
	uint32_t max_fps;			/**< Sensor maximum FPS value */
	cam_device_sensor_auto_focus_mode_t
		af_mode; /**< Sensor auto focusing mode */
	cam_device_sensor_data_type_t data_type; /**< Sensor data type */
	cam_device_sensor_itf_type_t itf_type;	 /**< Sensor interface type */
} cam_device_sensor_drv_mode_info_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor mode list information.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_list_info_s {
	uint32_t number; /**< Sensor index number */
	char name[20];	 /**< Sensor name */
	cam_device_sensor_drv_mode_info_t
		sensor_mode_info[SENSOR_QUERY_NUM]; /**< Sensor mode information */
} cam_device_sensor_list_info_t;

/*****************************************************************************/
/**
 * @brief   CamDevice sensor connection port information.
 *
 *****************************************************************************/
typedef struct cam_device_sensor_connect_port_info_s {
	uint32_t chip_id; /**< Sensor chip id */
	char name[20];	  /**< Pointer to sensor name */
} cam_device_sensor_connect_port_info_t;

/****************************************************************************/
/**
 * @brief   Get the sensor information, e.g., sensor name, calibration file,
 *etc.
 *
 * @param   h_cam_device		  CamDevice driver handle
 * @param   p_sensor_info		 Pointer to sensor information structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_info(cam_device_handle_t h_cam_device,
					  cam_device_sensor_info_t *p_sensor_info);

/****************************************************************************/
/**
 * @brief   Open the sensor from the sensor driver.
 *
 * @param   h_cam_device		  CamDevice driver handle
 * @param   mode_index		   sensor drv mode index
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_FAILURE		 Operation failed
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_open(struct visp_dev *isp_dev,
				  cam_device_handle_t h_cam_device,
				  uint32_t mode_index);

/****************************************************************************/
/**
 * @brief   Close the sensor from the sensor driver.
 *
 * @param   h_cam_device		  CamDevice driver handle
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_FAILURE		 Operation failed
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_close(struct visp_dev *isp_dev,
				   cam_device_handle_t h_cam_device);

/****************************************************************************/
/**
 * @brief   Sensor driver handle register.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   sensor_drv_handle	 Sensor driver handle
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_drv_handle_register(
	struct visp_dev *isp_dev, cam_device_handle_t h_cam_device,
	const cam_device_sensor_drv_cfg_t *p_sensor_drv);

/****************************************************************************/
/**
 * @brief   Sensor driver handle un-register.
 *
 * @param   h_cam_device		  Cam Device driver handle
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 *
 *****************************************************************************/
RESULT
vsi_cam_device_sensor_drv_handle_un_register(struct visp_dev *isp_dev,
						 cam_device_handle_t h_cam_device);
#if 0
/****************************************************************************/
/**
 * @brief   Changes the sensor output from a live image to a test pattern.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_test_pattern		Pointer to sensor test pattern structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_set_test_pattern(
	struct visp_dev *isp_dev, cam_device_handle_t h_cam_device,
	const cam_device_sensor_test_pattern_t *p_test_pattern);
#endif
/****************************************************************************/
/**
 * @brief   Mapping the sensor driver.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_sensor_name		 Pointer to sensor driver name
 * @param   p_sensor_drvhandle	Sensor driver handle pointer
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_FAILURE		 General failure
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 * @retval  RET_INVALID_PARM	Invalid parameter
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_mapping(
	struct visp_dev *isp_dev, cam_device_handle_t h_cam_device,
	const char *p_sensor_name,
	cam_device_sensor_drv_handle_t *p_sensor_drvhandle);

/****************************************************************************/
/**
 * @brief   Application control set a value via I2C to the sensor registers.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_register		   Pointer to sensor register structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT
vsi_cam_device_sensor_set_register(cam_device_handle_t h_cam_device,
				   cam_device_sensor_register_t *p_register);

/****************************************************************************/
/**
 * @brief   Application control gets a value via I2C from sensor registers.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_register		   Pointer to sensor register structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT
vsi_cam_device_sensor_get_register(cam_device_handle_t h_cam_device,
				   cam_device_sensor_register_t *p_register);

/****************************************************************************/
/**
 * @brief   Set the sensor frame rate.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_fps				Pointer to sensor frame per second
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_set_frame_rate(struct visp_dev *isp_dev,
						cam_device_handle_t h_cam_device,
						/*float32_t **/ uint32_t *p_fps);

/****************************************************************************/
/**
 * @brief   Get the current frame rate of the sensor.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_fps				Pointer to sensor frame per second
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_frame_rate(cam_device_handle_t h_cam_device,
						float32_t *p_fps);

/****************************************************************************/
/**
 * @brief   Get the current working sensor mode, including the sensor working
		status, (HDR/Linear, image width/height, etc.).
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_mode			   Pointer to sensor mode structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT
vsi_cam_device_sensor_get_mode_info(cam_device_handle_t h_cam_device,
					cam_device_sensor_mode_info_t *p_mode);

/****************************************************************************/
/**
 * @brief   Get all supported modes for the sensor to the application, including
		sensor driver mode(width, height, bit width, etc).
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_query			  Pointer to sensor query structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_query(struct visp_dev *isp_dev,
				   cam_device_handle_t h_cam_device,
				   cam_device_sensor_query_t *p_query);

/****************************************************************************/
/**
 * @brief   Get the current sensor exposure time, gain value, etc.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_status			 Pointer to sensor status structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_status(cam_device_handle_t h_cam_device,
					cam_device_sensor_status_t *p_status);

/****************************************************************************/
/**
 * @brief   Application control set gain value to the sensor driver.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_gain			   Pointer to sensor gain structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_set_gain(cam_device_handle_t h_cam_device,
					  cam_device_sensor_gain_t *p_gain);

/****************************************************************************/
/**
 * @brief   Application control get gain value from sensor driver.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_gain			   Pointer to sensor gain structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported

 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_gain(cam_device_handle_t h_cam_device,
					  cam_device_sensor_gain_t *p_gain);

/****************************************************************************/
/**
 * @brief   Structure to set exposure time value to the sensor driver.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_exp_ctrl			Pointer to sensor exposure structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_set_exposure_control(
	cam_device_handle_t h_cam_device,
	cam_device_sensor_exposure_control_t *p_exp_ctrl);

/****************************************************************************/
/**
 * @brief   Structure to get exposure time value from the sensor driver.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_exp_ctrl			Pointer to sensor exposure structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_exposure_control(
	cam_device_handle_t h_cam_device,
	cam_device_sensor_exposure_control_t *p_exp_ctrl

);

/****************************************************************************/
/**
 * @brief   This function sets sensor motor focusing position.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_focus_pos		   Pointer to the focusing position
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_set_focus_positon(
	cam_device_handle_t h_cam_device,
	cam_device_sensor_focus_pos_t *p_focus_pos);

/****************************************************************************/
/**
 * @brief   This function gets sensor motor focusing position.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_focus_pos		   Pointer to the focusing position
 * @param   p_range_limit		 Pointer to the focusing position range
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_focus_positon(
	cam_device_handle_t h_cam_device,
	cam_device_sensor_focus_pos_t *p_focus_pos,
	cam_device_sensor_integer_range_t *p_range_limit);

/****************************************************************************/
/**
 * @brief   Application control set streaming on or off to the sensor driver.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   is_enable			Enable or Disable the streaming
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_set_streaming(cam_device_handle_t h_cam_device,
					   bool_t is_enable);

/****************************************************************************/
/**
 * @brief   Get the one-time program information from sensor driver.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_otp_module_info	  Pointer to sensor OTP module information
 *structure
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_otp_info(
	cam_device_handle_t h_cam_device,
	cam_device_sensor_otp_module_info_t *p_otp_module_info);

/****************************************************************************/
/**
 * @brief   Get sensor metadata support attribute.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_meta_attr		   Pointer to metadata attribute parameters
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_metadata_attr(
	cam_device_handle_t h_cam_device,
	cam_device_sensor_metadata_attr_t *p_meta_attr);

/****************************************************************************/
/**
 * @brief   Set sensor metadata enable attribute.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   meta_attr			Metadata attribute parameters
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_set_metadata_attr(
	cam_device_handle_t h_cam_device,
	cam_device_sensor_metadata_attr_t meta_attr);

/****************************************************************************/
/**
 * @brief   Get sensor metadata window information.
 *
 * @param   h_cam_device		  Cam Device driver handle
 * @param   p_meta_win			Pointer to sensor metadata window info
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 * @retval  RET_WRONG_HANDLE	Invalid handle
 * @retval  RET_NULL_POINTER	Null pointer
 * @retval  RET_WRONG_STATE	 State machine in wrong state
 * @retval  RET_NOTSUPP		 Feature not supported
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_metadata_win(
	cam_device_handle_t h_cam_device,
	cam_device_sensor_metadata_win_t *p_meta_win);

/****************************************************************************/
/**
 * @brief   Get the number of sensors.
 *
 * @param   p_number	 Pointer to sensor number
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_number(cam_device_handle_t h_cam_device,
					uint16_t *p_number);

/****************************************************************************/
/**
 * @brief   Get all sensor mode information.
 *
 * @param   h_cam_device		  CamDevice driver handle
 * @param   p_sensor_list_info	 Pointer to sensor mode list
 * @param   sensor_num		   The number of sensors
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_list_info(
	cam_device_handle_t h_cam_device,
	cam_device_sensor_list_info_t *p_sensor_list_info,
	const uint16_t sensor_num);

/****************************************************************************/
/**
 * @brief   Get sensor name which is connect to FPGA i2c-8/9/10/11,
 * currently only support i2c-8/9 query.
 *
 * @param   h_cam_device		  CamDevice driver handle
 * @param   port_id			  FPGA port index
 * @param   p_port_info		   Pointer to port information
 *
 * @retval  RET_SUCCESS		 Operation succeeded
 *
 *****************************************************************************/
RESULT vsi_cam_device_sensor_get_connect_port_info(
	struct visp_dev *isp_dev, cam_device_handle_t h_cam_device,
	cam_device_mcm_port_id_t port_id,
	cam_device_sensor_connect_port_info_t *p_port_info);

/* @} cam_device_sensor */

#ifdef __cplusplus
extern "C" {
#endif

#endif // __CAMDEV_SENSOR_API_H__
