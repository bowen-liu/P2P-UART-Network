#include "frame.h"

void print_bytes(uchar *buf, size_t bytes)
{
	int i;
	
	for(i=0; i < bytes; i++)
	{
		//if(i%2 == 0)printf(" ");
		printf("%02X", buf[i]);
	}
	printf("\n");
}


FRAME create_frame(uint8_t src, uint8_t dst, uint8_t size, uchar *payload)
{
	FRAME frame;
	
	frame.preamble = FRAME_PREAMBLE;
	frame.src = src;
	frame.dst = dst;
	frame.size = size;
 
	frame.payload = payload;
	
	return frame;
}


uchar* frame_header_tobuf (FRAME *frame)
{
	uchar* header_str = malloc(FRAME_HEADER_SIZE + 1);
	memcpy(header_str, (uchar*) frame, FRAME_HEADER_SIZE);
  
	return header_str;
}

FRAME buf_to_frame(uchar* buf)
{
	FRAME frame;
	
	frame.preamble 	= *((uint16_t*)&buf[0]);
	frame.dst 		= ((*((uint8_t*) &buf[2])) >> 4);
	frame.src 		= (*((uint8_t*) &buf[2])) & 0x0F;
	frame.size 		= *((uint8_t*)&buf[3]);
 
  frame.payload = &buf[FRAME_HEADER_SIZE + 1];

	return frame;
}



RAW_FRAME frame_to_buf (FRAME frame)
{
  RAW_FRAME raw_frame;

  //Marshal the headers first 
  raw_frame.size = FRAME_HEADER_SIZE + frame.size + 2;  //header size + payload size + "STX" + "ETX"
  raw_frame.buf = frame_header_tobuf(&frame);

  //resize the buffer to fit the payload as well. 
  realloc(raw_frame.buf, raw_frame.size);
  
  //Move the payload into the buffer, along with "STX" and "ETX"
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
}

