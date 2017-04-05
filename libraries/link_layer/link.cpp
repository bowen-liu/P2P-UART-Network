/*This file must be declared as a .cpp file since it uses the HardwareSerial Object from Arduino's library*/
#include "link.h"


/*******************************
Initialization
*******************************/

void link_init(HardwareSerial *port, uint8_t my_id, LINK_TYPE link_type, LINK *link)
{
  link->port = port;
  link->link_type = link_type;
  link->end_link_type = UNKNOWN;
  link->id = my_id;
  
  link->rbuf_writeidx = 0;
  link->rbuf_valid = 0;
  link->rbuf_expectedsize = 0;
  link->rqueue_pending = 0;
  link->rqueue_head = 0;
  link->squeue_pending = 0;
  link->squeue_lastsent = 0;
  link->rtable_entries = 0;

  
  memset(link->recvbuf, 0, RECV_BUFFER_SIZE);
  memset(link->recv_queue, 0, RECV_QUEUE_SIZE * sizeof(FRAME));
  memset(link->send_queue, 0, SEND_QUEUE_SIZE * sizeof(RAW_FRAME));
  
  //memset(link.rtable, 0, RTABLE_LENGTH * sizeof(NODE));
  for(int i=0; i<RTABLE_LENGTH; i++ )
  {
	link->rtable[i].hops = 0;
	link->rtable[i].rtt = 0;
	link->rtable[i].ticks = 0;
	link->rtable[i].last_ping_recvd = 0;
	link->rtable[i].last_ping_sent = 0;
  }
  

}


/***************************
RECEIVED FRAMES AND QUEUE THEM
***************************/

uint8_t read_serial(LINK *link)
{
  uint8_t frames_received = 0;
  size_t bytes;
  RAW_FRAME rawframe;

  //See if any new bytes are available for reading
  bytes = check_new_bytes(link);

  //Check if buffer contains one or more complete packets
  if (!(bytes > 0 || link->rbuf_valid))
	  return 0;
  
  //Extract frames from the raw receive buffer
  rawframe = extract_frame_from_rbuf(link);
  while(rawframe.size > 0)
  {
    //Parse the frame and store it in the recvd buffer
	parse_raw_and_store(rawframe, link);
    
	//Check if the rbuf contains more complete packets
    proc_buf(NULL, 0, link);
    rawframe = extract_frame_from_rbuf(link);
	frames_received++;
  }	
  
  return frames_received;
}

FRAME pop_recv_queue(LINK *link)
{
	FRAME retframe;
	FRAME *curhead;
	
	//Make sure the recv queue has pending data first
	if(link->rqueue_pending == 0)
	{
		retframe.size = 0;
		return retframe;
	}
	
	//Copy the current head out of the recv queue
	curhead = &link->recv_queue[link->rqueue_head];
	memcpy(&retframe, curhead, sizeof(FRAME));
	
	//Advances queue head and cleanup
	if (++link->rqueue_head >= RECV_QUEUE_SIZE) 
		link->rqueue_head = 0;
	
	curhead->size = 0;
	link->rqueue_pending--;
	
	
	//Remember to free the payload


	return retframe;
}


/***************************
SENDING FRAMES
***************************/

//IMPORTANT: User must free frame.payload MANUALLY when using any of these 3 functions!
uint8_t send_frame(FRAME frame, LINK *link)
{
	return add_to_send_queue(frame_to_raw(frame), link);
}

uint8_t create_send_frame(uint8_t src, uint8_t dst, uint8_t size, uchar *payload, LINK *link)
{
	return add_to_send_queue(frame_to_raw(create_frame(src, dst, size, payload)), link);
}

uint8_t create_send_cframe(uint8_t src, uint8_t dst, uint8_t size, uchar *payload, LINK *link)
{
	return add_to_send_queue(frame_to_raw(create_cframe(src, dst, size, payload)), link);
}

uint8_t transmit_next(LINK *link)
{
  uint8_t i, j;

  //Check if we have anything to transmit
  if (link->squeue_pending == 0)
    return 0;

  //Find the next packet to transmit, starting from the last sending index
  for (i = link->squeue_lastsent, j = 0; j < SEND_QUEUE_SIZE; j++)
  {
    //Wrap index i around to the beginning if needed
    if (i >= SEND_QUEUE_SIZE - 1) i = 0;
    else if (i < SEND_QUEUE_SIZE - 1) i++;

    if (link->send_queue[i].size > 0) break;
  }

  
  //Transmit the packet out onto the link
  link->port->write(link->send_queue[i].buf, link->send_queue[i].size);
  
  /*
  printf("\nTransmitting:\n");
  print_frame(raw_to_frame(link->send_queue[i]));
  printf("\n\n");
  */

  
  //cleanup & mark this slot as free
  link->squeue_lastsent = i;
  link->squeue_pending--;
  link->send_queue[i].size = 0;
  free(link->send_queue[i].buf);

  return i;
}



