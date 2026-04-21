#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/videodev2.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/media-entity.h>
#include <media/v4l2-async.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-fwnode.h>

/* Dummy sensor capabilities */
#define DUMMY_SENSOR_NAME		"dummy-sensor"
#define DUMMY_SENSOR_WIDTH_MIN		640
#define DUMMY_SENSOR_WIDTH_MAX		3840
#define DUMMY_SENSOR_HEIGHT_MIN		480
#define DUMMY_SENSOR_HEIGHT_MAX		2160

struct dummy_sensor_mode {
	u32 width;
	u32 height;
	u32 code;
	u32 fps;
	const char *name;
};

/* Supported sensor modes */
static const struct dummy_sensor_mode dummy_modes[] = {
	{
		.width = 1920,
		.height = 1080,
		.code = MEDIA_BUS_FMT_SRGGB12_1X12,
		.fps = 30,
		.name = "1080p30",
	},
	{
		.width = 3840,
		.height = 2160,
		.code = MEDIA_BUS_FMT_SRGGB12_1X12,
		.fps = 30,
		.name = "4k30",
	},
	{
		.width = 1280,
		.height = 720,
		.code = MEDIA_BUS_FMT_SRGGB12_1X12,
		.fps = 60,
		.name = "720p60",
	},
};

struct dummy_sensor_dev {
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct device *dev;

	/* V4L2 controls */
	struct v4l2_ctrl_handler ctrl_handler;
	struct v4l2_ctrl *exposure;
	struct v4l2_ctrl *gain;

	/* Format and frame information */
	struct v4l2_mbus_framefmt format;
	struct v4l2_fract frame_interval;
	const struct dummy_sensor_mode *current_mode;

	/* Power and streaming state */
	bool powered;
	bool streaming;

	/* Optional resources (for real sensor compatibility) */
	struct clk *xclk;
	struct gpio_desc *reset_gpio;
	struct gpio_desc *powerdown_gpio;
	struct regulator *dovdd;
	struct regulator *avdd;
	struct regulator *dvdd;

	/* Endpoint info from device tree */
	struct v4l2_fwnode_endpoint endpoint;
	bool has_endpoint;
};

/* Power management */
static int dummy_sensor_power_on(struct dummy_sensor_dev *sensor)
{
	int ret = 0;

	if (sensor->powered)
		return 0;

	dev_info(sensor->dev, "Powering on dummy sensor\n");

	/* Enable regulators if available */
	if (sensor->dovdd) {
		ret = regulator_enable(sensor->dovdd);
		if (ret < 0) {
			dev_err(sensor->dev, "Failed to enable DOVDD: %d\n", ret);
			return ret;
		}
	}

	if (sensor->avdd) {
		ret = regulator_enable(sensor->avdd);
		if (ret < 0) {
			dev_err(sensor->dev, "Failed to enable AVDD: %d\n", ret);
			goto err_avdd;
		}
	}

	if (sensor->dvdd) {
		ret = regulator_enable(sensor->dvdd);
		if (ret < 0) {
			dev_err(sensor->dev, "Failed to enable DVDD: %d\n", ret);
			goto err_dvdd;
		}
	}

	/* Enable clock if available */
	if (sensor->xclk) {
		ret = clk_prepare_enable(sensor->xclk);
		if (ret < 0) {
			dev_err(sensor->dev, "Failed to enable clock: %d\n", ret);
			goto err_clk;
		}
	}

	/* Release reset if available */
	if (sensor->reset_gpio) {
		gpiod_set_value_cansleep(sensor->reset_gpio, 0);
		usleep_range(1000, 2000);
	}

	/* Release powerdown if available */
	if (sensor->powerdown_gpio) {
		gpiod_set_value_cansleep(sensor->powerdown_gpio, 0);
		usleep_range(1000, 2000);
	}

	sensor->powered = true;
	dev_info(sensor->dev, "Dummy sensor powered on successfully\n");

	return 0;

err_clk:
	if (sensor->dvdd)
		regulator_disable(sensor->dvdd);
err_dvdd:
	if (sensor->avdd)
		regulator_disable(sensor->avdd);
err_avdd:
	if (sensor->dovdd)
		regulator_disable(sensor->dovdd);

	return ret;
}

static int dummy_sensor_power_off(struct dummy_sensor_dev *sensor)
{
	if (!sensor->powered)
		return 0;

	dev_info(sensor->dev, "Powering off dummy sensor\n");

	/* Assert powerdown */
	if (sensor->powerdown_gpio)
		gpiod_set_value_cansleep(sensor->powerdown_gpio, 1);

	/* Assert reset */
	if (sensor->reset_gpio)
		gpiod_set_value_cansleep(sensor->reset_gpio, 1);

	/* Disable clock */
	if (sensor->xclk)
		clk_disable_unprepare(sensor->xclk);

	/* Disable regulators */
	if (sensor->dvdd)
		regulator_disable(sensor->dvdd);
	if (sensor->avdd)
		regulator_disable(sensor->avdd);
	if (sensor->dovdd)
		regulator_disable(sensor->dovdd);

	sensor->powered = false;
	dev_info(sensor->dev, "Dummy sensor powered off\n");

	return 0;
}

/* Sub-device core operations */
static int dummy_sensor_s_power(struct v4l2_subdev *sd, int on)
{
	struct dummy_sensor_dev *sensor = container_of(sd, struct dummy_sensor_dev, sd);

	dev_info(sensor->dev, "s_power: %s\n", on ? "ON" : "OFF");

	if (on)
		return dummy_sensor_power_on(sensor);
	else
		return dummy_sensor_power_off(sensor);
}

/* Sub-device video operations */
static int dummy_sensor_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct dummy_sensor_dev *sensor = container_of(sd, struct dummy_sensor_dev, sd);
	int ret = 0;

	/* Prominent entry log so stream-on/off is always visible in dmesg */
	dev_info(sensor->dev,
		 ">>> dummy_sensor s_stream(%d/%s) called: subdev='%s' powered=%s streaming=%s\n",
		 enable, enable ? "ON" : "OFF",
		 sd->name,
		 sensor->powered ? "yes" : "no",
		 sensor->streaming ? "yes" : "no");
	/* Idempotency: skip if already in the requested state */
	if (enable && sensor->streaming) {
		dev_info(sensor->dev,
			 "dummy_sensor s_stream(ON): already streaming, skipping (idempotent)\n");
		return 0;
	}
	if (!enable && !sensor->streaming) {
		dev_info(sensor->dev,
			 "dummy_sensor s_stream(OFF): not streaming, skipping (idempotent)\n");
		return 0;
	}
	if (enable) {
		if (sensor->streaming) {
			dev_dbg(sensor->dev, "dummy_sensor already streaming, skipping s_stream(ON)\n");
			return 0;
		}
		if (!sensor->powered) {
			ret = dummy_sensor_power_on(sensor);
			if (ret) {
				dev_err(sensor->dev,
					"dummy_sensor s_stream(ON) FAILED: power_on returned %d\n",
					ret);
				return ret;
			}
		}

		/* Simulate sensor initialization */
		dev_info(sensor->dev,
			 "dummy_sensor starting stream: mode='%s' %ux%u@%dfps\n",
			 sensor->current_mode->name,
			 sensor->current_mode->width,
			 sensor->current_mode->height,
			 sensor->current_mode->fps);

		sensor->streaming = true;
		dev_info(sensor->dev, "<<< dummy_sensor s_stream(ON) SUCCESS\n");
	} else {
		if (!sensor->streaming) {
			dev_dbg(sensor->dev, "dummy_sensor not streaming, skipping s_stream(OFF)\n");
			return 0;
		}
		dev_info(sensor->dev, "dummy_sensor stopping stream\n");
		sensor->streaming = false;
		dev_info(sensor->dev, "<<< dummy_sensor s_stream(OFF) done\n");
	}

	return 0;
}

/* Sub-device pad operations */
static int dummy_sensor_enum_mbus_code(struct v4l2_subdev *sd,
				       struct v4l2_subdev_state *state,
				       struct v4l2_subdev_mbus_code_enum *code)
{
	struct dummy_sensor_dev *sensor = container_of(sd, struct dummy_sensor_dev, sd);

	if (code->index >= ARRAY_SIZE(dummy_modes))
		return -EINVAL;

	code->code = dummy_modes[code->index].code;

	dev_dbg(sensor->dev, "enum_mbus_code[%d]: 0x%04x (%s)\n",
		code->index, code->code, dummy_modes[code->index].name);

	return 0;
}

static int dummy_sensor_enum_frame_size(struct v4l2_subdev *sd,
					struct v4l2_subdev_state *state,
					struct v4l2_subdev_frame_size_enum *fse)
{
	struct dummy_sensor_dev *sensor = container_of(sd, struct dummy_sensor_dev, sd);

	if (fse->index >= ARRAY_SIZE(dummy_modes))
		return -EINVAL;

	/* Check if the requested code matches any of our supported modes */
	if (fse->code != dummy_modes[fse->index].code)
		return -EINVAL;

	fse->min_width = fse->max_width = dummy_modes[fse->index].width;
	fse->min_height = fse->max_height = dummy_modes[fse->index].height;

	dev_dbg(sensor->dev, "enum_frame_size[%d]: %ux%u for code 0x%04x\n",
		fse->index, fse->max_width, fse->max_height, fse->code);

	return 0;
}

static int dummy_sensor_get_fmt(struct v4l2_subdev *sd,
				struct v4l2_subdev_state *state,
				struct v4l2_subdev_format *format)
{
	struct dummy_sensor_dev *sensor = container_of(sd, struct dummy_sensor_dev, sd);

	if (format->pad != 0)
		return -EINVAL;

	format->format = sensor->format;

	dev_dbg(sensor->dev, "get_fmt: %ux%u code:0x%x field:%u colorspace:%u\n",
		format->format.width, format->format.height,
		format->format.code, format->format.field,
		format->format.colorspace);

	return 0;
}

static int dummy_sensor_set_fmt(struct v4l2_subdev *sd,
				struct v4l2_subdev_state *state,
				struct v4l2_subdev_format *format)
{
	struct dummy_sensor_dev *sensor = container_of(sd, struct dummy_sensor_dev, sd);
	const struct dummy_sensor_mode *mode = NULL;
	int i;

	if (format->pad != 0)
		return -EINVAL;

	/* Find closest supported mode */
	for (i = 0; i < ARRAY_SIZE(dummy_modes); i++) {
		if (dummy_modes[i].width == format->format.width &&
		    dummy_modes[i].height == format->format.height &&
		    dummy_modes[i].code == format->format.code) {
			mode = &dummy_modes[i];
			break;
		}
	}

	/* If no exact match, use first mode as default */
	if (!mode) {
		mode = &dummy_modes[0];
		dev_info(sensor->dev, "Format not found, using default: %s\n", mode->name);
	}

	/* Update format with selected mode */
	format->format.width = mode->width;
	format->format.height = mode->height;
	format->format.code = mode->code;
	format->format.field = V4L2_FIELD_NONE;
	format->format.colorspace = V4L2_COLORSPACE_RAW;
	format->format.ycbcr_enc = V4L2_YCBCR_ENC_DEFAULT;
	format->format.quantization = V4L2_QUANTIZATION_DEFAULT;
	format->format.xfer_func = V4L2_XFER_FUNC_DEFAULT;

	/* Store the format and mode */
	sensor->format = format->format;
	sensor->current_mode = mode;

	/* Update frame interval */
	sensor->frame_interval.numerator = 1;
	sensor->frame_interval.denominator = mode->fps;

	dev_info(sensor->dev, "set_fmt: %s (%ux%u code:0x%x @%dfps)\n",
		 mode->name, format->format.width, format->format.height,
		 format->format.code, mode->fps);

	return 0;
}

static int dummy_sensor_get_frame_interval(struct v4l2_subdev *sd,
					   struct v4l2_subdev_state *state,
					   struct v4l2_subdev_frame_interval *fi)
{
	struct dummy_sensor_dev *sensor = container_of(sd, struct dummy_sensor_dev, sd);

	fi->interval = sensor->frame_interval;

	dev_dbg(sensor->dev, "get_frame_interval: %u/%u (%dfps)\n",
		fi->interval.numerator, fi->interval.denominator,
		fi->interval.denominator / fi->interval.numerator);

	return 0;
}

static int dummy_sensor_set_frame_interval(struct v4l2_subdev *sd,
					   struct v4l2_subdev_state *state,
					   struct v4l2_subdev_frame_interval *fi)
{
	struct dummy_sensor_dev *sensor = container_of(sd, struct dummy_sensor_dev, sd);
	const struct dummy_sensor_mode *best_mode = &dummy_modes[0];
	unsigned int best_fps_diff = UINT_MAX;
	unsigned int requested_fps, fps_diff;
	int i;

	if (fi->interval.numerator == 0) {
		fi->interval.numerator = 1;
		fi->interval.denominator = 30; /* Default to 30fps */
	}

	requested_fps = fi->interval.denominator / fi->interval.numerator;

	/* Find the closest matching frame rate */
	for (i = 0; i < ARRAY_SIZE(dummy_modes); i++) {
		fps_diff = abs((int)dummy_modes[i].fps - (int)requested_fps);
		if (fps_diff < best_fps_diff) {
			best_fps_diff = fps_diff;
			best_mode = &dummy_modes[i];
		}
	}

	/* Update frame interval and mode */
	sensor->frame_interval.numerator = 1;
	sensor->frame_interval.denominator = best_mode->fps;
	sensor->current_mode = best_mode;

	/* Return the actual frame interval */
	fi->interval = sensor->frame_interval;

	dev_info(sensor->dev, "set_frame_interval: requested %dfps, set %dfps (%s)\n",
		 requested_fps, best_mode->fps, best_mode->name);

	return 0;
}

/* V4L2 control operations */
static int dummy_sensor_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct dummy_sensor_dev *sensor = container_of(ctrl->handler,
							struct dummy_sensor_dev,
							ctrl_handler);

	switch (ctrl->id) {
	case V4L2_CID_EXPOSURE:
		dev_info(sensor->dev, "Set exposure: %d\n", ctrl->val);
		break;
	case V4L2_CID_GAIN:
		dev_info(sensor->dev, "Set gain: %d\n", ctrl->val);
		break;
	default:
		dev_err(sensor->dev, "Unsupported control: 0x%x\n", ctrl->id);
		return -EINVAL;
	}

	return 0;
}

static const struct v4l2_ctrl_ops dummy_sensor_ctrl_ops = {
	.s_ctrl = dummy_sensor_s_ctrl,
};

/* V4L2 subdev operations structures */
static const struct v4l2_subdev_core_ops dummy_sensor_core_ops = {
	.s_power = dummy_sensor_s_power,
};

static const struct v4l2_subdev_video_ops dummy_sensor_video_ops = {
	.s_stream = dummy_sensor_s_stream,
};

static const struct v4l2_subdev_pad_ops dummy_sensor_pad_ops = {
	.enum_mbus_code = dummy_sensor_enum_mbus_code,
	.enum_frame_size = dummy_sensor_enum_frame_size,
	.get_fmt = dummy_sensor_get_fmt,
	.set_fmt = dummy_sensor_set_fmt,
	.get_frame_interval = dummy_sensor_get_frame_interval,
	.set_frame_interval = dummy_sensor_set_frame_interval,
	.link_validate = v4l2_subdev_link_validate_default,
};

static const struct v4l2_subdev_ops dummy_sensor_ops = {
	.core = &dummy_sensor_core_ops,
	.video = &dummy_sensor_video_ops,
	.pad = &dummy_sensor_pad_ops,
};

/* Initialize V4L2 controls */
static int dummy_sensor_init_controls(struct dummy_sensor_dev *sensor)
{
	struct v4l2_ctrl_handler *ctrl_hdlr = &sensor->ctrl_handler;
	int ret;

	ret = v4l2_ctrl_handler_init(ctrl_hdlr, 2);
	if (ret)
		return ret;

	/* Exposure control */
	sensor->exposure = v4l2_ctrl_new_std(ctrl_hdlr, &dummy_sensor_ctrl_ops,
					     V4L2_CID_EXPOSURE,
					     1, 65535, 1, 1000);
	if (!sensor->exposure) {
		dev_err(sensor->dev, "Failed to create exposure control\n");
		ret = ctrl_hdlr->error ? ctrl_hdlr->error : -ENOMEM;
		goto error;
	}

	/* Gain control */
	sensor->gain = v4l2_ctrl_new_std(ctrl_hdlr, &dummy_sensor_ctrl_ops,
					 V4L2_CID_GAIN,
					 1, 256, 1, 16);
	if (!sensor->gain) {
		dev_err(sensor->dev, "Failed to create gain control\n");
		ret = ctrl_hdlr->error ? ctrl_hdlr->error : -ENOMEM;
		goto error;
	}

	if (ctrl_hdlr->error) {
		ret = ctrl_hdlr->error;
		dev_err(sensor->dev, "Control handler error: %d\n", ret);
		goto error;
	}

	sensor->sd.ctrl_handler = ctrl_hdlr;

	return 0;

error:
	v4l2_ctrl_handler_free(ctrl_hdlr);
	return ret;
}

/* Parse device tree endpoint */
static int dummy_sensor_parse_dt(struct dummy_sensor_dev *sensor)
{
	struct device *dev = sensor->dev;
	struct fwnode_handle *ep;
	int ret;

	/* Parse endpoint */
	ep = fwnode_graph_get_next_endpoint(dev_fwnode(dev), NULL);
	if (ep) {
		ret = v4l2_fwnode_endpoint_parse(ep, &sensor->endpoint);
		fwnode_handle_put(ep);

		if (ret < 0) {
			/*
			 * Endpoint parse failure is non-fatal for the dummy sensor.
			 * The DT endpoint may use Xilinx-specific properties that
			 * the generic parser doesn't understand.  Continue without
			 * endpoint metadata.
			 */
			dev_warn(dev, "Endpoint parse returned %d, continuing without endpoint info\n",
				 ret);
			sensor->has_endpoint = false;
		} else {
			sensor->has_endpoint = true;
			dev_info(dev, "Parsed endpoint: bus_type=%d, lanes=%d\n",
				 sensor->endpoint.bus_type,
				 sensor->endpoint.bus.mipi_csi2.num_data_lanes);
		}
	} else {
		dev_info(dev, "No endpoint found in device tree\n");
		sensor->has_endpoint = false;
	}

	return 0;
}

/* Get optional resources */
static int dummy_sensor_get_resources(struct dummy_sensor_dev *sensor)
{
	struct device *dev = sensor->dev;

	/* Optional clock */
	sensor->xclk = devm_clk_get_optional(dev, "xclk");
	if (IS_ERR(sensor->xclk)) {
		dev_info(dev, "No external clock available\n");
		sensor->xclk = NULL;
	}

	/* Optional GPIOs */
	sensor->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(sensor->reset_gpio)) {
		dev_info(dev, "No reset GPIO available\n");
		sensor->reset_gpio = NULL;
	}

	sensor->powerdown_gpio = devm_gpiod_get_optional(dev, "powerdown", GPIOD_OUT_HIGH);
	if (IS_ERR(sensor->powerdown_gpio)) {
		dev_info(dev, "No powerdown GPIO available\n");
		sensor->powerdown_gpio = NULL;
	}

	/* Optional regulators */
	sensor->dovdd = devm_regulator_get_optional(dev, "dovdd");
	if (IS_ERR(sensor->dovdd)) {
		dev_info(dev, "No DOVDD regulator available\n");
		sensor->dovdd = NULL;
	}

	sensor->avdd = devm_regulator_get_optional(dev, "avdd");
	if (IS_ERR(sensor->avdd)) {
		dev_info(dev, "No AVDD regulator available\n");
		sensor->avdd = NULL;
	}

	sensor->dvdd = devm_regulator_get_optional(dev, "dvdd");
	if (IS_ERR(sensor->dvdd)) {
		dev_info(dev, "No DVDD regulator available\n");
		sensor->dvdd = NULL;
	}

	return 0;
}

/* Probe function */
static int dummy_sensor_probe(struct platform_device *pdev)
{
	struct dummy_sensor_dev *sensor;
	int ret;

	sensor = devm_kzalloc(&pdev->dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;

	sensor->dev = &pdev->dev;
	platform_set_drvdata(pdev, sensor);

	/* Parse device tree */
	ret = dummy_sensor_parse_dt(sensor);
	if (ret)
		return ret;

	/* Get optional resources */
	ret = dummy_sensor_get_resources(sensor);
	if (ret)
		return ret;

	/* Initialize V4L2 subdevice */
	v4l2_subdev_init(&sensor->sd, &dummy_sensor_ops);
	strscpy(sensor->sd.name, DUMMY_SENSOR_NAME, sizeof(sensor->sd.name));
	sensor->sd.owner = THIS_MODULE;
	sensor->sd.dev = &pdev->dev;
	sensor->sd.fwnode = dev_fwnode(&pdev->dev);
	sensor->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE | V4L2_SUBDEV_FL_HAS_EVENTS;

	dev_info(&pdev->dev, "Dummy sensor: fwnode = %p, dev_name = %s\n",
		 sensor->sd.fwnode, dev_name(&pdev->dev));

	/* Initialize controls */
	ret = dummy_sensor_init_controls(sensor);
	if (ret) {
		dev_err(&pdev->dev, "Failed to initialize controls: %d\n", ret);
		goto err_cleanup;
	}

	/* Initialize default format and mode */
	sensor->current_mode = &dummy_modes[0]; /* Default to 1080p30 */
	sensor->format.width = sensor->current_mode->width;
	sensor->format.height = sensor->current_mode->height;
	sensor->format.code = sensor->current_mode->code;
	sensor->format.field = V4L2_FIELD_NONE;
	sensor->format.colorspace = V4L2_COLORSPACE_RAW;
	sensor->format.ycbcr_enc = V4L2_YCBCR_ENC_DEFAULT;
	sensor->format.quantization = V4L2_QUANTIZATION_DEFAULT;
	sensor->format.xfer_func = V4L2_XFER_FUNC_DEFAULT;

	/* Initialize frame interval */
	sensor->frame_interval.numerator = 1;
	sensor->frame_interval.denominator = sensor->current_mode->fps;

	/* Setup media entity and pad */
	sensor->pad.flags = MEDIA_PAD_FL_SOURCE;
	sensor->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	sensor->sd.entity.ops = NULL;

	ret = media_entity_pads_init(&sensor->sd.entity, 1, &sensor->pad);
	if (ret) {
		dev_err(&pdev->dev, "Failed to initialize media entity: %d\n", ret);
		goto err_free_ctrls;
	}

	/* Register async subdevice */
	ret = v4l2_async_register_subdev(&sensor->sd);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register async subdev: %d\n", ret);
		goto err_cleanup_entity;
	}

	dev_info(&pdev->dev, "Dummy sensor registered: entity_id=%u, name='%s', mode='%s'\n",
		 sensor->sd.entity.graph_obj.id, sensor->sd.name, sensor->current_mode->name);

	dev_info(&pdev->dev, "Dummy sensor initialization completed successfully\n");

	return 0;

err_cleanup_entity:
	media_entity_cleanup(&sensor->sd.entity);
err_free_ctrls:
	v4l2_ctrl_handler_free(&sensor->ctrl_handler);
err_cleanup:
	return ret;
}

/* Remove function */
static void dummy_sensor_remove(struct platform_device *pdev)
{
	struct dummy_sensor_dev *sensor = platform_get_drvdata(pdev);

	if (sensor) {
		/* Power off if powered */
		if (sensor->powered)
			dummy_sensor_power_off(sensor);

		/* Unregister and cleanup */
		v4l2_async_unregister_subdev(&sensor->sd);
		media_entity_cleanup(&sensor->sd.entity);
		v4l2_ctrl_handler_free(&sensor->ctrl_handler);

		dev_info(&pdev->dev, "Dummy sensor removed\n");
	}
}

/* Device tree compatibility */
static const struct of_device_id dummy_sensor_of_match[] = {
	{ .compatible = "dummy,sensor" },
	{ .compatible = "dummy,v4l2" },  /* Keep backward compatibility */
	{ .compatible = "dummy,camera-sensor" },
	{ }
};
MODULE_DEVICE_TABLE(of, dummy_sensor_of_match);

/* Platform driver structure */
static struct platform_driver dummy_sensor_driver = {
	.probe = dummy_sensor_probe,
	.remove = dummy_sensor_remove,
	.driver = {
		.name = "dummy-sensor",
		.of_match_table = dummy_sensor_of_match,
	},
};

module_platform_driver(dummy_sensor_driver);

MODULE_DESCRIPTION("Dummy Camera Sensor V4L2 Driver");
MODULE_AUTHOR("AMD VeriSilicon ISP Team");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:dummy-sensor");
