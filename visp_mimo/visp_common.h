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



#ifndef __visp_COMMON_H__
#define __visp_COMMON_H__

int visp_buf_done(struct v4l2_subdev *sd, void *arg);
int visp_set_frame_interval_public(struct visp_dev *isp_dev, struct v4l2_subdev_frame_interval *fi);
int visp_set_fmt_public(struct visp_dev *isp_dev, struct v4l2_subdev_format *format);
int MediaIspDeviceSetFrameRate(struct visp_dev *isp_dev, uint8_t Port,  uint32_t *FrameRate);
int MediaIspDeviceStreamOn(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspDeviceStreamOnOut(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int IspDeviceDestroy(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int IspDestroyCamDevice(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int IspDestroyPipeline(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspStreamOff(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspDeviceQbuf(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn, MediaBuf *Buf);
int MediaIspQBuf(struct visp_dev *isp_dev, int Pad_index, MediaBuf *Buf);
int MediaIspHalMbusFmtToMediaFmt(uint32_t *Code, uint32_t *PixelFormat, uint32_t Fourcc);
int MediaIspDeviceSetFormat(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspCalibGetModeInfo(struct visp_dev *isp_dev, uint8_t Port, CamDeviceSensorModeInfo_t *ModeInfo);
int MediaIspCalibGetSensorName(struct visp_dev *isp_dev, uint8_t Port, char *SensorName);
int MediaIspCalibGetSensorMode(struct visp_dev *isp_dev, uint8_t Port, uint8_t *SensorMode);
int MediaIspDeviceSensorOpen(struct visp_dev *isp_dev, uint8_t Port);
int MediaIspDeviceCameraConnect(struct visp_dev *isp_dev, uint8_t Index);
int MediaIspCalibQuerySensor(struct visp_dev *isp_dev, uint8_t Port);
int MediaIspCalibLoadIspConfig(struct visp_dev *isp_dev, uint8_t Port);
int MediaIspHalSetFmt(struct visp_dev *isp_dev, int Pad, MediaFmt *Format);
int MediaIspSetFormat(struct visp_dev *isp_dev, uint32_t pad_index, MediaFmt Format_t);
int MediaIspSetFrameRate(struct visp_dev *isp_dev, int Pad, uint32_t *FrameRate);
int MediaIspHalBufDone(struct v4l2_subdev *sd, int pad, const MediaBuf *Buf);
int IspDeviceCreate(struct visp_dev *isp_dev, uint8_t Port);

RESULT VsiCamDeviceAwbDisable(struct visp_dev *isp_dev, CamDeviceHandle_t hCamDevice);
RESULT VsiCamDeviceUnRegisterAwbLib(struct visp_dev *isp_dev,
                                    CamDeviceHandle_t hCamDevice);
RESULT VsiCamDeviceAeDisable(struct visp_dev *isp_dev,
                             CamDeviceHandle_t hCamDevice);
RESULT VsiCamDeviceUnRegisterAeLib(struct visp_dev *isp_dev,
                                   CamDeviceHandle_t hCamDevice);

#endif
