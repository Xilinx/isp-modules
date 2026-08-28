/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Advanced Micro Devices, Inc.
 * Copyright (c) 2022 Leopard Imaging, Inc.
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
 * Copyright (c) 2026 Advanced Micro Devices, Inc.
 * Copyright (c) 2022 Leopard Imaging, Inc.
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
 *****************************************************************************
 *
 * OX03F10 CMOS Image Sensor driver.
 *
 * Derived from the Leopard Imaging OX03F10 driver by Weicen Zhou
 * <weicenz@leopardimaging.com>.
 *
 *****************************************************************************/

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>

#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include "ox03f10_tables.h"
#include <linux/err.h>

#include "visp_sensor_common.h"

/* regulator supplies */
static const char * const ox03f10_supply_names[] = {
	"vddl",  /* IF (1.2V) supply */
	"vdig",  /* Digital Core (1.8V) supply */
	"vana",  /* Analog (2.8V) supply */
};

#define OX03F10_NUM_SUPPLIES ARRAY_SIZE(ox03f10_supply_names)

static const struct regmap_config ox03f10_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_NONE,
};

/*
 * Parameters for each ox03f10 readout mode.
 *
 * These are the values to configure the sensor in one of the
 * implemented modes.
 *
 * @init_regs: registers to initialize the mode
 * @wbin_ratio: width downscale factor (e.g. 3 for 1280; 3 = 1920/1280)
 * @hbin_ratio: height downscale factor (e.g. 3 for 720; 3 = 1080/720)
 * @min_frame_len: Minimum frame length for each mode (see "Frame Rate
 *                 Adjustment (CSI-2)" in the datasheet)
 * @min_SHR: Minimum SHR register value (see "Shutter Setting (CSI-2)" in the
 *           datasheet)
 * @max_fps: Maximum frames per second
 * @nocpiop: Number of clocks per internal offset period (see "Integration Time
 *           in Each Readout Drive Mode (CSI-2)" in the datasheet)
 */
struct ox03f10_mode {
	const struct reg_8 *init_regs;
	u8 wbin_ratio;
	u8 hbin_ratio;
	int min_frame_len;
	int min_SHR;
	int max_fps;
	int nocpiop;
};

/* nocpiop happens to be the same number for the implemented modes */
static const struct ox03f10_mode ox03f10_modes[] = {
	{
		/* mode 1, 2K */
		.wbin_ratio = 1, /* 1920 */
		.hbin_ratio = 1, /* 1080 */
		.init_regs = ox03f10_mode1_1920x1080_raw12_37fps,
		.min_frame_len = 0x330,
		.min_SHR = 12,
		.max_fps = 37,
		.nocpiop = 112,
	},
};

/*
 * struct ox03f10_ctrls - ox03f10 ctrl structure
 * @handler: V4L2 ctrl handler structure
 * @exposure: Pointer to expsure ctrl structure
 * @gain: Pointer to gain ctrl structure
 */
struct ox03f10_ctrls {
	struct v4l2_ctrl_handler handler;
	struct v4l2_ctrl *exposure;
	struct v4l2_ctrl *gain;
};

/**
 * struct max9295_device - max9295 serializer device
 * @client: The i2c client for the max9295 instance
 */
struct max9295_device {
	struct i2c_client *client;
};

/*
 * struct st_ox03f10 - ox03f10 device structure
 * @sd: V4L2 subdevice structure
 * @pad: Media pad structure
 * @client: Pointer to I2C client
 * @ctrls: ox03f10 control structure
 * @crop: rect to be captured
 * @compose: compose rect, i.e. output resolution
 * @format: V4L2 media bus frame format structure
 *          (width and height are in sync with the compose rect)
 * @frame_rate: V4L2 frame rate structure
 * @regmap: Pointer to regmap structure
 * @reset_gpio: Pointer to reset gpio
 * @supplies: List of analog and digital supply regulators
 * @inck: Pointer to sensor input clock
 * @lock: Mutex structure
 * @mode: Parameters for the selected readout mode
 */
struct st_ox03f10 {
	struct device			*dev;
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct i2c_client *client;
	struct ox03f10_ctrls ctrls;

	struct max9295_device serializer;
	struct i2c_client	  *sensor;
	u32				addrs[2];

	int deser_id;

	struct v4l2_rect crop;
	struct v4l2_mbus_framefmt format;
	struct v4l2_fract frame_interval;
	struct regmap *regmap;
	struct gpio_desc *reset_gpio;
	struct regulator_bulk_data supplies[OX03F10_NUM_SUPPLIES];
	struct clk *inck;
	struct mutex lock; /* mutex lock for operations */
	const struct ox03f10_mode *mode;
};

#define OX03F10_ROUND(dim, step, flags)			\
	((flags) & V4L2_SEL_FLAG_GE			\
	 ? roundup((dim), (step))			\
	 : ((flags) & V4L2_SEL_FLAG_LE			\
	    ? rounddown((dim), (step))			\
	    : rounddown((dim) + (step) / 2, (step))))

/*
 * Function declaration
 */
static int ox03f10_set_gain(struct st_ox03f10 *priv, struct v4l2_ctrl *ctrl);
static int ox03f10_set_exposure(struct st_ox03f10 *priv, int val);

/*
 * v4l2_ctrl and v4l2_subdev related operations
 */
static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
	return &container_of(ctrl->handler,
			     struct st_ox03f10, ctrls.handler)->sd;
}

static inline struct st_ox03f10 *to_ox03f10(struct v4l2_subdev *sd)
{
	return container_of(sd, struct st_ox03f10, sd);
}

/*
 * Writing a register table
 *
 * @priv: Pointer to device
 * @table: Table containing register values (with optional delays)
 *
 * This is used to write register table into sensor's reg map.
 *
 * Return: 0 on success, errors otherwise
 */
static int ox03f10_write_table(struct st_ox03f10 *priv, const struct reg_8 table[])
{
	struct regmap *regmap = priv->regmap;
	int err = 0;
	int i = 0;

	for (i = 0; ; i++) {
		if (table[i].addr == OX03F10_TABLE_WAIT) {
			msleep(table[i].val);
			continue;
		}
		if (table[i].addr == OX03F10_TABLE_END)
			break;
		err = regmap_write(regmap, table[i].addr, table[i].val);
		if (err)
			return err;
	}

	return 0;
}

static inline int ox03f10_write_reg(struct st_ox03f10 *priv, u16 addr, u8 val)
{
	int err;

	err = regmap_write(priv->regmap, addr, val);
	if (err)
		dev_err(priv->dev,
			"%s : i2c write failed, %x = %x\n", __func__,
			addr, val);

	return err;
}

static inline int ox03f10_read_reg(struct st_ox03f10 *priv, u16 addr, u8 *val)
{
	int err;
	uint32_t reg_val = 0;

	err = regmap_read(priv->regmap, addr, &reg_val);
	if (err)
		dev_err(priv->dev,
			"%s : i2c read failed, %x = %x\n", __func__,
			addr, reg_val);

	*val = (uint8_t)reg_val;
	return err;
}

/*
 * ox03f10_start_stream - Function for starting stream per mode index
 * @priv: Pointer to device structure
 *
 * Return: 0 on success, errors otherwise
 */
static int ox03f10_start_stream(struct st_ox03f10 *priv)
{
	return ox03f10_write_table(priv, ox03f10_start);
}

/**
 * ox03f10_power_on - Runtime PM resume callback
 * @dev: Pointer to the device structure
 *
 * Return: 0 on success, errors otherwise
 */
static int ox03f10_power_on(struct device *dev)
{
	return 0;
}

/**
 * ox03f10_power_off - Runtime PM suspend callback
 * @dev: Pointer to the device structure
 *
 * Return: 0 on success, errors otherwise
 */
static int ox03f10_power_off(struct device *dev)
{
	return 0;
}

/**
 * ox03f10_s_ctrl - This is used to set the ox03f10 V4L2 controls
 * @ctrl: V4L2 control to be set
 *
 * This function is used to set the V4L2 controls for the ox03f10 sensor.
 *
 * Return: 0 on success, errors otherwise
 */
static int ox03f10_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct st_ox03f10 *ox03f10 = to_ox03f10(sd);
	int ret = -EINVAL;

	dev_dbg(&ox03f10->client->dev,
		"%s : s_ctrl: %s, value: %d\n", __func__,
		ctrl->name, ctrl->val);

	switch (ctrl->id) {
	case V4L2_CID_EXPOSURE:
		dev_dbg(&ox03f10->client->dev,
			"%s : set V4L2_CID_EXPOSURE\n", __func__);
		ret = ox03f10_set_exposure(ox03f10, ctrl->val);
		break;

	case V4L2_CID_GAIN:
		dev_dbg(&ox03f10->client->dev,
			"%s : set V4L2_CID_GAIN\n", __func__);
		ret = ox03f10_set_gain(ox03f10, ctrl);
		break;
	}

	return ret;
}

static int ox03f10_binning_goodness(struct st_ox03f10 *ox03f10,
				    int w, int ask_w,
				   int h, int ask_h, u32 flags)
{
	struct device *dev = &ox03f10->client->dev;
	const int goodness = 100000;
	int val = 0;

	if (flags & V4L2_SEL_FLAG_GE) {
		if (w < ask_w)
			val -= goodness;
		if (h < ask_h)
			val -= goodness;
	}

	if (flags & V4L2_SEL_FLAG_LE) {
		if (w > ask_w)
			val -= goodness;
		if (h > ask_h)
			val -= goodness;
	}

	val -= abs(w - ask_w);
	val -= abs(h - ask_h);

	dev_dbg(dev, "%s: ask %dx%d, size %dx%d, goodness %d\n",
		__func__, ask_w, ask_h, w, h, val);

	return val;
}

/**
 * __ox03f10_change_compose - Helper function to change binning and set both
 *	compose and format.
 *
 * We have two entry points to change binning: set_fmt and
 * set_selection(COMPOSE). Both have to compute the new output size
 * and set it in both the compose rect and the frame format size. We
 * also need to do the same things after setting cropping to restore
 * 1:1 binning.
 *
 * This function contains the common code for these three cases, it
 * has many arguments in order to accommodate the needs of all of
 * them.
 *
 * Must be called with ox03f10->lock locked.
 *
 * @ox03f10: The device object
 * @sd_state: The subdev state we are editing for TRY requests
 * @which:  V4L2_SUBDEV_FORMAT_ACTIVE or V4L2_SUBDEV_FORMAT_TRY from the caller
 * @width:  Input-output parameter: set to the desired width before
 *          the call, contains the chosen value after returning successfully
 * @height: Input-output parameter for height (see @width)
 * @flags:  Selection flags from struct v4l2_subdev_selection, or 0 if not
 *          available (when called from set_fmt)
 */
static int __ox03f10_change_compose(struct st_ox03f10 *ox03f10,
				    struct v4l2_subdev_state *sd_state,
				   u32 which,
				   u32 *width,
				   u32 *height,
				   u32 flags)
{
	struct device *dev = &ox03f10->client->dev;
	const struct v4l2_rect *cur_crop;
	struct v4l2_mbus_framefmt *tgt_fmt;
	unsigned int i;
	const struct ox03f10_mode *best_mode = &ox03f10_modes[0];
	int best_goodness = INT_MIN;

	if (which == V4L2_SUBDEV_FORMAT_TRY) {
		//cur_crop = &sd_state->pads->try_crop;
		//tgt_fmt = &sd_state->pads->try_fmt;
	} else {
		cur_crop = &ox03f10->crop;
		tgt_fmt = &ox03f10->format;
	}

	for (i = 0; i < ARRAY_SIZE(ox03f10_modes); i++) {
		u8 wratio = ox03f10_modes[i].wbin_ratio;
		u8 hratio = ox03f10_modes[i].hbin_ratio;

		int goodness = ox03f10_binning_goodness(
			ox03f10,
			cur_crop->width / wratio, *width,
			cur_crop->height / hratio, *height,
			flags);

		if (goodness >= best_goodness) {
			best_goodness = goodness;
			best_mode = &ox03f10_modes[i];
		}
	}

	*width = cur_crop->width / best_mode->wbin_ratio;
	*height = cur_crop->height / best_mode->hbin_ratio;

	if (which == V4L2_SUBDEV_FORMAT_ACTIVE)
		ox03f10->mode = best_mode;

	dev_dbg(dev, "%s: selected %ux%u binning\n",
		__func__, best_mode->wbin_ratio, best_mode->hbin_ratio);

	tgt_fmt->width = *width;
	tgt_fmt->height = *height;
	tgt_fmt->field = V4L2_FIELD_NONE;

	return 0;
}

/**
 * ox03f10_get_fmt - Get the pad format
 * @sd: Pointer to V4L2 Sub device structure
 * @sd_state: Pointer to sub device state structure
 * @fmt: Pointer to pad level media bus format
 *
 * This function is used to get the pad format information.
 *
 * Return: 0 on success
 */
static int ox03f10_get_fmt(struct v4l2_subdev *sd,
			   struct v4l2_subdev_state *sd_state,
			  struct v4l2_subdev_format *fmt)
{
	struct st_ox03f10 *ox03f10 = to_ox03f10(sd);

	mutex_lock(&ox03f10->lock);
	fmt->format = ox03f10->format;
	mutex_unlock(&ox03f10->lock);
	return 0;
}

/**
 * ox03f10_set_fmt - This is used to set the pad format
 * @sd: Pointer to V4L2 Sub device structure
 * @sd_state: Pointer to sub device state information structure
 * @format: Pointer to pad level media bus format
 *
 * This function is used to set the pad format.
 *
 * Return: 0 on success
 */
static int ox03f10_set_fmt(struct v4l2_subdev *sd,
			   struct v4l2_subdev_state *sd_state,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *fmt = &format->format;
	struct st_ox03f10 *ox03f10 = to_ox03f10(sd);
	int err = 0;

	mutex_lock(&ox03f10->lock);

	err = __ox03f10_change_compose(ox03f10, sd_state, format->which,
				       &fmt->width, &fmt->height, 0);

	if (err)
		goto out;

	/*
	 * __ox03f10_change_compose already set width and height in the
	 * applicable format, but we need to keep all other format
	 * values, so do a full copy here
	 */
	fmt->field = V4L2_FIELD_NONE;
	if (format->which != V4L2_SUBDEV_FORMAT_TRY)
		ox03f10->format = *fmt;
out:
	mutex_unlock(&ox03f10->lock);

	return err;
}

/**
 * ox03f10_get_selection - Get a selection rectangle
 * @sd: Pointer to V4L2 sub-device structure
 * @sd_state: Pointer to sub-device state structure
 * @sel: Pointer to the selection to query
 *
 * Return the requested crop or compose rectangle for the sensor.
 *
 * Return: 0 on success, errors otherwise
 */
static int ox03f10_get_selection(struct v4l2_subdev *sd,
				 struct v4l2_subdev_state *sd_state,
				struct v4l2_subdev_selection *sel)
{
	struct st_ox03f10 *ox03f10 = to_ox03f10(sd);
	const struct v4l2_rect *src_crop;
	const struct v4l2_mbus_framefmt *src_fmt;
	int ret = 0;

	if (sel->pad != 0)
		return -EINVAL;

	if (sel->target == V4L2_SEL_TGT_CROP_BOUNDS) {
		sel->r.left = 0;
		sel->r.top = 0;
		sel->r.width = OX03F10_MAX_WIDTH;
		sel->r.height = OX03F10_MAX_HEIGHT;
		return 0;
	}

	if (sel->which == V4L2_SUBDEV_FORMAT_TRY) {
		//src_crop = &sd_state->pads->try_crop;
		//src_fmt = &sd_state->pads->try_fmt;
	} else {
		src_crop = &ox03f10->crop;
		src_fmt = &ox03f10->format;
	}

	mutex_lock(&ox03f10->lock);

	switch (sel->target) {
	case V4L2_SEL_TGT_CROP:
		sel->r = *src_crop;
		break;
	case V4L2_SEL_TGT_COMPOSE_BOUNDS:
		sel->r.top = 0;
		sel->r.left = 0;
		sel->r.width = src_crop->width;
		sel->r.height = src_crop->height;
		break;
	case V4L2_SEL_TGT_COMPOSE:
		sel->r.top = 0;
		sel->r.left = 0;
		sel->r.width = src_fmt->width;
		sel->r.height = src_fmt->height;
		break;
	default:
		ret = -EINVAL;
	}

	mutex_unlock(&ox03f10->lock);

	return ret;
}

static int ox03f10_set_selection_crop(struct st_ox03f10 *ox03f10,
				      struct v4l2_subdev_state *sd_state,
				     struct v4l2_subdev_selection *sel)
{
	struct v4l2_rect *tgt_crop;
	struct v4l2_rect new_crop;
	bool size_changed;

	/*
	 * h_step could be 12 or 24 depending on the binning. But we
	 * won't know the binning until we choose the mode later in
	 * __ox03f10_change_compose(). Thus let's be safe and use the
	 * most conservative value in all cases.
	 */
	const u32 h_step = 24;

	new_crop.width = min_t(u32,
			       OX03F10_ROUND(sel->r.width, h_step, sel->flags),
			       OX03F10_MAX_WIDTH);

	/* Constraint: HTRIMMING_END - HTRIMMING_START >= 144 */
	if (new_crop.width < 144)
		new_crop.width = 144;

	new_crop.left = min_t(u32,
			      OX03F10_ROUND(sel->r.left, h_step, 0),
			      OX03F10_MAX_WIDTH - new_crop.width);

	new_crop.height = min_t(u32,
				OX03F10_ROUND(sel->r.height, 2, sel->flags),
				OX03F10_MAX_HEIGHT);

	new_crop.top = min_t(u32, OX03F10_ROUND(sel->r.top, 2, 0),
			     OX03F10_MAX_HEIGHT - new_crop.height);

	sel->r = new_crop;

	if (sel->which != V4L2_SUBDEV_FORMAT_TRY)
		tgt_crop = &ox03f10->crop;
	mutex_lock(&ox03f10->lock);

	size_changed = (new_crop.width != tgt_crop->width ||
			new_crop.height != tgt_crop->height);

	/* __ox03f10_change_compose needs the new size in *tgt_crop */
	*tgt_crop = new_crop;

	/* if crop size changed then reset the output image size */
	if (size_changed)
		__ox03f10_change_compose(ox03f10, sd_state, sel->which,
					 &new_crop.width, &new_crop.height,
					sel->flags);

	mutex_unlock(&ox03f10->lock);

	return 0;
}

/**
 * ox03f10_set_selection - Set a selection rectangle
 * @sd: Pointer to V4L2 sub-device structure
 * @sd_state: Pointer to sub-device state structure
 * @sel: Pointer to the selection to apply
 *
 * Apply the crop or compose rectangle requested by the caller.
 *
 * Return: 0 on success, errors otherwise
 */
static int ox03f10_set_selection(struct v4l2_subdev *sd,
				 struct v4l2_subdev_state *sd_state,
				struct v4l2_subdev_selection *sel)
{
	struct st_ox03f10 *ox03f10 = to_ox03f10(sd);

	if (sel->pad != 0)
		return -EINVAL;

	if (sel->target == V4L2_SEL_TGT_CROP)
		return ox03f10_set_selection_crop(ox03f10, sd_state, sel);

	if (sel->target == V4L2_SEL_TGT_COMPOSE) {
		int err;

		mutex_lock(&ox03f10->lock);
		err =  __ox03f10_change_compose(ox03f10, sd_state, sel->which,
						&sel->r.width, &sel->r.height,
					       sel->flags);
		mutex_unlock(&ox03f10->lock);

		/*
		 * __ox03f10_change_compose already set width and
		 * height in set->r, we still need to set top-left
		 */
		if (!err) {
			sel->r.top = 0;
			sel->r.left = 0;
		}

		return err;
	}

	return -EINVAL;
}

/**
 * ox03f10_load_default - load default control values
 * @priv: Pointer to device structure
 *
 * Return: 0 on success, errors otherwise
 */
static void ox03f10_load_default(struct st_ox03f10 *priv)
{
	/* load default control values */
	priv->frame_interval.numerator = 1;
	priv->frame_interval.denominator = OX03F10_DEF_FRAME_RATE;
	priv->ctrls.exposure->val = 1000000 / OX03F10_DEF_FRAME_RATE;
	priv->ctrls.gain->val = OX03F10_DEF_GAIN;
}

/**
 * ox03f10_s_stream - It is used to start/stop the streaming.
 * @sd: V4L2 Sub device
 * @on: Flag (True / False)
 *
 * This function controls the start or stop of streaming for the
 * ox03f10 sensor.
 *
 * Return: 0 on success, errors otherwise
 */
static int ox03f10_s_stream(struct v4l2_subdev *sd, int on)
{
	struct st_ox03f10 *ox03f10 = to_ox03f10(sd);
	int ret = 0;

	dev_dbg(&ox03f10->client->dev, "%s : %s, mode index = %td\n", __func__,
		on ? "Stream Start" : "Stream Stop",
		ox03f10->mode - &ox03f10_modes[0]);

	mutex_lock(&ox03f10->lock);

	if (on) {
		/* start stream */
		ret = ox03f10_start_stream(ox03f10);
		if (ret)
			goto fail;
	} else {
		/* stop stream */
		ret = ox03f10_write_table(ox03f10, ox03f10_stop);
		if (ret)
			goto fail;
	}

	mutex_unlock(&ox03f10->lock);
	return 0;

fail:
	mutex_unlock(&ox03f10->lock);
	dev_err(&ox03f10->client->dev, "s_stream failed\n");
	return ret;
}

/*
 * ox03f10_set_gain - Function called when setting gain
 * @priv: Pointer to device structure
 * @val: Value of gain. the real value = val << OX03F10_GAIN_SHIFT;
 * @ctrl: v4l2 control pointer
 *
 * Set the gain based on input value.
 * The caller should hold the mutex lock ox03f10->lock if necessary
 *
 * Return: 0 on success
 */
static int ox03f10_set_gain(struct st_ox03f10 *priv, struct v4l2_ctrl *ctrl)
{
	int err;
	u32 gain;

	gain = (u32)(ctrl->val);

	if (gain > OX03F10_MAX_GAIN)
		gain = OX03F10_MAX_GAIN;
	if (gain < OX03F10_MIN_GAIN)
		gain = OX03F10_MIN_GAIN;

	err = ox03f10_write_reg(priv, OX03F10_GAIN_H, gain & 0xff);
	if (err) {
		dev_err(&priv->client->dev, "VFLIP control error\n");
		return err;
	}

	return err;
}

/*
 * ox03f10_set_coarse_time - Function called when setting SHR value
 * @priv: Pointer to device structure
 * @val: Value for exposure time in number of line_length, or [HMAX]
 *
 * Set SHR value based on input value.
 *
 * Return: 0 on success
 */
static int ox03f10_set_coarse_time(struct st_ox03f10 *priv, u32 coarse_time)
{
	int err = 0;

	if (coarse_time > priv->mode->min_frame_len)
		coarse_time = priv->mode->min_frame_len;
	err = ox03f10_write_reg(priv, OX03F10_EXPOSURE_H, (coarse_time >> 8) & 0xff);
	if (err) {
		dev_err(&priv->client->dev, "%s: write error\n", __func__);
		return err;
	}

	err = ox03f10_write_reg(priv, OX03F10_EXPOSURE_L, coarse_time & 0xff);
	if (err) {
		dev_err(&priv->client->dev, "%s: write error\n", __func__);
		return err;
	}

	return err;
}

/*
 * ox03f10_set_exposure - Function called when setting exposure time
 * @priv: Pointer to device structure
 * @val: Variable for exposure time, in the unit of micro-second
 *
 * Set exposure time based on input value.
 * The caller should hold the mutex lock ox03f10->lock if necessary
 *
 * Return: 0 on success
 */
static int ox03f10_set_exposure(struct st_ox03f10 *priv, int val)
{
	int err;
	u32 coarse_time; /* exposure time in unit of line (HMAX)*/

	dev_dbg(&priv->client->dev,
		"%s : EXPOSURE control input = %d\n", __func__, val);

	coarse_time = val * priv->mode->min_frame_len * OX03F10_DEF_FRAME_RATE / 1000000;

	err = ox03f10_set_coarse_time(priv, coarse_time);
	if (err)
		return err;

	return 0;
}

static int ox03f10_query_capability(struct st_ox03f10 *priv, void *arg)
{
	struct v4l2_capability *pcap = (struct v4l2_capability *)arg;

	strcpy((char *)pcap->driver, "ox03f10");
	sprintf((char *)pcap->bus_info, "deserid%d", priv->deser_id);

	return 0;
}

/**
 * ox03f10_ioctl - Handle sensor-specific V4L2 sub-device ioctls
 * @sd: Pointer to V4L2 sub-device structure
 * @cmd: ioctl command
 * @arg: ioctl argument
 *
 * Handles VIDIOC_QUERYCAP and the VISP sensor register get/set ioctls.
 *
 * Return: 0 on success, errors otherwise
 */
static long ox03f10_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct st_ox03f10 *ox03f10 = to_ox03f10(sd);
	struct visp_sensor_reg *reg;

	if (!ox03f10)
		return -1;

	switch (cmd) {
	case VIDIOC_QUERYCAP:
		ox03f10_query_capability(ox03f10, arg);
		break;

	case VISP_SENSOR_S_REGISTER:
		reg = (struct visp_sensor_reg *)arg;
		ox03f10_write_reg(ox03f10, reg->addr, (uint8_t)reg->val);
		break;

	case VISP_SENSOR_G_REGISTER:
		reg = (struct visp_sensor_reg *)arg;
		ox03f10_read_reg(ox03f10, reg->addr, (uint8_t *)&reg->val);
		break;

	default:
		break;
	}

	return 0;
}

static const struct v4l2_subdev_core_ops ox03f10_core_ops = {
	.ioctl = ox03f10_ioctl,
};

static const struct v4l2_subdev_pad_ops ox03f10_pad_ops = {
	.get_fmt = ox03f10_get_fmt,
	.set_fmt = ox03f10_set_fmt,
	.get_selection = ox03f10_get_selection,
	.set_selection = ox03f10_set_selection,
};

static const struct v4l2_subdev_video_ops ox03f10_video_ops = {
	.s_stream = ox03f10_s_stream,
};

static const struct v4l2_subdev_ops ox03f10_subdev_ops = {
	.core = &ox03f10_core_ops,
	.pad = &ox03f10_pad_ops,
	.video = &ox03f10_video_ops,
};

static const struct v4l2_ctrl_ops ox03f10_ctrl_ops = {
	.s_ctrl	= ox03f10_s_ctrl,
};

static const struct of_device_id ox03f10_of_id_table[] = {
	{ .compatible = "leopard,ox03f10" },
	{ }
};
MODULE_DEVICE_TABLE(of, ox03f10_of_id_table);

static const struct i2c_device_id ox03f10_id[] = {
	{ "OX03F10", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ox03f10_id);

/*
 * max929x_write_reg - Function called when write register to GMSL2 device
 * @i2c_client: Pointer to i2c_client device
 * @slave_addr: device slave addr
 * @reg: device register addr
 * @val: register val
 *
 * write val to gmsl2 device.
 *
 * Return: 0 on success
 */
static int max929x_write_reg(struct i2c_client *i2c_client, u8 slave_addr, u16 reg, u8 val)
{
	struct i2c_client *client = i2c_client;
	struct i2c_msg msgs[1];
	u8 send_buf[3];
	int ret;
	int back_addr = client->addr;

	client->addr = slave_addr;
	send_buf[0] = reg >> 8;
	send_buf[1] = reg & 0xff;
	send_buf[2] = val & 0xff;

	msgs[0].addr = client->addr;
	msgs[0].len = 3;
	msgs[0].buf = send_buf;
	msgs[0].flags = 0;

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));

	if (ret != ARRAY_SIZE(msgs)) {
		client->addr = back_addr;
		dev_err(&client->dev, "Serializer write failed\n");
		return ret < 0 ? ret : -EIO;
	}
	ret = 0;
	msleep(50);

	dev_dbg(&client->dev, " slave_addr:0x%x reg:0x%x val:0x%x\n",
		slave_addr, reg, val);

	client->addr = back_addr;
	return ret;
}

/**
 * ox03f10_probe - Probe the OX03F10 sensor
 * @client: Pointer to the I2C client
 *
 * Initialise the MAX9295 serializer, program the sensor I2C alias address,
 * and register the V4L2 sub-device, controls and media pad.
 *
 * Return: 0 on success, errors otherwise
 */
static int ox03f10_probe(struct i2c_client *client)
{
	struct v4l2_subdev *sd;
	struct st_ox03f10 *ox03f10;
	int ret;
	int gain = 2;
	int i = 0;
	int cam_link;
	int deser_num = -1;

	/* initialize ox03f10 */
	ox03f10 = devm_kzalloc(&client->dev, sizeof(*ox03f10), GFP_KERNEL);
	if (!ox03f10)
		return -ENOMEM;

	ox03f10->dev = &client->dev;

	mutex_init(&ox03f10->lock);

	ox03f10->serializer.client = client;
	ret = of_property_read_u32_array(client->dev.of_node, "reg",
					 ox03f10->addrs, 2);

	if (ret < 0) {
		dev_err(ox03f10->dev, "Invalid DT reg property: %d\n", ret);
		ret = -EINVAL;
		goto err_destroy_mutex;
	}

	ret = of_property_read_u32(client->dev.of_node, "link", &cam_link);
	cam_link = 1;

	/* get deser_num from device tree */
	ret = of_property_read_u32(client->dev.of_node, "deser_num", &deser_num);
	if (ret < 0)
		dev_err(&client->dev, "Invalid DT deser_num property: %d\n", ret);
	ox03f10->deser_id = deser_num;

	/* Verify communication with the MAX9295: wake it from its default address. */
	msleep(500);
	i2c_smbus_read_byte(ox03f10->serializer.client);

	/*
	 * Initialize the MAX9295 with its default address, then move it to a
	 * per-sensor translation address so multi-instance setups do not clash.
	 */
	for (i = 0; ; i++) {
		if (serializer_initialization_a[i].addr == MAX9295_TABLE_END ||
		    serializer_initialization_b[i].addr == MAX9295_TABLE_END)
			break;
		if (serializer_initialization_a[i].addr == MAX9295_TABLE_WAIT ||
		    serializer_initialization_b[i].addr == MAX9295_TABLE_WAIT) {
			msleep(serializer_initialization_a[i].val);
			continue;
		}

		if (cam_link == 1)
			ret = max929x_write_reg(ox03f10->serializer.client, 0x62,
						serializer_initialization_a[i].addr,
						serializer_initialization_a[i].val);
		else
			ret = max929x_write_reg(ox03f10->serializer.client, 0x62,
						serializer_initialization_b[i].addr,
						serializer_initialization_b[i].val);
		if (ret) {
			dev_info(ox03f10->dev, "serializer write failed\n");
			ret = -EPROBE_DEFER;
			goto err_destroy_mutex;
		}
	}

	ret = max929x_write_reg(ox03f10->serializer.client, 0x62, 0x0000,
				ox03f10->addrs[0] << 1);

	/* Check that the new slave (alias) address is reachable. */
	ret = max929x_write_reg(ox03f10->serializer.client, ox03f10->addrs[0],
				0x0042, ox03f10->addrs[1] << 1);
	dev_info(ox03f10->dev, "serializer alias source address set\n");

	ret = max929x_write_reg(ox03f10->serializer.client, ox03f10->addrs[0],
				0x0043, 0x36 << 1);
	dev_info(ox03f10->dev, "serializer alias destination address set\n");

	if (ret < 0) {
		dev_info(ox03f10->dev, "set address failed!\n");
		ret = -EPROBE_DEFER;
		goto err_destroy_mutex;
	}

	/*
	 * Once the alias address is set, use it as the client address.
	 * This value is the first "reg" entry (index 0) in the DT node.
	 */
	/* Create the dummy I2C client for the sensor. */
	ox03f10->sensor = i2c_new_dummy_device(client->adapter,
					       ox03f10->addrs[1]);
	if (IS_ERR(ox03f10->sensor)) {
		ret = PTR_ERR(ox03f10->sensor);
		goto err_destroy_mutex;
	}

	/* initialize format */
	ox03f10->mode = &ox03f10_modes[0];
	ox03f10->crop.width = OX03F10_MAX_WIDTH;
	ox03f10->crop.height = OX03F10_MAX_HEIGHT;
	ox03f10->format.width = ox03f10->crop.width / ox03f10->mode->wbin_ratio;
	ox03f10->format.height = ox03f10->crop.height / ox03f10->mode->hbin_ratio;
	ox03f10->format.field = V4L2_FIELD_NONE;
	ox03f10->format.code = MEDIA_BUS_FMT_SGRBG12_1X12;
	ox03f10->format.colorspace = V4L2_COLORSPACE_SRGB;
	ox03f10->frame_interval.numerator = 1;
	ox03f10->frame_interval.denominator = OX03F10_DEF_FRAME_RATE;

	/* initialize regmap */
	ox03f10->regmap = devm_regmap_init_i2c(ox03f10->sensor, &ox03f10_regmap_config);
	if (IS_ERR(ox03f10->regmap)) {
		dev_err(&client->dev,
			"regmap init failed: %ld\n", PTR_ERR(ox03f10->regmap));
		ret = -ENODEV;
		goto err_unregister_sensor;
	}

	ret = ox03f10_write_reg(ox03f10, OX03F10_GAIN_H, gain & 0xff);
	if (ret) {
		dev_info(ox03f10->dev, "ox03f10 sensor write failed !\n");
		ret = -EPROBE_DEFER;
		goto err_unregister_sensor;
	}

	dev_info(ox03f10->dev, "ox03f10 sensor write success !\n");

	/* initialize subdevice */
	/*
	 * Use the primary client (alias address) rather than the dummy sensor
	 * client so driver-wide changes stay minimal.
	 */
	ox03f10->client = client;

	/* without this v4l2 setting be an issue and cause no call for ops */
	ox03f10->client->addr = ox03f10->addrs[0];  /* changed on 27th March */

	sd = &ox03f10->sd;

	v4l2_i2c_subdev_init(sd, client, &ox03f10_subdev_ops);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE | V4L2_SUBDEV_FL_HAS_EVENTS;

	/* initialize subdev media pad */
	ox03f10->pad.flags = MEDIA_PAD_FL_SOURCE;
	sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret = media_entity_pads_init(&sd->entity, 1, &ox03f10->pad);
	if (ret < 0) {
		dev_err(&client->dev,
			"%s : media entity init Failed %d\n", __func__, ret);
		goto err_unregister_sensor;
	}

	/* initialize controls */
	ret = v4l2_ctrl_handler_init(&ox03f10->ctrls.handler, 2);
	if (ret < 0) {
		dev_err(&client->dev,
			"%s : ctrl handler init Failed\n", __func__);
		goto err_free_entity;
	}

	ox03f10->ctrls.handler.lock = &ox03f10->lock;

	/* add new controls */

	ox03f10->ctrls.gain = v4l2_ctrl_new_std(
		&ox03f10->ctrls.handler,
		&ox03f10_ctrl_ops,
		V4L2_CID_GAIN, OX03F10_MIN_GAIN,
		OX03F10_MAX_GAIN, 1,
		OX03F10_DEF_GAIN);

	ox03f10->ctrls.exposure = v4l2_ctrl_new_std(
		&ox03f10->ctrls.handler,
		&ox03f10_ctrl_ops,
		V4L2_CID_EXPOSURE, OX03F10_MIN_EXPOSURE_TIME,
		1000000 / OX03F10_DEF_FRAME_RATE, 1,
		OX03F10_MIN_EXPOSURE_TIME);

	ox03f10->sd.ctrl_handler = &ox03f10->ctrls.handler;
	if (ox03f10->ctrls.handler.error) {
		ret = ox03f10->ctrls.handler.error;
		goto err_free_ctrls;
	}
	/* load default control values */
	ox03f10_load_default(ox03f10);

	usleep_range(10000, 15000);
	/* register subdevice */
	ret = v4l2_async_register_subdev(sd);
	if (ret < 0) {
		dev_err(&client->dev,
			"%s : v4l2_async_register_subdev failed %d\n",
			__func__, ret);
		goto err_free_ctrls;
	}

	dev_info(&client->dev, "ox03f10 : ox03f10 probe success !\n");

	return 0;

err_free_ctrls:
	v4l2_ctrl_handler_free(&ox03f10->ctrls.handler);
err_free_entity:
	media_entity_cleanup(&sd->entity);
err_unregister_sensor:
	i2c_unregister_device(ox03f10->sensor);
err_destroy_mutex:
	mutex_destroy(&ox03f10->lock);
	return ret;
}

/**
 * ox03f10_remove - Remove the OX03F10 sensor
 * @client: Pointer to the I2C client
 *
 * Unregister the V4L2 sub-device and release all acquired resources.
 */
static void ox03f10_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct st_ox03f10 *ox03f10 = to_ox03f10(sd);

	ox03f10_power_off(&client->dev);

	v4l2_async_unregister_subdev(sd);
	v4l2_ctrl_handler_free(&ox03f10->ctrls.handler);

	media_entity_cleanup(&sd->entity);
	i2c_unregister_device(ox03f10->sensor);
	mutex_destroy(&ox03f10->lock);
}

static const struct dev_pm_ops ox03f10_pm_ops = {
	SET_RUNTIME_PM_OPS(ox03f10_power_off, ox03f10_power_on, NULL)
};

static struct i2c_driver ox03f10_i2c_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.pm = &ox03f10_pm_ops,
		.of_match_table	= ox03f10_of_id_table,
	},
	.probe	= ox03f10_probe,
	.remove		= ox03f10_remove,
	.id_table	= ox03f10_id,
};

module_i2c_driver(ox03f10_i2c_driver);

MODULE_AUTHOR("Amit Verma <amit.verma@amd.com>");
MODULE_DESCRIPTION("OX03F10 CMOS Image Sensor and max9295 serializer driver");
MODULE_LICENSE("GPL v2");

