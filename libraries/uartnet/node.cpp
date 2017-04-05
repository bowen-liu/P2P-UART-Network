#include "node.h"

static LINK link;

/******************************/
//Node functions
/******************************/


void proc_frame(FRAME frame, LINK *link)
{
  //Is this packet from the switch itself?
  if (frame.src == 0 || frame.preamble == CFRAME_PREAMBLE)
  {   
    parse_control_frame(frame, link);
    free(frame.payload);
    return;
  }

  //Handle broadcast packets. End nodes will treat broadcast frames as normal frames
  if (frame.dst == MAX_ADDRESS)
    printf("Broadcasting Packet!\n");

  //Call the user's message parser
  message_handler(frame);
  free(frame.payload);
  return;

  //Send the received frame to the transport layer for further processing
  //parse_recvd_frame(frame);

}


/******************************/
//main
/******************************/

LINK* node_init(uint8_t id, int8_t (*mparser)(FRAME))
{
  //Initializing transport layer data for serial1
  Serial1.begin(115200);
  link_init(&Serial1, id, ENDPOINT, &link);

  //Set the message parser
  set_message_handler(mparser);

  //Send out a HELLO message out onto the link
  send_hello(link.id, 0, &link);

  return &link;
}

//uint8_t done = 0;
void net_task(uint8_t continuous)
{
  while (1)
  {

    //Attempt to read serial. Process any pending frames if received anything new
    if (read_serial(&link) > 0)
    {
      while (link.rqueue_pending > 0)
        proc_frame(pop_recv_queue(&link), &link);
    }

    //Transmit a packet in the sending queue, if any
    transmit_next(&link);
    delay(100);

    if (!continuous) break;
  }
}



