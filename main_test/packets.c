#include "packets.h"
#include "crc8.h"


/***************************
COMMON
***************************/

//WARNING: pay attention to endianness!
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




/***************************
Unreliable Message Packets
***************************/


UMPACKET create_umpacket(uint8_t src, uint8_t dst, uint8_t size, uchar *payload)
{
	UMPACKET packet;
	
	packet.preamble = UMPACKET_PREAMBLE;
	packet.src = src;
	packet.dst = dst;
	packet.size = size;
	packet.payload = payload;
	
	return packet;
}


uchar* umpacket_header_tobuf (UMPACKET *packet)
{
	uchar* header_str = malloc(UMPACKET_HEADER_SIZE + 1);
	
	memcpy(header_str, (uchar*) packet, UMPACKET_HEADER_SIZE);
	
	//printf("Size of header: %d\n", UMPACKET_HEADER_SIZE);
	//print_bytes((uchar*) packet, UMPACKET_HEADER_SIZE);
	
	return header_str;
}

UMPACKET buf_to_umpacket(uchar* buf)
{
	UMPACKET packet;
	
	packet.preamble = *((uint16_t*)&buf[0]);
	
	//hard coded way to extract dst/src
	packet.dst 		= ((*((uint8_t*) &buf[2])) >> 4);
	packet.src 		= (*((uint8_t*) &buf[2])) & 0x0F;
	
	packet.size 	= *((uint8_t*)&buf[3]);
	
	//print_umpacket(packet);

	return packet;
}


void print_umpacket(UMPACKET packet)
{
	printf("preamble: %X\n", packet.preamble);
	printf("src: %X\n", packet.src);
	printf("dst: %X\n", packet.dst);
	printf("size: %X\n", packet.size);
}


/***************************
Unreliable Stream Packets
***************************/


USPACKET create_uspacket(uint8_t src, uint8_t dst, uint8_t id, uint8_t total_size, uint16_t payload_offset, uint8_t payload_size, uchar *payload)
{
	USPACKET packet;
	
	packet.preamble = USPACKET_PREAMBLE;
	
	packet.src = src;
	packet.dst = dst;
	packet.id = id;
	packet.total_size = total_size;
	packet.payload_size = payload_size;
	packet.payload_offset = payload_offset;
	
	packet.payload = payload;
	
	return packet;
}


uchar* uspacket_header_tobuf (USPACKET *packet)
{
	uchar* header_str = malloc(USPACKET_HEADER_SIZE + 1);
	
	memcpy(header_str, (uchar*) packet, USPACKET_HEADER_SIZE);
	
	//printf("Size of header: %d\n", USPACKET_HEADER_SIZE);
	//print_bytes((uchar*) packet, USPACKET_HEADER_SIZE);
	
	return header_str;
}

USPACKET buf_to_uspacket(uchar* buf)
{
	USPACKET packet;
	
	packet.preamble = *((uint16_t*)&buf[0]);
	
	
	//hard coded way to extract dst/src
	packet.dst 		= ((*((uint8_t*) &buf[2])) >> 4);
	packet.src 		= (*((uint8_t*) &buf[2])) & 0x0F;
	
	packet.id 		= *((uint16_t*)&buf[3]);
	packet.total_size 	= *((uint16_t*)&buf[5]);
	packet.payload_offset 	= *((uint16_t*)&buf[7]);
	packet.payload_size 	= *((uint8_t*)&buf[9]);
	
	//print_uspacket(packet);

	return packet;
}

void print_uspacket(USPACKET packet)
{
	printf("preamble: %X\n", packet.preamble);
	printf("src: %X\n", packet.src);
	printf("dst: %X\n", packet.dst);
	printf("id: %X\n", packet.id);
	printf("total_size: %X\n", packet.total_size);
	printf("payload_offset: %X\n", packet.payload_offset);
	printf("payload_size: %X\n", packet.payload_size);
}


/***************************
Reliable Message Packets
***************************/

RSPACKET create_rspacket(uint16_t preamble, uint8_t src, uint8_t dst, uint16_t id, uint16_t size, uchar *payload)
{
	RSPACKET packet;

	packet.preamble = preamble;
	packet.src = src;
	packet.dst = dst;
	packet.id = id;
	packet.size = size;
	packet.payload = payload;
	

	//generate CRC8 checksum of the header 
	packet.checksum = crc8_block(INITIAL_CRC, (uchar*) &packet, RSPACKET_HEADER_SIZE);
	
	//generate the CRC of the payload by continuing with the header's checksum
	if(packet.payload != NULL)
		packet.checksum = crc8_block(packet.checksum, (uchar*) packet.payload, packet.size);
	
	return packet;
}


RSPACKET create_rspacket_syn(uint8_t src, uint8_t dst, uint16_t size)
{	
	srand((unsigned) time(NULL));
	return create_rspacket(RSPACKET_SYN_PREAMBLE, src, dst, (uint16_t)rand(), size, NULL);
}


RSPACKET create_rspacket_ack(uint8_t src, uint8_t dst, uint16_t id, uint16_t size)
{	
	return create_rspacket(RSPACKET_ACK_PREAMBLE, src, dst, id, size, NULL);
}


RSPACKET create_rspacket_data(uint8_t src, uint8_t dst, uint16_t id, uint16_t size, uchar *payload)
{	
	return create_rspacket(RSPACKET_DATA_PREAMBLE, src, dst, id, size, payload);
}


uchar* rspacket_header_tobuf (RSPACKET *packet)
{
	uchar* header_str = malloc(RSPACKET_HEADER_SIZE + 1);
	
	memcpy(header_str, (uchar*) packet, RSPACKET_HEADER_SIZE);
	
	//printf("Size of header: %d\n", RSPACKET_HEADER_SIZE);
	//print_bytes((uchar*) packet, RSPACKET_HEADER_SIZE);
	
	return header_str;
}


RSPACKET buf_to_rspacket(uchar* buf)
{
	RSPACKET packet;
	
	packet.preamble = *((uint16_t*)&buf[0]);
	
	//return a packet with preamble = 0 to indicate an error, if the received preamble is not for rspacket
	/*if(!(packet.preamble == RSPACKET_SYN_PREAMBLE || packet.preamble == RSPACKET_ACK_PREAMBLE || packet.preamble == RSPACKET_DATA_PREAMBLE))
	{
		printf("Preamble not recognized for RSPACKET");
		packet.preamble = 0;
		return packet;
	}*/
	
	//hard coded way to extract dst/src
	packet.dst 		= ((*((uint8_t*) &buf[2])) >> 4);
	packet.src 		= (*((uint8_t*) &buf[2])) & 0x0F;
	
	packet.id 		= *((uint16_t*)&buf[3]);
	packet.size 	= *((uint8_t*)&buf[5]);
	packet.checksum = *((uint8_t*)&buf[7]);
	
	//print_rspacket(packet);

	return packet;
}


void print_rspacket(RSPACKET packet)
{
	printf("preamble: %X ", packet.preamble);
	
	switch(packet.preamble)
	{
		case RSPACKET_SYN_PREAMBLE:
			printf("(SYN)\n");
			break;
		
		case RSPACKET_ACK_PREAMBLE:
			printf("(ACK)\n");
			break;
		
		case RSPACKET_DATA_PREAMBLE:
			printf("(DATA)\n");
			break;
		
		default:
			printf("(UNKNOWN)\n");
	}
	
	printf("src: %X\n", packet.src);
	printf("dst: %X\n", packet.dst);
	printf("id: %X\n", packet.id);
	printf("size: %X\n", packet.size);
	printf("checksum: %X\n", packet.checksum);
}
