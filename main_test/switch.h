#ifndef _UARTNET_SWITCHH_
#define _UARTNET_SWITCHH_

#include "packets.h"
#include "send_recv.h"

#define RECV_BUFFERS      4
#define RECV_BUFFER_SIZE  2*(MAX_PAYLOAD_SIZE + 16)     //add extra bytes for headers and other
#define FLUSH_THRESHOLD   RECV_BUFFER_SIZE * 0.5


//probing messages
#define PROBE_MSG					((const char*) "WHATAREYOU?")


typedef struct{

  uchar recvbuf[RECV_BUFFER_SIZE];
  size_t bytes_current;             //Also used as write index
  size_t bytes_remaining;
  
}PENDING_PKT;


#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif
