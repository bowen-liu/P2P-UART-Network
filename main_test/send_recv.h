#ifndef _UARTNET_PEERAPIH_
#define _UARTNET_PEERAPIH_

#include "packets.h"

//maximum numerical values support by user configurable header fields
#define MAX_ADDRESS					(1 << ADDRESS_WIDTH) - 1 
#define MAX_PAYLOAD_SIZE			(1 << PAYLOAD_SIZE_WIDTH) - 1 
#define MAX_STREAM_SIZE				(1 << STREAM_SIZE_WIDTH) - 1 
#define MAX_ID						(1 << ID_WIDTH) - 1 

//Max concurrent connections
#define MAX_INBOUND_STREAMS			MAX_ADDRESS + 1
#define MAX_OUTBOUND_STREAMS		MAX_ADDRESS + 1


//Generic Packet datatype used to return a newly received packet
typedef union {
	
	UMPACKET umpacket;
	USPACKET uspacket;
	RSPACKET rspacket;
	
}PACKET_RAW;


//Connection states for inbound/outbound stream connections
typedef struct {

	uint8_t reliable :1;		//1 = reliable stream; 0 = unreliable stream
	uint8_t	target;				//target receiver/sender
	uint16_t id;				//ID of the stream connection
	
	uint16_t total_size;		//Total size of the stream to be transferred
	uint16_t last_ack;			//Last ACK number received (if sender) / Sent (if receiver)
	uint16_t last_transferred;	//Offset of last payload send (if sender) / received (if received)
	
	uint8_t failed_packets;		//How many consecutive failures so far?
	uint16_t rtt;				//Average RTT with the target
	
	uchar* buffer;				//Where are the stream payloads stored at?
	
} CONNECTION;







#ifdef __cplusplus
extern "C" {
#endif

int send_umessage(uint8_t src, uint8_t dst, uint8_t size, uchar *data);
int send_ustream(uint8_t src, uint8_t dst, uint8_t size, uchar *data);
int send_rstream(uint8_t src, uint8_t dst, uint8_t size, uchar *data);

int parse_recvd_packet(PACKET_RAW *buf);


#ifdef __cplusplus
}
#endif

#endif