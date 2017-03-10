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
	
	for(j=0; j<10; j++)
	{

	  /*Testing UMPACKET*/
	  
	  fprintf(uart, "\nhellowosasdfasfdasd\n");

	  uchar payload_test[20] = "ABCDEFGHIJKLMNOPQRS";
	  UMPACKET testPacket1 = create_umpacket(0xc, 0x6, 0x13, payload_test);
	  
	  header = umpacket_header_tobuf (&testPacket1);

	  
	  for(i=0; i<UMPACKET_HEADER_SIZE; i++)
	  	putc(header[i], uart);

	  putc(0x2, uart);
	  for(i=0; i<19; i++)
	  	putc(testPacket1.payload[i], uart);
	  putc(0x3, uart);
	  	
	  free(header);
	  
	  fprintf(uart, "\n23134asdfasfdsa\n");
	  

	usleep(50000);
	  
	  
	  /*new packet*/

	  uchar payload_test2[61] = "ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE";
	  UMPACKET testPacket2 = create_umpacket(0xE, 0xD, 0x3c, payload_test2);
	  
	  header = umpacket_header_tobuf (&testPacket2);
	  
	  for(i=0; i<UMPACKET_HEADER_SIZE; i++)
	  	putc(header[i], uart);
	  
	  putc(0x2, uart);	
	  for(i=0; i<60; i++)
	  	putc(payload_test2[i], uart);
	  putc(0x3, uart);
	 
	 free(header);
	 
	 usleep(50000);
	  	
	  	
	  /*new packet*/

	  
	  
	  uchar payload_test3[39] = "xyxyxyxyxyxyxyxyxyxxyxyxyxyxyxyxyxyxyx";
	  UMPACKET testPacket3 = create_umpacket(0xA, 0xC, 0x26, payload_test3);
	  
	  header = umpacket_header_tobuf (&testPacket3);
	  
	  for(i=0; i<UMPACKET_HEADER_SIZE; i++)
	  	putc(header[i], uart);
	  
	  putc(0x2, uart);	
	  for(i=0; i<38; i++)
	  	putc(payload_test3[i], uart);
	  putc(0x3, uart);

	  free(header);
	  
	  fprintf(uart, "\n23134asdfasfdsa\n");
	  
	  usleep(50000);


	  }
	  
	  
	  /*link-layer msg*/
	  UMPACKET testPacket4 = create_umpacket(0xc, 0, 0, NULL);
	  header = umpacket_header_tobuf (&testPacket4);
	  
	  for(i=0; i<UMPACKET_HEADER_SIZE; i++)
	  	putc(header[i], uart);

	  putc(0x2, uart);
	  putc(0x3, uart);
	  	
	  free(header);
	  usleep(50000);
	  
	  
	  /*broadcast msg*/
	  UMPACKET testPacket5 = create_umpacket(0xc, 0xF, 0, NULL);
	  header = umpacket_header_tobuf (&testPacket5);
	  
	  for(i=0; i<UMPACKET_HEADER_SIZE; i++)
	  	putc(header[i], uart);

	  putc(0x2, uart);
	  putc(0x3, uart);
	  	
	  free(header);
	  usleep(50000);

}
