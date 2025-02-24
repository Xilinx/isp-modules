/******************************************************************************\
|* Copyright (c) <2021> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/


#include "mbox_error_code.h"
//#include <linux/mbox_fifo.h>
#include <sensor_cmd.h>
#include <mbox_fifo.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <asm-generic/barrier.h>
#if 0
void fifo_info(FifoControl *fifo)
{
    pr_err("%s-%d\n",__func__,__LINE__);	
    pr_err("\tbuffer_phy       (%llx) %llx\n", &fifo->buffer_phy, fifo->buffer_phy);
    pr_err("\tbuffer_virt       (%llx) %llx\n", &fifo->buffer_virt, fifo->buffer_virt);
    pr_err("\titem_size    (%llx) %lu\n", &fifo->item_size, fifo->item_size);
    pr_err("\titem_total   (%llx) %lu\n", &fifo->item_total, fifo->item_total);
    pr_err("\tbuffer_size  (%llx) %lu\n", &fifo->buffer_size, fifo->buffer_size);
    pr_err("\titem_stored  (%llx) %lu\n", &fifo->item_stored, fifo->item_stored);
    pr_err("\tread_offset  (%llx) %lu\n", &fifo->read_offset, fifo->read_offset);
    pr_err("\twrite_offset (%llx) %lu\n", &fifo->write_offset, fifo->write_offset);
}
EXPORT_SYMBOL_GPL(fifo_info);
void fifo_info(FifoControl *fifo)
{
    pr_err("%s-%d\n", __func__, __LINE__);
    pr_err("\tbuffer_phy       (%p) %llx\n", (void*)&fifo->buffer_phy, fifo->buffer_phy);
    pr_err("\tbuffer_virt      (%p) %llx\n", (void*)&fifo->buffer_virt, fifo->buffer_virt);
    pr_err("\titem_size        (%p) %llu\n", (void*)&fifo->item_size, fifo->item_size);
    pr_err("\titem_total       (%p) %llu\n", (void*)&fifo->item_total, fifo->item_total);
    pr_err("\tbuffer_size      (%p) %llu\n", (void*)&fifo->buffer_size, fifo->buffer_size);
    pr_err("\titem_stored      (%p) %llu\n", (void*)&fifo->item_stored, fifo->item_stored);
    pr_err("\tread_offset      (%p) %llu\n", (void*)&fifo->read_offset, fifo->read_offset);
    pr_err("\twrite_offset     (%p) %llu\n", (void*)&fifo->write_offset, fifo->write_offset);
}
EXPORT_SYMBOL_GPL(fifo_info);
#endif

void fifo_info(FifoControl *fifo)
{   
    pr_err("%s-%d\n", __func__, __LINE__);
    pr_err("\tbuffer_phy       (%p) %p\n", (void*)&fifo->buffer_phy, fifo->buffer_phy); 
    pr_err("\tbuffer_virt      (%p) %p\n", (void*)&fifo->buffer_virt, fifo->buffer_virt); 
    pr_err("\titem_size        (%p) %u\n", (void*)&fifo->item_size, fifo->item_size); 
    pr_err("\titem_total       (%p) %u\n", (void*)&fifo->item_total, fifo->item_total); 
    pr_err("\tbuffer_size      (%p) %u\n", (void*)&fifo->buffer_size, fifo->buffer_size); 
    pr_err("\titem_stored      (%p) %u\n", (void*)&fifo->item_stored, fifo->item_stored); 
    pr_err("\tread_offset      (%p) %u\n", (void*)&fifo->read_offset, fifo->read_offset); 
    pr_err("\twrite_offset     (%p) %u\n", (void*)&fifo->write_offset, fifo->write_offset); 
}   

EXPORT_SYMBOL_GPL(fifo_info);


int fifo_init(FifoControl *fifo, FifoInitData *init_fifo)
{
   if (fifo == NULL || init_fifo == NULL)
        return VPI_ERR_INVALID;

    fifo->buffer_phy = (uint64_t *)(init_fifo->buffer_addr_phy); //Physical Address
    fifo->buffer_virt = (uint64_t *)(init_fifo->buffer_addr_virt); //virtual Address
    fifo->item_size    = init_fifo->item_size;
    fifo->item_total   = init_fifo->item_total;
    fifo->buffer_size  = init_fifo->buffer_size;
#if 1    //TODO: do not init these values if this devices boots second.... or syncronize the init's to avoid overriding 
    fifo->item_stored  = 0;
    fifo->read_offset  = 0;
    fifo->write_offset = 0;
#endif
    //fifo_info(fifo);
    return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(fifo_init);

int fifo_write(MboxPostMsg *msg, FifoControl *fifo)
{
	//pr_err("[MBOX-FIFO] ");

    if (fifo == NULL || msg == NULL)
        return VPI_ERR_INVALID;
    if (fifo->item_stored >= fifo->item_total)
    {	pr_err("\n\n BUFFER FULL ERROR \n\n");
        return VPI_ERR_FULL;
    }

   //memcpy((((uint64_t)fifo->buffer_virt) + fifo->write_offset), msg, ALIGN(sizeof(MboxPostMsg) - sizeof(payload_packet) + msg->size, 8));
   // fifo_info(fifo);
   memcpy(((uint8_t *)fifo->buffer_virt + fifo->write_offset), msg, ALIGN(sizeof(MboxPostMsg) - sizeof(payload_packet) + msg->size, 8));
    dmb(ish);
    fifo->write_offset += fifo->item_size; //fifo->buffer_size are different msg->size !
    if (fifo->write_offset >= fifo->buffer_size)
        fifo->write_offset = 0;
    fifo->item_stored++;

    return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(fifo_write);


int fifo_read(MboxPostMsg *msg, FifoControl *fifo)
{

     MboxPostMsg *fifo_msg;// =(MboxPostMsg *) (((char *)fifo->buffer_virt + fifo->read_offset));
    if (fifo == NULL)
        return VPI_ERR_INVALID;
    if (fifo->item_stored == 0)
        return VPI_ERR_EMPTY;
    fifo_msg =(MboxPostMsg *) (((char *)fifo->buffer_virt + fifo->read_offset));
//    memcpy(msg, fifo_msg,sizeof(MboxPostMsg)-sizeof(payload_packet)+((fifo_msg->size)+63) & ~63 );

    memcpy(msg, fifo_msg, sizeof(MboxPostMsg) - sizeof(payload_packet) + (((fifo_msg->size) + 63) & ~63));
    fifo->read_offset += fifo->item_size;
    if (fifo->read_offset >= fifo->buffer_size)
        fifo->read_offset = 0;
    fifo->item_stored--;
    //fifo_info(fifo);
    return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(fifo_read);

int fifo_reset(FifoControl *fifo)
{
    if (fifo == NULL)
        return VPI_ERR_INVALID;

    fifo->item_stored  = 0;
    fifo->read_offset  = 0;
    fifo->write_offset = 0;

    return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(fifo_reset);

uint32_t fifo_get_stored(FifoControl *fifo)
{
    if (fifo == NULL)
        return VPI_ERR_INVALID;
    return fifo->item_stored;
}
EXPORT_SYMBOL_GPL(fifo_get_stored);

bool fifo_is_full(FifoControl *fifo)
{
    return fifo->item_stored >= fifo->item_total ? true : false;
}
EXPORT_SYMBOL_GPL(fifo_is_full);

bool fifo_is_empty(FifoControl *fifo)
{
//	xil_printf("fifo ctrl addresses %x,%x,%x,%x,->item stroed %x,read offset-> %x \n",&fifo->buffer,&fifo->item_size,&fifo->item_total,
//			&fifo->buffer_size,&fifo->item_stored,&fifo->read_offset);
    return fifo->item_stored == 0 ? true : false;
}
EXPORT_SYMBOL_GPL(fifo_is_empty);

int fifo_buffer_free(FifoControl *fifo)
{
    if (fifo == NULL)
        return VPI_ERR_INVALID;
    kfree(fifo->buffer_phy);
    kfree(fifo->buffer_virt);
    return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(fifo_buffer_free);

MODULE_LICENSE("GPL");
