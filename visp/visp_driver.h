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

#ifndef __VISP_DRIVER_H__
#define __VISP_DRIVER_H__
#include <linux/list.h>
#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/videobuf2-dma-contig.h>
#include "visp_v4l2_common.h"
//RKC-TODO Check if below headers were required in M13
#include "media_isp.h"
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/mailbox/zynqmp-ipi-message.h>
#include <linux/mailbox_client.h>
#include <linux/mailbox_controller.h>
#include <linux/skbuff.h>

#define VISP_NAME "visp-isp-subdev"
#define VISP_SUBDEV_NAME_SIZE 52

#define VISP_WIDTH_ALIGN 16
#define VISP_HEIGHT_ALIGN 8
#define VISP_WIDTH_MIN 32
#define VISP_HEIGHT_MIN 16

#define VISP_PORT_NR 4

/* IPI buffer MAX length */
#define IPI_BUF_LEN_MAX 32U
/* RX mailbox client buffer max length */
#define RX_MBOX_CLIENT_BUF_MAX \
	(IPI_BUF_LEN_MAX + sizeof(struct zynqmp_ipi_message))
#define MAX_BANKS 4U
#define MAX_PORTS 4 // Number of ports to parse
#define MAX_NO_ISP 6

enum visp_port_pad_e
{
	VISP_PORT_PAD_SINK = 0,
	VISP_PORT_PAD_SOURCE_MP,
	VISP_PORT_PAD_SOURCE_SP1,
	VISP_PORT_PAD_SOURCE_SP2,
	VISP_PORT_PAD_SOURCE_RAW,
	VISP_PORT_PAD_NR,
};

#define VISP_PAD_NR (VISP_PORT_NR * VISP_PORT_PAD_NR)

enum visp_path_out_type_e
{
	VISP_PATH_OUT_TYPE_MEMORY = 0, /**< path out in memory type*/
	VISP_PATH_OUT_TYPE_STREAM = 1, /**< path out in stream type */
	VISP_PATH_OUT_TYPE_MAX,		   /**< Maximum path out type. */
};

enum visp_mcm_input_select_e
{
	VISP_MCM_INPUT_SELECT_RPU = 0, /**< alloc mcm input buffer memory from rpu*/
	VISP_MCM_INPUT_SELECT_APU =
		1,					   /**< alloc mcm input buffer memory from apu */
	VISP_MCM_INPUT_SELECT_MAX, /**< Maximum mcm input buffer memory selection. */
};

struct visp_format
{
	uint32_t code;
	uint32_t fourcc;
};

struct visp_pad_data
{
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
};

struct visp_event_shm
{
	struct mutex event_lock;
	uint64_t phy_addr;
	void *virt_addr;
	uint32_t size;
};

#define MAX_IBA_PER_ISP 5

#if 0
struct iba_info {
   u32 ppc;
   u32 vcid;
   u32 frame_rate;
   u32 data_format;
   u32 max_width;
   u32 max_height;
};
#endif
typedef struct iba_info
{
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

struct visp_sensor_info
{
	char sensor[MEDIA_ISP_CHAR_LENGTH_MAX];
	uint8_t mode;
	char xml[MEDIA_ISP_CHAR_LENGTH_MAX];
	char manu_json[MEDIA_ISP_PATH_LENGTH_MAX];
	char auto_json[MEDIA_ISP_PATH_LENGTH_MAX];
	char one_json[MEDIA_ISP_PATH_LENGTH_MAX];
	uint32_t vc_id;
	uint32_t sensor_id;
};

struct visp_port_info
{
	uint32_t vcid;
	uint32_t data_format;
	uint32_t res_ver;
	uint32_t res_hor;
	uint32_t fps;
	uint32_t mode;
};
struct visp_ext_dma_buf
{
	uint64_t addr;
	void *vaddr;
	uint32_t size;
	struct list_head entry;
};

/* Structures to hold the rpu_device specific information */
struct rpu_dev
{
	struct device *dev;
	int rpu_id;
	dev_t devno;
	struct cdev cdev;
	struct mutex lock;
	struct mutex ack_lock;
	struct mutex read_lock;
	struct mutex rpu_lock;
	struct mutex write_lock;
	struct list_head node;
	struct kref refcount;
	struct tasklet_struct tasklet;
	struct visp_dev *isp_dev[MAX_NO_ISP];
	struct class *rpu_class[4];
	struct mbox_client tx_mc;
	struct mbox_client rx_mc;
	struct mbox_chan *tx_chan;
	struct mbox_chan *rx_chan;
	struct work_struct mbox_work;
	struct sk_buff_head tx_mc_skbs;
	struct completion mailbox_completion;
	int app_wait_flag;
};

struct atm_prop {
    u32 high_mem_addr;      // Higher 32-bit of memory address
    u32 is_64bit;     // True if 64-bit, False if 32-bit
    char node_name[50]; // Node name dynamically generated
};

struct visp_dev
{
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
	uint32_t refcnt;
	struct v4l2_subdev sd;
	struct media_pad pads[VISP_PAD_NR];
	struct v4l2_async_notifier notifier;
#ifdef VISP_PLATFORM_REGISTER
	struct fwnode_handle fwnode;
#endif
	struct visp_pad_data pad_data[VISP_PAD_NR];

	struct visp_event_shm event_shm;
	struct v4l2_ctrl_handler ctrl_handler;
	struct mutex ctrl_lock;
	uint32_t ctrl_pad;

	unsigned long pde;
	//struct visp_sensor_info sensor_info[VISP_PORT_NR];
	struct iba_info iba[MAX_IBA_PER_ISP];
	enum visp_path_out_type_e output_type[VISP_PORT_NR][VISP_PORT_PAD_NR - 1];
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
	u32 num_streams;		// Example member to store device tree property
	u32 isp_mem;
	u32 isp_mode;
	u32 isp_rpu;
	u32 dma_align;
	const char *vid_formats;
	const char *sensor_name;
	bool tile0_enable_mp0;
	bool tile0_enable_sp0;
	uint32_t PortsMask;
	struct visp_port_info port_info[MAX_PORTS]; // Ports info (up to 4)
	MediaIspPortAttr IspPorts[MEDIA_ISP_PORT_MAX];
	struct mutex port_lock[MEDIA_ISP_PORT_MAX];
	int streamon[VISP_PORT_PAD_NR * MAX_PORTS];
	//Flags
	struct completion apu_wait_for_ack;
	struct completion apu_wait_for_data;
	struct completion mailbox_completion;
	struct atm_prop atm;
	int k_apu_ack_flag;
	int k_apu_data_flag;
	int app_wait_flag;
};

int Handle_Frameout_Buffer(void *Enque_Buff_L, struct visp_dev *isp_dev);
#endif
