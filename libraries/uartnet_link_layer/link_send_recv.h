/*Backend for the link layer, primarily involving sending and receiving raw bytes*/

#ifndef _UARTNET_LINK_SEND_RECVH_
#define _UARTNET_LINK_SEND_RECVH_

#include "link_common.h"

//Receiving from link
void proc_buf(uchar *rawbuf, size_t chunk_size, LINK *link);
size_t check_complete_frame(LINK *link);
size_t check_new_bytes(LINK *link);
RAW_FRAME extract_frame_from_rbuf(LINK *link);
uint8_t parse_raw_and_store(RAW_FRAME raw, LINK *link);

//Sending to link
uint8_t add_to_send_queue(RAW_FRAME raw, LINK *link);

#endif