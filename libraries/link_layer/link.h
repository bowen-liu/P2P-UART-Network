#ifndef _UARTNET_LINKH_
#define _UARTNET_LINKH_

#include <HardwareSerial.h>
#include "frame.h"

//Default configuration values if not provided by the user
#ifndef SEND_QUEUE_SIZE
#define SEND_QUEUE_SIZE   5
#endif

#ifndef RECV_BUFFER_SIZE
#define RECV_BUFFER_SIZE  2*(MAX_PAYLOAD_SIZE + 16)     //add extra bytes for headers and other
#endif

#ifndef FLUSH_THRESHOLD
#define FLUSH_THRESHOLD   RECV_BUFFER_SIZE * 0.5
#endif


//Link Layer messages
#define LINK_MSG_SIZE               6
#define PROBE_MSG_PREAMBLE          ((const char*) "!HELLO")


//Link descriptor for a UART port
typedef struct{

  uint8_t id;
  //enum PORT_TYPE {GATEWAY, NODE} port_type;
  HardwareSerial *port;
  

  //Receive buffer for this link
  uchar recvbuf[RECV_BUFFER_SIZE];
  uint16_t rbuf_writeidx;             //TODO: Make this into a circular buffer
  uint8_t rbuf_valid;
  uint16_t rbuf_expectedsize;

  //Send buffer
  RAW_FRAME send_queue[SEND_QUEUE_SIZE];
  uint8_t squeue_pending;
  uint8_t squeue_lastsent;
  
}LINK;



//Functions


LINK link_init(HardwareSerial *port);

void proc_buf(uchar *rawbuf, size_t chunk_size, LINK *link);
size_t check_link_rw(LINK *link);
RAW_FRAME extract_frame_from_rbuf(LINK *link);
FRAME parse_raw_frame(RAW_FRAME raw);


uint8_t add_to_send_queue(RAW_FRAME raw, LINK *link);
uint8_t transmit_next(LINK *link);
uint8_t send_frame(FRAME frame, LINK *link);
uint8_t create_send_frame(uint8_t src, uint8_t dst, uint8_t size, uchar *payload, LINK *link);


#endif
