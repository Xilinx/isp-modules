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

/**
 * @defgroup cam_device_calibration Cam Device Calibration Definitions
 * @{
 *
 *
 */

#ifndef __CAMDEV_CALIBRATION_H__
#define __CAMDEV_CALIBRATION_H__

#include <ebase/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAMDEV_CALIB_LENGTH 50

/*****************************************************************************/
/**
 * @brief   Cam Device Calibration doortype of an illumination profile
 */
/*****************************************************************************/
typedef enum cam_device_door_type_e {
	CAMDEV_DOOR_TYPE_OUTDOOR = 0, /**< Scene type, outdoor */
	CAMDEV_DOOR_TYPE_INDOOR,      /**< Scene type, indoor */
	CAMDEV_DOOR_TYPE_MAX,
	DUMMY_CAMDEV_DOOR_TYPE = 0xDEADFEED
} cam_device_door_type_t;

/*****************************************************************************/
/**
 * @brief   Cam Device Calibration AWB manual/auto control type of an
 * illumination profile
 */
/*****************************************************************************/
typedef enum cam_device_calib_awb_type_e {
	CAMDEV_CALIB_AWB_TYPE_MANUAL = 0, /**< Manual AWB type */
	CAMDEV_CALIB_AWB_TYPE_AUTO,	  /**< Auto AWB type */
	CAMDEV_CALIB_AWB_TYPE_MAX,
	DUMMY_CALIB_AWB_TYPE = 0xDEADFEED
} cam_device_calib_awb_type_t;

/*****************************************************************************/
/**
 * @brief   Cam Device illumination type of calibration
 */
/*****************************************************************************/
typedef enum cam_device_calib_illum_type_e {
	CAMDEV_CALIB_ILLUM_TYPE_A = 0, /**< Illumination type: A */
	CAMDEV_CALIB_ILLUM_TYPE_D50,   /**< Illumination type: D50*/
	CAMDEV_CALIB_ILLUM_TYPE_D65,   /**< Illumination type: D65 */
	CAMDEV_CALIB_ILLUM_TYPE_D75,   /**< Illumination type: D75 */
	CAMDEV_CALIB_ILLUM_TYPE_F2,    /**< Illumination type: F2 */
	CAMDEV_CALIB_ILLUM_TYPE_F11,   /**< Illumination type: F11 */
	CAMDEV_CALIB_ILLUM_TYPE_F12,   /**< Illumination type: F12 */
	CAMDEV_CALIB_ILLUM_TYPE_H,     /**< Illumination type: H */
	CAMDEV_CALIB_ILLUM_TYPE_MAX,
	DUMMY_CALIB_ILLUM_TYPE = 0xDEADFEED
} cam_device_calib_illum_type_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration file header resolution parameters: framerate
 *configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_header_resolution_framerate_s {
	char name[CAMDEV_CALIB_LENGTH]; /**< Framerate header name */
	float fps;			/**< Framerate value */
} cam_device_calib_header_resolution_framerate_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration file header parameters: resolution
 *configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_header_resolution_s {
	char name[CAMDEV_CALIB_LENGTH]; /**< Resolution header name */
	char id[CAMDEV_CALIB_LENGTH];	/**< Resolution index */
	uint16_t width;			/**< width */
	uint16_t height;		/**< Height */

	cam_device_calib_header_resolution_framerate_t
	    *p_framerate;	  /**< Framerate list */
	uint8_t framerate_number; /**< The number of framerate list members */
} cam_device_calib_header_resolution_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration file header configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_header_s {
	char date[CAMDEV_CALIB_LENGTH];	       /**< Calibration file date */
	char creator[CAMDEV_CALIB_LENGTH];     /**< Calibration file creator */
	char sensor_name[CAMDEV_CALIB_LENGTH]; /**< Calibration file sensor name
						*/
	char sample_name[CAMDEV_CALIB_LENGTH]; /**< Calibration file sample name
						*/
	char generator_version[CAMDEV_CALIB_LENGTH]; /**< Calibration file
							generator version */

	cam_device_calib_header_resolution_t *
	    p_resolutions; /**< Calibration file header resolution parameters */
	uint8_t resolution_number; /**< Resolution parameters number */
} cam_device_calib_header_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AWB IIR configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_awb_iir_s {
	float damp_coef_add; /**< Increment of damping, which is an infinite
			      * impulse response filter (IIR) parameter
			      */
	float damp_coef_sub; /**< Decrement of damping, which is an IIR
			      * parameter.
			      */
	float damp_filter_threshold; /**< Threshold of indoor probability
				      * change, which is an IIR parameter.
				      */
	float damping_coef_min;
				/* Minimum value of damping, which is an IIR
				 * parameter.
				 */
	float damping_coef_max;
				/* *< Maximum value of damping, which is an IIR
				 * parameter.
				 */
	float damping_coef_init; /**< Initial value of the ring-shaped buffer
				  * (multi-frame indoor probability).
				  */
	int32_t
	    exp_prior_filter_size_max;
					/* *< Ring buffer size of the AWB damping
					 * queue, which is an IIR parameter.
					 */
	int32_t exp_prior_filter_size_min; /**< IIR parameter, which is
					   * currently unused.
					   */
	float
	    exp_prior_middle; /**< IIR parameter, which is currently unused.*/
} cam_device_calib_sensor_awb_iir_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AWB global configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_awb_global_s {
	char name[CAMDEV_CALIB_LENGTH];	      /**< AWB header name */
	char resolution[CAMDEV_CALIB_LENGTH]; /**< Resolution */
	float svd_mean_value[3]; /**< Mean of singular value decomposition */
	float pca_matrix[6];	 /**< Transformation matrix of the PCA domain */
	float center_line[3]; /**< Position coordinate of first-order straight
			       * lines fitted from white points during AWB
			       * calibration.
			       */
	float af_rg2[16]; /**< Horizontal coordinate of the projection of the
			   * upper edge of the black box along the sampling
			   * point to the centerline during AWB calibration.
			   */
	float
	    af_max_dist2[16];
				/**< Distance between the horizontal coordinate
				 * of the sample point on the upper edge of the
				 * black box and af_rg2 during AWB calibration.
				 */
	float af_rg1[16]; /**< Horizontal coordinate of the projection of the
			   * sampling point to the centerline along the lower
			   * edge of the black box during AWB calibration.
			   */
	float
	    af_max_dist1[16];
			/**< Distance between the horizontal coordinate
			 * of the sampling point along the lower edge of
			 * the black box and af_rg1 during AWB calibration.
			 */
	float af_global_fade2[16]; /**< Horizontal coordinate of the projection
				    * of the upper edge of the orange box along
				    * the sampling point to the centerline
				    * during AWB calibration.
				    */
	float af_global_gain_distance2
	    [16]; /**< Distance between the horizontal coordinate of the
		    * sampling point on the upper edge of the orange box
		    * and af_global_fade2 during AWB calibration.
		    */
	float af_global_fade1[16]; /**< Horizontal coordinate of the projection
				    * of the sampling point to the centerline
				    * along the lower edge of the orange box
				    * during the AWB calibration.
				    */
	float af_global_gain_distance1
	    [16];
		/**< Distance between the horizontal coordinate of the
		 * sampling point on the upper edge of the orange box
		 * and af_global_fade1 during AWB calibration.
		 */
	float f_rg_proj_indoor_min; /**< Minimum Rg value in indoor scenes when
				     * the AWB clip box is calibrated.
				     */
	float f_rg_proj_max;
		/**< Maximum Rg boundary value of the black box
		 * when the AWB clip box is calibrated.
		 */
	float f_rg_proj_max_sky; /**< Maximum Rg boundary value of the orange
				  * box when the AWB clip box is calibrated.
				  */
	float f_rg_proj_outdoor_min; /**< Rg of indoor and outdoor dividing line
				      * when the AWB clip box is calibrated.
				      */
	char awb_clip_out_door[CAMDEV_CALIB_LENGTH]; /**< name of the light
						      * source which is the
						      * dividing line between
						      * indoor and outdoor
						      */
	float k_factor; /**< Sensor sensitivity coefficient, which determines
			 * whether the scene is indoor or outdoor
			 */
	float
	    af_fade2[6]; /**< Six coordinates on the Rg-Bg coordinate system. */
	int32_t af_cb_min_region_max[6]; /**< White points are located in the
					  * max range specified by cb.
					  */
	int32_t af_cr_min_region_max[6]; /**< White points are located in the
					  * max range specified by cr.
					  */
	int32_t
	    af_max_c_sum_region_max[6];
				/**< White points are located in the
				 * min range specified by cb and cr.
				 */
	int32_t af_cb_min_region_min[6];
				/**< White points are located in the
				 * min range specified by cb.
				 */
	int32_t af_cr_min_region_min[6];
				/**< White points are located in the
				 * min range specified by cr.
				 */
	int32_t af_max_c_sum_region_min[6]; /**< White points are located in the
					     * range specified by cb and cr.
					     */
	int32_t region_size; /**< Controls the proportional scaling of the white
			      * point area.
			      */
	float region_size_inc; /**< Increment used for adjusting the RegionSize
				* in the driver
				*/
	float region_size_dec; /**< Decrement used for adjusting the RegionSize
				* in the driver
				*/

	cam_device_calib_sensor_awb_iir_t awb_iir;
} cam_device_calib_sensor_awb_global_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AWB Illumination Gaussian-mixed-model
 *configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_awb_illumination_gmm_s {
	float inv_cov_matrix[4]; /**< InvCovMatrix: 2 x 2 inverse covariance
				  * matrix.
				  */
	float gaussian_scaling_factor; /**< GaussianScalingFactor: 1 x 1
					* Gaussian scaling factor
					*/
	float tau[2]; /**< Tau: 1 x 2 matrix, corresponding to the interpolation
		       * strategies tau1 and tau2 adjusted in the PCA domain
		       * during AWB calibration.
		       */
	float gaussian_mean_value[2]; /**< GaussianMeanValue: 1 x 2 matrix,
				       * Gaussian mean value.
				       */
} cam_device_calib_sensor_awb_illumination_gmm_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AWB Illumination  auto LSC
 *configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_awb_illumination_alsc_s {
	char resolution[CAMDEV_CALIB_LENGTH]; /**< Auto LSC resolution */
	char profile[CAMDEV_CALIB_LENGTH *
		     4]; /**< LSC profile corresponding to the light source */
} cam_device_calib_sensor_awb_illumination_alsc_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AWB Illumination saturation
 *configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_awb_illumination_sat_ct_s {
	int32_t
	    gain[4]; /* *< Saturation gains: a set of gains, 10 gains at most. */
	int32_t saturation[4]; /**< A set of saturation values corresponding to
				* gain values, 10 values at most.
				*/
} cam_device_calib_sensor_awb_illumination_sat_ct_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AWB Illumination LSC compensation
 *configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_awb_illumination_vig_ct_s {
	int32_t gain[4];       /**< A set of gains, five gains at most. */
	int32_t vignetting[4]; /**< A set of LSC compensation proportional
				* parameters corresponding to gain values, five
				* parameters at most.
				*/
} cam_device_calib_sensor_awb_illumination_vig_ct_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AWB Illumination auto CC
 *configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_awb_illumination_acc_s {
	char profile[CAMDEV_CALIB_LENGTH *
		     4]; /**< CC profile corresponding to the light source */
} cam_device_calib_sensor_awb_illumination_acc_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AWB Illumination configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_awb_illumination_s {
	char name[CAMDEV_CALIB_LENGTH];	     /**< name of the light source. */
	char door_type[CAMDEV_CALIB_LENGTH]; /**< Scene type, which includes
					      * indoor and outdoor.
					      */

	cam_device_calib_sensor_awb_illumination_gmm_t
	    gmm; /**< Illumination Gaussian-mixed-model. */
	cam_device_calib_sensor_awb_illumination_alsc_t
	    alsc; /**< Illumination  auto LSC configuration. */

	float
	    manual_wb[4]; /**< WB gain value calibrated for the light source. */
	float manual_cc_matrix[9]; /**< CC matrix value calibrated for the light
				    * source.
				    */
	float manual_cc_offset[3]; /**< CC offset value calibrated for the light
				    * source.
				    */

	char awb_type[CAMDEV_CALIB_LENGTH]; /**< AWB type, including AUTO and
					     * MANUAL.
					     */

	cam_device_calib_sensor_awb_illumination_sat_ct_t
	    sat_ct; /**< Illumination saturation configuration. */
	cam_device_calib_sensor_awb_illumination_vig_ct_t
	    vig_ct; /**< Illumination LSC compensation configuration. */
	cam_device_calib_sensor_awb_illumination_acc_t
	    acc; /**< Illumination auto color-correction configuration. */
} cam_device_calib_sensor_awb_illumination_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AWB configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_awb_s {
	cam_device_calib_sensor_awb_global_t
	    *p_globals; /**< Calibration sensor AWB global configuration. */
	uint16_t global_number; /**< Global configuration number. */

	cam_device_calib_sensor_awb_illumination_t
	    *p_illuminations;
				/* *< Calibration sensor AWB Illumination
				 * configuration
				 */
	uint16_t illumination_number; /**< Illumination configuration number */
} cam_device_calib_sensor_awb_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor LSC configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_lsc_s {
	char name[CAMDEV_CALIB_LENGTH];		/**< LSC header name. */
	char resolution[CAMDEV_CALIB_LENGTH];	/**< resolution. */
	char illumination[CAMDEV_CALIB_LENGTH]; /**< Illumination. */
	int32_t sectors;			/**< Number of Lens sectors. */
	int32_t no; /**< Corresponds to HW Repres in the Lens Shade Correction
		     * Tool.
		     */
	int32_t xo; /**< Corresponds to Horizontal multiplication-factors in the
		     * Lens Shade Correction Tool.
		     */
	int32_t yo; /**< Corresponds to Vertical multiplication-factors in the
		     * Lens Shade Correction Tool.
		     */
	int32_t sect_size_x[32]; /**< Horizontal spacing between nodes. Because
				  * of symmetry, only half of the data is
				  * recorded [1x8]..
				  */
	int32_t sect_size_y[32]; /**< Vertical spacing between nodes. Because of
				  * symmetry, only half of the data is recorded
				  * [1x8]..
				  */
	int32_t vignetting;	 /**< Proportion of compensation. */

	int32_t sample_red[33][33]; /**< Lens compensation coefficient for the
				     * red channel, [17 x 17] matrix.
				     */
	int32_t sample_green_r[33]
			      [33]; /**< LSC compensation coefficient of the
				      * green red channel, [17 x 17] matrix.
				      */
	int32_t sample_green_b[33]
			      [33]; /**< LSC compensation coefficient of the
				      * green blue channel, [17 x 17] matrix.
				      */
	int32_t sample_blue[33][33]; /**< LSC compensation coefficient of the
				       * blue channel, [17 x 17] matrix.
				       */
} cam_device_calib_sensor_lsc_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor CC configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_cc_s {
	char name[CAMDEV_CALIB_LENGTH]; /**< Color-correction header name. */
	float saturation;		/**< Saturation value. */
	float cc_matrix[9];		/**< CC matrices of the light source. */
	float cc_offset[3]; /**< CC offset matrices of the light source. */
	float wb[4];	    /**< WB gains of the light source. */
} cam_device_calib_sensor_cc_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AEC ECM priority Schemes
 *configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_priority_scheme_s {
	char name[CAMDEV_CALIB_LENGTH]; /**< Priority Schemes header name. */
	float offset_t0_fac;		/**< parameter. */
	float slope_a0; /**< Exponential weight of exposure decomposition
			  * for time and gain allocation.
			  */
} cam_device_calib_sensor_priority_scheme_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AEC exposure control mechanism(ECM)
 *configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_aec_ecm_s {
	char name[CAMDEV_CALIB_LENGTH]; /**< AEC header name. */
	cam_device_calib_sensor_priority_scheme_t
	    priority_scheme[3]; /**< Priority Schemes configuration. */
} cam_device_calib_sensor_aec_ecm_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AEC configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_aec_s {
	float set_point;       /**< Set point to hit by the AE control system */
	float clm_tolerance;   /**< AE adjustment tolerance value.. */
	float damp_over; /**< Convergence parameter for AE adjustment from
			   * overexposure to normal state. The smaller the
			   * value, the faster the convergence.
			   */
	float damp_under; /**< Convergence parameter for AE adjustment from
			    * underexposure to normal state. The smaller the
			    * value, the faster the convergence.
			    */
	float damp_over_video; /**< Parameter. */
	float damp_under_video; /**< Parameter. */

	cam_device_calib_sensor_aec_ecm_t
	    *p_ecm;	     /**< Calibration sensor AEC ECM configuration. */
	uint16_t ecm_number; /**< ECM configuration number. */

	float afps_max_gain; /**< AFPS max gain value. */
} cam_device_calib_sensor_aec_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor BLS configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_bls_s {
	char name[CAMDEV_CALIB_LENGTH];	      /**< BLS header name. */
	char resolution[CAMDEV_CALIB_LENGTH]; /**< Resolution. */
	int32_t data[4]; /**< BLS value of RAW Bayer channel. */
} cam_device_calib_sensor_bls_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor DEGAMMA configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_de_gamma_s {
	char name[CAMDEV_CALIB_LENGTH]; /**< DEGAMMA header name. */
	float dx[16]; /**< Brightness value before linearity correction. */
	float y[17];  /**< Brightness value after linearity correction. */
} cam_device_calib_sensor_de_gamma_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor WDR curve configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_wdr_curve_s {
	float x_val[33]; /**< X direction curve value. */
	float y_val[33]; /**< y direction curve value. */
} cam_device_calib_sensor_wdr_curve_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor WDR configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_wdr_s {
	cam_device_calib_sensor_wdr_curve_t curve1,
	    curve2; /**< WDR curve value. */
} cam_device_calib_sensor_wdr_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor CAC configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_cac_s {
	char name[CAMDEV_CALIB_LENGTH];	      /**< CAC header name.*/
	char resolution[CAMDEV_CALIB_LENGTH]; /**< Resolution. */
	float x_normal_shift;	  /**< Horizontal normalization shift */
	float x_normal_factor;	  /**< Horizontal normalization factor */
	float y_normal_shift;	  /**< Vertical normalization shift */
	float y_normal_factor;	  /**< Vertical normalization factor */
	float x_offset;		  /**< HCenterOffset */
	float y_offset;		  /**< VCenterOffset */
	float red_parameters[3];  /**< Coefficients A, b and C for red */
	float blue_parameters[3]; /**< Coefficients A, b and C for blue */
} cam_device_calib_sensor_cac_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor DPF configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_dpf_s {
	char name[CAMDEV_CALIB_LENGTH];	      /**< DPF header name. */
	char resolution[CAMDEV_CALIB_LENGTH]; /**< Resolution. */
	float
	    nll_segmentation; /**< Distribution method of sampling points on
				* the X-axis. 0: linear equidistance, 1:
				* logarithm.
				*/
	int32_t nll_coeff_n[17]; /**< Standard noise variances, which are a set
				  * of values derived from the Noise Calibration
				  * Tool.
				  */
	float sensor_gains;	 /**< Sensor gain */
	float nma;		 /**< Noise model slope of Bayer domain */
	float nmb;		 /**< Noise model intercept of Bayer domain */
	float sigma_green;    /**< Spatial filter sigma of the green channel. */
	float sigma_red_blue; /**< Spatial filter sigma of the red and blue
			       * channels.
			       */
	float gradient; /**< Default fixed value which can be tuned using the
			 * VSI Tuning Tool.
			 */
	float offset;	/**< Offset used by the driver to calculate the de-noise
			 * strength based on the exposure gain (SensorGain) in
			 * actual situation.
			 */
	float nl_gains[4]; /**< White balance gains for the red, Gr, Gb, and
			    * blue Channels.
			    */
} cam_device_calib_sensor_dpf_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor DPCC register configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_dpcc_register_s {
	char name[CAMDEV_CALIB_LENGTH];	 /**< Register. */
	char value[CAMDEV_CALIB_LENGTH]; /**< Register value. */
} cam_device_calib_sensor_dpcc_register_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor DPCC configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_dpcc_s {
	char name[CAMDEV_CALIB_LENGTH];	      /**< DPCC header name. */
	char resolution[CAMDEV_CALIB_LENGTH]; /**< Resolution. */

	uint32_t dpcc_mode;	     /**< DPCC mode */
	uint32_t dpcc_output_mode;   /**< DPCC output mode */
	uint32_t dpcc_set_use;	     /**< DPCC enable */
	uint32_t dpcc_methods_set1;  /**< DPCC set method 1 */
	uint32_t dpcc_methods_set2;  /**< DPCC set method 2 */
	uint32_t dpcc_methods_set3;  /**< DPCC set method 3 */
	uint32_t dpcc_line_thresh1;  /**< DPCC line threshold */
	uint32_t dpcc_line_mad_fac1; /**< DPCC line mad factor */
	uint32_t dpcc_pg_fac1;	     /**< DPCC page factor 1 */
	uint32_t dpcc_rnd_thresh1;   /**< DPCC RND threshold 1 */
	uint32_t dpcc_rg_fac1;	     /**< DPCC RG factor 1 */
	uint32_t dpcc_line_thresh2;  /**< DPCC line threshold 2 */
	uint32_t dpcc_line_mad_fac2; /**< DPCC line mad factor 2 */
	uint32_t dpcc_pg_fac2;	     /**< DPCC page factor 2 */
	uint32_t dpcc_rnd_thresh2;   /**< DPCC RND threshold 2 */
	uint32_t dpcc_rg_fac2;	     /**< DPCC RG factor 2 */
	uint32_t dpcc_line_thresh3;  /**< DPCC line threshold 3 */
	uint32_t dpcc_line_mad_fac3; /**< DPCC line mad factor 3 */
	uint32_t dpcc_pg_fac3;	     /**< DPCC page factor 3 */
	uint32_t dpcc_rnd_thresh3;   /**< DPCC RND threshold 3 */
	uint32_t dpcc_rg_fac3;	     /**< DPCC RG factor 3 */
	uint32_t dpcc_ro_limits;     /**< DPCC RO limits */
	uint32_t dpcc_rnd_offs;	     /**< DPCC RND offset */
} cam_device_calib_sensor_dpcc_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration sensor AEC configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_sensor_s {
	cam_device_calib_sensor_awb_t
	    awb; /**< Calibration sensor AWB configuration. */

	cam_device_calib_sensor_lsc_t
	    *p_lsc;	     /**< Calibration sensor LSC configuration. */
	uint16_t lsc_number; /**< LSC configure number. */

	cam_device_calib_sensor_cc_t
	    *p_cc;	    /**< Calibration sensor CC configuration. */
	uint16_t cc_number; /**< CC configure number. */

	cam_device_calib_sensor_aec_t
	    aec; /**< Calibration sensor AEC configuration. */

	cam_device_calib_sensor_bls_t
	    *p_bls;	     /**< Calibration sensor BLS configuration. */
	uint16_t bls_number; /**< BLS configure number. */

	cam_device_calib_sensor_de_gamma_t
	    *p_de_gamma; /**< Calibration sensor DEGAMMA configuration. */
	uint16_t de_gamma_number; /**< DEGAMMA configure number. */

	cam_device_calib_sensor_wdr_t
	    wdr; /**< Calibration sensor WDR configuration. */

	cam_device_calib_sensor_cac_t
	    *p_cac;	     /**< Calibration sensor CAC configuration. */
	uint16_t cac_number; /**< CAC configure number. */

	cam_device_calib_sensor_dpf_t
	    *p_dpf;	     /**< Calibration sensor DPF configuration. */
	uint16_t dpf_number; /**< DPF configure number. */

	cam_device_calib_sensor_dpcc_t
	    *p_dpcc;	      /**< Calibration sensor DPCC configuration. */
	uint16_t dpcc_number; /**< DPCC configure number. */
} cam_device_calib_sensor_t;

/******************************************************************************/
/**
 * @brief   Cam Device Calibration system configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_system_s {
	char afps_default[CAMDEV_CALIB_LENGTH]; /**< Auto FPS default value. */
} cam_device_calib_system_t;

/*****************************************************************************/
/**
 * @brief   Cam Device Calibration configuration.
 *
 *****************************************************************************/
typedef struct cam_device_calib_cfg_s {
	cam_device_calib_header_t
	    header; /**< Calibration header configuration. */
	cam_device_calib_sensor_t
	    sensor; /**< Calibration sensor configuration. */
	cam_device_calib_system_t
	    system; /**< Calibration system configuration. */
} cam_device_calib_cfg_t;

#ifdef __cplusplus
}
#endif

/* @} cam_device_calibration */

#endif // __CAMDEV_CALIBRATION_H__
