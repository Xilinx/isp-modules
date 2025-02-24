/**
 * Copyright (C) 2021 VeriSilicon Holdings Co., Ltd.
 *
 * @file mbox_cmd.h
 * @brief Header of  the interface of Mailbox cmd
 * @author shenzhunwen <zhunwen.shen@verisilicon.com>
 */
#ifndef _MBOX_CMD_H_
#define _MBOX_CMD_H_

//#include "vsi_command.h"
//#include <linux/mbox_api.h>
#include "mbox_api.h"
#include "mbox_error_code.h"
//#include <linux/mbox_fifo.h>
#include "mbox_fifo.h"
#include "mbox_hardware.h"
//#include <linux/sensor_cmd.h>
#include "sensor_cmd.h"

#define MAX_MSGS_PER_BOX 12
#define MBOX_FIFO_BLOCK_SIZE \
	(sizeof(FifoControl) + sizeof(MboxPostMsg) * MAX_MSGS_PER_BOX)
//#define MBOX_FIFO_START_ADDR 0x4ed08000
#define MBOX_SECTION_SIZE 0x100000
#define ONE_MB 0x100000

/*
#define MAX_MSGS_PER_BOX 4
#define MBOX_FIFO_BLOCK_SIZE (sizeof(FifoControl) + sizeof(MboxPostMsg)*MAX_MSGS_PER_BOX)
#define MBOX_FIFO_START_ADDR 0x4ed08000
#define MBOX_SECTION_SIZE  0x100000
#define ONE_MB 0x100000
*/

/************************* Test Configuration ********************************/

#define IPI_DEVICE_ID_0 XPAR_XIPIPSU_0_DEVICE_ID
//#define IPI_DEVICE_ID_1	XPAR_XIPIPSU_1_DEVICE_ID
//#define IPI_DEVICE_ID_2	XPAR_XIPIPSU_2_DEVICE_ID

#define IPI_CH0_BIT_MASK XPAR_XIPIPSU_0_BIT_MASK
//#define IPI_CH1_BIT_MASK XPAR_XIPIPSU_1_BIT_MASK
//#define IPI_CH2_BIT_MASK XPAR_XIPIPSU_2_BIT_MASK

/* Test message length in words. Max is 8 words (32 bytes) */
#define TEST_MSG_LEN 8
/* Interrupt Controller device ID */
#define INTC_DEVICE_ID XPAR_SCUGIC_0_DEVICE_ID
/* Time out parameter while polling for response */
#define TIMEOUT_COUNT 100000

/*****************************************************************************/

void mailbox_init(uint32_t cpu, uint64_t MBOX_FIFO_START_ADDR,
				  uint64_t MBOX_FIFO_START_ADDR_PHY);
uint32_t write_mboxcmd(uint32_t cmdId, void *struct_msg, uint16_t size,
					   MboxCoreId receiver_id, MboxCoreId core_id);

int Send_Command(MBCmdId_E cmd, void *data, uint32_t size, uint8_t dest_cpu,
				 uint8_t src_cpu);
struct response_user_packet
{
	/*Define your data fields*/
	u32 cmdid;
	void *data;
	payload_packet res_payload_pkt;
};

int apu_mailbox_read(/*void *CallbackRef*/ uint32_t, uint32_t *isp_id);
uint32_t ParseCommand(MBCmdId_E, void *, uint32_t, MboxCoreId, MboxCoreId);
void apu_postmsg(MboxCoreId receiver_id);
uint32_t kmbox_write(int32_t cmdId, void *msg, uint16_t size,
                     MboxCoreId receiver_id, MboxCoreId core_id);
//int32_t kmbox_register_parse(kmbox_parse_process_t parse_process);
void mailbox_close(void);
int Send_Response(MBCmdId_E res, payload_packet *data, uint32_t size,
                  uint8_t dest_cpu, uint8_t src_cpu);
void apu_mailbox_read_data(uint32_t IpiSrcMask, void *pdst);
int32_t kmbox_unregister_parse(void);



#endif
