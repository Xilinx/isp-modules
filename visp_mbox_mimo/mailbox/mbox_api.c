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

    if (NULL == init_fifo)
        return VPI_ERR_NOMEM;

    init_fifo->item_size = sizeof(MboxPostMsg);
    init_fifo->buffer_size = shm_block_size - ((sizeof(FifoControl) + 7) / 8 * 8);
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

               pr_debug(" buffer addr sender_id -%d,receiver_id-%d, buffadd %llx\n",sender_id ,receiver_id, (mbox_fifo + mlist)->buffer_address_phy);
                init_fifo->buffer_addr_virt = (mbox_fifo + mlist)->buffer_address_virt;
                init_fifo->buffer_addr_phy = (mbox_fifo + mlist)->buffer_address_phy;
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


MboxFifoCtrl *vpi_mbox_init(MboxCoreId core_id, uint64_t shm_addr,uint64_t shm_addr_phy,
                            uint64_t shm_block_size)
{
    MboxFifoCtrl *mbox_fifo = NULL;

    if (core_id < 0 || core_id >= MBOX_CORE_MAX)
        return NULL;

    mbox_fifo = (MboxFifoCtrl *)kmalloc(sizeof(MboxFifoCtrl) *
                                           ((MBOX_CORE_MAX - 1) * 2),GFP_KERNEL);

    if (NULL == mbox_fifo)
        return NULL;
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

    if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
    {
        return VPI_ERR_UNINITED;
    }

    if (NULL == msg)
    {
        return VPI_ERR_INVALID;
    }
    if (VPI_ERR_INVALID ==
        mbox_core_id_check(mbox_fifo, mbox_fifo->core_id, receiver_id))
        {
        return VPI_ERR_INVALID;
        }

	
    ret = fifo_write(msg,
                    (mbox_fifo + g_mem_list[mbox_fifo->core_id][receiver_id])
                        ->fifo);
	
    if (ret != VPI_SUCCESS)
    {
        return ret;
    }
    if (!mbox_driver_cb)
    {
        return VPI_ERR_INVALID;
    }
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

