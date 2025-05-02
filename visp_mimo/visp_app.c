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


#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <linux/delay.h>
#include "visp_event.h"
#include "visp_driver.h"
#include "media_isp.h"
#include <linux/string.h>
#include <linux/slab.h>
#include <cam_device.h>
#include "cam_device_api.h"
#include "cam_device_buffer_api.h"
#include <linux/delay.h>
#include "sensor_cmd.h"
#include "visp_app.h"
#include "visp_v4l2_std_exts.h"
#include "cam_device_sensor_api.h"
#include "iba.h"
#include "visp_common.h"

#define MEDIA_ISP_MCM_BUF_COUNT         4
#define MEDIA_ISP_EMPTY_BUF_WAIT_TIME   10
#define MEDIA_ISP_FULL_BUF_WAIT_TIME    120
int MediaIspDeviceMcmSetFormat(struct visp_dev *isp_dev , uint8_t Port);
int visp_set_fmt_public(struct visp_dev *isp_dev, struct v4l2_subdev_format *format);
int MediaIspDeviceCameraConnect(struct visp_dev *isp_dev, uint8_t Index);
int MediaIspDeviceSetFormat(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);



int MediaIspDeviceSetFrameRate(struct visp_dev *isp_dev, uint8_t Port, uint32_t *FrameRate)
{
	MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;

    RetVal = VsiCamDeviceSensorSetFrameRate(isp_dev, IspPort->CamDeviceHandle, FrameRate);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev, "CamDevice set framerate failed, ret is %d", RetVal);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
    }

    return RetVal;
}


int visp_buffer_alloc_public(struct visp_dev *isp_dev, struct visp_ext_buf_info *ext_buf_info);

int visp_buffer_free_public(struct visp_dev *isp_dev, struct visp_ext_buf_info *ext_buf_info);

int MediaIspHalAllocBuf(struct visp_dev *isp_dev, int Port, MediaBuf *BufInfo);
int MediaIspHalAllocBuf(struct visp_dev *isp_dev, int Port, MediaBuf *BufInfo)
{
    struct visp_ext_buf_info ExtBufInfo;
    int RetVal = VSI_SUCCESS;

    ExtBufInfo.port = Port;
    ExtBufInfo.plane.size = BufInfo->Planes[0].DmaSize;
    RetVal=visp_buffer_alloc_public(isp_dev, &ExtBufInfo);
    if (RetVal) {
        dev_err(isp_dev->dev, "Port %d alloc dma buffer failed:  \n", Port);
        return VSI_ERR_ILLEGAL_PARAM;
    }

    BufInfo->Planes[0].DmaAddr = ExtBufInfo.plane.dma_addr;

    return RetVal;
}

int  visp_buffer_free_public_wrapper(struct visp_dev *isp_dev, void *arg);

int MediaIspHalFreeBuf(struct visp_dev *isp_dev, int Port, MediaBuf *BufInfo);
int MediaIspHalFreeBuf(struct visp_dev *isp_dev, int Port, MediaBuf *BufInfo)
{
    struct visp_ext_buf_info ExtBufInfo;

    ExtBufInfo.port = Port;
    ExtBufInfo.plane.dma_addr = BufInfo->Planes[0].DmaAddr;
    visp_buffer_free_public_wrapper(isp_dev, &ExtBufInfo);
    dev_info(isp_dev->dev," Port %d buffer free: Dma 0x%x Size %d", Port, BufInfo->Planes[0].DmaAddr, BufInfo->Planes[0].DmaSize);

    return VSI_SUCCESS;
}

static int MediaIspDeviceDestroyBufPool(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;

    VsiCamDeviceReleaseBufMgmt(isp_dev , IspPort->CamDeviceHandle, Chn);
    VsiCamDeviceDestroyBufPool(isp_dev , IspPort->CamDeviceHandle, Chn);

        if (Chn == CAMDEV_BUFCHAIN_RDMA) {
        if (IspPort->McmAttr.InputSelect == MEDIA_ISP_MCM_INPUT_SELECT_RPU) {
		    int i=0;
            for (i = 0; i < IspPort->McmAttr.NumBufs; i++) {
                VsiCamDeviceFreeResMemory(isp_dev,IspPort->CamDeviceHandle, IspPort->McmAttr.Bufs[i].Planes[0].DmaAddr);
                IspPort->McmAttr.Bufs[i].Planes[0].DmaAddr = 0;
            }
        }
        else if (IspPort->McmAttr.InputSelect == MEDIA_ISP_MCM_INPUT_SELECT_APU) {
	        int i=0;
            for ( i = 0; i < IspPort->McmAttr.NumBufs; i++) {
                MediaBuf *BufInfo = &IspPort->McmAttr.Bufs[i];
                MediaIspHalFreeBuf(isp_dev, Port, BufInfo);
                BufInfo->Planes[0].DmaAddr = 0;
            }
        }
    }
    VsiCamDeviceDeInitBufChain(isp_dev, IspPort->CamDeviceHandle, Chn);

    return RetVal;
}





int MediaIspDeviceStreamOff(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspDeviceStreamOff(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    int RetVal  = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];

    CamDevicePathStreamingCfg_t PathStatus;
    VsiCamDeviceGetPathStreaming(isp_dev , IspPort->CamDeviceHandle, &PathStatus);
    PathStatus.outPathEnable &=  ~(1 << Chn);
    RetVal = VsiCamDeviceSetPathStreaming(isp_dev, IspPort->CamDeviceHandle, &PathStatus);
    MediaIspDeviceDestroyBufPool(isp_dev, Port, Chn);

    return RetVal;
}   


#if 1
RESULT VsiCamDeviceUnRegisterAeLib
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice
);
RESULT VsiCamDeviceAeDisable
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice
);
RESULT VsiCamDeviceAwbDisable
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t  hCamDevice
);
RESULT VsiCamDeviceUnRegisterAwbLib
(
    struct visp_dev *isp_dev,
    CamDeviceHandle_t hCamDevice
);

static int MediaIspDeviceUnRegister3aLib(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    #ifdef LOAD_CALIB_ENABLE

        MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
        VsiCamDeviceAeDisable(isp_dev, IspPort->CamDeviceHandle);
        VsiCamDeviceUnRegisterAeLib(isp_dev, IspPort->CamDeviceHandle);
        VsiCamDeviceAwbDisable(isp_dev, IspPort->CamDeviceHandle);
        VsiCamDeviceUnRegisterAwbLib(isp_dev, IspPort->CamDeviceHandle);
        //
    #endif

    return VSI_SUCCESS;
}

static int MediaIspDeviceMcmTerminate(struct visp_dev *isp_dev, uint8_t Port)
{
    int RetVal = VSI_SUCCESS;
    uint8_t Chn = CAMDEV_BUFCHAIN_RDMA;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];

    MediaIspDeviceDestroyBufPool(isp_dev, 0, Chn);
    IspPort->McmAttr.NumBufs = 0;

    return RetVal;
}



int MediaIspDeviceCameraDisConnect(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    dev_info(isp_dev->dev, "[VVCAM-CLEANUP]->%s %d\n", __func__, __LINE__);

    if (IspPort->CameraConnectRefCnt > 0) {
        IspPort->CameraConnectRefCnt--;
    }

    dev_info(isp_dev->dev, "[VVCAM-CLEANUP]->%s %d Camcnt=%d\n", __func__, __LINE__, IspPort->CameraConnectRefCnt);    

    if (IspPort->CameraConnectRefCnt == 0) {
        dev_info(isp_dev->dev, "[VVCAM-CLEANUP]->%s %d\n", __func__, __LINE__);    
        MediaIspDeviceUnRegister3aLib(isp_dev, Port, Chn);
        VsiCamDeviceDisconnectCamera(isp_dev, IspPort->CamDeviceHandle);
        MediaIspDeviceMcmTerminate(isp_dev, Port);
    }

    dev_info(isp_dev->dev, "[VVCAM-CLEANUP]->%s %d\n", __func__, __LINE__);    

    return VSI_SUCCESS;
}

#endif

int MediaIspDeviceCreateBufPool(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspDeviceCreateBufPool(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;
    int i = 0;
    uint32_t NumBufs = 0;
    CamDeviceBufPoolConfig_t BufPoolCfg;
    uint32_t BufSize = 0;

    CamDeviceBufChainConfig_t BufferChain;

    if (Chn < MEDIA_ISP_CHN_MAX) {
        NumBufs = IspPort->IspChns[Chn].NumBufs;
    } else {
        NumBufs = IspPort->McmAttr.NumBufs;
    }
    if(Chn==CAMDEV_BUFCHAIN_RDMA) {
	NumBufs=2;
    } else {
	NumBufs=4;
    }

    /**** STEP 1.INIT BUF CHAIN **********/
    memset(&BufferChain, 0, sizeof(CamDeviceBufChainConfig_t));
    BufferChain.skipInterval = 0;
    BufferChain.bufQueLength = NumBufs;
    BufferChain.emptyQueOp.blockType = CAMDEV_BUFQUE_TIMEOUT_TYPE;
    BufferChain.emptyQueOp.waitTime  = 5000; //MEDIA_ISP_EMPTY_BUF_WAIT_TIME;
    BufferChain.fullQueOp.blockType  = CAMDEV_BUFQUE_TIMEOUT_TYPE;
    BufferChain.fullQueOp.waitTime   = 5000; //MEDIA_ISP_FULL_BUF_WAIT_TIME;

    RetVal = VsiCamDeviceInitBufChain(isp_dev , IspPort->CamDeviceHandle,
        Chn, &BufferChain);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: CamDevice init buf chain %d failed, ret is %d",
            __func__, Chn, RetVal);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
        return RetVal;
    }

    /****** STEP 2. RESERVE BUFFERS *******/
    memset(&BufPoolCfg, 0, sizeof(BufPoolCfg));
    BufPoolCfg.bufNum        = NumBufs;
    BufPoolCfg.bufMode       = CAMDEV_BUFMODE_USERPTR;
    BufPoolCfg.is_mapped     = 0;
    BufPoolCfg.pBaseAddrList = kmalloc(NumBufs * sizeof(uint32_t),GFP_KERNEL);
    BufPoolCfg.pIplAddrList  = kmalloc(NumBufs * sizeof(void *),GFP_KERNEL);

    if (Chn < MEDIA_ISP_CHN_MAX) {
       // create isp buf pool by user allocated dma memory
        BufPoolCfg.bufSize = isp_dev->cap_sizeimage;

        for (i = 0; i < NumBufs; i++) {
		BufPoolCfg.pBaseAddrList[i] = isp_dev->op_a[i];
                BufPoolCfg.pIplAddrList[i]   = VSI_NULL;
        }
    } else if (Chn == CAMDEV_BUFCHAIN_RDMA) {
        // create mcm buf pool by camdevice reserved memory
        //uint32_t PhyAddr = 0;
        uint32_t *pIplAddr = VSI_NULL;
        RetVal = VsiCamDeviceGetBufferSize(isp_dev, IspPort->CamDeviceHandle, Chn, &BufSize);
        if (RetVal != VSI_SUCCESS) {
            dev_err(isp_dev->dev,"%s: CamDevice get chain %d buf size failed, ret is %d",
                __func__, Chn, RetVal);
            RetVal = VSI_ERR_ILLEGAL_PARAM;
            goto ERR_TO_DEINIT_CHAIN;
        }
        BufPoolCfg.bufSize = BufSize;
        BufPoolCfg.bufSize = isp_dev->out_sizeimage;	//
        IspPort->McmAttr.InputSelect = MEDIA_ISP_MCM_INPUT_SELECT_APU;

        if (IspPort->McmAttr.InputSelect == MEDIA_ISP_MCM_INPUT_SELECT_RPU) {
        } 
        else if (IspPort->McmAttr.InputSelect == MEDIA_ISP_MCM_INPUT_SELECT_APU) { 

            for (i = 0; i < NumBufs; i++) {
                MediaBuf *BufInfo = &IspPort->McmAttr.Bufs[i];
                BufInfo->Planes[0].DmaSize = BufSize;

				BufPoolCfg.pBaseAddrList[i] = isp_dev->ip_a[i];
                BufPoolCfg.pIplAddrList[i]   = (void *)pIplAddr;

            }
        }
    }

    else {
        dev_err(isp_dev->dev,"%s: current not support chn %d to create buf pool",
            __func__, Chn);
        RetVal = VSI_ERR_NOT_SUPPORT;
        goto ERR_TO_DEINIT_CHAIN;
    }

    /************ STEP 3. CREATE BUFFER POOL ****************/
    RetVal = VsiCamDeviceCreateBufPool(isp_dev , IspPort->CamDeviceHandle,
        Chn, &BufPoolCfg);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"CamDevice chn %d create buf pool failed, ret is %d",
            Chn, RetVal);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
        goto ERR_TO_RELEASE_MEM;
    }
    /************ STEP 4. SETUP BUF MGMT *******************/
    RetVal = VsiCamDeviceSetupBufMgmt(isp_dev, IspPort->CamDeviceHandle, Chn);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"CamDevice chn %d setup buf managementfailed, ret is %d",
            Chn, RetVal);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
       goto ERR_TO_DESTROY_BUFPOOL;
    }

    kfree(BufPoolCfg.pIplAddrList);
    kfree(BufPoolCfg.pBaseAddrList);
    return RetVal;

ERR_TO_DESTROY_BUFPOOL:
    VsiCamDeviceDestroyBufPool(isp_dev , IspPort->CamDeviceHandle, Chn);
ERR_TO_RELEASE_MEM:
  if (Chn == CAMDEV_BUFCHAIN_RDMA) {
        if (IspPort->McmAttr.InputSelect == MEDIA_ISP_MCM_INPUT_SELECT_RPU) {
            for (--i; i >= 0; --i) {
                VsiCamDeviceFreeResMemory(isp_dev,IspPort->CamDeviceHandle,
                    BufPoolCfg.pBaseAddrList[i]);
                IspPort->McmAttr.Bufs[i].Planes[0].DmaAddr = 0;
            }
        } else if (IspPort->McmAttr.InputSelect == MEDIA_ISP_MCM_INPUT_SELECT_APU) {
            for (--i; i >= 0; --i) {
                MediaBuf *BufInfo = &IspPort->McmAttr.Bufs[i];
                MediaIspHalFreeBuf(isp_dev, Port, BufInfo);
                BufInfo->Planes[0].DmaAddr = 0;
            }
        }
    }
ERR_TO_DEINIT_CHAIN:
    kfree(BufPoolCfg.pIplAddrList);
    kfree(BufPoolCfg.pBaseAddrList);
    VsiCamDeviceDeInitBufChain(isp_dev , IspPort->CamDeviceHandle, Chn);

    return RetVal;
}

int MediaIspDeviceStreamOn(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    int RetVal = VSI_SUCCESS;

    MediaIspDeviceMcmSetFormat(isp_dev,Port);

    /*Create Buffer pool for input*/
    RetVal = MediaIspDeviceCreateBufPool(isp_dev, Port, 6);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: Port %d Chn %d create buf pool failed, ret is %d",
            __func__, Port, Chn, RetVal);
        return RetVal;
    }
    /* Try connecting the sensor - not required for MIMO*/
    MediaIspDeviceCameraConnect(isp_dev,0);
    return 0;
}

int MediaIspDeviceStreamOnOut(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;
    int pad_index=-1;
    CamDevicePathStreamingCfg_t PathStatus;

    /* Set Input format */
    MediaIspDeviceSetFormat(isp_dev, 0, 0);

    Chn = CAMDEV_BUFCHAIN_MP;
    /*Create Buffer pool for output*/
    RetVal = MediaIspDeviceCreateBufPool(isp_dev, 0, Chn);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: Port %d Chn %d create buf pool failed, ret is %d",
            __func__, Port, Chn, RetVal);
        return RetVal;
    }

    PathStatus.outPathEnable |= (1 << Chn);

    /*Set streaming state*/
    RetVal = VsiCamDeviceSetPathStreaming(isp_dev,IspPort->CamDeviceHandle, &PathStatus);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"Port %d Chn %d CamDevice start stream failed, ret is %d",
            Port, Chn, RetVal);
        RetVal = VSI_ERR_NOTREADY;
        goto ERR_TO_DESTROY_BUFPOOL;
    }

    pad_index = (Port*MEDIA_ISP_PORT_PAD_COUNT)+ Chn+1;
    isp_dev->streamon[pad_index]=1;
    return RetVal;


ERR_TO_DESTROY_BUFPOOL:
	MediaIspDeviceDestroyBufPool(isp_dev, Port, Chn);

    return RetVal;
}
EXPORT_SYMBOL(MediaIspDeviceStreamOn);
EXPORT_SYMBOL(MediaIspDeviceStreamOnOut);


int MediaIspDeviceDeque(struct visp_dev *isp_dev, uint8_t Port);
int MediaIspDeviceDeque(struct visp_dev *isp_dev, uint8_t Port)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    MediaBuffer_t           *pMediaBuf;

    if (IspPort->CamDeviceHandle) {

	RetVal=VsiCamDeviceDeQueBuffer(isp_dev, IspPort->CamDeviceHandle, CAMDEV_BUFCHAIN_RDMA, &pMediaBuf);
        if (RetVal != VSI_SUCCESS) {
            dev_err(isp_dev->dev, "VsiCamDeviceDeQueBuffer failed %d", RetVal);
            RetVal = VSI_ERR_TIMEOUT;
            return RetVal;
        }

	RetVal=VsiCamDeviceEnQueBuffer(isp_dev, IspPort->CamDeviceHandle, CAMDEV_BUFCHAIN_RDMA, pMediaBuf);
        if (RetVal != VSI_SUCCESS) {
            dev_err(isp_dev->dev, "VsiCamDeviceEnQueBuffer failed %d", RetVal);
            RetVal = VSI_ERR_TIMEOUT;
            return RetVal;
        }
   }
    return RetVal;
}
EXPORT_SYMBOL(MediaIspDeviceDeque);

int IspDeviceDestroy(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];

    if (IspPort->CamDeviceHandle) {
//        RetVal = VsiCamDeviceSensorDrvHandleUnRegister(isp_dev, IspPort->CamDeviceHandle);
        if (RetVal != VSI_SUCCESS) {
            dev_err(isp_dev->dev, "CamDevice unregister sensor driver Failed, ret is %d", RetVal);
            RetVal = VSI_ERR_TIMEOUT;
            return RetVal;
        }

        RetVal = VsiCamDeviceDestroy(isp_dev, IspPort->CamDeviceHandle);
        if (RetVal != VSI_SUCCESS) {
            dev_err(isp_dev->dev, "CamDevice Isp Device handle destroy Failed, ret is %d", RetVal);
            RetVal = VSI_ERR_TIMEOUT;
            return RetVal;
        }
        IspPort->CamDeviceHandle = VSI_NULL;
    }
    return RetVal;
}


int IspDestroyCamDevice(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{

    if (isp_dev->IspPorts[Port].RefCount)
        isp_dev->IspPorts[Port].RefCount--;

    if (!isp_dev->IspPorts[Port].RefCount) {
        IspDeviceDestroy(isp_dev, Port, Chn);
    }

    return VSI_SUCCESS;
}

int IspDestroyPipeline(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    IspDestroyCamDevice(isp_dev, Port, Chn);

    return VSI_SUCCESS;
}

int MediaIspStreamOff(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{

    MediaIspDeviceStreamOff(isp_dev, Port, Chn);


    IspDestroyPipeline(isp_dev, Port, Chn);
    
    return VSI_SUCCESS;
}
EXPORT_SYMBOL(MediaIspStreamOff);    



int MediaIspDeviceQbuf(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn, MediaBuf *Buf)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort   = &isp_dev->IspPorts[Port];
    MediaIspChnAttr  *IspChn    = &IspPort->IspChns[Chn];
    MediaBuffer_t *pMediaBuffer = VSI_NULL;
    pMediaBuffer = IspChn->CamDeviceBufs[Buf->Index];

    if (pMediaBuffer == VSI_NULL) {
        dev_err(isp_dev->dev,"CamDevice queue buf is null");
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }    
    //  dev_info(isp_dev->dev,"%s %d pMediaBuffer=0x%x pMediaBuffer->pOwner 0x%x\n",__func__,__LINE__,pMediaBuffer->baseAddress,pMediaBuffer->pOwner);

    RetVal = VsiCamDeviceEnQueBuffer(isp_dev, IspPort->CamDeviceHandle, Chn, pMediaBuffer);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"CamDevice queue buf failed, ret is %d", RetVal);
        RetVal = VSI_ERR_TIMEOUT;
        return RetVal;
    }    
  
      return RetVal;
}


int MediaIspQBuf(struct visp_dev *isp_dev, int Pad_index, MediaBuf *Buf)
{
    int RetVal = VSI_SUCCESS;
    int Port = Pad_index / MEDIA_ISP_PORT_PAD_COUNT;
    int Chn  = (Pad_index % MEDIA_ISP_PORT_PAD_COUNT) - 1;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    MediaIspChnAttr  *IspChn  = &IspPort->IspChns[Chn];
    //  dev_info(isp_dev->dev," %d %d %x %x \n",Buf->Index,Buf->NumPlanes, Buf->Planes[0].DmaAddr, Buf->Planes[0].DmaSize );

    if(Buf==NULL)
    {
		dev_err(isp_dev->dev,"got NULL BUFFER\n");
	return -1;
    }

    if (isp_dev->streamon[Pad_index]==0) {
        memcpy(&IspChn->Bufs[Buf->Index], Buf, sizeof(MediaBuf));
    } else {
        RetVal = MediaIspDeviceQbuf(isp_dev, Port, Chn, Buf);
        if (RetVal != VSI_SUCCESS) {
            dev_err(isp_dev->dev,"Port %d Chn %d Qbuf failed, ret is %d", Port, Chn, RetVal);
        }
    }
    return RetVal;
}


static int MediaIspHalMediaFmtToMBusFmt(uint32_t *Code, uint32_t *PixelFormat)
{
    switch(*Code) {
        case MEDIA_PIX_FMT_NV16:
            *PixelFormat = MEDIA_BUS_FMT_YUYV8_2X8;
            break;
        case MEDIA_PIX_FMT_NV12:
            *PixelFormat = MEDIA_BUS_FMT_YUYV8_1_5X8;
            break;
        case MEDIA_PIX_FMT_YUYV:
            *PixelFormat = MEDIA_BUS_FMT_YUYV8_1X16;
            break;
        case MEDIA_PIX_FMT_GREY:
            *PixelFormat = MEDIA_BUS_FMT_Y8_1X8;
            break;
        case MEDIA_PIX_FMT_Y10BPACK:
            *PixelFormat = MEDIA_BUS_FMT_Y10_1X10;
            break;
        case MEDIA_PIX_FMT_Y10DWA:
            *PixelFormat = MEDIA_BUS_FMT_Y10_1X10;
            break;
        case MEDIA_PIX_FMT_Y10:
            *PixelFormat = MEDIA_BUS_FMT_Y10_1X10;
            break;
        case MEDIA_PIX_FMT_P00BPACK:
            *PixelFormat = MEDIA_BUS_FMT_YUYV10_2X10;
            break;
        case MEDIA_PIX_FMT_P00DWA:
            *PixelFormat = MEDIA_BUS_FMT_YUYV10_2X10;
            break;
        case MEDIA_PIX_FMT_P010:
            *PixelFormat = MEDIA_BUS_FMT_YUYV10_2X10;
            break;
        case MEDIA_PIX_FMT_P02BPACK:
            *PixelFormat = MEDIA_BUS_FMT_YUYV12_2X12;
            break;
        case MEDIA_PIX_FMT_P20BPACK:
            *PixelFormat = MEDIA_BUS_FMT_YUYV10_2X10;
            break;
        case MEDIA_PIX_FMT_P20DWA:
            *PixelFormat = MEDIA_BUS_FMT_YUYV10_2X10;
            break;
        case MEDIA_PIX_FMT_P210:
            *PixelFormat = MEDIA_BUS_FMT_YUYV10_2X10;
            break;
        case MEDIA_PIX_FMT_P22BPACK:
            *PixelFormat = MEDIA_BUS_FMT_YUYV12_2X12;
            break;
        case MEDIA_PIX_FMT_I20BPACK:
            *PixelFormat = MEDIA_BUS_FMT_YUYV10_2X10;
            break;
        case MEDIA_PIX_FMT_I210:
            *PixelFormat = MEDIA_BUS_FMT_YUYV10_2X10;
            break;
        case MEDIA_PIX_FMT_M48BPACK:
            *PixelFormat = MEDIA_BUS_FMT_YUV8_1X24;
            break;
        case MEDIA_PIX_FMT_I48BPACK:
            *PixelFormat = MEDIA_BUS_FMT_YUV8_1X24;
            break;
        case MEDIA_PIX_FMT_I48DWA:
            *PixelFormat = MEDIA_BUS_FMT_YUV8_1X24;
            break;
        case MEDIA_PIX_FMT_I40DWA:
            *PixelFormat = MEDIA_BUS_FMT_YUV8_1X24;
            break;
        case MEDIA_PIX_FMT_RGB24:
            *PixelFormat = MEDIA_BUS_FMT_RGB888_1X24;
            break;
        case MEDIA_PIX_FMT_RGB24DWA:
            *PixelFormat = MEDIA_BUS_FMT_RGB888_1X24;
            break;
        case MEDIA_PIX_FMT_RGB24P:
            *PixelFormat = MEDIA_BUS_FMT_RGB888_3X8;
            break;;
        case MEDIA_PIX_FMT_SBGGR8:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR8_1X8;
            break;
        case MEDIA_PIX_FMT_SGBRG8:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG8_1X8;
            break;
        case MEDIA_PIX_FMT_SGRBG8:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG8_1X8;
            break;
        case MEDIA_PIX_FMT_SRGGB8:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB8_1X8;
            break;
        case MEDIA_PIX_FMT_SBGGR10:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR10_1X10;
            break;
        case MEDIA_PIX_FMT_SGBRG10:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG10_1X10;
            break;
        case MEDIA_PIX_FMT_SGRBG10:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG10_1X10;
            break;
        case MEDIA_PIX_FMT_SRGGB10:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB10_1X10;
            break;
        case MEDIA_PIX_FMT_SBGGR10BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR10_1X10;
            break;
        case MEDIA_PIX_FMT_SGBRG10BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG10_1X10;
            break;
        case MEDIA_PIX_FMT_SGRBG10BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG10_1X10;
            break;
        case MEDIA_PIX_FMT_SRGGB10BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB10_1X10;
            break;
        case MEDIA_PIX_FMT_SBGGR10DWA:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR10_1X10;
            break;
        case MEDIA_PIX_FMT_SGBRG10DWA:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG10_1X10;
            break;
        case MEDIA_PIX_FMT_SGRBG10DWA:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG10_1X10;
            break;
        case MEDIA_PIX_FMT_SRGGB10DWA:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB10_1X10;
            break;
        case MEDIA_PIX_FMT_SBGGR12:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR12_1X12;
            break;
        case MEDIA_PIX_FMT_SGBRG12:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG12_1X12;
            break;
        case MEDIA_PIX_FMT_SGRBG12:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG12_1X12;
            break;
        case MEDIA_PIX_FMT_SRGGB12:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB12_1X12;
            break;
        case MEDIA_PIX_FMT_SBGGR12BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR12_1X12;
            break;
        case MEDIA_PIX_FMT_SGBRG12BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG12_1X12;
            break;
        case MEDIA_PIX_FMT_SGRBG12BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG12_1X12;
            break;
        case MEDIA_PIX_FMT_SRGGB12BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB12_1X12;
            break;
        case MEDIA_PIX_FMT_SBGGR12DWA:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR12_1X12;
            break;
        case MEDIA_PIX_FMT_SGBRG12DWA:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG12_1X12;
            break;
        case MEDIA_PIX_FMT_SGRBG12DWA:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG12_1X12;
            break;
        case MEDIA_PIX_FMT_SRGGB12DWA:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB12_1X12;
            break;
        case MEDIA_PIX_FMT_SBGGR14BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR14_1X14;
            break;
        case MEDIA_PIX_FMT_SGBRG14BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG14_1X14;
            break;
        case MEDIA_PIX_FMT_SGRBG14BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG14_1X14;
            break;
        case MEDIA_PIX_FMT_SRGGB14BPACK:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB14_1X14;
            break;
        case MEDIA_PIX_FMT_SBGGR14DWA:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR14_1X14;
            break;
        case MEDIA_PIX_FMT_SGBRG14DWA:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG14_1X14;
            break;
        case MEDIA_PIX_FMT_SGRBG14DWA:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG14_1X14;
            break;
        case MEDIA_PIX_FMT_SRGGB14DWA:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB14_1X14;
            break;
        case MEDIA_PIX_FMT_SBGGR14:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR14_1X14;
            break;
        case MEDIA_PIX_FMT_SGBRG14:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG14_1X14;
            break;
        case MEDIA_PIX_FMT_SGRBG14:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG14_1X14;
            break;
        case MEDIA_PIX_FMT_SRGGB14:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB14_1X14;
            break;
        case MEDIA_PIX_FMT_SBGGR16:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR16_1X16;
            break;
        case MEDIA_PIX_FMT_SGBRG16:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG16_1X16;
            break;
        case MEDIA_PIX_FMT_SGRBG16:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG16_1X16;
            break;
        case MEDIA_PIX_FMT_SRGGB16:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB16_1X16;
            break;
        case MEDIA_PIX_FMT_SBGGR24:
            *PixelFormat = MEDIA_BUS_FMT_SBGGR24_1X24;
            break;
        case MEDIA_PIX_FMT_SGBRG24:
            *PixelFormat = MEDIA_BUS_FMT_SGBRG24_1X24;
            break;
        case MEDIA_PIX_FMT_SGRBG24:
            *PixelFormat = MEDIA_BUS_FMT_SGRBG24_1X24;
            break;
        case MEDIA_PIX_FMT_SRGGB24:
            *PixelFormat = MEDIA_BUS_FMT_SRGGB24_1X24;
            break;
        default:
            return VSI_ERR_ILLEGAL_PARAM;
    }

    return VSI_SUCCESS;
}



int MediaIspHalMbusFmtToMediaFmt(uint32_t *Code, uint32_t *PixelFormat, uint32_t Fourcc)
{
    uint32_t NewCode = 0;
    switch(*Code) {
        case MEDIA_BUS_FMT_YUYV8_2X8:
            *PixelFormat = MEDIA_PIX_FMT_NV16;
            break;
        case MEDIA_BUS_FMT_YUYV8_1_5X8:
            *PixelFormat = MEDIA_PIX_FMT_NV12;
            break;
        case MEDIA_BUS_FMT_YUYV8_1X16:
            *PixelFormat = MEDIA_PIX_FMT_YUYV;
            break;
        case MEDIA_BUS_FMT_Y8_1X8:
            *PixelFormat = MEDIA_PIX_FMT_GREY;
            break;
        case MEDIA_BUS_FMT_RGB888_3X8:
            *PixelFormat = MEDIA_PIX_FMT_RGB24P;
            break;
        case MEDIA_BUS_FMT_SBGGR8_1X8:
            *PixelFormat = MEDIA_PIX_FMT_SBGGR8;
            break;
        case MEDIA_BUS_FMT_SGBRG8_1X8:
            *PixelFormat = MEDIA_PIX_FMT_SGBRG8;
            break;
        case MEDIA_BUS_FMT_SGRBG8_1X8:
            *PixelFormat = MEDIA_PIX_FMT_SGRBG8;
            break;
        case MEDIA_BUS_FMT_SRGGB8_1X8:
            *PixelFormat = MEDIA_PIX_FMT_SRGGB8;
            break;
        case MEDIA_BUS_FMT_Y10_1X10:
        case MEDIA_BUS_FMT_YUYV10_2X10:
        case MEDIA_BUS_FMT_YUYV12_2X12:
        case MEDIA_BUS_FMT_YUV8_1X24:
        case MEDIA_BUS_FMT_RGB888_1X24:
        case MEDIA_BUS_FMT_SBGGR10_1X10:
        case MEDIA_BUS_FMT_SGBRG10_1X10:
        case MEDIA_BUS_FMT_SGRBG10_1X10:
        case MEDIA_BUS_FMT_SRGGB10_1X10:
        case MEDIA_BUS_FMT_SBGGR12_1X12:
        case MEDIA_BUS_FMT_SGBRG12_1X12:
        case MEDIA_BUS_FMT_SGRBG12_1X12:
        case MEDIA_BUS_FMT_SRGGB12_1X12:
        case MEDIA_BUS_FMT_SBGGR14_1X14:
        case MEDIA_BUS_FMT_SGBRG14_1X14:
        case MEDIA_BUS_FMT_SGRBG14_1X14:
        case MEDIA_BUS_FMT_SRGGB14_1X14:
        case MEDIA_BUS_FMT_SBGGR16_1X16:
        case MEDIA_BUS_FMT_SGBRG16_1X16:
        case MEDIA_BUS_FMT_SGRBG16_1X16:
        case MEDIA_BUS_FMT_SRGGB16_1X16:
        case MEDIA_BUS_FMT_SBGGR24_1X24:
        case MEDIA_BUS_FMT_SGBRG24_1X24:
        case MEDIA_BUS_FMT_SGRBG24_1X24:
        case MEDIA_BUS_FMT_SRGGB24_1X24:
            *PixelFormat = Fourcc;
            break;
        default:
            return VSI_ERR_ILLEGAL_PARAM;
    }

    /* double check fourcc and media bus format*/
    MediaIspHalMediaFmtToMBusFmt(&NewCode, PixelFormat);
    if (NewCode != *Code) {
        return VSI_ERR_ILLEGAL_PARAM;
    }

    return VSI_SUCCESS;
}

int MediaIspHalSetFmt(struct visp_dev *isp_dev, int Pad, MediaFmt *Format)
{
    struct v4l2_subdev_format *SdFmt;
    int RetVal = 0;

	SdFmt = kmalloc(sizeof(SdFmt), GFP_KERNEL);
	if(!SdFmt)
	{
	    dev_err(isp_dev->dev, "FAILED TO KMALLOC %s %d\n", __func__, __LINE__);
	    return -ENOMEM;
	}
    memset(SdFmt, 0, sizeof(struct v4l2_subdev_format));

    SdFmt->pad = Pad,
    SdFmt->which = V4L2_SUBDEV_FORMAT_ACTIVE;
    SdFmt->format.width = Format->Width;
    SdFmt->format.height = Format->Height;
    SdFmt->format.colorspace = Format->ColorSpace;
    SdFmt->format.quantization = Format->Quantization;

	if (sizeof(SdFmt->format.reserved) == (sizeof(uint16_t) * 10)) {
		memcpy(SdFmt->format.reserved, &Format->PixelFormat, sizeof(Format->PixelFormat));
	} else {
		memcpy(&SdFmt->format.reserved[1], &Format->PixelFormat, sizeof(Format->PixelFormat));
    }

	RetVal = MediaIspHalMediaFmtToMBusFmt(&Format->PixelFormat, &SdFmt->format.code);
	if (RetVal != 0) {
		kfree(SdFmt);
		dev_err(isp_dev->dev,"%s: MediaIspHalSetFmt failed %d", __func__, RetVal);
		return RetVal;
	}

	RetVal = visp_set_fmt_public(isp_dev, SdFmt);
	if (RetVal != 0) {
		kfree(SdFmt);
		dev_err(isp_dev->dev, " %s: visp_set_fmt_public failed %d", __func__, RetVal);
		return RetVal;
	}
//TODO - FREE SdFMt check if it is not neded after this
	kfree(SdFmt);
	return RetVal;
}
EXPORT_SYMBOL(MediaIspHalSetFmt);

#if 1
static int MediaFmtToIspFmt(uint32_t *MediaFmt, CamDevicePipeOutFmt_t *IspFmt)
{
    switch (*MediaFmt) {
        case MEDIA_PIX_FMT_YUYV:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV422I;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_NV16:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV422SP;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_NV12:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV420SP;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_GREY:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV400;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_Y10BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV400;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_Y10DWA:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV400_ALIGNED_MODE0;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_Y10:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV400_ALIGNED_MODE1;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_P00BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV420SP;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_P00DWA:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE0;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_P010:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV420SP_ALIGNED_MODE1;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_P02BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV420SP;
            IspFmt->dataBits  = 12;
            break;
        case MEDIA_PIX_FMT_P20BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV422SP;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_P20DWA:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE0;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_P210:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV422SP_ALIGNED_MODE1;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_P22BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV422SP;
            IspFmt->dataBits  = 12;
            break;
        case MEDIA_PIX_FMT_I20BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV422I;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_I210:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV422I_ALIGNED_MODE1;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_M48BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV444P;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_I48BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV444I;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_I48DWA:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV444I_ALIGNED_MODE0;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_I40DWA:
            IspFmt->outFormat = CAMDEV_PIX_FMT_YUV444I_ALIGNED_MODE0;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_RGB24:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RGB888;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_RGB24DWA:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RGB888_ALIGNED_MODE0;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_RGB24P:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RGB888P;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_SBGGR8:
        case MEDIA_PIX_FMT_SGBRG8:
        case MEDIA_PIX_FMT_SGRBG8:
        case MEDIA_PIX_FMT_SRGGB8:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW8;
            IspFmt->dataBits  = 8;
            break;
        case MEDIA_PIX_FMT_SBGGR10:
        case MEDIA_PIX_FMT_SGBRG10:
        case MEDIA_PIX_FMT_SGRBG10:
        case MEDIA_PIX_FMT_SRGGB10:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE1;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_SBGGR10BPACK:
        case MEDIA_PIX_FMT_SGBRG10BPACK:
        case MEDIA_PIX_FMT_SGRBG10BPACK:
        case MEDIA_PIX_FMT_SRGGB10BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW10;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_SBGGR10DWA:
        case MEDIA_PIX_FMT_SGBRG10DWA:
        case MEDIA_PIX_FMT_SGRBG10DWA:
        case MEDIA_PIX_FMT_SRGGB10DWA:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW10_ALIGNED_MODE0;
            IspFmt->dataBits  = 10;
            break;
        case MEDIA_PIX_FMT_SBGGR12:
        case MEDIA_PIX_FMT_SGBRG12:
        case MEDIA_PIX_FMT_SGRBG12:
        case MEDIA_PIX_FMT_SRGGB12:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE1;
            IspFmt->dataBits  = 12;
            break;
        case MEDIA_PIX_FMT_SBGGR12BPACK:
        case MEDIA_PIX_FMT_SGBRG12BPACK:
        case MEDIA_PIX_FMT_SGRBG12BPACK:
        case MEDIA_PIX_FMT_SRGGB12BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW12;
            IspFmt->dataBits  = 12;
            break;
        case MEDIA_PIX_FMT_SBGGR12DWA:
        case MEDIA_PIX_FMT_SGBRG12DWA:
        case MEDIA_PIX_FMT_SGRBG12DWA:
        case MEDIA_PIX_FMT_SRGGB12DWA:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE0;
            IspFmt->dataBits  = 12;
            break;
        case MEDIA_PIX_FMT_SBGGR14:
        case MEDIA_PIX_FMT_SGBRG14:
        case MEDIA_PIX_FMT_SGRBG14:
        case MEDIA_PIX_FMT_SRGGB14:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE1;
            IspFmt->dataBits  = 14;
            break;
        case MEDIA_PIX_FMT_SBGGR14BPACK:
        case MEDIA_PIX_FMT_SGBRG14BPACK:
        case MEDIA_PIX_FMT_SGRBG14BPACK:
        case MEDIA_PIX_FMT_SRGGB14BPACK:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW14;
            IspFmt->dataBits  = 14;
            break;
        case MEDIA_PIX_FMT_SBGGR14DWA:
        case MEDIA_PIX_FMT_SGBRG14DWA:
        case MEDIA_PIX_FMT_SGRBG14DWA:
        case MEDIA_PIX_FMT_SRGGB14DWA:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE0;
            IspFmt->dataBits  = 14;
            break;
        case MEDIA_PIX_FMT_SBGGR16:
        case MEDIA_PIX_FMT_SGBRG16:
        case MEDIA_PIX_FMT_SGRBG16:
        case MEDIA_PIX_FMT_SRGGB16:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW16;
            IspFmt->dataBits  = 16;
            break;
        case MEDIA_PIX_FMT_SBGGR24:
        case MEDIA_PIX_FMT_SGBRG24:
        case MEDIA_PIX_FMT_SGRBG24:
        case MEDIA_PIX_FMT_SRGGB24:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW24;
            IspFmt->dataBits  = 24;
            break;
        default:
            printk(KERN_ERR "Not support format %s", (char *)MediaFmt);
            return VSI_ERR_NOT_SUPPORT;
    }
    return VSI_SUCCESS;
}
#endif


//

int MediaIspDeviceSetFormat(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    /************ STEP 2 Streamon --> SetOutFormat******************/
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];//
    int RetVal=0;

    CamDevicePipeOutFmt_t IspFormat;
    memset(&IspFormat, 0, sizeof(IspFormat));
    IspFormat.outWidth = isp_dev->cap_w;
    IspFormat.outHeight = isp_dev->cap_h;
    IspFormat.pathOutType = 0;

    RetVal = MediaFmtToIspFmt(&isp_dev->cap_fmt, &IspFormat);

    RetVal = VsiCamDeviceSetOutFormat(isp_dev , IspPort->CamDeviceHandle, Chn, &IspFormat);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev, "CamDevice set format failed, ret is %d", RetVal);
        dev_err(isp_dev->dev, "Port %d Chn %d set format failed, ret is %d", Port, Chn, RetVal);
        RetVal = VSI_ERR_TIMEOUT;
        goto ERR_TO_CAMERA_DISCONNECT;
    }

    return RetVal;
ERR_TO_CAMERA_DISCONNECT:
// RANJITH ADD the cleanup later
    return RetVal;
}
EXPORT_SYMBOL(MediaIspDeviceSetFormat);


int MediaIspCalibGetModeInfo( struct visp_dev *isp_dev , uint8_t Port, CamDeviceSensorModeInfo_t *ModeInfo)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = VSI_NULL;


    if (!isp_dev || !ModeInfo) {
        dev_err(isp_dev->dev,"%s: null pointer of handle", __func__);
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }

    IspPort = &isp_dev->IspPorts[Port];
	memcpy(ModeInfo, &IspPort->SensorInfo.ModeInfo, sizeof(IspPort->SensorInfo.ModeInfo));

    return RetVal;
}

static int MediaIspDeviceSubModuleInit(struct visp_dev *isp_dev, uint8_t Port, CamDevicePipeSubmoduleCtrl_u *SubModuleInit)
{
    int RetVal = VSI_SUCCESS;
    CamDeviceSensorModeInfo_t ModeInfo;
    memset(&ModeInfo, 0, sizeof(ModeInfo));

    SubModuleInit->allCtrl = 0xFFFFFFFF;
    SubModuleInit->subCtrl.rgbirEnable = 0;
    SubModuleInit->subCtrl.hdrEnable   = 0;
    SubModuleInit->subCtrl.pdafEnable  = 0;
    SubModuleInit->subCtrl.dpfEnable   = 0;
    SubModuleInit->subCtrl.compressEnable = 0;
    SubModuleInit->subCtrl.expandEnable   = 0;
    SubModuleInit->subCtrl.eeEnable    = 0;
    SubModuleInit->subCtrl.ynrEnable   = 0;
    SubModuleInit->subCtrl.cnrEnable   = 0;
    SubModuleInit->subCtrl.lut3dEnable = 0;
    SubModuleInit->subCtrl.dnr2Enable  = 0;
    SubModuleInit->subCtrl.dnr3Enable  = 0;
    SubModuleInit->subCtrl.gtmEnable   = 0;
    SubModuleInit->subCtrl.wdrEnable   = 0;
    SubModuleInit->subCtrl.lscEnable   = 0;

    RetVal = MediaIspCalibGetModeInfo(isp_dev, Port, &ModeInfo);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev, "%s: get sensor mode failed, ret is %d", __func__, RetVal);
        return RetVal;
    }

    if (ModeInfo.sensorType == CAMDEV_SENSOR_TYPE_STITCHING_HDR) {
        SubModuleInit->subCtrl.hdrEnable = 1;
    }

    return RetVal;
}

int MediaIspCalibGetSensorName(struct visp_dev *isp_dev, uint8_t Port, char *SensorName)
{
    MediaIspPortAttr *IspPort = VSI_NULL;
    int RetVal = VSI_SUCCESS;

    IspPort = &isp_dev->IspPorts[Port];

    dev_info(isp_dev->dev,"%s: isp : %d", __func__, isp_dev->id);
    dev_info(isp_dev->dev,"%s: port: %d", __func__, Port);
    dev_info(isp_dev->dev,"%s: name: %s", __func__, IspPort->SensorInfo.Name);
    dev_info(isp_dev->dev,"%s: mode: %d", __func__, IspPort->SensorInfo.Mode);
    dev_info(isp_dev->dev,"%s: xml : %s", __func__, IspPort->SensorInfo.CalibXml);
    dev_info(isp_dev->dev,"%s: manu_json: %s", __func__, IspPort->SensorInfo.ManuJson);
    dev_info(isp_dev->dev,"%s: auto_json: %s", __func__, IspPort->SensorInfo.AutoJson);



    if (!SensorName || !isp_dev) {
        dev_err(isp_dev->dev,"%s: null pointer of handle", __func__);
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }

    IspPort = &isp_dev->IspPorts[Port];

    if (strlen(IspPort->SensorInfo.Name)) {
        strncpy(SensorName, IspPort->SensorInfo.Name, strlen(IspPort->SensorInfo.Name)+1);
    } else {
        dev_err(isp_dev->dev,"%s: get null string of sensor name", __func__);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
    }

    return RetVal;
}

int MediaIspCalibGetSensorMode(struct visp_dev *isp_dev , uint8_t Port, uint8_t *SensorMode)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = VSI_NULL;


    if (!SensorMode || !isp_dev) {
        dev_err(isp_dev->dev, "%s: null pointer of handle", __func__);
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }

    IspPort = &isp_dev->IspPorts[Port];
    *SensorMode = IspPort->SensorInfo.Mode;

    return RetVal;
}

int MediaIspDeviceSensorOpen(struct visp_dev *isp_dev, uint8_t Port)
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;
    uint8_t ModeIndex = 0;
    char SensorName[MEDIA_ISP_CHAR_LENGTH_MAX];

    memset(SensorName, 0, sizeof(SensorName));

    RetVal = MediaIspCalibGetSensorMode(isp_dev, Port, &ModeIndex);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: get sensor mode failed, ret is %d", __func__, RetVal);
    }
    RetVal = VsiCamDeviceSensorOpen(isp_dev , IspPort->CamDeviceHandle, (uint32_t )ModeIndex);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"CamDevice open sensor %s mode %d driver Failed, ret is %d", SensorName, ModeIndex, RetVal);
    }

    return RetVal;
}



int MediaIspDeviceMcmSetFormat(struct visp_dev *isp_dev , uint8_t Port)
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;
    CamDevicePipeInFmt_t InFormat;
    CamDeviceSensorModeInfo_t ModeInfo;
    CamDevicePipeInPathType_t InPath = CAMDEV_PIPE_INPATH_RDMA;

    memset(&InFormat, 0, sizeof(CamDevicePipeInFmt_t));
    memset(&ModeInfo, 0, sizeof(CamDeviceSensorModeInfo_t));

    RetVal = MediaIspCalibGetModeInfo(isp_dev, Port, &ModeInfo);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: Port %d get sensor mode info failed, ret is %d", __func__, Port, RetVal);
        return RetVal;
    }
    InFormat.inWidth    = isp_dev->out_w;
    InFormat.inHeight   = isp_dev->out_h;
    InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_RGGB;
    InFormat.stitchMode = (CamDeviceStitchingMode_t) CAMDEV_SENSOR_TYPE_LINEAR;

    if (ModeInfo.sensorType == CAMDEV_SENSOR_TYPE_STITCHING_HDR) {
        // hardware limit, may cause accuracy loss for bitwidth
        InFormat.inFormat = CAMDEV_INPUT_FMT_RAW16;
    } else {
        switch (ModeInfo.bitWidth) {
        case 8:
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW8;
            break;
        case 10:
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW10;
            break;
        case 12:
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW12;
            break;
        case 14:
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW14;
            break;
        case 16:
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW16;
            break;
        default:
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW16;
            break;
        }
    }

	if(isp_dev->out_fmt == V4L2_PIX_FMT_SRGGB10) {
		InFormat.inFormat = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
	} else if(isp_dev->out_fmt == V4L2_PIX_FMT_SRGGB12){
		InFormat.inFormat = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
	} else if(isp_dev->out_fmt == V4L2_PIX_FMT_SRGGB8){
		InFormat.inFormat = CAMDEV_INPUT_FMT_RAW8;
	} else {
		InFormat.inFormat = isp_dev->out_fmt;
	}

	switch (isp_dev->out_fmt) {
		case V4L2_PIX_FMT_SRGGB8:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_RGGB;
			InFormat.inFormat = CAMDEV_INPUT_FMT_RAW8;
             break;
        case V4L2_PIX_FMT_SRGGB10:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_RGGB;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
             break;
        case V4L2_PIX_FMT_SRGGB12:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_RGGB;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
             break;
        case V4L2_PIX_FMT_SRGGB14:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_RGGB;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW14_ALIGNED1;
             break;
        case V4L2_PIX_FMT_SRGGB16:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_RGGB;
             InFormat.inFormat = CAMDEV_INPUT_FMT_RAW16;
             break;
		case V4L2_PIX_FMT_SBGGR8:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_BGGR;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW8;
            break;
        case V4L2_PIX_FMT_SBGGR10:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_BGGR;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
            break;
        case V4L2_PIX_FMT_SBGGR12:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_BGGR;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
            break;
        case V4L2_PIX_FMT_SBGGR14:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_BGGR;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW14_ALIGNED1;
            break;
        case V4L2_PIX_FMT_SBGGR16:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_BGGR;
             InFormat.inFormat = CAMDEV_INPUT_FMT_RAW16;
             break;
		case V4L2_PIX_FMT_SGBRG8:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_GBRG;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW8;
            break;
        case V4L2_PIX_FMT_SGBRG10:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_GBRG;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
            break;
        case V4L2_PIX_FMT_SGBRG12:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_GBRG;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
            break;
        case V4L2_PIX_FMT_SGBRG14:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_GBRG;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW14_ALIGNED1;
            break;
        case V4L2_PIX_FMT_SGBRG16:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_GBRG;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW16;
            break;
		case V4L2_PIX_FMT_SGRBG8:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_GRBG;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW8;
            break;
        case V4L2_PIX_FMT_SGRBG10:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_GRBG;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW10_ALIGNED1;
            break;
        case V4L2_PIX_FMT_SGRBG12:
			InFormat.inPattern = CAMDEV_RAW_RGB_PAT_GRBG;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW12_ALIGNED1;
            break;
        case V4L2_PIX_FMT_SGRBG14:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_GRBG;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW14_ALIGNED1;
            break;
        case V4L2_PIX_FMT_SGRBG16:
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_GRBG;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW16;
            break;
        default:
			dev_err(isp_dev->dev,"unsupported in format");
			InFormat.inPattern  = CAMDEV_RAW_RGB_PAT_RGGB;
            InFormat.inFormat = CAMDEV_INPUT_FMT_RAW12;
            break;
	}

    pr_err("RKC-PRINT %s %d \n",__func__, __LINE__);
	RetVal = VsiCamDeviceSetInFormat(isp_dev , IspPort->CamDeviceHandle , InPath, &InFormat);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"CamDevice set input path %d format failed, ret is %d", InPath, RetVal);
        RetVal = VSI_ERR_ILLEGAL_PARAM;;
    }

    return RetVal;
}
EXPORT_SYMBOL(MediaIspDeviceMcmSetFormat);

int MediaIspDeviceCameraConnect(struct visp_dev *isp_dev, uint8_t Index)
{
    int Port = Index / MEDIA_ISP_PORT_PAD_COUNT;
    int Chn  = (Index % MEDIA_ISP_PORT_PAD_COUNT) - 1;
 
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;
    CamDevicePipeSubmoduleCtrl_u SubModuleInit;

    memset(&SubModuleInit, 0, sizeof(SubModuleInit));

    RetVal = MediaIspDeviceSubModuleInit(isp_dev, Port, &SubModuleInit);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev," Port %d Chn %d init submodule failed, ret is %d", Port, Chn, RetVal);
        goto exit;
    }

    RetVal = VsiCamDeviceConnectCamera( isp_dev, IspPort->CamDeviceHandle, &SubModuleInit);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"CamDevice camera connect failed, ret is %d", RetVal);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
    }
exit:
	return RetVal;

}

int visp_buf_done(struct v4l2_subdev *sd, void *arg);

#if 1
int MediaIspHalBufDone(struct v4l2_subdev *sd, int pad, const MediaBuf *Buf)
{
    struct visp_dev *isp_dev =v4l2_get_subdevdata(sd);
    struct visp_buf KernelBuf;
    uint32_t i;
    int RetVal=0;
    if( !Buf || !isp_dev )
    {
        dev_err(isp_dev->dev, "Got a NULL BUFFER BUFFER  \n");
        return -EINVAL;
    } 
    KernelBuf.index = Buf->Index;
    KernelBuf.num_planes = Buf->NumPlanes;
    for (i = 0; i < KernelBuf.num_planes; i++) {
        KernelBuf.planes[i].dma_addr = Buf->Planes[i].DmaAddr;
        KernelBuf.planes[i].size = Buf->Planes[i].DmaSize;
    }
    KernelBuf.pad = pad;


    RetVal = visp_buf_done(sd, &KernelBuf);
    if(RetVal != 0)
    {
        return RetVal;
    }
    return VSI_SUCCESS;
}
#endif

int  Read_DQ_Bufinfo(void *data ,MediaBuffer_t * pMediaBuffer, struct Chn_info *info );
int  Read_DQ_Bufinfo(void *data ,MediaBuffer_t * pMediaBuffer, struct Chn_info *info )
{
    uint8_t *p_data = NULL; 
	uint32_t hw_id_t = 100;
    payload_packet *packet = data; 
    if (!packet) {
        loge("NO Data in DQ Payload%s %d\n", __func__, __LINE__);
        return -ENOMEM;
    }

    p_data = packet->payload;

	#if 1
    memcpy(&(hw_id_t), p_data, sizeof(uint32_t));
    p_data += sizeof(uint32_t);

    memcpy(info, p_data, sizeof(struct Chn_info));
    p_data += sizeof(struct Chn_info);
	#endif	
    pMediaBuffer->pMetaData = kzalloc(sizeof(PicBufMetaData_t), GFP_KERNEL);
   	if(!(pMediaBuffer->pMetaData))
	{	
		pr_err("FAILED TO KZALLOC %s %d\n", __func__, __LINE__);
		return -ENOMEM;
 	}

    memcpy(pMediaBuffer->pMetaData, p_data, sizeof(PicBufMetaData_t));
    p_data += sizeof(PicBufMetaData_t);

    memcpy(&(pMediaBuffer->baseAddress), p_data, sizeof(uint32_t));
    p_data += sizeof(uint32_t);

    memcpy(&(pMediaBuffer->baseSize), p_data, sizeof(uint32_t));
    p_data += sizeof(uint32_t);

    memcpy(&(pMediaBuffer->lockCount),p_data, sizeof(uint32_t));
    p_data += sizeof(uint32_t);

    memcpy( &(pMediaBuffer->isFull),p_data, sizeof(bool_t));
    p_data += sizeof(bool_t);

    memcpy(&(pMediaBuffer->index), p_data, sizeof(uint8_t));
    p_data += sizeof(uint8_t);

    memcpy(&(pMediaBuffer->bufMode), p_data, sizeof(BUFF_MODE));
    p_data += sizeof(BUFF_MODE);

    memcpy(&(pMediaBuffer->pIplAddress), p_data, sizeof(uint32_t));
    p_data += sizeof(uint32_t);

    memcpy(&(pMediaBuffer->pOwner), p_data, sizeof(uint32_t));
    return 0;
}
#if 1
int MediaIspDeviceDqbuf_out(struct visp_dev *isp_dev, struct Chn_info *info, MediaBuf *Buf, void * Packet_from_RPU, MediaBuffer_t *pMediaBuffer);

int MediaIspDeviceDqbuf_out(struct visp_dev *isp_dev, struct Chn_info *info, MediaBuf *Buf, void * Packet_from_RPU, MediaBuffer_t *pMediaBuffer)
{
    int RetVal = VSI_SUCCESS;

	if(!Packet_from_RPU)
  	{
		dev_err(isp_dev->dev, "Received Null data %s %d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	Read_DQ_Bufinfo(Packet_from_RPU, pMediaBuffer, info);
	isp_dev->isp_dq_out_index = pMediaBuffer->index;

	CamDeviceContext_t pCamDevCtx;
	pCamDevCtx.ispHwId = 0;
	pCamDevCtx.ispVtId = 0;
	pCamDevCtx.instanceId =0 ;
	pCamDevCtx.cookie =99;

	RetVal=VsiCamDeviceEnQueBuffer(isp_dev, &pCamDevCtx, CAMDEV_BUFCHAIN_MP, pMediaBuffer);
	if (RetVal != VSI_SUCCESS) {
		dev_err(isp_dev->dev, "VsiCamDeviceEnQueBuffer failed %d", RetVal);
		RetVal = VSI_ERR_TIMEOUT;
		return RetVal;
	}

	return RetVal;
}
EXPORT_SYMBOL(MediaIspDeviceDqbuf_out);
#endif


int MediaIspSetFormat(struct visp_dev *isp_dev, uint32_t pad_index, MediaFmt Format_t)
{
    int Port =pad_index / MEDIA_ISP_PORT_PAD_COUNT;
    int Chn  = (pad_index % MEDIA_ISP_PORT_PAD_COUNT) - 1;


    memcpy(&isp_dev->IspPorts[Port].IspChns[Chn].Format, &Format_t, sizeof(MediaFmt));

    return VSI_SUCCESS;
}

static int MediaIspDeviceGetPortSinkInfo(struct visp_dev *isp_dev, uint8_t Port, MediaSinkInfo *SinkInfo)
{
	int RetVal = VSI_SUCCESS;
	CamDeviceSensorModeInfo_t ModeInfo_s = {0};
	CamDeviceSensorModeInfo_t *ModeInfo=&ModeInfo_s;

    RetVal = MediaIspCalibGetModeInfo(isp_dev, Port, ModeInfo);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: get sensor mode info failed, ret is %d", __func__, RetVal);
        return RetVal;
    }

    SinkInfo->Rect.Top               = ModeInfo->size.top;
    SinkInfo->Rect.Left              = ModeInfo->size.left;
    SinkInfo->Rect.Width             = ModeInfo->size.width;
    SinkInfo->Rect.Height            = ModeInfo->size.height;
    SinkInfo->FrmivalMin.Numerator   = 1;
    SinkInfo->FrmivalMin.Denominator = 1;
    SinkInfo->FrmivalMax.Numerator   = 30;
    SinkInfo->FrmivalMax.Denominator = 1;

	
    switch (ModeInfo->bayerPattern) {
    case CAMDEV_RAW_RGBIR_PAT_RGGIR:
    case CAMDEV_RAW_RGBIR_PAT_RGIRB:
    case CAMDEV_RAW_RGBIR_PAT_IRGGB:
    case CAMDEV_RAW_RGBIR_PAT_RIRGB:
    case CAMDEV_RAW_RGB_PAT_RGGB:
        if (ModeInfo->bitWidth == 8) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SRGGB8;
        } else if (ModeInfo->bitWidth == 10) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SRGGB10;
        } else if (ModeInfo->bitWidth == 12) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SRGGB12;
        } else if (ModeInfo->bitWidth == 14) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SRGGB14;
        } else {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SRGGB16;
        }
        break;
    case CAMDEV_RAW_RGBIR_PAT_GRIRG:
    case CAMDEV_RAW_RGBIR_PAT_GIRBG:
    case CAMDEV_RAW_RGBIR_PAT_GRBIR:
    case CAMDEV_RAW_RGBIR_PAT_IRRBG:
    case CAMDEV_RAW_RGB_PAT_GRBG:
        if (ModeInfo->bitWidth == 8) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SGRBG8;
        } else if (ModeInfo->bitWidth == 10) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SGRBG10;
        } else if (ModeInfo->bitWidth == 12) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SGRBG12;
        } else if (ModeInfo->bitWidth == 14) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SGRBG14;
        } else {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SGRBG16;
        }
        break;
    case CAMDEV_RAW_RGBIR_PAT_GBIRG:
    case CAMDEV_RAW_RGBIR_PAT_GIRRG:
    case CAMDEV_RAW_RGBIR_PAT_IRBRG:
    case CAMDEV_RAW_RGBIR_PAT_GBRIR:
    case CAMDEV_RAW_RGB_PAT_GBRG:
        if (ModeInfo->bitWidth == 8) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SGBRG8;
        } else if (ModeInfo->bitWidth == 10) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SGBRG10;
        } else if (ModeInfo->bitWidth == 12) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SGBRG12;
        } else if (ModeInfo->bitWidth == 14) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SGBRG14;
        } else {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SGBRG16;
        }
        break;
    case CAMDEV_RAW_RGBIR_PAT_BGGIR:
    case CAMDEV_RAW_RGBIR_PAT_IRGGR:
    case CAMDEV_RAW_RGBIR_PAT_BIRGR:
    case CAMDEV_RAW_RGBIR_PAT_BGIRR:
    case CAMDEV_RAW_RGB_PAT_BGGR:
        if (ModeInfo->bitWidth == 8) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SBGGR8;
        } else if (ModeInfo->bitWidth == 10) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SBGGR10;
        } else if (ModeInfo->bitWidth == 12) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SBGGR12;
        } else if (ModeInfo->bitWidth == 14) {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SBGGR14;
        } else {
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SBGGR16;
        }
        break;
    default:
        dev_err(isp_dev->dev ,"Not support pattern %d bitwidth %d\n", ModeInfo->bayerPattern, ModeInfo->bitWidth);
        SinkInfo->Fourcc = MEDIA_PIX_FMT_SRGGB12;
        break;
    }

    return RetVal;
}


int MediaIspCalibQuerySensor(struct visp_dev *isp_dev, uint8_t Port)
{
    int RetVal = VSI_SUCCESS;
    uint8_t SensorMode = 0;
    CamDeviceSensorQuery_t QueryInfo_s = {0};
    CamDeviceSensorQuery_t *QueryInfo = &QueryInfo_s;
    MediaIspPortAttr *IspPort = VSI_NULL;

   if (!isp_dev) {
        dev_err(isp_dev->dev, "%s: null pointer of handle", __func__);
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }

    IspPort = &isp_dev->IspPorts[Port];

    RetVal = MediaIspCalibGetSensorMode(isp_dev, Port, &SensorMode);
    if (RetVal != VSI_SUCCESS) {
	    kfree(QueryInfo);
        dev_err(isp_dev->dev,"%s: port %d get sensor mode failed", __func__, Port);
        return RetVal;
    }

    RetVal = VsiCamDeviceSensorQuery(isp_dev,IspPort->CamDeviceHandle, QueryInfo);
    if (RetVal != VSI_SUCCESS) {
	    kfree(QueryInfo);
        dev_err(isp_dev->dev,"%s: port %d CamDevice query sensor info failed %d", __func__, Port, RetVal);
        RetVal = VSI_ERR_NOTREADY;
        return RetVal;
    }

    if (SensorMode >= QueryInfo->number) {
	    kfree(QueryInfo);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
        dev_err(isp_dev->dev,"%s: port %d sensor mode %d out of range [0, %d]", __func__, Port, SensorMode, QueryInfo->number - 1);
        return RetVal;
    }

    memcpy(&IspPort->SensorInfo.ModeInfo, &QueryInfo->sensorModeInfo[SensorMode], sizeof(QueryInfo->sensorModeInfo[SensorMode]));

	IspPort->SensorInfo.FrameRate = IspPort->SensorInfo.ModeInfo.maxFps;

    return RetVal;
}

int MediaIspCalibLoadIspConfig(struct visp_dev *isp_dev, uint8_t Port)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = VSI_NULL;
    char DevName[MEDIA_ISP_CHAR_LENGTH_MAX];

    if (!isp_dev) {
        dev_err(isp_dev->dev,"%s: null pointer of handle", __func__);
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }

    IspPort = &isp_dev->IspPorts[Port];

	snprintf(DevName,sizeof(DevName), "/proc/vsi/isp_subdev%d", isp_dev->id);
	dev_err(isp_dev->dev,"%s: parse %s info:", __func__, DevName);
	dev_err(isp_dev->dev,"%s: isp : %d", __func__, isp_dev->id);
	dev_err(isp_dev->dev,"%s: port: %u", __func__, Port);
	dev_err(isp_dev->dev,"%s: name: %s ", __func__, IspPort->SensorInfo.Name);
	dev_err(isp_dev->dev,"SenNAme : %s\n",IspPort->SensorInfo.Name);


    dev_err(isp_dev->dev,"%s: mode: %u", __func__, IspPort->SensorInfo.Mode);
    dev_err(isp_dev->dev,"%s: xml : %s", __func__, IspPort->SensorInfo.CalibXml);
    dev_err(isp_dev->dev,"%s: manu_json: %s", __func__, IspPort->SensorInfo.ManuJson);
    dev_err(isp_dev->dev,"%s: auto_json: %s", __func__, IspPort->SensorInfo.AutoJson);
    dev_err(isp_dev->dev,"%zu %zu %zu\n",strlen(IspPort->SensorInfo.CalibXml),strlen(IspPort->SensorInfo.ManuJson),strlen(IspPort->SensorInfo.AutoJson));

    dev_err(isp_dev->dev,"%s: sensor_id: %u", __func__, IspPort->SensorInfo.sensor_id);
	dev_err(isp_dev->dev,"%s: port: %u", __func__, Port);
    dev_err(isp_dev->dev,"%s: buf_mode: %s", __func__, IspPort->bufmode);
	dev_err(isp_dev->dev,"%s: port: %u", __func__, Port);



    return RetVal;
}

int MediaIspSetFrameRate(struct visp_dev *isp_dev, int Pad, /*float * */uint32_t *FrameRate)
{
    int Port = Pad / MEDIA_ISP_PORT_PAD_COUNT;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];

    if (*(uint32_t *)FrameRate < /*1.0f*/1 || *(uint32_t *)FrameRate > IspPort->SensorInfo.ModeInfo.maxFps) {
        dev_info(isp_dev->dev , " Port %d set FrameRate %d Out of range (1 ~ %d)", Port, *FrameRate, IspPort->SensorInfo.ModeInfo.maxFps);
        *FrameRate = (*(uint32_t *)FrameRate < 1) ? /*1.0f*/1 : IspPort->SensorInfo.ModeInfo.maxFps;
    }   
    IspPort->SensorInfo.FrameRate = *FrameRate;

    return 0;
}


int visp_set_frame_interval_public( struct visp_dev *isp_dev, struct v4l2_subdev_frame_interval *fi);
int MediaIspHalSetFrameRate(struct visp_dev *isp_dev, int Pad, /*float **/uint32_t *FrameRate);
int MediaIspHalSetFrameRate(struct visp_dev *isp_dev, int Pad, /*float **/uint32_t *FrameRate)
{
    struct v4l2_subdev_frame_interval SdFi;

    memset(&SdFi, 0, sizeof(SdFi));
    SdFi.pad = Pad;
    SdFi.interval.denominator = (uint32_t)*FrameRate;
    SdFi.interval.numerator = 1;
	visp_set_frame_interval_public(isp_dev, &SdFi);

    return VSI_SUCCESS;
}


int IspDeviceCreate(struct visp_dev *isp_dev , uint8_t Port)
{
	return 0;

	int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    CamDeviceConfig_t CamConfig;
    MediaFmt *Format=NULL;
    CamDeviceSensorDrvHandle_t SensorDrvHandle = NULL;
	CamDeviceSensorDrvCfg_t devSensorDrv = {NULL, 0};
    char SensorName[MEDIA_ISP_CHAR_LENGTH_MAX];
    
    /*Enter Port Level Critical Section */



    memset(&CamConfig, 0 ,sizeof(CamConfig)); //

    CamConfig.ispHwId = 0; //isp_dev->id; 
    CamConfig.inputCfg.inputType = CAMDEV_INPUT_TYPE_IMAGE ; //CAMDEV_INPUT_TYPE_SENSOR; 
    if (IspPort->CamDeviceHandle) {
        return VSI_SUCCESS;
    }

   	if (isp_dev->PortsMask != 0x01) {
       	CamConfig.workCfg.workMode = CAMDEV_WORK_MODE_RDMA;
       	CamConfig.workCfg.modeCfg.mcm.portId = Port+1;     //"1:CAMDEV_MCM_PORT_0, 2:CAMDEV_MCM_PORT_1, ..."
       	CamConfig.workCfg.modeCfg.mcm.mcmOp = 1;             //"1:CAMDEV_MCM_OP_SW, 2:CAMDEV_MCM_OP_HW"
 	}
 	else {
       	CamConfig.workCfg.workMode = CAMDEV_WORK_MODE_RDMA;
       	CamConfig.workCfg.modeCfg.stream.portId = Port + 1;  //"1:CAMDEV_MCM_PORT_0, 2:CAMDEV_MCM_PORT_1, ..."
        dev_err(isp_dev->dev, "%s %d port = %d %d\n",__func__,__LINE__,Port,CamConfig.workCfg.modeCfg.stream.portId);
    }

	CamConfig.outputCfg.outputType = CAMDEV_OUTPUT_TYPE_MEMORY;
    CamConfig.priority = CAMDEV_SEQ_PRI_0;

    /****CamDeviceCreate*****/
    RetVal = VsiCamDeviceCreate(isp_dev, &CamConfig, &IspPort->CamDeviceHandle);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"CamDevice Creat Isp Device Handle Failed, ret is %d", RetVal);
        RetVal = VSI_ERR_TIMEOUT;
        return RetVal;
    }

    /*****Map the sensor*****/
    RetVal = VsiCamDeviceSensorMapping(isp_dev, IspPort->CamDeviceHandle, SensorName, &SensorDrvHandle);
    if(RetVal != VSI_SUCCESS || SensorDrvHandle == VSI_NULL) {
        dev_err(isp_dev->dev ,"CamDevice sensor mapping %s Failed ret is %d", SensorName, RetVal);
        RetVal = VSI_ERR_TIMEOUT;
        goto ERR_TO_DESTROY_CAMDEVICE_HANDLE;
    }


    devSensorDrv.sensorDrvHandle = SensorDrvHandle;
    devSensorDrv.sensorDevId     = IspPort->SensorInfo.sensor_id ;

    /***** FMC Config *****/
    RetVal = VsiCamDeviceSensorDrvHandleRegister(isp_dev, IspPort->CamDeviceHandle, /*SensorDrvHandle*/&devSensorDrv);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"CamDevice register sensor %s driver Failed, ret is %d", /*SensorPortInfo.name*/ SensorName, RetVal);
        RetVal = VSI_ERR_TIMEOUT;
        goto ERR_TO_DESTROY_CAMDEVICE_HANDLE;
    }

    kfree(devSensorDrv.sensorDrvHandle);
    
    /***** Query Sensor *****/
    RetVal = MediaIspCalibQuerySensor(isp_dev, Port);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: query sensor failed %d", __func__, RetVal);
        goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
    }

    /***** Configure sink port *****/
    RetVal = MediaIspDeviceGetPortSinkInfo(isp_dev, Port, &IspPort->SinkInfo);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: get port sink info failed %d", __func__, RetVal);
        goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
    }

    Format= kmalloc(sizeof(MediaFmt), GFP_KERNEL);
    if(!Format)
    {
        dev_err(isp_dev->dev,"FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
        RetVal = -ENOMEM;
        goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
    } 

    memset(Format, 0, sizeof(MediaFmt));
    Format->Width       = IspPort->SinkInfo.Rect.Width;
    Format->Height      = IspPort->SinkInfo.Rect.Height;
    Format->PixelFormat = IspPort->SinkInfo.Fourcc;

    RetVal= MediaIspHalSetFmt(isp_dev , Port * MEDIA_ISP_PORT_PAD_COUNT , Format);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: MediaIspHalSetFmt failed %d", __func__, RetVal);
        goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
    }

    /***** Set Frame Rate *****/
	RetVal =  MediaIspHalSetFrameRate(isp_dev, Port * MEDIA_ISP_PORT_PAD_COUNT, &IspPort->SensorInfo.FrameRate);
	if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: MediaIspHalSetFramerate failed %d", __func__, RetVal);
        goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
    }

   	kfree(Format);
    /*Exit Port Level Critical Section */
	IBA_init_send_command(isp_dev,IspPort->CamDeviceHandle);

	return RetVal;

ERR_TO_UNREGISTER_SENSOR_HANDLE:
  	VsiCamDeviceSensorDrvHandleUnRegister( isp_dev , IspPort->CamDeviceHandle);
ERR_TO_DESTROY_CAMDEVICE_HANDLE:
	VsiCamDeviceDestroy(isp_dev, IspPort->CamDeviceHandle);
	IspPort->CamDeviceHandle = VSI_NULL;
	return RetVal;
}

