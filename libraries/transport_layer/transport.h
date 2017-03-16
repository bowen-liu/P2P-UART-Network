#ifndef _UARTNET_TRANSPORTH_
#define _UARTNET_TRANSPORTH_

#include "packets.h"
#include "crc8.h"

//#include "../link_layer/link.h"
#include <link.h>

//Max concurrent connections
#define MAX_INBOUND_STREAMS			MAX_ADDRESS + 1
#define MAX_OUTBOUND_STREAMS		MAX_ADDRESS + 1
#define MAX_RECEIVED_BUF        	8


//Generic Packet datatype used to return a newly received packet
enum RECVD_TYPE {INVALID = 0, DATA, TRANSFER_REQ};

typedef struct {
  
  enum RECVD_TYPE type;
  char *data;
  RSPACKET transfer_req;

}RECVD_DATA;


//transport layer stuff
typedef struct {
	
	LINK *links;
	uint8_t links_total;
	
	//Streams that are still in-flight
	//static CONNECTION *in_streams;
	//static CONNECTION *out_streams;
	
	//Buffer for received messages or completed streams waiting to be read by the user
	RECVD_DATA recvd_queue[MAX_RECEIVED_BUF];
	int recvd_count;
	

}TRANSPORT;





//Connection states for inbound/outbound stream connections
typedef struct {

	uint8_t reliable :1;		//1 = reliable stream; 0 = unreliable stream
	uint8_t	target;				//target receiver/sender
	uint16_t id;				//ID of the stream connection
	
	uint16_t total_size;		//Total size of the stream to be transferred
	uint16_t last_ack;			//Last ACK number received (if sender) / Sent (if receiver)
	uint16_t last_transferred;	//Offset of last payload send (if sender) / received (if received)
	
	uint8_t failed_packets;		//How many consecutive failures so far?
	uint16_t rtt;				//Average RTT with the target
	
	uchar* buffer;				//Where are the stream payloads stored at?
	
} CONNECTION;



#ifdef __cplusplus
extern "C" {
#endif

TRANSPORT transport_initialize();
void transport_register_link(TRANSPORT *transport, HardwareSerial *port);

void transport_check_recv(TRANSPORT *tr);
//ECVD_PACKET parse_recvd_frame(FRAME frame, size_t bytes);

int send_umpacket(UMPACKET packet, LINK *link);
int send_uspacket(USPACKET packet, LINK *link);
int send_rspacket(RSPACKET packet, LINK *link);



#ifdef __cplusplus
}
#endif

#endif
