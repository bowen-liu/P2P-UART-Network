#include "send_recv.h"

static CONNECTION *in_streams;
static CONNECTION *out_streams;

/***************************
Front-end Functions
***************************/

void network_task()
{
	static uint8_t last_sent = 0;
	
	while(1)
	{
		
	}
	
}

void initialize()
{
	in_streams = calloc(MAX_INBOUND_STREAMS, sizeof(CONNECTION));
	out_streams = calloc(MAX_OUTBOUND_STREAMS, sizeof(CONNECTION));
	
}

int send_umessage(uint8_t src, uint8_t dst, uint8_t size, uchar *data)
{
	if(src > MAX_ADDRESS || dst > MAX_ADDRESS)
	{
		printf("***ERROR (send_umessage): src or dst size is greater than %d\n", MAX_ADDRESS);
		return -1;
	}
	
	if(size > MAX_PAYLOAD_SIZE)
	{
		printf("***WARNING (send_umessage): data size is greater than %d. Trying to send as ustream instead...\n", MAX_PAYLOAD_SIZE);
		return send_ustream(src, dst, size, data);
	}
	
	return 0;
}


int send_ustream(uint8_t src, uint8_t dst, uint8_t size, uchar *data)
{
	if(src > MAX_ADDRESS || dst > MAX_ADDRESS)
	{
		printf("***ERROR (send_ustream): src or dst size is greater than %d\n", MAX_ADDRESS);
		return -1;
	}
	
	if(size > MAX_STREAM_SIZE)
	{
		printf("***ERROR (send_ustream): data size is greater than %d\n", MAX_STREAM_SIZE);
		return -1;
	}
	
	return 0;
}


int send_rstream(uint8_t src, uint8_t dst, uint8_t size, uchar *data)
{
	if(src > MAX_ADDRESS || dst > MAX_ADDRESS)
	{
		printf("***ERROR (send_rstream): src or dst size is greater than %d\n", MAX_ADDRESS);
		return -1;
	}
	
	if(size > MAX_STREAM_SIZE)
	{
		printf("***ERROR (send_rstream): data size is greater than %d\n", MAX_STREAM_SIZE);
		return -1;
	}
	
	return 0;
}

int parse_recvd_packet(PACKET_RAW *buf)
{
	return 0;
}