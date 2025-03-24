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
	//Assigning isp_dev structure value to isp_dev present in rpu_dev struct
	isp_dev->rpu->isp_dev[isp_dev->id] = isp_dev;
	return 0;
}

int IspDeviceCreateMIMO(struct vvcam_isp_dev *isp_dev , uint8_t Port)
{
	int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];
    CamDeviceConfig_t CamConfig;

    //mutex_lock(&isp_dev->port_lock[Port]);

    memset(&CamConfig, 0 ,sizeof(CamConfig));

    CamConfig.ispHwId = isp_dev->id;
    CamConfig.inputCfg.inputType = CAMDEV_INPUT_TYPE_IMAGE;
	if (IspPort->CamDeviceHandle) {
        return VSI_SUCCESS;
    }

	CamConfig.workCfg.workMode = CAMDEV_WORK_MODE_RDMA;
	CamConfig.workCfg.modeCfg.stream.portId =  0;

	CamConfig.outputCfg.outputType = CAMDEV_OUTPUT_TYPE_MEMORY;
    CamConfig.priority = CAMDEV_SEQ_PRI_0;

    /****CamDeviceCreate*****/
    RetVal = VsiCamDeviceCreate(isp_dev, &CamConfig, &IspPort->CamDeviceHandle);
    if (RetVal != VSI_SUCCESS) {
        dev_err(isp_dev->dev,"CamDevice Creat Isp Device Handle Failed, ret is %d\n", RetVal);
        RetVal = VSI_ERR_TIMEOUT;
        return RetVal;
    }

    return RetVal;

}


int IspDeviceDistroyMIMO(struct vvcam_isp_dev *isp_dev , uint8_t Port)
{
	int RetVal = VSI_SUCCESS;
    MediaIspPortAttr *IspPort = &isp_dev->IspPorts[Port];

    /*Enter Port Level Critical Section */
    //mutex_lock(&isp_dev->port_lock[Port]);

	RetVal= VsiCamDeviceDestroy(isp_dev, &IspPort->CamDeviceHandle);
	if (RetVal != VSI_SUCCESS) {
		dev_err(isp_dev->dev,"CamDevice Distroy Isp Device Handle Failed, ret is %d\n", RetVal);
		RetVal = VSI_ERR_TIMEOUT;
		return RetVal;
	}
	IspPort->CamDeviceHandle = VSI_NULL;

    return RetVal;
}


