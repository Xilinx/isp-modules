#ifndef __KMBOX__
#define __KMBOX__
uint8_t xlnx_mbox_apu_wait_for_data(struct visp_dev *isp_dev, void *data);
void xlnx_mbox_apu_wait_for_ack(struct visp_dev *isp_dev);
int Send_Command(MBCmdId_E cmd, void *data, uint32_t size, uint8_t dest_cpu,
				 uint8_t src_cpu);
#endif

