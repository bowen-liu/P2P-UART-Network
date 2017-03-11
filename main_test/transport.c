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
  int i;
  uint16_t preamble = *((uint16_t*)&buf[0]);
  char *payload;

  //print_bytes(buf, bytes);

  //Check if receive packet buffer is full
  if (recvd_packets_count == MAX_RECEIVED_BUF)
  {
    printf("Received Packet Buffer is full! Dropping packet... \n");
    return;
  }
  recvd_packets_count++;

  //Find a free spot in the received packet buffer
  for (i = 0; i < MAX_RECEIVED_BUF; i++)
    if (recvd_packets[i].type == INVALID) break;

  //Make sure this is a packet we know
  if (preamble != FRAME_PREAMBLE)
    return -1;


  //Todo: read sub-preamble to determine the packet type
  


  //Store the packet in the buffer
  //recvd_packets[i].type = PACKET_TYPE;
  recvd_packets[i].type = UMPACKET_TYPE;
  recvd_packets[i].umpacket =  buf_to_frame(buf);

  //Check if payload is complete
  if (buf[FRAME_HEADER_SIZE] != STX)
    printf("STX not found, packet header may be corrupt!\n");
  if (buf[FRAME_HEADER_SIZE + recvd_packets[i].umpacket.size + 1] != ETX)
    printf("ETX not found, packet may be corrupt or payload was truncated!\n");

  //Create a new buffer for the payload
  recvd_packets[i].umpacket.payload = malloc(recvd_packets[i].umpacket.size);
  memcpy(recvd_packets[i].umpacket.payload, &buf[FRAME_HEADER_SIZE + 1], recvd_packets[i].umpacket.size);

  printf("\n****************************\n");
  print_frame(recvd_packets[i].umpacket);
  printf("payload: ");
  print_bytes(recvd_packets[i].umpacket.payload, recvd_packets[i].umpacket.size);
  printf("\n****************************\n");

  //Remember to mark a checked packet as INVALID, and free its payload when done!


  return 1;
}









