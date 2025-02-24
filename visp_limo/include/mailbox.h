#ifndef __MAILBOX_H__
#define __MAILBOX_H__

#include "mailbox_cmd.h"

#define MAX_ITEM 16396
#define payload_extra_size /*12*/ 20 //RKC-REMOVE

extern uint32_t dest_cpu_id, src_cpu_id;

typedef enum payload_type
{
	CMD,
	RESP,
} Payload_type;

typedef struct payload_user_template
{
	Payload_type type;
	uint8_t return_res;
	uint32_t cookie;
	uint32_t payload_size;
	uint8_t payload_data[MAX_ITEM];
} Payload_packet;

// response
typedef struct payload_template
{
	Payload_type type;
	uint8_t return_res;
	uint32_t cookie;
	uint32_t payload_size;
	uint8_t payload[MAX_ITEM];
} Payload_packet_response;

typedef struct response_user_packet_t
{
	/*Define your data fields*/
	uint32_t cmdid;
	void *data;
	Payload_packet_response res_payload_pkt;
} response_user_packet;

typedef struct response_template
{
	Payload_type payload_type;
	uint8_t returen_res;
	uint32_t cookie;
	MBCmdId_E cmdid;
	uint32_t error_subcode;
} Response_packet;

#endif
