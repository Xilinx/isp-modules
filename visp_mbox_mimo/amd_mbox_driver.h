/******************************************************************************\
|* Copyright (c) 2023-2024 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")  *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __AMD_MBOX_DRIVER_H__
#define __AMD_MBOX_DRIVER_H__

#include "vvcam_isp_driver.h"
#define CHAR_DEV_NAME "mailbox_dev"
#define SUCCESS 0
struct rpu_dev *get_rpu_dev(int rpu_id);
extern struct response_user_packet data_from_interrupt;
void xlnx_mbox_apu_wait_for_ack(struct vvcam_isp_dev *isp_dev);
void mailbox_init(uint32_t cpu,uint64_t MBOX_FIFO_START_ADDR,uint64_t MBOX_FIFO_START_ADDR_PHY);
uint8_t xlnx_mbox_apu_wait_for_data(struct vvcam_isp_dev *isp_dev,void *data);
extern int Handle_Frameout_Buffer(void * Enque_Buff_L, struct vvcam_isp_dev *isp_dev);
//int (* exported_func1)(void *,struct vvcam_isp_dev *isp_dev);
#if 1
typedef struct payload_user_template {

        Payload_type type;
        MBCmdId_E cmd_id ;
        __u32 cookie;
        __u32 payload_size;
         response_field_t resp_field;
	   uint8_t payload[MAX_ITEM];

}payload_user_packet ;
#endif
struct reserved_memory {
    phys_addr_t phys_addr;
    void __iomem *virt_addr;
};



#endif
