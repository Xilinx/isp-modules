/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/*Copy from cam_device.cpp*/

#include "cam_device_buffer_api.h"
#include "cam_device.h"
#include "cam_device_api.h"
//#include "mailbox.h"
#include "sensor_cmd.h"
#include "kmbox.h"
#include "vvcam_isp_driver.h"
extern uint32_t cookie;

RESULT VsiCamDeviceInitBufChain
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t            hCamDevice,
    CamDeviceBufChainId_t        bufId,
    CamDeviceBufChainConfig_t   *pConfig
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx || NULL == pConfig) {
        return RET_NULL_POINTER;
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
		return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
    packet->payload_size += sizeof(CamDeviceBufChainId_t);
    p_data += sizeof(CamDeviceBufChainId_t);
    memcpy(p_data, pConfig, sizeof(CamDeviceBufChainConfig_t));
    packet->payload_size += sizeof(CamDeviceBufChainConfig_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_INIT_BUF_CHAIN, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(RET_SUCCESS != result) {
        os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif

#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command( APU_2_RPU_MB_CMD_INIT_BUF_CHAIN , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack();
    kfree(packet);
	return result;
}


RESULT VsiCamDeviceDeInitBufChain
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t       hCamDevice,
    CamDeviceBufChainId_t   bufId
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
		return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
    packet->payload_size += sizeof(CamDeviceBufChainId_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_DEINIT_BUF_CHAIN, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(RET_SUCCESS != result) {
        os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif


#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command(APU_2_RPU_MB_CMD_DEINIT_BUF_CHAIN, packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack();

    kfree(packet);

    return result;
}


RESULT VsiCamDeviceCreateBufPool
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t           hCamDevice,
	CamDeviceBufChainId_t       bufId,
	CamDeviceBufPoolConfig_t  *hBufferPoolCfg
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx || NULL == hBufferPoolCfg) {
        return RET_NULL_POINTER;
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
		return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    // to solve 64-bit space -> 32-bit space problem, we can not directly copy 	CamDeviceBufPoolSetupCfg_t

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
    packet->payload_size += sizeof(CamDeviceBufChainId_t);
    p_data += sizeof(CamDeviceBufChainId_t);

//    memcpy(p_data, &(hBufferPoolSetupCfg->bufIo), sizeof(CamDeviceBufChainId_t));
//    packet.payload_size += sizeof(CamDeviceBufChainId_t);
//    p_data += sizeof(CamDeviceBufChainId_t);
//
//    memcpy(p_data, &(hBufferPoolSetupCfg->bufferChain), sizeof(CamDeviceBufChainConfig_t));
//	packet.payload_size += sizeof(CamDeviceBufChainConfig_t);
//	p_data += sizeof(CamDeviceBufChainConfig_t);

    memcpy(p_data, &(hBufferPoolCfg->bufMode), sizeof(CamDeviceBufMode_t));
	packet->payload_size += sizeof(CamDeviceBufMode_t);
	p_data += sizeof(CamDeviceBufMode_t);

    memcpy(p_data, &(hBufferPoolCfg->bufNum), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

    memcpy(p_data, &(hBufferPoolCfg->bufSize), sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);

	// 6 physical address
    memcpy(p_data, hBufferPoolCfg->pBaseAddrList, hBufferPoolCfg->bufNum * sizeof(uint32_t));
	packet->payload_size += hBufferPoolCfg->bufNum * sizeof(uint32_t);
	p_data += hBufferPoolCfg->bufNum * sizeof(uint32_t);

    memcpy(p_data, &(hBufferPoolCfg->is_mapped), sizeof(bool_t));
	packet->payload_size += sizeof(bool_t);
	p_data += sizeof(bool_t);

    memcpy(p_data, hBufferPoolCfg->pIplAddrList, hBufferPoolCfg->bufNum * sizeof(uint32_t));
	packet->payload_size += hBufferPoolCfg->bufNum * sizeof(uint32_t);
	p_data += hBufferPoolCfg->bufNum * sizeof(uint32_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0 
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_CREATE_BUFFER_POOL, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(RET_SUCCESS != result) {
        os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif


#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command( APU_2_RPU_MB_CMD_CREATE_BUFFER_POOL , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack();

    kfree(packet);

    return result;
}



RESULT VsiCamDeviceDestroyBufPool
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t       hCamDevice,
    CamDeviceBufChainId_t   bufId
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
		return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
    packet->payload_size += sizeof(CamDeviceBufChainId_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0 
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_DESTORY_BUFFER_POOL, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(RET_SUCCESS != result) {
        os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif


#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command( APU_2_RPU_MB_CMD_DESTORY_BUFFER_POOL, packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    	kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack();

    kfree(packet);

    return result;
}


RESULT VsiCamDeviceSetupBufMgmt
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t       hCamDevice,
    CamDeviceBufChainId_t   bufId
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
		return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
    packet->payload_size += sizeof(CamDeviceBufChainId_t);
#if 0
	//RANJITH REMOVE BELOW CODE
	*(packet->payload) =(int *)0x0;
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
#endif


    if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SETUP_BUF_MGMT, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(RET_SUCCESS != result) {
        os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif


#if 1

    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command( APU_2_RPU_MB_CMD_SETUP_BUF_MGMT, packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    xlnx_mbox_apu_wait_for_ack();

    kfree(packet);

    return result;
}


/*****************************************************************************/
/**
 * @brief   This function releases buffer management.
 *
 * @param   hCamDevice          Handle to the CamDevice instance
 *
 * @retval  RET_SUCCESS         Operation succeeded
 *
 *****************************************************************************/
RESULT VsiCamDeviceReleaseBufMgmt
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t       hCamDevice,
    CamDeviceBufChainId_t   bufId
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
		return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
    packet->payload_size += sizeof(CamDeviceBufChainId_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_RELEASE_BUF_MGMT, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(RET_SUCCESS != result) {
        os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif

#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command( APU_2_RPU_MB_CMD_RELEASE_BUF_MGMT , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif


    xlnx_mbox_apu_wait_for_ack();

    kfree(packet);

    return result;
}

int DRV_DQ_CNT=0;
RESULT VsiCamDeviceDeQueBuffer
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t         hCamDevice,
    CamDeviceBufChainId_t     bufId,
    MediaBuffer_t           **pMediaBuf
)
{
    RESULT result = RET_SUCCESS;

     DRV_DQ_CNT++;

    *pMediaBuf = kmalloc(sizeof(MediaBuffer_t),GFP_KERNEL);
    if(!pMediaBuf)
    {
		return -ENOMEM;
    } 
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx || NULL == *pMediaBuf) {
        return RET_NULL_POINTER;
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
		return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    (*pMediaBuf)->pMetaData = (PicBufMetaData_t*)kmalloc(sizeof(PicBufMetaData_t),GFP_KERNEL);
    memset((*pMediaBuf)->pMetaData, 0, sizeof(PicBufMetaData_t));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
    packet->payload_size += sizeof(CamDeviceBufChainId_t);
    p_data += sizeof(CamDeviceBufChainId_t);

    memcpy(p_data, (*pMediaBuf)->pMetaData, sizeof(PicBufMetaData_t));
    packet->payload_size += sizeof(PicBufMetaData_t);
    p_data += sizeof(PicBufMetaData_t);

    memcpy(p_data, &((*pMediaBuf)->baseAddress), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &((*pMediaBuf)->baseSize), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &((*pMediaBuf)->lockCount), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &((*pMediaBuf)->isFull), sizeof(bool_t));
    packet->payload_size += sizeof(bool_t);
    p_data += sizeof(bool_t);

    memcpy(p_data, &((*pMediaBuf)->index), sizeof(uint8_t));
    packet->payload_size += sizeof(uint8_t);
    p_data += sizeof(uint8_t);

    memcpy(p_data, &((*pMediaBuf)->bufMode), sizeof(BUFF_MODE));
    packet->payload_size += sizeof(BUFF_MODE);
    p_data += sizeof(BUFF_MODE);

    memcpy(p_data, &((*pMediaBuf)->pIplAddress), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &((*pMediaBuf)->pOwner), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_DEQUE_BUFFER, packet, packet->payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(RET_SUCCESS != result) {
        os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_mb_data(packet->cookie, packet->payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }
	if (result) {
    	os_unlock_mutex(&mbox_lock);
	return result;
	}

    os_unlock_mutex(&mbox_lock);
#endif


#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command(APU_2_RPU_MB_CMD_DEQUE_BUFFER , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif
	uint8_t __result = xlnx_mbox_apu_wait_for_data(packet->payload);
    if(__result)
    {
		kfree(packet);
        return 1;
    }

	p_data = packet->payload;
	p_data += (sizeof(uint32_t) + sizeof(CamDeviceBufChainId_t));


	memcpy( (*pMediaBuf)->pMetaData, p_data, sizeof(PicBufMetaData_t));
	p_data += sizeof(PicBufMetaData_t);

	memcpy( &((*pMediaBuf)->baseAddress), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&((*pMediaBuf)->baseSize), p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy(&((*pMediaBuf)->lockCount),p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);

	memcpy( &((*pMediaBuf)->isFull),p_data, sizeof(bool_t));
	p_data += sizeof(bool_t);

	memcpy(&((*pMediaBuf)->index), p_data, sizeof(uint8_t));
	p_data += sizeof(uint8_t);

	memcpy(&((*pMediaBuf)->bufMode), p_data, sizeof(BUFF_MODE));
    p_data += sizeof(BUFF_MODE);

    memcpy(&((*pMediaBuf)->pIplAddress), p_data, sizeof(uint32_t));
    p_data += sizeof(uint32_t);

    memcpy(&((*pMediaBuf)->pOwner), p_data, sizeof(uint32_t));
	kfree(packet);

    return result;
}

int DRV_ENQ_CNT=0;
RESULT VsiCamDeviceEnQueBuffer
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t       hCamDevice,
    CamDeviceBufChainId_t   bufId,
    MediaBuffer_t          *pMediaBuf
)
{
    RESULT result = RET_SUCCESS;
 DRV_ENQ_CNT++;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx ) {
        return RET_NULL_POINTER;
    }

    if ( NULL == pMediaBuf) {
        return RET_NULL_POINTER;
    }

    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
		return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
    packet->payload_size += sizeof(CamDeviceBufChainId_t);
    p_data += sizeof(CamDeviceBufChainId_t);

    if(pMediaBuf->pMetaData) {
    	kfree(pMediaBuf->pMetaData);
    }

    memcpy(p_data, &(pMediaBuf->baseAddress), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &(pMediaBuf->baseSize), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &(pMediaBuf->lockCount), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &(pMediaBuf->isFull), sizeof(bool_t));
    packet->payload_size += sizeof(bool_t);
    p_data += sizeof(bool_t);

    memcpy(p_data, &(pMediaBuf->index), sizeof(uint8_t));
    packet->payload_size += sizeof(uint8_t);
    p_data += sizeof(uint8_t);

    memcpy(p_data, &(pMediaBuf->bufMode), sizeof(BUFF_MODE));
    packet->payload_size += sizeof(BUFF_MODE);
    p_data += sizeof(BUFF_MODE);

//0x12345678
  /*  //p_data = (uint8_t*)(&pMediaBuf->pOwner);
    *p_data = (uint8_t *)((pMediaBuf->pOwner)>>0);
    p_data += sizeof(uint8_t);
    *p_data = (uint8_t *)((pMediaBuf->pOwner)>>8);
    p_data += sizeof(uint8_t);
    *p_data = (uint8_t *)((pMediaBuf->pOwner)>>16);
    p_data += sizeof(uint8_t);
    *p_data = (uint8_t *)((pMediaBuf->pOwner)>>24);
    p_data += sizeof(uint8_t);
*/
    memcpy(p_data, &((pMediaBuf)->pIplAddress), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

    memcpy(p_data, &((pMediaBuf)->pOwner), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
// RANJITH THE BELOW CODE is ADDED due to some memcpy issue
    memcpy(p_data, &((pMediaBuf)->pIplAddress), sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);

//    kfree(pMediaBuf);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command(APU_2_RPU_MB_CMD_ENQUE_BUFFER , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

    kfree(pMediaBuf);  // RANJITH ADD IT LATER 
#endif
    xlnx_mbox_apu_wait_for_ack();

	kfree(packet);

    return result;
}

RESULT VsiCamDeviceGetBufferSize
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t        hCamDevice,
    CamDeviceBufChainId_t    bufId,
    uint32_t                *pBufSize
)
{
    if (NULL == hCamDevice || NULL == pBufSize) {
        return RET_NULL_POINTER;
    }
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if(!packet)
    {
		return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &bufId, sizeof(CamDeviceBufChainId_t));
    packet->payload_size += sizeof(CamDeviceBufChainId_t);
    p_data += sizeof(CamDeviceBufChainId_t);
    memcpy(p_data, pBufSize, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_GET_BUFFER_SIZE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(RET_SUCCESS != result) {
        os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
    result = apu_wait_for_mb_data(packet.cookie, packet.payload);
    if (result) {
	    os_unlock_mutex(&mbox_lock);
	    return RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif


#if 1
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command(  APU_2_RPU_MB_CMD_GET_BUFFER_SIZE ,  packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif

    uint8_t  __result = xlnx_mbox_apu_wait_for_data(packet->payload);
    if(__result)
    {
        return -1;
    }
    memcpy(pBufSize, p_data, sizeof(uint32_t));

    kfree(packet);
    return result;
}




