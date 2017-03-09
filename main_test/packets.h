#ifndef _UARTNET_PACKETH_
#define _UARTNET_PACKETH_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//Note that the preamble are the concatenation of the two ASCII symbols, with each symbol's 8th bit set to 1.
#define UMPACKET_PREAMBLE  			0x84CD			//SOH + M
#define USPACKET_PREAMBLE  			0x84D3			//SOH + S
#define RSPACKET_SYN_PREAMBLE  		0x8116			//SOH + SYN
#define RSPACKET_ACK_PREAMBLE  		0x8486			//SOH + ACK
#define RSPACKET_DATA_PREAMBLE  	0x8482			//SOH + STX

#define PACKET_POSTAMBLE  			0x97 			//ETB with 8th bit set to 1

//bit widths for all packet header fields that are shared across all packet types
#define PREAMBLE_WIDTH				16
#define ADDRESS_WIDTH 				4 
#define PAYLOAD_SIZE_WIDTH			8
#define CHECKSUM_WIDTH				8

//bit widths for all packet header fields that are shared across all stream packet types
#define STREAM_SIZE_WIDTH			16
#define ID_WIDTH					16


//header size in bytes
#define UMPACKET_HEADER_SIZE 		(PREAMBLE_WIDTH + 2*ADDRESS_WIDTH + PAYLOAD_SIZE_WIDTH) /8
#define USPACKET_HEADER_SIZE 		(PREAMBLE_WIDTH + 2*ADDRESS_WIDTH + ID_WIDTH + 2*STREAM_SIZE_WIDTH + PAYLOAD_SIZE_WIDTH) /8
#define RSPACKET_HEADER_SIZE 		(PREAMBLE_WIDTH + 2*ADDRESS_WIDTH + ID_WIDTH + STREAM_SIZE_WIDTH + CHECKSUM_WIDTH) /8


typedef unsigned char uchar;


/***************************
Unreliable Message Packets
***************************/

//Note: The "__attribute__((packed))" compiler directive disables struct byte padding on GCC

typedef struct {
	
	unsigned int preamble 	: PREAMBLE_WIDTH;
	unsigned int src 		: ADDRESS_WIDTH;
	unsigned int dst 		: ADDRESS_WIDTH;
	unsigned int size		: PAYLOAD_SIZE_WIDTH; 
	unsigned char *payload;
	
} __attribute__((packed)) UMPACKET;



/***************************
Unreliable Stream Packets
***************************/


typedef struct {
	
	unsigned int preamble		: PREAMBLE_WIDTH;
	unsigned int src			: ADDRESS_WIDTH;
	unsigned int dst			: ADDRESS_WIDTH;
	unsigned int id				: ID_WIDTH;
	unsigned int total_size		: STREAM_SIZE_WIDTH; 
	unsigned int payload_offset	: STREAM_SIZE_WIDTH; 
	unsigned int payload_size	: PAYLOAD_SIZE_WIDTH; 
	unsigned char *payload;
	
} __attribute__((packed)) USPACKET;



/***************************
Reliable Message Packets
***************************/

typedef struct {
	
	unsigned int preamble	: PREAMBLE_WIDTH;
	unsigned int src		: ADDRESS_WIDTH;
	unsigned int dst		: ADDRESS_WIDTH;
	unsigned int id			: ID_WIDTH;
	unsigned int size		: STREAM_SIZE_WIDTH; 
	unsigned int checksum	: CHECKSUM_WIDTH;
	unsigned char *payload;
	
} __attribute__((packed)) RSPACKET;


//Must add this for Arduino IDE to link functions in c headers
#ifdef __cplusplus
extern "C" {
#endif

//functions
void print_bytes(uchar *buf, size_t bytes);

uchar* umpacket_header_tobuf (UMPACKET *packet);
uchar* uspacket_header_tobuf (USPACKET *packet);
uchar* rspacket_header_tobuf (RSPACKET *packet);

UMPACKET buf_to_umpacket(uchar* buf);
USPACKET buf_to_uspacket(uchar* buf);
RSPACKET buf_to_rspacket(uchar* buf);

void print_umpacket(UMPACKET packet);
void print_uspacket(USPACKET packet);
void print_rspacket(RSPACKET packet);


UMPACKET create_umpacket(uint8_t src, uint8_t dst, uint8_t size, uchar *payload);
USPACKET create_uspacket(uint8_t src, uint8_t dst, uint8_t id, uint8_t total_size, uint16_t payload_offset, uint8_t payload_size, uchar *payload);
RSPACKET create_rspacket_syn(uint8_t src, uint8_t dst, uint16_t size);
RSPACKET create_rspacket_ack(uint8_t src, uint8_t dst, uint16_t id, uint16_t size);
RSPACKET create_rspacket_data(uint8_t src, uint8_t dst, uint16_t id, uint16_t size, uchar *payload);

#ifdef __cplusplus
}
#endif



#endif
