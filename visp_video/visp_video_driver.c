/****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
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
 *****************************************************************************/
#include "visp_video_driver.h"

#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of_graph.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/videobuf2-dma-contig.h>

#include "visp_video_register.h"

static int visp_video_register_ports(struct visp_media_dev *visp_mdev)
{
	int i = 0;
	int ret = 0;
	for (i = 0; i < visp_mdev->ports; i++)
	{
		if (visp_mdev->video_params[i].m2m == false)
		{
			ret = visp_video_register(visp_mdev, i);
			if (ret)
			{
				goto err_register_video;
			}
		}
	}
	return 0;

err_register_video:
	for (i = 0; i < visp_mdev->ports; i++)
	{
		if (visp_mdev->video_params[i].m2m == false)
			visp_video_unregister(visp_mdev, i);
	}

	return ret;
}

static int visp_video_unregister_ports(struct visp_media_dev *visp_mdev)
{
	int i;

	for (i = 0; i < visp_mdev->ports; i++)
	{
		if (visp_mdev->video_params[i].m2m == false)
			visp_video_unregister(visp_mdev, i);
	}

	return 0;
}

static int visp_video_notifier_bound(struct v4l2_async_notifier *notifier,
									 struct v4l2_subdev *sd,
									 struct v4l2_async_connection *asc)
{
	struct visp_media_dev *visp_mdev =
		container_of(notifier, struct visp_media_dev, notifier);
	struct device *dev = visp_mdev->dev;
	struct fwnode_handle *ep = NULL;
	struct v4l2_fwnode_link link;
	struct media_entity *source, *sink;
	unsigned int source_pad, sink_pad;
	struct visp_video_dev *visp_vdev;
	struct video_device *vdev;
	int ret;

	while (1)
	{
		ep = fwnode_graph_get_next_endpoint(sd->fwnode, ep);
		if (!ep) break;
		ret = v4l2_fwnode_parse_link(ep, &link);
		if (ret < 0)
		{
			dev_err(dev, "failed to parse link for %pOF: %d\n", to_of_node(ep),
					ret);
			continue;
		}

		if (sd->entity.pads[link.local_port].flags == MEDIA_PAD_FL_SINK)
			continue;

		visp_vdev = visp_mdev->video_devs[link.remote_port];
		vdev = visp_vdev->video;
		source = &sd->entity;
		source_pad = link.local_port;
		sink = &vdev->entity;
		sink_pad = 0;
		v4l2_fwnode_put_link(&link);
		ret = media_create_pad_link(source, source_pad, sink, sink_pad,
									MEDIA_LNK_FL_ENABLED);
		if (ret)
		{
			dev_err(dev, "failed to create %s:%u -> %s:%u link\n", source->name,
					source_pad, sink->name, sink_pad);
			break;
		}
	}
	fwnode_handle_put(ep);

	return 0;
}

static void visp_video_notifier_unbound(struct v4l2_async_notifier *notifier,
										struct v4l2_subdev *sd,
										struct v4l2_async_connection *asc)
{
	return;
}

static int visp_video_notifier_complete(struct v4l2_async_notifier *notifier)
{
	struct visp_media_dev *visp_mdev =
		container_of(notifier, struct visp_media_dev, notifier);

	return v4l2_device_register_subdev_nodes(&visp_mdev->v4l2_dev);
}

static const struct v4l2_async_notifier_operations visp_video_async_nf_ops = {
	.bound = visp_video_notifier_bound,
	.unbind = visp_video_notifier_unbound,
	.complete = visp_video_notifier_complete,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
static struct v4l2_async_connection *visp_video_async_nf_add_fwnode_remote(
	struct v4l2_async_notifier *notif, struct fwnode_handle *endpoint,
	unsigned int asd_struct_size)
{
	struct v4l2_async_connection *asd;
	struct fwnode_handle *remote;
	unsigned int i;
	remote = fwnode_graph_get_remote_port_parent(endpoint);
	if (!remote) return ERR_PTR(-ENOTCONN);
	// Add the link check
	struct list_head *lists[] = {&notif->done_list, &notif->waiting_list};
	for (i = 0; i < ARRAY_SIZE(lists); i++)
	{
		list_for_each_entry(asd, lists[i], asc_entry)
		{
			if (asd->match.fwnode == remote)
			{
				fwnode_handle_put(remote);
				printk(KERN_ERR "%s %d Found Matching entity\n", __func__, __LINE__);
				return asd;
			}
		}
	}
	asd = __v4l2_async_nf_add_fwnode(notif, remote, asd_struct_size);
	fwnode_handle_put(remote);

	return asd;
}
#endif

static int visp_video_async_register_subdev(struct visp_media_dev *visp_mdev)
{
	int ret = 0;
	struct fwnode_handle *ep;
	//	struct v4l2_async_subdev *asd;
	struct v4l2_async_connection *asd;
	unsigned int port_id = 0;

	visp_mdev->notifier.ops = &visp_video_async_nf_ops;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	v4l2_async_nf_init(&visp_mdev->notifier, &visp_mdev->v4l2_dev);
#else
	v4l2_async_notifier_init(&visp_mdev->notifier);
#endif

	while (1)
	{
		ep = fwnode_graph_get_endpoint_by_id(
			dev_fwnode(visp_mdev->dev), port_id, 0, FWNODE_GRAPH_ENDPOINT_NEXT);
		if (!ep)
		{
			break;
		}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
		asd = visp_video_async_nf_add_fwnode_remote(
			&visp_mdev->notifier, ep, sizeof(struct v4l2_async_connection));
#else
		asd = v4l2_async_notifier_add_fwnode_remote_subdev(
			&visp_mdev->notifier, ep, struct v4l2_async_subdev);
#endif
		fwnode_handle_put(ep);

		if (IS_ERR(asd))
		{
			ret = PTR_ERR(asd);
			if (ret != -EEXIST)
			{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
				v4l2_async_nf_cleanup(&visp_mdev->notifier);
#else
				v4l2_async_notifier_cleanup(&visp_mdev->notifier);
#endif

				return ret;
			}
		}
		port_id++;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	ret = v4l2_async_nf_register(&visp_mdev->notifier);
#else
	ret = v4l2_async_notifier_register(&visp_mdev->v4l2_dev,
									   &visp_mdev->notifier);
#endif
	if (ret)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
		v4l2_async_nf_cleanup(&visp_mdev->notifier);
#else
		v4l2_async_notifier_cleanup(&visp_mdev->notifier);
#endif
		dev_err(visp_mdev->dev, "v4l2 async notifier register error %d \n",
				ret);
		return ret;
	}

	return 0;
}

static int visp_video_async_unregister_subdev(struct visp_media_dev *visp_mdev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	v4l2_async_nf_unregister(&visp_mdev->notifier);
	v4l2_async_nf_cleanup(&visp_mdev->notifier);
#else
	v4l2_async_notifier_unregister(&visp_mdev->notifier);
	v4l2_async_notifier_cleanup(&visp_mdev->notifier);
#endif

	return 0;
}

static const struct media_device_ops visp_video_media_ops = {
	.link_notify = v4l2_pipeline_link_notify,
};

static int visp_video_parse_params(struct visp_media_dev *visp_mdev,
								   struct platform_device *pdev)
{
	unsigned int port_id = 0;
	int ret;
	char node_name[50];
	struct device_node *mem_np;

	struct fwnode_handle *ep;
	fwnode_property_read_u32(of_fwnode_handle(pdev->dev.of_node), "id",
							 &visp_mdev->id);

	while (1)
	{
		ep = fwnode_graph_get_endpoint_by_id(
			dev_fwnode(visp_mdev->dev), port_id, 0, FWNODE_GRAPH_ENDPOINT_NEXT);
		if (!ep)
		{
			break;
		}
		port_id++;
	}

	snprintf(node_name, sizeof(node_name),
             "isp%d_reserve_memory", visp_mdev->id);

    mem_np = of_find_node_by_name(NULL, node_name);

	if(mem_np){
		ret = of_reserved_mem_device_init(&pdev->dev);
		if (ret)
		dev_dbg(&pdev->dev, "of_reserved_mem_device_init: %d\n", ret);

		ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
		if (ret) {
			dev_err(&pdev->dev, "dma_set_mask_and_coherent: %d\n", ret);
			return -ENOMEM;
			//goto error;
		}
	}


	visp_mdev->ports = port_id;
	return 0;
}

static int visp_video_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct media_device *mdev;
	struct device *dev = &pdev->dev;
	struct visp_media_dev *visp_mdev;

	visp_mdev = devm_kzalloc(dev, sizeof(struct visp_media_dev), GFP_KERNEL);
	if (!visp_mdev) return -ENOMEM;

	visp_mdev->dev = dev;
	platform_set_drvdata(pdev, visp_mdev);

	ret = visp_video_parse_params(visp_mdev, pdev);
	if (ret)
	{
		dev_err(dev, "parse device params error\n");
		return ret;
	}

	mdev = &visp_mdev->mdev;
	mdev->dev = dev;
	mdev->ops = &visp_video_media_ops;
	strscpy(mdev->model, "verisilicon_media", sizeof(mdev->model));
	media_device_init(mdev);

	visp_mdev->v4l2_dev.mdev = mdev;
	ret = v4l2_device_register(dev, &visp_mdev->v4l2_dev);
	if (ret)
	{
		dev_err(dev, "register v4l2 device error\n");
		return ret;
	}

	ret = visp_video_register_ports(visp_mdev);
	if (ret)
	{
		dev_err(dev, "register video device nodes error\n");
		goto err_unregister_v4l2_device;
	}

	ret = visp_video_async_register_subdev(visp_mdev);
	if (ret)
	{
		dev_err(dev, "register v4l2 async notifier error\n");
		goto err_unregister_video_ports;
	}

	ret = media_device_register(mdev);
	if (ret)
	{
		dev_err(dev, "register media device error\n");
		goto err_unregister_subdev;
	}
	dev_info(&pdev->dev, "visp video driver probe success\n");
	return 0;

err_unregister_subdev:
	visp_video_async_unregister_subdev(visp_mdev);
err_unregister_video_ports:
	visp_video_unregister_ports(visp_mdev);
err_unregister_v4l2_device:
	v4l2_device_unregister(&visp_mdev->v4l2_dev);

	return ret;
}

static void visp_video_remove(struct platform_device *pdev)
{
	struct visp_media_dev *visp_mdev;

	visp_mdev = platform_get_drvdata(pdev);

	media_device_unregister(&visp_mdev->mdev);
	visp_video_async_unregister_subdev(visp_mdev);
	visp_video_unregister_ports(visp_mdev);
	v4l2_device_unregister(&visp_mdev->v4l2_dev);
	dev_info(&pdev->dev, "visp video driver remove\n");

	return ;
}

static const struct of_device_id visp_video_of_match[] = {
	{
		.compatible = "verisilicon,video",
	},
	{/* sentinel */},
};

MODULE_DEVICE_TABLE(of, visp_video_of_match);

static struct platform_driver visp_video_driver = {
	.probe = visp_video_probe,
	.remove = visp_video_remove,
	.driver =
		{
			.name = VISP_VIDEO_NAME,
			.owner = THIS_MODULE,
			.of_match_table = visp_video_of_match,
		},
};

static int __init visp_video_init_module(void)
{
	int ret;
	ret = platform_driver_register(&visp_video_driver);
	if (ret)
	{
		printk(KERN_ERR "Failed to register video driver\n");
		return ret;
	}

	printk(KERN_INFO "visp-video driver insmod success\n");

	return ret;
}

static void __exit visp_video_exit_module(void)
{
	platform_driver_unregister(&visp_video_driver);

	printk(KERN_INFO "visp-video driver rmmod success\n");
	return;
}

module_init(visp_video_init_module);
module_exit(visp_video_exit_module);

MODULE_DESCRIPTION("Verisilicon isp driver");
MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL");
