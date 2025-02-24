
#ifndef __vvcam_isp_COMMON_H__
#define __vvcam_isp_COMMON_H__
//#include "vvcam_isp_common.h"

int vvcam_isp_buf_done(struct v4l2_subdev *sd, void *arg);
//int vvcam_isp_buffer_free_public_wrapper(struct vvcam_isp_dev *isp_dev, void *arg);
//int vvcam_isp_buffer_alloc_public(struct vvcam_isp_dev *isp_dev, struct vvcam_isp_ext_buf_info *ext_buf_info);
//int vvcam_isp_buffer_free_public_wrapper(struct vvcam_isp_dev *isp_dev, void *arg);
int vvcam_isp_set_frame_interval_public(struct vvcam_isp_dev *isp_dev, struct v4l2_subdev_frame_interval *fi);
int vvcam_isp_set_fmt_public(struct vvcam_isp_dev *isp_dev, struct v4l2_subdev_format *format);
//int vvcam_isp_buffer_free_public(struct vvcam_isp_dev *isp_dev, struct vvcam_isp_ext_buf_info *ext_buf_info);
int MediaIspDeviceSetFrameRate(struct vvcam_isp_dev *isp_dev, uint8_t Port,  uint32_t *FrameRate);
int MediaIspDeviceStreamOn(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int IspDeviceDestroy(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int IspDestroyCamDevice(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int IspDestroyPipeline(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspStreamOff(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspDeviceQbuf(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn, MediaBuf *Buf);
int MediaIspQBuf(struct vvcam_isp_dev *isp_dev, int Pad_index, MediaBuf *Buf);
int MediaIspHalMbusFmtToMediaFmt(uint32_t *Code, uint32_t *PixelFormat, uint32_t Fourcc);
int MediaIspDeviceSetFormat(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t Chn);
int MediaIspCalibGetModeInfo(struct vvcam_isp_dev *isp_dev, uint8_t Port, CamDeviceSensorModeInfo_t *ModeInfo);
int MediaIspCalibGetSensorName(struct vvcam_isp_dev *isp_dev, uint8_t Port, char *SensorName);
int MediaIspCalibGetSensorMode(struct vvcam_isp_dev *isp_dev, uint8_t Port, uint8_t *SensorMode);
int MediaIspDeviceSensorOpen(struct vvcam_isp_dev *isp_dev, uint8_t Port);
int MediaIspDeviceCameraConnect(struct vvcam_isp_dev *isp_dev, uint8_t Index);
//int Read_DQ_Bufinfo(void *data, OutputBuffer_t *pMediaBuffer, struct Chn_info *info);
//int MediaIspDeviceDqbuf(struct vvcam_isp_dev *isp_dev, struct Chn_info *info, MediaBuf *Buf, void *Packet_from_RPU, OutputBuffer_t *OutputBuffer);
int MediaIspCalibQuerySensor(struct vvcam_isp_dev *isp_dev, uint8_t Port);
int MediaIspCalibLoadIspConfig(struct vvcam_isp_dev *isp_dev, uint8_t Port);
int MediaIspHalSetFmt(struct vvcam_isp_dev *isp_dev, int Pad, MediaFmt *Format);
int MediaIspSetFormat(struct vvcam_isp_dev *isp_dev, uint32_t pad_index, MediaFmt Format_t);
int MediaIspSetFrameRate(struct vvcam_isp_dev *isp_dev, int Pad, uint32_t *FrameRate);
int MediaIspHalBufDone(struct v4l2_subdev *sd, int pad, const MediaBuf *Buf);
int IspDeviceCreate(struct vvcam_isp_dev *isp_dev, uint8_t Port);

RESULT VsiCamDeviceAwbDisable(struct vvcam_isp_dev *isp_dev, CamDeviceHandle_t hCamDevice);
RESULT VsiCamDeviceUnRegisterAwbLib(struct vvcam_isp_dev *isp_dev,
                                    CamDeviceHandle_t hCamDevice);
RESULT VsiCamDeviceAeDisable(struct vvcam_isp_dev *isp_dev,
                             CamDeviceHandle_t hCamDevice);
RESULT VsiCamDeviceUnRegisterAeLib(struct vvcam_isp_dev *isp_dev,
                                   CamDeviceHandle_t hCamDevice);

#endif
