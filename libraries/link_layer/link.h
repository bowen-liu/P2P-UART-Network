#ifndef _UARTNET_LINKH_
#define _UARTNET_LINKH_

#include <HardwareSerial.h>
#include "frame.h"

/***************************
ROUTING
***************************/

//Link Layer messages
#define LINK_MSG_SIZE               6
#define PROBE_PREAMBLE			((const char*) "!HELLO")
#define JOIN_PREAMBLE          	((const char*) "!NJOIN")
#define ROUTING_PREAMBLE		((const char*) "!RTBLE")


#define SWITCH_LINK				's'
#define NODE_LINK				'n'




/***************************
LINK
***************************/


#define RECV_BUFFER_SIZE  	2*(MAX_PAYLOAD_SIZE + 16)     //add extra bytes for headers and other
#define FLUSH_THRESHOLD   	RECV_BUFFER_SIZE * 0.5

#define RECV_QUEUE_SIZE		8
#define SEND_QUEUE_SIZE   	8

#define RTABLE_LENGTH		MAX_ADDRESS			//TODO: exclude "0" and broadcast address



//Used for the routing table. Maybe declare in routing.h instead?
#define NODE_LENGTH		2		//size of id + hops
typedef struct{
	
	uint8_t id;
	uint8_t hops;
	
}NODE;


typedef enum {UNKNOWN = 0, GATEWAY, ENDPOINT} LINK_TYPE;


//Link descriptor for a UART port
typedef struct{

  //Physical Link Configurations
  HardwareSerial *port;
  LINK_TYPE link_type;					//What is this link configured as?
  LINK_TYPE end_link_type;				//What is the other end of this link configured as?
  uint8_t id;							//My GUID
  
  
  //Raw Receive buffer for this link
  uchar recvbuf[RECV_BUFFER_SIZE];
  uint16_t rbuf_writeidx;             //TODO: Make this into a circular buffer
  uint8_t rbuf_valid;
  uint16_t rbuf_expectedsize;
  
  //Pending Frames to be used by upper layers
  FRAME recv_queue[RECV_QUEUE_SIZE];
  uint8_t rqueue_pending;
  uint8_t rqueue_head;
  

  //Send buffer
  RAW_FRAME send_queue[SEND_QUEUE_SIZE];
  uint8_t squeue_pending;
  uint8_t squeue_lastsent;
  
  //Routing Table
  NODE rtable[RTABLE_LENGTH];
  uint8_t rtable_entries;
  
}LINK;



//Functions


LINK link_init(HardwareSerial *port, uint8_t my_id, LINK_TYPE link_type);


void proc_buf(uchar *rawbuf, size_t chunk_size, LINK *link);
size_t check_new_bytes(LINK *link);
RAW_FRAME extract_frame_from_rbuf(LINK *link);


uint8_t parse_raw_and_store(RAW_FRAME raw, LINK *link);
FRAME pop_recv_queue(LINK *link);
uint8_t read_serial(LINK *link);

uint8_t add_to_send_queue(RAW_FRAME raw, LINK *link);
uint8_t transmit_next(LINK *link);
uint8_t send_frame(FRAME frame, LINK *link);
uint8_t create_send_frame(uint8_t src, uint8_t dst, uint8_t size, uchar *payload, LINK *link);


//rOUTING fUNCTIONS

uint8_t update_rtable_entry(uint8_t id, uint8_t hops, LINK *link);

uint8_t send_probe_msg(uint8_t my_id, LINK *link);
uint8_t send_join_msg(uint8_t my_id, LINK *link);
uint8_t send_rtble_msg(uint8_t dst, LINK *link);

uint8_t parse_probe_msg(FRAME frame, LINK *link);
uint8_t parse_join_msg(FRAME frame, LINK *link);
uint8_t parse_rtble_msg(FRAME frame, LINK *link);
uint8_t parse_routing_frame(FRAME frame, LINK *link);




#endif
