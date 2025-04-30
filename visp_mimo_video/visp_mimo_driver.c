/*
 * Copyright 2025 Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */


#include <linux/module.h>
#include <linux/of_graph.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/v4l2-mediabus.h>
#include <media/videobuf2-dma-contig.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-event.h>
#include <visp_mimo_driver.h>
#include <visp_video_event.h>
#include <visp_v4l2_std_exts.h>
#include <linux/miscdevice.h>

#define DEBUG_ENABLE
/* version 1.0 */
#ifndef DEBUG_ENABLE
#define visp_pr_info(fmt, ...) ;
#define isp_pr_debug(fmt, ...) ;
#define visp_pr_err(fmt, ...) ;
#else
#define visp_pr_info pr_err
#define visp_pr_debug pr_err
#define visp_pr_err pr_err
#endif
#define visp_v4l2_dbg(fmt, ...) ;


static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "debug level (0-3)");

struct isp_mimo_ctx;


struct visp_format visp_mp_fmts[] = {
	{
		.fourcc = V4L2_PIX_FMT_NV16,
		.code = MEDIA_BUS_FMT_YUYV8_2X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV12,
		.code = MEDIA_BUS_FMT_YUYV8_1_5X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_YUYV,
		.code = MEDIA_BUS_FMT_YUYV8_1X16,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR8,
		.code = MEDIA_BUS_FMT_SBGGR8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG8,
		.code = MEDIA_BUS_FMT_SGBRG8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG8,
		.code = MEDIA_BUS_FMT_SGRBG8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB8,
		.code = MEDIA_BUS_FMT_SRGGB8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR10,
		.code = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG10,
		.code = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG10,
		.code = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB10,
		.code = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR12,
		.code = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG12,
		.code = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG12,
		.code = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB12,
		.code = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_P010,
		.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_GREY,
		.code = MEDIA_BUS_FMT_Y8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_Y10,
		.code = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB24,
		.code = MEDIA_BUS_FMT_RGB888_1X24,
	},
};

struct visp_format visp_sp_fmts[] = {
	{
		.fourcc = V4L2_PIX_FMT_NV16,
		.code = MEDIA_BUS_FMT_YUYV8_2X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV12,
		.code = MEDIA_BUS_FMT_YUYV8_1_5X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_YUYV,
		.code = MEDIA_BUS_FMT_YUYV8_1X16,
	},
	{
		.fourcc = V4L2_PIX_FMT_P010,
		.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_GREY,
		.code = MEDIA_BUS_FMT_Y8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_Y10BPACK,
		.code = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_Y10DWA,
		.code = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_Y10,
		.code = MEDIA_BUS_FMT_Y10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_P00BPACK,
		.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_P00DWA,
		.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	// {
	//     .fourcc    = V4L2_PIX_FMT_P02BPACK,
	//     .code      = MEDIA_BUS_FMT_YUYV12_2X12,
	// },
	{
		.fourcc = V4L2_PIX_FMT_P20BPACK,
		.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_P20DWA,
		.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_P210,
		.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	// {
	//     .fourcc    = V4L2_PIX_FMT_P22BPACK,
	//     .code      = MEDIA_BUS_FMT_YUYV12_2X12,
	// },
	{
		.fourcc = V4L2_PIX_FMT_I210,
		.code = MEDIA_BUS_FMT_YUYV10_2X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_M48BPACK,
		.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_I48BPACK,
		.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_I48DWA,
		.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_I40DWA,
		.code = MEDIA_BUS_FMT_YUV8_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB24,
		.code = MEDIA_BUS_FMT_RGB888_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB24DWA,
		.code = MEDIA_BUS_FMT_RGB888_1X24,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB24P,
		.code = MEDIA_BUS_FMT_RGB888_3X8,
	},
};

// main path
struct visp_format visp_raw_fmts[] = {
	{
		.fourcc = V4L2_PIX_FMT_SBGGR8,
		.code = MEDIA_BUS_FMT_SBGGR8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG8,
		.code = MEDIA_BUS_FMT_SGBRG8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG8,
		.code = MEDIA_BUS_FMT_SGRBG8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB8,
		.code = MEDIA_BUS_FMT_SRGGB8_1X8,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR10,
		.code = MEDIA_BUS_FMT_SBGGR10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG10,
		.code = MEDIA_BUS_FMT_SGBRG10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG10,
		.code = MEDIA_BUS_FMT_SGRBG10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB10,
		.code = MEDIA_BUS_FMT_SRGGB10_1X10,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR12,
		.code = MEDIA_BUS_FMT_SBGGR12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG12,
		.code = MEDIA_BUS_FMT_SGBRG12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG12,
		.code = MEDIA_BUS_FMT_SGRBG12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB12,
		.code = MEDIA_BUS_FMT_SRGGB12_1X12,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR14,
		.code = MEDIA_BUS_FMT_SBGGR14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG14,
		.code = MEDIA_BUS_FMT_SGBRG14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG14,
		.code = MEDIA_BUS_FMT_SGRBG14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB14,
		.code = MEDIA_BUS_FMT_SRGGB14_1X14,
	},
	{
		.fourcc = V4L2_PIX_FMT_SBGGR16,
		.code = MEDIA_BUS_FMT_SBGGR16_1X16,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGBRG16,
		.code = MEDIA_BUS_FMT_SGBRG16_1X16,
	},
	{
		.fourcc = V4L2_PIX_FMT_SGRBG16,
		.code = MEDIA_BUS_FMT_SGRBG16_1X16,
	},
	{
		.fourcc = V4L2_PIX_FMT_SRGGB16,
		.code = MEDIA_BUS_FMT_SRGGB16_1X16,
	},

};

void inspect_source_buffers(struct v4l2_m2m_ctx *m2m_ctx, dma_addr_t *s, dma_addr_t *d);
void inspect_source_buffers(struct v4l2_m2m_ctx *m2m_ctx, dma_addr_t *s, dma_addr_t *d)
{
    struct vb2_queue *vb2_q;
    struct vb2_buffer *vb;
    dma_addr_t dma_addr;
    unsigned int i;

    vb2_q = v4l2_m2m_get_src_vq(m2m_ctx);
    for (i = 0; i < vb2_q->max_num_buffers; i++) {
        vb = vb2_q->bufs[i];
        if(!vb) {
            break;
        }
        dma_addr = vb2_dma_contig_plane_dma_addr(vb, 0);
		s[i] = dma_addr;
    }

    vb2_q = v4l2_m2m_get_dst_vq(m2m_ctx);
    for (i = 0; i < vb2_q->max_num_buffers; i++) {
        vb = vb2_q->bufs[i];
        if(!vb) {
            break;
        }
        dma_addr = vb2_dma_contig_plane_dma_addr(vb, 0);
		d[i] = dma_addr;
    }
}
static int on;
  void isp_mimo_device_run(void *priv)
{
	struct isp_mimo_ctx *ctx = priv;
	struct isp_mimo *device = ctx->device;
	struct vb2_v4l2_buffer *src_vb, *dst_vb;

	dma_addr_t input_addr[2], output_addr[4];
	struct vb2_v4l2_buffer *src_buf, *dst_buf;
	int s_cnt, d_cnt;
	struct isp_mimo_ctx *curr_ctx=ctx;

	s_cnt = v4l2_m2m_num_src_bufs_ready(ctx->fh.m2m_ctx);
	d_cnt = v4l2_m2m_num_dst_bufs_ready(ctx->fh.m2m_ctx);

	src_buf = v4l2_m2m_next_src_buf(ctx->fh.m2m_ctx);
	dst_buf = v4l2_m2m_next_dst_buf(ctx->fh.m2m_ctx);

    inspect_source_buffers(ctx->fh.m2m_ctx, input_addr, output_addr);

	device->isp_dev->ip_a[0]=input_addr[0];
	device->isp_dev->ip_a[1]=input_addr[1];
	device->isp_dev->op_a[0]=output_addr[0];
	device->isp_dev->op_a[1]=output_addr[1];
	device->isp_dev->op_a[2]=output_addr[2];
	device->isp_dev->op_a[3]=output_addr[3];
	if (!on)
	{
        int ret;
        ret = visp_l_calib_event(device, 0, VIDEO_EVENT_LOAD_CALIB);
        if (ret != 0) {
                pr_err("[EVENT_FAIL] %s %d\n", __func__,__LINE__);
        }
	    MediaIspDeviceStreamOn(device->isp_dev, 0, 0);

        ret = visp_l_calib_event(device, 0, VIDEO_EVENT_LOAD_JSON);
        if (ret != 0) {
                pr_err("[EVENT_FAIL] %s %d\n", __func__,__LINE__);
        }
	    MediaIspDeviceStreamOnOut(device->isp_dev, 0, 0);
		on = 1;
	}
	MediaIspDeviceDeque(device->isp_dev, 0);

	device->isp_dev->apu_wait_for_isp_frame_done = 1;
	wait_event_interruptible(device->isp_dev->wq_frame_done_finished, !device->isp_dev->apu_wait_for_isp_frame_done);

	/* return the src and dst buffers back to V4L2 M2M layer to return to application */
	src_vb = v4l2_m2m_src_buf_remove(curr_ctx->fh.m2m_ctx);
	dst_vb = v4l2_m2m_dst_buf_remove_by_idx(curr_ctx->fh.m2m_ctx,device->isp_dev->isp_dq_out_index);
	if (src_vb && dst_vb) 
	{
		dst_vb->vb2_buf.timestamp = src_vb->vb2_buf.timestamp;
		dst_vb->timecode = src_vb->timecode;
		dst_vb->flags &= ~V4L2_BUF_FLAG_TSTAMP_SRC_MASK;
		dst_vb->flags |= src_vb->flags & V4L2_BUF_FLAG_TSTAMP_SRC_MASK;
        	v4l2_m2m_buf_done(src_vb, VB2_BUF_STATE_DONE);
	        v4l2_m2m_buf_done(dst_vb, VB2_BUF_STATE_DONE);
	}
	src_vb->sequence = dst_vb->sequence = curr_ctx->sequence++;
	v4l2_m2m_job_finish(device->m2m_dev, curr_ctx->fh.m2m_ctx);

}

static const struct v4l2_format_info *visp_video_vfmt_info(u32 format)
{
	/* format info for user define or supported by later versions */
	static const struct v4l2_format_info formats[] = {
		{
			.format = V4L2_PIX_FMT_GREY,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {1, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {1, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_Y10BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {5, 0, 0, 0},
			.bpp_div = {4, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_Y10DWA,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 0, 0, 0},
			.bpp_div = {3, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_Y10,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		// 420
		{
			.format = V4L2_PIX_FMT_P00BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 2,
			.hdiv = 2,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {5, 10, 0, 0},
			.bpp_div = {4, 4, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_P00DWA,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 2,
			.hdiv = 2,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 8, 0, 0},
			.bpp_div = {3, 3, 1, 1},
#else
			.bpp = {2, 2, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_P010,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 2,
			.hdiv = 1,
			.vdiv = 2,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 2, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 2, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_P02BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 2,
			.hdiv = 2,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {3, 6, 0, 0},
			.bpp_div = {2, 2, 1, 1},
#else
			.bpp = {2, 2, 0, 0},
#endif
		},
		// 422
		{
			.format = V4L2_PIX_FMT_P20BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 2,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {5, 10, 0, 0},
			.bpp_div = {4, 4, 1, 1},
#else
			.bpp = {2, 2, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_P20DWA,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 2,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 8, 0, 0},
			.bpp_div = {3, 3, 1, 1},
#else
			.bpp = {2, 2, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_P210,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 2,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 4, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 2, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_P22BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 2,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {3, 6, 0, 0},
			.bpp_div = {2, 2, 1, 1},
#else
			.bpp = {2, 2, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_I20BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {15, 0, 0, 0},
			.bpp_div = {4, 1, 1, 1},
#else
			.bpp = {4, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_I210,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {4, 0, 0, 0},
#endif
		},
		// 444
		{
			.format = V4L2_PIX_FMT_M48BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 3,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {1, 1, 1, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {1, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_I48BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {3, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {3, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_I48DWA,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {4, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_I40DWA,
			.pixel_enc = V4L2_PIXEL_ENC_YUV,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {4, 0, 0, 0},
#endif
		},
		// RGB
		{
			.format = V4L2_PIX_FMT_RGB24,
			.pixel_enc = V4L2_PIXEL_ENC_RGB,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {3, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {3, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_RGB24DWA,
			.pixel_enc = V4L2_PIXEL_ENC_RGB,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {4, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_RGB24P,
			.pixel_enc = V4L2_PIXEL_ENC_RGB,
			.mem_planes = 1,
			.comp_planes = 3,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {1, 1, 1, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {1, 1, 1, 0},
#endif
		},
		// RAW
		{
			.format = V4L2_PIX_FMT_SBGGR10BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {5, 0, 0, 0},
			.bpp_div = {4, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGBRG10BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {5, 0, 0, 0},
			.bpp_div = {4, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGRBG10BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {5, 0, 0, 0},
			.bpp_div = {4, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SRGGB10BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {5, 0, 0, 0},
			.bpp_div = {4, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SBGGR10DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 0, 0, 0},
			.bpp_div = {3, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGBRG10DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 0, 0, 0},
			.bpp_div = {3, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGRBG10DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 0, 0, 0},
			.bpp_div = {3, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SRGGB10DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {4, 0, 0, 0},
			.bpp_div = {3, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SBGGR12BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {3, 0, 0, 0},
			.bpp_div = {2, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGBRG12BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {3, 0, 0, 0},
			.bpp_div = {2, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGRBG12BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {3, 0, 0, 0},
			.bpp_div = {2, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SRGGB12BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {3, 0, 0, 0},
			.bpp_div = {2, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SBGGR12DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {8, 0, 0, 0},
			.bpp_div = {5, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGBRG12DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {8, 0, 0, 0},
			.bpp_div = {5, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGRBG12DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {8, 0, 0, 0},
			.bpp_div = {5, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SRGGB12DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {8, 0, 0, 0},
			.bpp_div = {5, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SBGGR14BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {7, 0, 0, 0},
			.bpp_div = {4, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGBRG14BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {7, 0, 0, 0},
			.bpp_div = {4, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGRBG14BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {7, 0, 0, 0},
			.bpp_div = {4, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SRGGB14BPACK,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {7, 0, 0, 0},
			.bpp_div = {4, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SBGGR14DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {16, 0, 0, 0},
			.bpp_div = {9, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGBRG14DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {16, 0, 0, 0},
			.bpp_div = {9, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGRBG14DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {16, 0, 0, 0},
			.bpp_div = {9, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SRGGB14DWA,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {16, 0, 0, 0},
			.bpp_div = {9, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SBGGR14,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGBRG14,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGRBG14,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SRGGB14,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SBGGR16,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGBRG16,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGRBG16,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SRGGB16,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {2, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SBGGR24,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {3, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGBRG24,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {3, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SGRBG24,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {3, 0, 0, 0},
#endif
		},
		{
			.format = V4L2_PIX_FMT_SRGGB24,
			.pixel_enc = V4L2_PIXEL_ENC_BAYER,
			.mem_planes = 1,
			.comp_planes = 1,
			.hdiv = 1,
			.vdiv = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
			.bpp = {2, 0, 0, 0},
			.bpp_div = {1, 1, 1, 1},
#else
			.bpp = {3, 0, 0, 0},
#endif
		},

	};

	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(formats); i++)
	{
		if (formats[i].format == format) return &formats[i];
	}

	return NULL;
}


  struct v4l2_format* isp_mimo_get_format(struct isp_mimo_ctx *ctx, enum v4l2_buf_type type);
  struct v4l2_format* isp_mimo_get_format(struct isp_mimo_ctx *ctx, enum v4l2_buf_type type)
{
	struct isp_mimo *device = ctx->device;
	struct v4l2_device *v4l2_dev = &device->v4l2_dev;

	switch (type) {
		case V4L2_BUF_TYPE_VIDEO_OUTPUT:
			return &ctx->fmt[FMT_OUTPUT];

		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			return &ctx->fmt[FMT_CAPTURE];

		default:
			v4l2_err(v4l2_dev, "Unknown type \n");
			return ERR_PTR(-EINVAL);
	}
}

  int isp_mimo_v4l2_m2m_ioctl_querycap(struct file *file, void *priv,
			   		struct v4l2_capability *cap)
{
	strscpy(cap->driver, MEM2MEM_NAME, sizeof(cap->driver));
	strscpy(cap->card, MEM2MEM_NAME, sizeof(cap->card));
	snprintf(cap->bus_info, sizeof(cap->bus_info),
			"platform:%s", MEM2MEM_NAME);
	return 0;
}


static int visp_m2m_cal_imagesize(struct v4l2_format *f)
{
	const struct v4l2_format_info *info;
	uint32_t bytesperline;
	uint32_t sizeimage = 0;
	uint32_t width;
	uint32_t height;
	int i;
	int private_fmt = 0;

	width = f->fmt.pix.width;
	height = f->fmt.pix.height;

	info = v4l2_format_info(f->fmt.pix.pixelformat);
	if (info != NULL)
	{
		bytesperline = info->bpp[0] * width;
	}
	else
	{
		private_fmt = 1;
		info = visp_video_vfmt_info(f->fmt.pix.pixelformat);
		if (info == NULL)
		{
			visp_pr_err("%s %d width %d height %d format %d\n", __func__, __LINE__, width, height, f->fmt.pix.pixelformat);
			return -EINVAL;
		}

		// calculate size information here for private format
		switch (info->format)
		{
			/* YUV format */
			// 8bit unalign
			case V4L2_PIX_FMT_GREY:
			case V4L2_PIX_FMT_M48BPACK:
			case V4L2_PIX_FMT_I48BPACK:
				bytesperline = width;
				if (info->format == V4L2_PIX_FMT_I48BPACK)
				{
					bytesperline *= 3;
				}
				break;

			// 8bit dwa
			case V4L2_PIX_FMT_I48DWA:
			case V4L2_PIX_FMT_I40DWA:
				bytesperline = ((width * 4) / 3) * 3;
				break;

			// 10bit unalign
			case V4L2_PIX_FMT_Y10BPACK:
			case V4L2_PIX_FMT_P00BPACK:
			case V4L2_PIX_FMT_P20BPACK:
			case V4L2_PIX_FMT_I20BPACK:
				bytesperline = DIV_ROUND_UP(width * 10, 128) * 16;
				if (info->format == V4L2_PIX_FMT_I20BPACK)
				{
					bytesperline *= 4;
				}
				break;

			// 10 bit double align
			case V4L2_PIX_FMT_Y10DWA:
			case V4L2_PIX_FMT_P00DWA:
			case V4L2_PIX_FMT_P20DWA:
				bytesperline = width * 4 / 3;
				break;

			// 10 bit mode 1, per pixel 16bits
			case V4L2_PIX_FMT_Y10:
			case V4L2_PIX_FMT_P010:
			case V4L2_PIX_FMT_P210:
			case V4L2_PIX_FMT_I210:
				bytesperline = DIV_ROUND_UP(width, 8) * 16;
				if (info->format == V4L2_PIX_FMT_I210)
				{
					bytesperline *= 2;
				}
				break;

			// 12bit unalign
			case V4L2_PIX_FMT_P02BPACK:
			case V4L2_PIX_FMT_P22BPACK:
				bytesperline = DIV_ROUND_UP(width * 12, 128) * 16;
				break;

			// RGB
			case V4L2_PIX_FMT_RGB24:
				bytesperline = width * 3;
				break;

			case V4L2_PIX_FMT_RGB24P:
				bytesperline = width;
				break;

			case V4L2_PIX_FMT_RGB24DWA:
				bytesperline = (width * 4 / 3) * 3;
				break;

			// RAW
			// 10 bit-unalign
			case V4L2_PIX_FMT_SBGGR10BPACK:
			case V4L2_PIX_FMT_SGBRG10BPACK:
			case V4L2_PIX_FMT_SGRBG10BPACK:
			case V4L2_PIX_FMT_SRGGB10BPACK:
				bytesperline = DIV_ROUND_UP(width * 10, 128) * 16;
				break;

			case V4L2_PIX_FMT_SBGGR10DWA:
			case V4L2_PIX_FMT_SGBRG10DWA:
			case V4L2_PIX_FMT_SGRBG10DWA:
			case V4L2_PIX_FMT_SRGGB10DWA:
				bytesperline = DIV_ROUND_UP(width, 12) * 16;
				break;

			// 12bit
			case V4L2_PIX_FMT_SBGGR12BPACK:
			case V4L2_PIX_FMT_SGBRG12BPACK:
			case V4L2_PIX_FMT_SGRBG12BPACK:
			case V4L2_PIX_FMT_SRGGB12BPACK:
				bytesperline = DIV_ROUND_UP(width * 12, 128) * 16;
				break;

			case V4L2_PIX_FMT_SBGGR12DWA:
			case V4L2_PIX_FMT_SGBRG12DWA:
			case V4L2_PIX_FMT_SGRBG12DWA:
			case V4L2_PIX_FMT_SRGGB12DWA:
				bytesperline = DIV_ROUND_UP(width, 10) * 16;
				break;

			// 14 bit
			case V4L2_PIX_FMT_SBGGR14BPACK:
			case V4L2_PIX_FMT_SGBRG14BPACK:
			case V4L2_PIX_FMT_SGRBG14BPACK:
			case V4L2_PIX_FMT_SRGGB14BPACK:
				bytesperline = DIV_ROUND_UP(width * 14, 128) * 16;
				break;

			case V4L2_PIX_FMT_SBGGR14DWA:
			case V4L2_PIX_FMT_SGBRG14DWA:
			case V4L2_PIX_FMT_SGRBG14DWA:
			case V4L2_PIX_FMT_SRGGB14DWA:
				bytesperline = DIV_ROUND_UP(width, 9) * 16;
				break;

			case V4L2_PIX_FMT_SBGGR14:
			case V4L2_PIX_FMT_SGBRG14:
			case V4L2_PIX_FMT_SGRBG14:
			case V4L2_PIX_FMT_SRGGB14:
				bytesperline = DIV_ROUND_UP(width, 8) * 16;
				break;

			case V4L2_PIX_FMT_SBGGR16:
			case V4L2_PIX_FMT_SGBRG16:
			case V4L2_PIX_FMT_SGRBG16:
			case V4L2_PIX_FMT_SRGGB16:
				bytesperline = DIV_ROUND_UP(width * 16, 128) * 16;
				break;

			case V4L2_PIX_FMT_SBGGR24:
			case V4L2_PIX_FMT_SGBRG24:
			case V4L2_PIX_FMT_SGRBG24:
			case V4L2_PIX_FMT_SRGGB24:
				bytesperline = DIV_ROUND_UP(width * 24, 128) * 16;
				break;

			default:
				break;
		}
	}

	sizeimage = bytesperline * height;

	if (info->comp_planes == 1)
	{
		f->fmt.pix.bytesperline = bytesperline;
		f->fmt.pix.sizeimage = sizeimage;
		return 0;
	}

	f->fmt.pix.bytesperline = bytesperline;
	f->fmt.pix.sizeimage = sizeimage;

	for (i = 1; i < info->comp_planes; i++)
	{
		if (private_fmt)
		{
			bytesperline = DIV_ROUND_UP(bytesperline, info->hdiv);
		}
		else
		{
			bytesperline = info->bpp[i] * DIV_ROUND_UP(width, info->hdiv);
		}
		sizeimage = bytesperline * DIV_ROUND_UP(height, info->vdiv);
		f->fmt.pix.sizeimage += sizeimage;
	}

	return 0;

}
int isp_mimo_v4l2_m2m_ioctl_try_fmt_out(struct file *file, void*priv,
					struct v4l2_format *f)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct visp_dev *isp_dev = ctx->device->isp_dev;
	struct v4l2_pix_format_mplane *pix = &f->fmt.pix_mp;
	struct v4l2_format *fmt;
	int i;
	int ret = 0;

	/*
	 * V4L2 specification suggests the driver corrects the
	 * format struct if any of the dimensions is unsupported
	 */
	if (pix->height < VISPM2M_MIN_HEIGHT)
		pix->height = VISPM2M_MIN_HEIGHT;
	else if (pix->height > VISPM2M_MAX_HEIGHT)
		pix->height = VISPM2M_MAX_HEIGHT;

	if (pix->width < VISPM2M_MIN_WIDTH)
		pix->width = VISPM2M_MIN_WIDTH;
	else if (pix->width > VISPM2M_MAX_WIDTH)
		pix->width = VISPM2M_MAX_WIDTH;

	for (i = 0; i < ARRAY_SIZE(visp_raw_fmts); i++) {
		if (visp_raw_fmts[i].fourcc == f->fmt.pix_mp.pixelformat)
			break;
	}
	if (i == ARRAY_SIZE(visp_raw_fmts)) {
		return -EINVAL;
	}

	ret = visp_m2m_cal_imagesize(f);
	if (ret) {
		return -EINVAL;
	}

	fmt = isp_mimo_get_format(ctx, f->type);
	fmt->fmt.pix.sizeimage = f->fmt.pix.sizeimage;
	fmt->fmt.pix.width = f->fmt.pix.width;
	fmt->fmt.pix.height = f->fmt.pix.height;
	fmt->fmt.pix.pixelformat = f->fmt.pix.pixelformat;

	isp_dev->out_sizeimage = f->fmt.pix.sizeimage;
	isp_dev->out_w = f->fmt.pix.width;
	isp_dev->out_h = f->fmt.pix.height;
	isp_dev->out_fmt = f->fmt.pix.pixelformat;
	return 0;
}

int isp_mimo_v4l2_m2m_ioctl_try_fmt_cap(struct file *file, void *priv,
													struct v4l2_format *f)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct visp_dev *isp_dev = ctx->device->isp_dev;
	struct v4l2_pix_format_mplane *pix = &f->fmt.pix_mp;
	struct v4l2_format *fmt;
	int i;
	int ret;

	/*
	 * V4L2 specification suggests the driver corrects the
	 * format struct if any of the dimensions is unsupported
	 */
	if (pix->height < VISPM2M_MIN_HEIGHT)
		pix->height = VISPM2M_MIN_HEIGHT;
	else if (pix->height > VISPM2M_MAX_HEIGHT)
		pix->height = VISPM2M_MAX_HEIGHT;

	if (pix->width < VISPM2M_MIN_WIDTH)
		pix->width = VISPM2M_MIN_WIDTH;
	else if (pix->width > VISPM2M_MAX_WIDTH)
		pix->width = VISPM2M_MAX_WIDTH;

	for (i = 0; i < ARRAY_SIZE(visp_mp_fmts); i++) {
//		fmt = &visp_mp_fmts[i];
		if (visp_mp_fmts[i].fourcc == f->fmt.pix_mp.pixelformat)
			break;
	}
	if (i == ARRAY_SIZE(visp_mp_fmts)) {
		return -EINVAL;
	}

	ret = visp_m2m_cal_imagesize(f);
	if (ret) {
		return -EINVAL;
	}

	fmt = isp_mimo_get_format(ctx, f->type);
	fmt->fmt.pix.sizeimage = f->fmt.pix.sizeimage;
	fmt->fmt.pix.width = f->fmt.pix.width;
	fmt->fmt.pix.height = f->fmt.pix.height;
	fmt->fmt.pix.pixelformat = f->fmt.pix.pixelformat;

	isp_dev->cap_sizeimage = f->fmt.pix.sizeimage;
	isp_dev->cap_w = f->fmt.pix.width;
	isp_dev->cap_h = f->fmt.pix.height;
	isp_dev->cap_fmt = f->fmt.pix.pixelformat;
	return 0;
}

int isp_mimo_v4l2_m2m_ioctl_enum_fmt_cap(struct file *file, void *priv,
							struct v4l2_fmtdesc *f)
{
	if (f->index >= ARRAY_SIZE(visp_mp_fmts))
		return -EINVAL;

	f->pixelformat = visp_mp_fmts[f->index].fourcc;
    return 0;

}

int isp_mimo_v4l2_m2m_ioctl_enum_fmt_out(struct file *file, void *priv,
						struct v4l2_fmtdesc *f)
{
	if (f->index >= ARRAY_SIZE(visp_raw_fmts))
		return -EINVAL;

	f->pixelformat = visp_raw_fmts[f->index].fourcc;
    return 0;
}

int isp_mimo_v4l2_m2m_ioctl_g_fmt(struct file *file, void *priv,
						struct v4l2_format *f)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct v4l2_format *fmt;

	fmt = isp_mimo_get_format(ctx, f->type);

	if(IS_ERR(fmt))
		return -EINVAL;

	f->fmt.pix = fmt->fmt.pix;
	f->fmt.pix.height = fmt->fmt.pix.height;
	f->fmt.pix.width = fmt->fmt.pix.width;
	f->fmt.pix.sizeimage = fmt->fmt.pix.sizeimage;
	
	return 0;	
}

  int isp_mimo_v4l2_m2m_ioctl_s_fmt_out(struct file *file, void *priv,
			     			struct v4l2_format *f)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct v4l2_format *fmt;
	
	isp_mimo_v4l2_m2m_ioctl_try_fmt_out(file, priv, f);

	fmt = isp_mimo_get_format(ctx, f->type);

	if(IS_ERR(fmt))
		return -EINVAL;

	fmt->fmt.pix = f->fmt.pix;
	fmt->fmt.pix.sizeimage = f->fmt.pix.sizeimage;
	fmt->fmt.pix.width = f->fmt.pix.width;
	fmt->fmt.pix.height = f->fmt.pix.height;
	fmt->fmt.pix.pixelformat = f->fmt.pix.pixelformat;

	return 0;	
}
  int isp_mimo_v4l2_m2m_ioctl_s_fmt_cap(struct file *file, void *priv,
			     			struct v4l2_format *f)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct v4l2_format *fmt;
	
	isp_mimo_v4l2_m2m_ioctl_try_fmt_cap(file, priv, f);

	fmt = isp_mimo_get_format(ctx, f->type);

	if(IS_ERR(fmt))
		return -EINVAL;

	fmt->fmt.pix = f->fmt.pix;
	fmt->fmt.pix.sizeimage = f->fmt.pix.sizeimage;
	fmt->fmt.pix.width = f->fmt.pix.width;
	fmt->fmt.pix.height = f->fmt.pix.height;
	fmt->fmt.pix.pixelformat = f->fmt.pix.pixelformat;

	return 0;	
}

int isp_mimo_v4l2_m2m_ioctl_streamon(struct file *file, void *fh,
                           			enum v4l2_buf_type type)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) fh;

	return v4l2_m2m_streamon(file, ctx->fh.m2m_ctx, type);
}

int isp_mimo_v4l2_m2m_ioctl_streamoff(struct file *file, void *fh,
                            			enum v4l2_buf_type type)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
	if (on) {
		MediaIspStreamOff(ctx->device->isp_dev, 0, 0);
		on = 0;
	}
	return v4l2_m2m_streamoff(file, ctx->fh.m2m_ctx, type);
}

int isp_mimo_v4l2_m2m_ioctl_reqbufs(struct file *file, void *fh,
                            		struct v4l2_requestbuffers *rb)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
        return v4l2_m2m_ioctl_reqbufs(file, ctx->fh.m2m_ctx, rb);
}

int isp_mimo_v4l2_m2m_ioctl_querybuf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
        return v4l2_m2m_ioctl_querybuf(file, ctx->fh.m2m_ctx, buf);
}

int isp_mimo_v4l2_m2m_ioctl_qbuf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
        return v4l2_m2m_ioctl_qbuf(file, ctx->fh.m2m_ctx, buf);
}

int isp_mimo_v4l2_m2m_ioctl_dqbuf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf)
{
	int status;
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;

	status = v4l2_m2m_ioctl_dqbuf(file, ctx->fh.m2m_ctx, buf);

	return status;
}

int isp_mimo_v4l2_m2m_ioctl_prepare_buf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;

	return v4l2_m2m_ioctl_prepare_buf(file, ctx->fh.m2m_ctx, buf);
}

int isp_mimo_v4l2_m2m_ioctl_create_bufs(struct file *file, void *fh,
                            		struct v4l2_create_buffers *create)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;

	return v4l2_m2m_ioctl_create_bufs(file, ctx->fh.m2m_ctx, create);
}

int isp_mimo_v4l2_m2m_ioctl_expbuf(struct file *file, void *fh,
                            		struct v4l2_exportbuffer *eb)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;

	return v4l2_m2m_ioctl_expbuf(file, ctx->fh.m2m_ctx, eb);
}

int isp_mimo_queue_setup(struct vb2_queue *vq,
			   unsigned int *nbuffers, unsigned int *nplanes,
			   unsigned int sizes[], struct device *alloc_devs[])
{
	struct isp_mimo_ctx *ctx = vb2_get_drv_priv(vq);

	struct v4l2_format *fmt;

	fmt = isp_mimo_get_format(ctx, vq->type);
	if(IS_ERR(fmt))
		return -EINVAL;

	*nplanes = 1;	
	sizes[0] = fmt->fmt.pix.sizeimage;


	return 0;
}

int isp_mimo_buf_prepare(struct vb2_buffer *vb)
{
	struct isp_mimo_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);

	struct v4l2_format *fmt;

	fmt = isp_mimo_get_format(ctx, vb->type);
	if(IS_ERR(fmt))
		return -EINVAL;


	visp_v4l2_dbg(1, debug, v4l2_dev, "type: %d\n", vb->vb2_queue->type);

	vb2_set_plane_payload(vb, 0, fmt->fmt.pix.sizeimage);

	return 0;
}

void isp_mimo_buf_queue(struct vb2_buffer *vb)
{
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct isp_mimo_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);

	v4l2_m2m_buf_queue(ctx->fh.m2m_ctx, vbuf);
}

int isp_mimo_start_streaming(struct vb2_queue *q, unsigned int count)
{
	struct isp_mimo_ctx *ctx = vb2_get_drv_priv(q);
	ctx->sequence = 0;
	return 0;
}

void isp_mimo_stop_streaming(struct vb2_queue *q)
{
	struct isp_mimo_ctx *ctx = vb2_get_drv_priv(q);
	struct vb2_v4l2_buffer *vbuf;

	for (;;) {
		if (V4L2_TYPE_IS_OUTPUT(q->type))
			vbuf = v4l2_m2m_src_buf_remove(ctx->fh.m2m_ctx);
		else
			vbuf = v4l2_m2m_dst_buf_remove(ctx->fh.m2m_ctx);
		if (vbuf == NULL)
			return;
		v4l2_m2m_buf_done(vbuf, VB2_BUF_STATE_ERROR);
	}
}

static int subdev_event_subscribe(struct v4l2_subdev *sd,
                                  struct v4l2_fh *fh,
                                  struct v4l2_event_subscription *sub)
{
    return v4l2_event_subscribe(fh, sub, 10, NULL);
}

static int subdev_s_stream(struct v4l2_subdev *sd, int enable)
{
    return 0;
}

static const struct v4l2_subdev_video_ops subdev_video_ops = {
    .s_stream = subdev_s_stream,
};


static long visp_return_rpu_id(struct v4l2_subdev *sd, void *arg)
{
    long ret = 0;
    if(!arg)
    {
        dev_err(sd->dev, "%s %d NULL ARG \n",__func__,__LINE__);
    }

    struct isp_mimo *device = v4l2_get_subdevdata(sd);
    if(!device)
    {
        pr_err("%s %d NULLL\n", __func__, __LINE__);
    }


    struct isp_rpu *temp = (struct isp_rpu *)arg;

    temp->rpu = device->isp_dev->isp_rpu;
    temp->isp = device->isp_dev->id;

    return ret;
}

static long visp_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = -EINVAL;
	switch (cmd)
	{
		case VISP_GET_RPU_ID:
			ret = visp_return_rpu_id(sd, arg);
			break;
		default:
			break;
	}
	return ret;
}




static const struct v4l2_subdev_core_ops subdev_core_ops = {
    .ioctl = visp_priv_ioctl,
    .subscribe_event = subdev_event_subscribe,
//    .unsubscribe_event = subdev_event_unsubscribe,
};

static const struct v4l2_subdev_ops subdev_ops = {
    .video = &subdev_video_ops,
    .core = &subdev_core_ops,
};



static int event_mmap(struct file *filp, struct vm_area_struct *vma)
{
//struct isp_mimo *device = video_drvdata(filp);
struct isp_mimo *device = filp->private_data;

    unsigned long pfn = device->event_shm.phy_addr >> PAGE_SHIFT;
    size_t size = vma->vm_end - vma->vm_start;

    if (size > device->event_shm.size)
        return -EINVAL;

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    return remap_pfn_range(vma,
                           vma->vm_start,
                           pfn,
                           size,
                           vma->vm_page_prot);
}

static int event_open(struct inode *inode, struct file *file)
{
 //   struct miscdevice *misc = container_of(file->f_inode->i_cdev, struct miscdevice, this_device->dev);
struct miscdevice *misc = file->private_data;
   struct isp_mimo *device = container_of(misc, struct isp_mimo, event_misc);
    file->private_data = device;
    return 0;
}


static const struct file_operations event_fops = {
    .owner = THIS_MODULE,
    .open = event_open,
    .mmap = event_mmap,
};




static const struct v4l2_file_operations isp_mimo_fops = {
	.owner		= THIS_MODULE,
	.open		= isp_mimo_open,
	.release	= isp_mimo_release,
	.poll		= v4l2_m2m_fop_poll,
	.unlocked_ioctl	= video_ioctl2,
	.mmap		= v4l2_m2m_fop_mmap,
	//.mmap		= mmap_new,
};

/*
 * File operations
 */
static const struct v4l2_ioctl_ops isp_mimo_ioctl_ops = {
	.vidioc_querycap	 = isp_mimo_v4l2_m2m_ioctl_querycap,
	.vidioc_enum_fmt_vid_cap = isp_mimo_v4l2_m2m_ioctl_enum_fmt_cap,
	.vidioc_g_fmt_vid_cap	 = isp_mimo_v4l2_m2m_ioctl_g_fmt,
	.vidioc_try_fmt_vid_cap	 = isp_mimo_v4l2_m2m_ioctl_try_fmt_cap,
	.vidioc_s_fmt_vid_cap	 = isp_mimo_v4l2_m2m_ioctl_s_fmt_cap,
	.vidioc_enum_fmt_vid_out = isp_mimo_v4l2_m2m_ioctl_enum_fmt_out,
	.vidioc_g_fmt_vid_out	 = isp_mimo_v4l2_m2m_ioctl_g_fmt,
	.vidioc_try_fmt_vid_out	 = isp_mimo_v4l2_m2m_ioctl_try_fmt_out,
	.vidioc_s_fmt_vid_out	 = isp_mimo_v4l2_m2m_ioctl_s_fmt_out,
	.vidioc_reqbufs		 = isp_mimo_v4l2_m2m_ioctl_reqbufs,
	.vidioc_querybuf	 = isp_mimo_v4l2_m2m_ioctl_querybuf,
	.vidioc_qbuf		 = isp_mimo_v4l2_m2m_ioctl_qbuf,
	.vidioc_dqbuf		 = isp_mimo_v4l2_m2m_ioctl_dqbuf,
	.vidioc_prepare_buf	 = isp_mimo_v4l2_m2m_ioctl_prepare_buf,
	.vidioc_create_bufs	 = isp_mimo_v4l2_m2m_ioctl_create_bufs,
	.vidioc_expbuf		 = isp_mimo_v4l2_m2m_ioctl_expbuf,
	.vidioc_streamon	 = isp_mimo_v4l2_m2m_ioctl_streamon,
	.vidioc_streamoff	 = isp_mimo_v4l2_m2m_ioctl_streamoff,
	//.vidioc_subscribe_event     = visp_videoc_subscribe_event,
    //.vidioc_unsubscribe_event   = v4l2_event_unsubscribe,
};

static const struct video_device isp_mimo_video_dev = {
	.name		= MEM2MEM_NAME,
	.vfl_dir	= VFL_DIR_M2M,
	.fops		= &isp_mimo_fops,
	.device_caps	= V4L2_CAP_VIDEO_M2M | V4L2_CAP_STREAMING,
	.ioctl_ops	= &isp_mimo_ioctl_ops,
	.minor		= -1,
	.release	= video_device_release_empty,
};

static const struct v4l2_m2m_ops m2m_ops = {
	.device_run	= isp_mimo_device_run,
};

static const struct vb2_ops isp_mimo_qops = {
	.queue_setup	 = isp_mimo_queue_setup,
	.buf_prepare	 = isp_mimo_buf_prepare,
	.buf_queue	 = isp_mimo_buf_queue,
	.start_streaming = isp_mimo_start_streaming,
	.stop_streaming  = isp_mimo_stop_streaming,
	.wait_prepare	 = vb2_ops_wait_prepare,
	.wait_finish	 = vb2_ops_wait_finish,
};

inline struct isp_mimo_ctx *file2ctx(struct file *file)
{
	return container_of(file->private_data, struct isp_mimo_ctx, fh);
}




int queue_init(void *priv, struct vb2_queue *src_vq,
		      struct vb2_queue *dst_vq)
{
	struct isp_mimo_ctx *ctx = priv;
	int ret;

	src_vq->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	src_vq->io_modes = VB2_MMAP | VB2_DMABUF | VB2_USERPTR;
	src_vq->drv_priv = ctx;
	src_vq->buf_struct_size = sizeof(struct v4l2_m2m_buffer);
	src_vq->ops = &isp_mimo_qops;
	src_vq->mem_ops = &vb2_dma_contig_memops;
	src_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	src_vq->lock = &ctx->device->lock;
	src_vq->dev = ctx->device->v4l2_dev.dev;

	ret = vb2_queue_init(src_vq);
	if (ret)
		return ret;

	dst_vq->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dst_vq->io_modes = VB2_MMAP | VB2_DMABUF | VB2_USERPTR;
	dst_vq->drv_priv = ctx;
	dst_vq->buf_struct_size = sizeof(struct v4l2_m2m_buffer);
	dst_vq->ops = &isp_mimo_qops;
	dst_vq->mem_ops = &vb2_dma_contig_memops;
	dst_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	dst_vq->lock = &ctx->device->lock;
	dst_vq->dev = ctx->device->v4l2_dev.dev;

	return vb2_queue_init(dst_vq);
}
static char dev_open = 0;
int isp_mimo_open(struct file *file)
{
	static int device_open_count=0;

	++device_open_count;

	if(!strcmp(current->comm,"v4l_id")) {
		visp_pr_debug("===== [VISP_M2M] Device opened %d times \n", device_open_count);
		//return 0;
	}

	struct isp_mimo_ctx *ctx = NULL;

	struct isp_mimo *device = video_drvdata(file);
	int rc = 0;


	if (mutex_lock_interruptible(&device->lock))
		return -ERESTARTSYS;
	if(!ctx) {
		ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
		if (!ctx) {
			rc = -ENOMEM;
			goto open_unlock;
		}
        ctx->id = device_open_count;
		v4l2_fh_init(&ctx->fh, video_devdata(file));
		file->private_data = &ctx->fh;
		ctx->device = device;

		ctx->fh.m2m_ctx = v4l2_m2m_ctx_init(device->m2m_dev, ctx, &queue_init);

		if (IS_ERR(ctx->fh.m2m_ctx)) {
			rc = PTR_ERR(ctx->fh.m2m_ctx);

			v4l2_fh_exit(&ctx->fh);
			kfree(ctx);
			goto open_unlock;
		}

		v4l2_fh_add(&ctx->fh);

		/* set default format */
		ctx->fmt[FMT_OUTPUT].type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		ctx->fmt[FMT_OUTPUT].fmt.pix.pixelformat = FORMAT_OUT_BAYER12;
		ctx->fmt[FMT_OUTPUT].fmt.pix.width = DEFAULT_WIDTH;
		ctx->fmt[FMT_OUTPUT].fmt.pix.height = DEFAULT_HEIGHT;
		ctx->fmt[FMT_OUTPUT].fmt.pix.colorspace = V4L2_COLORSPACE_RAW;

		ctx->fmt[FMT_CAPTURE].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		ctx->fmt[FMT_CAPTURE].fmt.pix.pixelformat = FORMAT_CAP_NV12;
		ctx->fmt[FMT_CAPTURE].fmt.pix.width = DEFAULT_WIDTH;
		ctx->fmt[FMT_CAPTURE].fmt.pix.height = DEFAULT_HEIGHT;
		ctx->fmt[FMT_CAPTURE].fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

		visp_v4l2_dbg(1, debug, v4l2_dev, "[v4l2] Created instance: [0x%x], m2m_ctx: [0x%x]\n", ctx, ctx->fh.m2m_ctx);
		if(!dev_open) {
			rc = IspDeviceCreateMIMO(device->isp_dev,0);
			dev_open = 1;
		}else {
			visp_pr_debug("===== [VISP_M2M] %s : %d device already opened ..\n",__func__, __LINE__);
		}
	} else {
		file->private_data =  &ctx->fh;
	}

open_unlock:
	mutex_unlock(&device->lock);
	return rc;
}

int isp_mimo_release(struct file *file)
{
	struct isp_mimo *device = video_drvdata(file);
	struct isp_mimo_ctx *ctx = file2ctx(file);

	if (dev_open == 1) {
		dev_open = 0;
	}

	v4l2_fh_del(&ctx->fh);
	v4l2_fh_exit(&ctx->fh);
	mutex_lock(&device->lock);
	v4l2_m2m_ctx_release(ctx->fh.m2m_ctx);
	mutex_unlock(&device->lock);
	kfree(ctx);
	return 0;
}

static int visp_parse_params(struct visp_dev *isp_dev, struct platform_device *pdev)
{
    int ret = 0;
    struct device_node *node = pdev->dev.of_node;

    fwnode_property_read_u32(of_fwnode_handle(node),
            "isp_id", &isp_dev->id);
    if (!node) {
        dev_err(&pdev->dev, "No device tree node found\n");
        return -EINVAL;
    }

    if(isp_dev->id < 0 && isp_dev->id > 6)
    {
        dev_err(&pdev->dev, "Invalid ISP Id %d\n",isp_dev->id);
        return -EINVAL;
    }
    dev_dbg(&pdev->dev, "Found visp device in device tree.\n");

	// Read string property for SS-MODE-I0 (LIMO, etc.)
    ret = of_property_read_string(node, "xlnx,io_mode", &isp_dev->ss_mode_i0);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read xlnx,io_mode\n");
        return ret;
    } else {
        dev_dbg(&pdev->dev, "xlnx,io_mode: %s\n", isp_dev->ss_mode_i0);
    }

	// Read stream info (multi-stream, single-stream)
    ret = of_property_read_u32(node, "xlnx,num_streams", &isp_dev->num_streams);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read xlnx,num_streams property\n");
        return ret;
    } else {
        dev_dbg(&pdev->dev, "xlnx,num_streams: %u\n", isp_dev->num_streams);
    }
        dev_dbg(&pdev->dev, "xlnx,num_streams: %u\n", isp_dev->num_streams);

    ret = of_property_read_u32(node, "xlnx,mem_inputs", &isp_dev->isp_mem);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read xlnx,mem_inputs property\n");
        return ret;
    } else {
        dev_dbg(&pdev->dev, "xlnx,mem_inputs: %u\n", isp_dev->isp_mem);
    }

    ret = of_property_read_u32(node, "xlnx,rpu", &isp_dev->isp_rpu);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read xlnx,rpu property\n");
        return ret;
    } else {
        dev_dbg(&pdev->dev, "xlnx,rpu: %u\n", isp_dev->isp_rpu);
    }

	return 0;
}


int isp_mimo_probe(struct platform_device *pdev)
{
	static int probe_cnt = 0;

	struct isp_mimo *device;
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct video_device *vfd;
	struct v4l2_device *v4l2_dev;
	int ret = 0;
	int num_mems = 0, i;

	if(probe_cnt >= MAX_SUPPORTED_DEVICE_COUNT){
		return 0;
	}
	pdev->id = probe_cnt;


	device = devm_kzalloc(dev, sizeof(*device), GFP_KERNEL);
	if(!device)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    v4l2_subdev_init(&device->subdev, &subdev_ops);
    strscpy(device->subdev.name, "visp-mimo-video-subdev", sizeof(device->subdev.name));

    device->subdev.owner = THIS_MODULE;
    device->subdev.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    device->subdev.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;

    v4l2_set_subdevdata(&device->subdev, device);
    device->subdev.dev = &pdev->dev;

	ret = v4l2_device_register(&pdev->dev, &device->v4l2_dev);
	if (ret) {
		dev_err(dev, "could not register video device rc=%d\n", ret);
		return ret;
	}

    ret = v4l2_device_register_subdev(&device->v4l2_dev, &device->subdev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to register subdev\n");
        return ret;
    }
    ret = v4l2_device_register_subdev_nodes(&device->v4l2_dev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to register subdev\n");
        return ret;
    }
	v4l2_dev = &device->v4l2_dev;
	device->video_dev = isp_mimo_video_dev;
	vfd = &device->video_dev;
	vfd->lock = &device->lock;
	vfd->v4l2_dev = &device->v4l2_dev;

    device->video_dev.v4l2_dev = &device->v4l2_dev;
	video_set_drvdata(vfd, device);
	platform_set_drvdata(pdev, device);
	snprintf(vfd->name, sizeof(vfd->name), "%s", MEM2MEM_NAME);

	device->m2m_dev = v4l2_m2m_init(&m2m_ops);
	if (IS_ERR(device->m2m_dev)) {
		v4l2_err(v4l2_dev, "Failed to init mem2mem device\n");
		ret = PTR_ERR(device->m2m_dev);
		goto err_v4l2;
	}

	device->isp_dev = devm_kzalloc(&pdev->dev, sizeof(struct visp_dev), GFP_KERNEL);
	if (!device->isp_dev)
		return -ENOMEM;

	mutex_init(&device->isp_dev->mlock);
	mutex_init(&device->isp_dev->ctrl_lock);
	device->isp_dev->dev = &pdev->dev;

	ret = visp_parse_params(device->isp_dev, pdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to parse params\n");
		return -EINVAL;
	}

	ret = xlnx_link_mbox(device->isp_dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to init mbox\n");
		return -EINVAL;
	}
   num_mems = of_count_phandle_with_args(pdev->dev.of_node, "memory-region", NULL);
    if (num_mems < 0) {
        pr_err("%s no memory for calibration\n", __func__);
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
        pr_debug("reserve name:%s base:%llx size:%llx\n", rmem->name, rmem->base, rmem->size);
        device->reserve_mem.pa = rmem->base;
        device->reserve_mem.size = rmem->size;
    }
	ret = video_register_device(vfd, VFL_TYPE_VIDEO, 0);
	if (ret) {
		v4l2_err(v4l2_dev, "Failed to register video device\n");
		goto err_m2m;
	}

	device->event_shm.virt_addr = (void *)__get_free_pages(GFP_KERNEL, 0);
    device->event_shm.size = PAGE_SIZE;
    memset(device->event_shm.virt_addr, 9, device->event_shm.size);
    device->event_shm.phy_addr = virt_to_phys(device->event_shm.virt_addr);
    mutex_init(&device->event_shm.event_lock);
	device->reserve_mem.va =
        ioremap_wc(device->reserve_mem.pa, device->reserve_mem.size);

    device->event_misc.minor = MISC_DYNAMIC_MINOR;
    device->event_misc.name = "event_shm";
    device->event_misc.fops = &event_fops;
    device->event_misc.mode = 0666;
    device->event_misc.parent = &pdev->dev;

    ret = misc_register(&device->event_misc);
    if (ret)
    {
        return ret;
    }

	probe_cnt +=1;

	return 0;

err_m2m:
	v4l2_m2m_release(device->m2m_dev);
err_v4l2:
	v4l2_device_unregister(&device->v4l2_dev);

	return ret;
}

void isp_mimo_remove(struct platform_device *pdev)
{
	struct isp_mimo *device = platform_get_drvdata(pdev);

	if(pdev->id >= 0) {
		video_unregister_device(&device->video_dev);
		v4l2_m2m_release(device->m2m_dev);
		v4l2_device_unregister(&device->v4l2_dev);
	}
	return;
}

static const struct of_device_id isp_mimo_dt_ids[] = {
	{.compatible = "xlnx,visp-ss-mimo-1.0",},
	{ },
};

static struct platform_driver isp_mimo_driver = {
	.probe		= isp_mimo_probe,
	.remove		= isp_mimo_remove,
	.driver		= {
		.name	= MEM2MEM_NAME,
		.of_match_table = isp_mimo_dt_ids,
	},
};


MODULE_DEVICE_TABLE(of, isp_mimo_dt_ids);
module_platform_driver(isp_mimo_driver);

MODULE_DESCRIPTION("AMD ISP MIMO driver");
MODULE_AUTHOR("AMD ISP SW Team");
MODULE_LICENSE("GPL");
