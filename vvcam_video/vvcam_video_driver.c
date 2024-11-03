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
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_graph.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/videobuf2-dma-contig.h>
#include <media/v4l2-fwnode.h>

#include "vvcam_video_driver.h"
#include "vvcam_video_register.h"

static int vvcam_video_register_ports(struct vvcam_media_dev *vvcam_mdev)
{
    int i = 0;
    int ret = 0;
    for (i = 0; i < vvcam_mdev->ports; i++) {
        if (vvcam_mdev->video_params[i].m2m == false) {
            ret = vvcam_video_register(vvcam_mdev, i);
            if (ret) {
                goto err_register_video;
            }
        }
    }
    return 0;

err_register_video:
    for (i = 0; i < vvcam_mdev->ports; i++) {
        if (vvcam_mdev->video_params[i].m2m == false)
            vvcam_video_unregister(vvcam_mdev, i);
    }

    return ret;
}

static int vvcam_video_unregister_ports(struct vvcam_media_dev *vvcam_mdev)
{
    int i;

    for (i = 0; i < vvcam_mdev->ports; i++) {
        if (vvcam_mdev->video_params[i].m2m == false)
            vvcam_video_unregister(vvcam_mdev, i);
    }

    return 0;
}

static int vvcam_video_notifier_bound(struct v4l2_async_notifier *notifier,
				struct v4l2_subdev *sd, struct v4l2_async_connection *asc)
{
    struct vvcam_media_dev *vvcam_mdev = container_of(notifier,
							   struct vvcam_media_dev, notifier);
	struct device *dev = vvcam_mdev->dev;
    struct fwnode_handle *ep = NULL;
	struct v4l2_fwnode_link link;
	struct media_entity *source, *sink;
	unsigned int source_pad, sink_pad;
    struct vvcam_video_dev *vvcam_vdev;
	struct video_device *vdev;
	int ret;

	while (1) {
		ep = fwnode_graph_get_next_endpoint(sd->fwnode, ep);
		if (!ep)
			break;
		ret = v4l2_fwnode_parse_link(ep, &link);
		if (ret < 0) {
			dev_err(dev, "failed to parse link for %pOF: %d\n", to_of_node(ep), ret);
			continue;
		}

		if (sd->entity.pads[link.local_port].flags == MEDIA_PAD_FL_SINK)
			continue;

        vvcam_vdev = vvcam_mdev->video_devs[link.remote_port];
		vdev = vvcam_vdev->video;
		source = &sd->entity;
		source_pad = link.local_port;
		sink = &vdev->entity;
		sink_pad = 0;
		v4l2_fwnode_put_link(&link);
		ret = media_create_pad_link(source, source_pad, sink, sink_pad, MEDIA_LNK_FL_ENABLED);
		if (ret) {
			dev_err(dev, "failed to create %s:%u -> %s:%u link\n", source->name, source_pad, sink->name, sink_pad);
			break;
		}

	}
	fwnode_handle_put(ep);

    return 0;
}


static void vvcam_video_notifier_unbound(struct v4l2_async_notifier *notifier,
					struct v4l2_subdev *sd, struct v4l2_async_connection *asc)
{
	return;
}

static int vvcam_video_notifier_complete(struct v4l2_async_notifier *notifier)
{
    struct vvcam_media_dev *vvcam_mdev = container_of(notifier,
							   struct vvcam_media_dev, notifier);

    return v4l2_device_register_subdev_nodes(&vvcam_mdev->v4l2_dev);
}

static const struct v4l2_async_notifier_operations vvcam_video_async_nf_ops = {
	.bound    = vvcam_video_notifier_bound,
	.unbind   = vvcam_video_notifier_unbound,
	.complete = vvcam_video_notifier_complete,
};
#if 0
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
static struct v4l2_async_subdev *
vvcam_video_async_nf_add_fwnode_remote(struct v4l2_async_notifier *notif,
				  struct fwnode_handle *endpoint,
				  unsigned int asd_struct_size)
{
	struct v4l2_async_subdev *asd;
	struct fwnode_handle *remote;

	remote = fwnode_graph_get_remote_port_parent(endpoint);
	if (!remote)
		return ERR_PTR(-ENOTCONN);

	asd = __v4l2_async_nf_add_fwnode(notif, remote, asd_struct_size);
	fwnode_handle_put(remote);

	return asd;
}
#endif
#endif
static int vvcam_video_async_register_subdev(struct vvcam_media_dev *vvcam_mdev)
{
#if 0
    int ret = 0;
	struct fwnode_handle *ep;
	struct v4l2_async_subdev *asd;
    unsigned int port_id = 0;
	vvcam_mdev->notifier.ops = &vvcam_video_async_nf_ops;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	v4l2_async_nf_init(&vvcam_mdev->notifier);
#else
    v4l2_async_notifier_init(&vvcam_mdev->notifier);
#endif

    while (1) {
        ep = fwnode_graph_get_endpoint_by_id(dev_fwnode(vvcam_mdev->dev),
                port_id, 0, FWNODE_GRAPH_ENDPOINT_NEXT);
	pr_err("RKC-VIDDRV %s %d\n",__func__,__LINE__);
        if (!ep)
            break;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
        asd = vvcam_video_async_nf_add_fwnode_remote(&vvcam_mdev->notifier,
                ep, sizeof(struct v4l2_async_subdev));
#else
        asd = v4l2_async_notifier_add_fwnode_remote_subdev(&vvcam_mdev->notifier,
                ep, struct v4l2_async_subdev);
#endif
        fwnode_handle_put(ep);

        if (IS_ERR(asd)) {
			ret = PTR_ERR(asd);
			if (ret != -EEXIST) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
                v4l2_async_nf_cleanup(&vvcam_mdev->notifier);
#else
				v4l2_async_notifier_cleanup(&vvcam_mdev->notifier);
#endif
				return ret;
			}
		}
        port_id++;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
    ret = v4l2_async_nf_register(&vvcam_mdev->v4l2_dev, &vvcam_mdev->notifier);
#else
    ret = v4l2_async_notifier_register(&vvcam_mdev->v4l2_dev, &vvcam_mdev->notifier);
#endif
	if (ret) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
        v4l2_async_nf_cleanup(&vvcam_mdev->notifier);
#else
		v4l2_async_notifier_cleanup(&vvcam_mdev->notifier);
#endif
		dev_err(vvcam_mdev->dev, "v4l2 async notifier register error\n");
        return ret;
    }
#endif
    return 0;
}

static int vvcam_video_async_unregister_subdev(struct vvcam_media_dev *vvcam_mdev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
    v4l2_async_nf_unregister(&vvcam_mdev->notifier);
    v4l2_async_nf_cleanup(&vvcam_mdev->notifier);
#else
    v4l2_async_notifier_unregister(&vvcam_mdev->notifier);
    v4l2_async_notifier_cleanup(&vvcam_mdev->notifier);
#endif

    return 0;
}

static const struct media_device_ops vvcam_video_media_ops = {
    .link_notify = v4l2_pipeline_link_notify,
};

static int vvcam_video_parse_params(struct vvcam_media_dev *vvcam_mdev,
                                struct platform_device *pdev)
{
    unsigned int port_id = 0;
    int num_mems, i;
    struct fwnode_handle *ep;
    fwnode_property_read_u32(of_fwnode_handle(pdev->dev.of_node),
			"id", &vvcam_mdev->id);

     while (1) {
        ep = fwnode_graph_get_endpoint_by_id(dev_fwnode(vvcam_mdev->dev),
                port_id, 0, FWNODE_GRAPH_ENDPOINT_NEXT);
        if (!ep)
            break;
        port_id++;
     }

    num_mems = of_count_phandle_with_args(pdev->dev.of_node, "memory-region", NULL);

    if (num_mems < 0) {
        return -ENOMEM;
    }

    for (i = 0; i < num_mems; i++) {
        struct device_node *node;
        struct reserved_mem *rmem;

        node = of_parse_phandle(pdev->dev.of_node, "memory-region", i);
        if (!node) {
                return -EINVAL;
        }

        rmem = of_reserved_mem_lookup(node);
        if (!rmem)
                return -EINVAL;
        pr_info("reserve name:%s base:%llx size:%llx\n", rmem->name, rmem->base, rmem->size);
        vvcam_mdev->reserve_mem.pa = rmem->base;
        vvcam_mdev->reserve_mem.size = rmem->size;
    }



    vvcam_mdev->ports = port_id;

    return 0;
}

static int vvcam_video_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct media_device *mdev;
	struct device *dev = &pdev->dev;
    struct vvcam_media_dev *vvcam_mdev;

    vvcam_mdev = devm_kzalloc(dev, sizeof(struct vvcam_media_dev), GFP_KERNEL);
	if (!vvcam_mdev)
		return -ENOMEM;

    vvcam_mdev->dev = dev;
    platform_set_drvdata(pdev, vvcam_mdev);

    ret = vvcam_video_parse_params(vvcam_mdev, pdev);
    if (ret) {
        dev_err(dev, "parse device params error\n");
		return ret;
    }

    mdev = &vvcam_mdev->mdev;
    mdev->dev = dev;
    mdev->ops = &vvcam_video_media_ops;
    strscpy(mdev->model, "verisilicon_media", sizeof(mdev->model));
	media_device_init(mdev);

    vvcam_mdev->v4l2_dev.mdev = mdev;
    ret = v4l2_device_register(dev, &vvcam_mdev->v4l2_dev);
	if (ret) {
		dev_err(dev, "register v4l2 device error\n");
		return ret;
	}

    ret = vvcam_video_register_ports(vvcam_mdev);
    if (ret) {
		dev_err(dev, "register video device nodes error\n");
		goto err_unregister_v4l2_device;
	}

    ret = vvcam_video_async_register_subdev(vvcam_mdev);
    if (ret) {
        dev_err(dev, "register v4l2 async notifier error\n");
		goto err_unregister_video_ports;
    }

    ret = media_device_register(mdev);
    if (ret) {
        dev_err(dev, "register media device error\n");
        goto err_unregister_subdev;
    }
    dev_info(&pdev->dev, "vvcam video driver probe success\n");
    return 0;

err_unregister_subdev:
    vvcam_video_async_unregister_subdev(vvcam_mdev);
err_unregister_video_ports:
    vvcam_video_unregister_ports(vvcam_mdev);
err_unregister_v4l2_device:
    v4l2_device_unregister(&vvcam_mdev->v4l2_dev);

    return ret;
}

static int vvcam_video_remove(struct platform_device *pdev)
{
    struct vvcam_media_dev *vvcam_mdev;

    vvcam_mdev = platform_get_drvdata(pdev);

    media_device_unregister(&vvcam_mdev->mdev);
    vvcam_video_async_unregister_subdev(vvcam_mdev);
    vvcam_video_unregister_ports(vvcam_mdev);
    v4l2_device_unregister(&vvcam_mdev->v4l2_dev);
    dev_info(&pdev->dev, "vvcam video driver remove\n");

    return 0;
}

static const struct of_device_id vvcam_video_of_match[] = {
	{.compatible = "verisilicon,video",},
	{ /* sentinel */ },
};

MODULE_DEVICE_TABLE(of, vvcam_video_of_match);

static struct platform_driver vvcam_video_driver = {
	.probe  = vvcam_video_probe,
	.remove = vvcam_video_remove,
	.driver = {
        .name  = VVCAM_VIDEO_NAME,
        .owner = THIS_MODULE,
        .of_match_table = vvcam_video_of_match,
    },
};

static int __init vvcam_video_init_module(void)
{
    int ret;
    ret = platform_driver_register(&vvcam_video_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register video driver\n");
        return ret;
    }

    printk(KERN_INFO "vvcam-video driver insmod success\n");

    return ret;
}

static void __exit vvcam_video_exit_module(void)
{
    platform_driver_unregister(&vvcam_video_driver);

    printk(KERN_INFO "vvcam-video driver rmmod success\n");
    return;
}

module_init(vvcam_video_init_module);
module_exit(vvcam_video_exit_module);

MODULE_DESCRIPTION("Verisilicon isp driver");
MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL");

