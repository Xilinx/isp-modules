#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <linux/delay.h>
#include "vvcam_isp_event.h"
#include "vvcam_isp_driver.h"
#include "media_isp.h"
#include "media_isp_device.h"
#include "media_isp_calib.h"
#include <linux/string.h>
#include <linux/slab.h>
#include "cam_device_api.h"
#include "cam_device_buffer_api.h"
#include <linux/delay.h>
#include "sensor_cmd.h"

#define MEDIA_ISP_MCM_BUF_COUNT         4
#define MEDIA_ISP_EMPTY_BUF_WAIT_TIME   10
#define MEDIA_ISP_FULL_BUF_WAIT_TIME    120
extern int  __IspFd; 

static int MediaIspDeviceDestroyBufPool(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn) 
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;

    VsiCamDeviceReleaseBufMgmt(isp_dev , IspPort->CamDeviceHandle, Chn);
    VsiCamDeviceDestroyBufPool(isp_dev , IspPort->CamDeviceHandle, Chn);

    // release mcm buf pool by camdevice reserved memory
    if (Chn == CAMDEV_BUFCHAIN_RDMA) {
	int i=0;
    	for (i = 0; i < IspPort->McmAttr.NumBufs; i++) {
    	    VsiCamDeviceFreeResMemory( isp_dev, IspPort->CamDeviceHandle,
    	    IspPort->McmAttr.BufAddrs[i]);
            IspPort->McmAttr.BufAddrs[i] = 0; 
        }    
    }    

    VsiCamDeviceDeInitBufChain(isp_dev, IspPort->CamDeviceHandle, Chn);

    return RetVal;
}





int MediaIspDeviceStreamOff( struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{   
    int RetVal  = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];

    CamDevicePathStreamingCfg_t PathStatus;
    VsiCamDeviceGetPathStreaming(isp_dev , IspPort->CamDeviceHandle, &PathStatus);
    PathStatus.outPathEnable &=  ~(1 << Chn);
    RetVal = VsiCamDeviceSetPathStreaming(isp_dev , IspPort->CamDeviceHandle, &PathStatus);
    MediaIspDeviceDestroyBufPool(isp_dev, Port, Chn);

    return RetVal;
}   


#if 1
static int MediaIspDeviceUnRegister3aLib(MediaIspPortAttr *IspPort, uint8_t Port, uint8_t Chn)
{

    //VsiCamDeviceAeDisable(IspPort->CamDeviceHandle);   //RKC-UNCOMMMENT
    //VsiCamDeviceUnRegisterAeLib(IspPort->CamDeviceHandle);  //RKC-UNCOMMENT
    //VsiCamDeviceAwbDisable(IspPort->CamDeviceHandle);    //UNCOMMENT
    //VsiCamDeviceUnRegisterAwbLib(IspPort->CamDeviceHandle);  //UNCOMMENT
    // VsiCamDeviceAfDisable(IspPort->CamDeviceHandle);
    // VsiCamDeviceAfUnRegister(IspPort->CamDeviceHandle);
    //
    return VSI_SUCCESS;
}
                                
static int MediaIspDeviceSensorClose(  struct vvcam_isp_dev *isp_dev  , uint8_t Port)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort=&isp_dev->IspPorts[Port];
    RetVal = VsiCamDeviceSensorClose( isp_dev,  IspPort->CamDeviceHandle);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "CamDevice close sensor failed, ret is %d", RetVal);
        RetVal = VSI_ERR_TIMEOUT;
    }

    return RetVal;
}
#if 0
static int MediaIspDeviceMcmTerminate( struct vvcam_isp_dev *isp_dev,
                                      uint8_t Port)
{
    int RetVal = VSI_SUCCESS;
    //    MediaIspDeviceMcmDestroyBufPool(IspEventDev, Port);
    uint8_t Chn = CAMDEV_BUFCHAIN_RDMA;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];

    MediaIspDeviceDestroyBufPool(isp_dev, Port, Chn);
    IspPort->McmAttr.NumBufs = 0;

    return RetVal;
}
#endif


int MediaIspDeviceCameraDisConnect(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
#if 1
    if (IspPort->CameraConnectRefCnt > 0) {
        IspPort->CameraConnectRefCnt--;
    }

    if (IspPort->CameraConnectRefCnt == 0) {
        MediaIspDeviceUnRegister3aLib(IspPort, Port, Chn);
        VsiCamDeviceDisconnectCamera(isp_dev, IspPort->CamDeviceHandle);
      //  if (IspEventDev->PortsMask != 0x01) //RANJITH Enable the logic during MCM
		 {
//            MediaIspDeviceMcmTerminate(isp_dev, Port);
        }
        MediaIspDeviceSensorClose(isp_dev, Port);
    }
#endif
    return VSI_SUCCESS;
}

#endif


int MediaIspDeviceCreateBufPool(struct vvcam_isp_dev *isp_dev,
                                        uint8_t Port, uint8_t Chn)
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;
    int i = 0;
    int NumBufs = 0;

    if (Chn < MEDIA_ISP_CHN_MAX) {
        NumBufs = IspPort->IspChns[Chn].NumBufs;
    } else {
        NumBufs = IspPort->McmAttr.NumBufs;
    }
	/**** STEP 1.INIT BUF CHAIN **********/
    CamDeviceBufChainConfig_t BufferChain;
    memset(&BufferChain, 0, sizeof(CamDeviceBufChainConfig_t));
    BufferChain.skipInterval = 0;
    BufferChain.bufQueLength = NumBufs;
    BufferChain.emptyQueOp.blockType = CAMDEV_BUFQUE_TIMEOUT_TYPE;
    BufferChain.emptyQueOp.waitTime  = MEDIA_ISP_EMPTY_BUF_WAIT_TIME;
    BufferChain.fullQueOp.blockType  = CAMDEV_BUFQUE_TIMEOUT_TYPE;
    BufferChain.fullQueOp.waitTime   = MEDIA_ISP_FULL_BUF_WAIT_TIME;

    RetVal = VsiCamDeviceInitBufChain(isp_dev , IspPort->CamDeviceHandle,
        Chn, &BufferChain);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "%s: CamDevice init buf chain %d failed, ret is %d",
            __func__, Chn, RetVal);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
        return RetVal;
    }

    /****** STEP 2. RESERVE BUFFERS *******/
    CamDeviceBufPoolConfig_t BufPoolCfg;
    uint32_t BufSize = 0;
    memset(&BufPoolCfg, 0, sizeof(BufPoolCfg));
    BufPoolCfg.bufNum        = NumBufs;
    BufPoolCfg.bufMode       = CAMDEV_BUFMODE_USERPTR;
    BufPoolCfg.is_mapped     = 0;
    BufPoolCfg.pBaseAddrList = kmalloc(NumBufs * sizeof(uint32_t),GFP_KERNEL);
    BufPoolCfg.pIplAddrList  = kmalloc(NumBufs * sizeof(void *),GFP_KERNEL);

    if (Chn < MEDIA_ISP_CHN_MAX) {
		// create isp buf pool by user allocated dma memory
        for (i = 0; i < IspPort->IspChns[Chn].Bufs[0].NumPlanes; i++) {
            BufSize += IspPort->IspChns[Chn].Bufs[0].Planes[i].DmaSize;
        }
        BufPoolCfg.bufSize = BufSize;

        for (i = 0; i < NumBufs; i++) {
            BufPoolCfg.pBaseAddrList[i] =
                IspPort->IspChns[Chn].Bufs[i].Planes[0].DmaAddr;
            BufPoolCfg.pIplAddrList[i] = VSI_NULL;
            printk(KERN_ERR "Buf[%d] = 0x%x size 0x%x", i, BufPoolCfg.pBaseAddrList[i],
                BufSize);
        }
    } else if (Chn == CAMDEV_BUFCHAIN_RDMA) {
        // create mcm buf pool by camdevice reserved memory
        RetVal = VsiCamDeviceGetBufferSize(isp_dev , IspPort->CamDeviceHandle,
            Chn, &BufSize);
        if (RetVal != VSI_SUCCESS) {
            printk(KERN_ERR "%s: CamDevice get chain %d buf size failed, ret is %d",
                __func__, Chn, RetVal);
            RetVal = VSI_ERR_ILLEGAL_PARAM;
            goto ERR_TO_DEINIT_CHAIN;
        }
        BufPoolCfg.bufSize = BufSize;

        uint32_t PhyAddr = 0;
        uint32_t *pIplAddr = VSI_NULL;
        for (i = 0; i < NumBufs; i++) {
            RetVal = VsiCamDeviceAllocResMemory( isp_dev , IspPort->CamDeviceHandle,
                BufSize, &PhyAddr, (void**)&pIplAddr);
            if (RetVal != VSI_SUCCESS) {
                printk(KERN_ERR "%s: CamDevice chain %d alloc buf[%d] failed, ret is %d",
                    __func__, Chn, i, RetVal);
                RetVal = VSI_ERR_ILLEGAL_PARAM;
                goto ERR_TO_RELEASE_MEM;
            }

            BufPoolCfg.pBaseAddrList[i]  = PhyAddr;
            BufPoolCfg.pIplAddrList[i]   = (void *)pIplAddr;
            IspPort->McmAttr.BufAddrs[i] = PhyAddr;


	#if 0
            pr_err(KERN_ERR "%s: Alloc mcm buffer[%d]: Phy_Addr:0x%x, size:0x%x",
                __func__, i, PhyAddr, BufSize);
            pr_err("%s: Alloc mcm buffer[%d]: Ipl_Addr:%p",
                __func__, i, (void *)pIplAddr);
	#endif
        }
    } else {
        printk(KERN_ERR "%s: current not support chn %d to create buf pool",
            __func__, Chn);
        RetVal = VSI_ERR_NOT_SUPPORT;
        goto ERR_TO_DEINIT_CHAIN;
    }
	/************ STEP 3. CREATE BUFFER POOL ****************/
    RetVal = VsiCamDeviceCreateBufPool(isp_dev , IspPort->CamDeviceHandle,
        Chn, &BufPoolCfg);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "CamDevice chn %d create buf pool failed, ret is %d",
            Chn, RetVal);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
        goto ERR_TO_RELEASE_MEM;
    }
    /************ STEP 4. SETUP BUF MGMT *******************/
    RetVal = VsiCamDeviceSetupBufMgmt(isp_dev, IspPort->CamDeviceHandle, Chn);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "CamDevice chn %d setup buf managementfailed, ret is %d",
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
        for (--i; i >=0; --i) {
            VsiCamDeviceFreeResMemory(isp_dev , IspPort->CamDeviceHandle,
                BufPoolCfg.pBaseAddrList[i]);
        }
    }
ERR_TO_DEINIT_CHAIN:
    kfree(BufPoolCfg.pIplAddrList);
    kfree(BufPoolCfg.pBaseAddrList);
    VsiCamDeviceDeInitBufChain(isp_dev , IspPort->CamDeviceHandle, Chn);

    return RetVal;
}



//int STREAM_ON_FLAG=0;
int MediaIspDeviceStreamOn(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;

    RetVal = MediaIspDeviceCreateBufPool(isp_dev, Port, Chn);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "%s: Port %d Chn %d create buf pool failed, ret is %d",
            __func__, Port, Chn, RetVal);
        return RetVal;
    }

    CamDevicePathStreamingCfg_t PathStatus;
    VsiCamDeviceGetPathStreaming(isp_dev, IspPort->CamDeviceHandle, &PathStatus);
    PathStatus.outPathEnable |= (1 << Chn);


    RetVal = VsiCamDeviceSetPathStreaming(isp_dev,IspPort->CamDeviceHandle, &PathStatus);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "Port %d Chn %d CamDevice start stream failed, ret is %d",
            Port, Chn, RetVal);
        RetVal = VSI_ERR_NOTREADY;
        goto ERR_TO_DESTROY_BUFPOOL;
    }
    int pad_index= (Port*MEDIA_ISP_PORT_PAD_COUNT)+ Chn+1;
    isp_dev->streamon[pad_index]=1;
    //STREAM_ON_FLAG=1;
    return RetVal;


ERR_TO_DESTROY_BUFPOOL:
	MediaIspDeviceDestroyBufPool(isp_dev, Port, Chn);

    return RetVal;
}

  
int MediaIspStreamOff(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{       

    MediaIspDeviceStreamOff(isp_dev, Port, Chn);
    MediaIspDeviceCameraDisConnect(isp_dev, Port, Chn);
    
    return VSI_SUCCESS;
}
    



int MediaIspDeviceQbuf(struct vvcam_isp_dev *isp_dev , uint8_t Port, uint8_t Chn, MediaBuf *Buf)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort   = &isp_dev->IspPorts[Port];
    MediaIspChnAttr  *IspChn    = &IspPort->IspChns[Chn];
    MediaBuffer_t *pMediaBuffer = VSI_NULL;
    pMediaBuffer = IspChn->CamDeviceBufs[Buf->Index];

    if (pMediaBuffer == VSI_NULL) {
        printk(KERN_ERR "CamDevice queue buf is null");
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }    

    RetVal = VsiCamDeviceEnQueBuffer(isp_dev, IspPort->CamDeviceHandle, Chn, pMediaBuffer);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "CamDevice queue buf failed, ret is %d", RetVal);
        RetVal = VSI_ERR_TIMEOUT;
        return RetVal;
    }    
//  IspChn->CamDeviceBufs[pMediaBuffer->index] = VSI_NULL;
  
      return RetVal;
}


int MediaIspQBuf(struct vvcam_isp_dev *isp_dev, int Pad_index , MediaBuf *Buf)
{
    int RetVal = VSI_SUCCESS;
    int Port = Pad_index / MEDIA_ISP_PORT_PAD_COUNT;
    int Chn  = (Pad_index % MEDIA_ISP_PORT_PAD_COUNT) - 1;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    MediaIspChnAttr  *IspChn  = &IspPort->IspChns[Chn];

    if(Buf==NULL)//RANJTIH Repleace this with kernel buffer done thread status
    {
	printk(KERN_ERR "got NULL BUFFER\n");
	return -1;
    }
#if 0
#endif


    if (/*IspChn->ThreadStatus == MEDIA_THREAD_STOPPED*/isp_dev->streamon[Pad_index]==0) {
        memcpy(&IspChn->Bufs[Buf->Index], Buf, sizeof(MediaBuf));
    } else {
        RetVal = MediaIspDeviceQbuf(isp_dev, Port, Chn, Buf);
        if (RetVal != VSI_SUCCESS) {
            ///***RKC-ERR***/printk(KERN_ERR "%/s Port %d Chn %d Qbuf failed, ret is %d",
              //  /*MediaEntity->DevName,*/ Port, Chn, RetVal);
        }
    }
    return RetVal;
}

int MediaIspHalMbusFmtToMediaFmt(uint32_t *Code, uint32_t *PixelFormat)
{
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
        case MEDIA_BUS_FMT_SBGGR10_1X10:
            *PixelFormat = MEDIA_PIX_FMT_SBGGR10;
            break;
        case MEDIA_BUS_FMT_SGBRG10_1X10:
            *PixelFormat = MEDIA_PIX_FMT_SGBRG10;
            break;
        case MEDIA_BUS_FMT_SGRBG10_1X10:
            *PixelFormat = MEDIA_PIX_FMT_SGRBG10;
            break;
        case MEDIA_BUS_FMT_SRGGB10_1X10:
            *PixelFormat = MEDIA_PIX_FMT_SRGGB10;
            break;
        case MEDIA_BUS_FMT_SBGGR12_1X12:
            *PixelFormat = MEDIA_PIX_FMT_SBGGR12;
            break;
        case MEDIA_BUS_FMT_SGBRG12_1X12:
            *PixelFormat = MEDIA_PIX_FMT_SGBRG12;
            break;
        case MEDIA_BUS_FMT_SGRBG12_1X12:
            *PixelFormat = MEDIA_PIX_FMT_SGRBG12;
            break;
        case MEDIA_BUS_FMT_SRGGB12_1X12:
            *PixelFormat = MEDIA_PIX_FMT_SRGGB12;
            break;
        default:
            return VSI_ERR_ILLEGAL_PARAM;
    }

    return VSI_SUCCESS;
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
        default: 
            return VSI_ERR_ILLEGAL_PARAM; 
    } 
 
    return VSI_SUCCESS; 
} 

//

int vvcam_isp_set_fmt_public(/*struct v4l2_subdev *sd,	struct v4l2_subdev_state *sd_state,*/struct vvcam_isp_dev *isp_dev,struct v4l2_subdev_format *format);

int MediaIspHalSetFmt(struct vvcam_isp_dev *isp_dev, int Pad, MediaFmt *Format)
{
//  McIspHalHandle *HalHandle = &isp_dev->HalHandle;

    struct v4l2_subdev_format *SdFmt;
    SdFmt= kmalloc(sizeof(SdFmt), GFP_KERNEL);
    if(!SdFmt)
    {
	printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
	return -ENOMEM;
    } 

    memset(SdFmt, 0, sizeof(struct v4l2_subdev_format));

    SdFmt->pad = Pad,
    SdFmt->which = V4L2_SUBDEV_FORMAT_ACTIVE;
    SdFmt->format.width = Format->Width;
    SdFmt->format.height = Format->Height;
    SdFmt->format.colorspace = Format->ColorSpace;
    SdFmt->format.quantization = Format->Quantization;

    MediaIspHalMediaFmtToMBusFmt(&Format->PixelFormat, &SdFmt->format.code);
  //  ioctl(/*HalHandle->IspFd*/__IspFd, VIDIOC_SUBDEV_S_FMT, SdFmt);
    vvcam_isp_set_fmt_public(/*struct v4l2_subdev *sd,struct v4l2_subdev_state *sd_state,*/isp_dev,SdFmt);

    return VSI_SUCCESS;
}




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
        case MEDIA_PIX_FMT_SBGGR12:
        case MEDIA_PIX_FMT_SGBRG12:
        case MEDIA_PIX_FMT_SGRBG12:
        case MEDIA_PIX_FMT_SRGGB12:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW12_ALIGNED_MODE1;
            IspFmt->dataBits  = 12;
            break;
        case MEDIA_PIX_FMT_SBGGR14:
        case MEDIA_PIX_FMT_SGBRG14:
        case MEDIA_PIX_FMT_SGRBG14:
        case MEDIA_PIX_FMT_SRGGB14:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW14_ALIGNED_MODE1;
            IspFmt->dataBits  = 14;
            break;
        case MEDIA_PIX_FMT_SBGGR16:
        case MEDIA_PIX_FMT_SGBRG16:
        case MEDIA_PIX_FMT_SGRBG16:
        case MEDIA_PIX_FMT_SRGGB16:
            IspFmt->outFormat = CAMDEV_PIX_FMT_RAW16;
            IspFmt->dataBits  = 16;
            break;
        default:
            printk(KERN_ERR "Not support format %s", (char *)MediaFmt);
            return VSI_ERR_NOT_SUPPORT;
    }
    return VSI_SUCCESS;
}

int MediaIspDeviceSetFormat(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn)
{
    /************ STEP 2 Streamon --> SetOutFormat******************/
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];// &IspEventDev->IspPorts[Port];

    int RetVal;

    CamDevicePipeOutFmt_t IspFormat;
    IspFormat.outWidth = isp_dev->IspPorts[Port].IspChns[Chn].Format.Width; // Format->Width;
    IspFormat.outHeight =   isp_dev->IspPorts[Port].IspChns[Chn].Format.Height;//  Format->Height;
    RetVal = MediaFmtToIspFmt( &(isp_dev->IspPorts[Port].IspChns[Chn].Format.PixelFormat) , &IspFormat);
    if (RetVal) {
        return RetVal;
    }

    RetVal = VsiCamDeviceSetOutFormat(isp_dev , IspPort->CamDeviceHandle, Chn, &IspFormat);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "CamDevice set format failed, ret is %d", RetVal);
        RetVal = VSI_ERR_TIMEOUT;
        goto ERR_TO_CAMERA_DISCONNECT;
    }

    return RetVal;
ERR_TO_CAMERA_DISCONNECT:
// RANJITH ADD the cleanup later
    return RetVal;
}


int MediaIspCalibGetModeInfo( struct vvcam_isp_dev *isp_dev , uint8_t Port, CamDeviceSensorModeInfo_t *ModeInfo)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = VSI_NULL;

    if (!isp_dev || !ModeInfo) {
        /****ERROR****/printk(KERN_ERR "%s: null pointer of handle", __func__);
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }

    IspPort = &isp_dev->IspPorts[Port];
	memcpy(ModeInfo, &IspPort->SensorInfo.ModeInfo,
        sizeof(IspPort->SensorInfo.ModeInfo));

    return RetVal;
}

static int MediaIspDeviceSubModuleInit(struct vvcam_isp_dev *isp_dev, uint8_t Port,
                                       CamDevicePipeSubmoduleCtrl_u *SubModuleInit)
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
	printk(KERN_ERR "RKC-ISP_APP %s %d \n",__func__,__LINE__);

    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "%s: get sensor mode failed, ret is %d",
            __func__, RetVal);
        return RetVal;
    }

    if (ModeInfo.sensorType == CAMDEV_SENSOR_TYPE_STITCHING_HDR) {
        SubModuleInit->subCtrl.hdrEnable = 1; //RANJITH_REMOCE THIS somehow this is not working in our flow
    }

    return RetVal;
}

int MediaIspCalibGetSensorName(struct vvcam_isp_dev *isp_dev, uint8_t Port, char *SensorName)
{
    MediaIspPortAttr *IspPort = VSI_NULL;

    IspPort = &isp_dev->IspPorts[Port];


#if 0
    /****INFO****/pr_err("%s: isp : %d", __func__, isp_dev->id);
    /****INFO****/pr_err("%s: port: %d", __func__, Port);
    /****INFO****/pr_err("%s: name: %s", __func__, IspPort->SensorInfo.Name);
    /****INFO****/pr_err("%s: mode: %d", __func__, IspPort->SensorInfo.Mode);
    /****INFO****/pr_err("%s: xml : %s", __func__, IspPort->SensorInfo.CalibXml);
    /****INFO****/pr_err("%s: manu_json: %s", __func__, IspPort->SensorInfo.ManuJson);
    /****INFO****/pr_err("%s: auto_json: %s", __func__, IspPort->SensorInfo.AutoJson);
#endif


    int RetVal = VSI_SUCCESS;
    if (!SensorName || !isp_dev) {
        /****ERROR****/printk(KERN_ERR "%s: null pointer of handle", __func__);
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }

    IspPort = &isp_dev->IspPorts[Port];

    if (strlen(IspPort->SensorInfo.Name)) {
        strncpy(SensorName, IspPort->SensorInfo.Name, strlen(IspPort->SensorInfo.Name));
    } else {
        /****ERROR****/printk(KERN_ERR "%s: get null string of sensor name", __func__);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
    }

    return RetVal;
}

int MediaIspCalibGetSensorMode(struct vvcam_isp_dev *isp_dev , uint8_t Port, uint8_t *SensorMode)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = VSI_NULL;

    if (!SensorMode || !isp_dev) {
        /****ERROR****/printk(KERN_ERR "%s: null pointer of handle", __func__);
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }

    IspPort = &isp_dev->IspPorts[Port];
    *SensorMode = IspPort->SensorInfo.Mode;

    return RetVal;
}

int MediaIspDeviceSensorOpen(struct vvcam_isp_dev *isp_dev, uint8_t Port)
{
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;
    uint8_t ModeIndex = 0;
    char SensorName[MEDIA_ISP_CHAR_LENGTH_MAX];

    memset(SensorName, 0, sizeof(SensorName));

    RetVal = MediaIspCalibGetSensorMode(isp_dev, Port, &ModeIndex);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "%s: get sensor mode failed, ret is %d",
            __func__, RetVal);
    }

    RetVal = MediaIspCalibGetSensorName(isp_dev, Port, SensorName);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "%s: get sensor name failed, ret is %d",
            __func__, RetVal);
    }

    RetVal = VsiCamDeviceSensorOpen(isp_dev , IspPort->CamDeviceHandle, ModeIndex);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "CamDevice open sensor %s mode %d driver Failed, ret is %d",
            SensorName, ModeIndex, RetVal);
    }

    return RetVal;
}

//int cam_connect_flag=0;
int MediaIspDeviceCameraConnect(struct vvcam_isp_dev *isp_dev , uint8_t Index)
{
    int Port = Index / MEDIA_ISP_PORT_PAD_COUNT;
 
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    int RetVal = VSI_SUCCESS;

/*    if (IspPort->CameraConnectRefCnt != 0) {
        IspPort->CameraConnectRefCnt++;
        return 0;
    }
*/

    RetVal = MediaIspDeviceSensorOpen(isp_dev, Port);

    if (RetVal != VSI_SUCCESS) {
        goto ERR_TO_RELEASE_CAMCOMMON;
    }

/*
    if (IspEventDev->PortsMask != 0x01) {
        RetVal = MediaIspDeviceMcmInit(IspEventDev, Port);
        if (RetVal != VSI_SUCCESS) {
            printk(KERN_ERR "%s Port %d Chn %d init mcm failed, ret is %d",
                IspEventDev->MediaEntity->DevName, Port, Chn, RetVal);
            goto ERR_TO_CLOSE_SENSOR;
        }
    }
*/
    CamDevicePipeSubmoduleCtrl_u SubModuleInit;
    memset(&SubModuleInit, 0, sizeof(SubModuleInit));

    RetVal = MediaIspDeviceSubModuleInit(isp_dev, Port, &SubModuleInit);
    if (RetVal != VSI_SUCCESS) {
        goto ERR_TO_TERMINATE_MCM;
    }

    RetVal = VsiCamDeviceConnectCamera( isp_dev, IspPort->CamDeviceHandle, &SubModuleInit);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "CamDevice camera connect failed, ret is %d", RetVal);
        RetVal = VSI_ERR_ILLEGAL_PARAM;
        goto ERR_TO_TERMINATE_MCM;
	goto ERR_TO_CLOSE_SENSOR;
    }
/*
*/

//    IspPort->CameraConnectRefCnt++;
//	cam_connect_flag++;
	return RetVal;

//    MediaIspDeviceUnRegister3aLib(IspEventDev, Port, Chn);
 //   VsiCamDeviceDisconnectCamera(IspPort->CamDeviceHandle);
ERR_TO_TERMINATE_MCM:
//    if (isp_dev->PortsMask != 0x01) {
   //     MediaIspDeviceMcmTerminate(IspEventDev, Port);
 //   }
ERR_TO_CLOSE_SENSOR:
	RetVal = VsiCamDeviceSensorClose(isp_dev , IspPort->CamDeviceHandle);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "CamDevice close sensor failed, ret is %d", RetVal);
        RetVal = VSI_ERR_TIMEOUT;
    }

ERR_TO_RELEASE_CAMCOMMON:
//    VsiCamCommonDestroy(&CamCommon);
    return RetVal;
}

int vvcam_isp_buf_done(struct v4l2_subdev *sd, void *arg);

#if 1
int MediaIspHalBufDone(struct v4l2_subdev *sd, int pad, MediaBuf *Buf)
{
    //struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);
    struct vvcam_isp_buf KernelBuf;
    uint32_t i;
    if( !Buf )
	{
		printk(KERN_ERR "Got a NULL BUFFER BUFFER  \n");
		while(1);
	} 
    KernelBuf.index = Buf->Index;
    KernelBuf.num_planes = Buf->NumPlanes;
    for (i = 0; i < KernelBuf.num_planes; i++) {
        KernelBuf.planes[i].dma_addr = Buf->Planes[i].DmaAddr;
        KernelBuf.planes[i].size = Buf->Planes[i].DmaSize;
    }
    KernelBuf.pad = pad;
//    printk(KERN_ERR "%s Pad %d BufDone[%d] Dma 0x%x Size %d\n",__func__,/*MediaEntity->DevName,*/ pad, KernelBuf.index,KernelBuf.planes[0].dma_addr, KernelBuf.planes[0].size);

//    ioctl(HalHandle->IspFd, VVCAM_ISP_IOC_BUFDONE, &KernelBuf);

	vvcam_isp_buf_done(sd, &KernelBuf);
    return VSI_SUCCESS;
}
#endif
#if 0
int MediaIspHalBufDone(struct v4l2_subdev *sd, int pad, MediaBuf *Buf)
{
    struct vvcam_isp_dev *isp_dev = v4l2_get_subdevdata(sd);

    struct vvcam_isp_buf *KernelBuf= kzalloc(sizeof( struct vvcam_isp_buf), GFP_KERNEL);
    if(!KernelBuf)
	{
	pr_err("Got a NULL BUFFER BUFFER %s %d \n",__func__,__LINE__);
	pr_err("Got a NULL BUFFER BUFFER %s %d \n",__func__,__LINE__);
	pr_err("Got a NULL BUFFER BUFFER %s %d \n",__func__,__LINE__);
	while(1);
    }

    uint32_t i;
    if( !Buf )
	{
        pr_err("Got a NULL BUFFER BUFFER %s %d \n",__func__,__LINE__);
	pr_err("Got a NULL BUFFER BUFFER %s %d \n",__func__,__LINE__);
	pr_err("Got a NULL BUFFER BUFFER %s %d \n",__func__,__LINE__);
	while(1);
    } 
    KernelBuf->index = Buf->Index;
    KernelBuf->num_planes = Buf->NumPlanes;
    for (i = 0; i < KernelBuf->num_planes; i++) {
        KernelBuf->planes[i].dma_addr = Buf->Planes[i].DmaAddr;
        KernelBuf->planes[i].size = Buf->Planes[i].DmaSize;
    }
    KernelBuf->pad = pad;
    int ret = vvcam_isp_buf_done(sd, KernelBuf);
    if(ret!=0)
	{
	    pr_err("Failed to VB2 PUSH FAILED....! %d \n",ret);
	}
    return VSI_SUCCESS;
}
#endif
//extern volatile void * Enque_Buff_G;

extern volatile int DQ_BUF_AVAILABLE;


int   Read_DQ_Bufinfo(void *data ,MediaBuffer_t * Enque_Buff_L ){

        MediaBuffer_t Display_Mediabuff_G;
	payload_packet *packet=data; 
        if (!packet) {
                kfree(packet);
                return -ENOMEM;
        }

        uint8_t *p_data = packet->payload;
        memcpy( Display_Mediabuff_G.pMetaData  , p_data, sizeof(PicBufMetaData_t));
        p_data += sizeof(PicBufMetaData_t);
        memcpy( &(Display_Mediabuff_G.baseAddress), p_data, sizeof(uint32_t));
        p_data += sizeof(uint32_t);

        memcpy(&(Display_Mediabuff_G.baseSize), p_data, sizeof(uint32_t));
        p_data += sizeof(uint32_t);

        memcpy(&(Display_Mediabuff_G.lockCount),p_data, sizeof(uint32_t));
        p_data += sizeof(uint32_t);

        memcpy( &(Display_Mediabuff_G.isFull),p_data, sizeof(bool_t));
        p_data += sizeof(bool_t);

        memcpy(&(Display_Mediabuff_G.index), p_data, sizeof(uint8_t));
        p_data += sizeof(uint8_t);

        memcpy(&(Display_Mediabuff_G.bufMode), p_data, sizeof(BUFF_MODE));
        p_data += sizeof(BUFF_MODE);

        memcpy(&(Display_Mediabuff_G.pIplAddress), p_data, sizeof(uint32_t));
        p_data += sizeof(uint32_t);

        memcpy(&(Display_Mediabuff_G.pOwner), p_data, sizeof(uint32_t));

        Enque_Buff_L->baseAddress=Display_Mediabuff_G.baseAddress;
        Enque_Buff_L->baseSize=Display_Mediabuff_G.baseSize;
        Enque_Buff_L->buf=Display_Mediabuff_G.buf;
        Enque_Buff_L->bufMode=Display_Mediabuff_G.bufMode;
        Enque_Buff_L->index=Display_Mediabuff_G.index;
        Enque_Buff_L->isFull=Display_Mediabuff_G.isFull;
        Enque_Buff_L->lockCount=Display_Mediabuff_G.lockCount;
        Enque_Buff_L->pIplAddress=Display_Mediabuff_G.pIplAddress;
        Enque_Buff_L->pMetaData=Display_Mediabuff_G.pMetaData;
        Enque_Buff_L->pOwner=Display_Mediabuff_G.pOwner;

        return 0;
}

#if 0
int MediaIspDeviceDqbuf(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn, MediaBuf *Buf)
{     
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort   = &isp_dev->IspPorts[Port];
    MediaIspChnAttr  *IspChn    = &IspPort->IspChns[Chn];
    MediaBuffer_t *pMediaBuffer = VSI_NULL;
    RetVal = VsiCamDeviceDeQueBuffer(isp_dev , IspPort->CamDeviceHandle, Chn, &pMediaBuffer);
    if (RetVal != VSI_SUCCESS) {
        pr_err("CamDevice dequeue buf failed, ret is %d", RetVal);
        RetVal = VSI_ERR_TIMEOUT;
        return RetVal;
    }
    /*    pr_err("Media buf base_addr 0x%x powner 0x%x index %d\n",__func__,__LINE__,pMediaBuffer->baseAddress,pMediaBuffer->pOwner,pMediaBuffer->index);*/
    /*  pr_err("Media IPL addr 0x%x 0x%p \n",__func__,__LINE__,pMediaBuffer->pIplAddress,pMediaBuffer->pIplAddress);*/

    IspChn->CamDeviceBufs[pMediaBuffer->index] = pMediaBuffer;
    /*  pr_err("pMediaBuffer_addr 0x%x pMediaBuffer_addr 0x%x   IspChn->CamDeviceBufs[pMediaBuffer->index 0x%x\n]",pMediaBuffer,pMediaBuffer->baseAddress,IspChn->CamDeviceBufs[pMediaBuffer->index]);*/

      /*  pr_err("%s Port %d Chn %d DQBuf[%d] pMediaBuffer %p",__func__, Port, Chn,     pMediaBuffer->index,IspChn->CamDeviceBufs[pMediaBuffer->index]);*/
    memcpy(Buf, &IspChn->Bufs[pMediaBuffer->index], sizeof(MediaBuf));
    /*   pr_err("ADDR=%x\n",pMediaBuffer);      */
    /*  kfree(pMediaBuffer);//RANJITH check if free is done elsewhere/original flow */
    return RetVal;
}

#endif


//
#if 1
int MediaIspDeviceDqbuf(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn, MediaBuf *Buf,void * Packet_from_RPU , MediaBuffer_t **Temp_DQ_BUF)
{       int RetVal = VSI_SUCCESS;
	MediaIspPortAttr *IspPort   = &isp_dev->IspPorts[Port];
	MediaIspChnAttr  *IspChn    = &IspPort->IspChns[Chn];
    	MediaBuffer_t *pMediaBuffer = VSI_NULL;

    	pMediaBuffer = kmalloc(sizeof(MediaBuffer_t),GFP_KERNEL);


   	if(!pMediaBuffer)
	{	
		printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
		return -ENOMEM;
 	} 
	if(!Packet_from_RPU)
  	{
		printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
		return -ENOMEM;
	}
	Read_DQ_Bufinfo(Packet_from_RPU,pMediaBuffer);

	IspChn->CamDeviceBufs[pMediaBuffer->index] = pMediaBuffer;

    	memcpy(Buf, &IspChn->Bufs[pMediaBuffer->index], sizeof(MediaBuf));

        //  kfree(pMediaBuffer);//RKC check if free is done elsewhere/original flow 
         
	*Temp_DQ_BUF=pMediaBuffer;
	
	return RetVal;
}
#endif

#if 0
int MediaIspStreamOn(struct vvcam_isp_dev *isp_dev , int Index /*  MediaPadAttr *Pad*/)
{
ERR_TO_DESTROY_BUFPOOL:
    VsiCamDeviceDestroyBufPool(IspPort->CamDeviceHandle, Chn);
ERR_TO_RELEASE_MEM:
    if (Chn == CAMDEV_BUFCHAIN_RDMA) {
        for (--i; i >=0; --i) {
            VsiCamDeviceFreeResMemory(IspPort->CamDeviceHandle,
                BufPoolCfg.pBaseAddrList[i]);
        }
    }
ERR_TO_DEINIT_CHAIN:
    kfree(BufPoolCfg.pIplAddrList);
    kfree(BufPoolCfg.pBaseAddrList);
    VsiCamDeviceDeInitBufChain(IspPort->CamDeviceHandle, Chn);
    return VSI_SUCCESS;

ERR_TO_STOP_STREAM:
	MediaIspDeviceStreamOff(isp_dev, Port, Chn);
ERR_TO_CAMERA_DISCONNECT:
//	MediaIspDeviceCameraDisConnect(isp_dev, Port, Chn);
	return RetVal;
}
#endif

int MediaIspSetFormat( struct vvcam_isp_dev *isp_dev,uint32_t pad_index,/*  MediaPadAttr *Pad,*/ MediaFmt Format_t)
{
    int Port =/* Pad->Index*/pad_index / MEDIA_ISP_PORT_PAD_COUNT;
    int Chn  = (/*Pad->Index*/pad_index % MEDIA_ISP_PORT_PAD_COUNT) - 1;

    memcpy(&isp_dev->IspPorts[Port].IspChns[Chn].Format, &Format_t, sizeof(MediaFmt));

    return VSI_SUCCESS;
}

static int MediaIspDeviceGetPortSinkInfo(struct vvcam_isp_dev *isp_dev,
                                    uint8_t Port, MediaSinkInfo *SinkInfo)
{
    int RetVal = VSI_SUCCESS;
    CamDeviceSensorModeInfo_t *ModeInfo;
    ModeInfo= kmalloc(sizeof(CamDeviceSensorModeInfo_t), GFP_KERNEL);
    if(!ModeInfo)
    {
		printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
		return -ENOMEM;
    } 

    memset(ModeInfo, 0, sizeof(CamDeviceSensorModeInfo_t));

    RetVal = MediaIspCalibGetModeInfo(isp_dev, Port, ModeInfo);
    if (RetVal != VSI_SUCCESS) {
        printk(KERN_ERR "%s: get sensor mode info failed, ret is %d",
            __func__, RetVal);
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
            printk(KERN_ERR "Not support pattern %d bitwidth %d\n", ModeInfo->bayerPattern, ModeInfo->bitWidth);
            SinkInfo->Fourcc = MEDIA_PIX_FMT_SRGGB12;
            break;
    }

    return RetVal;
}


int MediaIspCalibQuerySensor(struct vvcam_isp_dev *isp_dev, uint8_t Port)
{
    int RetVal = VSI_SUCCESS;
    uint8_t SensorMode = 0;
    CamDeviceSensorQuery_t *QueryInfo;
    QueryInfo= kmalloc(sizeof(QueryInfo), GFP_KERNEL);
    if(!QueryInfo)
    {
	printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
	return -ENOMEM;
    } 
    MediaIspPortAttr *IspPort = VSI_NULL;

    if (!isp_dev) {
        /****ERROR****/printk(KERN_ERR "%s: null pointer of handle", __func__);
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }

    memset(QueryInfo, 0, sizeof(CamDeviceSensorQuery_t ));
    IspPort = &isp_dev->IspPorts[Port];

    RetVal = MediaIspCalibGetSensorMode(isp_dev, Port, &SensorMode);
    if (RetVal != VSI_SUCCESS) {
        /****ERROR****/printk(KERN_ERR "%s: port %d get sensor mode failed", __func__, Port);
        return RetVal;
    }

    RetVal = VsiCamDeviceSensorQuery(isp_dev,IspPort->CamDeviceHandle, QueryInfo);
    if (RetVal != VSI_SUCCESS) {
        /****ERROR****/printk(KERN_ERR "%s: port %d CamDevice query sensor info failed %d",
            __func__, Port, RetVal);
        RetVal = VSI_ERR_NOTREADY;
        return RetVal;
    }

    if (SensorMode >= QueryInfo->number) {
        RetVal = VSI_ERR_ILLEGAL_PARAM;
        /****ERROR****/printk(KERN_ERR "%s: port %d sensor mode %d out of range [0, %d]",
            __func__, Port, SensorMode, QueryInfo->number - 1);
        return RetVal;
    }

    //memcpy(&IspPort->SensorInfo.ModeInfo, &(QueryInfo->sensorModeInfo[SensorMode]),
      //  sizeof(QueryInfo->sensorModeInfo[SensorMode]));
	kfree(QueryInfo);
    return RetVal;
}


int MediaIspCalibLoadSensorInfo(struct vvcam_isp_dev *isp_dev, uint8_t Port)
{
    int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = VSI_NULL;
    char DevName[MEDIA_ISP_CHAR_LENGTH_MAX];

    if (!isp_dev) {
        /****ERROR****/printk(KERN_ERR "%s: null pointer of handle", __func__);
        RetVal = VSI_ERR_NULL_PTR;
        return RetVal;
    }

    IspPort = &isp_dev->IspPorts[Port];

//	int __ispID=isp_dev->id,__portID=Port;
	uint8_t smode=0;
	char *sname="ox03f10";

	char *calibxml="/run/media/mmcblk0p1/OX03f10.xml";
	char *Ajson="/run/media/mmcblk0p1/auto.json\n";
	char *Mjson="/run/media/mmcblk0p1/manual_ext.json\n";

	char sentestname[20];
//	memcpy(&IspId,&__ispID,sizeof(__ispID));
//	memcpy(&PortId,&__portID,sizeof(__portID));


	memcpy( IspPort->SensorInfo.Name,sname,strlen(sname)+1 );
	memcpy( sentestname,sname,strlen(sname)+1 );
#if 0
	if (!sname ||!IspPort->SensorInfo.CalibXml ) {
    	printk(KERN_ERR "Sourc1111e or destination pointer is NULL\n");
    	return -EINVAL;
	}
	if (!IspPort->SensorInfo.Name ) {
    	printk(KERN_ERR "Sourc2222222222e or destination pointer is NULL\n");
    	return -EINVAL;
	}
#endif
	memcpy( IspPort->SensorInfo.CalibXml,calibxml,strlen(calibxml)+1);
	memcpy(  IspPort->SensorInfo.AutoJson ,Ajson,strlen(Ajson)+1 );
	memcpy(    IspPort->SensorInfo.ManuJson,Mjson,strlen(Mjson)+1 );
	memcpy(  &IspPort->SensorInfo.Mode,&smode,sizeof(IspPort->SensorInfo.Mode) );

	snprintf(DevName,sizeof(DevName), "/proc/vsi/isp_subdev%d", isp_dev->id);
	/****INFO****/pr_err("%s: parse %s info:", __func__, DevName);
	/****INFO****/pr_err("%s: isp : %d", __func__, isp_dev->id);
	/****INFO****/pr_err("%s: port: %d", __func__, Port);
	/****INFO****/pr_err("%s: name: %s ; test name %s", __func__, IspPort->SensorInfo.Name,sentestname);
	pr_err("SenNAme : %s\n",IspPort->SensorInfo.Name);


        /****INFO****/pr_err("%s: mode: %d", __func__, IspPort->SensorInfo.Mode);
    	/****INFO****/pr_err("%s: xml : %s", __func__, IspPort->SensorInfo.CalibXml);
    	/****INFO****/pr_err("%s: manu_json: %s", __func__, IspPort->SensorInfo.ManuJson);
    	/****INFO****/pr_err("%s: auto_json: %s", __func__, IspPort->SensorInfo.AutoJson);

    	//fclose(Fp);

    return RetVal;
}


int IspDeviceCreate(struct vvcam_isp_dev *isp_dev , uint8_t Port)
{
	int RetVal = VSI_SUCCESS;
    	MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    	CamDeviceConfig_t CamConfig;
    	memset(&CamConfig, 0 ,sizeof(CamConfig)); // RNJITH-UNDO to COMMENT

    	CamConfig.ispHwId = isp_dev->id;
    	CamConfig.inputCfg.inputType = CAMDEV_INPUT_TYPE_SENSOR;
    	if (IspPort->CamDeviceHandle) {
      		return VSI_SUCCESS;
    	}
	
/*CamConfig = kmalloc(sizeof(CamDeviceConfig_t), GFP_KERNEL);//(CamDeviceConfig_t*)malloc(sizeof(CamDeviceConfig_t)); 
    memset(CamConfig, 0 ,sizeof(CamConfig));
    CamConfig->ispHwId = isp_dev->id;
    CamConfig->inputCfg.inputType = CAMDEV_INPUT_TYPE_SENSOR;*/
#if 0 //RANJITH what about below section ?
    if (IspEventDev->PortsMask != 0x01) {
        CamConfig.workCfg.workMode = CAMDEV_WORK_MODE_MCM;
        CamConfig.workCfg.modeCfg.mcm.portId = Port + 1;     //"1:CAMDEV_MCM_PORT_0, 2:CAMDEV_MCM_PORT_1, ..."
        CamConfig.workCfg.modeCfg.mcm.mcmOp = 1;             //"1:CAMDEV_MCM_OP_SW, 2:CAMDEV_MCM_OP_HW"
    }
#endif
// else {
        CamConfig.workCfg.workMode = CAMDEV_WORK_MODE_STREAM;
        CamConfig.workCfg.modeCfg.stream.portId = Port + 1;  //"1:CAMDEV_MCM_PORT_0, 2:CAMDEV_MCM_PORT_1, ..."
  //  }

	CamConfig.outputCfg.outputType = CAMDEV_OUTPUT_TYPE_MEMORY;
    	CamConfig.priority = CAMDEV_SEQ_PRI_0;

	/* As memset is now added for camConfig , the below may be removed*/
#if 1
	CamConfig.workCfg.tileCfg.tileOp=0;
	CamConfig.workCfg.tileCfg.tileNum=1;
	CamConfig.workCfg.tileCfg.xPices=1;
	CamConfig.workCfg.tileCfg.yPices=1;
#endif
    	RetVal = VsiCamDeviceCreate( isp_dev , &CamConfig, &IspPort->CamDeviceHandle);
#if 0
#endif

    	if (RetVal != VSI_SUCCESS) {
        	printk(KERN_ERR "CamDevice Creat Isp Device Handle Failed, ret is %d", RetVal);
        	RetVal = VSI_ERR_TIMEOUT;
        	return RetVal;
    	}
    	RetVal = MediaIspCalibLoadSensorInfo(isp_dev, Port);
    	if (RetVal != VSI_SUCCESS) {
        	printk(KERN_ERR "%s: load sensor info failed %d", __func__, RetVal);
        	goto ERR_TO_DESTROY_CAMDEVICE_HANDLE;
    	}

    	char SensorName[MEDIA_ISP_CHAR_LENGTH_MAX];
    	memset(SensorName, 0, sizeof(SensorName));
    	RetVal = MediaIspCalibGetSensorName(isp_dev, Port, SensorName);
    	if (RetVal != VSI_SUCCESS) {
        	printk(KERN_ERR "%s: get sensor name failed %d", __func__, RetVal);
        	goto ERR_TO_DESTROY_CAMDEVICE_HANDLE;
    	}

    	CamDeviceSensorDrvHandle_t SensorDrvHandle = VSI_NULL;
    	RetVal = VsiCamDeviceSensorMapping( isp_dev, IspPort->CamDeviceHandle, SensorName, &SensorDrvHandle);
    	if(RetVal != VSI_SUCCESS || SensorDrvHandle == VSI_NULL) {
        	printk(KERN_ERR "CamDevice sensor mapping %s Failed", SensorName);
        	RetVal = VSI_ERR_TIMEOUT;
        	goto ERR_TO_DESTROY_CAMDEVICE_HANDLE;
    	}


    	RetVal = VsiCamDeviceSensorDrvHandleRegister(isp_dev, IspPort->CamDeviceHandle, SensorDrvHandle);
    	if (RetVal != VSI_SUCCESS) {
        	printk(KERN_ERR "CamDevice register sensor %s driver Failed, ret is %d", SensorName, RetVal);
        	RetVal = VSI_ERR_TIMEOUT;
        	goto ERR_TO_DESTROY_CAMDEVICE_HANDLE;
    	}
//while(1);
    	RetVal = MediaIspCalibQuerySensor(isp_dev, Port);

    	if (RetVal != VSI_SUCCESS) {
        	printk(KERN_ERR "%s: query sensor failed %d", __func__, RetVal);
        	goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
    	}
    	RetVal = MediaIspDeviceGetPortSinkInfo(isp_dev, Port, &IspPort->SinkInfo);
    	if (RetVal != VSI_SUCCESS) {
        	printk(KERN_ERR "%s: get port sink info failed %d", __func__, RetVal);
        	goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
    	}


    	MediaFmt *Format;
    	Format= kmalloc(sizeof(MediaFmt), GFP_KERNEL);
    	if(!Format)
    	{
		printk(KERN_ERR "FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
		return -ENOMEM;
    	} 
//	IspPort->SinkInfo.Rect.Height=1080;
//	IspPort->SinkInfo.Rect.Width=1920;
    	memset(Format, 0, sizeof(MediaFmt));
    	Format->Width       = IspPort->SinkInfo.Rect.Width;
    	Format->Height      = IspPort->SinkInfo.Rect.Height;
    	Format->PixelFormat = IspPort->SinkInfo.Fourcc;
    
    	MediaIspHalSetFmt(isp_dev, Port * MEDIA_ISP_PORT_PAD_COUNT, Format);
   	kfree(Format);

    	return RetVal;
ERR_TO_UNREGISTER_SENSOR_HANDLE:
    	VsiCamDeviceSensorDrvHandleUnRegister( isp_dev, IspPort->CamDeviceHandle);
ERR_TO_DESTROY_CAMDEVICE_HANDLE:
    	VsiCamDeviceDestroy(isp_dev, &CamConfig , IspPort->CamDeviceHandle);
    	IspPort->CamDeviceHandle = VSI_NULL;
    	return RetVal;
}














