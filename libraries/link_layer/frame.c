#include "frame.h"

void print_bytes(uchar *buf, size_t bytes)
{
	int i;
	
	for(i=0; i < bytes; i++)
	{
		//if(i%2 == 0)printf(" ");
		printf("%02X", buf[i]);
	}
}

//We always assume the payloads for frames are not dynamically allocated
FRAME create_frame(uint8_t src, uint8_t dst, uint8_t size, uchar *payload)
{
	FRAME frame;
	
	frame.preamble = FRAME_PREAMBLE;
	frame.src = src;
	frame.dst = dst;
	frame.size = size;
 
	frame.payload = payload;
	//frame.payload = malloc(frame.size);
	//memcpy(frame.payload, payload, frame.size);
	
	return frame;
}


//Turns a structured FRAME into a RAW_FRAME for transmission
RAW_FRAME frame_to_raw (FRAME frame)
{
  RAW_FRAME raw_frame;
  raw_frame.size = FRAME_HEADER_SIZE + frame.size + 2;  //header size + payload size + "STX" + "ETX"
  raw_frame.buf = malloc(raw_frame.size);
  
  //Marshal the headers first 
  memcpy(raw_frame.buf, (uchar*)&frame, FRAME_HEADER_SIZE);
  
  //Append the payload into the buffer, along with "STX" and "ETX" added around the payload
  raw_frame.buf[FRAME_HEADER_SIZE] = STX;
  memcpy(&raw_frame.buf[FRAME_HEADER_SIZE + 1], frame.payload, frame.size);
  raw_frame.buf[FRAME_HEADER_SIZE + 1 + frame.size] = ETX;

  return raw_frame;
}


//Turn a RAW_FRAME into a FRAME struct. If intended for link layer, handle it immediately
FRAME raw_to_frame(RAW_FRAME raw)
{
  FRAME frame;  
  int i;
  
	//Extract the frame headers
	frame.preamble 	= *((uint16_t*) &raw.buf[0]);
	frame.dst 		= ((*((uint8_t*) &raw.buf[2])) >> 4);
	frame.src 		= (*((uint8_t*) &raw.buf[2])) & 0x0F;
	frame.size 		= *((uint8_t*) &raw.buf[3]);
 
 
	//create a new buffer for the payload
	if(frame.size > 0)
	{
		frame.payload = malloc(frame.size);
		memcpy(frame.payload, &raw.buf[FRAME_HEADER_SIZE + 1], frame.size); //removes "STX" and "ETX"
	}
	
	
	//Check if payload is complete
	if (raw.buf[FRAME_HEADER_SIZE] != STX)
		printf("STX not found, packet header may be corrupt!\n");
	if (raw.buf[FRAME_HEADER_SIZE + frame.size + 1] != ETX)
		printf("ETX not found, packet may be corrupt or payload was truncated!\n");

  
  //Remember to free its payload when done!

  return frame;
}



void print_frame(FRAME frame)
{
	printf("preamble: %X\n", frame.preamble);
	printf("src: %X\n", frame.src);
	printf("dst: %X\n", frame.dst);
	printf("size: %X\n", frame.size);
	
	printf("payload: ");
	print_bytes(frame.payload, frame.size);
	printf("\n");
}

