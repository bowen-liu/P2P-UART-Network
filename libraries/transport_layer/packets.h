#ifndef _UARTNET_PACKETH_
#define _UARTNET_PACKETH_

//#include "../link_layer/frame.h"
#include <frame.h>



//Note that the preamble are the concatenation of the two ASCII symbols, with each symbol's 8th bit set to 1.
#define UMPACKET_PREAMBLE  			0x84CD			//SOH + M
#define USPACKET_PREAMBLE  			0x84D3			//SOH + S
#define RSPACKET_SYN_PREAMBLE  		0x8116			//SOH + SYN
#define RSPACKET_ACK_PREAMBLE  		0x8486			//SOH + ACK
#define RSPACKET_DATA_PREAMBLE  	0x8482			//SOH + STX


//secondary header widths
#define ID_WIDTH					16
#define USTREAM_SIZE_WIDTH			16
#define RSTREAM_SIZE_WIDTH			32


//secondary header size in bytes
#define USPACKET_HEADER_EXTRA 		(PREAMBLE_WIDTH + ID_WIDTH + 2*USTREAM_SIZE_WIDTH) /8
#define RSPACKET_HEADER_EXTRA 		(PREAMBLE_WIDTH + ID_WIDTH + RSTREAM_SIZE_WIDTH + CHECKSUM_WIDTH) /8


//Total header size. +1 for "STX"
#define UMPACKET_HEADER_TOTAL    	FRAME_HEADER_SIZE 
#define USPACKET_HEADER_TOTAL    	FRAME_HEADER_SIZE + 1 + USPACKET_HEADER_EXTRA
#define RSPACKET_HEADER_TOTAL    	FRAME_HEADER_SIZE + 1 + RSPACKET_HEADER_EXTRA


//max payload size
#define UMPACKET_MAX_PAYLOAD		MAX_PAYLOAD_SIZE
#define USPACKET_MAX_PAYLOAD		MAX_PAYLOAD_SIZE - USPACKET_HEADER_EXTRA
#define RSPACKET_MAX_PAYLOAD		MAX_PAYLOAD_SIZE - RSPACKET_HEADER_EXTRA



/***************************
Unreliable Message Packets
***************************/


//UMPACKET is essentially a bare frame in the transport layer.
typedef FRAME UMPACKET;



/***************************
Unreliable Stream Packets
***************************/

typedef struct {
	
	//Primary Header Fields
	unsigned int src			: ADDRESS_WIDTH;
	unsigned int dst			: ADDRESS_WIDTH;
	unsigned int payload_size   : PAYLOAD_SIZE_WIDTH;
  
	//Secondary Header Fields
	unsigned int type    		: PREAMBLE_WIDTH;
	unsigned int id				: ID_WIDTH;
	unsigned int total_size		: USTREAM_SIZE_WIDTH; 
	unsigned int payload_offset	: USTREAM_SIZE_WIDTH; 
	
	unsigned char *payload;
	
} __attribute__((packed)) USPACKET;



/***************************
Reliable Message Packets
***************************/

typedef struct {

	unsigned int src		: ADDRESS_WIDTH;
	unsigned int dst		: ADDRESS_WIDTH;

	unsigned int type    	: PREAMBLE_WIDTH;
	unsigned int id			: ID_WIDTH;
	uint32_t size    		: RSTREAM_SIZE_WIDTH;
	unsigned int checksum	: CHECKSUM_WIDTH;
	
	unsigned char *payload;
	
} __attribute__((packed)) RSPACKET;




//Functions

//Must add this for Arduino IDE to link functions in c headers
#ifdef __cplusplus
extern "C" {
#endif


UMPACKET create_umpacket(uint8_t src, uint8_t dst, uint8_t size, uchar *payload);
USPACKET create_uspacket(uint8_t src, uint8_t dst, uint16_t id, uint8_t total_size, uint16_t payload_offset, uint8_t payload_size, uchar *payload);
RSPACKET create_rspacket(uint16_t preamble, uint8_t src, uint8_t dst, uint16_t id, uint32_t size, uchar *payload);


USPACKET frame_to_uspacket(FRAME frame);
void print_uspacket(USPACKET packet);


#ifdef __cplusplus
}
#endif





#endif
