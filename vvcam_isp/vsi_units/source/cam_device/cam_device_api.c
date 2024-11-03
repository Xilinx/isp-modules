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

#include "cam_device.h"
#include "cam_device_api.h"
//#include "mailbox.h"
#include "sensor_cmd.h"
#include <linux/delay.h>
#include "vvcam_isp_driver.h"
//#include <linux/xlnx_isp_def.h>
#include <linux/string.h>
#include "kmbox.h"
//#define __section_t(S)          __attribute__((__section__(#S)))
//#define __cam_load_calib        __section_t(.camdevice_load_calib)
//
//uint8_t  cam_load_calib[160010] __attribute__((section(".cam_load_calib")));
uint8_t  *cam_load_calib = NULL; //[160010] __attribute__((section(".cam_load_calib")));

void dummy_isp(void){

    printk(KERN_ERR "RKC-DRV %s %d\n",__func__,__LINE__);
	return ;
}



RESULT VsiCamDeviceCreate
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceConfig_t *pCamConfig,
    CamDeviceHandle_t *pHandleCamDevice
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;
    if (NULL == pCamConfig || NULL == pHandleCamDevice) {
    	printk(KERN_ERR "RKC-DRV %s %d\n",__func__,__LINE__);
        return RET_NULL_POINTER;
    }
    uint32_t hwId = pCamConfig->ispHwId;

    /*
     * AMD : Multi-Core Changes
     * Verifying Multi-core
     */
    // // xil_printf("%s %d camConfig.ispHwId=%x \n\r",__func__,__LINE__,hwId);

#ifdef DEBUG_FLAG
#else
    if (hwId >= CAMDEV_HARDWARE_ID_MAX) {
        return RET_OUTOFRANGE;
    }
#endif

    CamDeviceIspcoreInit();
    /*Get Cam Device Handle*/
    uint32_t virtualId = 0;
    CamDeviceHandle_t hCamDevice = NULL;
    result = CamDeviceRequestInstance(hwId, &hCamDevice, &virtualId);
    if(RET_SUCCESS != result || NULL == hCamDevice) {
        return RET_FAILURE;
    }
    memset(hCamDevice, 0, sizeof(CamDeviceContext_t));

    /*Mapping instance id for a single device*/
    uint32_t instanceId = 0;
    result = CamDeviceInstanceIdMapping(hwId, virtualId, &instanceId);
    if(RET_SUCCESS != result) {
        return RET_UNSUPPORT_ID;
    }
   // RANJITH ADDED TEST DATA
//	hwId=0xA1,virtualId=0xB2,instanceId=0xEC;
//	  pCamConfig->ispHwId=0xA1;;
//pCamConfig->inputCfg.inputType=0xB2;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    pCamDevCtx->ispHwId =hwId;
    pCamDevCtx->ispVtId =virtualId;
    pCamDevCtx->instanceId = instanceId;
    pCamDevCtx->cookie = 0;
    payload_packet *packet;
    packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
if(!packet)
{
printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
return -ENOMEM;
} 
    memset(packet, 0x0, sizeof(payload_packet));

    packet->cookie = 0;//pCamDevCtx->cookie; //RANJITH UNDO
    packet->type = CMD;
    packet->payload_size = 0;

    //*(packet->payload) = (int *)0xDEADBEEF;
    uint8_t *p_data = packet->payload;
    #if 1
    memcpy(p_data, &instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pCamConfig, sizeof(CamDeviceConfig_t));
    packet->payload_size += sizeof(CamDeviceConfig_t);
//    *(p_data) = (int *)0xBADBABE;
//    p_data += sizeof(uint32_t);

#ifdef DEBUG_FLAG
    // // xil_printf("APU create cam device payload size:%d.\r\n", packet.payload_size);
	// // xil_printf("0=%x 0=%x \r\n", 0,0);
#endif
#endif
	if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
    
    CamDeviceConfig_t *temp =(CamDeviceConfig_t *)p_data; 
    //printk(KERN_ERR "%d %d %x %p\n", packet->cookie ,  packet->cookie, packet->cookie, packet->cookie   ); 

    printk(KERN_ERR "AUG 16 %d %d %d %d \n",temp->ispHwId,temp->inputCfg.inputType,temp->workCfg.workMode,temp->outputCfg.outputType);

#if 1
    mutex_lock(&isp_dev->mlock);
    ret = Send_Command (APU_2_RPU_MB_CMD_CREATE_INSTANCE,packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    //ret = Send_Command (APU_2_RPU_MB_CMD_SET_PATH_STREAMING  ,packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
        kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);
#endif

    *pHandleCamDevice = hCamDevice;

//#ifdef DEBUG_FLAG
//    // xil_printf("APU create cam device waiting for return.\r\n");
//#endif
#if 0
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif
#if 1
    printk(KERN_ERR "RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
    //int  mbox_send_ret = xlnx_mailbox_send_msg_client(isp_dev->tx_chan,packet,APU_2_RPU_MB_CMD_CREATE_INSTANCE);
    printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);
#endif
//    *pHandleCamDevice = hCamDevice;

    printk(KERN_ERR "%s %d cookie = %d VtID = %d  HWID=%d instANCE_ID = %d \n",__func__,__LINE__,pCamDevCtx->cookie,pCamDevCtx->ispVtId,pCamDevCtx->ispHwId,pCamDevCtx->instanceId);
  //mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_CREATE_INSTANCE);
	//printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);
#ifdef DEBUG_FLAG
     // xil_printf("APU create cam device successfully return.\r\n");
#endif

    printk(KERN_ERR "Waiting for ACK bypass\n");
   xlnx_mbox_apu_wait_for_ack();
    printk(KERN_ERR "ACK RECIVEDC\n");
printk(KERN_ERR "Waiting at while1\n");
    kfree(packet);
	return result;
}

RESULT VsiCamDeviceDestroy
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceConfig_t *pCamConfig,
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
packet= kmalloc(sizeof(payload_packet), GFP_KERNEL);
if(!packet)
{
printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
return -ENOMEM;
} 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

	    if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_DESTORY, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
		os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }

    result = CamDeviceFreeInstance(hCamDevice, pCamDevCtx->ispHwId);
    if (result != RET_SUCCESS) {
        return result;
    }

    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);
#endif
//
#if 1
    //os_lock_mutex(&mbox_lock);
    printk(KERN_ERR "RANJITJHI BEOFR ELOCK \n");
    mutex_lock(&isp_dev->mlock);
	ret = Send_Command (APU_2_RPU_MB_CMD_DESTORY ,packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
  //ret = Send_Command (APU_2_RPU_MB_CMD_CREATE_INSTANCE, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
//		os_unlock_mutex(&mbox_lock);
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
	printk(KERN_ERR "RANJITJHI AFTER ELOCK \n");
	ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif

//



//	printk(KERN_ERR "RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
  //  int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_DESTORY);
//	printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);

    xlnx_mbox_apu_wait_for_ack();

    kfree(packet);
	return result;
}

RESULT VsiCamDeviceSetOutFormat
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeOutPathType_t path,
    CamDevicePipeOutFmt_t *pFmt
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;
	printk(KERN_ERR "RKC-ISPDRV_APP %s %d ",__func__,__LINE__);

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
	if (pCamDevCtx==NULL || pFmt==NULL)
	{
		printk(KERN_ERR "RKC-ISP_APP %s %d  \n",__func__,__LINE__);
		return 0;
	}
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pFmt) {
        return (RET_NULL_POINTER);
    }
#ifdef DEBUG_FLAG
	// xil_printf("APU set out format :%d.\r\n");
    // xil_printf("out width: %d.\r\n", pFmt->outWidth);
    // xil_printf("databits: %d.\r\n", pFmt->dataBits);
    // xil_printf("outfmt: %d.\r\n", pFmt->outFormat);
#endif
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet = (payload_packet *)kmalloc(sizeof(payload_packet),GFP_KERNEL);
if(!packet)
{
printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
return -ENOMEM;
} 
    memset(packet, 0, sizeof(payload_packet));

	printk(KERN_ERR "RKC-ISPDRV_APP %s %d ",__func__,__LINE__);
    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &path, sizeof(CamDevicePipeOutPathType_t));
    packet->payload_size += sizeof(CamDevicePipeOutPathType_t);
    p_data += sizeof(CamDevicePipeOutPathType_t);
    memcpy(p_data, pFmt, sizeof(CamDevicePipeOutFmt_t));
    packet->payload_size += sizeof(CamDevicePipeOutFmt_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_SET_OUT_FORMAT, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
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
    ret = Send_Command ( APU_2_RPU_MB_CMD_SET_OUT_FORMAT , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif



//	printk(KERN_ERR "RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
//	int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_SET_OUT_FORMAT);
//	printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);

    xlnx_mbox_apu_wait_for_ack();

	kfree(packet);
	return result;
}
#if 0
RESULT VsiCamDeviceSetInFormat
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeInPathType_t path,
    CamDevicePipeInFmt_t *pFmt
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pFmt) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &path, sizeof(CamDevicePipeInPathType_t));
    p_data += sizeof(CamDevicePipeInPathType_t);
    packet.payload_size += sizeof(CamDevicePipeInPathType_t);
    memcpy(p_data, pFmt, sizeof(CamDevicePipeInFmt_t));
    packet.payload_size += sizeof(CamDevicePipeInFmt_t);

    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SET_IN_FORMAT, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(RET_SUCCESS != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}
#endif
RESULT VsiCamDeviceGetOutFormat
(
    CamDeviceConfig_t *pCamConfig,
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeOutPathType_t path,
    CamDevicePipeOutFmt_t *pFmt
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }

    (pCamDevCtx->cookie)++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;
#if 0
	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &path, sizeof(CamDevicePipeOutPathType_t));
	packet.payload_size += sizeof(CamDevicePipeOutPathType_t);
	p_data += sizeof(CamDevicePipeOutPathType_t);
	memcpy(p_data, pFmt, sizeof(CamDevicePipeOutFmt_t));
	packet.payload_size += sizeof(CamDevicePipeOutFmt_t);

	memcpy(pFmt, p_data, sizeof(CamDevicePipeOutFmt_t));
#endif
    return RET_SUCCESS;
}
#if 0
RESULT VsiCamDeviceGetInFormat
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeInPathType_t path,
    CamDevicePipeInFmt_t *pFmt
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    RESULT result = RET_SUCCESS;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }

    (pCamDevCtx->cookie)++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &path, sizeof(CamDevicePipeInPathType_t));
	packet.payload_size += sizeof(CamDevicePipeInPathType_t);
	p_data += sizeof(CamDevicePipeInPathType_t);
	memcpy(p_data, pFmt, sizeof(CamDevicePipeInFmt_t));
	packet.payload_size += sizeof(CamDevicePipeInFmt_t);

	    if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_GET_IN_FORMAT, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pFmt, p_data, sizeof(CamDevicePipeInFmt_t));

#ifdef DEBUG_FLAG
	// xil_printf("APU get in format results:%d.\r\n");
    // xil_printf("in width: %d.\r\n", pFmt->inWidth);
    // xil_printf("infmt: %d.\r\n", pFmt->inFormat);
#endif

    return RET_SUCCESS;
}

RESULT VsiCamDeviceSetIspWindow
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeOutPathType_t path,
    const CamDevicePipeIspWindow_t *pIspWindow
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    RESULT result = RET_SUCCESS;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pIspWindow) {
        return (RET_NULL_POINTER);
    }
#ifdef DEBUG_FLAG
	// xil_printf("APU set isp window :%d.\r\n");
    // xil_printf("cropWindow.hOffset: %d.\r\n", pIspWindow->cropWindow.hOffset);
    // xil_printf("cropWindow.height: %d.\r\n", pIspWindow->cropWindow.height);
#endif

    (pCamDevCtx->cookie)++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &path, sizeof(CamDevicePipeOutPathType_t));
	packet.payload_size += sizeof(CamDevicePipeOutPathType_t);
	p_data += sizeof(CamDevicePipeOutPathType_t);
	memcpy(p_data, pIspWindow, sizeof(CamDevicePipeIspWindow_t));
	packet.payload_size += sizeof(CamDevicePipeIspWindow_t);

	    if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SET_ISP_WINDOW, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(RET_SUCCESS != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}

	result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

    return RET_SUCCESS;
}

RESULT VsiCamDeviceGetIspWindow
(
    CamDeviceHandle_t hCamDevice,
    CamDevicePipeOutPathType_t path,
    CamDevicePipeIspWindow_t *pIspWindow
){
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    RESULT result = RET_SUCCESS;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }

    (pCamDevCtx->cookie)++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &path, sizeof(CamDevicePipeOutPathType_t));
	packet.payload_size += sizeof(CamDevicePipeOutPathType_t);
	p_data += sizeof(CamDevicePipeOutPathType_t);
	memcpy(p_data, pIspWindow, sizeof(CamDevicePipeIspWindow_t));
	packet.payload_size += sizeof(CamDevicePipeIspWindow_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_GET_ISP_WINDOW, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(pIspWindow, p_data, sizeof(CamDevicePipeIspWindow_t));

    return RET_SUCCESS;
}
#endif


RESULT VsiCamDeviceConnectCamera
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice,
    const CamDevicePipeSubmoduleCtrl_u *pSubCtrl
)
{
    RESULT result = RET_SUCCESS;

	printk(KERN_ERR "RKC-ISP_APP %s %d \n",__func__,__LINE__);
//    struct vvcam_isp_dev *isp_dev = pCamConfig->isp_dev;    
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;

	if (pCamDevCtx==NULL || pSubCtrl==NULL)
	{
		printk(KERN_ERR "RKC-ISP_APP NULL HANDLE %s %d  \n",__func__,__LINE__);
		while(1);
		return 0;
	}
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pSubCtrl) {
        return (RET_NULL_POINTER);
    }
#ifdef DEBUG_FLAG
	// xil_printf("APU connect camera :%d.\r\n");
    // xil_printf("aeEnable: %d.\r\n", pSubCtrl->subCtrl.aeEnable);
    // xil_printf("aeEnable: %d.\r\n", pSubCtrl->subCtrl.aeEnable);
    // xil_printf("aeEnable: %d.\r\n", pSubCtrl->subCtrl.aeEnable);
#endif
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet = (payload_packet *)kmalloc(sizeof(payload_packet),GFP_KERNEL);
if(!packet)
{
printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
return -ENOMEM;
} 
    memset(packet, 0, sizeof(payload_packet));

    printk(KERN_ERR "RKC-ISP_APP %s %d \n",__func__,__LINE__);
    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, pSubCtrl, sizeof(CamDevicePipeSubmoduleCtrl_u));
    packet->payload_size += sizeof(CamDevicePipeSubmoduleCtrl_u);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

#if 0
    os_lock_mutex(&mbox_lock);
    result = Send_Command (APU_2_RPU_MB_CMD_CONNECT_CAMERA, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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
    int ret = Send_Command(APU_2_RPU_MB_CMD_CONNECT_CAMERA , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif




//	printk(KERN_ERR "RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
  //  int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_CONNECT_CAMERA);
//	printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);

    xlnx_mbox_apu_wait_for_ack();//RANJITH CHECK THIS
	printk(KERN_ERR "RKC-DRV %s %d  Mailbox done\n",__func__,__LINE__);
//ssleep(5);
    kfree(packet);

	printk(KERN_ERR "RKC-DRV %s %d  Mailbox done\n",__func__,__LINE__);
	return result;
}

RESULT VsiCamDeviceDisconnectCamera
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet = (payload_packet *)kmalloc(sizeof(payload_packet),GFP_KERNEL);
if(!packet)
{
printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
return -ENOMEM;
} 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_DISCONNECT_CAMERA, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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
    int ret = Send_Command(APU_2_RPU_MB_CMD_DISCONNECT_CAMERA , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif




//	printk(KERN_ERR "RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);

  //  int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_DISCONNECT_CAMERA);
//	printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);
    xlnx_mbox_apu_wait_for_ack();
    kfree(packet);
	return result;
}
#if 0
RESULT VsiCamDeviceGetSoftwareVersion
(
    CamDeviceHandle_t   hCamDevice,
    char                *pVersionId
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    RESULT result = RET_SUCCESS;

    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pVersionId) {
        return (RET_NULL_POINTER);
    }

    char *version = "V6.0.0";
    memcpy(pVersionId, version, strlen(version));

//    pCamDevCtx->cookie ++;
//
//	payload_packet packet;
//	memset(&packet, 0, sizeof(payload_packet));
//
//	packet.cookie = pCamDevCtx->cookie;
//	packet.type = CMD;
//	packet.payload_size = 0;
//
//	uint8_t *p_data = packet.payload;
//	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
//	packet.payload_size += sizeof(uint32_t);
//	p_data += sizeof(uint32_t);
//	memcpy(p_data, pVersionId, sizeof(char));
//	packet.payload_size += sizeof(char);

//	    if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

//    result = Send_Command (APU_2_RPU_MB_CMD_GET_SOFTWARE_VERSION, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
//	if(RET_SUCCESS != result) {
//		return RET_FAILURE;
//	}
//	result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

	return result;
}
#endif

RESULT VsiCamDeviceStartStreaming
(
    CamDeviceConfig_t *pCamConfig,
    CamDeviceHandle_t hCamDevice,
    uint32_t frames                //0-continue
)
{
    RESULT result = RET_SUCCESS;

    struct vvcam_isp_dev *isp_dev = pCamConfig->isp_dev;    
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet = (payload_packet *)kmalloc(sizeof(payload_packet),GFP_KERNEL);
if(!packet)
{
printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
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
    memcpy(p_data, &frames, sizeof(uint32_t));
    packet->payload_size += sizeof(uint32_t);

        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_START_STREAMING,&packet,sizeof(packet),0,0);
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
    int ret = Send_Command(APU_2_RPU_MB_CMD_START_STREAMING , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif



//	printk(KERN_ERR "RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
//    int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_START_STREAMING);
//	printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);
    xlnx_mbox_apu_wait_for_ack();
    kfree(packet);
	return result;
}


RESULT VsiCamDeviceStopStreaming
(
    CamDeviceConfig_t *pCamConfig,
    CamDeviceHandle_t hCamDevice
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    // uint32_t hwId = pCamDevCtx->ispHwId;

    /*
     * AMD : Multi-Core Changes
     * Verifying Multi-core
     */
    // // xil_printf("%s %d ispHwId=%x \n\r",__func__,__LINE__,hwId);
    // selectDestinationCore(hwId);

    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_STOP_STREAMING,&packet,sizeof(packet), dest_cpu_id, src_cpu_id);
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
	return result;
}

RESULT VsiCamDeviceSetPathStreaming
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice,
    CamDevicePathStreamingCfg_t *pConfig
){
    RESULT result = RET_SUCCESS;
printk(KERN_ERR "RKC %s %dPathStatus %d \n",__func__,__LINE__,pConfig->outPathEnable);
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pConfig) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet = (payload_packet *)kmalloc(sizeof(payload_packet),GFP_KERNEL);
    if(!packet)
    {
    	printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
	return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
	printk(KERN_ERR "RKC %s %d pCamDevCtx->instanceId %d\n",__func__,__LINE__,pCamDevCtx->instanceId);
    //*(packet->payload) =(int *)0x0a;
#if 0
    *(packet->payload) =pCamDevCtx->instanceId;//(int *)0x0;
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    printk(KERN_ERR "RKC-pConfig->outPathEnable=%d\n",(int *)pConfig->outPathEnable);
    *(p_data) =(int *)pConfig->outPathEnable;//0x1;
    //*(p_data) =/*(int *) 0x1;*/(int *)pConfig->outPathEnable;//0x1;
    packet->payload_size += sizeof(uint32_t);
    printk(KERN_ERR "RKC-p_data->outPathEnable=%d\n",*((int *)p_data));
    p_data += sizeof(uint32_t);
#endif   

  *(int *)(packet->payload) = pCamDevCtx->instanceId;
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    printk(KERN_ERR "RKC-pConfig->outPathEnable=%d\n", pConfig->outPathEnable);
    *(int *)p_data = pConfig->outPathEnable; // 0x1
    //*(int *)p_data =/*(int *) 0x1;*/pConfig->outPathEnable;//0x1;
    packet->payload_size += sizeof(uint32_t);
    printk(KERN_ERR "RKC-p_data->outPathEnable=%d\n", *((int *)p_data));
    p_data += sizeof(uint32_t);
 
/*    *(packet->payload) =(int *)0xA;
    packet->payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
  */ 
        if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_SET_PATH_STREAMING, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
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
    int ret = Send_Command (APU_2_RPU_MB_CMD_SET_PATH_STREAMING,packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    //int ret = Send_Command( APU_2_RPU_MB_CMD_SET_PATH_STREAMING , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif



//	printk(KERN_ERR "RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
  //  int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_SET_PATH_STREAMING);
//	printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);
    xlnx_mbox_apu_wait_for_ack();
    kfree(packet);
	return result;
}

RESULT VsiCamDeviceGetPathStreaming
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t hCamDevice,
    CamDevicePathStreamingCfg_t *pConfig
){
    RESULT result = RET_SUCCESS;
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pConfig) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet *packet;
    packet = (payload_packet *)kmalloc(sizeof(payload_packet),GFP_KERNEL);
    if(!packet)
    {
    	printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
	return -ENOMEM;
    } 
    memset(packet, 0, sizeof(payload_packet));

    packet->cookie = pCamDevCtx->cookie;
    packet->type = CMD;
    packet->payload_size = 0;

    uint8_t *p_data = packet->payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);
    memcpy(p_data, pConfig, sizeof(CamDevicePathStreamingCfg_t));
    packet->payload_size += sizeof(CamDevicePathStreamingCfg_t);

	if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_GET_PATH_STREAMING, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != result) {
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
    printk(KERN_ERR " [APU]-%sSEND COOKIE == %d payload_size=%d\n",__func__,packet->cookie,packet->payload_size);
    mutex_lock(&isp_dev->mlock);
    int ret = Send_Command( APU_2_RPU_MB_CMD_GET_PATH_STREAMING , packet,packet->payload_size + payload_extra_size ,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif




//	printk(KERN_ERR "RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
  //  int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_GET_PATH_STREAMING);
//	printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);


    uint8_t __result = xlnx_mbox_apu_wait_for_data(packet->payload);
	if(__result)
    {
        printk(KERN_ERR "FAILED TO SET PATH STREAMING\n");
        return -1;
    }


	memcpy(pConfig, p_data, sizeof(CamDevicePathStreamingCfg_t));
      printk(KERN_ERR " [APU]->GET Pathstreaming = %d\n",pConfig->outPathEnable);
    kfree(packet);
	return result;
}

RESULT VsiCamDeviceGetHardwareId
(
    CamDeviceHandle_t hCamDevice,
    uint32_t         *pHwId
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    if (NULL == pCamDevCtx) {
        //TRACE(CAM_DEV_API_ERR, " %s: init Cam Device Context firstly!\n", __func__);
        return RET_WRONG_HANDLE;
    }
    if (NULL == pHwId) {
        //TRACE(CAM_DEV_API_ERR, " %s: invalid null pointer!\n", __func__);
        return RET_NULL_POINTER;
    }
    if (pCamDevCtx->ispHwId > CAMDEV_HARDWARE_ID_MAX) {
        return RET_OUTOFRANGE;
    }else {
       *pHwId = pCamDevCtx->ispHwId;
        return RET_SUCCESS;
    }
}

RESULT VsiCamDeviceAllocResMemory
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t   hCamDevice,
    uint32_t            byteSize,
    uint32_t           *pBaseAddress,
    void              **pIplAddress
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie ++;

	payload_packet* packet;
	memset(packet, 0, sizeof(payload_packet));

	packet->cookie = pCamDevCtx->cookie;
	packet->type = CMD;
	packet->payload_size = 0;

	uint8_t *p_data = packet->payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &byteSize, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	p_data += sizeof(uint32_t);
	memcpy(p_data, pBaseAddress, sizeof(uint32_t));
	packet->payload_size += 2 * sizeof(uint32_t);

	    if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0 
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_ALLOC_RES_MEMORY, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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
    int ret = Send_Command( APU_2_RPU_MB_CMD_ALLOC_RES_MEMORY , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif


//	printk(KERN_ERR "RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
  //  int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_ALLOC_RES_MEMORY);
//	printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);
 
    uint8_t __result = xlnx_mbox_apu_wait_for_data(packet->payload);
    if(__result)
    {
        printk(KERN_ERR "FAILED TO ALLOC RES MEMORY\n");
        return -1;
    }

    memcpy(pBaseAddress, p_data, sizeof(uint32_t));
	p_data += sizeof(uint32_t);
	memcpy(pIplAddress, p_data, sizeof(uint32_t));
    kfree(packet);
    return RET_SUCCESS;
}

RESULT VsiCamDeviceFreeResMemory
(
    struct vvcam_isp_dev *isp_dev,    
    CamDeviceHandle_t   hCamDevice,
    uint32_t            baseAddress
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie ++;

	payload_packet *packet;
    packet = (payload_packet *)kmalloc(sizeof(payload_packet),GFP_KERNEL);
if(!packet)
{
printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
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
	memcpy(p_data, &baseAddress, sizeof(uint32_t));
	packet->payload_size += sizeof(uint32_t);

	    if(packet->payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;
#if 0 
    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_FREE_RES_MEMORY, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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
    int ret = Send_Command( APU_2_RPU_MB_CMD_FREE_RES_MEMORY , packet,packet->payload_size + payload_extra_size,isp_dev->isp_rpu,MBOX_CORE_APU);
    if(0 != ret) {
    kfree(packet);
        return ret;
    }
    mutex_unlock(&isp_dev->mlock);
    ret = mbox_send_message(isp_dev->tx_chan,NULL);

#endif



//	printk(KERN_ERR "RKC-DRV %s %d Need to send MAILBOX Command here\n",__func__,__LINE__);
//int  mbox_send_ret = xlnx_mailbox_send_msg_client(packet,APU_2_RPU_MB_CMD_FREE_RES_MEMORY);
//	printk(KERN_ERR "RKC-DRV %s %d done MAILBOX Command here\n",__func__,__LINE__);
    xlnx_mbox_apu_wait_for_ack();
    kfree(packet);
    return RET_SUCCESS;
}

#if 0
RESULT VsiCamDeviceWriteRegister
(
    CamDeviceHandle_t    hCamDevice,
    uint32_t             address,
    uint32_t             value
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    RESULT result = RET_SUCCESS;
    if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &address, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &value, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	    if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_WRITE_REGISTER, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
	if(RET_SUCCESS != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
	result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

    return RET_SUCCESS;
}

RESULT VsiCamDeviceReadRegister
(
    CamDeviceHandle_t    hCamDevice,
    uint32_t             address,
    uint32_t             *value
)
{
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    RESULT result = RET_SUCCESS;
    if (NULL == pCamDevCtx) {
		return (RET_WRONG_HANDLE);
	}
	pCamDevCtx->cookie ++;

	payload_packet packet;
	memset(&packet, 0, sizeof(payload_packet));

	packet.cookie = pCamDevCtx->cookie;
	packet.type = CMD;
	packet.payload_size = 0;

	uint8_t *p_data = packet.payload;
	memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, &address, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);
	p_data += sizeof(uint32_t);
	memcpy(p_data, value, sizeof(uint32_t));
	packet.payload_size += sizeof(uint32_t);

	    if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_READ_REGISTER, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
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

	memcpy(value, p_data, sizeof(uint32_t));

    return RET_SUCCESS;
}

RESULT VsiCamDeviceImageSetMetadata
(
 CamDeviceHandle_t     hCamDevice,
 CamDeviceMetadataConfig_t *pMetadata
)
{
    RESULT result = RET_SUCCESS;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    if (NULL == pMetadata) {
        return (RET_NULL_POINTER);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);
    p_data += sizeof(uint32_t);
    memcpy(p_data, &pMetadata, sizeof(CamDeviceMetadataConfig_t));
    p_data += sizeof(CamDeviceMetadataConfig_t);
    packet.payload_size += sizeof(CamDeviceMetadataConfig_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    result = Send_Command (APU_2_RPU_MB_CMD_IMAGE_SET_METADATA, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(RET_SUCCESS != result) {
		os_unlock_mutex(&mbox_lock);
		return RET_FAILURE;
	}
    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}


RESULT VsiCamDeviceNrRelocEnable
(
 CamDeviceHandle_t     hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

	if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_NrRelocEnable, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
		os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }

    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}


RESULT VsiCamDeviceNrRelocDisable
(
 CamDeviceHandle_t     hCamDevice
)
{
    RESULT result = RET_SUCCESS;
    int ret = 0;

    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t*) hCamDevice;
    if (NULL == pCamDevCtx) {
        return (RET_WRONG_HANDLE);
    }
    pCamDevCtx->cookie ++;

    payload_packet packet;
    memset(&packet, 0, sizeof(payload_packet));

    packet.cookie = pCamDevCtx->cookie;
    packet.type = CMD;
    packet.payload_size = 0;

    uint8_t *p_data = packet.payload;
    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    packet.payload_size += sizeof(uint32_t);

        if(packet.payload_size > MAX_ITEM)
    	return RET_OUTOFRANGE;

    os_lock_mutex(&mbox_lock);

    ret = Send_Command (APU_2_RPU_MB_CMD_NrRelocDisable, &packet, packet.payload_size + payload_extra_size, dest_cpu_id, src_cpu_id);
    if(0 != ret) {
		os_unlock_mutex(&mbox_lock);
        return RET_FAILURE;
    }

    result = apu_wait_for_ACK(packet.cookie);
    if (result) {
        result = RET_FAILURE;
    }

    os_unlock_mutex(&mbox_lock);

	return result;
}
}
#endif
