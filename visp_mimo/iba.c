/*
 * Copyright 2025 Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "sensor_cmd.h"

#include "visp_mbox_driver.h"
//#include "kmbox.h"
#include "cam_device.h"
#include "cam_device_api.h"
#include "iba.h"

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
    pr_err("  IBA Enabled: %u\n", iba_info->is_iba_enabled);
    pr_err("  H-Blank Prog: %u\n", iba_info->hblank_prog);
    pr_err("  V-Blank Prog: %u\n", iba_info->vblank_prog);
    pr_err("  VCID: %u\n", iba_info->vcid);
    pr_err("  PPC: %u\n", iba_info->ppc);
    pr_err("  ISP ID: %u\n", iba_info->isp_id);
    pr_err("  Frame Rate: %u\n", iba_info->frame_rate);
}


int IBA_init_send_command(struct visp_dev *isp_dev, CamDeviceHandle_t hCamDevice)
{
    int result = 0;
    payload_packet *packet;
    uint8_t *p_data;
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

