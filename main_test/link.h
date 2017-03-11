#ifndef _UARTNET_LINKH_
#define _UARTNET_LINKH_

#include <HardwareSerial.h>
#include "frame.h"


#define SEND_QUEUE_SIZE   5
#define RECV_BUFFER_SIZE  2*(MAX_PAYLOAD_SIZE + 16)     //add extra bytes for headers and other
#define FLUSH_THRESHOLD   RECV_BUFFER_SIZE * 0.5

//buffer for a raw unprocessed frame
typedef struct{

  size_t size;
  uchar *buf;

}RAW_FRAME;


//Link descriptor for a UART port
typedef struct{

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
int check_link_rw(LINK *link);
RAW_FRAME extract_frame_from_rbuf(LINK *link);

int add_to_send_queue(RAW_FRAME raw, LINK *link);
int transmit_next(LINK *link);







#endif
