
#ifndef __VISP_COMMON_H__
#define __VISP_COMMON_H__
#include "visp_common.h"

int visp_buf_done(struct v4l2_subdev *sd, void *arg);
//int visp_buffer_free_public_wrapper(struct visp_dev *isp_dev, void *arg);
//int visp_buffer_alloc_public(struct visp_dev *isp_dev, struct visp_ext_buf_info *ext_buf_info);
//int visp_buffer_free_public_wrapper(struct visp_dev *isp_dev, void *arg);
int visp_set_frame_interval_public(struct visp_dev *isp_dev, struct v4l2_subdev_frame_interval *fi);
int visp_set_fmt_public(struct visp_dev *isp_dev, struct v4l2_subdev_format *format);
//int visp_buffer_free_public(struct visp_dev *isp_dev, struct visp_ext_buf_info *ext_buf_info);
int MediaIspDeviceSetFrameRate(struct visp_dev *isp_dev, uint8_t Port,  uint32_t *FrameRate);
int MediaIspDeviceStreamOn(struct visp_dev *isp_dev, uint8_t Port, uint8_t Chn);
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

int Read_DQ_Bufinfo(void *data, struct visp_dev *isp_dev,
                    struct Chn_info *info, uint8_t *buf_index);
int MediaIspDeviceDqbuf(struct visp_dev *isp_dev, struct Chn_info *info, MediaBuf *Buf, void *Packet_from_RPU, OutputBuffer_t *OutputBuffer);
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
