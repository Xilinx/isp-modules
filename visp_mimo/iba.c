
#include "sensor_cmd.h"

#include "amd_mbox_driver.h"
//#include "kmbox.h"
#include "cam_device.h"
#include "cam_device_api.h"
#include "iba.h"
#if 0
iba_inst_t IBA_ConfigTable[XPAR_ISP_INSTANCE][XPAR_IBA_INSTANCES] =
{
		 {//ISP-0
				{   /*Tile-0, ISP-0, IBA0 configuration*/
					.DeviceId                  = IBA_0,
					.BaseAddress[0]            = XPAR_TILE0_IBA0_BASEADDR,
					.data_format               = 0x10, //RAW8
					.hres                      = 1920,
					.vres                      = 1080,
					.fps                       = 2,
					.tile_id                   = TILE_0,
					.iba_is_enabled            = IBA_ENABLED, /*Disabled by default,
					 	 	 	 	 	 	 	 	 	 	 	 it is automatically enabled in the code,
					 	 	 	 	 	 	 	 	 	 	 	 depending on port selection*/
					.virtual_channel_id        = VC0,
					.input_pixel_width         = QUAD_PIXEL_WIDTH,
					.output_fifo_write_mode    = QUAD_FIFO_WRITE, /*This can be derived from input_pixel_width*/
					.isp_instance              = ISP0,
					.iba_out_isp_in_port       = ISP0_MCM_PORT_0
				},
				{  //Tile-0, ISP-0, IBA1 configuration
					.DeviceId                  = IBA_1,
					.BaseAddress[0]            = XPAR_TILE0_IBA1_BASEADDR,
					.data_format               = 0x10, //RAW8
					.hres                      = 1920,
					.vres                      = 1080,
					.fps                       = 2,
					.tile_id                   = TILE_0,
					.iba_is_enabled            = IBA_ENABLED, /*Disabled by default,
					 	 	 	 	 	 	 	 	 	 	 	 it is automatically enabled in the code,
					 	 	 	 	 	 	 	 	 	 	 	 depending on port selection*/
					.virtual_channel_id        = VC0,
					.input_pixel_width         = QUAD_PIXEL_WIDTH,
					.output_fifo_write_mode    = QUAD_FIFO_WRITE,
					.isp_instance              = ISP0,
					.iba_out_isp_in_port       = ISP0_MCM_PORT_1
				},
				{  //Tile-0, ISP-0, IBA2 configuration
					.DeviceId                  = IBA_2,
					.BaseAddress[0]            = XPAR_TILE0_IBA2_BASEADDR,
					.data_format               = 0x10, //RAW8
					.hres                      = 1920,
					.vres                      = 1080,
					.fps                       = 2,
					.tile_id                   = TILE_0,
					.iba_is_enabled            = IBA_ENABLED, /*Disabled by default,
					 	 	 	 	 	 	 	 	 	 	 	 it is automatically enabled in the code,
					 	 	 	 	 	 	 	 	 	 	 	 depending on port selection*/
					.virtual_channel_id        = VC1,
					.input_pixel_width         = QUAD_PIXEL_WIDTH,
					.output_fifo_write_mode    = QUAD_FIFO_WRITE,
					.isp_instance              = ISP0,
					.iba_out_isp_in_port       = ISP0_MCM_PORT_2
				},
				{  //Tile-0, ISP-0, IBA3 configuration,This IBA can switch between ISP0-IBA3 <-> ISP1-IBA1.
					.DeviceId                  = IBA_3,
					.BaseAddress[0]            = XPAR_TILE0_IBA3_BASEADDR,
					.data_format               = 0x10, //RAW8
					.hres                      = 1920,
					.vres                      = 1080,
					.fps                       = 2,
					.tile_id                   = TILE_0,
					.iba_is_enabled            = IBA_ENABLED, /*Disabled by default,
					 	 	 	 	 	 	 	 	 	 	 	 it is automatically enabled in the code,
					 	 	 	 	 	 	 	 	 	 	 	 depending on port selection*/
					.virtual_channel_id        = VC0,
					.input_pixel_width         = QUAD_PIXEL_WIDTH,
					.output_fifo_write_mode    = QUAD_FIFO_WRITE,
					.isp_instance              = ISP0,
					.iba_out_isp_in_port       = ISP0_MCM_PORT_3
				}

		 },
		 {//ISP-1
				 {  //Tile-0, ISP-1, IBA0 configuration
				 	.DeviceId                  = IBA_4,
				 	.BaseAddress[0]            = XPAR_TILE0_IBA4_BASEADDR,
				 	.data_format               = 0x10, //RAW8
				 	.hres                      = 1920,
				 	.vres                      = 1080,
				 	.fps                       = 2,
				 	.tile_id                   = TILE_0,
					.iba_is_enabled            = IBA_ENABLED, /*Disabled by default,
					 	 	 	 	 	 	 	 	 	 	 	 it is automatically enabled in the code,
					 	 	 	 	 	 	 	 	 	 	 	 depending on port selection*/
				 	.virtual_channel_id        = VC0,
				 	.input_pixel_width         = QUAD_PIXEL_WIDTH,
				 	.output_fifo_write_mode    = QUAD_FIFO_WRITE,
				 	.isp_instance              = ISP1,
				 	.iba_out_isp_in_port       = ISP1_MCM_PORT_0
				 },
				 {  //Tile-0, ISP-1, IBA1 configuration.This IBA can switch between ISP0-IBA3 <-> ISP1-IBA1.
				 	.DeviceId                  = IBA_3,
				 	.BaseAddress[0]            = XPAR_TILE0_IBA3_BASEADDR,
				 	.data_format               = 0x10, //RAW8
				 	.hres                      = 1920,
				 	.vres                      = 1080,
				 	.fps                       = 2,
				 	.tile_id                   = TILE_0,
					.iba_is_enabled            = IBA_DISABLED, /*Disabled by default,
					 	 	 	 	 	 	 	 	 	 	 	 it is automatically enabled in the code,
					 	 	 	 	 	 	 	 	 	 	 	 depending on port selection*/
				 	.virtual_channel_id        = VC0,
				 	.input_pixel_width         = QUAD_PIXEL_WIDTH,
				 	.output_fifo_write_mode    = QUAD_FIFO_WRITE,
				 	.isp_instance              = ISP1,
				 	.iba_out_isp_in_port       = ISP1_MCM_PORT_1
				 }

		 }
};
static void populate_iba_config(struct vvcam_isp_dev *isp_dev)
{
    int i;
    int isp_id = isp_dev->id;
    int num_streams = isp_dev->num_streams;
    iba_inst_t *iba_config;

    if (isp_id < ISP0 || isp_id > ISP5) {
        dev_err(isp_dev->dev, "%s: Invalid isp_id %d\n", __func__, isp_id);
        return;
    }

    for (i = 0; i < num_streams; i++) {
        int iba_index;

        if ((isp_id % 2) == 0) {
            iba_index = i;
        } else if ((isp_id % 2) == 1) {
            iba_index = (num_streams == 1) ? IBA_4 : IBA_3 + i;
        }

        iba_config = &IBA_ConfigTable[isp_id][iba_index];

        iba_config->DeviceId = iba_index;
        iba_config->fps = isp_dev->iba[i].frame_rate;
        iba_config->data_format = isp_dev->iba[i].data_format;
        iba_config->iba_is_enabled = IBA_ENABLED;

        dev_info(isp_dev->dev,
                 "Populated IBA_ConfigTable[%d][%d]: DeviceId=%d, hres=%d, vres=%d, fps=%d, data_format=%d, enabled=%d\n",
                 isp_id, iba_index, iba_config->DeviceId, iba_config->hres, iba_config->vres, iba_config->fps,
                 iba_config->data_format, iba_config->iba_is_enabled);
    }
}
#endif
void print_iba_info(const iba_info_t *iba_info) {
    if (!iba_info) {
        pr_err("IBA info is NULL\n");
        return;
    }

    pr_err("IBA Info:\n");
    pr_err("  Base Address: 0x%x\n", iba_info->baseaddress);
    pr_err("  IBA ID: %u\n", iba_info->iba_id);
    pr_err("  Max Width: %u\n", iba_info->max_width);
    pr_err("  Max Height: %u\n", iba_info->max_height);
    pr_err("  Data Format: %u\n", iba_info->data_format);
    pr_err("  IBA Enabled: %u\n", iba_info->is_iba_enabled); // Assuming iba_status_t is u32
    pr_err("  H-Blank Prog: %u\n", iba_info->hblank_prog);
    pr_err("  V-Blank Prog: %u\n", iba_info->vblank_prog);
    pr_err("  VCID: %u\n", iba_info->vcid);
    pr_err("  PPC: %u\n", iba_info->ppc);
    pr_err("  ISP ID: %u\n", iba_info->isp_id);
    pr_err("  Frame Rate: %u\n", iba_info->frame_rate);
}


int IBA_init_send_command(struct vvcam_isp_dev *isp_dev, CamDeviceHandle_t hCamDevice)
{
    int result = 0;
    payload_packet *packet;// = kmalloc(sizeof(payload_packet), GFP_KERNEL);
    uint8_t *p_data;// = packet->payload;
    CamDeviceContext_t *pCamDevCtx = (CamDeviceContext_t *)hCamDevice;
    iba_info_t *iba_info;
    uint32_t iba_id;
    if (NULL == pCamDevCtx) {
        return RET_WRONG_HANDLE;
    }

    pCamDevCtx->cookie++;
    iba_id = pCamDevCtx->ispVtId;
    pr_err("IBA %s %d iba_id/Vtid =%d\n",__func__,__LINE__,iba_id);
    iba_info = &isp_dev->iba[iba_id];
    //iba_info->baseaddress = 0x88520000;
    iba_info->iba_id = iba_id;
    iba_info->max_width = 1920;
    iba_info->max_height = 1080;
    iba_info->is_iba_enabled = IBA_ENABLED;
    iba_info->isp_id = isp_dev->id;
    print_iba_info(iba_info);
    packet = kmalloc(sizeof(payload_packet), GFP_KERNEL);
    if (!packet) {
        dev_err(isp_dev->dev, "%s: Failed to allocate memory for packet\n", __func__);
        return -ENOMEM;
    }
    p_data = packet->payload;
    packet->cookie = 0x99;
    packet->type = CMD;
    packet->payload_size = 0;

    memcpy(p_data, &pCamDevCtx->instanceId, sizeof(uint32_t));
    p_data += sizeof(uint32_t);
    packet->payload_size += sizeof(uint32_t);

    memcpy(p_data, iba_info, sizeof(iba_info_t));

    packet->payload_size += sizeof(iba_info_t);
    if (packet->payload_size > MAX_ITEM) {
        dev_err(isp_dev->dev, "%s: Payload size (%d) exceeds maximum allowed (%d)\n",
                __func__, packet->payload_size, MAX_ITEM);
        kfree(packet);
        return RET_OUTOFRANGE;
    }

    mutex_lock(&isp_dev->mlock);

    result = Send_Command(APU_2_RPU_MB_CMD_IBA_INIT, packet,
                          packet->payload_size + payload_extra_size, isp_dev->isp_rpu, MBOX_CORE_APU);
    if (0 != result) {
        dev_err(isp_dev->dev, "%s: Send_Command failed with error code %d\n", __func__, result);
        mutex_unlock(&isp_dev->mlock);
        kfree(packet);
        return result;
    }

    mbox_send_message(isp_dev->tx_chan, NULL);
    xlnx_mbox_apu_wait_for_ack(isp_dev);

    dev_info(isp_dev->dev, "ACK received\n");

    mutex_unlock(&isp_dev->mlock);
    kfree(packet);

    return result;
}

