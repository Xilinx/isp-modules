#include <amd_isp_mimo_driver.h>

int xlnx_link_mbox(struct vvcam_isp_dev *isp_dev)
{
    /* Find or create a new RPU with the given rpu_id */
    isp_dev->rpu = get_rpu_dev(isp_dev->isp_rpu);
    if (!isp_dev->rpu) {
        dev_err(isp_dev->dev, "Failed to find or create RPU: %d\n", isp_dev->isp_rpu);
        return -ENOMEM;
    }
	 /* initialise completion used in while waiting for ack & data*/
    init_completion(&isp_dev->apu_wait_for_ack);
    init_completion(&isp_dev->apu_wait_for_data);

    isp_dev->apu_wait_for_isp_frame_done = 0;
init_waitqueue_head(&isp_dev->wq_frame_done_finished);
	
	if(!isp_dev->rpu->tx_chan || !isp_dev->rpu->rx_chan ){
        dev_err(isp_dev->dev, "No TX or RX Channel found on RPU: %d\n", isp_dev->isp_rpu);
        return -ENOMEM;
    }
			
  	isp_dev->tx_chan = isp_dev->rpu->tx_chan; 
  	isp_dev->rx_chan = isp_dev->rpu->rx_chan;
	isp_dev->rpu->isp_dev[isp_dev->id] = isp_dev; //Assigning isp_dev structure value to isp_dev present in rpu_dev struct 
    
    return 0;
	
}
int IspDeviceCreateMIMO(struct vvcam_isp_dev *isp_dev , uint8_t Port)
{
	int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    CamDeviceConfig_t CamConfig;
    MediaFmt *Format=NULL;
    
    /*Enter Port Level Critical Section */

    //mutex_lock(&isp_dev->port_lock[Port]);

	dev_info(isp_dev->dev, "RKC-ISPDRV_APP PORT = %u %s %d ", Port, __func__, __LINE__);

    memset(&CamConfig, 0 ,sizeof(CamConfig)); // RKC-UNDO to COMMENT

    CamConfig.ispHwId = 0; //isp_dev->id; //--MSLPK
    CamConfig.inputCfg.inputType = CAMDEV_INPUT_TYPE_IMAGE ; //CAMDEV_INPUT_TYPE_SENSOR; //--MSLPK
    if (IspPort->CamDeviceHandle) {
        return VSI_SUCCESS;
    }

	CamConfig.workCfg.workMode = CAMDEV_WORK_MODE_RDMA; //CAMDEV_WORK_MODE_STREAM; //MSLPK
	CamConfig.workCfg.modeCfg.stream.portId =  0; //Port + 1;  //"1:CAMDEV_MCM_PORT_0, 2:CAMDEV_MCM_PORT_1, ..."
	dev_err(isp_dev->dev, "%s %d port = %d %d\n",__func__,__LINE__,Port,CamConfig.workCfg.modeCfg.stream.portId);

	CamConfig.outputCfg.outputType = CAMDEV_OUTPUT_TYPE_MEMORY;
    CamConfig.priority = CAMDEV_SEQ_PRI_0;

    /****CamDeviceCreate*****/
    RetVal = VsiCamDeviceCreate(isp_dev, &CamConfig, &IspPort->CamDeviceHandle);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"CamDevice Creat Isp Device Handle Failed, ret is %d", RetVal);
        RetVal = VSI_ERR_TIMEOUT;
        return RetVal;
    }
	pr_err("[MSLPK] ----- done creating cam device %s %d [%p] [%p]\n",__func__,__LINE__,isp_dev->IspPorts[Port].CamDeviceHandle,IspPort->CamDeviceHandle);

#if 1
    Format= kmalloc(sizeof(MediaFmt), GFP_KERNEL);
    if(!Format)
    {
        dev_err(isp_dev->dev,"FAILED TO KMALLOC %s %d\n",__func__,__LINE__);
        RetVal = -ENOMEM;
        goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
    } 

    memset(Format, 0, sizeof(MediaFmt));
    Format->Width       = isp_dev->out_w;//IspPort->SinkInfo.Rect.Width; //--MSLPK
    Format->Height      = isp_dev->out_h; //IspPort->SinkInfo.Rect.Height;
    Format->PixelFormat = MEDIA_PIX_FMT_SBGGR8;//isp->out_fmt; //IspPort->SinkInfo.Fourcc;
	pr_err("[MSLPK] ----- at %s %d w %d h %d fmt %d \n",__func__,__LINE__,Format->Width,Format->Height,Format->PixelFormat);

    //RetVal= MediaIspHalSetFmt(isp_dev , Port * MEDIA_ISP_PORT_PAD_COUNT , Format); //MSLPK
    RetVal= MediaIspHalSetFmt(isp_dev , Port * MEDIA_ISP_PORT_PAD_COUNT , Format); //MSLPK
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: MediaIspHalSetFmt failed %d", __func__, RetVal);
//        goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
    }

	Format->Width       = isp_dev->cap_w;//IspPort->SinkInfo.Rect.Width; //--MSLPK
    Format->Height      = isp_dev->cap_h; //IspPort->SinkInfo.Rect.Height;
    Format->PixelFormat = isp_dev->cap_fmt; //IspPort->SinkInfo.Fourcc;
    RetVal= MediaIspHalSetFmt(isp_dev , 1 , Format); //MSLPK
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"%s: MediaIspHalSetFmt failed %d", __func__, RetVal);
//        goto ERR_TO_UNREGISTER_SENSOR_HANDLE;
    }
#endif

    return RetVal;

ERR_TO_UNREGISTER_SENSOR_HANDLE:
   	dev_err(isp_dev->dev, "RKC-DRV %s %d \n ", __func__, __LINE__);
   //	VsiCamDeviceSensorDrvHandleUnRegister( isp_dev , IspPort->CamDeviceHandle);
   	dev_err(isp_dev->dev, "RKC-DRV %s %d \n ", __func__, __LINE__);
   //	VsiCamDeviceDestroy(isp_dev, IspPort->CamDeviceHandle);
   	IspPort->CamDeviceHandle = VSI_NULL;
    return RetVal;
}



