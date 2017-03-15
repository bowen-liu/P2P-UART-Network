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

FRAME buf_to_frame(uchar* buf)
{
	FRAME frame;
	
	//Extract the frame headers
	frame.preamble 	= *((uint16_t*)&buf[0]);
	frame.dst 		= ((*((uint8_t*) &buf[2])) >> 4);
	frame.src 		= (*((uint8_t*) &buf[2])) & 0x0F;
	frame.size 		= *((uint8_t*)&buf[3]);
 
 
	//create a new buffer for the payload
	if(frame.size > 0)
	{
		frame.payload = malloc(frame.size);
		memcpy(frame.payload, &buf[FRAME_HEADER_SIZE + 1], frame.size);
	}

	return frame;
}


RAW_FRAME frame_to_buf (FRAME frame)
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

