/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
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
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/
#include <linux/module.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_graph.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/arm-smccc.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/videobuf2-dma-contig.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-ctrls.h>
//#include <linux/sensor_cmd.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include "vvcam_v4l2_common.h"
#include "vvcam_isp_driver.h"
#include "vvcam_isp_event.h"
#include "vvcam_isp_ctrl.h"
#include "vvcam_isp_procfs.h"
#include <linux/interrupt.h>
#include<linux/delay.h>
#include "sensor_cmd.h"
//#include "mailbox.h"
#include "mbox_api.h"
#include "mbox_cmd.h"

#define MAX_ISP_INSTANCES 10
#ifdef VVCAM_PLATFORM_REGISTER
#define VVCAM_ISP_DEFAULT_SENSOR        "os08a20"
#define VVCAM_ISP_DEFAULT_SENSOR_MODE   0
#define VVCAM_ISP_DEFAULT_SENSOR_XML    "OS08a20_8M_02_1080p.xml"
#define VVCAM_ISP_DEFAULT_SENSOR_MANU_JSON    "vvbcfg/simulator/manual_ext.json"
#define VVCAM_ISP_DEFAULT_SENSOR_AUTO_JSON    "vvbcfg/simulator/auto.json"
#endif

int  __IspFd; 


struct class *mailbox_class;
static DEFINE_MUTEX(rpu_list_lock);
static LIST_HEAD(rpu_devices);




void apu_mailbox_read(/*void *CallbackRef*/ uint32_t);
void mailbox_init(uint32_t cpu,uint64_t MBOX_FIFO_START_ADDR,uint64_t MBOX_FIFO_START_ADDR_PHY);

static DECLARE_COMPLETION(mailbox_completion);
static DECLARE_COMPLETION(apu_wait_for_data);
static DECLARE_COMPLETION(apu_wait_for_ack);

// declare a global void pointer array and initialize it with NULL
 void *g_instance_array[MAX_ISP_INSTANCES] = { NULL };

#if 0
struct response_user_packet {
        /*Define your data fields*/
        u32 cmdid;
        void *data;
        payload_packet res_payload_pkt;
};
#endif    

extern struct response_user_packet data_from_interrupt;

static int  inuse = 0;
struct response_user_packet1 {
        /*Define your data fields*/
        u32 cmdid;
        void *data;
        payload_packet res_payload_pkt;
};
#if 1
typedef struct payload_user_template {

        Payload_type type;
        MBCmdId_E cmd_id ;
        __u32 cookie;
        __u32 payload_size;
        unsigned char payload[MAX_ITEM];

}payload_user_packet ;
#endif
struct reserved_memory {
    phys_addr_t phys_addr;
    void __iomem *virt_addr;
} reserved_memory;


struct mailbox_data {
    struct mbox_client mbox_client_tx;
    struct mbox_client mbox_client_rx;
    struct mbox_chan *tx_chan;
    struct mbox_chan *rx_chan;
    struct task_struct *thread;
};
struct mailbox_data *mbox_data;
struct class *mailbox_class;
/* Default IPI SMC function IDs */
#define SMC_IPI_MAILBOX_OPEN            0x82001000U
#define SMC_IPI_MAILBOX_RELEASE         0x82001001U
#define SMC_IPI_MAILBOX_STATUS_ENQUIRY  0x82001002U
#define SMC_IPI_MAILBOX_NOTIFY          0x82001003U
#define SMC_IPI_MAILBOX_ACK             0x82001004U
#define SMC_IPI_MAILBOX_ENABLE_IRQ      0x82001005U
#define SMC_IPI_MAILBOX_DISABLE_IRQ     0x82001006U
#define MAILBOX_TX_QUEUE_LEN 20
#define CHAR_DEV_NAME "mailbox_dev"
#define SUCCESS 0

#define TXDONE_BY_IRQ   BIT(0) /* controller has remote RTR irq */
#define TXDONE_BY_POLL  BIT(1) /* controller can read status of last TX */
#define TXDONE_BY_ACK   BIT(2) /* S/W ACK received by Client ticks the TX */
        

struct vvcam_isp_mbus_fmt vvcam_isp_mp_fmts[] = {
    {
        .code = MEDIA_BUS_FMT_YUYV8_2X8,   /*NV16*/
    },
    {
        .code = MEDIA_BUS_FMT_YUYV8_1_5X8, /*NV12*/
    },
    {
        .code = MEDIA_BUS_FMT_YUYV8_1X16,  /*YUYV*/
    }
};

struct vvcam_isp_mbus_fmt vvcam_isp_sp_fmts[] = {
    {
        .code = MEDIA_BUS_FMT_YUYV8_2X8,   /*NV16*/
    },
    {
        .code = MEDIA_BUS_FMT_YUYV8_1_5X8, /*NV12*/
    },
    {
        .code = MEDIA_BUS_FMT_YUYV8_1X16,  /*YUYV*/
    }
};

static int vvcam_isp_querycap(struct v4l2_subdev *sd, void *arg)
{
	struct v4l2_capability *cap = (struct v4l2_capability *)arg;

        strlcpy(cap->driver, sd->name, sizeof(cap->driver));
	strlcpy(cap->card, sd->name, sizeof(cap->card));
	snprintf(cap->bus_info, sizeof(cap->bus_info),
            "platform:%s", sd->name);

	return 0;
}

static int vvcam_isp_pad_requbufs(struct v4l2_subdev *sd, void *arg)
{
	struct vvcam_pad_reqbufs *pad_requbufs = (struct vvcam_pad_reqbufs *)arg;
        struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
	int status=0;

        int Port = pad_requbufs->pad / MEDIA_ISP_PORT_PAD_COUNT;
        int Chn  = (pad_requbufs->pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;
	
	isp_dev->IspPorts[Port].IspChns[Chn].NumBufs=pad_requbufs->num_buffers;
	return status;
}

int MediaIspQBuf(struct vvcam_isp_dev *isp_dev, int Pad_index , MediaBuf *Buf);
static int vvcam_isp_pad_buf_queue(struct v4l2_subdev *sd, void *arg)
{
        struct vvcam_pad_buf *pad_buf = (struct vvcam_pad_buf *)arg;
        struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
        int ret=0;
        unsigned long flags;
        struct vvcam_isp_pad_data *cur_pad;

        cur_pad = &isp_dev->pad_data[pad_buf->pad];

    	spin_lock_irqsave(&cur_pad->qlock, flags);

    	list_add_tail(&pad_buf->buf->list, &cur_pad->queue);

	spin_unlock_irqrestore(&cur_pad->qlock, flags);

	MediaBuf Buf;
   	Buf.Index = pad_buf->buf->sequence;
   	Buf.NumPlanes = pad_buf->buf->num_planes;
	int i=0;

	for (i = 0; i < Buf.NumPlanes; i++) {
    	Buf.Planes[i].DmaAddr = pad_buf->buf->planes[i].dma_addr;
        Buf.Planes[i].DmaSize = pad_buf->buf->planes[i].size;
    }   	

	  MediaIspQBuf(isp_dev,pad_buf->pad ,  &Buf);
	 return ret;
}

typedef struct thread_data
{
	struct v4l2_subdev *sd;
	int Port;
	int Chn;
        int Pad;
}thread_data_t;

struct task_struct *task;

extern volatile int DQ_BUF_AVAILABLE;

int MediaIspHalBufDone(struct v4l2_subdev *sd, int pad, MediaBuf *Buf);

#if 0
int MediaIspDeviceDqbuf(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn, MediaBuf *Buf);

int bufdone_thread(void *data) {
        thread_data_t * tdata=data;

        struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(tdata->sd);

        MediaBuf *Buf=kmalloc(sizeof(MediaBuf),GFP_KERNEL);
        if(!Buf)
    		{
		return -ENOMEM;
	    } 

	int RetVal=0;

	while  (!kthread_should_stop()  ) {
		RetVal = MediaIspDeviceDqbuf(isp_dev, tdata->Port, tdata->Chn, Buf);
		if (RetVal != VSI_SUCCESS) {
		    msleep(200);
		    continue;
		}
		MediaIspHalBufDone(tdata->sd, /*pad->Index*/tdata->Pad, Buf);
	//      msleep(300);
		DQ_BUF_AVAILABLE=0;
			 
	}
        kfree(Buf);                            
        kfree(data);
        return 0;
}
#endif

thread_data_t * tdata_g= NULL;
#if 1
int MediaIspDeviceDqbuf(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn, MediaBuf *Buf,void *Enque_Buff_G , MediaBuffer_t **Temp_DQ_BUF);
int Handle_Frameout_Buffer(void * Packet_from_RPU) {

        struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(tdata_g->sd);
        MediaBuf *Buf=kmalloc(sizeof(MediaBuf),GFP_KERNEL);
        if(!Buf)
    		{
		return -ENOMEM;
	    } 
	int RetVal=0;
	
    	MediaBuffer_t *Temp_DQ_BUF = VSI_NULL;

	RetVal = MediaIspDeviceDqbuf(isp_dev, tdata_g->Port, tdata_g->Chn, Buf,Packet_from_RPU,&Temp_DQ_BUF);
	if (RetVal != VSI_SUCCESS) {
	    	msleep(200);
	}

	MediaIspHalBufDone(tdata_g->sd, /*pad->Index*/tdata_g->Pad, Buf);
			 
/*	 
    int Port = tdata_g->Pad / MEDIA_ISP_PORT_PAD_COUNT;
    int Chn  = (tdata_g->Pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
 
    RetVal = VsiCamDeviceEnQueBuffer(isp_dev, IspPort->CamDeviceHandle, tdata_g->Chn, Temp_DQ_BUF);
    if (RetVal != VSI_SUCCESS) {
        RetVal = VSI_ERR_TIMEOUT;
        return RetVal;
    }    
*/
        kfree(Buf);                            
        return 0;
}

#endif


//
int MediaIspDeviceCameraConnect(struct vvcam_isp_dev *isp_dev , uint8_t Index);
int MediaIspDeviceSetFormat(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspDeviceStreamOn(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspStreamOff(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspStreamOn(struct vvcam_isp_dev *isp_dev , int Index /*  MediaPadAttr *Pad*/);

static int vvcam_isp_pad_s_stream(struct v4l2_subdev *sd, void *arg)
{
	struct vvcam_pad_stream_status *pad_stream = (struct vvcam_pad_stream_status *)arg;
    	struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    	int ret;
	int Port = pad_stream->pad / MEDIA_ISP_PORT_PAD_COUNT;
	int Chn  = (pad_stream->pad % MEDIA_ISP_PORT_PAD_COUNT) - 1;

    	isp_dev->pad_data[pad_stream->pad].stream = pad_stream->status;

	if (pad_stream->status == 0 ) {
		INIT_LIST_HEAD(&isp_dev->pad_data[pad_stream->pad].queue);
	}

	if(pad_stream->status==1) //streamon
	{

		if (isp_dev->IspPorts[Port].CameraConnectRefCnt == 0) { 

        		isp_dev->IspPorts[Port].CameraConnectRefCnt++;

			ret = vvcam_isp_l_calib_event(isp_dev, pad_stream->pad);

			MediaIspDeviceCameraConnect(isp_dev, pad_stream->pad );

			ret = vvcam_isp_l_json_event(isp_dev, pad_stream->pad);

    		}             
		else
		{
        		isp_dev->IspPorts[Port].CameraConnectRefCnt++;
		}


		MediaIspDeviceSetFormat(isp_dev, Port,  Chn);

		MediaIspDeviceStreamOn(isp_dev, Port, Chn);

		thread_data_t *thread_data_s=kmalloc(sizeof(thread_data_t),GFP_KERNEL) ;

    		if(!thread_data_s)
    		{
			return -ENOMEM;
    		} 
		thread_data_s->sd=sd;
		thread_data_s->Port=Port;//0;
		thread_data_s->Chn=Chn;//0;
		thread_data_s->Pad=pad_stream->pad;//0;
		tdata_g=thread_data_s;
	}

	else //streamoff
	{

		if(task) // stop the DQ thread
			{
			kthread_stop(task);
			task=NULL;		
		}
     		MediaIspStreamOff(isp_dev, Port, Chn);
	}

    return ret;
}


#if 1
/*static*/ int vvcam_isp_buf_done(struct v4l2_subdev *sd, void *arg)
{
    struct vvcam_isp_buf ubuf;
    struct vvcam_isp_pad_data *cur_pad;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    unsigned long flags;
    struct vvcam_vb2_buffer *pos, *next;
    struct vvcam_vb2_buffer *buf = NULL;
    struct media_pad *pad;
    struct v4l2_subdev *subdev;
    struct video_device *video;
    struct vvcam_pad_buf pad_buf;
    int ret;

    memcpy(&ubuf, arg, sizeof(struct vvcam_isp_buf));

    cur_pad = &isp_dev->pad_data[ubuf.pad];


    /*if (list_empty(&cur_pad->queue) || (cur_pad->stream == 0))
        return -EINVAL;*/
    if (list_empty(&cur_pad->queue) )
	{
		printk(KERN_ERR "EMPTY LISt\n");
       		return -EINVAL;
	}
    if ( (cur_pad->stream == 0))
	{
        	return -EINVAL;
	}

    spin_lock_irqsave(&cur_pad->qlock, flags);
    list_for_each_entry_safe(pos, next, &cur_pad->queue, list) {
        if (pos && (pos->sequence == ubuf.index)) {
           buf = pos;
           list_del(&pos->list);
           break;
        }
    }
    spin_unlock_irqrestore(&cur_pad->qlock, flags);

    if (buf) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&isp_dev->pads[ubuf.pad]);
#else
        pad = media_entity_remote_pad(&isp_dev->pads[ubuf.pad]);
#endif
        if (!pad)
            return -EINVAL;
        if (is_media_entity_v4l2_subdev(pad->entity)) {

            subdev = media_entity_to_v4l2_subdev(pad->entity);
            pad_buf.pad = pad->index;
            pad_buf.buf = buf;
            ret = v4l2_subdev_call(subdev, core, ioctl, VVCAM_PAD_BUF_DONE, &pad_buf);
            if (ret)
                return ret;

        } else if (is_media_entity_v4l2_video_device(pad->entity)){
            video = media_entity_to_video_device(pad->entity);
            if (buf->sequence < video->queue->num_buffers) {
                if (buf->vb.vb2_buf.state == VB2_BUF_STATE_ACTIVE) {
                    vb2_buffer_done(&buf->vb.vb2_buf, VB2_BUF_STATE_DONE);
                }
            }
        }
    }

    return 0;
}
#endif

static int vvcam_isp_queryctrl(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_queryctrl *pad_querctrl =
                                    (struct vvcam_pad_queryctrl *)arg;
    ret = v4l2_queryctrl(&isp_dev->ctrl_handler, pad_querctrl->query_ctrl);

    return ret;
}

static int vvcam_isp_query_ext_ctrl(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_query_ext_ctrl *pad_quer_ext_ctrl =
                                    (struct vvcam_pad_query_ext_ctrl *)arg;
    ret = v4l2_query_ext_ctrl(&isp_dev->ctrl_handler,
                        pad_quer_ext_ctrl->query_ext_ctrl);

    return ret;
}

static int vvcam_isp_querymenu(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_querymenu *pad_quermenu =
                                    (struct vvcam_pad_querymenu *)arg;
    ret = v4l2_querymenu(&isp_dev->ctrl_handler,
                        pad_quermenu->querymenu);

    return ret;
}

static int vvcam_isp_g_ctrl(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_control *pad_ctrl = (struct vvcam_pad_control *)arg;

    mutex_lock(&isp_dev->ctrl_lock);
    isp_dev->ctrl_pad = pad_ctrl->pad;
    ret = v4l2_g_ctrl(&isp_dev->ctrl_handler, pad_ctrl->control);
    mutex_unlock(&isp_dev->ctrl_lock);

    return ret;
}

static int vvcam_isp_s_ctrl(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_control *pad_ctrl = (struct vvcam_pad_control *)arg;

    mutex_lock(&isp_dev->ctrl_lock);
    isp_dev->ctrl_pad = pad_ctrl->pad;
    ret = v4l2_s_ctrl(NULL, &isp_dev->ctrl_handler, pad_ctrl->control);
    mutex_unlock(&isp_dev->ctrl_lock);

    return ret;
}

static int vvcam_isp_g_ext_ctrls(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_ext_controls *pad_ext_ctrls =
                            (struct vvcam_pad_ext_controls *)arg;

    mutex_lock(&isp_dev->ctrl_lock);
    isp_dev->ctrl_pad = pad_ext_ctrls->pad;
    ret = v4l2_g_ext_ctrls(&isp_dev->ctrl_handler, sd->devnode,
                            sd->v4l2_dev->mdev,
                            pad_ext_ctrls->ext_controls);
    mutex_unlock(&isp_dev->ctrl_lock);

    return ret;
}

static int vvcam_isp_s_ext_ctrls(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_ext_controls *pad_ext_ctrls =
                            (struct vvcam_pad_ext_controls *)arg;

    mutex_lock(&isp_dev->ctrl_lock);
    isp_dev->ctrl_pad = pad_ext_ctrls->pad;
    ret = v4l2_s_ext_ctrls(NULL, &isp_dev->ctrl_handler, sd->devnode,
                            sd->v4l2_dev->mdev,
                            pad_ext_ctrls->ext_controls);
    mutex_unlock(&isp_dev->ctrl_lock);

    return ret;
}

static int vvcam_isp_try_ext_ctrls(struct v4l2_subdev *sd,void *arg)
{
    int ret;
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_pad_ext_controls *pad_ext_ctrls =
                            (struct vvcam_pad_ext_controls *)arg;
    ret = v4l2_try_ext_ctrls(&isp_dev->ctrl_handler, sd->devnode,
                            sd->v4l2_dev->mdev,
                            pad_ext_ctrls->ext_controls);

    return ret;
}

static long vvcam_isp_priv_ioctl(struct v4l2_subdev *sd,
                                unsigned int cmd, void *arg)
{
    int ret = -EINVAL;
    switch (cmd) {
        case VIDIOC_QUERYCAP:
            ret = vvcam_isp_querycap(sd, arg);
            break;
        case VVCAM_PAD_REQUBUFS:
            ret = vvcam_isp_pad_requbufs(sd, arg);
            break;
        case VVCAM_PAD_BUF_QUEUE:
            ret = vvcam_isp_pad_buf_queue(sd, arg);
            break;
        case VVCAM_PAD_S_STREAM:
            ret = vvcam_isp_pad_s_stream(sd, arg);
            break;
        case VVCAM_ISP_IOC_BUFDONE:
            ret = vvcam_isp_buf_done(sd, arg);
            break;
        case VVCAM_PAD_QUERYCTRL:
            ret = vvcam_isp_queryctrl(sd, arg);
            break;
        case VVCAM_PAD_QUERY_EXT_CTRL:
            ret = vvcam_isp_query_ext_ctrl(sd, arg);
            break;
        case VVCAM_PAD_G_CTRL:
            ret = vvcam_isp_g_ctrl(sd, arg);
            break;
        case VVCAM_PAD_S_CTRL:
            ret = vvcam_isp_s_ctrl(sd, arg);
            break;
        case VVCAM_PAD_G_EXT_CTRLS:
            ret = vvcam_isp_g_ext_ctrls(sd, arg);
            break;
        case VVCAM_PAD_S_EXT_CTRLS:
            ret = vvcam_isp_s_ext_ctrls(sd, arg);
            break;
        case VVCAM_PAD_TRY_EXT_CTRLS:
            ret = vvcam_isp_try_ext_ctrls(sd, arg);
            break;
        case VVCAM_PAD_QUERYMENU:
            ret = vvcam_isp_querymenu(sd, arg);
            break;
        default:
            break;
    }

    return ret;
}

int vvcam_isp_subscribe_event(struct v4l2_subdev *sd,
			                struct v4l2_fh *fh,
			                struct v4l2_event_subscription *sub)
{
    switch (sub->type) {
        case V4L2_EVENT_CTRL:
            return v4l2_ctrl_subdev_subscribe_event(sd, fh, sub);
        case VVCAM_ISP_DEAMON_EVENT:
            return v4l2_event_subscribe(fh, sub, 2, NULL);
        default:
            return -EINVAL;
    }

}

static struct v4l2_subdev_core_ops vvcam_isp_core_ops = {
	.ioctl             = vvcam_isp_priv_ioctl,
	.subscribe_event   = vvcam_isp_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static struct v4l2_subdev_video_ops vvcam_isp_video_ops = {
	/*.s_stream = vvcam_isp_s_stream,*/
};


int MediaIspHalMbusFmtToMediaFmt(uint32_t *Code, uint32_t *PixelFormat);
int MediaIspSetFormat( struct vvcam_isp_dev *isp_dev,uint32_t pad_index,/*  MediaPadAttr *Pad,*/ MediaFmt *Format);
static int vvcam_isp_set_fmt(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
			struct v4l2_subdev_format *format)
{
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    uint32_t w, h;
    uint32_t sink_pad_index;
    struct vvcam_isp_pad_data *cur_pad = &isp_dev->pad_data[format->pad];
    struct vvcam_isp_pad_data *sink_pad;
    struct vvcam_isp_pad_data *source_pad;
    int i;
    int ret;
    sink_pad_index = format->pad - (format->pad % VVCAM_ISP_PORT_PAD_NR);
    sink_pad = &isp_dev->pad_data[sink_pad_index];

    if (sink_pad == cur_pad) {
        cur_pad->sink_detected = 1;
        cur_pad->format = format->format;
        for (i = 1; i < VVCAM_ISP_PORT_PAD_NR; i++) {
            source_pad = &isp_dev->pad_data[sink_pad_index + i];
            source_pad->sink_detected = 1;

            switch (i) {
                case VVCAM_ISP_PORT_PAD_SOURCE_MP:
                case VVCAM_ISP_PORT_PAD_SOURCE_SP1:
                case VVCAM_ISP_PORT_PAD_SOURCE_SP2:
                    source_pad->format = format->format;
                    source_pad->format.code = source_pad->mbus_fmt[0].code;
                    source_pad->format.field = V4L2_FIELD_NONE;
                    source_pad->format.quantization = V4L2_QUANTIZATION_DEFAULT;
                    source_pad->format.colorspace = V4L2_COLORSPACE_DEFAULT;
                    break;
                case VVCAM_ISP_PORT_PAD_SOURCE_RAW:
                    source_pad->format = format->format;
                    source_pad->mbus_fmt[0].code = format->format.code;
                    break;
                default:
                    break;
            }
        }
        return 0;
    }

    w = ALIGN(format->format.width, VVCAM_ISP_WIDTH_ALIGN);
    h = ALIGN(format->format.height, VVCAM_ISP_HEIGHT_ALIGN);
    w = clamp_t(uint32_t, w, VVCAM_ISP_WIDTH_MIN, sink_pad->format.width);
    h = clamp_t(uint32_t, h, VVCAM_ISP_HEIGHT_MIN, sink_pad->format.height);
	if(h==0)
	{
		h=1080;
	}
    format->format.width = w;
    format->format.height = h;


    for (i = 0; i < cur_pad->num_formats; i++) {
        if (format->format.code == cur_pad->mbus_fmt[i].code)
            break;
    }

    if (i >= cur_pad->num_formats) {
        format->format.code = cur_pad->mbus_fmt[0].code;
    }

	MediaFmt Format_media;

    memset(&Format_media, 0, sizeof(Format_media));
	struct v4l2_mbus_framefmt *MBusFormat;

    MBusFormat = (struct v4l2_mbus_framefmt *)&format->format;
    Format_media.Width = MBusFormat->width;
    Format_media.Height = MBusFormat->height;
    Format_media.ColorSpace = MBusFormat->colorspace;
    Format_media.Quantization = MBusFormat->quantization;
    MediaIspHalMbusFmtToMediaFmt(&MBusFormat->code, &Format_media.PixelFormat);
	
    struct media_pad *mediapad_t = &isp_dev->pads[format->pad];

    MediaIspSetFormat(isp_dev,mediapad_t->index, &Format_media);


    if (ret)
        return ret;

    cur_pad->format = format->format;

    return 0;
}
#if 1
int vvcam_isp_set_fmt_public(/*struct v4l2_subdev *sd,	struct v4l2_subdev_state *sd_state,*/struct vvcam_isp_dev *isp_dev,struct v4l2_subdev_format *format)
{
    //RANJITH This function is not required atleast the 2nd part of it check and delete
    // return  vvcam_isp_set_fmt(sd,sd_state, format);
    //    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    uint32_t w, h;
    uint32_t sink_pad_index;
    struct vvcam_isp_pad_data *cur_pad = &isp_dev->pad_data[format->pad];
    struct vvcam_isp_pad_data *sink_pad;
    struct vvcam_isp_pad_data *source_pad;
    int i;
    int ret;
    sink_pad_index = format->pad - (format->pad % VVCAM_ISP_PORT_PAD_NR);
    sink_pad = &isp_dev->pad_data[sink_pad_index];

    if (sink_pad == cur_pad) {
        cur_pad->sink_detected = 1;
        cur_pad->format = format->format;
        for (i = 1; i < VVCAM_ISP_PORT_PAD_NR; i++) {
            source_pad = &isp_dev->pad_data[sink_pad_index + i];
            source_pad->sink_detected = 1;

            switch (i) {
                case VVCAM_ISP_PORT_PAD_SOURCE_MP:
                case VVCAM_ISP_PORT_PAD_SOURCE_SP1:
                case VVCAM_ISP_PORT_PAD_SOURCE_SP2:
                    source_pad->format = format->format;
                    source_pad->format.code = source_pad->mbus_fmt[0].code;
                    source_pad->format.field = V4L2_FIELD_NONE;
                    source_pad->format.quantization = V4L2_QUANTIZATION_DEFAULT;
                    source_pad->format.colorspace = V4L2_COLORSPACE_DEFAULT;
                    break;
                case VVCAM_ISP_PORT_PAD_SOURCE_RAW:
                    source_pad->format = format->format;
                    source_pad->mbus_fmt[0].code = format->format.code;
                    break;
                default:
                    break;
            }
        }
        return 0;
    }

    w = ALIGN(format->format.width, VVCAM_ISP_WIDTH_ALIGN);
    h = ALIGN(format->format.height, VVCAM_ISP_HEIGHT_ALIGN);
    w = clamp_t(uint32_t, w, VVCAM_ISP_WIDTH_MIN, sink_pad->format.width);
    h = clamp_t(uint32_t, h, VVCAM_ISP_HEIGHT_MIN, sink_pad->format.height);

    format->format.width = w;
    format->format.height = h;

    for (i = 0; i < cur_pad->num_formats; i++) {
        if (format->format.code == cur_pad->mbus_fmt[i].code)
            break;
    }

    if (i >= cur_pad->num_formats) {
        format->format.code = cur_pad->mbus_fmt[0].code;
    }

    if (ret)
        return ret;
    MediaFmt Format_media;

    memset(&Format_media, 0, sizeof(Format_media));
    struct v4l2_mbus_framefmt *MBusFormat;

    MBusFormat = (struct v4l2_mbus_framefmt *)&format->format;
    Format_media.Width = MBusFormat->width;
                Format_media.Height = MBusFormat->height;
                Format_media.ColorSpace = MBusFormat->colorspace;
                Format_media.Quantization = MBusFormat->quantization;
                MediaIspHalMbusFmtToMediaFmt(&MBusFormat->code, &Format_media.PixelFormat);


    struct media_pad *mediapad_t = &isp_dev->pads[format->pad];
    MediaIspSetFormat(isp_dev,mediapad_t->index, &Format_media);
    if (ret)
        return ret;

    cur_pad->format = format->format;

    return 0;

}
#endif
//
static int vvcam_isp_get_fmt(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
			struct v4l2_subdev_format *format)
{
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_isp_pad_data *pad_data = &isp_dev->pad_data[format->pad];

    if (pad_data->sink_detected) {
        format->format = pad_data->format;
    } else {
        return -EINVAL;
    }

    return 0;
}

int IspDeviceCreate(struct vvcam_isp_dev *isp_dev , uint8_t Port);
static int vvcam_isp_enum_mbus_code(struct v4l2_subdev *sd,
			struct v4l2_subdev_state *sd_state,
            struct v4l2_subdev_mbus_code_enum *code)
{

    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_isp_pad_data *pad_data = &isp_dev->pad_data[code->pad];
    if (code->index >= pad_data->num_formats) {
        return -EINVAL;
    }

    code->code = pad_data->mbus_fmt[code->index].code;

   /*Create Instance*/
    int Index = isp_dev->pads[code->pad].index;
    int Port = Index / MEDIA_ISP_PORT_PAD_COUNT;
    //int Chn = (Index % MEDIA_ISP_PORT_PAD_COUNT) - 1;

	// camdevice_create;
	
	if (!isp_dev->IspPorts[Port].RefCount) {
   		//int RetVal = VSI_SUCCESS;
		MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
		//CamDeviceConfig_t* CamConfig; 
		if (IspPort->CamDeviceHandle) {
	 		return VSI_SUCCESS; 
 		} 
		//TO-DO Need to implement for supportung self path
		if (!isp_dev->IspPorts[Port].RefCount)
	 	{		
			IspDeviceCreate(isp_dev, Port);
		}
	}
    isp_dev->IspPorts[Port].RefCount++;

    return 0;
}

static const struct v4l2_subdev_pad_ops vvcam_isp_pad_ops = {
	.set_fmt        = vvcam_isp_set_fmt,
    .get_fmt        = vvcam_isp_get_fmt,
    .enum_mbus_code = vvcam_isp_enum_mbus_code,
};


struct v4l2_subdev_ops vvcam_isp_subdev_ops = {
	.core  = &vvcam_isp_core_ops,
	.video = &vvcam_isp_video_ops,
	.pad   = &vvcam_isp_pad_ops,
};

static int vvcam_isp_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&isp_dev->mlock);

	isp_dev->refcnt++;
	pm_runtime_get_sync(sd->dev);

	mutex_unlock(&isp_dev->mlock);
	return 0;
}

static int vvcam_isp_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&isp_dev->mlock);

	isp_dev->refcnt--;
	pm_runtime_put_sync(sd->dev);

	mutex_unlock(&isp_dev->mlock);

	return 0;
}


static struct v4l2_subdev_internal_ops vvcam_isp_internal_ops = {
	.open  = vvcam_isp_open,
	.close = vvcam_isp_close,
};

static int vvcam_isp_link_setup(struct media_entity *entity,
		const struct media_pad *local,
		const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations vvcam_isp_entity_ops = {
	.link_setup     = vvcam_isp_link_setup,
	.link_validate  = v4l2_subdev_link_validate,
	.get_fwnode_pad = v4l2_subdev_get_fwnode_pad_1_to_1,

};

static int vvcam_isp_notifier_bound(struct v4l2_async_notifier *notifier,
		                            struct v4l2_subdev *sd,
		                            struct v4l2_async_connection * asc)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev = container_of(notifier,
			struct vvcam_isp_dev, notifier);
    struct device *dev =  isp_dev->dev;

    struct fwnode_handle *ep = NULL;
	struct v4l2_fwnode_link link;
	struct media_entity *source, *sink;
	unsigned int source_pad, sink_pad;

    while(1) {
        ep = fwnode_graph_get_next_endpoint(sd->fwnode, ep);
        if (!ep)
			break;

        ret = v4l2_fwnode_parse_link(ep, &link);
		if (ret < 0) {
			dev_err(dev, "failed to parse link for %pOF: %d\n",
                    to_of_node(ep), ret);
			continue;
		}

        if (sd->entity.pads[link.local_port].flags == MEDIA_PAD_FL_SINK)
			continue;

        source     = &sd->entity;
		source_pad = link.remote_port;
		sink       = &isp_dev->sd.entity;
		sink_pad   = link.local_port;
		v4l2_fwnode_put_link(&link);
		ret = media_create_pad_link(source, source_pad,
				sink, sink_pad, MEDIA_LNK_FL_ENABLED);
        if (ret) {
			dev_err(dev, "failed to create %s:%u -> %s:%u link\n",
				source->name, source_pad,
				sink->name, sink_pad);
			break;
		}

    }

    fwnode_handle_put(ep);

	return ret;
}

static void vvcam_isp_notifier_unbound(struct v4l2_async_notifier *notifier,
		                            struct v4l2_subdev *sd,
		                            struct v4l2_async_connection *asd)
{
	return;
}

static const struct v4l2_async_notifier_operations vvcam_isp_notify_ops = {
	.bound    = vvcam_isp_notifier_bound,
	.unbind   = vvcam_isp_notifier_unbound,
};

static int vvcam_isp_async_notifier(struct vvcam_isp_dev *isp_dev)
{
  //  struct device *dev = isp_dev->dev;
    int ret = 0;
    //int pad = 0;
#if 0
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
//    v4l2_async_nf_init(&isp_dev->notifier);
#else
//    v4l2_async_notifier_init(&isp_dev->notifier);
#endif

    isp_dev->notifier.ops = &vvcam_isp_notify_ops;

    if (dev_fwnode(isp_dev->dev) == NULL)
        return 0;

    for (pad = 0; pad < VVCAM_ISP_PAD_NR; pad++) {

        if (isp_dev->pads[pad].flags != MEDIA_PAD_FL_SINK)
            continue;

        ep = fwnode_graph_get_endpoint_by_id(dev_fwnode(dev),
                                        pad, 0, FWNODE_GRAPH_ENDPOINT_NEXT);
        if (!ep)
            continue;
        remote_ep = fwnode_graph_get_remote_endpoint(ep);
        if (!remote_ep) {
            fwnode_handle_put(ep);
            continue;
        }
        fwnode_handle_put(remote_ep);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
        asd = v4l2_async_nf_add_fwnode_remote(&isp_dev->notifier,
                                            ep, struct v4l2_async_subdev);
#else
        asd = v4l2_async_notifier_add_fwnode_remote_subdev(&isp_dev->notifier,
                                                ep, struct v4l2_async_subdev);
#endif

        fwnode_handle_put(ep);

        if (IS_ERR(asd)) {
            ret = PTR_ERR(asd);
			if (ret != -EEXIST) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
                v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
				v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
				return ret;
			}
        }
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
        ret = v4l2_async_subdev_nf_register(&isp_dev->sd,
						  &isp_dev->notifier);
#else
        ret = v4l2_async_subdev_notifier_register(&isp_dev->sd,
						  &isp_dev->notifier);
#endif
    if (ret) {
        dev_err(isp_dev->dev, "Async notifier register error\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
        v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
		v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
    }
#endif
    return ret;
}

static int vvcam_isp_pads_init(struct vvcam_isp_dev *isp_dev)
{
    int pad = 0;
    for (pad = 0; pad < VVCAM_ISP_PAD_NR; pad++) {
        if ((pad % VVCAM_ISP_PORT_PAD_NR) == VVCAM_ISP_PORT_PAD_SINK) {
            isp_dev->pads[pad].flags = MEDIA_PAD_FL_SINK;
        } else {
            isp_dev->pads[pad].flags = MEDIA_PAD_FL_SOURCE;
        }

        switch (pad % VVCAM_ISP_PORT_PAD_NR) {
            case VVCAM_ISP_PORT_PAD_SINK:
                break;
            case VVCAM_ISP_PORT_PAD_SOURCE_MP:
                isp_dev->pad_data[pad].num_formats = ARRAY_SIZE(vvcam_isp_mp_fmts);
                isp_dev->pad_data[pad].mbus_fmt = vvcam_isp_mp_fmts;
                break;
            case VVCAM_ISP_PORT_PAD_SOURCE_SP1:
                isp_dev->pad_data[pad].num_formats = ARRAY_SIZE(vvcam_isp_sp_fmts);
                isp_dev->pad_data[pad].mbus_fmt = vvcam_isp_sp_fmts;
                break;
            case VVCAM_ISP_PORT_PAD_SOURCE_SP2:
                isp_dev->pad_data[pad].num_formats = ARRAY_SIZE(vvcam_isp_sp_fmts);
                isp_dev->pad_data[pad].mbus_fmt = vvcam_isp_sp_fmts;
                break;
            case VVCAM_ISP_PORT_PAD_SOURCE_RAW:
                isp_dev->pad_data[pad].num_formats = 1;
                isp_dev->pad_data[pad].mbus_fmt = devm_kzalloc(isp_dev->dev,
		                sizeof(struct vvcam_isp_mbus_fmt), GFP_KERNEL);;
                break;
            default:
                break;
        }

        INIT_LIST_HEAD(&isp_dev->pad_data[pad].queue);
        spin_lock_init(&isp_dev->pad_data[pad].qlock);
    }

    return 0;
}

// Function to parse the port information from the device tree
static int vvcam_isp_parse_ports(struct vvcam_isp_dev *isp_dev, struct device_node *node) {
    struct device_node *port_node;
    int port_idx = 0;

    for_each_child_of_node(node, port_node) {
        if (port_idx >= MAX_PORTS)  // Limiting to the number of defined ports
            break;

        of_property_read_u32(port_node, "xlnx,CC_TILE0_ISP0_IBA0_vcid", &isp_dev->port_info[port_idx].vcid);
        of_property_read_u32(port_node, "xlnx,CC_TILE0_ISP0_IBA0_data_format", &isp_dev->port_info[port_idx].data_format);
        of_property_read_u32(port_node, "xlnx,CC_TILE0_ISP0_IBA0_res_ver", &isp_dev->port_info[port_idx].res_ver);
        of_property_read_u32(port_node, "xlnx,CC_TILE0_ISP0_IBA0_res_hor", &isp_dev->port_info[port_idx].res_hor);
        of_property_read_u32(port_node, "xlnx,CC_TILE0_ISP0_IBA0_fps", &isp_dev->port_info[port_idx].fps);
        of_property_read_u32(port_node, "xlnx,CC_TILE0_ISP0_IBA0_mode", &isp_dev->port_info[port_idx].mode);

        dev_info(isp_dev->dev, "Port %d: VCID=%u, Format=%u, Res=%ux%u, FPS=%u, Mode=%u\n",
                 port_idx,
                 isp_dev->port_info[port_idx].vcid,
                 isp_dev->port_info[port_idx].data_format,
                 isp_dev->port_info[port_idx].res_hor,
                 isp_dev->port_info[port_idx].res_ver,
                 isp_dev->port_info[port_idx].fps,
                 isp_dev->port_info[port_idx].mode);

        port_idx++;
    }

    return 0;
}

static int vvcam_isp_parse_params(struct vvcam_isp_dev *isp_dev,
                        struct platform_device *pdev)
{
    fwnode_property_read_u32(of_fwnode_handle(pdev->dev.of_node),
			"id", &isp_dev->id);
    struct device_node *node = pdev->dev.of_node;
    int ret;
 
    if (!node) {
        dev_err(&pdev->dev, "No device tree node found\n");
        return -EINVAL;
    }
    // Read the mode property
    ret = of_property_read_u32(node, "xlnx,tile0-ISP0-mode", &isp_dev->isp_mode);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read xlnx,tile0-ISP0-mode property\n");
        return ret;
    } else {
		isp_dev->sensor_info[0].mode = isp_dev->isp_mode;
    }
	// Read the sensor-name property
    ret = of_property_read_string(node, "sensor-name", &isp_dev->sensor_name);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read sensor-name property\n");
        return ret;
    } else {
        // Safely copy the sensor name into the sensor_info structure
        strncpy(isp_dev->sensor_info[0].sensor, isp_dev->sensor_name, sizeof(isp_dev->sensor_info[0].sensor) - 1);
        isp_dev->sensor_info[0].sensor[sizeof(isp_dev->sensor_info[0].sensor) - 1] = '\0'; // Ensure null termination
    }
	
	// Read string property for SS-MODE-I0 (LIMO, etc.)
    ret = of_property_read_string(node, "xlnx,tile0-SS-MODE-I0", &isp_dev->ss_mode_i0);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read tile0-SS-MODE-I0 property\n");
        return ret;
    }

    // Read stream info (multi-stream, single-stream)
    ret = of_property_read_u32(node, "xlnx,tile0-ISP0-STREAM", &isp_dev->isp_stream);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read tile0-ISP0-STREAM property\n");
        return ret;
    }

    // Read RPU value
    ret = of_property_read_u32(node, "xlnx,tile0-ISP0-rpu", &isp_dev->isp_rpu);
    if (ret) {
        dev_err(&pdev->dev, "Failed to read tile0-ISP0-rpu property\n");
        return ret;
    }

    // Check for MP enable flag
    isp_dev->isp0_enable_mp = of_property_read_bool(node, "xlnx,tile0-isp0-enable-mp");

    // Parse ports information
    node = of_get_child_by_name(node, "ports");
    if (node) {
        ret = vvcam_isp_parse_ports(isp_dev, node);
        if (ret) {
            dev_err(&pdev->dev, "Failed to parse ports\n");
            return ret;
        }
    } else {
        dev_err(&pdev->dev, "No ports found in device tree\n");
        return -EINVAL;
    }

 
    return 0;
}

static int event_notified_idr_cb(int id, void *data, void *isp_dev_ptr)
{
    
    // Implement the callback logic here
    // Example: Process the event based on the id and data

    // Add your logic to process the event notification here
    // e.g., handling specific IDs or triggering specific actions

    return 0;
}

static void handle_event_notified(struct work_struct *work)
{
    struct vvcam_isp_dev *isp_dev;

    /* Get the vvcam_isp_dev structure from the work struct */
    isp_dev = container_of(work, struct vvcam_isp_dev, mbox_work);

    /* Send a message to the RX mailbox channel */
    (void)mbox_send_message(isp_dev->rx_chan, NULL);

    /* Iterate over all notify IDs and process them */
    idr_for_each(&isp_dev->notifyids, event_notified_idr_cb, isp_dev);
}


static void vvcam_isp_mb_tx_done(struct mbox_client *cl, void *msg, int r)
{
    struct vvcam_isp_dev *isp_dev;
    struct sk_buff *skb;

    /* If no message was sent, simply return */
    if (!msg)
        return;

    /* Retrieve the vvcam_isp_dev structure from the mbox_client */
    isp_dev = container_of(cl, struct vvcam_isp_dev, tx_mc);

    /* Dequeue the corresponding SKB (socket buffer) from the TX SKB queue */
    skb = skb_dequeue(&isp_dev->tx_mc_skbs);

    /* Free the SKB after it has been sent */
    kfree_skb(skb);

    /* Optionally, you could add logging or error handling here based on the value of 'r' */
    if (r < 0) {
        dev_err(isp_dev->dev, "Mailbox TX operation failed with status %d\n", r);
    } else {
       //RKC-LOG_ENABLE dev_dbg(isp_dev->dev, "Mailbox TX operation completed successfully\n");
    }
}

int k_apu_ack_flag,k_apu_data_flag; 
static void my_tasklet_function(unsigned long data) {
   struct vvcam_isp_dev *isp_dev = (struct vvcam_isp_dev *)data;
    if(isp_dev != NULL){
        //RKC-LOG_ENABLE   pr_err("%s %d isp_dev->isp_rpu %d\n",__func__,__LINE__,isp_dev->isp_rpu);
         apu_mailbox_read(isp_dev->isp_rpu);//0 --> rpu0 1 --> rpu1
    if(k_apu_ack_flag){
	//pr_err("%s %d done reading mbox-read k_apu_ack_flag %d \n",__func__,__LINE__,k_apu_ack_flag);	 
       complete(&apu_wait_for_ack);
    }
    else if(k_apu_data_flag){
       complete(&apu_wait_for_data);
      // pr_err("%s %d done reading mbox-read k_apu_data_flag %d \n",__func__,__LINE__,k_apu_data_flag);	 
    }
    else{
      // pr_err("%s %d done reading mbox Going to application\n",__func__,__LINE__);	 
       complete(&mailbox_completion);
    }

     }
}

static void vvcam_isp_mb_rx_cb(struct mbox_client *cl, void *msg)
{
    struct vvcam_isp_dev *isp_dev;
  //RKC-LOG_ENABLE     pr_err("RECEIVED Interrupt\n");
    isp_dev = container_of(cl, struct vvcam_isp_dev, rx_mc);
#if 1
    if (msg) {
        // If needed, process the received message here
        // For example, you could copy the received message to a buffer in the isp_dev structure

        // Assuming you might have a similar structure as in zynqmp_r5_rproc
        struct zynqmp_ipi_message *ipi_msg, *buf_msg;
        size_t len;

        ipi_msg = (struct zynqmp_ipi_message *)msg;
        buf_msg = (struct zynqmp_ipi_message *)isp_dev->rx_mc_buf; // Assuming you have a buffer for the received message
        len = (ipi_msg->len >= IPI_BUF_LEN_MAX) ? IPI_BUF_LEN_MAX : ipi_msg->len;
        buf_msg->len = len;
        // memcpy(buf_msg->data, ipi_msg->data, len); // Uncomment if you need to copy the data
    }
#endif
    // Schedule work to handle the received event
    schedule_work(&isp_dev->mbox_work);
    // If you have any additional tasklets to schedule, you can do it here
     tasklet_schedule(&isp_dev->tasklet);
}


static int vvcam_isp_setup_mbox(struct vvcam_isp_dev *isp_dev, struct device_node *node)
{
    struct mbox_client *mclient;

    /* Setup TX mailbox channel client */
    mclient = &isp_dev->tx_mc;
    mclient->rx_callback = NULL;
    mclient->tx_block = false;
    mclient->knows_txdone = false;
    mclient->tx_done = vvcam_isp_mb_tx_done;  // Set the TX done callback here
    mclient->dev = isp_dev->dev;

    /* Setup RX mailbox channel client */
    mclient = &isp_dev->rx_mc;
    mclient->dev = isp_dev->dev;
    mclient->rx_callback = vvcam_isp_mb_rx_cb;  // Set the RX callback here
    mclient->tx_block = false;
    mclient->knows_txdone = false;

    /* Initialize work */
    INIT_WORK(&isp_dev->mbox_work, handle_event_notified);

    /* Request TX and RX channels (implement this part if necessary) */
    isp_dev->tx_chan = mbox_request_channel_byname(&isp_dev->tx_mc, "tx");
   if (IS_ERR(isp_dev->tx_chan)) {
      dev_err(isp_dev->dev, "failed to request mbox tx channel.\n");
      isp_dev->tx_chan = NULL;
      return -EINVAL;
   }

   isp_dev->rx_chan = mbox_request_channel_byname(&isp_dev->rx_mc, "rx");
   if (IS_ERR(isp_dev->rx_chan)) {
      dev_err(isp_dev->dev, "failed to request mbox rx channel.\n");
      isp_dev->rx_chan = NULL;
      return -EINVAL;
   }
   skb_queue_head_init(&isp_dev->tx_mc_skbs);
    return 0;
}


static int char_dev_open(struct inode *inode, struct file *file)
{
	//struct arm_smccc_res res;
	struct rpu_dev *rpu=NULL;
        /*if (inuse) {
                printk(KERN_ERR "Device busy %s", CHAR_DEV_NAME);
                return -EBUSY;
        }*/
   	rpu  = container_of(inode->i_cdev, struct rpu_dev, cdev);
   	if (rpu){
//              pr_err("%s %d rpu->id %d addr 0x%x\n",__func__,__LINE__,isp_dev->id,isp_dev);
	     	file->private_data = rpu;
    	}

        return SUCCESS;
}
static int char_dev_release(struct inode *inode, struct file *file)
{
        inuse = 0;
        return SUCCESS;
}
static ssize_t char_dev_write(struct file *file, const char __user * buf,
                              size_t lbuf, loff_t * offset)
{
 	int ret;
	spinlock_t lock;
    	payload_packet *packet;
    	payload_user_packet *user_packet = NULL;
        struct zynqmp_ipi_message *apu_msg = NULL;
	//struct vvcam_isp_dev *isp_dev1 = NULL;//file->private_data;
	struct rpu_dev *rpu=file->private_data;
        struct vvcam_isp_dev *isp_dev = rpu->isp_dev;
        /* Ensure that user_buffer is large enough to hold the payload_user_packet struct. */
        if (lbuf < sizeof(payload_user_packet)) {
                return -EINVAL; /* Handle an error for insufficient data.*/
        }
	 //RKC-LOG_ENABLE pr_err("before %s %d\n",__func__,__LINE__);

	//   mutex_lock(&rpu->lock);
	 //RKC-LOG_ENABLE pr_err("after %s %d\n",__func__,__LINE__);

        /* Allocate memory for user_packet */
        user_packet = kmalloc(sizeof(payload_user_packet), GFP_KERNEL);
        if (!user_packet) {
                return -ENOMEM; /* Handle memory allocation failure.*/
        }
        /* Copy data from user space to kernel space. */
        if (copy_from_user(user_packet, buf, sizeof(payload_user_packet))) {
                kfree(user_packet); /* Free allocated memory on error */
                return -EFAULT;     /* Handle an error for the copy operation. */
        }
        packet = (payload_packet *)kmalloc(sizeof(payload_packet),GFP_KERNEL);
        if (!packet) {
                kfree(user_packet);
                return -ENOMEM;
        }
        packet->type = user_packet->type;
        packet->cookie = user_packet->cookie;
        packet->payload_size = user_packet->payload_size;
        memcpy(packet->payload,user_packet->payload,sizeof(user_packet->payload));
         /* get the parameter data from playload_data*/
         uint8_t *p_data = packet->payload;//  packet->payload_data;
         uint32_t instanceId = 0x99;
        memcpy(&instanceId, p_data, sizeof(uint32_t));
	 //RKC-LOG_ENABLE pr_err("%s %d\n",__func__,__LINE__);
	 //RKC-LOG_ENABLE pr_err("%s %d\n",__func__,__LINE__);
	 //RKC-LOG_ENABLE pr_err("isp_dev 0x%x g_inst 0x%x\n",isp_dev,g_instance_array[instanceId]);
	 //RKC-LOG_ENABLE pr_err("%s %d\n",__func__,__LINE__);
	 //RKC-LOG_ENABLE pr_err("%s %d\n",__func__,__LINE__);
        if(isp_dev != NULL){
	
//       isp_dev = g_instance_array[instanceId];
        spin_lock(&lock);
        ret = Send_Command (user_packet->cmd_id,packet,sizeof(payload_packet),isp_dev->isp_rpu,MBOX_CORE_APU);
        spin_unlock(&lock);                               
        ret = mbox_send_message(isp_dev->tx_chan,apu_msg);                                  
        if (ret < 0) {                                                                      
             	printk(KERN_ERR "%s: mbox_send_message() failed: %d\n",                        
                     __func__, ret);                                                     
         	kfree(user_packet);
                kfree(packet);  // Add this line to prevent memory leak
                return -EIO;                                                              
        }        
        }
	else{

             printk(KERN_ERR "%s Instance is NULL\n",__func__);
        }
        kfree(packet);  
        kfree(user_packet);
//      mutex_unlock(&rpu->lock);
//      RKC-LOG_ENABLE pr_err("after unlock %s %d\n",__func__,__LINE__);
        return lbuf;
}
static ssize_t my_driver_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
        //struct vvcam_isp_dev *isp_dev = rpu->isp_dev;
       //RKC-LOG_ENABLE   pr_err("%s-%d\n",__func__,__LINE__);
//         mutex_lock(&rpu->lock);
         int result = wait_for_completion_interruptible(&mailbox_completion);
        if (result == 0) {
                //RKC-LOG_ENABLE  pr_err("Completion signaled successfully VVCAM\n");
        } else {
                printk(KERN_ERR "Completion waiting interrupted VVCAM (error code: %d)\n", result);
        }
	
        if (copy_to_user(buf, &data_from_interrupt, sizeof(struct response_user_packet))) {
                printk(KERN_ERR "Failed to copy\n");
                return -EFAULT; // Handle the copy_to_user error
         }        
    
//      mutex_unlock(&rpu->lock);
        //RKC-LOG_ENABLE   pr_err("Before returning \n");
         return 0xA;
}

static struct file_operations char_dev_fops = {
        .owner = THIS_MODULE,
        .read  = my_driver_read,
        .write = char_dev_write,
        .open = char_dev_open,
        .release = char_dev_release
};
struct reserved_memory reserved_memory;

static struct rpu_dev *find_or_create_rpu(struct platform_device *pdev, int rpu_id)
{
    struct rpu_dev *rpu;
  //  dev_t devno;
    int ret;

    mutex_lock(&rpu_list_lock);

    /* Look for an existing RPU with the given rpu_id */
    list_for_each_entry(rpu, &rpu_devices, node) {
        if (rpu->rpu_id == rpu_id) {
            kref_get(&rpu->refcount);
            mutex_unlock(&rpu_list_lock);
            return rpu;
        }
    }

    char dev_name[20];
    snprintf(dev_name, sizeof(dev_name), "%s_%d", CHAR_DEV_NAME, rpu_id);
    /* Create a new RPU */
    rpu = devm_kzalloc(&pdev->dev, sizeof(*rpu), GFP_KERNEL);
    if (!rpu) {
        mutex_unlock(&rpu_list_lock);
        return ERR_PTR(-ENOMEM);
    }

    rpu->rpu_id = rpu_id;
    /* Initialize the mutex, cdev, and reference count */
    mutex_init(&rpu->lock);
    cdev_init(&rpu->cdev, &char_dev_fops);
    rpu->cdev.owner = THIS_MODULE;

    /* Allocate a device number (dynamic allocation) */
    ret = alloc_chrdev_region(&rpu->devno, rpu_id, 1, dev_name);
    if (ret) {
	printk(KERN_ERR "%s %d failed to allocate \n",__func__,__LINE__);
        devm_kfree(&pdev->dev, rpu);
        mutex_unlock(&rpu_list_lock);
        return ERR_PTR(ret);
    }
    /* Add the character device */
    ret = cdev_add(&rpu->cdev, rpu->devno, 1);
    if (ret) {
        unregister_chrdev_region(rpu->devno, 1);
        devm_kfree(&pdev->dev, rpu);
        mutex_unlock(&rpu_list_lock);
        return ERR_PTR(ret);
    }

    /* Create a device node for this RPU */
    device_create(mailbox_class, NULL, rpu->devno, NULL, dev_name);

/*
    rpu_class = class_create(THIS_MODULE, "rpu_class");
    if (IS_ERR(rpu_class)) {
        printk(KERN_ERR "Failed to create class\n");
        return PTR_ERR(rpu_class);
    }
*/	
    //device_create(rpu_class,NULL,rpu->devno, NULL,dev_name);
    kref_init(&rpu->refcount);

    /* Add the new RPU to the list */
    list_add_tail(&rpu->node, &rpu_devices);

    mutex_unlock(&rpu_list_lock);
    return rpu;
}

int reserved_memory_init(const char *node_name)
{
    struct device_node *node;
    //int ret;

    /* Find the reserved memory node */
    node = of_find_node_by_name(NULL, node_name);
    if (!node) {
        printk(KERN_ERR "Reserved memory node not found\n");
        return -ENODEV;
    }

    /* Get the physical address */
    if (of_property_read_u64(node, "reg", &reserved_memory.phys_addr)) {
        printk(KERN_ERR "Failed to read reg property\n");
        return -EINVAL;
    }
    /* Map to virtual address */
    reserved_memory.virt_addr = of_iomap(node, 0);
    if (!reserved_memory.virt_addr) {
        printk(KERN_ERR "Failed to map reserved memory to virtual address\n");
        return -ENOMEM;
    }
    return 0;
}

void reserved_memory_exit(void)
{
    if (reserved_memory.virt_addr) {
        iounmap(reserved_memory.virt_addr);
        reserved_memory.virt_addr = NULL;
    }
}

#if 1
uint8_t xlnx_mbox_apu_wait_for_data(void *data){
    //int ret = 0;
        k_apu_data_flag =1;
	//pr_err("Waiting for completetion signal at data k_apu_ack_flag %d\n",k_apu_data_flag);
    int result = wait_for_completion_interruptible(&apu_wait_for_data);
    if (result == 0) {
       // pr_err("Completion signaled successfully in VVCAM\n");
    } else {
        printk(KERN_ERR "Completion waiting interrupted (error code: %d)\n", result);
        return 1;
    }
    memcpy(data, data_from_interrupt.res_payload_pkt.payload, sizeof(data_from_interrupt.res_payload_pkt.payload));
      k_apu_data_flag=0;
 
//    pr_err("completed fetching data\n");
    return data_from_interrupt.res_payload_pkt.return_res;
}
EXPORT_SYMBOL(xlnx_mbox_apu_wait_for_data);
 
void xlnx_mbox_apu_wait_for_ack(void){
    //int ret = 0;
        k_apu_ack_flag =1;
    int result = wait_for_completion_interruptible(&apu_wait_for_ack);
    if (result == 0) {
        //pr_err("Completion signaled successfully in VVVCAM\n");
    } else {
        printk(KERN_ERR "Completion waiting interrupted (error code: %d)\n", result);
       /* return NULL;*/
    }
        k_apu_ack_flag =0;
  /*  memcpy(data, data_from_interrupt.res_payload_pkt.payload, sizeof(data_from_interrupt.res_payload_pkt.payload));*/
 
/*    pr_err("completed fetching data\n");*/
/*   return data;*/
}
#endif
int Mailbox_Initialization(struct vvcam_isp_dev *isp_dev){

        int ret;
        //struct arm_smccc_res res;
        //unsigned long a0,a1,a2,a3;
        ret = reserved_memory_init("rpu0mbox0buffer");
        if (ret) {
                printk(KERN_ERR "Failed to initialize reserved memory\n");
                return ret;
        }

        /*      MBOX_CORE_APU*/
        mailbox_init(MBOX_CORE_APU,(uint64_t)(uintptr_t)reserved_memory.virt_addr,reserved_memory.phys_addr);
        ret = mbox_send_message(isp_dev->tx_chan,NULL);                                
        if (ret < 0) {
            printk(KERN_ERR "Failed to trigger IPI\n");
         }
        return SUCCESS; 

}


static int vvcam_isp_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct vvcam_isp_dev *isp_dev;
    int ret;
    struct device_node *node;

    node = pdev->dev.of_node;

    isp_dev = devm_kzalloc(&pdev->dev,
		        sizeof(struct vvcam_isp_dev), GFP_KERNEL);
    if (!isp_dev)
        return -ENOMEM;

   tasklet_init(&isp_dev->tasklet, my_tasklet_function,
                     (unsigned long)isp_dev);
    mutex_init(&isp_dev->mlock);
    mutex_init(&isp_dev->ctrl_lock);
    isp_dev->dev = &pdev->dev;
    platform_set_drvdata(pdev, isp_dev);
    dev_set_drvdata(&pdev->dev,isp_dev);

    ret = vvcam_isp_parse_params(isp_dev, pdev);
    if (ret) {
        dev_err(&pdev->dev, "failed to parse params\n");
        return -EINVAL;
    }
   // store the instance pointer in the array
    if (isp_dev){
      g_instance_array[isp_dev->id] = isp_dev;
   }
    v4l2_subdev_init(&isp_dev->sd, &vvcam_isp_subdev_ops);
	snprintf(isp_dev->sd.name, V4L2_SUBDEV_NAME_SIZE,
		"%s.%d",VVCAM_ISP_NAME, isp_dev->id);

    isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
    isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
    isp_dev->sd.dev =  &pdev->dev;
    isp_dev->sd.owner = THIS_MODULE;
    isp_dev->sd.internal_ops = &vvcam_isp_internal_ops;
    isp_dev->sd.entity.ops = &vvcam_isp_entity_ops;
    isp_dev->sd.entity.function = MEDIA_ENT_F_IO_V4L;
    isp_dev->sd.entity.obj_type = MEDIA_ENTITY_TYPE_V4L2_SUBDEV;
    isp_dev->sd.entity.name = isp_dev->sd.name;
    v4l2_set_subdevdata(&isp_dev->sd, isp_dev);

    vvcam_isp_pads_init(isp_dev);
    ret = media_entity_pads_init(&isp_dev->sd.entity,
                                VVCAM_ISP_PAD_NR, isp_dev->pads);
    if (ret)
       return ret;

    ret = vvcam_isp_async_notifier(isp_dev);
    if (ret)
        goto err_async_notifier;

    ret = v4l2_async_register_subdev(&isp_dev->sd);
    if (ret) {
		dev_err(dev, "register subdev error\n");
		goto error_regiter_subdev;
	}
    // Assign the XML path to sensor_info[0].xml
    strncpy(isp_dev->sensor_info[0].xml, "/run/media/mmcblk0p1/OX03f10.xml", sizeof(isp_dev->sensor_info[0].xml) - 1);
    isp_dev->sensor_info[0].xml[sizeof(isp_dev->sensor_info[0].xml) - 1] = '\0'; // Ensure null termination

    // Assign the JSON paths
    strncpy(isp_dev->sensor_info[0].auto_json, "/run/media/mmcblk0p1/auto.json", sizeof(isp_dev->sensor_info[0].auto_json) - 1);
    isp_dev->sensor_info[0].auto_json[sizeof(isp_dev->sensor_info[0].auto_json) - 1] = '\0'; // Ensure null termination

    strncpy(isp_dev->sensor_info[0].manu_json, "/run/media/mmcblk0p1/manual_ext.json", sizeof(isp_dev->sensor_info[0].manu_json) - 1);
    isp_dev->sensor_info[0].manu_json[sizeof(isp_dev->sensor_info[0].manu_json) - 1] = '\0'; // Ensure null termination

    ret = vvcam_isp_procfs_register(isp_dev, &isp_dev->pde);
    if (ret) {
        dev_err(dev, "register procfs failed.\n");
        goto err_register_procfs;
    }

   /*Addition for mbox code*/
    mbox_data = devm_kzalloc(dev, sizeof(*mbox_data), GFP_KERNEL);
    if (!mbox_data){
      return -ENOMEM;
    }
      char dev_name[20];
    snprintf(dev_name, sizeof(dev_name), "%s_%d", CHAR_DEV_NAME, isp_dev->isp_rpu);

   
    /* Find or create a new RPU with the given rpu_id */
    isp_dev->rpu = find_or_create_rpu(pdev, isp_dev->isp_rpu);
    if (!isp_dev->rpu) {
        dev_err(&pdev->dev, "Failed to find or create RPU: %d\n", isp_dev->isp_rpu);
        return -ENOMEM;
    }
    isp_dev->rpu->isp_dev = isp_dev;	
#if 0
       /* create the device name */
       //alloc_chrdev_region(&mydev, 0, 1, dev_name);
       alloc_chrdev_region(&mydev, 0, 1, CHAR_DEV_NAME);
        
      /* Initialize the character device and register it with the kernel*/
        cdev_init(&mbox_cdev, &char_dev_fops);
        mbox_cdev.owner = THIS_MODULE;
        ret = cdev_add(&mbox_cdev, mydev, 1);
        if (ret < 0) {
                pr_err("Error registering device driver");
                return ret;
        }

        mailbox_class = class_create(THIS_MODULE, "mailbox-class");
        if (IS_ERR(mailbox_class)) {
                pr_err("Failed to create class\n");
                unregister_chrdev_region(mydev, 1);
                return PTR_ERR(mailbox_class);
        }
        //device_create(mailbox_class, NULL, mydev, NULL, dev_name);
        device_create(mailbox_class, NULL, mydev, NULL, CHAR_DEV_NAME);
#endif
   	
	if (of_property_read_bool(node, "mboxes")) {
        ret = vvcam_isp_setup_mbox(isp_dev, node);
        if (ret) {
            dev_err(&pdev->dev, "Failed to set up mailboxes.\n");
            goto err_mbox_setup;
        }
        dev_info(&pdev->dev, "Mailbox communication set up successfully.\n");
    }

        mbox_data->rx_chan = isp_dev->rx_chan;
        mbox_data->tx_chan = isp_dev->tx_chan;
        mbox_data->mbox_client_tx = isp_dev->tx_mc;
        mbox_data->mbox_client_rx = isp_dev->rx_mc;
        ret = Mailbox_Initialization(isp_dev);
        if(ret)
           printk(KERN_ERR "Failed Mailbox  Initialization\n");

    isp_dev->event_shm.virt_addr = (void *)__get_free_pages(GFP_KERNEL, 3);
    isp_dev->event_shm.size = PAGE_SIZE * 8;
    memset(isp_dev->event_shm.virt_addr, 0, isp_dev->event_shm.size);
    isp_dev->event_shm.phy_addr = virt_to_phys(isp_dev->event_shm.virt_addr);
    mutex_init(&isp_dev->event_shm.event_lock);

    pm_runtime_enable(&pdev->dev);
    vvcam_isp_ctrl_init(isp_dev);

    dev_info(&pdev->dev, "vvcam isp driver probe success\n");

    return 0;

err_register_procfs:
    v4l2_async_unregister_subdev(&isp_dev->sd);

error_regiter_subdev:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
                v4l2_async_nf_unregister(&isp_dev->notifier);
                v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
				v4l2_async_notifier_unregister(&isp_dev->notifier);
	            v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
err_async_notifier:
    media_entity_cleanup(&isp_dev->sd.entity);
err_mbox_setup:
    return ret;
}
#if 0
static void rpu_release(struct kref *ref)
{
    struct rpu_dev *rpu = container_of(ref, struct rpu_dev, refcount);

    list_del(&rpu->node);
    mutex_destroy(&rpu->lock);
    kfree(rpu);
}
#endif

static void rpu_cleanup(struct kref *ref)
{
/*    struct rpu_dev *rpu = container_of(ref, struct rpu_dev, refcount);
 *        devm_kfree(&rpu->pdev->dev, rpu);*/
}

//void rpu_remove(struct platform_device *pdev, int rpu_id) {
void rpu_remove(int rpu_id) {
    struct rpu_dev *rpu;
    mutex_lock(&rpu_list_lock);
    /* Find the RPU with the given rpu_id */
    list_for_each_entry(rpu, &rpu_devices, node) {
        if (rpu->rpu_id == rpu_id) {
            /* Release the RPU if its reference count is 1 (only held by the driver) */
      if (atomic_read(&rpu->refcount.refcount.refs) == 1) {
                /* Delete the RPU device node */
		device_destroy(mailbox_class, rpu->devno);
        	class_destroy(mailbox_class);


                //device_destroy(rpu_class, rpu->devno);
    		/* Destroy the class */
    		//class_destroy(rpu_class);
                /* Delete the character device and unregister the device number */
                cdev_del(&rpu->cdev);
                unregister_chrdev_region(rpu->devno, 1);
                kref_put(&rpu->refcount, rpu_cleanup); // Define rpu_cleanup to handle freeing resources.
		/* Remove the RPU from the list */
                list_del(&rpu->node);
                /* Free the RPU structure */
            } else {
                kref_put(&rpu->refcount, rpu_cleanup); // Define rpu_cleanup to handle freeing resources.
            }
            break;
        }
    }

       mutex_unlock(&rpu_list_lock);
}


static int vvcam_isp_remove(struct platform_device *pdev)
{
    struct vvcam_isp_dev *isp_dev;

    isp_dev = platform_get_drvdata(pdev);
      
    /* Release the RPU */
    //kref_put(&isp_dev->rpu->refcount, rpu_release);

    vvcam_isp_procfs_unregister(isp_dev->pde);
    v4l2_async_unregister_subdev(&isp_dev->sd);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
                v4l2_async_nf_unregister(&isp_dev->notifier);
                v4l2_async_nf_cleanup(&isp_dev->notifier);
#else
				v4l2_async_notifier_unregister(&isp_dev->notifier);
	            v4l2_async_notifier_cleanup(&isp_dev->notifier);
#endif
    media_entity_cleanup(&isp_dev->sd.entity);
    pm_runtime_disable(&pdev->dev);
    free_pages((unsigned long)isp_dev->event_shm.virt_addr, 3);
    vvcam_isp_ctrl_destroy(isp_dev);
    dev_info(&pdev->dev, "vvcam isp driver remove\n");
      
  if (of_property_read_bool(isp_dev->dev->of_node, "mboxes")) {
                        printk(KERN_ERR "Freeing Mbox channel\n");
                        mbox_free_channel(isp_dev->tx_chan);
                        mbox_free_channel(isp_dev->rx_chan);
                }
  // Clean up reserved_memory structure
  reserved_memory_exit();
  /* Unmap and release the memory region when done*/
  //iounmap(virt_addr);
  //release_mem_region(my_reserved_mem->start, resource_size(my_reserved_mem));
  //rpu_remove(pdev,isp_dev->isp_rpu);
  return 0;
}

static int vvcam_isp_system_suspend(struct device *dev)
{
    int ret = 0;
    ret = pm_runtime_force_suspend(dev);
    if (ret) {
        dev_err(dev, "force suspend %s failed\n", dev_name(dev));
        return ret;
    }
	return ret;
}

static int vvcam_isp_system_resume(struct device *dev)
{
    int ret = 0;
    ret = pm_runtime_force_resume(dev);
    if (ret) {
        dev_err(dev, "force resume %s failed\n", dev_name(dev));
        return ret;
    }
	return ret;
}

static int vvcam_isp_runtime_suspend(struct device *dev)
{
    return 0;
}

static int vvcam_isp_runtime_resume(struct device *dev)
{
    return 0;
}

static const struct dev_pm_ops vvcam_isp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(vvcam_isp_system_suspend, vvcam_isp_system_resume)
	SET_RUNTIME_PM_OPS(vvcam_isp_runtime_suspend, vvcam_isp_runtime_resume, NULL)
};

static const struct of_device_id vvcam_isp_of_match[] = {
	{.compatible = "verisilicon,isp",},
	{ /* sentinel */ },
};

static struct platform_driver vvcam_isp_driver = {
	.probe  = vvcam_isp_probe,
	.remove = vvcam_isp_remove,
	.driver = {
		.name           = VVCAM_ISP_NAME,
		.owner          = THIS_MODULE,
        .of_match_table = vvcam_isp_of_match,
        .pm             = &vvcam_isp_pm_ops,
	}
};

static int __init vvcam_isp_init_module(void)
{
    int ret;

    mailbox_class = class_create("mailbox-class");
    if (IS_ERR(mailbox_class)) {
        printk(KERN_ERR "Failed to create class\n");
        return PTR_ERR(mailbox_class);
    }
    ret = platform_driver_register(&vvcam_isp_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register isp driver\n");
	class_destroy(mailbox_class);
        return ret;
    }

    return ret;
}

static void __exit vvcam_isp_exit_module(void)
{
  rpu_remove(0);
    platform_driver_unregister(&vvcam_isp_driver);
}

module_init(vvcam_isp_init_module);
module_exit(vvcam_isp_exit_module);

MODULE_DESCRIPTION("Verisilicon isp v4l2 driver");
MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL");
