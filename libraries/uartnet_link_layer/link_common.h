/*Common declarations shared between all link layer modules*/

#ifndef _UARTNET_LINK_COMMONH_
#define _UARTNET_LINK_COMMONH_

#include "frame.h"

#include "Arduino.h"
#include <HardwareSerial.h>


/*******************************
Routing Table Nodes
*******************************/

#define NODE_LENGTH		2		//size of id + hops

typedef struct{
	
	uint8_t hops;
	uint16_t rtt;						//RTT from the last PING/PONG
	
	uint8_t ticks;						//Ticks elapsed since last heard from this node
	unsigned long last_ping_recvd;		//When was the last time a ping was received
	unsigned long last_ping_sent;		//"Timestamp" of when a ping is sent to this node. Used to calculate RTT when this node replies to the ping

}NODE;


/*******************************
Link descriptor
*******************************/

//#define RECV_BUFFER_SIZE  	2*(MAX_PAYLOAD_SIZE + 16)     //add extra bytes for headers and other
#define RECV_BUFFER_SIZE  	MAX_PAYLOAD_SIZE 
#define FLUSH_THRESHOLD   	RECV_BUFFER_SIZE * 0.5

#define RECV_QUEUE_SIZE		8
#define SEND_QUEUE_SIZE   	8

#define RTABLE_LENGTH		MAX_ADDRESS			//TODO: exclude "0" and broadcast address


typedef enum {UNKNOWN = 0, GATEWAY, ENDPOINT} LINK_TYPE;


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




#endif