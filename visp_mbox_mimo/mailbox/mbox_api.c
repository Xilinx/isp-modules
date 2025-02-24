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


//#include <linux/mbox_api.h>
//#include <linux/mbox_fifo.h>
#include <mbox_api.h>
#include <mbox_fifo.h>
#include "mbox_error_code.h"
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include<linux/module.h>
#include <linux/ioport.h>

#define INVALID_PATH 0xFFFF
static uint16_t g_mem_list[MBOX_CORE_MAX][MBOX_CORE_MAX] = {{INVALID_PATH, 0, 1},
                                                            {2, INVALID_PATH, INVALID_PATH},
                                                            {3, INVALID_PATH, INVALID_PATH}};



int mbox_mem_map(MboxFifoCtrl *mbox_fifo, MboxCoreId core_id,
                        uint64_t shm_start_addr,uint64_t shm_start_addr_phy, uint32_t shm_block_size);
int mbox_mem_map(MboxFifoCtrl *mbox_fifo, MboxCoreId core_id,
                        uint64_t shm_start_addr,uint64_t shm_start_addr_phy, uint32_t shm_block_size)
{
	MboxCoreId sender_id;
    MboxCoreId receiver_id;
    uint32_t mlist, ret;
    uint64_t buffer_addr_virt = shm_start_addr; //Virtual Address 
    uint64_t buffer_addr_phy = shm_start_addr_phy;
 
   	FifoInitData *init_fifo = NULL;
	
    init_fifo = (FifoInitData *)kmalloc(sizeof(FifoInitData),GFP_KERNEL);

    pr_err("mbox_mem_map entering... coreid %d \n",core_id);
    if (NULL == init_fifo)
        return VPI_ERR_NOMEM;

    init_fifo->item_size = sizeof(MboxPostMsg);
    init_fifo->buffer_size = shm_block_size - ((sizeof(FifoControl) + 7) / 8 * 8);
//    pr_err("%s-%d init_fifo->buffer_size : 0x%llx\n",__func__,__LINE__,init_fifo->buffer_size);
    init_fifo->item_total = init_fifo->buffer_size / init_fifo->item_size;

    mlist = 0;

    for (sender_id = MBOX_CORE_RPU0; sender_id < MBOX_CORE_MAX; sender_id++)
        for (receiver_id = MBOX_CORE_RPU0; receiver_id < MBOX_CORE_MAX; receiver_id++)
        {
        // Check for invalid communication paths and ignore such combinations
            if  (sender_id == receiver_id                                      ||

                (sender_id == MBOX_CORE_RPU0 && receiver_id == MBOX_CORE_RPU1) ||
				(sender_id == MBOX_CORE_RPU0 && receiver_id == MBOX_CORE_RPU2) ||
				(sender_id == MBOX_CORE_RPU0 && receiver_id == MBOX_CORE_RPU3) ||

                (sender_id == MBOX_CORE_RPU1 && receiver_id == MBOX_CORE_RPU0) ||
				(sender_id == MBOX_CORE_RPU1 && receiver_id == MBOX_CORE_RPU2) ||
				(sender_id == MBOX_CORE_RPU1 && receiver_id == MBOX_CORE_RPU3) ||

                (sender_id == MBOX_CORE_RPU2 && receiver_id == MBOX_CORE_RPU0) ||
				(sender_id == MBOX_CORE_RPU2 && receiver_id == MBOX_CORE_RPU1) ||
				(sender_id == MBOX_CORE_RPU2 && receiver_id == MBOX_CORE_RPU3) ||

                (sender_id == MBOX_CORE_RPU3 && receiver_id == MBOX_CORE_RPU0) ||
				(sender_id == MBOX_CORE_RPU3 && receiver_id == MBOX_CORE_RPU1) ||
				(sender_id == MBOX_CORE_RPU3 && receiver_id == MBOX_CORE_RPU2)

				)
            {
                continue;
            }

			if (sender_id == core_id || receiver_id == core_id)
            {
                g_mem_list[sender_id ][receiver_id] = mlist;
                (mbox_fifo + mlist)->core_id = core_id;
                (mbox_fifo + mlist)->buffer_address_phy =
                    buffer_addr_phy + ((sizeof(FifoControl) ) ); //Storing phy addr here as RPU will read this adrress from register
                (mbox_fifo + mlist)->buffer_address_virt =
                    buffer_addr_virt + ((sizeof(FifoControl) ) ); //Storing phy addr here as RPU will read this adrress from register
                (mbox_fifo + mlist)->sender_id   = sender_id ;
                (mbox_fifo + mlist)->receiver_id = receiver_id;
                (mbox_fifo + mlist)->fifo =
                    (FifoControl *)(uint64_t)(buffer_addr_virt); //buffer_addr == 0 for first time

               pr_err(" buffer addr sender_id -%d,receiver_id-%d, buffadd %llx\n",sender_id ,receiver_id, (mbox_fifo + mlist)->buffer_address_phy);
                init_fifo->buffer_addr_virt = (mbox_fifo + mlist)->buffer_address_virt;
                init_fifo->buffer_addr_phy = (mbox_fifo + mlist)->buffer_address_phy;
		pr_err("%s-%d\n",__func__,__LINE__);
                ret = fifo_init((mbox_fifo + mlist)->fifo, init_fifo);
  //              fifo_info((mbox_fifo + mlist)->fifo);
                if (ret != VPI_SUCCESS)
                    return ret;
                mlist++;
            }
            buffer_addr_virt += shm_block_size;
	    buffer_addr_phy += shm_block_size;
        }

    kfree(init_fifo);

    return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(mbox_mem_map);



#if 0
int mbox_mem_map(MboxFifoCtrl *mbox_fifo, MboxCoreId core_id,
                        uint64_t shm_start_addr,uint64_t shm_start_addr_phy, uint32_t shm_block_size)
{
    MboxCoreId i, j;
    uint32_t mlist, ret;
    uint64_t buffer_addr_virt = shm_start_addr; //Virtual Address 
    uint64_t buffer_addr_phy = shm_start_addr_phy;
 
   FifoInitData *init_fifo = NULL;
	
    init_fifo = (FifoInitData *)kmalloc(sizeof(FifoInitData),GFP_KERNEL);

    pr_err("mbox_mem_map entering... coreid %d \n",core_id);
    if (NULL == init_fifo)
        return VPI_ERR_NOMEM;

    init_fifo->item_size = sizeof(MboxPostMsg);
    init_fifo->buffer_size = shm_block_size - ((sizeof(FifoControl) + 7) / 8 * 8);
//    pr_err("%s-%d init_fifo->buffer_size : 0x%llx\n",__func__,__LINE__,init_fifo->buffer_size);
    init_fifo->item_total = init_fifo->buffer_size / init_fifo->item_size;

    mlist = 0;
    for (i = 0; i < MBOX_CORE_MAX; i++)
        for (j = 0; j < MBOX_CORE_MAX; j++) {
//		pr_err("AJAY::%s-%d i=%d j=%d mlist=%dbuffer_addr=0x%llxshm_block_size=0x%llxsizeof(FifoControl):%d \n",__func__,__LINE__,i,j,mlist,buffer_addr_virt,shm_block_size,sizeof(FifoControl));
            if (i == j) /*self to self is not need*/
                continue;
            if (i == MBOX_CORE_RPU0 && j == i + 1) /*RPU0 to RPU1 is not need*/
                continue;
            if (i == MBOX_CORE_RPU1 && j == i - 1) /*RPU1 to RPU0 is not need*/
                continue;

            if (i == core_id || j == core_id) {
                g_mem_list[i][j] = mlist;
                (mbox_fifo + mlist)->core_id = core_id;
                (mbox_fifo + mlist)->buffer_address_phy =
                    buffer_addr_phy + ((sizeof(FifoControl) ) ); //Storing phy addr here as RPU will read this adrress from register
                (mbox_fifo + mlist)->buffer_address_virt =
                    buffer_addr_virt + ((sizeof(FifoControl) ) ); //Storing phy addr here as RPU will read this adrress from register
//pr_err("%s-%d mlist : %d buffer_addr_virt : 0x%x mlist : 0x%llx size of FIFO_CTL : 0x%llx  \n",__func__,__LINE__,mlist,(buffer_addr_virt + ((sizeof(FifoControl)))),(mbox_fifo + mlist)->buffer_address_virt,sizeof(FifoControl));
                (mbox_fifo + mlist)->sender_id   = i;
                (mbox_fifo + mlist)->receiver_id = j;
                (mbox_fifo + mlist)->fifo =
                    (FifoControl *)(uint64_t)(buffer_addr_virt); //buffer_addr == 0 for first time

                pr_err(" buffer addr i-%d,j-%d, buffadd %llx\n",i,j, (mbox_fifo + mlist)->buffer_address_phy);

                init_fifo->buffer_addr_virt = (mbox_fifo + mlist)->buffer_address_virt;
                init_fifo->buffer_addr_phy = (mbox_fifo + mlist)->buffer_address_phy;
		pr_err("%s-%d\n",__func__,__LINE__);
                ret = fifo_init((mbox_fifo + mlist)->fifo, init_fifo);
  //              fifo_info((mbox_fifo + mlist)->fifo);
                if (ret != VPI_SUCCESS)
                    return ret;
                mlist++;
            }
            buffer_addr_virt += shm_block_size;
	    buffer_addr_phy += shm_block_size;
        }

    kfree(init_fifo);

    return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(mbox_mem_map);
#endif
MboxFifoCtrl *vpi_mbox_init(MboxCoreId core_id, uint64_t shm_addr,uint64_t shm_addr_phy,
                            uint64_t shm_block_size)
{
    MboxFifoCtrl *mbox_fifo = NULL;

//    pr_err("%s-%d Virtual Address shm_addr : 0x%llx\n",__func__,__LINE__,shm_addr);
  //  pr_err("vpi_mbox_init coreid -> %d \n",core_id);
    if (core_id < 0 || core_id >= MBOX_CORE_MAX)
        return NULL;

    mbox_fifo = (MboxFifoCtrl *)kmalloc(sizeof(MboxFifoCtrl) *
                                           ((MBOX_CORE_MAX - 1) * 2),GFP_KERNEL);

    //pr_err("%x <- mbox_fifo \n",mbox_fifo);
    if (NULL == mbox_fifo)
        return NULL;
    //pr_err("%s-%d shm_addr=%llx,shm_addr_phy=%llx",shm_addr,shm_addr_phy);	
    if (VPI_SUCCESS !=
        mbox_mem_map(mbox_fifo, core_id, shm_addr,shm_addr_phy, shm_block_size))
    {

    	kfree(mbox_fifo);

    	return NULL;
    }

    return mbox_fifo;
}
EXPORT_SYMBOL_GPL(vpi_mbox_init);



static inline int mbox_fifoctrl_init_check(MboxFifoCtrl *mbox_fifo)
{
    return (mbox_fifo == NULL) ? VPI_ERR_UNINITED : VPI_SUCCESS;
}

int mbox_core_id_check(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
                              MboxCoreId receiver_id);
int mbox_core_id_check(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
                              MboxCoreId receiver_id)
{
    if (sender_id == receiver_id)
        return VPI_ERR_INVALID;
    if (sender_id < 0 || sender_id >= MBOX_CORE_MAX || receiver_id < 0 ||
        receiver_id >= MBOX_CORE_MAX)
        return VPI_ERR_INVALID;
    if (sender_id != mbox_fifo->core_id && receiver_id != mbox_fifo->core_id)
        return VPI_ERR_INVALID;
    return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(mbox_core_id_check);

int vpi_mbox_post(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg, MboxCoreId receiver_id,
                  MboxDriverCb mbox_driver_cb)
{
    uint16_t ret;
 //   MboxPostMsg msg1;
   // MboxFifoCtrl mbox_fifo1;

    if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
        return VPI_ERR_UNINITED;
    if (NULL == msg)
        return VPI_ERR_INVALID;
    if (VPI_ERR_INVALID ==
        mbox_core_id_check(mbox_fifo, mbox_fifo->core_id, receiver_id))
        return VPI_ERR_INVALID;

	
    ret = fifo_write(msg,
                    (mbox_fifo + g_mem_list[mbox_fifo->core_id][receiver_id])
                        ->fifo);
	
    if (ret != VPI_SUCCESS)
        return ret;
    if (!mbox_driver_cb)
        return VPI_ERR_INVALID;
    mbox_driver_cb(receiver_id);
    return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(vpi_mbox_post);

int vpi_mbox_broadcast(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg,
                       MboxDriverCb mbox_driver_cb)
{
    MboxCoreId recv_core;
    uint16_t ret;

    if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
        return VPI_ERR_UNINITED;
    if (msg == NULL)
        return VPI_ERR_INVALID;

    for (recv_core = 0; recv_core < MBOX_CORE_MAX; recv_core++)
        if (mbox_fifo->core_id != recv_core) {
            ret = vpi_mbox_post(mbox_fifo, msg, recv_core, mbox_driver_cb);
            if (ret != VPI_SUCCESS)
                return ret;
        }
    return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(vpi_mbox_broadcast);

int vpi_mbox_read(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg, MboxCoreId sender_id)
{
    if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
        return VPI_ERR_UNINITED;
    if (msg == NULL)
        return VPI_ERR_INVALID;
    if (VPI_ERR_INVALID ==
        mbox_core_id_check(mbox_fifo, sender_id, mbox_fifo->core_id))
        return VPI_ERR_INVALID;

    return fifo_read(msg,
    (mbox_fifo + g_mem_list[sender_id][mbox_fifo->core_id])
        ->fifo);
}
EXPORT_SYMBOL_GPL(vpi_mbox_read);
int vpi_mbox_reset(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
                   MboxCoreId receiver_id)
{
    if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
        return VPI_ERR_UNINITED;
    if (VPI_ERR_INVALID ==
        mbox_core_id_check(mbox_fifo, sender_id, receiver_id))
        return VPI_ERR_INVALID;

    return fifo_reset((mbox_fifo + g_mem_list[sender_id][receiver_id])->fifo);
}
EXPORT_SYMBOL_GPL(vpi_mbox_reset);

uint32_t vpi_mbox_get_stored(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
                             MboxCoreId receiver_id)
{
    if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
        return VPI_ERR_UNINITED;
    if (VPI_ERR_INVALID ==
        mbox_core_id_check(mbox_fifo, sender_id, receiver_id))
        return VPI_ERR_INVALID;

    return fifo_get_stored(
        (mbox_fifo + g_mem_list[sender_id][receiver_id])->fifo);
}
EXPORT_SYMBOL_GPL(vpi_mbox_get_stored);

bool vpi_mbox_is_full(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
                      MboxCoreId receiver_id)
{
    return fifo_is_full((mbox_fifo + g_mem_list[sender_id][receiver_id])->fifo);
}
EXPORT_SYMBOL_GPL(vpi_mbox_is_full);

bool vpi_mbox_is_empty(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
                       MboxCoreId receiver_id)
{
    return fifo_is_empty(
        (mbox_fifo + g_mem_list[sender_id][receiver_id])->fifo);
}
EXPORT_SYMBOL_GPL(vpi_mbox_is_empty);

void vpi_mbox_destroy(MboxFifoCtrl *mbox_fifo);
void vpi_mbox_destroy(MboxFifoCtrl *mbox_fifo)
{
    uint16_t i;
    for (i = 0; i < (MBOX_CORE_MAX - 1) * 2; i++) {
        kfree((mbox_fifo + i)->fifo);
    }
}
EXPORT_SYMBOL_GPL(vpi_mbox_destroy);

MODULE_AUTHOR ("anandam");
MODULE_DESCRIPTION ("mbox_fifo");
MODULE_LICENSE ("GPL");

