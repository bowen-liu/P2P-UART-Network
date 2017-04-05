/*Front end functions for the link layer. Intended to be used by the user.*/

#ifndef _UARTNET_LINKH_
#define _UARTNET_LINKH_

#include "link_common.h"
#include "link_send_recv.h"
#include "routing.h"


/*******************************
Initialization
*******************************/

void link_init(HardwareSerial *port, uint8_t my_id, LINK_TYPE link_type, LINK *link);


/*******************************
Reading the link, and retrieve from queue
*******************************/

uint8_t read_serial(LINK *link);
FRAME pop_recv_queue(LINK *link);


/*******************************
Sending Frames
*******************************/

uint8_t send_frame(FRAME frame, LINK *link);
uint8_t create_send_frame(uint8_t src, uint8_t dst, uint8_t size, uchar *payload, LINK *link);
uint8_t create_send_cframe(uint8_t src, uint8_t dst, uint8_t size, uchar *payload, LINK *link);
uint8_t transmit_next(LINK *link);



#endif
