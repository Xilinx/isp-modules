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


#ifndef __AMD_ISP_MIMO_H__
#define __AMD_ISP_MIMO_H__


#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <media/v4l2-mem2mem.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-common.h>
#include <media/videobuf2-dma-contig.h>
#include <media/v4l2-event.h>
#include <media/videobuf2-v4l2.h>

#include <vvcam_isp_driver.h>
#include "sensor_cmd.h"
#include "mbox_api.h"
#include "mbox_cmd.h"
#include "vvcam_isp_app.h"
#include "amd_mbox_driver.h"
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <linux/miscdevice.h>

#define MAX_SUPPORTED_DEVICE_COUNT 1
#define MEM2MEM_NAME		"isp_m2m"

#define FORMAT_OUT_BAYER8 V4L2_PIX_FMT_SRGGB8
#define FORMAT_OUT_BAYER10 V4L2_PIX_FMT_SRGGB10
#define FORMAT_OUT_BAYER12 V4L2_PIX_FMT_SRGGB12
#define FORMAT_CAP_RGB V4L2_PIX_FMT_RGB24
#define FORMAT_CAP_NV12 V4L2_PIX_FMT_NV12
#define FORMAT_CAP_NV16 V4L2_PIX_FMT_NV16

#define MAX_WIDTH (1920) 
#define MAX_HEIGHT (1080) 
#define VISPM2M_MIN_WIDTH (64)
#define VISPM2M_MIN_HEIGHT (64)

#define VISPM2M_MAX_WIDTH (4096)
#define VISPM2M_MAX_HEIGHT (4096)

#
#define FMT_OUTPUT	(0)
#define FMT_CAPTURE	(1)
#define FMT_MAX	(2)

#define DEFAULT_WIDTH	(1920)
#define DEFAULT_HEIGHT	(1080)

struct vvcam_video_event_shm {
    struct mutex event_lock;
    uint64_t phy_addr;
    void *virt_addr;
    uint32_t size;
};

struct vvcam_video_reserve_mem {
    dma_addr_t pa;
    int size;
    void *va;
};


struct isp_mimo {
	struct v4l2_device	v4l2_dev;
	struct video_device	video_dev;
     struct v4l2_subdev subdev;
	struct v4l2_m2m_dev	*m2m_dev;
	struct mutex 		lock;
    struct vvcam_isp_dev 	*isp_dev;
    struct media_device mdev;
	struct vvcam_video_reserve_mem reserve_mem;
    struct vvcam_video_event_shm event_shm;
    struct work_struct event_work;
    struct miscdevice event_misc;
};

struct visp_format
{
	uint32_t code;
	uint32_t fourcc;
};


struct isp_mimo_ctx {	
	struct v4l2_fh		fh;
	struct isp_mimo	*device;
	struct v4l2_format 	fmt[2];
	uint64_t 		sequence;
    int id;
};

extern int MediaIspHalSetFmt(struct vvcam_isp_dev *isp_dev, int Pad, MediaFmt *Format);
extern int MediaIspDeviceStreamOn(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
extern int MediaIspDeviceStreamOnOut(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
extern int MediaIspStreamOff(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
extern int MediaIspDeviceDeque(struct vvcam_isp_dev *isp_dev, uint8_t Port);
extern int vvcam_isp_pads_init(struct vvcam_isp_dev *isp_dev);
extern int MediaIspDeviceMcmSetFormat(struct vvcam_isp_dev *isp_dev , uint8_t Port);
extern int MediaIspDeviceSetFormat(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);


int xlnx_link_mbox(struct vvcam_isp_dev *isp_dev);
int IspDeviceCreateMIMO(struct vvcam_isp_dev *isp_dev , uint8_t Port);
int IspDeviceDistroyMIMO(struct vvcam_isp_dev *isp_dev , uint8_t Port);

int isp_mimo_open(struct file *file);
  int isp_mimo_release(struct file *file);
  int isp_mimo_probe(struct platform_device *pdev);
  void isp_mimo_remove(struct platform_device *pdev);

  int isp_mimo_v4l2_m2m_ioctl_querycap(struct file *file, void *priv,
					struct v4l2_capability *cap);
  int isp_mimo_v4l2_m2m_ioctl_try_fmt_out(struct file *file, void*priv, 
					struct v4l2_format *f);
  int isp_mimo_v4l2_m2m_ioctl_try_fmt_cap(struct file *file, void*priv, 
					struct v4l2_format *f);
  int isp_mimo_v4l2_m2m_ioctl_enum_fmt_cap(struct file *file, void *priv,
							struct v4l2_fmtdesc *f);
  int isp_mimo_v4l2_m2m_ioctl_enum_fmt_out(struct file *file, void *priv,
						struct v4l2_fmtdesc *f);
  int isp_mimo_v4l2_m2m_ioctl_g_fmt(struct file *file, void *priv,
						struct v4l2_format *f);
  int isp_mimo_v4l2_m2m_ioctl_s_fmt_out(struct file *file, void *priv,
			     			struct v4l2_format *f);
  int isp_mimo_v4l2_m2m_ioctl_s_fmt_cap(struct file *file, void *priv,
			     			struct v4l2_format *f);
  int isp_mimo_v4l2_m2m_ioctl_streamon(struct file *file, void *fh,
                           			enum v4l2_buf_type type);
  int isp_mimo_v4l2_m2m_ioctl_streamoff(struct file *file, void *fh,
                            			enum v4l2_buf_type type);
  int isp_mimo_v4l2_m2m_ioctl_reqbufs(struct file *file, void *fh,
                            		struct v4l2_requestbuffers *rb);
  int isp_mimo_v4l2_m2m_ioctl_querybuf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf);
  int isp_mimo_v4l2_m2m_ioctl_qbuf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf);
  int isp_mimo_v4l2_m2m_ioctl_dqbuf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf);
  int isp_mimo_v4l2_m2m_ioctl_prepare_buf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf);
  int isp_mimo_v4l2_m2m_ioctl_create_bufs(struct file *file, void *fh,
                            		struct v4l2_create_buffers *create);
  int isp_mimo_v4l2_m2m_ioctl_expbuf(struct file *file, void *fh,
                            		struct v4l2_exportbuffer *eb);

  void isp_mimo_device_run(void *priv);

  int isp_mimo_queue_setup(struct vb2_queue *vq,
			   unsigned int *nbuffers, unsigned int *nplanes,
			   unsigned int sizes[], struct device *alloc_devs[]);
  int isp_mimo_buf_prepare(struct vb2_buffer *vb);
  void isp_mimo_buf_queue(struct vb2_buffer *vb);
  int isp_mimo_start_streaming(struct vb2_queue *q, unsigned int count);
  void isp_mimo_stop_streaming(struct vb2_queue *q);
  int queue_init(void *priv, struct vb2_queue *src_vq,
		      				struct vb2_queue *dst_vq);

#endif
