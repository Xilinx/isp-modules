#ifndef __MAILBOX_H__
#define __MAILBOX_H__

//#include <stdint.h>
#include "mailbox_cmd.h"
//#include "mc_oslayer.h"

#define MAX_ITEM 16396

#define payload_extra_size /*12*/20//RKC-REMOVE

extern uint32_t dest_cpu_id, src_cpu_id;
//extern os_mutex_t mbox_lock;


typedef enum payload_type {
    CMD,
	RESP,
} Payload_type;

// hardcode, only for compatible

// send
#if 1
typedef struct payload_user_template {
	Payload_type type;
        uint8_t return_res;
	uint32_t cookie;
	uint32_t payload_size;
	uint8_t payload_data[MAX_ITEM];
} Payload_packet ;
#endif
// response
typedef struct payload_template {
    Payload_type type;
    uint8_t return_res;
    uint32_t cookie;
    uint32_t payload_size;
    uint8_t payload[MAX_ITEM];
} Payload_packet_response;
#if 1
typedef struct response_user_packet_t {
    /*Define your data fields*/
    uint32_t cmdid;
    void *data;
    Payload_packet_response res_payload_pkt;
} response_user_packet;
#endif

typedef struct response_template {
	Payload_type payload_type;
	uint8_t returen_res;
	uint32_t cookie;
	MBCmdId_E cmdid;
	uint32_t error_subcode;
} Response_packet;

#if 0
int MailBoxInit();
void MailBoxDeInit();

int Send_Command(int cmd, void *data, uint32_t size, uint8_t dest_cpu, uint8_t src_cpu);
int Send_Response(int cmd, void *data, uint32_t size, uint8_t dest_cpu, uint8_t src_cpu);
int apu_wait_for_mb_data(uint32_t cookie, void *data);
int apu_wait_for_ACK(uint32_t cookie);
#endif
#endif





