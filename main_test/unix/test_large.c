#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "packets.h"

#define UART_ADDR 	"/dev/ttyUSB0"


int main()
{
	FILE *uart = fopen(UART_ADDR, "w");
	char buf[256];
	int i, j;
	
	if(uart == NULL)
	{
		printf("cannot open %s\n", UART_ADDR);
		exit(-1);
	}
	
	
	
	uchar* header;
	
	for(j=0; j<5; j++)
	{

	  
	  fprintf(uart, "\nhellowosasdfasfdasd\n");


	 
	 uchar payload_test2[256] = "ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE";
	  UMPACKET testPacket2 = create_umpacket(0xE, 0xD, 0xFF, payload_test2);
	  
	  header = umpacket_header_tobuf (&testPacket2);
	  
	  for(i=0; i<UMPACKET_HEADER_SIZE; i++)
	  	putc(header[i], uart);
	  
	  putc(0x2, uart);
	  for(i=0; i<255; i++)
	  	putc(payload_test2[i], uart);
	  putc(0x3, uart);
	 
	 free(header);
	 
	 fprintf(uart, "\n23134asdfasfdsa\n");
	 
	 usleep(100000);
	  	
	}
	  
	  

	
	
}
