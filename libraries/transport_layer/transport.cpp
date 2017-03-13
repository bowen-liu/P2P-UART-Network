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

  while (1)
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


int parse_recvd_packet(uchar *buf, size_t bytes)
{
 

  return 1;
}



/***************************
  SENDING SINGLE PACKET
***************************/

int send_umpacket(UMPACKET packet, LINK *link)
{
	//return create_send_frame(packet.src, packet.dst, packet.size, packet.payload, link);
	return send_frame((FRAME)packet, link);
}


int send_uspacket(USPACKET packet, LINK *link)
{
	FRAME frame;
	int retval;
	
	/*** uspacket_to_frame ***/
	
	frame = create_frame(packet.src, packet.dst, (packet.payload_size + USPACKET_HEADER_EXTRA), NULL);
	frame.payload = malloc(frame.size);
	
	//Copy the secondary header fields into the beginning of the payload. Skip src, dst, payload_size
	memcpy(frame.payload, &(((uchar*)&packet)[2]), USPACKET_HEADER_EXTRA);
	
	//Append the remaining payload
	memcpy(&frame.payload[USPACKET_HEADER_EXTRA], packet.payload, packet.payload_size);
	
	/*************************/
	
	
	//Sending the packed uspacket frame
	retval = send_frame(frame, link);
	free(frame.payload);
	
	return retval;
}









