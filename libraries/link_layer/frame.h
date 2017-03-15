#ifndef _UARTNET_FRAMEH_
#define _UARTNET_FRAMEH_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef unsigned char uchar;

//Link Layer Frames
#define FRAME_PREAMBLE          0x84CD      //SOH + M

//"Start of Text" and "End of Text" ASCII character that wraps around payload
#define STX 0x2   
#define ETX 0x3

//bit widths for all packet header fields that are shared across all packet types
#define PREAMBLE_WIDTH				16
#define ADDRESS_WIDTH 				4 
#define PAYLOAD_SIZE_WIDTH			8
#define CHECKSUM_WIDTH				8

//maximum numerical values support by user configurable header fields
#define MAX_ADDRESS          (1 << ADDRESS_WIDTH) - 1 
#define MAX_PAYLOAD_SIZE      (1 << PAYLOAD_SIZE_WIDTH) - 1 
#define MAX_STREAM_SIZE       (1 << STREAM_SIZE_WIDTH) - 1 
#define MAX_ID            (1 << ID_WIDTH) - 1 

//Header size of the frame in bytes
#define FRAME_HEADER_SIZE     (PREAMBLE_WIDTH + 2*ADDRESS_WIDTH + PAYLOAD_SIZE_WIDTH) /8


//Frame format
//Note: The "__attribute__((packed))" compiler directive disables struct byte padding on GCC

typedef struct {
  
  unsigned int preamble   : PREAMBLE_WIDTH;
  unsigned int src    : ADDRESS_WIDTH;
  unsigned int dst    : ADDRESS_WIDTH;
  unsigned int size   : PAYLOAD_SIZE_WIDTH; 
  unsigned char *payload;
  
} __attribute__((packed)) FRAME;


//buffer for a raw unprocessed frame
typedef struct{

  size_t size;
  uchar *buf;

}RAW_FRAME;


//Functions

//Must add this for Arduino IDE to link functions in c headers
#ifdef __cplusplus
extern "C" {
#endif

//functions
void print_bytes(uchar *buf, size_t bytes);

FRAME create_frame(uint8_t src, uint8_t dst, uint8_t size, uchar *payload);
FRAME buf_to_frame(uchar* buf);
RAW_FRAME frame_to_buf (FRAME frame);
void print_frame(FRAME frame);




#ifdef __cplusplus
}
#endif


#endif
