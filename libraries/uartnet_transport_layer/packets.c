#include "packets.h"
#include "crc8.h"

//We always assume the payloads for frames and packets are not dynamically allocated, or it's part of a larger buffer


/***************************
Unreliable Message Packets
***************************/

UMPACKET create_umpacket(uint8_t src, uint8_t dst, uint8_t size, uchar *payload)
{
	UMPACKET packet;
	
	//Primary Header Fields
	packet.src 				= src;					
	packet.dst 				= dst;
	packet.size 			= size;			

	packet.payload 			= payload;
	
	return packet;
}






/***************************
Unreliable Stream Packets
***************************/

USPACKET create_uspacket(uint8_t src, uint8_t dst, uint16_t id, uint8_t total_size, uint16_t payload_offset, uint8_t payload_size, uchar *payload)
{
	USPACKET packet;
	
	//Size check
	if(payload_size > USPACKET_MAX_PAYLOAD)
	{
		printf("create_uspacket: Payload size exceeds maximum!\n");
		payload_size = USPACKET_MAX_PAYLOAD;
	}
	
	//Primary Header Fields
	packet.src 				= src;					//Will be added to the FRAME src/dst field
	packet.dst 				= dst;
	packet.payload_size 	= payload_size;			//Calculated from the frame size by frame.size - USPACKET_HEADER_EXTRA
	
	//Secondary Header Fields
	packet.type 			= USPACKET_PREAMBLE;
	packet.id 				= id;
	packet.total_size 		= total_size;
	packet.payload_offset 	= payload_offset;
	
	packet.payload 			= payload;
	
	return packet;
}


/*
FRAME uspacket_to_frame(USPACKET packet)
{
	FRAME frame = create_frame(packet.src, packet.dst, (packet.payload_size + USPACKET_HEADER_EXTRA), NULL);
	
	//Allocate memory for the frame's payload
	frame.payload = (uchar*)malloc(frame.size);
	
	//Copy the secondary header fields into the beginning of the payload. Skip src, dst, payload_size
	memcpy(frame.payload, &(((uchar*)&packet)[2]), USPACKET_HEADER_EXTRA);
	
	//Append the remaining payload
	memcpy(&frame.payload[USPACKET_HEADER_EXTRA], packet.payload, packet.payload_size);
	
	//WARNING: frame.payload is dynamically allocated! You must free this manually!
	
	return frame;
}
*/

USPACKET frame_to_uspacket(FRAME frame)
{
	USPACKET packet;
	
	//Extract the primary headers
	packet.src = frame.src;
	packet.dst = frame.dst;
	packet.payload_size = frame.size - USPACKET_HEADER_EXTRA;
	
	//Extract the secondary headers
	packet.type = *((uint16_t*)&frame.payload[0]);
	packet.id = *((uint16_t*)&frame.payload[2]);
	packet.total_size = *((uint16_t*)&frame.payload[4]);
	packet.payload_offset = *((uint16_t*)&frame.payload[6]);
	
	//Copy the payload without the secondary headers
	packet.payload = &frame.payload[USPACKET_HEADER_EXTRA]; //good idea? maybe use memmov to remove the uneeded bytes?
	
	return packet;
	
}

void print_uspacket(USPACKET packet)
{
	printf("type: %X\n", packet.type);
	printf("src: %X\n", packet.src);
	printf("dst: %X\n", packet.dst);
	printf("id: %X\n", packet.id);
	printf("total_size: %X\n", packet.total_size);
	printf("payload_offset: %X\n", packet.payload_offset);
	printf("payload_size: %X\n", packet.payload_size);
	
	printf("payload: ");
	print_bytes(packet.payload, packet.payload_size);
	printf("\n\n");
}




/***************************
Reliable Message Packets
***************************/

RSPACKET create_rspacket(uint16_t preamble, uint8_t src, uint8_t dst, uint8_t payload_size, uint16_t id, uint32_t payload_offset, uchar *payload)
{
	RSPACKET packet;

	packet.src = src;
	packet.dst = dst;
	packet.payload_size = payload_size;
	
	packet.type = preamble;
	packet.id = id;
	packet.payload_offset = payload_offset;
	packet.payload = payload;
	

	/*
	//generate CRC8 checksum of the header 
	packet.checksum = crc8_block(INITIAL_CRC, (uchar*) &packet, RSPACKET_HEADER_SIZE);
	
	//generate the CRC of the payload by continuing with the header's checksum
	if(packet.payload != NULL)
		packet.checksum = crc8_block(packet.checksum, (uchar*) packet.payload, packet.size);
	*/
	packet.checksum = 0xFF;
	
	return packet;
}

RSPACKET create_rspacket_syn(uint8_t src, uint8_t dst, uint32_t stream_size)
{	
	srand((unsigned) time(NULL));
	return create_rspacket(RSPACKET_SYN_PREAMBLE, src, dst, stream_size, (uint16_t)rand(), 0, NULL);
}


RSPACKET create_rspacket_ack(uint8_t src, uint8_t dst, uint8_t payload_size, uint16_t id, uint32_t payload_offset)
{	
	return create_rspacket(RSPACKET_ACK_PREAMBLE, src, dst, payload_size, id, payload_offset, NULL);
}


RSPACKET create_rspacket_data(uint8_t src, uint8_t dst, uint8_t payload_size, uint16_t id, uint32_t payload_offset, uchar *payload)
{	
	return create_rspacket(RSPACKET_DATA_PREAMBLE, src, dst, payload_size, id, payload_offset, payload);
}


RSPACKET frame_to_rspacket(FRAME frame)
{
	RSPACKET packet;
	
	//Extract the primary headers
	packet.src = frame.src;
	packet.dst = frame.dst;
	packet.payload_size = frame.size - RSPACKET_HEADER_EXTRA;
	
	//Extract the secondary headers
	packet.type = *((uint16_t*)&frame.payload[0]);
	packet.id = *((uint16_t*)&frame.payload[2]);
	packet.payload_offset = *((uint16_t*)&frame.payload[4]);
	packet.checksum = *((uint16_t*)&frame.payload[8]);
	
	//Copy the payload without the secondary headers
	packet.payload = &frame.payload[RSPACKET_HEADER_EXTRA]; //good idea? maybe use memmov to remove the uneeded bytes?
	
	return packet;
	
}

/*
FRAME rspacket_to_frame(RSPACKET packet)
{
	FRAME frame;
	int retval;
	size_t actual_pl_size;
	

	//Determine the actual payload size for the FRAME. SYN/ACK packets only have the secondary headers in the frame payload.
	if(packet.type == RSPACKET_DATA_PREAMBLE) 
		actual_pl_size = packet.payload_size + RSPACKET_HEADER_EXTRA;
	else 
		actual_pl_size = RSPACKET_HEADER_EXTRA;
	
	//Create a new frame to encapsulate the RSPACKET
	frame = create_frame(packet.src, packet.dst, actual_pl_size, NULL);
	
	//Allocate memory for the frame's payload
	frame.payload = (uchar*)malloc(frame.size);
	
	//Copy the secondary header fields into the beginning of the payload. Skip src, dst, payload_size
	memcpy(frame.payload, &(((uchar*)&packet)[2]), RSPACKET_HEADER_EXTRA);
	
	//Append the remaining payload if it's a DATA packet
	if(packet.type == RSPACKET_DATA_PREAMBLE)
		memcpy(&frame.payload[RSPACKET_HEADER_EXTRA], packet.payload, packet.payload_size);
	
	//WARNING: frame.payload is dynamically allocated! You must free this manually!
	
	return frame;
}
*/

void print_rspacket(RSPACKET packet)
{
	printf("src: %X\n", packet.src);
	printf("dst: %X\n", packet.dst);
	
	printf("type: %X ", packet.type);
	switch(packet.type)
	{
		case RSPACKET_SYN_PREAMBLE:
			printf("(SYN)\n");
			printf("Total Stream Size: %X\n", packet.payload_size);
			break;
		
		case RSPACKET_ACK_PREAMBLE:
			printf("(ACK)\n");
			printf("Acked byte: %X\n", packet.payload_size);
			break;
		
		case RSPACKET_DATA_PREAMBLE:
			printf("(DATA)\n");
			printf("payload_size: %X\n", packet.payload_size);
			break;
		
		default:
			printf("(UNKNOWN)\n");
			return;
	}

	printf("id: %X\n", packet.id);
	printf("payload_offset: %X\n", packet.payload_offset);
	printf("checksum: %X\n", packet.checksum);
	
	if(packet.type == RSPACKET_DATA_PREAMBLE)
	{
		printf("payload: ");
		print_bytes(packet.payload, packet.payload_size);
		printf("\n\n");
	}

}


