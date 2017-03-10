#include "transport.h"

//Streams that are still in-flight
static CONNECTION *in_streams;
static CONNECTION *out_streams;

//Buffer for newly received packets pending to be processed by the application
static RECEIVED_PACKET *recvd_packets;
static int recvd_packets_count;


/***************************
Front-end Functions
***************************/

void network_task()
{
	static uint8_t last_sent = 0;
	
	while(1)
	{
		
	}
	
}

void transport_initialize()
{
  recvd_packets_count = 0;
  
  in_streams = calloc(MAX_INBOUND_STREAMS, sizeof(CONNECTION));
  out_streams = calloc(MAX_OUTBOUND_STREAMS, sizeof(CONNECTION));
  recvd_packets = calloc(MAX_RECEIVED_BUF, sizeof(RECEIVED_PACKET));

}

int send_umessage(uint8_t src, uint8_t dst, uint8_t size, uchar *data)
{
	if(src > MAX_ADDRESS || dst > MAX_ADDRESS)
	{
		printf("***ERROR (send_umessage): src or dst size is greater than %d\n", MAX_ADDRESS);
		return -1;
	}
	
	if(size > MAX_PAYLOAD_SIZE)
	{
		printf("***WARNING (send_umessage): data size is greater than %d. Trying to send as ustream instead...\n", MAX_PAYLOAD_SIZE);
		return send_ustream(src, dst, size, data);
	}

 //Sending Header
	
	return 0;
}


int send_ustream(uint8_t src, uint8_t dst, uint8_t size, uchar *data)
{
	if(src > MAX_ADDRESS || dst > MAX_ADDRESS)
	{
		printf("***ERROR (send_ustream): src or dst size is greater than %d\n", MAX_ADDRESS);
		return -1;
	}
	
	if(size > MAX_STREAM_SIZE)
	{
		printf("***ERROR (send_ustream): data size is greater than %d\n", MAX_STREAM_SIZE);
		return -1;
	}
	
	return 0;
}


int send_rstream(uint8_t src, uint8_t dst, uint8_t size, uchar *data)
{
	if(src > MAX_ADDRESS || dst > MAX_ADDRESS)
	{
		printf("***ERROR (send_rstream): src or dst size is greater than %d\n", MAX_ADDRESS);
		return -1;
	}
	
	if(size > MAX_STREAM_SIZE)
	{
		printf("***ERROR (send_rstream): data size is greater than %d\n", MAX_STREAM_SIZE);
		return -1;
	}
	
	return 0;
}

int parse_recvd_packet(uchar *buf, size_t bytes)
{
	int i;
	uint16_t preamble = *((uint16_t*)&buf[0]);
  char *payload;

  //print_bytes(buf, bytes);

  //Check if receive packet buffer is full
  if(recvd_packets_count == MAX_RECEIVED_BUF)
  {
    printf("Received Packet Buffer is full! Dropping packet... \n"); 
    return;
  }
  recvd_packets_count++;
  
  //Find a free spot in the received packet buffer
  for(i=0; i<MAX_RECEIVED_BUF; i++)
    if(recvd_packets[i].type == INVALID) break;

  
  switch(preamble)
  {
    case UMPACKET_PREAMBLE:
    
      //Store the packet in the buffer
      recvd_packets[i].type = UMPACKET_TYPE;
      recvd_packets[i].umpacket =  buf_to_umpacket(buf);

      //Check if payload is complete
      if(buf[UMPACKET_HEADER_SIZE] != STX)
        printf("STX not found, packet header may be corrupt!\n");
      if(buf[UMPACKET_HEADER_SIZE + recvd_packets[i].umpacket.size + 1] != ETX)
        printf("ETX not found, packet may be corrupt or payload was truncated!\n");

      //Create a new buffer for the payload
      recvd_packets[i].umpacket.payload = malloc(recvd_packets[i].umpacket.size);
      memcpy(recvd_packets[i].umpacket.payload, &buf[UMPACKET_HEADER_SIZE + 1], recvd_packets[i].umpacket.size);
     
      printf("\n****************************\n");
      print_umpacket(recvd_packets[i].umpacket);
      printf("payload: ");
      print_bytes(recvd_packets[i].umpacket.payload, recvd_packets[i].umpacket.size);
      printf("\n****************************\n");

      //Remember to mark a checked packet as INVALID, and free its payload when done!
      

      

        

      
    case USPACKET_PREAMBLE:


      
    case RSPACKET_SYN_PREAMBLE:
    case RSPACKET_ACK_PREAMBLE:

        
      
    case RSPACKET_DATA_PREAMBLE:

    
    default:
      //printf("check_packet_complete: invalid preamble at the start of rbuf\n");
      return -1;
  }

	return 0;
}









