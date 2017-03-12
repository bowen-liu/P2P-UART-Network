#ifndef _UARTNET_PACKETH_
#define _UARTNET_PACKETH_

#include "../link_layer/frame.h"

//Note that the preamble are the concatenation of the two ASCII symbols, with each symbol's 8th bit set to 1.
#define UMPACKET_PREAMBLE  			0x84CD			//SOH + M
#define USPACKET_PREAMBLE  			0x84D3			//SOH + S
#define RSPACKET_SYN_PREAMBLE  		0x8116			//SOH + SYN
#define RSPACKET_ACK_PREAMBLE  		0x8486			//SOH + ACK
#define RSPACKET_DATA_PREAMBLE  	0x8482			//SOH + STX

//bit widths for all packet header fields that are shared across all stream packet types
#define STREAM_SIZE_WIDTH			16
#define ID_WIDTH					16

//header size in bytes
#define UMPACKET_HEADER_SIZE    FRAME_HEADER_SIZE
#define USPACKET_HEADER_SIZE 		(PREAMBLE_WIDTH + 2*ADDRESS_WIDTH + ID_WIDTH + 2*STREAM_SIZE_WIDTH + PAYLOAD_SIZE_WIDTH) /8
#define RSPACKET_HEADER_SIZE 		(PREAMBLE_WIDTH + 2*ADDRESS_WIDTH + ID_WIDTH + STREAM_SIZE_WIDTH + CHECKSUM_WIDTH) /8

/***************************
Unreliable Message Packets
***************************/

//UMPACKET is essentially a bare frame in the transport layer.
typedef FRAME UMPACKET;

/***************************
Unreliable Stream Packets
***************************/
typedef struct {
	
	unsigned int preamble		: PREAMBLE_WIDTH;
	unsigned int src			: ADDRESS_WIDTH;
	unsigned int dst			: ADDRESS_WIDTH;
  unsigned int size     : PAYLOAD_SIZE_WIDTH;   //Size of additional fields + actual payload
  
	unsigned int type    : PREAMBLE_WIDTH;
	unsigned int id				: ID_WIDTH;
	unsigned int total_size		: STREAM_SIZE_WIDTH; 
	unsigned int payload_offset	: STREAM_SIZE_WIDTH; 
	unsigned char *payload;
	
} __attribute__((packed)) USPACKET;



/***************************
Reliable Message Packets
***************************/

typedef struct {
	
	unsigned int preamble	: PREAMBLE_WIDTH;
	unsigned int src		: ADDRESS_WIDTH;
	unsigned int dst		: ADDRESS_WIDTH;
  unsigned int size    : STREAM_SIZE_WIDTH;   //Size of additional fields + actual payload

  unsigned int type    : PREAMBLE_WIDTH;
	unsigned int id			: ID_WIDTH;
	unsigned int checksum	: CHECKSUM_WIDTH;
	unsigned char *payload;
	
} __attribute__((packed)) RSPACKET;






#endif
