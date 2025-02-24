#include <amd_isp_mimo_driver.h>

/* version 1.0 */
#ifndef DEBUG_ENABLE
#define visp_pr_info(fmt, ...) ;
#define visp_pr_debug(fmt, ...) ;
#define visp_pr_err(fmt, ...) ;
#else
#define visp_pr_info pr_info
#define visp_pr_debug pr_debug
#define visp_pr_err pr_err
#endif
#define visp_v4l2_dbg(fmt, ...) ;


static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "debug level (0-3)");

struct isp_mimo_ctx;

static const struct of_device_id isp_mimo_dt_ids[] = {
	{ .compatible = "xlnx,visp-ss-mimo", .data = NULL },
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

static const struct v4l2_file_operations isp_mimo_fops = {
	.owner		= THIS_MODULE,
	.open		= isp_mimo_open,
	.release	= isp_mimo_release,
	.poll		= v4l2_m2m_fop_poll,
	.unlocked_ioctl	= video_ioctl2,
	.mmap		= v4l2_m2m_fop_mmap,
};

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

void inspect_source_buffers(struct v4l2_m2m_ctx *m2m_ctx, dma_addr_t *s, dma_addr_t *d);
void inspect_source_buffers(struct v4l2_m2m_ctx *m2m_ctx, dma_addr_t *s, dma_addr_t *d)
{
    struct vb2_queue *vb2_q;
    struct vb2_buffer *vb;
    dma_addr_t dma_addr;
    unsigned int i;
    visp_pr_info("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);

    vb2_q = v4l2_m2m_get_src_vq(m2m_ctx);
    for (i = 0; i < vb2_q->max_num_buffers; i++) {
        vb = vb2_q->bufs[i];
        if(!vb) {
            break;
        }
        dma_addr = vb2_dma_contig_plane_dma_addr(vb, 0);
	s[i] = dma_addr;
        visp_pr_info("=====[VISP_M2M] SRC Buffer %d DMA address: 0x%x\n", i, dma_addr);
    }

    vb2_q = v4l2_m2m_get_dst_vq(m2m_ctx);
    for (i = 0; i < vb2_q->max_num_buffers; i++) {
        vb = vb2_q->bufs[i];
        if(!vb) {
            break;
        }
        dma_addr = vb2_dma_contig_plane_dma_addr(vb, 0);
	d[i] = dma_addr;
        visp_pr_info("=====[VISP_M2M] DST Buffer %d DMA address: 0x%x\n", i, dma_addr);
    }
}

  void isp_mimo_device_run(void *priv)
{
#if 0
	static int a=0;
	visp_pr_info("===== [VISP_M2M] %s : %d device run count [%d] \n",__func__, __LINE__,++a);
#endif

	struct isp_mimo_ctx *ctx = priv;
	struct isp_mimo *device = ctx->device;
	struct vb2_v4l2_buffer *src_vb, *dst_vb;

	dma_addr_t input_addr[2], output_addr[4];
	struct vb2_v4l2_buffer *src_buf, *dst_buf;
	int s_cnt, d_cnt;
	struct isp_mimo_ctx *curr_ctx=ctx;

	s_cnt = v4l2_m2m_num_src_bufs_ready(ctx->fh.m2m_ctx);
	d_cnt = v4l2_m2m_num_dst_bufs_ready(ctx->fh.m2m_ctx);
	visp_pr_info("===== [VISP_M2M] %s : %d src & dest buf num [%d-%d]  \n",__func__, __LINE__,s_cnt,d_cnt);

	src_buf = v4l2_m2m_next_src_buf(ctx->fh.m2m_ctx);

        dst_buf = v4l2_m2m_next_dst_buf(ctx->fh.m2m_ctx);

        inspect_source_buffers(ctx->fh.m2m_ctx, input_addr, output_addr);
	visp_pr_info("===== [VISP_M2M] %s : %d src[0x%x 0x%x] dst[0x%x 0x%x 0x%x 0x%x]  \n",__func__, __LINE__,
		input_addr[0], input_addr[1],output_addr[0],output_addr[1],output_addr[2],output_addr[3]);

	device->isp_dev->ip_a[0]=input_addr[0];
	device->isp_dev->ip_a[1]=input_addr[0];
	device->isp_dev->op_a[0]=output_addr[0];
	device->isp_dev->op_a[1]=output_addr[1];
	device->isp_dev->op_a[2]=output_addr[2];
	device->isp_dev->op_a[3]=output_addr[3];

        MediaIspDeviceStreamOn(device->isp_dev, 0, CAMDEV_BUFCHAIN_RDMA);
	MediaIspDeviceDeque(device->isp_dev, 0);

	visp_pr_info("[VISP_M2M] ===== SYNC_WAITB %s : %d\n",__func__, __LINE__);
	device->isp_dev->apu_wait_for_isp_frame_done = 1;
	wait_event_interruptible(device->isp_dev->wq_frame_done_finished, ! device->isp_dev->apu_wait_for_isp_frame_done);
	visp_pr_info("[VISP_M2M] ===== SYNC_WAITA %s : %d isp_dq_out_index = %d \n",__func__, __LINE__,device->isp_dev->isp_dq_out_index);

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
#if 0 //--TODO
	MediaIspStreamOff(device->isp_dev, 0, 6);
	MediaIspStreamOff(device->isp_dev, 0, 0);
#endif
}

  struct v4l2_format* isp_mimo_get_format(struct isp_mimo_ctx *ctx, enum v4l2_buf_type type);
  struct v4l2_format* isp_mimo_get_format(struct isp_mimo_ctx *ctx, enum v4l2_buf_type type)
{
	visp_pr_info("===== [VISP_M2M] %s : %d v4l2_buf_type is %d out_type %d cap_type %d \n",__func__, __LINE__,type,V4L2_BUF_TYPE_VIDEO_OUTPUT, V4L2_BUF_TYPE_VIDEO_CAPTURE);
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
	visp_pr_info("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
	strscpy(cap->driver, MEM2MEM_NAME, sizeof(cap->driver));
	strscpy(cap->card, MEM2MEM_NAME, sizeof(cap->card));
	snprintf(cap->bus_info, sizeof(cap->bus_info),
			"platform:%s", MEM2MEM_NAME);
	return 0;
}
  int isp_mimo_v4l2_m2m_ioctl_try_fmt_out(struct file *file, void*priv,
					struct v4l2_format *f)
{
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct vvcam_isp_dev *isp_dev = ctx->device->isp_dev;

	visp_pr_debug("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);

	if(f->fmt.pix.width > MAX_WIDTH)
		f->fmt.pix.width = MAX_WIDTH;
	
	if(f->fmt.pix.height > MAX_HEIGHT)
		f->fmt.pix.height = MAX_HEIGHT;

	switch(f->fmt.pix.pixelformat) {
		case FORMAT_OUT_BAYER10:
			visp_pr_debug("===== [VISP_M2M] %s : %d FORMAT_OUT_BAYER10\n",__func__, __LINE__);
			f->fmt.pix.sizeimage = f->fmt.pix.width * f->fmt.pix.height * 2; 
			break;
		case FORMAT_OUT_BAYER12:
			visp_pr_debug("===== [VISP_M2M] %s : %d FORMAT_OUT_BAYER12\n",__func__, __LINE__);
			f->fmt.pix.sizeimage = f->fmt.pix.width * f->fmt.pix.height * 2; 
			break;
		case FORMAT_OUT_BAYER8:
			visp_pr_debug("===== [VISP_M2M] %s : %d FORMAT_OUT_BAYER8\n",__func__, __LINE__);
			f->fmt.pix.sizeimage = f->fmt.pix.width * f->fmt.pix.height; 
			break;
		default:
			visp_pr_err("===== [VISP_M2M] %s : %d !!!!! ERROR default to FORMAT_OUT_BAYER12 \n",__func__, __LINE__);
		        f->fmt.pix.pixelformat = FORMAT_OUT_BAYER12;
			f->fmt.pix.sizeimage = f->fmt.pix.width * f->fmt.pix.height;
			break;
	}
	isp_dev->out_sizeimage = f->fmt.pix.sizeimage;
	isp_dev->out_w = f->fmt.pix.width;
	isp_dev->out_h = f->fmt.pix.height;
	isp_dev->out_fmt = f->fmt.pix.pixelformat;
	return 0;
}
  int isp_mimo_v4l2_m2m_ioctl_try_fmt_cap(struct file *file, void*priv,
					struct v4l2_format *f)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct vvcam_isp_dev *isp_dev = ctx->device->isp_dev;

	if(f->fmt.pix.width > MAX_WIDTH)
		f->fmt.pix.width = MAX_WIDTH;
	
	if(f->fmt.pix.height > MAX_HEIGHT)
		f->fmt.pix.height = MAX_HEIGHT;

	switch(f->fmt.pix.pixelformat) {
		case FORMAT_CAP_NV12:
	visp_pr_debug("===== [VISP_M2M] %s : %d FORMAT_CAP_NV12 \n",__func__, __LINE__);
			f->fmt.pix.sizeimage = f->fmt.pix.width * ((f->fmt.pix.height * 3) / 2);
		break;
		case FORMAT_CAP_RGB:
	visp_pr_debug("===== [VISP_M2M] %s : %d FORMAT_CAP_RGB \n",__func__, __LINE__);
			f->fmt.pix.sizeimage = f->fmt.pix.width * f->fmt.pix.height * 3;
		break;
		case FORMAT_CAP_NV16:
	visp_pr_debug("===== [VISP_M2M] %s : %d FORMAT_CAP_NV16 \n",__func__, __LINE__);
			f->fmt.pix.sizeimage = f->fmt.pix.width * f->fmt.pix.height * 2;
		break;
		default:
	visp_pr_err("===== [VISP_M2M] %s : %d !!!!! ERROR default to FORMAT_CAP_RGB\n",__func__, __LINE__);
			f->fmt.pix.pixelformat = FORMAT_CAP_RGB;
			f->fmt.pix.sizeimage = f->fmt.pix.width * f->fmt.pix.height * 3;
		break;
	}
	isp_dev->cap_sizeimage = f->fmt.pix.sizeimage;
	isp_dev->cap_w = f->fmt.pix.width;
	isp_dev->cap_h = f->fmt.pix.height;
	isp_dev->cap_fmt = f->fmt.pix.pixelformat;
	return 0;
}

  int isp_mimo_v4l2_m2m_ioctl_enum_fmt_cap(struct file *file, void *priv,
							struct v4l2_fmtdesc *f)
{
	visp_pr_debug("===== [VISP_M2M] %s :%d [0x%x] [0x%x]\n",__func__, __LINE__,file->private_data,priv);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct v4l2_format *fmt;
	
	fmt = isp_mimo_get_format(ctx, f->type);

	if(IS_ERR(fmt))
		return -EINVAL;

	switch(f->index){
	case 0:
		visp_pr_debug("===== [VISP_M2M] %s : %d FORMAT_CAP_NV12\n",__func__, __LINE__);
		f->pixelformat = FORMAT_CAP_NV12;
		break;
	case 1:
		visp_pr_debug("===== [VISP_M2M] %s : %d FORMAT_CAP_RGB\n",__func__, __LINE__);
		f->pixelformat = FORMAT_CAP_RGB;
		break;
	case 2:
		visp_pr_debug("===== [VISP_M2M] %s : %d FORMAT_CAP_NV16 \n",__func__, __LINE__);
		f->pixelformat = FORMAT_CAP_NV16;
		break;
	default:
		visp_pr_err("===== [VISP_M2M] %s : %d ERROR: default case \n",__func__, __LINE__);
		return -EINVAL;
	}
	return 0;	
}

  int isp_mimo_v4l2_m2m_ioctl_enum_fmt_out(struct file *file, void *priv,
						struct v4l2_fmtdesc *f)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d f->type = %d \n",__func__, __LINE__,f->type);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct v4l2_format *fmt;
	
	fmt = isp_mimo_get_format(ctx, f->type); //-- VISP_M2M check this f-type should be valid

	if(IS_ERR(fmt))
		return -EINVAL;

	switch(f->index){
	case 0:
		visp_pr_debug("===== [VISP_M2M] %s : %d  FORMAT_OUT_BAYER10 \n",__func__, __LINE__);
		f->pixelformat = FORMAT_OUT_BAYER10;
		break;
	case 1:
		visp_pr_debug("===== [VISP_M2M] %s : %d  FORMAT_OUT_BAYER12 \n",__func__, __LINE__);
		f->pixelformat = FORMAT_OUT_BAYER12;
		break;
	case 2:
		visp_pr_debug("===== [VISP_M2M] %s : %d  FORMAT_OUT_BAYER8 \n",__func__, __LINE__);
		f->pixelformat = FORMAT_OUT_BAYER8;
		break;
	default:
		visp_pr_debug("===== [VISP_M2M] %s : %d  ERROR: default case \n",__func__, __LINE__);
		return -EINVAL;
	}
	return 0;	
}



  int isp_mimo_v4l2_m2m_ioctl_g_fmt(struct file *file, void *priv,
						struct v4l2_format *f)
{
visp_pr_debug("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct v4l2_format *fmt;

	fmt = isp_mimo_get_format(ctx, f->type);

	if(IS_ERR(fmt))
		return -EINVAL;

	f->fmt.pix = fmt->fmt.pix;
	
	return 0;	
}

  int isp_mimo_v4l2_m2m_ioctl_s_fmt_out(struct file *file, void *priv,
			     			struct v4l2_format *f)
{
visp_pr_debug("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct v4l2_format *fmt;
	
	isp_mimo_v4l2_m2m_ioctl_try_fmt_out(file, priv, f);

	fmt = isp_mimo_get_format(ctx, f->type);

	if(IS_ERR(fmt))
		return -EINVAL;

	fmt->fmt.pix = f->fmt.pix;

	return 0;	
}
  int isp_mimo_v4l2_m2m_ioctl_s_fmt_cap(struct file *file, void *priv,
			     			struct v4l2_format *f)
{
visp_pr_debug("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) priv;
	struct v4l2_format *fmt;
	
	isp_mimo_v4l2_m2m_ioctl_try_fmt_cap(file, priv, f);

	fmt = isp_mimo_get_format(ctx, f->type);

	if(IS_ERR(fmt))
		return -EINVAL;

	fmt->fmt.pix = f->fmt.pix;

	return 0;	
}



  int isp_mimo_v4l2_m2m_ioctl_streamon(struct file *file, void *fh,
                           			enum v4l2_buf_type type)
{
visp_pr_debug("===== [VISP_M2M] %s : %d\n",  __func__, __LINE__);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *) fh;

        return v4l2_m2m_streamon(file, ctx->fh.m2m_ctx, type);
}

  int isp_mimo_v4l2_m2m_ioctl_streamoff(struct file *file, void *fh,
                            			enum v4l2_buf_type type)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d\n",  __func__, __LINE__);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;

        return v4l2_m2m_streamoff(file, ctx->fh.m2m_ctx, type);

}

  int isp_mimo_v4l2_m2m_ioctl_reqbufs(struct file *file, void *fh,
                            		struct v4l2_requestbuffers *rb)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d count [%d] type [%d]\n",  __func__, __LINE__,rb->count, rb->type);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
        return v4l2_m2m_ioctl_reqbufs(file, ctx->fh.m2m_ctx, rb);
}

  int isp_mimo_v4l2_m2m_ioctl_querybuf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d buf index [%d] type[%d] \n",  __func__, __LINE__,buf->index,buf->type);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
        return v4l2_m2m_ioctl_querybuf(file, ctx->fh.m2m_ctx, buf);
}

  int isp_mimo_v4l2_m2m_ioctl_qbuf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d\n",  __func__, __LINE__);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
        return v4l2_m2m_ioctl_qbuf(file, ctx->fh.m2m_ctx, buf);
}

  int isp_mimo_v4l2_m2m_ioctl_dqbuf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf)
{
	int status;
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
        status = v4l2_m2m_ioctl_dqbuf(file, ctx->fh.m2m_ctx, buf);
	visp_pr_info("[VISP_M2M] %s : %d buf index actual [%d] real [%d] for buf type [%d]\n",  __func__, __LINE__,
				buf->index, ctx->device->isp_dev->isp_dq_out_index,buf->type);
	return status;
}

  int isp_mimo_v4l2_m2m_ioctl_prepare_buf(struct file *file, void *fh,
                            			struct v4l2_buffer *buf)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d\n",  __func__, __LINE__);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
        return v4l2_m2m_ioctl_prepare_buf(file, ctx->fh.m2m_ctx, buf);
}

  int isp_mimo_v4l2_m2m_ioctl_create_bufs(struct file *file, void *fh,
                            		struct v4l2_create_buffers *create)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d index %d count %d\n",  __func__, __LINE__,create->index, create->count);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
        return v4l2_m2m_ioctl_create_bufs(file, ctx->fh.m2m_ctx, create);
}

  int isp_mimo_v4l2_m2m_ioctl_expbuf(struct file *file, void *fh,
                            		struct v4l2_exportbuffer *eb)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d\n",  __func__, __LINE__);
	struct isp_mimo_ctx *ctx = (struct isp_mimo_ctx *)fh;
        return v4l2_m2m_ioctl_expbuf(file, ctx->fh.m2m_ctx, eb);
}

  int isp_mimo_queue_setup(struct vb2_queue *vq,
			   unsigned int *nbuffers, unsigned int *nplanes,
			   unsigned int sizes[], struct device *alloc_devs[])
{
	visp_pr_debug("===== [VISP_M2M] %s : %d count=%d \n",__func__, __LINE__,*nbuffers);
	struct isp_mimo_ctx *ctx = vb2_get_drv_priv(vq);

	struct v4l2_format *fmt;

	fmt = isp_mimo_get_format(ctx, vq->type);
	if(IS_ERR(fmt))
		return -EINVAL;

	*nplanes = 1;	
	sizes[0] = fmt->fmt.pix.sizeimage;

	//visp_v4l2_dbg(1, debug, v4l2_dev, "get %d buffer(s) of size %d each.\n", count, sizes[0]);

	return 0;
}

  int isp_mimo_buf_prepare(struct vb2_buffer *vb)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
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
	visp_pr_debug("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct isp_mimo_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);

	v4l2_m2m_buf_queue(ctx->fh.m2m_ctx, vbuf);
}

  int isp_mimo_start_streaming(struct vb2_queue *q, unsigned int count)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d count=%d \n",__func__, __LINE__,count);
	struct isp_mimo_ctx *ctx = vb2_get_drv_priv(q);

	ctx->sequence = 0;
	return 0;
}

  void isp_mimo_stop_streaming(struct vb2_queue *q)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
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


  int queue_init(void *priv, struct vb2_queue *src_vq,
		      struct vb2_queue *dst_vq)
{
	visp_pr_debug("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
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

/*
 * File operations
 */
int isp_mimo_open(struct file *file)
{
	static int device_open_count=0;
	static char c=0;

	++device_open_count;
	visp_pr_debug("ENTER ===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
	visp_pr_debug("===== [VISP_M2M] Device opened by process: %s [PID: %d] total device_open_count [%d] \n", 
					current->comm, current->pid, device_open_count);

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
		visp_pr_debug("===== [VISP_M2M] %s : %d create a new CTX .. \n",__func__, __LINE__);
		ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
		if (!ctx) {
			rc = -ENOMEM;
			goto open_unlock;
		}

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


		if(!c) {
			IspDeviceCreateMIMO(device->isp_dev,0);
			visp_pr_debug("=====Done IspDeviceCreateMIMO [VISP_M2M] %s : %d\n",__func__, __LINE__);
		}else {
			visp_pr_debug("===== [VISP_M2M] %s : %d device already opened ..\n",__func__, __LINE__);
		}
		c = 1;
	} else {
		file->private_data =  &ctx->fh;
	}
	visp_pr_debug("=====open [VISP_M2M] %s : %d ctx 0x%x m2m_ctx 0x%x \n",__func__, __LINE__,ctx, ctx->fh.m2m_ctx);

open_unlock:
	mutex_unlock(&device->lock);
	visp_pr_debug("EXIT ===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
	return rc;
}

  int isp_mimo_release(struct file *file)
{
#if 0
	static int device_close_count=0;
	visp_pr_debug("=====ENTER [VISP_M2M] %s : %d total device_close_count [%d] \n",__func__, __LINE__, ++device_close_count);
#endif

	struct isp_mimo *device = video_drvdata(file);
	struct isp_mimo_ctx *ctx = file2ctx(file);

	visp_pr_debug("=====release [VISP_M2M] %s : %d ctx 0x%x m2m_ctx 0x%x \n",__func__, __LINE__,ctx, ctx->fh.m2m_ctx);

#if 1
	v4l2_fh_del(&ctx->fh);
	v4l2_fh_exit(&ctx->fh);
	mutex_lock(&device->lock);
	v4l2_m2m_ctx_release(ctx->fh.m2m_ctx);
	mutex_unlock(&device->lock);
	kfree(ctx);
	visp_pr_debug("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
#else
	v4l2_m2m_streamoff(file, ctx->fh.m2m_ctx, V4L2_BUF_TYPE_VIDEO_OUTPUT);
	visp_pr_err("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);

	v4l2_m2m_streamoff(file, ctx->fh.m2m_ctx, V4L2_BUF_TYPE_VIDEO_CAPTURE);
	visp_pr_err("===== [VISP_M2M] %s : %d\n",__func__, __LINE__);
#endif
	visp_pr_debug("=====EXIT [VISP_M2M] %s : %d\n",__func__, __LINE__);

	return 0;
}




int isp_mimo_probe(struct platform_device *pdev)
{
	static int probe_cnt = 0;
	if(probe_cnt >= MAX_SUPPORTED_DEVICE_COUNT){
		visp_pr_debug("[VISP_M2M] %s : %d Skipping unsupported probe cnt [%d] for pdev[0x%x]..\n",__func__, __LINE__, probe_cnt,pdev);
		return 0;
	}
	pdev->id = probe_cnt;
	visp_pr_debug("[VISP_M2M] %s : %d pdev[0x%x] num_res[%d] id[%d] id_auto[%d] .. \n",__func__, __LINE__,pdev, pdev->num_resources, pdev->id, pdev->id_auto);

	struct isp_mimo *device;
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct video_device *vfd;
	struct v4l2_device *v4l2_dev = &device->v4l2_dev;
	int ret = 0;

	device = devm_kzalloc(dev, sizeof(*device), GFP_KERNEL);
	if(!device)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	visp_pr_info("===== [VISP_M2M] %s : %d v4l2_dev = [0x%x] \n",__func__, __LINE__,&device->v4l2_dev);

	ret = v4l2_device_register(&pdev->dev, &device->v4l2_dev);
	if (ret) {
		dev_err(dev, "could not register video device rc=%d\n", ret);
		return ret;
	}
	
	device->video_dev = isp_mimo_video_dev;
	vfd = &device->video_dev;
	vfd->lock = &device->lock;
	vfd->v4l2_dev = &device->v4l2_dev;

	video_set_drvdata(vfd, device);
	platform_set_drvdata(pdev, device);

//	struct isp_mimo *device1 = platform_get_drvdata(pdev);
	
	snprintf(vfd->name, sizeof(vfd->name), "%s", MEM2MEM_NAME);

	device->m2m_dev = v4l2_m2m_init(&m2m_ops);
	if (IS_ERR(device->m2m_dev)) {
		v4l2_err(v4l2_dev, "Failed to init mem2mem device\n");
		ret = PTR_ERR(device->m2m_dev);
		goto err_v4l2;
	}

	ret = video_register_device(vfd, VFL_TYPE_VIDEO, 0);
        if (ret) {
		v4l2_err(v4l2_dev, "Failed to register video device\n");
		goto err_m2m;
	}
	
	device->isp_dev = devm_kzalloc(&pdev->dev, sizeof(struct vvcam_isp_dev), GFP_KERNEL);
	if (!device->isp_dev)
        	return -ENOMEM;

	mutex_init(&device->isp_dev->mlock);
	mutex_init(&device->isp_dev->ctrl_lock);
	device->isp_dev->dev = &pdev->dev;

	device->isp_dev->isp_rpu = 0;
	ret = xlnx_link_mbox(device->isp_dev);
	if (ret) {
        	dev_err(&pdev->dev, "failed to init mbox\n");
		return -EINVAL;
	}
	vvcam_isp_pads_init(device->isp_dev);

	probe_cnt +=1;

	visp_pr_info("===== [VISP_M2M] %s : %d done \n",__func__, __LINE__);

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


MODULE_DEVICE_TABLE(of, isp_mimo_dt_ids);
module_platform_driver(isp_mimo_driver);

MODULE_DESCRIPTION("AMD ISP MIMO driver");
MODULE_AUTHOR("AMD ISP SW Team");
MODULE_LICENSE("GPL");
