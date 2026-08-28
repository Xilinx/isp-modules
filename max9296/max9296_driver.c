/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Advanced Micro Devices, Inc.
 * Copyright (c) 2017-2019 Jacopo Mondi
 * Copyright (c) 2017-2019 Kieran Bingham
 * Copyright (c) 2017-2019 Laurent Pinchart
 * Copyright (c) 2017-2019 Niklas Söderlund
 * Copyright (c) 2016 Renesas Electronics Corporation
 * Copyright (c) 2015 Cogent Embedded, Inc.
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
 * Copyright (c) 2017-2019 Jacopo Mondi
 * Copyright (c) 2017-2019 Kieran Bingham
 * Copyright (c) 2017-2019 Laurent Pinchart
 * Copyright (c) 2017-2019 Niklas Söderlund
 * Copyright (c) 2016 Renesas Electronics Corporation
 * Copyright (c) 2015 Cogent Embedded, Inc.
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
 * Maxim MAX9296 GMSL2 Deserializer driver.
 *
 * Derived from the mainline Maxim GMSL (max9286) driver.
 *
 *****************************************************************************/

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fwnode.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_graph.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/regmap.h>
#include "max9296.h"

#include <media/v4l2-async.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-subdev.h>

/* Register 0x00 */
#define MAX9296_MSTLINKSEL_AUTO		(7 << 5)
#define MAX9296_MSTLINKSEL(n)		((n) << 5)
#define MAX9296_EN_VS_GEN		BIT(4)
#define MAX9296_LINKEN(n)		(1 << (n))
/* Register 0x01 */
#define MAX9296_FSYNCMODE_ECU		(3 << 6)
#define MAX9296_FSYNCMODE_EXT		(2 << 6)
#define MAX9296_FSYNCMODE_INT_OUT	(1 << 6)
#define MAX9296_FSYNCMODE_INT_HIZ	(0 << 6)
#define MAX9296_GPIEN			BIT(5)
#define MAX9296_ENLMO_RSTFSYNC		BIT(2)
#define MAX9296_FSYNCMETH_AUTO		(2 << 0)
#define MAX9296_FSYNCMETH_SEMI_AUTO	(1 << 0)
#define MAX9296_FSYNCMETH_MANUAL	(0 << 0)
#define MAX9296_REG_FSYNC_PERIOD_L	0x06
#define MAX9296_REG_FSYNC_PERIOD_M	0x07
#define MAX9296_REG_FSYNC_PERIOD_H	0x08
/* Register 0x0a */
#define MAX9296_FWDCCEN(n)		(1 << ((n) + 4))
#define MAX9296_REVCCEN(n)		(1 << (n))
/* Register 0x0c */
#define MAX9296_HVEN			BIT(7)
#define MAX9296_EDC_6BIT_HAMMING	(2 << 5)
#define MAX9296_EDC_6BIT_CRC		(1 << 5)
#define MAX9296_EDC_1BIT_PARITY		(0 << 5)
#define MAX9296_DESEL			BIT(4)
#define MAX9296_INVVS			BIT(3)
#define MAX9296_INVHS			BIT(2)
#define MAX9296_HVSRC_D0		(2 << 0)
#define MAX9296_HVSRC_D14		(1 << 0)
#define MAX9296_HVSRC_D18		(0 << 0)
/* Register 0x0f */
#define MAX9296_0X0F_RESERVED		BIT(3)
/* Register 0x12 */
#define MAX9296_CSILANECNT(n)		(((n) - 1) << 6)
#define MAX9296_CSIDBL			BIT(5)
#define MAX9296_DBL			BIT(4)
#define MAX9296_DATATYPE_USER_8BIT	(11 << 0)
#define MAX9296_DATATYPE_USER_YUV_12BIT	(10 << 0)
#define MAX9296_DATATYPE_USER_24BIT	(9 << 0)
#define MAX9296_DATATYPE_RAW14		(8 << 0)
#define MAX9296_DATATYPE_RAW11		(7 << 0)
#define MAX9296_DATATYPE_RAW10		(6 << 0)
#define MAX9296_DATATYPE_RAW8		(5 << 0)
#define MAX9296_DATATYPE_YUV422_10BIT	(4 << 0)
#define MAX9296_DATATYPE_YUV422_8BIT	(3 << 0)
#define MAX9296_DATATYPE_RGB555		(2 << 0)
#define MAX9296_DATATYPE_RGB565		(1 << 0)
#define MAX9296_DATATYPE_RGB888		(0 << 0)
/* Register 0x15 */
#define MAX9296_VC(n)			((n) << 5)
#define MAX9296_VCTYPE			BIT(4)
#define MAX9296_CSIOUTEN		BIT(3)
#define MAX9296_0X15_RESV		(3 << 0)
/* Register 0x1b */
#define MAX9296_SWITCHIN(n)		(1 << ((n) + 4))
#define MAX9296_ENEQ(n)			(1 << (n))
/* Register 0x27 */
#define MAX9296_LOCKED			BIT(7)
/* Register 0x31 */
#define MAX9296_FSYNC_LOCKED		BIT(6)
/* Register 0x34 */
#define MAX9296_I2CLOCACK		BIT(7)
#define MAX9296_I2CSLVSH_1046NS_469NS	(3 << 5)
#define MAX9296_I2CSLVSH_938NS_352NS	(2 << 5)
#define MAX9296_I2CSLVSH_469NS_234NS	(1 << 5)
#define MAX9296_I2CSLVSH_352NS_117NS	(0 << 5)
#define MAX9296_I2CMSTBT_837KBPS	(7 << 2)
#define MAX9296_I2CMSTBT_533KBPS	(6 << 2)
#define MAX9296_I2CMSTBT_339KBPS	(5 << 2)
#define MAX9296_I2CMSTBT_173KBPS	(4 << 2)
#define MAX9296_I2CMSTBT_105KBPS	(3 << 2)
#define MAX9296_I2CMSTBT_84KBPS		(2 << 2)
#define MAX9296_I2CMSTBT_28KBPS		(1 << 2)
#define MAX9296_I2CMSTBT_8KBPS		(0 << 2)
#define MAX9296_I2CSLVTO_NONE		(3 << 0)
#define MAX9296_I2CSLVTO_1024US		(2 << 0)
#define MAX9296_I2CSLVTO_256US		(1 << 0)
#define MAX9296_I2CSLVTO_64US		(0 << 0)
/* Register 0x3b */
#define MAX9296_REV_TRF(n)		((n) << 4)
#define MAX9296_REV_AMP(n)		((((n) - 30) / 10) << 1) /* in mV */
#define MAX9296_REV_AMP_X		BIT(0)
#define MAX9296_REV_AMP_HIGH		170
/* Register 0x3f */
#define MAX9296_EN_REV_CFG		BIT(6)
#define MAX9296_REV_FLEN(n)		((n) - 20)
/* Register 0x49 */
#define MAX9296_VIDEO_DETECT_MASK	0x0f
/* Register 0x69 */
#define MAX9296_LFLTBMONMASKED		BIT(7)
#define MAX9296_LOCKMONMASKED		BIT(6)
#define MAX9296_AUTOCOMBACKEN		BIT(5)
#define MAX9296_AUTOMASKEN		BIT(4)
#define MAX9296_MASKLINK(n)		((n) << 0)

/* Register 0x0313: MIPI CSI-2 output control; BIT(1) enables the output. */
#define MAX9296_REG_CSI_OUT_EN		0x0313
#define MAX9296_CSI_OUT_EN		BIT(1)

/*
 * The sink and source pads are created to match the OF graph port numbers so
 * that their indexes can be used interchangeably.
 */
#define MAX9296_NUM_GMSL		1
#define MAX9296_N_SINKS			1
#define MAX9296_N_PADS			2
#define MAX9296_SRC_PAD			1

struct max9296_source {
	struct v4l2_subdev *sd;
	struct fwnode_handle *fwnode;
	struct device *dev;
	struct regulator *regulator;

};

static const struct regmap_config deser_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_NONE,
};

static const struct regmap_config ser_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_NONE,
};

struct max9296_priv {
	struct device	*dev;
	struct i2c_client *client;
	struct i2c_client *ldac;
	struct i2c_client *regul;
	struct i2c_client *exp;
	struct i2c_client *deser;
	struct i2c_client *ser;
	struct i2c_client *fpga;

	struct regmap	*regulator_regmap;
	struct regmap   *ldac_regmap;
	struct regmap   *deser_regmap;
	struct regmap   *fpga_regmap;
	struct regmap   *ser_regmap;

	struct gpio_desc *gpiod_pwdn;
	struct v4l2_subdev sd;
	struct media_pad pads[MAX9296_N_PADS];
	struct regulator *regulator;
	u32    addrs[4];
	u32    link;
	u32    deser_id;
	u8     csi_out_val;
	struct gpio_chip gpio;
	u8 gpio_state;

	struct i2c_mux_core *mux;
	unsigned int mux_channel;
	bool mux_open;

	/* The initial reverse control channel amplitude. */
	u32 init_rev_chan_mv;
	u32 rev_chan_mv;

	struct v4l2_ctrl_handler ctrls;
	struct v4l2_ctrl *pixelrate;

	struct v4l2_mbus_framefmt fmt[MAX9296_N_PADS];

	/* Protects controls and fmt structures */
	struct mutex mutex;

	unsigned int nsources;
	unsigned int source_mask;
	unsigned int route_mask;
	unsigned int bound_sources;
	unsigned int csi2_data_lanes;
	struct max9296_source sources[MAX9296_NUM_GMSL];
	struct v4l2_async_notifier notifier;
};

static inline struct max9296_priv *sd_to_max9296(struct v4l2_subdev *sd)
{
	return container_of(sd, struct max9296_priv, sd);
}

static int max929x_write_reg(struct i2c_client *i2c_client, u8 slave_addr,
			     u16 reg, u8 val);

static int max9296_power_up(struct i2c_client *i2c_client)
{
	struct max9296_priv *priv = i2c_get_clientdata(i2c_client);
	struct i2c_client *client = i2c_client;
	struct device *dev = &client->dev;
	struct i2c_msg msgs[1];
	u8 send_buf[3];
	int ret;
	int back_addr = client->addr;

	client->addr = 0x2f;
	send_buf[0] = 0x00;
	send_buf[1] = 0x04;

	msgs[0].addr = client->addr;
	msgs[0].len = 2;
	msgs[0].buf = send_buf;
	msgs[0].flags = 0;

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs)) {
		dev_err(dev, "Failed to write register, ret = %d\n", ret);
		return ret < 0 ? ret : -EIO;
	}
	msleep(100);

	client->addr = 0x4c;
	send_buf[0] = 0x60;
	send_buf[1] = 0xff;
	send_buf[2] = 0xff;

	msgs[0].addr = client->addr;
	msgs[0].len = 3;
	msgs[0].buf = send_buf;
	msgs[0].flags = 0;

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs)) {
		dev_err(dev, "Failed to write register, ret = %d\n", ret);
		return ret < 0 ? ret : -EIO;
	}
	msleep(100);

	client->addr = 0x4c;
	send_buf[0] = 0x3f;
	send_buf[1] = 0x00;
	send_buf[2] = 0x00;

	msgs[0].addr = client->addr;
	msgs[0].len = 3;
	msgs[0].buf = send_buf;
	msgs[0].flags = 0;

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs)) {
		dev_err(dev, "Failed to write register, ret = %d\n", ret);
		return ret < 0 ? ret : -EIO;
	}
	msleep(100);

	client->addr = 0x20;
	send_buf[0] = 0x01;
	send_buf[1] = 0x00;

	msgs[0].addr = client->addr;
	msgs[0].len = 2;
	msgs[0].buf = send_buf;
	msgs[0].flags = 0;

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs)) {
		dev_err(dev, "Failed to write register, ret = %d\n", ret);
		return ret < 0 ? ret : -EIO;
	}
	msleep(800);

	client->addr = 0x20;
	send_buf[0] = 0x15;

	if (back_addr == 0x68)
		send_buf[1] = 0x02;

	if (back_addr == 0x4a)
		send_buf[1] = 0x0a;

	if (back_addr == 0x6c)
		send_buf[1] = 0x1a;

	if (back_addr == 0x4b)
		send_buf[1] = 0x4;

	msgs[0].addr = client->addr;
	msgs[0].len = 2;
	msgs[0].buf = send_buf;
	msgs[0].flags = 0;

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs)) {
		dev_err(dev, "Failed to write register, ret = %d\n", ret);
		return ret < 0 ? ret : -EIO;
	}
	ret = 0;
	msleep(100);
	client->addr = back_addr;

	return ret;
}

/* -----------------------------------------------------------------------------
 * V4L2 Subdev
 */

/**
 * max9296_s_stream - Enable or disable the deserializer CSI-2 output
 * @sd: Pointer to V4L2 sub-device structure
 * @enable: Non-zero to start streaming, zero to stop
 *
 * Toggle the MAX9296 CSI-2 output enable so frames are forwarded only while
 * streaming.
 *
 * Return: 0 on success, errors otherwise
 */
static int max9296_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct max9296_priv *priv = sd_to_max9296(sd);
	struct i2c_client *client = priv->client;
	u8 val;
	int ret;

	if (enable)
		val = priv->csi_out_val | MAX9296_CSI_OUT_EN;
	else
		val = priv->csi_out_val & ~MAX9296_CSI_OUT_EN;

	ret = max929x_write_reg(client, priv->addrs[0],
				MAX9296_REG_CSI_OUT_EN, val);
	if (ret)
		dev_err(&client->dev, "Failed to %s streaming: %d\n",
			enable ? "start" : "stop", ret);

	return ret;
}

/**
 * max9296_enum_mbus_code - Enumerate supported media bus codes
 * @sd: Pointer to V4L2 sub-device structure
 * @sd_state: Pointer to sub-device state structure
 * @code: Pointer to the media bus code enumeration
 *
 * Return: 0 on success, errors otherwise
 */
static int max9296_enum_mbus_code(struct v4l2_subdev *sd,
				  struct v4l2_subdev_state *sd_state,
				  struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->pad || code->index > 0)
		return -EINVAL;

	code->code = MEDIA_BUS_FMT_UYVY8_1X16;

	return 0;
}

static struct v4l2_mbus_framefmt *
max9296_get_pad_format(struct max9296_priv *priv,
		       struct v4l2_subdev_state *sd_state,
		       unsigned int pad, u32 which)
{
	if (pad >= MAX9296_N_PADS)
		return NULL;

	return &priv->fmt[pad];
}

/**
 * max9296_set_fmt - Set the pad format
 * @sd: Pointer to V4L2 sub-device structure
 * @sd_state: Pointer to sub-device state structure
 * @format: Pointer to the pad format to apply
 *
 * Return: 0 on success, errors otherwise
 */
static int max9296_set_fmt(struct v4l2_subdev *sd,
			   struct v4l2_subdev_state *sd_state,
			   struct v4l2_subdev_format *format)
{
	struct max9296_priv *priv = sd_to_max9296(sd);
	struct v4l2_mbus_framefmt *cfg_fmt;

	/* Refuse non YUV422 formats as we hardcode DT to 8 bit YUV422 */
	switch (format->format.code) {
	case MEDIA_BUS_FMT_UYVY8_1X16:
	case MEDIA_BUS_FMT_VYUY8_1X16:
	case MEDIA_BUS_FMT_YUYV8_1X16:
	case MEDIA_BUS_FMT_YVYU8_1X16:
		break;
	default:
		format->format.code = MEDIA_BUS_FMT_UYVY8_1X16;
		break;
	}

	cfg_fmt = max9296_get_pad_format(priv, sd_state, format->pad,
					 format->which);
	if (!cfg_fmt)
		return -EINVAL;

	mutex_lock(&priv->mutex);
	*cfg_fmt = format->format;
	mutex_unlock(&priv->mutex);

	return 0;
}

/**
 * max9296_get_fmt - Get the pad format
 * @sd: Pointer to V4L2 sub-device structure
 * @sd_state: Pointer to sub-device state structure
 * @format: Pointer to the pad format to fill
 *
 * Return: 0 on success, errors otherwise
 */
static int max9296_get_fmt(struct v4l2_subdev *sd,
			   struct v4l2_subdev_state *sd_state,
			   struct v4l2_subdev_format *format)
{
	struct max9296_priv *priv = sd_to_max9296(sd);
	struct v4l2_mbus_framefmt *cfg_fmt;
	unsigned int pad = format->pad;

	cfg_fmt = max9296_get_pad_format(priv, sd_state, pad, format->which);
	if (!cfg_fmt)
		return -EINVAL;

	mutex_lock(&priv->mutex);
	format->format = *cfg_fmt;
	mutex_unlock(&priv->mutex);

	return 0;
}

static const struct v4l2_subdev_video_ops max9296_video_ops = {
	.s_stream	= max9296_s_stream,
};

static const struct v4l2_subdev_pad_ops max9296_pad_ops = {
	.enum_mbus_code = max9296_enum_mbus_code,
	.get_fmt	= max9296_get_fmt,
	.set_fmt	= max9296_set_fmt,
};

static const struct v4l2_subdev_ops max9296_subdev_ops = {
	.video		= &max9296_video_ops,
	.pad		= &max9296_pad_ops,
};

static void max9296_init_format(struct v4l2_mbus_framefmt *fmt)
{
	fmt->width		= 1920;
	fmt->height		= 1080;
	fmt->code		= MEDIA_BUS_FMT_SGRBG12_1X12;
	fmt->colorspace		= V4L2_COLORSPACE_SRGB;
	fmt->field		= V4L2_FIELD_NONE;
	fmt->ycbcr_enc		= V4L2_YCBCR_ENC_DEFAULT;
	fmt->quantization	= V4L2_QUANTIZATION_DEFAULT;
	fmt->xfer_func		= V4L2_XFER_FUNC_DEFAULT;
}

/**
 * max9296_s_ctrl - Set a V4L2 control
 * @ctrl: Pointer to the V4L2 control to set
 *
 * Return: 0 on success, errors otherwise
 */
static int max9296_s_ctrl(struct v4l2_ctrl *ctrl)
{
	switch (ctrl->id) {
	case V4L2_CID_PIXEL_RATE:
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct v4l2_ctrl_ops max9296_ctrl_ops = {
	.s_ctrl = max9296_s_ctrl,
};

static int max9296_v4l2_register(struct max9296_priv *priv)
{
	struct device *dev = &priv->client->dev;
	int ret;
	int i;

	/* Configure V4L2 for the MAX9296 itself */
	for (i = 0; i < MAX9296_N_PADS; i++)
		max9296_init_format(&priv->fmt[i]);

	v4l2_i2c_subdev_init(&priv->sd, priv->client, &max9296_subdev_ops);

	priv->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

	v4l2_ctrl_handler_init(&priv->ctrls, 1);

	priv->pixelrate = v4l2_ctrl_new_std(&priv->ctrls,
					    &max9296_ctrl_ops,
					    V4L2_CID_PIXEL_RATE,
					    1, INT_MAX, 1, 50000000);

	priv->sd.ctrl_handler = &priv->ctrls;
	ret = priv->ctrls.error;
	if (ret)
		goto err_free_ctrls;

	priv->sd.entity.function = MEDIA_ENT_F_VID_IF_BRIDGE;
	priv->pads[MAX9296_SRC_PAD].flags = MEDIA_PAD_FL_SOURCE;

	for (i = 0; i < MAX9296_SRC_PAD; i++)
		priv->pads[i].flags = MEDIA_PAD_FL_SINK;

	ret = media_entity_pads_init(&priv->sd.entity, MAX9296_N_PADS,
				     priv->pads);
	if (ret)
		goto err_free_ctrls;

	ret = v4l2_async_register_subdev(&priv->sd);
	if (ret < 0) {
		dev_err(dev, "Unable to register subdevice\n");
		goto err_free_entity;
	}

	return 0;

err_free_entity:
	media_entity_cleanup(&priv->sd.entity);
err_free_ctrls:
	v4l2_ctrl_handler_free(&priv->ctrls);

	return ret;
}

static void max9296_v4l2_unregister(struct max9296_priv *priv)
{
	v4l2_async_unregister_subdev(&priv->sd);
	v4l2_ctrl_handler_free(&priv->ctrls);
	media_entity_cleanup(&priv->sd.entity);
}

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
	struct device *dev = &client->dev;
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
		dev_err(dev, "Failed to write register, ret = %d\n", ret);
		return ret < 0 ? ret : -EIO;
	}
	ret = 0;

	msleep(50);

	client->addr = back_addr;

	return ret;
}

static int max9296_init(struct device *dev)
{
	struct max9296_priv *priv;
	struct i2c_client *client;
	int ret = 0;

	client = to_i2c_client(dev);
	priv = i2c_get_clientdata(client);

	/*
	 * Register all V4L2 interactions for the MAX9296 and notifiers for
	 * any subdevices connected.
	 */
	ret = max9296_v4l2_register(priv);
	if (ret)
		dev_err(dev, "Failed to register with V4L2\n");

	return ret;
}

/**
 * max9296_probe - Probe the MAX9296 deserializer
 * @client: Pointer to the I2C client
 *
 * Power up the deserializer, apply the raw12/raw10 initialisation table
 * selected by the DT "stream" property and register the V4L2 sub-device.
 *
 * Return: 0 on success, errors otherwise
 */
static int max9296_probe(struct i2c_client *client)
{
	struct max9296_priv *priv;
	int ret = 0, i = 0;
	int cam_link = 0;
	int deser_num = 0;
	const char *stream_type;

	priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = &client->dev;
	priv->client = client;

	i2c_set_clientdata(client, priv);

	mutex_init(&priv->mutex);

	/* For des id, slave and link */
	ret = of_property_read_u32_array(client->dev.of_node, "reg", priv->addrs, 4);
	if (ret < 0)
		dev_err(&client->dev, "Invalid DT reg property: %d\n", ret);

	ret = of_property_read_u32(client->dev.of_node, "link", &cam_link);
	if (ret < 0)
		dev_err(&client->dev, "Invalid DT link property: %d\n", ret);
	priv->link = cam_link;

	ret = of_property_read_u32(client->dev.of_node, "deser_num", &deser_num);
	if (ret < 0)
		dev_err(&client->dev, "Invalid DT deser_num property: %d\n", ret);
	priv->deser_id = deser_num;

	ret = of_property_read_string(client->dev.of_node, "stream", &stream_type);
	if (ret < 0) {
		dev_err(&client->dev, "Invalid DT stream property: %d\n", ret);
		goto err_destroy_mutex;
	}

	ret = max9296_power_up(client);
	if (ret) {
		dev_err(&client->dev, "Power-up failed: %d\n", ret);
		goto err_destroy_mutex;
	}
	msleep(200);

	/* Init MAX9296 */
	if (!strcmp(stream_type, "raw12")) {
		priv->csi_out_val = 0x62;
		for (i = 0; ; i++) {
			if (deserializer_initialization_raw12[i].addr == MAX9296_TABLE_END)
				break;

			if (deserializer_initialization_raw12[i].addr == MAX9296_TABLE_WAIT) {
				msleep(deserializer_initialization_raw12[i].val);
				continue;
			}

			if (client->addr == 0x4b)
				priv->addrs[0] = 0x4a;

			ret = max929x_write_reg(client, priv->addrs[0],
						deserializer_initialization_raw12[i].addr,
						deserializer_initialization_raw12[i].val);
			if (ret)
				goto err_destroy_mutex;
		}
	} else if (!strcmp(stream_type, "raw10")) {
		priv->csi_out_val = 0x52;
		for (i = 0; ; i++) {
			if (deserializer_initialization_raw10[i].addr == MAX9296_TABLE_END)
				break;

			if (deserializer_initialization_raw10[i].addr == MAX9296_TABLE_WAIT) {
				msleep(deserializer_initialization_raw10[i].val);
				continue;
			}

			if (client->addr == 0x4b)
				priv->addrs[0] = 0x4a;

			ret = max929x_write_reg(client, priv->addrs[0],
						deserializer_initialization_raw10[i].addr,
						deserializer_initialization_raw10[i].val);
			if (ret)
				goto err_destroy_mutex;
		}
	}

	if (client->addr == 0x4b) {
		struct i2c_msg msgs[1];
		u8 send_buf[3];
		int back_addr = client->addr;

		client->addr = 0x20;

		send_buf[0] = 0x15;
		send_buf[1] = 0x1e;

		msgs[0].addr = client->addr;
		msgs[0].len = 2;
		msgs[0].buf = send_buf;
		msgs[0].flags = 0;

		ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
		if (ret != ARRAY_SIZE(msgs)) {
			ret = ret < 0 ? ret : -EIO;
			goto err_destroy_mutex;
		}

		dev_info(priv->dev, "enable all deserializer");
		msleep(100);
		client->addr = back_addr;
	}

	msleep(1000);

	ret = max9296_init(&client->dev);
	if (ret < 0)
		goto err_destroy_mutex;

	dev_info(priv->dev, "max9296 probe success\n");
	return 0;

err_destroy_mutex:
	mutex_destroy(&priv->mutex);
	return ret;
}

/**
 * max9296_remove - Remove the MAX9296 deserializer
 * @client: Pointer to the I2C client
 *
 * Unregister the V4L2 sub-device and release all acquired resources.
 */
static void max9296_remove(struct i2c_client *client)
{
	struct max9296_priv *priv = i2c_get_clientdata(client);

	max9296_v4l2_unregister(priv);
	mutex_destroy(&priv->mutex);
}

static const struct of_device_id max9296_dt_ids[] = {
	{ .compatible = "maxim,max9296" },
	{},
};
MODULE_DEVICE_TABLE(of, max9296_dt_ids);

static struct i2c_driver max9296_i2c_driver = {
	.driver		= {
		.name		= "max9296",
		.of_match_table	= of_match_ptr(max9296_dt_ids),
	},
	.probe	= max9296_probe,
	.remove		= max9296_remove,
};

module_i2c_driver(max9296_i2c_driver);

MODULE_DESCRIPTION("Maxim MAX9296 GMSL Deserializer Driver");
MODULE_AUTHOR("Amit Verma <amit.verma@amd.com>");
MODULE_LICENSE("GPL");
