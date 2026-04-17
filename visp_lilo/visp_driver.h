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

#ifndef __VISP_DRIVER_H__
#define __VISP_DRIVER_H__
#include <linux/list.h>
#include <linux/videodev2.h>
#include <linux/completion.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/videobuf2-dma-contig.h>
#include "visp_v4l2_common.h"
#include "media_isp.h"
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/mailbox/zynqmp-ipi-message.h>
#include <linux/mailbox_client.h>
#include <linux/mailbox_controller.h>
#include <linux/skbuff.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include "iba.h"
#include "oba.h"

#define VISP_NAME "visp-isp-subdev-lilo"
#define VISP_SUBDEV_NAME_SIZE 52

#define VISP_WIDTH_ALIGN 16
#define VISP_HEIGHT_ALIGN 8
#define VISP_WIDTH_MIN 32
#define VISP_HEIGHT_MIN 16

#define VISP_PORT_NR 4

/* IPI buffer MAX length */
#define IPI_BUF_LEN_MAX 32U
/* RX mailbox client buffer max length */
#define RX_MBOX_CLIENT_BUF_MAX                                                 \
	(IPI_BUF_LEN_MAX + sizeof(struct zynqmp_ipi_message))
#define MAX_BANKS 4U
#define MAX_PORTS 4 // Number of ports to parse
#define MAX_NO_ISP 6
/* Allow MP+SP on the same port to enqueue in parallel */
#define ENQ_WQ_MAX_ACTIVE 1
/* Per-port per-chain enqueue workqueues (MP/SP1 only) */
#define ENQ_WQ_CHAIN_MAX 2

enum visp_port_pad_e {
	VISP_PORT_PAD_SINK = 0,
	VISP_PORT_PAD_SOURCE_MP,
	VISP_PORT_PAD_SOURCE_SP1,
	VISP_PORT_PAD_SOURCE_SP2,
	VISP_PORT_PAD_SOURCE_RAW,
	VISP_PORT_PAD_NR,
};

#define VISP_PAD_NR (VISP_PORT_NR * VISP_PORT_PAD_NR)
/* Forward declaration for mailbox message structure */
struct mbox_post_msg;

typedef int (*frameout_cb_t)(struct visp_dev *dev, int port, struct mbox_post_msg *msg);
enum visp_path_out_type_e {
	VISP_PATH_OUT_TYPE_MEMORY = 0, /**< path out in memory type*/
	VISP_PATH_OUT_TYPE_STREAM = 1, /**< path out in stream type */
	VISP_PATH_OUT_TYPE_MAX,	       /**< Maximum path out type. */
};

enum visp_mcm_input_select_e {
	VISP_MCM_INPUT_SELECT_RPU =
	    0, /**< alloc mcm input buffer memory from rpu*/
	VISP_MCM_INPUT_SELECT_APU =
	    1, /**< alloc mcm input buffer memory from apu */
	VISP_MCM_INPUT_SELECT_MAX, //Maximum mcm input buffer memory
				   //selection
};

struct visp_format {
	uint32_t code;
	uint32_t fourcc;
};

struct visp_pad_data {
	uint32_t sink_detected;
	struct v4l2_mbus_framefmt format;
	struct v4l2_fract frmival_min;
	struct v4l2_fract frmival_max;
	uint32_t num_formats;
	struct visp_format *fmts;
	struct list_head queue;
	spinlock_t qlock;
	uint32_t stream;
	struct v4l2_fract timeperframe;
	uint32_t sequence; /* Frame sequence counter for this pad */
};

struct visp_event_shm {
	struct mutex event_lock;
	void *virt_addr;
	uint32_t size;
	dma_addr_t dma_handle;
	struct dma_buf *dmabuf;
	int dmabuf_fd;
	struct device *dev;
	struct completion event_ack;
	uint32_t seq;
};

typedef struct iba_info {
	u32 baseaddress;
	u32 iba_id;
	u32 max_width;
	u32 max_height;
	u32 data_format;
	u32 is_iba_enabled;
	u32 hblank_prog;
	u32 vblank_prog;
	u32 vcid;
	u32 ppc;
	u32 isp_id;
	u32 frame_rate;
} __attribute((__packed__)) __attribute((aligned(8))) iba_info_t;

struct visp_sensor_info {
	char sensor[MEDIA_ISP_CHAR_LENGTH_MAX];
	uint8_t mode;
	char xml[MEDIA_ISP_CHAR_LENGTH_MAX];
	char manu_json[MEDIA_ISP_PATH_LENGTH_MAX];
	char auto_json[MEDIA_ISP_PATH_LENGTH_MAX];
	char one_json[MEDIA_ISP_PATH_LENGTH_MAX];
	uint32_t vc_id;
	uint32_t sensor_id;
};

struct visp_port_info {
	uint32_t vcid;
	uint32_t data_format;
	uint32_t res_ver;
	uint32_t res_hor;
	uint32_t fps;
	uint32_t mode;
};
struct visp_ext_dma_buf {
	uint64_t addr;
	void *vaddr;
	uint32_t size;
	struct list_head entry;
};

struct atm_prop {
	u32 high_mem_addr;  // Higher 32-bit of memory address
	u32 is_64bit;	    // True if 64-bit, False if 32-bit
	char node_name[50]; // Node name dynamically generated
};

struct visp_subdev_dma_buf {
	uint64_t pa;
	int size;
};

struct visp_reserve_mem {
	dma_addr_t pa;
	uint32_t size;
	void *va;
};

enum isp_mode {
	ISP_MODE_LIMO,
	ISP_MODE_LILO,
	ISP_MODE_MIMO,
	ISP_MODE_UNKNOWN,
};

struct visp_common {
	enum isp_mode mode;
	void *isp_dev; // pointer to specific driver struct
};

struct buf_instance {
	int num_bufs;       // how many buffers this instance has
	void **buffer;    // array of buffer pointers
};

struct visp_lilo_isp_dev_extended {
	int id;
	bool is_oba_yuv_420[VISP_PORT_PAD_NR];
	int yuv_420_format_index[VISP_PORT_PAD_NR];
	struct buf_instance buf_list[VISP_PORT_PAD_NR];
};

static inline enum isp_mode get_isp_mode_from_str(const char *mode_str)
{
	if (!mode_str)
		return ISP_MODE_UNKNOWN;

	if (strcmp(mode_str, "limo") == 0)
		return ISP_MODE_LIMO;
	else if (strcmp(mode_str, "lilo") == 0)
		return ISP_MODE_LILO;
	else if (strcmp(mode_str, "mimo") == 0)
		return ISP_MODE_MIMO;

	return ISP_MODE_UNKNOWN;
}

#define VISP_DISPLAY_KFIFO_SIZE 16

#define VISP_KFIFO_SIZE 64
struct visp_dev {
	phys_addr_t paddr;
	struct rpu_dev *rpu;
	uint32_t regs_size;
	void __iomem *base;
	void __iomem *reset;
	struct cdev cdev;
	int id;
	int fe_irq;
	int isp_irq;
	int mi_irq;
	struct device *dev;
	struct mutex mlock;
	/* isp_dev level calib lock */
	struct mutex calib_lock;
	uint32_t refcnt;
	struct v4l2_subdev sd;
	/* Dynamic pad allocation based on num_streams */
	int num_pads;
	struct media_pad *pads;
	struct v4l2_async_notifier notifier;
#ifdef VISP_PLATFORM_REGISTER
	struct fwnode_handle fwnode;
#endif
	struct visp_pad_data *pad_data;

	struct visp_reserve_mem reserve_mem;
	struct visp_event_shm event_shm;
	struct v4l2_ctrl_handler ctrl_handler;
	struct mutex ctrl_lock;
	uint32_t ctrl_pad;

	unsigned long pde;
	struct iba_info iba[MAX_IBA_PER_ISP];
	enum visp_path_out_type_e output_type[VISP_PORT_NR]
					     [VISP_PORT_PAD_NR - 1];
	enum visp_mcm_input_select_e mcm_input_select[VISP_PORT_NR];

	struct list_head mcm_input[VISP_PORT_NR];

	struct tasklet_struct tasklet;
	// Structure added for mailbox
	unsigned char rx_mc_buf[RX_MBOX_CLIENT_BUF_MAX];
	struct mbox_client tx_mc;
	struct mbox_client rx_mc;
	struct mbox_chan *tx_chan;
	struct mbox_chan *rx_chan;
	struct work_struct mbox_work;
	struct sk_buff_head tx_mc_skbs;
	struct idr notifyids;

	const char *ss_mode_i0; // Example member to store device tree property
	u32 num_streams;	// Example member to store device tree property
	u32 isp_mem;
	u32 isp_mode;
	u32 isp_rpu;
	u32 dma_align;
	const char *vid_formats;
	const char *sensor_name;
	bool tile0_enable_mp0;
	bool tile0_enable_sp0;
	uint32_t ports_mask;
	struct visp_port_info port_info[MAX_PORTS]; // Ports info (up to 4)
	struct oba_info oba[MAX_OBA_PER_ISP];
	media_isp_port_attr isp_ports[MEDIA_ISP_PORT_MAX];
	struct mutex port_lock[MEDIA_ISP_PORT_MAX];
	int streamon[VISP_PORT_PAD_NR * MAX_PORTS];
	// Pipeline subdevices storage
	// flags
	/* 3D array for ENQUE_BUFFER: [MAX_PORTS][path][buffer_index] */
	/* Support 4 instances, 4 paths, 32 buffer slots per path */
	struct completion apu_wait_for_enq_ack[4][4][32];
	/* Port-level completions for other commands */
	struct completion apu_wait_for_cmd_ack[4];  /* 4 ports */
	struct completion apu_wait_for_data;
	struct completion mailbox_completion;
	/* Port-level FIFOs for other commands */
	DECLARE_KFIFO_PTR(cmd_ack_fifo[4], struct mbox_post_msg *);
	struct mutex cmd_ack_fifo_lock[4];  /* One mutex per port */
	/* Direct error code storage - eliminates FIFO overhead for ENQUE_BUFFER */
	int enq_ack_error[4][4][32];  /* [port][path][buffer_index] */
	int cmd_ack_error[4];  /* [port] */
	/* Per-port enqueue workqueues (parallel up to ENQ_WQ_MAX_ACTIVE) */
	struct workqueue_struct *enq_wq[MAX_PORTS];
	/* Per-port, per-chain enqueue workqueues for MP/SP1 (chains 0/1) */
	struct workqueue_struct *enq_wq_chain[MAX_PORTS][ENQ_WQ_CHAIN_MAX];
	struct atm_prop atm;
	wait_queue_head_t wq_frame_done_finished;
	bool apu_wait_for_isp_frame_done;
	struct mbox_post_msg *pending_frameout_msg[MAX_PORTS];
	/* required for visp_mimo */
	spinlock_t frameout_lock[MAX_PORTS];
	frameout_cb_t frameout_cb;
	dma_addr_t ip_a[2];
	dma_addr_t op_a[4];
	unsigned int out_w;
	unsigned int out_h;
	unsigned int cap_w;
	unsigned int cap_h;
	unsigned long int out_sizeimage;
	unsigned long int cap_sizeimage;
	unsigned int out_fmt;
	unsigned int cap_fmt;
	unsigned int isp_dq_out_index;
	void *extended_struct;
};

#define ISP_DEV_EXTENDED(isp_dev) \
((struct visp_lilo_isp_dev_extended *)((isp_dev)->extended_struct))
static inline int visp_get_num_pads(struct visp_dev *isp_dev)
{
	return isp_dev->num_streams * VISP_PORT_PAD_NR;
}


#endif
